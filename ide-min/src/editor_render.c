#include "editor.h"
#include <stdlib.h>
#include <string.h>

void theme_default(Theme *th){
    th->font_px = 16;
    th->col_bg  = RGB(16,18,20);
    th->col_fg  = RGB(220,220,220);
    th->col_sel_bg = RGB(50,80,120);
    th->col_status_bg = RGB(32,36,40);
    th->col_output_bg = RGB(10,10,10);
    th->syn_colors[TK_TEXT]    = th->col_fg;
    th->syn_colors[TK_KW]      = RGB(120,170,255);
    th->syn_colors[TK_IDENT]   = RGB(220,220,220);
    th->syn_colors[TK_NUM]     = RGB(247,140,108);
    th->syn_colors[TK_STR]     = RGB(195,232,141);
    th->syn_colors[TK_CHAR]    = RGB(195,232,141);
    th->syn_colors[TK_COMMENT] = RGB(106,153,85);
    th->syn_colors[TK_PUNCT]   = RGB(200,200,200);
}

static HFONT make_font_px(int px){
    LOGFONTW lf = {0};
    lf.lfHeight = -px;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet= DEFAULT_CHARSET;
    wcscpy_s(lf.lfFaceName, 32, L"Consolas");
    return CreateFontIndirectW(&lf);
}

void editor_layout_metrics(AppState *app, HDC hdc){
    if(app->theme.hFont) DeleteObject(app->theme.hFont);
    app->theme.hFont = make_font_px(app->theme.font_px);
    SelectObject(hdc, app->theme.hFont);
    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    app->theme.line_h = tm.tmHeight + 2;
    SIZE sz; GetTextExtentPoint32W(hdc, L"M", 1, &sz);
    app->theme.ch_w = sz.cx;
}

void editor_recount_lines(AppState *app){
    // cheap scan for total lines
    size_t n = gb_length(&app->buf);
    int lines=1;
    for(size_t i=0;i<n;i++) if(gb_char_at(&app->buf,i)==L'\n') lines++;
    app->total_lines_cache=lines;
    app->need_recount=false;
}
int editor_total_lines(const AppState *app){ return app->total_lines_cache>0?app->total_lines_cache:1; }

size_t editor_line_start_index(const GapBuffer *gb, int line){
    if(line<=0) return 0;
    int cur=0; size_t n=gb_length(gb);
    for(size_t i=0;i<n;i++){
        if(gb_char_at(gb,i)==L'\n'){ cur++; if(cur==line) return i+1; }
    }
    return n;
}

void editor_index_to_linecol(const GapBuffer *gb, size_t idx, int *out_line, int *out_col){
    int line=0; int col=0;
    size_t n = (idx>gb_length(gb)?gb_length(gb):idx);
    size_t i=0;
    while(i<n){
        wchar_t c = gb_char_at(gb,i++);
        if(c==L'\n'){ line++; col=0; } else col++;
    }
    *out_line=line; *out_col=col;
}

size_t editor_linecol_to_index(const GapBuffer *gb, int line, int col){
    size_t idx = editor_line_start_index(gb, line);
    size_t n = gb_length(gb);
    size_t i=idx; int c=0;
    while(i<n && c<col){ if(gb_char_at(gb,i)==L'\n') break; i++; c++; }
    return i;
}

// --- painting ---
static void fill_rect(HDC hdc, RECT *rc, COLORREF col){
    HBRUSH b = CreateSolidBrush(col);
    FillRect(hdc, rc, b);
    DeleteObject(b);
}
static void draw_text(HDC hdc, int x, int y, const wchar_t *s, int n, COLORREF col){
    SetTextColor(hdc, col);
    SetBkMode(hdc, TRANSPARENT);
    ExtTextOutW(hdc, x, y, 0, NULL, s, n, NULL);
}

static int paint_output(AppState *app, HDC hdc, int client_h){
    if(!app->out.visible) return 0;
    int h = app->out.height_px;
    RECT rc = {0, client_h-h, app->client_rc.right, client_h};
    fill_rect(hdc, &rc, app->theme.col_output_bg);

    // draw last lines of output
    size_t len = gb_length(&app->out.buf);
    int max_lines = h / app->theme.line_h;
    // find line starts from end
    int count=0; size_t i=len;
    size_t start=len;
    while(i>0 && count<max_lines){
        i--;
        if(gb_char_at(&app->out.buf, i)==L'\n'){ start=i+1; count++; }
    }
    int y = client_h - h + 2;
    int x = 4;
    size_t p=start; wchar_t tmp[4096];
    while(p<len && y < client_h-2){
        int k=0;
        while(p<len && k<4095){
            wchar_t c = gb_char_at(&app->out.buf,p++);
            if(c==L'\n') break;
            tmp[k++]=c;
        }
        draw_text(hdc, x, y, tmp, k, RGB(200,200,200));
        y += app->theme.line_h;
        if(p<len && gb_char_at(&app->out.buf,p)==L'\n') p++;
    }
    return h;
}

void editor_paint(AppState *app, HDC hdc){
    RECT rc = app->client_rc;
    int w = rc.right, h = rc.bottom;
    int out_h = paint_output(app, hdc, h);
    h -= out_h;

    RECT bg = {0,0,w,h}; fill_rect(hdc, &bg, app->theme.col_bg);

    // status bar
    RECT rcStatus = {0, h-app->theme.line_h-2, w, h};
    fill_rect(hdc, &rcStatus, app->theme.col_status_bg);

    // text area (exclude status)
    RECT rcText = {0,0,w, h-app->theme.line_h-2};

    // draw visible lines
    int lines_fit = rcText.bottom / app->theme.line_h;
    int first = app->top_line;
    int last  = first + lines_fit;
    if(app->need_recount) editor_recount_lines(app);
    int total = editor_total_lines(app);
    if(last>total) last=total;

    // selection range -> absolute indices
    size_t sel_a=0, sel_b=0;
    bool sel=false;
    if(app->selecting){
        size_t cidx = editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
        size_t aidx = editor_linecol_to_index(&app->buf, app->sel_anchor.line, app->sel_anchor.col);
        sel = (cidx!=aidx);
        if(sel){ sel_a = cidx<aidx?cidx:aidx; sel_b = cidx>aidx?cidx:aidx; }
    }

    const Syntax *syn = syntax_get(app->lang);

    int y=0;
    for(int line=first; line<last; ++line){
        size_t ls = editor_line_start_index(&app->buf, line);
        size_t le = editor_line_start_index(&app->buf, line+1);
        if(le<ls) le=ls;
        int k=0; // build a temporary line
        static wchar_t tbuf[8192];
        for(size_t i=ls;i<le;i++){
            wchar_t c = gb_char_at(&app->buf,i);
            if(c==L'\n') break;
            tbuf[k++] = c;
            if(k>=8190) break;
        }
        // syntax scan -> spans
        TokenSpan spans[256]; int ns=0;
        syn->scan_line(tbuf, k, spans, &ns);

        int x = 4 - app->left_col*app->theme.ch_w;
        int pos_in_line = 0;
        for(int s=0;s<ns;s++){
            int seglen = (int)spans[s].len;
            // selection fill if intersects
            if(sel){
                size_t seg_start = ls + spans[s].start;
                size_t seg_end   = seg_start + spans[s].len;
                size_t ss = seg_start<sel_a?sel_a:seg_start;
                size_t ee = seg_end>sel_b?sel_b:seg_end;
                if(ss<ee){
                    int a = (int)(spans[s].start + (ss - seg_start));
                    int b = a + (int)(ee - ss);
                    RECT hi = { x + a*app->theme.ch_w, y, x + b*app->theme.ch_w, y+app->theme.line_h };
                    fill_rect(hdc, &hi, app->theme.col_sel_bg);
                }
            }
            COLORREF col = app->theme.syn_colors[spans[s].cls];
            draw_text(hdc, x + (int)spans[s].start*app->theme.ch_w, y, tbuf+spans[s].start, seglen, col);
            pos_in_line += seglen;
        }
        y += app->theme.line_h;
    }

    // caret
    int cx = 4 + (app->caret.col - app->left_col) * app->theme.ch_w;
    int cy = (app->caret.line - app->top_line) * app->theme.line_h;
    RECT caret = {cx, cy, cx+ (app->overwrite?app->theme.ch_w:2), cy+app->theme.line_h};
    InvertRect(hdc, &caret);

    // status text
    wchar_t status[256];
    swprintf(status, 256, L"%ls%s  |  Ln %d, Col %d  |  %ls  |  %ls",
             app->file_name[0]?app->file_name:L"(untitled)",
             app->buf.dirty?L"*":L"",
             app->caret.line+1, app->caret.col+1,
             (app->lang==LANG_C?L"C/C++":app->lang==LANG_PY?L"Python":app->lang==LANG_JS?L"JavaScript":L"Text"),
             app->out.visible?L"OUT:ON":L"OUT:OFF");
    draw_text(hdc, 6, rcStatus.top+2, status, (int)wcslen(status), RGB(180,180,180));

    // overlay
    if(app->overlay_active){
        RECT bar = {0,0,w, app->theme.line_h+6};
        fill_rect(hdc, &bar, RGB(25,30,40));
        wchar_t line[WOFL_CMD_MAX+64];
        swprintf(line, WOFL_CMD_MAX+64, L"%ls %ls", app->overlay_prompt, app->overlay_text);
        draw_text(hdc, 8, 3, line, (int)wcslen(line), RGB(235,235,235));
    }
}
