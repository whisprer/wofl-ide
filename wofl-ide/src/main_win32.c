// ==================== main_win32.c ====================
// Main window and application logic

#include "editor.h"
#include <commdlg.h>
#include <shellapi.h>
#include <wchar.h>

// Global application state
static AppState g_app;

// ===== Helper Functions =====

/**
 * Update window title with file info
 */
static void update_window_title(HWND hwnd) {
    wchar_t title[WOFL_MAX_PATH + 64];
    swprintf_s(title, WOFL_MAX_PATH + 64, L"WOFL IDE — %ls%ls",
               g_app.file_name[0] ? g_app.file_name : L"(untitled)",
               g_app.buf.dirty ? L" •" : L"");
    SetWindowTextW(hwnd, title);
}

/**
 * Set current file and update related state
 */
static void set_current_file(const wchar_t *path) {
    wcscpy_s(g_app.file_path, WOFL_MAX_PATH, path);
    
    // Extract directory and filename
    const wchar_t *last_sep = wcsrchr(path, L'\\');
    if (!last_sep) last_sep = wcsrchr(path, L'/');
    
    if (last_sep) {
        size_t dir_len = last_sep - path;
        wcsncpy_s(g_app.file_dir, WOFL_MAX_PATH, path, dir_len);
        g_app.file_dir[dir_len] = L'\0';
        wcscpy_s(g_app.file_name, WOFL_MAX_PATH, last_sep + 1);
    } else {
        g_app.file_dir[0] = L'\0';
        wcscpy_s(g_app.file_name, WOFL_MAX_PATH, path);
    }
    
    // Detect language
    g_app.lang = syntax_detect_language(path);
    g_app.need_recount = true;
    update_window_title(g_app.hwnd);
}

/**
 * Open file dialog
 */
static void open_file_dialog(void) {
    wchar_t path[WOFL_MAX_PATH] = {0};
    
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_app.hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = WOFL_MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0"
                     L"C/C++ Files\0*.c;*.cpp;*.h;*.hpp\0"
                     L"Python Files\0*.py\0"
                     L"JavaScript Files\0*.js;*.ts\0";
    ofn.lpstrTitle = L"Open File";
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameW(&ofn)) {
        EolMode eol;
        gb_free(&g_app.buf);
        
        if (gb_load_from_file(&g_app.buf, path, &eol)) {
            g_app.buf.eol_mode = eol;
            g_app.caret.line = 0;
            g_app.caret.col = 0;
            g_app.top_line = 0;
            g_app.left_col = 0;
            g_app.selecting = false;
            g_app.overwrite_mode = false;
            
            set_current_file(path);
            config_set_default_run_cmd(&g_app);
            config_load_run_cmd(&g_app);
            
            InvalidateRect(g_app.hwnd, NULL, TRUE);
        }
    }
}

/**
 * Save file as dialog
 */
static void save_file_as_dialog(void) {
    wchar_t path[WOFL_MAX_PATH] = {0};
    if (g_app.file_path[0]) {
        wcscpy_s(path, WOFL_MAX_PATH, g_app.file_path);
    }
    
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_app.hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = WOFL_MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.lpstrTitle = L"Save File As";
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameW(&ofn)) {
        if (gb_save_to_file(&g_app.buf, path, g_app.buf.eol_mode)) {
            set_current_file(path);
        }
    }
}

/**
 * Save current file
 */
static void save_file(void) {
    if (!g_app.file_path[0]) {
        save_file_as_dialog();
        return;
    }
    
    if (gb_save_to_file(&g_app.buf, g_app.file_path, g_app.buf.eol_mode)) {
        update_window_title(g_app.hwnd);
    }
}

/**
 * Ensure caret is visible in viewport
 */
static void ensure_caret_visible(void) {
    // Vertical scrolling
    if (g_app.caret.line < g_app.top_line) {
        g_app.top_line = g_app.caret.line;
    }
    
    int usable_height = g_app.client_rc.bottom - (g_app.theme.line_h + 2);
    if (g_app.out.visible) {
        usable_height -= g_app.out.height_px;
    }
    
    int lines_fit = max_int(usable_height / g_app.theme.line_h, 1);
    
    if (g_app.caret.line >= g_app.top_line + lines_fit) {
        g_app.top_line = g_app.caret.line - lines_fit + 1;
    }
    
    // Horizontal scrolling
    if (g_app.caret.col < g_app.left_col) {
        g_app.left_col = g_app.caret.col;
    }
    
    int cols_fit = max_int((g_app.client_rc.right - 8) / g_app.theme.ch_w, 1);
    
    if (g_app.caret.col >= g_app.left_col + cols_fit) {
        g_app.left_col = g_app.caret.col - cols_fit + 1;
    }
}

/**
 * Get line length
 */
static int get_line_length(int line) {
    size_t start = editor_line_start_index(&g_app.buf, line);
    size_t end = editor_line_start_index(&g_app.buf, line + 1);
    int length = 0;
    
    for (size_t i = start; i < end; i++) {
        if (gb_char_at(&g_app.buf, i) == L'\n') break;
        length++;
    }
    
    return length;
}

// ===== Selection Management =====

/**
 * Get current selection range
 */
static bool get_selection(size_t *start, size_t *end) {
    if (!g_app.selecting) return false;
    
    size_t caret_idx = editor_linecol_to_index(&g_app.buf,
                                                g_app.caret.line, g_app.caret.col);
    size_t anchor_idx = editor_linecol_to_index(&g_app.buf,
                                                 g_app.sel_anchor.line, g_app.sel_anchor.col);
    
    if (caret_idx == anchor_idx) return false;
    
    *start = min_size(caret_idx, anchor_idx);
    *end = max_size(caret_idx, anchor_idx);
    return true;
}

/**
 * Clear selection
 */
static void clear_selection(void) {
    g_app.selecting = false;
    g_app.sel_anchor = g_app.caret;
}

// ===== Movement Functions =====

static void move_left(bool select) {
    if (g_app.caret.col > 0) {
        g_app.caret.col--;
    } else if (g_app.caret.line > 0) {
        g_app.caret.line--;
        g_app.caret.col = get_line_length(g_app.caret.line);
    }
    
    if (!select) {
        g_app.sel_anchor = g_app.caret;
        g_app.selecting = false;
    } else {
        g_app.selecting = true;
    }
}

static void move_right(bool select) {
    int line_len = get_line_length(g_app.caret.line);
    
    if (g_app.caret.col < line_len) {
        g_app.caret.col++;
    } else if (g_app.caret.line + 1 < editor_total_lines(&g_app)) {
        g_app.caret.line++;
        g_app.caret.col = 0;
    }
    
    if (!select) {
        g_app.sel_anchor = g_app.caret;
        g_app.selecting = false;
    } else {
        g_app.selecting = true;
    }
}

static void move_up(bool select) {
    if (g_app.caret.line > 0) {
        g_app.caret.line--;
        int line_len = get_line_length(g_app.caret.line);
        if (g_app.caret.col > line_len) {
            g_app.caret.col = line_len;
        }
    }
    
    if (!select) {
        g_app.sel_anchor = g_app.caret;
        g_app.selecting = false;
    } else {
        g_app.selecting = true;
    }
}

static void move_down(bool select) {
    if (g_app.caret.line + 1 < editor_total_lines(&g_app)) {
        g_app.caret.line++;
        int line_len = get_line_length(g_app.caret.line);
        if (g_app.caret.col > line_len) {
            g_app.caret.col = line_len;
        }
    }
    
    if (!select) {
        g_app.sel_anchor = g_app.caret;
        g_app.selecting = false;
    } else {
        g_app.selecting = true;
    }
}

// ===== Editing Functions =====

/**
 * Insert text at current position
 */
static void insert_text(const wchar_t *text, int len) {
    if (len <= 0) return;
    
    size_t pos = editor_linecol_to_index(&g_app.buf, g_app.caret.line, g_app.caret.col);
    gb_move_gap(&g_app.buf, pos);
    gb_insert(&g_app.buf, text, len);
    
    g_app.buf.dirty = true;
    g_app.need_recount = true;
    
    // Update caret position
    for (int i = 0; i < len; i++) {
        if (text[i] == L'\n') {
            g_app.caret.line++;
            g_app.caret.col = 0;
        } else {
            g_app.caret.col++;
        }
    }
}

/**
 * Delete range of text
 */
static void delete_range(size_t start, size_t end) {
    if (end <= start) return;
    
    gb_delete_range(&g_app.buf, start, end - start);
    g_app.buf.dirty = true;
    g_app.need_recount = true;
    
    editor_index_to_linecol(&g_app.buf, start, &g_app.caret.line, &g_app.caret.col);
}

/**
 * Handle backspace
 */
static void backspace(void) {
    size_t start, end;
    
    if (get_selection(&start, &end)) {
        delete_range(start, end);
        g_app.selecting = false;
        return;
    }
    
    size_t pos = editor_linecol_to_index(&g_app.buf, g_app.caret.line, g_app.caret.col);
    if (pos > 0) {
        delete_range(pos - 1, pos);
    }
}

/**
 * Handle delete key
 */
static void delete_forward(void) {
    size_t start, end;
    
    if (get_selection(&start, &end)) {
        delete_range(start, end);
        g_app.selecting = false;
        return;
    }
    
    size_t pos = editor_linecol_to_index(&g_app.buf, g_app.caret.line, g_app.caret.col);
    size_t len = gb_length(&g_app.buf);
    
    if (pos < len) {
        delete_range(pos, pos + 1);
    }
}

/**
 * Insert newline with auto-indent
 */
static void insert_newline(void) {
    // Get indentation from current line
    size_t line_start = editor_line_start_index(&g_app.buf, g_app.caret.line);
    int indent = 0;
    
    while (true) {
        wchar_t ch = gb_char_at(&g_app.buf, line_start + indent);
        if (ch == L' ' || ch == L'\t') {
            indent++;
        } else {
            break;
        }
    }
    
    // Build newline + indentation
    wchar_t indent_text[1024];
    int k = 0;
    indent_text[k++] = L'\n';
    
    for (int i = 0; i < indent && k < 1023; i++) {
        indent_text[k++] = L' ';
    }
    
    insert_text(indent_text, k);
}

/**
 * Insert tab
 */
static void insert_tab(void) {
    int tab_width = g_app.tab_width > 0 ? g_app.tab_width : WOFL_DEFAULT_TAB;
    wchar_t spaces[32];
    
    for (int i = 0; i < tab_width && i < 31; i++) {
        spaces[i] = L' ';
    }
    spaces[tab_width] = L'\0';
    
    insert_text(spaces, tab_width);
}

// ===== Clipboard Operations =====

/**
 * Copy selection to clipboard
 */
static void copy_to_clipboard(void) {
    size_t start, end;
    if (!get_selection(&start, &end)) return;
    
    size_t len = end - start;
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
    if (!mem) return;
    
    wchar_t *buffer = (wchar_t*)GlobalLock(mem);
    for (size_t i = 0; i < len; i++) {
        buffer[i] = gb_char_at(&g_app.buf, start + i);
    }
    buffer[len] = L'\0';
    GlobalUnlock(mem
);
    
    if (OpenClipboard(g_app.hwnd)) {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, mem);
        CloseClipboard();
    } else {
        GlobalFree(mem);
    }
}

/**
 * Paste from clipboard
 */
static void paste_from_clipboard(void) {
    if (!OpenClipboard(g_app.hwnd)) return;
    
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (!data) {
        CloseClipboard();
        return;
    }
    
    wchar_t *text = (wchar_t*)GlobalLock(data);
    if (!text) {
        CloseClipboard();
        return;
    }
    
    // Normalize CRLF to LF
    size_t len = wcslen(text);
    wchar_t *normalized = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, 
                                               (len + 1) * sizeof(wchar_t));
    size_t write_pos = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (text[i] == L'\r') {
            if (i + 1 < len && text[i + 1] == L'\n') {
                // Skip \r in CRLF
                continue;
            } else {
                // Convert lone \r to \n
                normalized[write_pos++] = L'\n';
            }
        } else {
            normalized[write_pos++] = text[i];
        }
    }
    normalized[write_pos] = L'\0';
    
    // Delete selection if exists
    size_t start, end;
    if (get_selection(&start, &end)) {
        delete_range(start, end);
        g_app.selecting = false;
    }
    
    // Insert text
    insert_text(normalized, (int)write_pos);
    
    HeapFree(GetProcessHeap(), 0, normalized);
    GlobalUnlock(data);
    CloseClipboard();
}

// ===== UI Operations =====

/**
 * Run current file
 */
static void run_current_file(void) {
    // Save file if needed
    if (!g_app.file_path[0]) {
        save_file_as_dialog();
        if (!g_app.file_path[0]) return;
    }
    
    if (g_app.buf.dirty) {
        save_file();
    }
    
    // Setup run command
    config_set_default_run_cmd(&g_app);
    config_load_run_cmd(&g_app);
    
    // Execute
    run_spawn(&g_app, g_app.run_cmd);
}

/**
 * Open find dialog
 */
static void open_find_dialog(void) {
    g_app.mode = MODE_FIND;
    g_app.overlay_active = true;
    wcscpy_s(g_app.overlay_prompt, 128, L"Find:");
    g_app.overlay_text[0] = L'\0';
    g_app.overlay_len = 0;
    g_app.overlay_cursor = 0;
}

/**
 * Open goto line dialog
 */
static void open_goto_dialog(void) {
    g_app.mode = MODE_GOTO;
    g_app.overlay_active = true;
    wcscpy_s(g_app.overlay_prompt, 128, L"Line:");
    g_app.overlay_text[0] = L'\0';
    g_app.overlay_len = 0;
    g_app.overlay_cursor = 0;
}

/**
 * Handle overlay input
 */
static void overlay_insert_char(wchar_t ch) {
    if (g_app.overlay_len >= WOFL_CMD_MAX - 1) return;
    
    memmove(g_app.overlay_text + g_app.overlay_cursor + 1,
            g_app.overlay_text + g_app.overlay_cursor,
            (g_app.overlay_len - g_app.overlay_cursor) * sizeof(wchar_t));
    
    g_app.overlay_text[g_app.overlay_cursor] = ch;
    g_app.overlay_len++;
    g_app.overlay_cursor++;
    g_app.overlay_text[g_app.overlay_len] = L'\0';
}

/**
 * Handle overlay backspace
 */
static void overlay_backspace(void) {
    if (g_app.overlay_cursor <= 0) return;
    
    memmove(g_app.overlay_text + g_app.overlay_cursor - 1,
            g_app.overlay_text + g_app.overlay_cursor,
            (g_app.overlay_len - g_app.overlay_cursor) * sizeof(wchar_t));
    
    g_app.overlay_len--;
    g_app.overlay_cursor--;
    g_app.overlay_text[g_app.overlay_len] = L'\0';
}

/**
 * Confirm overlay action
 */
static void overlay_confirm(void) {
    switch (g_app.mode) {
        case MODE_FIND:
            if (g_app.overlay_len > 0) {
                g_app.find.active = true;
                wcscpy_s(g_app.find.text, WOFL_FIND_MAX, g_app.overlay_text);
                g_app.find.len = (int)wcslen(g_app.find.text);
                g_app.find.case_insensitive = true;
                g_app.find.wrap_around = true;
                g_app.find.last_dir_down = true;
                
                find_next(&g_app, g_app.find.text, 
                         g_app.find.case_insensitive,
                         g_app.find.wrap_around, true);
            }
            break;
            
        case MODE_GOTO: {
            int line_num = _wtoi(g_app.overlay_text);
            if (line_num > 0) {
                g_app.caret.line = line_num - 1;
                int total = editor_total_lines(&g_app);
                if (g_app.caret.line >= total) {
                    g_app.caret.line = total - 1;
                }
                if (g_app.caret.line < 0) {
                    g_app.caret.line = 0;
                }
                
                int line_len = get_line_length(g_app.caret.line);
                if (g_app.caret.col > line_len) {
                    g_app.caret.col = line_len;
                }
                
                ensure_caret_visible();
            }
            break;
        }
            
        case MODE_PALETTE:
            palette_confirm(&g_app);
            break;
    }
    
    g_app.overlay_active = false;
    g_app.mode = MODE_EDIT;
    InvalidateRect(g_app.hwnd, NULL, FALSE);
}

// ===== Window Procedure =====

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_app.hwnd = hwnd;

            // PluginManager pm;
            // plugin_manager_init(&pm);

            // Initialize theme
            theme_init_default(&g_app.theme);
            HDC hdc = GetDC(hwnd);
            editor_update_metrics(&g_app, hdc);
            ReleaseDC(hwnd, hdc);
            
            // ThemeConfig theme;
            // theme_load_gemini-dark(&theme);  // Or user's preference
            // theme_apply(&app, &theme);

            // Initialize buffer
            gb_init(&g_app.buf);
            
            // Initialize state
            g_app.tab_width = WOFL_DEFAULT_TAB;
            g_app.mode = MODE_EDIT;
            g_app.top_line = 0;
            g_app.left_col = 0;
            g_app.caret.line = 0;
            g_app.caret.col = 0;
            g_app.need_recount = true;
            
            // Initialize syntax system
            // syntax_init();
            
            update_window_title(hwnd);
            return 0;
        }
        
        case WM_SIZE: {
            g_app.client_rc.right = LOWORD(lParam);
            g_app.client_rc.bottom = HIWORD(lParam);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (!g_app.theme.hFont) {
                editor_update_metrics(&g_app, hdc);
            }
            
            if (g_app.need_recount) {
                editor_recount_lines(&g_app);
            }
            
            editor_paint(&g_app, hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int lines = (delta / WHEEL_DELTA) * 3;
            
            g_app.top_line -= lines;
            if (g_app.top_line < 0) {
                g_app.top_line = 0;
            }
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            SetCapture(hwnd);
            
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            
            int line = g_app.top_line + y / g_app.theme.line_h;
            int col = g_app.left_col + (x - 4) / g_app.theme.ch_w;
            
            // Clamp to valid range
            line = max_int(0, min_int(line, editor_total_lines(&g_app) - 1));
            col = max_int(0, min_int(col, get_line_length(line)));
            
            g_app.caret.line = line;
            g_app.caret.col = col;
            
            if (GetKeyState(VK_SHIFT) & 0x8000) {
                g_app.selecting = true;
            } else {
                g_app.selecting = false;
                g_app.sel_anchor = g_app.caret;
            }
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            if (GetCapture() == hwnd) {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                int line = g_app.top_line + y / g_app.theme.line_h;
                int col = g_app.left_col + (x - 4) / g_app.theme.ch_w;
                
                // Clamp to valid range
                line = max_int(0, min_int(line, editor_total_lines(&g_app) - 1));
                col = max_int(0, min_int(col, get_line_length(line)));
                
                g_app.caret.line = line;
                g_app.caret.col = col;
                g_app.selecting = true;
                
                ensure_caret_visible();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_LBUTTONUP:
            if (GetCapture() == hwnd) {
                ReleaseCapture();
            }
            return 0;
        
        case WM_CHAR: {
            wchar_t ch = (wchar_t)wParam;
            
            // Handle overlay input
            if (g_app.overlay_active) {
                if (ch == L'\r' || ch == L'\n') {
                    overlay_confirm();
                } else if (ch == L'\b') {
                    if (g_app.mode == MODE_PALETTE) {
                        palette_backspace(&g_app);
                    } else {
                        overlay_backspace();
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (ch == 27) {  // ESC
                    if (g_app.mode == MODE_PALETTE) {
                        palette_cancel(&g_app);
                    } else {
                        g_app.overlay_active = false;
                        g_app.mode = MODE_EDIT;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                } else if (ch >= 32) {
                    if (g_app.mode == MODE_PALETTE) {
                        palette_handle_char(&g_app, ch);
                    } else {
                        overlay_insert_char(ch);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            
            // Normal editing
            if (ch == 27) {  // ESC
                if (g_app.selecting) {
                    clear_selection();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (ch == L'\r' || ch == L'\n') {
                insert_newline();
                ensure_caret_visible();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (ch == L'\t') {
                insert_tab();
                ensure_caret_visible();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (ch >= 32) {
                // Delete selection if exists
                size_t start, end;
                if (get_selection(&start, &end)) {
                    delete_range(start, end);
                    g_app.selecting = false;
                }
                
                wchar_t text[2] = {ch, L'\0'};
                insert_text(text, 1);
                ensure_caret_visible();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            
            // Handle overlay navigation
            if (g_app.overlay_active) {
                switch (wParam) {
                    case VK_LEFT:
                        if (g_app.overlay_cursor > 0) {
                            g_app.overlay_cursor--;
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                        return 0;
                    case VK_RIGHT:
                        if (g_app.overlay_cursor < g_app.overlay_len) {
                            g_app.overlay_cursor++;
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                        return 0;
                    case VK_HOME:
                        g_app.overlay_cursor = 0;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    case VK_END:
                        g_app.overlay_cursor = g_app.overlay_len;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                }
            }
            
            // Global shortcuts
            if (ctrl) {
                switch (wParam) {
                    case 'O':
                        open_file_dialog();
                        return 0;
                    case 'S':
                        save_file();
                        return 0;
                    case 'P':
                        palette_open(&g_app);
                        return 0;
                    case 'F':
                        open_find_dialog();
                        return 0;
                    case 'G':
                        open_goto_dialog();
                        return 0;
                    case 'A':  // Select all
                        g_app.sel_anchor.line = 0;
                        g_app.sel_anchor.col = 0;
                        g_app.caret.line = editor_total_lines(&g_app) - 1;
                        g_app.caret.col = get_line_length(g_app.caret.line);
                        g_app.selecting = true;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    case 'C':
                        copy_to_clipboard();
                        return 0;
                    case 'X':
                        copy_to_clipboard();
                        {
                            size_t start, end;
                            if (get_selection(&start, &end)) {
                                delete_range(start, end);
                                g_app.selecting = false;
                                InvalidateRect(hwnd, NULL, FALSE);
                            }
                        }
                        return 0;
                    case 'V':
                        paste_from_clipboard();
                        ensure_caret_visible();
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                }
            }

            theme_default(&g_app.theme);

            // Navigation and editing keys
            switch (wParam) {
                case VK_F3:  // Find next
                    if (g_app.find.active) {
                        find_next(&g_app, g_app.find.text,
                                 g_app.find.case_insensitive,
                                 g_app.find.wrap_around,
                                 !shift);
                        ensure_caret_visible();
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    return 0;
                    
                case VK_F5:
                    run_current_file();
                    return 0;
                    
                case VK_F6:
                    g_app.out.visible = !g_app.out.visible;
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                    
                case VK_LEFT:
                    move_left(shift);
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_RIGHT:
                    move_right(shift);
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_UP:
                    move_up(shift);
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_DOWN:
                    move_down(shift);
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_HOME:
                    if (ctrl) {
                        g_app.caret.line = 0;
                        g_app.caret.col = 0;
                    } else {
                        g_app.caret.col = 0;
                    }
                    if (!shift) {
                        g_app.sel_anchor = g_app.caret;
                        g_app.selecting = false;
                    } else {
                        g_app.selecting = true;
                    }
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_END:
                    if (ctrl) {
                        g_app.caret.line = editor_total_lines(&g_app) - 1;
                    }
                    g_app.caret.col = get_line_length(g_app.caret.line);
                    if (!shift) {
                        g_app.sel_anchor = g_app.caret;
                        g_app.selecting = false;
                    } else {
                        g_app.selecting = true;
                    }
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_PRIOR:  // Page up
                    g_app.caret.line -= (g_app.client_rc.bottom - g_app.theme.line_h - 2) / 
                                       g_app.theme.line_h;
                    if (g_app.caret.line < 0) {
                        g_app.caret.line = 0;
                    }
                    if (!shift) {
                        g_app.sel_anchor = g_app.caret;
                        g_app.selecting = false;
                    } else {
                        g_app.selecting = true;
                    }
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_NEXT:  // Page down
                    g_app.caret.line += (g_app.client_rc.bottom - g_app.theme.line_h - 2) / 
                                       g_app.theme.line_h;
                    {
                        int total = editor_total_lines(&g_app);
                        if (g_app.caret.line >= total) {
                            g_app.caret.line = total - 1;
                        }
                    }
                    if (!shift) {
                        g_app.sel_anchor = g_app.caret;
                        g_app.selecting = false;
                    } else {
                        g_app.selecting = true;
                    }
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_BACK:
                    backspace();
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_DELETE:
                    delete_forward();
                    ensure_caret_visible();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                    
                case VK_INSERT:
                    g_app.overwrite_mode = !g_app.overwrite_mode;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
            }
            return 0;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 1: open_file_dialog(); break;
                case 2: save_file(); break;
                case 3: save_file_as_dialog(); break;
                case 4: run_current_file(); break;
                case 5: g_app.out.visible = !g_app.out.visible; 
                        InvalidateRect(hwnd, NULL, TRUE); break;
                case 6: open_find_dialog(); break;
                case 7: open_goto_dialog(); break;
            }
            return 0;
        }
        
        case WM_CLOSE: {
            // Check for unsaved changes
            if (g_app.buf.dirty) {
                int result = MessageBoxW(hwnd, 
                    L"You have unsaved changes. Do you want to save before closing?",
                    L"Unsaved Changes", 
                    MB_YESNOCANCEL | MB_ICONWARNING);
                    
                if (result == IDYES) {
                    save_file();
                } else if (result == IDCANCEL) {
                    return 0;
                }
            }
            
            run_kill(&g_app);
            DestroyWindow(hwnd);
            return 0;
        }
        
        case WM_DESTROY: {
            gb_free(&g_app.buf);
            if (g_app.out.buf.data) {
                gb_free(&g_app.out.buf);
            }
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ===== Entry Point =====

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    // Enable DPI awareness
    SetProcessDPIAware();
    
    // Register window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"WoflIdeWindow";
    RegisterClassW(&wc);
    
    // Create main window
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"WOFL IDE",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 1024, 768,
                                 NULL, NULL, hInstance, NULL);
    
    if (!hwnd) return 0;
    
    // Handle command line arguments (open file)
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv && argc > 1) {
        EolMode eol;
        if (gb_load_from_file(&g_app.buf, argv[1], &eol)) {
            g_app.buf.eol_mode = eol;
            set_current_file(argv[1]);
            config_set_default_run_cmd(&g_app);
            config_load_run_cmd(&g_app);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
    if (argv) LocalFree(argv);
    
    // Message loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    return (int)msg.wParam;
}
