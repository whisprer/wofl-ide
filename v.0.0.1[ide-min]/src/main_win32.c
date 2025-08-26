// ultra-minimal single-window code editor / mini-IDE (Win32)
// build: see /ide-min/build.bat
#define WIN32_LEAN_AND_MEAN
#include "editor.h"
#include <commdlg.h>
#include <shellapi.h>
#include <wchar.h>

static AppState G;

// ========== small helpers ==========

static void app_set_title(HWND hwnd){
    wchar_t t[WOFL_MAX_PATH+64];
    swprintf(t, WOFL_MAX_PATH+64, L"wofl-ide — %ls%ls",
             G.file_name[0]?G.file_name:L"(untitled)",
             G.buf.dirty?L"*":L"");
    SetWindowTextW(hwnd, t);
}

static void app_set_file(const wchar_t *path){
    wcsncpy_s(G.file_path, WOFL_MAX_PATH, path, _TRUNCATE);
    // split dir/name
    const wchar_t *slash = wcsrchr(path, L'\\');
    if(!slash) slash = wcsrchr(path, L'/');
    if(slash){
        size_t dlen = (size_t)(slash - path);
        wcsncpy_s(G.file_dir, WOFL_MAX_PATH, path, dlen);
        G.file_dir[dlen] = 0;
        wcsncpy_s(G.file_name, WOFL_MAX_PATH, slash+1, _TRUNCATE);
    }else{
        G.file_dir[0]=0;
        wcsncpy_s(G.file_name, WOFL_MAX_PATH, path, _TRUNCATE);
    }
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
        EolMode eol; gb_free(&G.buf);
        gb_load_from_utf8_file(&G.buf, path, &eol); G.buf.eol_mode=eol;
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
    int usable_h = G.client_rc.bottom - (G.theme.line_h+2) - (G.out.visible?G.out.height_px:0);
    int lines_fit = usable_h > 0 ? (usable_h / G.theme.line_h) : 1;
    if(G.caret.line >= G.top_line + lines_fit) G.top_line = G.caret.line - lines_fit + 1;
    if(G.caret.col < G.left_col) G.left_col = G.caret.col;
    int cols_fit = (G.client_rc.right-8)/G.theme.ch_w;
    if(cols_fit<1) cols_fit=1;
    if(G.caret.col >= G.left_col + cols_fit) G.left_col = G.caret.col - cols_fit + 1;
}

static int line_length(int line){
    size_t s = editor_line_start_index(&G.buf, line);
    size_t e = editor_line_start_index(&G.buf, line+1);
    int cols=0;
    for(size_t i=s;i<e;i++){ if(gb_char_at(&G.buf,i)==L'\n') break; cols++; }
    return cols;
}

// selection helpers
static bool get_selection(size_t *a, size_t *b){
    if(!G.selecting) return false;
    size_t c = editor_linecol_to_index(&G.buf, G.caret.line, G.caret.col);
    size_t s = editor_linecol_to_index(&G.buf, G.sel_anchor.line, G.sel_anchor.col);
    if(c==s) return false;
    if(c < s){ *a=c; *b=s; } else { *a=s; *b=c; }
    return true;
}

static void clear_selection(){
    G.selecting=false;
    G.sel_anchor = G.caret;
}

// movement
static void move_left(bool sel){
    if(G.caret.col>0){
        G.caret.col--;
    }else if(G.caret.line>0){
        G.caret.line--;
        G.caret.col = line_length(G.caret.line);
    }
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}
static void move_right(bool sel){
    int cols = line_length(G.caret.line);
    if(G.caret.col<cols){
        G.caret.col++;
    } else {
        if(G.caret.line+1 < editor_total_lines(&G)) { G.caret.line++; G.caret.col=0; }
    }
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}
static void move_up(bool sel){
    if(G.caret.line>0) G.caret.line--;
    int cols = line_length(G.caret.line);
    if(G.caret.col>cols) G.caret.col=cols;
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}
static void move_down(bool sel){
    if(G.caret.line+1<editor_total_lines(&G)) G.caret.line++;
    int cols = line_length(G.caret.line);
    if(G.caret.col>cols) G.caret.col=cols;
    if(!sel){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true;
}

// editing primitives
static void insert_text(const wchar_t *s, int n){
    size_t pos = editor_linecol_to_index(&G.buf, G.caret.line, G.caret.col);
    if(n<=0) return;
    gb_move_gap(&G.buf, pos);
    gb_insert(&G.buf, s, (size_t)n);
    G.buf.dirty = true;
    G.need_recount=true;
    // advance caret by UTF-16 code units inserted (we only handle BMP here)
    for(int i=0;i<n;i++){
        if(s[i]==L'\n'){
            G.caret.line++; G.caret.col=0;
        }else{
            G.caret.col++;
        }
    }
}

static void delete_range(size_t a, size_t b){
    if(b<=a) return;
    gb_delete_range(&G.buf, a, b-a);
    G.buf.dirty=true;
    G.need_recount=true;
    editor_index_to_linecol(&G.buf, a, &G.caret.line, &G.caret.col);
}

static void backspace(){
    size_t a,b;
    if(get_selection(&a,&b)){ delete_range(a,b); G.selecting=false; return; }
    size_t pos = editor_linecol_to_index(&G.buf, G.caret.line, G.caret.col);
    if(pos==0) return;
    // delete previous char
    delete_range(pos-1, pos);
}

static void del_forward(){
    size_t a,b;
    if(get_selection(&a,&b)){ delete_range(a,b); G.selecting=false; return; }
    size_t pos = editor_linecol_to_index(&G.buf, G.caret.line, G.caret.col);
    size_t n = gb_length(&G.buf);
    if(pos>=n) return;
    delete_range(pos, pos+1);
}

// new line with simple auto-indent
static void insert_newline(){
    // collect leading whitespace from current line
    size_t ls = editor_line_start_index(&G.buf, G.caret.line);
    int indent=0; for(;;){
        wchar_t c = gb_char_at(&G.buf, ls + indent);
        if(c==L' ' || c==L'\t') indent++;
        else break;
    }
    wchar_t tmp[1024];
    int k=0; tmp[k++]=L'\n';
    for(int i=0;i<indent && k<1023;i++) tmp[k++]=L' ';
    insert_text(tmp, k);
}

static void insert_tab(){
    int tw = G.tab_w>0?G.tab_w:4;
    for(int i=0;i<tw;i++) gb_insert_ch(&G.buf, L' ');
    G.buf.dirty=true;
    G.caret.col += tw;
}

// clipboard
static void copy_selection_to_clipboard(){
    size_t a,b; if(!get_selection(&a,&b)) return;
    size_t len = b-a;
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (len+1)*sizeof(wchar_t));
    if(!h) return;
    wchar_t *p = (wchar_t*)GlobalLock(h);
    size_t j=0; for(size_t i=a;i<b;i++) p[j++] = gb_char_at(&G.buf,i);
    p[j]=0; GlobalUnlock(h);
    if(OpenClipboard(G.hwnd)){
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, h);
        CloseClipboard();
    }else{
        GlobalFree(h);
    }
}

static void paste_from_clipboard(){
    if(!OpenClipboard(G.hwnd)) return;
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if(!h){ CloseClipboard(); return; }
    wchar_t *p = (wchar_t*)GlobalLock(h);
    if(!p){ CloseClipboard(); return; }
    // normalize CRLF -> LF
    size_t n = wcslen(p);
    wchar_t *tmp = (wchar_t*)HeapAlloc(GetProcessHeap(),0,(n+1)*sizeof(wchar_t));
    size_t k=0;
    for(size_t i=0;i<n;i++){
        if(p[i]==L'\r'){ if(i+1<n && p[i+1]==L'\n'){ /* skip \r */ } else tmp[k++]=L'\n'; }
        else tmp[k++]=p[i];
    }
    tmp[k]=0;
    size_t a,b;
    if(get_selection(&a,&b)){ delete_range(a,b); G.selecting=false; }
    insert_text(tmp, (int)k);
    HeapFree(GetProcessHeap(),0,tmp);
    GlobalUnlock(h);
    CloseClipboard();
}

// run helper
static void do_run_current(){
    if(!G.file_path[0]){ save_file_as_dialog(); if(!G.file_path[0]) return; }
    if(G.buf.dirty) save_file();
    config_set_default_run_cmd(&G);
    config_try_load_run_cmd(&G); // overrides default if build.wofl exists
    run_spawn(&G, G.run_cmd);
}

// UI overlays
static void open_find(){
    G.mode = MODE_FIND;
    G.overlay_active = true;
    wcscpy_s(G.overlay_prompt, 128, L"/");
    G.overlay_text[0]=0; G.overlay_len=0; G.overlay_cursor=0;
}
static void open_goto(){
    G.mode = MODE_GOTO;
    G.overlay_active = true;
    wcscpy_s(G.overlay_prompt, 128, L":");
    G.overlay_text[0]=0; G.overlay_len=0; G.overlay_cursor=0;
}

// overlay editing
static void overlay_insert(wchar_t ch){
    if(G.overlay_len+1 >= WOFL_CMD_MAX) return;
    memmove(G.overlay_text + G.overlay_cursor + 1,
            G.overlay_text + G.overlay_cursor,
            (G.overlay_len - G.overlay_cursor)*sizeof(wchar_t));
    G.overlay_text[G.overlay_cursor] = ch;
    G.overlay_len++; G.overlay_cursor++;
    G.overlay_text[G.overlay_len]=0;
}
static void overlay_backspace(){
    if(G.overlay_cursor<=0) return;
    memmove(G.overlay_text + G.overlay_cursor - 1,
            G.overlay_text + G.overlay_cursor,
            (G.overlay_len - G.overlay_cursor)*sizeof(wchar_t));
    G.overlay_len--; G.overlay_cursor--;
    G.overlay_text[G.overlay_len]=0;
}

// ========== window proc ==========

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
    case WM_CREATE:{
        G.hwnd = hwnd;
        theme_default(&G.theme);
        HDC hdc = GetDC(hwnd);
        editor_layout_metrics(&G, hdc);
        ReleaseDC(hwnd, hdc);
        gb_init(&G.buf);
        G.tab_w = 4;
        G.mode = MODE_EDIT;
        G.top_line = G.left_col = 0;
        G.caret.line = G.caret.col = 0;
        G.need_recount = true;
        app_set_title(hwnd);
    } return 0;

    case WM_SIZE:{
        G.client_rc.right  = LOWORD(lParam);
        G.client_rc.bottom = HIWORD(lParam);
        InvalidateRect(hwnd,NULL,TRUE);
    } return 0;

    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        InvalidateRect(hwnd,NULL,FALSE);
        return 0;

    case WM_PAINT:{
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        if(!G.theme.hFont){ editor_layout_metrics(&G, hdc); }
        editor_paint(&G, hdc);
        EndPaint(hwnd, &ps);
    } return 0;

    case WM_MOUSEWHEEL:{
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int lines = delta / WHEEL_DELTA * 3;
        G.top_line -= lines;
        if(G.top_line<0) G.top_line=0;
        InvalidateRect(hwnd,NULL,FALSE);
    } return 0;

    case WM_LBUTTONDOWN:{
        SetCapture(hwnd);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        int line = G.top_line + y / G.theme.line_h;
        int col  = G.left_col + (x-4) / G.theme.ch_w;
        if(line<0) line=0;
        int total = editor_total_lines(&G);
        if(line>=total) line=total-1;
        int len = line_length(line);
        if(col<0) col=0; if(col>len) col=len;
        G.caret.line=line; G.caret.col=col;
        if(GetKeyState(VK_SHIFT)&0x8000){ G.selecting=true; } else { G.selecting=false; G.sel_anchor=G.caret; }
        InvalidateRect(hwnd,NULL,FALSE);
    } return 0;

    case WM_MOUSEMOVE:{
        if(GetCapture()==hwnd){
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            int line = G.top_line + y / G.theme.line_h;
            int col  = G.left_col + (x-4) / G.theme.ch_w;
            if(line<0) line=0;
            int total = editor_total_lines(&G);
            if(line>=total) line=total-1;
            int len = line_length(line);
            if(col<0) col=0; if(col>len) col=len;
            G.caret.line=line; G.caret.col=col;
            G.selecting=true;
            ensure_caret_visible();
            InvalidateRect(hwnd,NULL,FALSE);
        }
    } return 0;

    case WM_LBUTTONUP:
        if(GetCapture()==hwnd) ReleaseCapture();
        return 0;

    case WM_CHAR:{
        wchar_t ch = (wchar_t)wParam;

        if(G.overlay_active){
            if(ch==L'\r' || ch==L'\n'){
                // confirm
                if(G.mode==MODE_FIND){
                    if(G.overlay_len>0){
                        G.find.active=true;
                        wcsncpy_s(G.find.text, WOFL_FIND_MAX, G.overlay_text, _TRUNCATE);
                        G.find.len = (int)wcslen(G.find.text);
                        find_next(&G, G.find.text, true, true, true);
                    }
                }else if(G.mode==MODE_GOTO){
                    int tgt = _wtoi(G.overlay_text);
                    if(tgt>0){
                        G.caret.line = (tgt-1);
                        int total = editor_total_lines(&G);
                        if(G.caret.line<0) G.caret.line=0;
                        if(G.caret.line>=total) G.caret.line=total-1;
                        int len = line_length(G.caret.line);
                        if(G.caret.col>len) G.caret.col=len;
                        ensure_caret_visible();
                    }
                }else if(G.mode==MODE_PALETTE){
                    palette_confirm(&G);
                }
                G.overlay_active=false; G.mode=MODE_EDIT;
                InvalidateRect(hwnd,NULL,FALSE);
                return 0;
            }else if(ch==L'\b'){
                if(G.mode==MODE_PALETTE) palette_backspace(&G);
                else overlay_backspace();
                InvalidateRect(hwnd,NULL,FALSE);
                return 0;
            }else if(ch==27){
                G.overlay_active=false; G.mode=MODE_EDIT;
                InvalidateRect(hwnd,NULL,FALSE);
                return 0;
            }else if(ch>=32){
                if(G.mode==MODE_PALETTE) palette_handle_char(&G, ch);
                else overlay_insert(ch);
                InvalidateRect(hwnd,NULL,FALSE);
                return 0;
            }
            return 0;
        }

        // normal edit mode
        if(ch==27){ // ESC
            if(G.selecting){ clear_selection(); InvalidateRect(hwnd,NULL,FALSE); }
            return 0;
        }
        if(ch==L'\r' || ch==L'\n'){ insert_newline(); ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0; }
        if(ch==L'\t'){ insert_tab(); ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0; }
        if(ch>=32){
            size_t a,b;
            if(get_selection(&a,&b)){ delete_range(a,b); G.selecting=false; }
            wchar_t s[2]={ch,0};
            insert_text(s,1);
            ensure_caret_visible();
            InvalidateRect(hwnd,NULL,FALSE);
            return 0;
        }
    } return 0;

    case WM_KEYDOWN:{
        bool ctrl = (GetKeyState(VK_CONTROL)&0x8000)!=0;
        bool shift= (GetKeyState(VK_SHIFT)&0x8000)!=0;

        if(G.overlay_active){
            if(wParam==VK_LEFT){ if(G.overlay_cursor>0){ G.overlay_cursor--; InvalidateRect(hwnd,NULL,FALSE);} return 0; }
            if(wParam==VK_RIGHT){ if(G.overlay_cursor<G.overlay_len){ G.overlay_cursor++; InvalidateRect(hwnd,NULL,FALSE);} return 0; }
            if(wParam==VK_HOME){ G.overlay_cursor=0; InvalidateRect(hwnd,NULL,FALSE); return 0; }
            if(wParam==VK_END){ G.overlay_cursor=G.overlay_len; InvalidateRect(hwnd,NULL,FALSE); return 0; }
        }

        // global shortcuts
        if(ctrl){
            switch(wParam){
            case 'O': open_file_dialog(); return 0;
            case 'S': save_file(); return 0;
            case 'P': palette_open(&G); return 0;
            case 'F': open_find(); return 0;
            case 'G': open_goto(); return 0;
            case 'A': // select all
                G.sel_anchor.line=0; G.sel_anchor.col=0;
                G.caret.line = editor_total_lines(&G)-1;
                G.caret.col  = line_length(G.caret.line);
                G.selecting=true;
                InvalidateRect(hwnd,NULL,FALSE); return 0;
            case 'C': copy_selection_to_clipboard(); return 0;
            case 'X': copy_selection_to_clipboard(); { size_t a,b; if(get_selection(&a,&b)) delete_range(a,b); G.selecting=false; InvalidateRect(hwnd,NULL,FALSE);} return 0;
            case 'V': paste_from_clipboard(); ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
            }
        }

        switch(wParam){
        case VK_F5: do_run_current(); return 0;
        case VK_F6: G.out.visible = !G.out.visible; InvalidateRect(hwnd,NULL,TRUE); return 0;
        case VK_LEFT:  move_left(shift);  ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_RIGHT: move_right(shift); ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_UP:    move_up(shift);    ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_DOWN:  move_down(shift);  ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_HOME:  G.caret.col=0; if(!shift){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true; ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_END:   G.caret.col = line_length(G.caret.line); if(!shift){ G.sel_anchor=G.caret; G.selecting=false; } else G.selecting=true; ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_PRIOR: G.caret.line -= ((G.client_rc.bottom - (G.theme.line_h+2)) / G.theme.line_h); if(G.caret.line<0) G.caret.line=0; if(!shift){G.sel_anchor=G.caret; G.selecting=false;} else G.selecting=true; ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_NEXT:  G.caret.line += ((G.client_rc.bottom - (G.theme.line_h+2)) / G.theme.line_h); {int tot=editor_total_lines(&G); if(G.caret.line>=tot) G.caret.line=tot-1;} if(!shift){G.sel_anchor=G.caret; G.selecting=false;} else G.selecting=true; ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_BACK:  backspace(); ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        case VK_DELETE: del_forward(); ensure_caret_visible(); InvalidateRect(hwnd,NULL,FALSE); return 0;
        }
    } return 0;

    case WM_COMMAND:{
        switch(LOWORD(wParam)){
        case 1: open_file_dialog(); break;
        case 2: save_file(); break;
        case 3: save_file_as_dialog(); break;
        case 4: do_run_current(); break;
        case 5: G.out.visible=!G.out.visible; InvalidateRect(hwnd,NULL,TRUE); break;
        case 6: open_find(); InvalidateRect(hwnd,NULL,FALSE); break;
        case 7: open_goto(); InvalidateRect(hwnd,NULL,FALSE); break;
        default: break;
        }
        return 0;
    }

    case WM_CLOSE:{
        // crude: no “save changes?” dialog in P0
        run_kill(&G);
        DestroyWindow(hwnd);
    } return 0;

    case WM_DESTROY:{
        gb_free(&G.buf);
        if(G.out.buf.data) gb_free(&G.out.buf);
        PostQuitMessage(0);
    } return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ========== entry point ==========

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpCmd, int nShow){
    (void)hPrev; (void)lpCmd;
    SetProcessDPIAware();

    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_IBEAM);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = L"WoflIdeWindow";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"wofl-ide",
                                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
                                NULL, NULL, hInst, NULL);
    if(!hwnd) return 0;

    // open file by drag-drop / shell open path if provided
    int argc=0; LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if(argv && argc>1){
        EolMode eol; gb_load_from_utf8_file(&G.buf, argv[1], &eol); G.buf.eol_mode=eol;
        app_set_file(argv[1]);
        config_set_default_run_cmd(&G);
        config_try_load_run_cmd(&G);
        InvalidateRect(hwnd,NULL,TRUE);
    }
    if(argv) LocalFree(argv);

    MSG msg;
    while(GetMessageW(&msg,NULL,0,0)>0){
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
