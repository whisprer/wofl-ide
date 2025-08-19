#define WIN32_LEAN_AND_MEAN
#include "editor.h"
#include <commdlg.h>
#include <wchar.h>

static AppState G;

static void app_set_title(HWND hwnd){
    wchar_t t[WOFL_MAX_PATH+64];
    swprintf(t, WOFL_MAX_PATH+64, L"wofl-ide — %ls%ls", G.file_name[0]?G.file_name:L"(untitled)", G.buf.dirty?L"*":L"");
    SetWindowTextW(hwnd, t);
}

static void app_set_file(const wchar_t *path){
    wcsncpy_s(G.file_path, WOFL_MAX_PATH, path, _TRUNCATE);
    // split dir/name
    const wchar_t *slash = wcsrchr(path, L'\\');
    if(!slash) slash = wcsrchr(path, L'/');
    if(slash){ wcsncpy_s(G.file_dir, WOFL_MAX_PATH, G.file_path, (int)(slash-G.file_path)); G.file_dir[slash-G.file_path]=0; wcsncpy_s(G.file_name, WOFL_MAX_PATH, slash+1, _TRUNCATE); }
    else { G.file_dir[0]=0; wcsncpy_s(G.file_name, WOFL_MAX_PATH, path, _TRUNCATE); }
    G.lang = lang_from_ext(path);
    G.need_recount=true;
    app_set_title(G.hwnd);
}

static void open_file_dialog(){
    wchar_t path[WOFL_MAX_PATH]={0};
    OPENFILENAMEW ofn={0}; ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=G.hwnd;
    ofn.lpstrFile = path; ofn.nMaxFile = WOFL_MAX_PATH;
    ofn.lpstrFilter=L"All\0*.*\0";
    ofn.lpstrTitle=L"Open File";
    ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST;
    if(GetOpenFileNameW(&ofn)){
        EolMode eol; gb_free(&G.buf); gb_load_from_utf8_file(&G.buf, path, &eol); G.buf.eol_mode=eol;
        G.caret.line=G.caret.col=0; G.top_line=G.left_col=0; G.selecting=false; G.overwrite=false;
        app_set_file(path);
        config_set_default_run_cmd(&G);
        config_try_load_run_cmd(&G);
        InvalidateRect(G.hwnd,NULL,TRUE);
    }
}

static void save_file_as_dialog(){
    wchar_t path[WOFL_MAX_PATH]={0};
    if(G.file_path[0]) wcsncpy_s(path, WOFL_MAX_PATH, G.file_path, _TRUNCATE);
    OPENFILENAMEW ofn={0}; ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=G.hwnd;
    ofn.lpstrFile = path; ofn.nMaxFile = WOFL_MAX_PATH;
    ofn.lpstrFilter=L"All\0*.*\0";
    ofn.lpstrTitle=L"Save File As";
    ofn.Flags = OFN_EXPLORER|OFN_OVERWRITEPROMPT;
    if(GetSaveFileNameW(&ofn)){
        if(gb_save_to_utf8_file(&G.buf, path, G.buf.eol_mode)){
            app_set_file(path);
        }
    }
}

static void save_file(){
    if(!G.file_path[0]){ save_file_as_dialog(); return; }
    if(gb_save_to_utf8_file(&G.buf, G.file_path, G.buf.eol_mode)){
        app_set_title(G.hwnd);
    }
}

static void ensure_caret_visible(){
    if(G.caret.line < G.top_line) G.top_line = G.caret.line;
    int lines_fit = (G.client_rc.bottom - (G.theme.line_h+2) - (G.out.visible?G.out.height_px:0)) / G.theme.line_h;
    if(G.caret.line >= G.top_line + lines_fit) G.top_line = G.caret.line - lines_fit + 1;
    if(G.caret.col < G.left_col) G.left_col = G.caret.col;
    int cols_fit = (G.client_rc.right-8)/G.theme.ch_w;
    if(G.caret.col >= G.left_col + cols_fit) G.left_col = G.caret.col - cols_fit + 1;
}

static void move_left(bool sel){ if(G.caret.col>0) G.caret.col--; else if(G.caret.line>0){ G.caret.line--; // move to prev line end
        size_t s = editor_line_start_index(&G.buf, G.caret.line);
        size_t e = editor_line_start_index(&G.buf, G.caret.line+1);
        int cols=0; for(size_t i=s;i<e;i++){ if(gb_char_at(&G.buf,i)==L'\n') break; cols++; } G.caret.col=cols;
    }
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}
static void move_right(bool sel){
    // compute line end
    size_t s = editor_line_start_index(&G.buf, G.caret.line);
    size_t e = editor_line_start_index(&G.buf, G.caret.line+1);
    int cols=0; for(size_t i=s;i<e;i++){ if(gb_char_at(&G.buf,i)==L'\n') break; cols++; }
    if(G.caret.col<cols) G.caret.col++; else { if(G.caret.line+1 < editor_total_lines(&G)) { G.caret.line++; G.caret.col=0; } }
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}
static void move_up(bool sel){ if(G.caret.line>0) G.caret.line--; // clamp col
    size_t s=editor_line_start_index(&G.buf,G.caret.line);
    size_t e=editor_line_start_index(&G.buf,G.caret.line+1);
    int cols=0; for(size_t i=s;i<e;i++){ if(gb_char_at(&G.buf,i)==L'\n') break; cols++; }
    if(G.caret.col>cols) G.caret.col=cols;
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}
static void move_down(bool sel){ if(G.caret.line+1<editor_total_lines(&G)) G.caret.line++; // clamp
    size_t s=editor_line_start_index(&G.buf,G.caret.line);
    size_t e=editor_line_start_index(&G.buf,G.caret.line+1);
    int cols=0; for(size_t i=s;i<e;i++){ if(gb_char_at(&G.buf,i)==L'\n') break; cols++; }
    if(G.caret.col>cols) G.caret.col=cols;
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false
