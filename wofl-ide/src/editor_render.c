// ==================== editor_render.c ====================
// Rendering and display functionality

#include "editor.h"
#include <stdlib.h>
#include <string.h>

/**
 * Initialize default theme colors and fonts
 */
void theme_init_default(Theme *th) {
    th->font_px = 16;
    th->col_bg = RGB(16, 18, 20);
    th->col_fg = RGB(220, 220, 220);
    th->col_sel_bg = RGB(50, 80, 120);
    th->col_status_bg = RGB(32, 36, 40);
    th->col_output_bg = RGB(10, 10, 10);
    
    // Syntax highlighting colors
    th->syn_colors[TK_TEXT]    = th->col_fg;
    th->syn_colors[TK_KW]      = RGB(120, 170, 255);  // Blue
    th->syn_colors[TK_IDENT]   = RGB(220, 220, 220);  // Default
    th->syn_colors[TK_NUM]     = RGB(247, 140, 108);  // Orange
    th->syn_colors[TK_STR]     = RGB(195, 232, 141);  // Green
    th->syn_colors[TK_CHAR]    = RGB(195, 232, 141);  // Green
    th->syn_colors[TK_COMMENT] = RGB(106, 153, 85);   // Dark green
    th->syn_colors[TK_PUNCT]   = RGB(200, 200, 200);  // Light gray
}

/**
 * Create font with specified pixel size
 */
static HFONT create_editor_font(int px_size) {
    LOGFONTW lf = {0};
    lf.lfHeight = -px_size;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, 32, L"Consolas");
    return CreateFontIndirectW(&lf);
}

/**
 * Update font metrics for rendering
 */
void editor_update_metrics(AppState *app, HDC hdc) {
    if (app->theme.hFont) {
        DeleteObject(app->theme.hFont);
    }
    
    app->theme.hFont = create_editor_font(app->theme.font_px);
    SelectObject(hdc, app->theme.hFont);
    
    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    app->theme.line_h = tm.tmHeight + 2;
    
    SIZE size;
    GetTextExtentPoint32W(hdc, L"M", 1, &size);
    app->theme.ch_w = size.cx;
}

/**
 * Count total lines in buffer
 */
void editor_recount_lines(AppState *app) {
    size_t len = gb_length(&app->buf);
    int lines = 1;
    
    for (size_t i = 0; i < len; i++) {
        if (gb_char_at(&app->buf, i) == L'\n') {
            lines++;
        }
    }
    
    app->total_lines_cache = lines;
    app->need_recount = false;
}

/**
 * Get total line count
 */
int editor_total_lines(const AppState *app) {
    return max_int(app->total_lines_cache, 1);
}

/**
 * Find start index of specified line
 */
size_t editor_line_start_index(const GapBuffer *gb, int line) {
    if (line <= 0) return 0;
    
    int current_line = 0;
    size_t len = gb_length(gb);
    
    for (size_t i = 0; i < len; i++) {
        if (gb_char_at(gb, i) == L'\n') {
            current_line++;
            if (current_line == line) {
                return i + 1;
            }
        }
    }
    
    return len;
}

/**
 * Convert buffer index to line/column
 */
void editor_index_to_linecol(const GapBuffer *gb, size_t idx, int *line, int *col) {
    int current_line = 0;
    int current_col = 0;
    size_t len = min_size(idx, gb_length(gb));
    
    for (size_t i = 0; i < len; i++) {
        if (gb_char_at(gb, i) == L'\n') {
            current_line++;
            current_col = 0;
        } else {
            current_col++;
        }
    }
    
    *line = current_line;
    *col = current_col;
}

/**
 * Convert line/column to buffer index
 */
size_t editor_linecol_to_index(const GapBuffer *gb, int line, int col) {
    size_t idx = editor_line_start_index(gb, line);
    size_t len = gb_length(gb);
    int current_col = 0;
    
    while (idx < len && current_col < col) {
        if (gb_char_at(gb, idx) == L'\n') break;
        idx++;
        current_col++;
    }
    
    return idx;
}

/**
 * Helper to fill rectangle with color
 */
static void fill_rect(HDC hdc, RECT *rc, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, rc, brush);
    DeleteObject(brush);
}

/**
 * Helper to draw text at position
 */
static void draw_text_ex(HDC hdc, int x, int y, const wchar_t *text, 
                          int len, COLORREF color) {
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    ExtTextOutW(hdc, x, y, 0, NULL, text, len, NULL);
}

/**
 * Paint output pane
 */
static int paint_output_pane(AppState *app, HDC hdc, int client_height) {
    if (!app->out.visible) return 0;
    
    int height = app->out.height_px;
    RECT rc = {0, client_height - height, app->client_rc.right, client_height};
    fill_rect(hdc, &rc, app->theme.col_output_bg);
    
    // Draw output text
    EnterCriticalSection(&app->out.lock);
    
    size_t len = gb_length(&app->out.buf);
    int max_lines = height / app->theme.line_h;
    
    // Find start of visible lines from end
    int line_count = 0;
    size_t start_pos = len;
    
    for (size_t i = len; i > 0 && line_count < max_lines; i--) {
        if (gb_char_at(&app->out.buf, i - 1) == L'\n') {
            start_pos = i;
            line_count++;
        }
    }
    
    // Draw lines
    int y = client_height - height + 2;
    int x = 4;
    wchar_t line_buf[WOFL_LINE_BUF_MAX];
    size_t pos = start_pos;
    
    while (pos < len && y < client_height - 2) {
        int k = 0;
        while (pos < len && k < WOFL_LINE_BUF_MAX - 1) {
            wchar_t ch = gb_char_at(&app->out.buf, pos++);
            if (ch == L'\n') break;
            line_buf[k++] = ch;
        }
        
        draw_text_ex(hdc, x, y, line_buf, k, RGB(200, 200, 200));
        y += app->theme.line_h;
    }
    
    LeaveCriticalSection(&app->out.lock);
    return height;
}

/**
 * Main paint function
 */
void editor_paint(AppState *app, HDC hdc) {
    RECT rc = app->client_rc;
    int width = rc.right;
    int height = rc.bottom;
    
    // Paint output pane first
    int output_height = paint_output_pane(app, hdc, height);
    height -= output_height;
    
    // Background
    RECT bg_rect = {0, 0, width, height};
    fill_rect(hdc, &bg_rect, app->theme.col_bg);
    
    // Status bar
    RECT status_rect = {0, height - app->theme.line_h - 2, width, height};
    fill_rect(hdc, &status_rect, app->theme.col_status_bg);
    
    // Text area
    RECT text_rect = {0, 0, width, height - app->theme.line_h - 2};
    
    // Calculate visible lines
    int lines_fit = text_rect.bottom / app->theme.line_h;
    int first_line = app->top_line;
    int last_line = min_int(first_line + lines_fit, editor_total_lines(app));
    
    // Get selection range
    size_t sel_start = 0, sel_end = 0;
    bool has_selection = false;
    
    if (app->selecting) {
        size_t caret_idx = editor_linecol_to_index(&app->buf, 
                                                    app->caret.line, app->caret.col);
        size_t anchor_idx = editor_linecol_to_index(&app->buf,
            app->sel_anchor.line, app->sel_anchor.col);
        if (caret_idx != anchor_idx) {
            has_selection = true;
            sel_start = min_size(caret_idx, anchor_idx);
            sel_end = max_size(caret_idx, anchor_idx);
        }
    }
    
    // Get syntax highlighter
    const Syntax *syntax = syntax_get(app->lang);
    
    // Draw visible lines
    int y = 0;
    for (int line = first_line; line < last_line; line++) {
        size_t line_start = editor_line_start_index(&app->buf, line);
        size_t line_end = editor_line_start_index(&app->buf, line + 1);
        
        // Build line text
        wchar_t line_text[WOFL_LINE_BUF_MAX];
        int line_len = 0;
        
        for (size_t i = line_start; i < line_end && line_len < WOFL_LINE_BUF_MAX - 1; i++) {
            wchar_t ch = gb_char_at(&app->buf, i);
            if (ch == L'\n') break;
            line_text[line_len++] = ch;
        }
        line_text[line_len] = L'\0';
        
        // Syntax highlight
        TokenSpan tokens[WOFL_MAX_TOKENS];
        int token_count = 0;
        syntax->scan_line(line_text, line_len, tokens, &token_count);
        
        int x = 4 - app->left_col * app->theme.ch_w;
        
        // Draw tokens with selection
        for (int t = 0; t < token_count; t++) {
            TokenSpan *token = &tokens[t];
            
            // Draw selection background if needed
            if (has_selection) {
                size_t token_start = line_start + token->start;
                size_t token_end = token_start + token->len;
                size_t sel_token_start = max_size(token_start, sel_start);
                size_t sel_token_end = min_size(token_end, sel_end);
                
                if (sel_token_start < sel_token_end) {
                    int sel_x_start = x + (int)(sel_token_start - token_start) * app->theme.ch_w;
                    int sel_x_end = x + (int)(sel_token_end - token_start) * app->theme.ch_w;
                    RECT sel_rect = {sel_x_start, y, sel_x_end, y + app->theme.line_h};
                    fill_rect(hdc, &sel_rect, app->theme.col_sel_bg);
                }
            }
            
            // Draw token text
            COLORREF color = app->theme.syn_colors[token->cls];
            draw_text_ex(hdc, x + (int)token->start * app->theme.ch_w, y,
                        line_text + token->start, (int)token->len, color);
        }
        
        y += app->theme.line_h;
    }
    
    // Draw caret
    int caret_x = 4 + (app->caret.col - app->left_col) * app->theme.ch_w;
    int caret_y = (app->caret.line - app->top_line) * app->theme.line_h;
    RECT caret_rect = {
        caret_x, caret_y,
        caret_x + (app->overwrite_mode ? app->theme.ch_w : 2),
        caret_y + app->theme.line_h
    };
    InvertRect(hdc, &caret_rect);
    
    // Draw status bar text
    wchar_t status[256];
    swprintf(status, 256, L"%ls%s  |  Ln %d, Col %d  |  %ls  |  %ls",
             app->file_name[0] ? app->file_name : L"(untitled)",
             app->buf.dirty ? L"*" : L"",
             app->caret.line + 1, app->caret.col + 1,
             syntax_get(app->lang)->keywords[0] ? L"Syntax ON" : L"Plain Text",
             app->out.visible ? L"OUT:ON" : L"OUT:OFF");
    draw_text_ex(hdc, 6, status_rect.top + 2, status, (int)wcslen(status), 
                RGB(180, 180, 180));
    
    // Draw overlay if active
    if (app->overlay_active) {
        RECT overlay_rect = {0, 0, width, app->theme.line_h + 6};
        fill_rect(hdc, &overlay_rect, RGB(25, 30, 40));
        
        wchar_t overlay_text[WOFL_CMD_MAX + 64];
        swprintf(overlay_text, WOFL_CMD_MAX + 64, L"%ls %ls", 
                app->overlay_prompt, app->overlay_text);
        draw_text_ex(hdc, 8, 3, overlay_text, (int)wcslen(overlay_text), 
                    RGB(235, 235, 235));
        
        // Draw overlay cursor
        int cursor_x = 8 + (int)(wcslen(app->overlay_prompt) + 1 + app->overlay_cursor) * app->theme.ch_w;
        RECT cursor_rect = {cursor_x, 3, cursor_x + 2, 3 + app->theme.line_h};
        InvertRect(hdc, &cursor_rect);
    }
}
