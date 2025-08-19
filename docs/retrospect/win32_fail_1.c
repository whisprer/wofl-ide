// ultra-minimal single-window code editor / mini-IDE (Win32)
// build: see /ide-min/build.bat
#define WIN32_LEAN_AND_MEAN
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "editor.h"

// ------------------------------ helpers

static void app_clear_selection(AppState *app){ app->selecting=false; app->sel_anchor = app->caret; }
static void app_mark_changed(AppState *app, const wchar_t *s, size_t n){
    // cheap line recount trigger
    for(size_t i=0;i<n;i++){ if(s[i]==L'\n'){ app->need_recount=true; break; } }
}

static void path_split(const wchar_t *full, wchar_t *dir, wchar_t *name){
    const wchar_t *slash = wcsrchr(full, L'\\');
    const wchar_t *slash2= wcsrchr(full, L'/');
    if(!slash || (slash2 && slash2>slash)) slash=slash2;
    if(!slash){ dir[0]=0; wcsncpy_s(name, WOFL_MAX_PATH, full, _TRUNCATE); }
    else{
        wcsncpy_s(dir, WOFL_MAX_PATH, full, slash-full);
        dir[slash-full]=0;
        wcsncpy_s(name, WOFL_MAX_PATH, slash+1, _TRUNCATE);
    }
}

static void app_set_file(AppState *app, const wchar_t *path){
    wcsncpy_s(app->file_path, WOFL_MAX_PATH, path, _TRUNCATE);
    path_split(path, app->file_dir, app->file_name);
    app->lang = lang_from_ext(app->file_name);
    app->need_recount = true;
    app->total_lines_cache = 0;
    if(!config_try_load_run_cmd(app)) config_set_default_run_cmd(app);
}

static void app_new_empty(AppState *app){
    if(app->buf.data) gb_free(&app->buf);
    gb_init(&app->buf);
    app->buf.dirty=false; app->buf.eol_mode=EOL_LF;
    app->caret.line=0; app->caret.col=0; app->caret.index=0;
    app->top_line=0; app->left_col=0; app->selecting=false;
    app->file_path[0]=app->file_dir[0]=app->file_name[0]=0;
    app->lang=LANG_NONE;
    app->need_recount=true; app->total_lines_cache=1;
    config_set_default_run_cmd(app);
}

static bool app_load_file(AppState *app, const wchar_t *path){
    EolMode eol=EOL_LF;
    if(app->buf.data) gb_free(&app->buf);
    gb_load_from_utf8_file(&app->buf, path, &eol);
    app_set_file(app, path);
    app->caret.line=0; app->caret.col=0; app->caret.index=0;
    app->top_line=0; app->left_col=0; app->selecting=false;
    app->need_recount=true;
    return true;
}

static bool app_save_file(AppState *app, const wchar_t *path){
    bool ok = gb_save_to_utf8_file(&app->buf, path, app->buf.eol_mode);
    if(ok){ app_set_file(app, path); }
    return ok;
}

// selection helpers
static bool app_get_selection(AppState *app, size_t *a, size_t *b){
    size_t cidx = editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
    size_t aidx = editor_linecol_to_index(&app->buf, app->sel_anchor.line, app->sel_anchor.col);
    if(cidx==aidx) return false;
    *a = cidx<aidx?cidx:aidx;
    *b = cidx>aidx?cidx:aidx;
    return true;
}
static void app_delete_selection(AppState *app){
    size_t a,b; if(!app_get_selection(app,&a,&b)) return;
    gb_delete_range(&app->buf, a, b-a);
    editor_index_to_linecol(&app->buf,a,&app->caret.line,&app->caret.col);
    app->need_recount=true;
    app_clear_selection(app);
}

static void app_clip_copy(AppState *app, bool cut){
    size_t a,b; if(!app_get_selection(app,&a,&b)) return;
    size_t n=b-a;
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (n+1)*sizeof(wchar_t));
    if(!h) return;
    wchar_t *dst = (wchar_t*)GlobalLock(h);
    for(size_t i=0;i<n;i++) dst[i]=gb_char_at(&app->buf, a+i);
    dst[n]=0; GlobalUnlock(h);
    if(OpenClipboard(app->hwnd)){
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, h);
        CloseClipboard();
    }else{
        GlobalFree(h);
    }
    if(cut){ app_delete_selection(app); InvalidateRect(app->hwnd,NULL,FALSE); }
}

static void app_clip_paste(AppState *app){
    if(!OpenClipboard(app->hwnd)) return;
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if(!h){ CloseClipboard(); return; }
    const wchar_t *src = (const wchar_t*)GlobalLock(h);
    if(!src){ CloseClipboard(); return; }
    if(app->selecting) app_delete_selection(app);
    size_t ins = wcslen(src);
    gb_move_gap(&app->buf, editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col));
    gb_insert(&app->buf, src, ins);
    app_mark_changed(app, src, ins);
    editor_index_to_linecol(&app->buf, gb_length(&app->buf), &app->caret.line, &app->caret.col); // coarse; fix below
    // set caret at end of insertion (recompute precisely)
    size_t idx = editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
    (void)idx;
    GlobalUnlock(h);
    CloseClipboard();
}

// caret movement
static void caret_to_index(AppState *app, size_t idx){
    editor_index_to_linecol(&app->buf, idx, &app->caret.line, &app->caret.col);
}
static size_t caret_index(AppState *app){
    return editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
}
static void caret_move
