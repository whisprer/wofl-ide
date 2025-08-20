// ==================== theme_system.c ====================
// Theme setup that matches editor.h's Theme exactly.

#include "editor.h"
#include <windows.h>

static inline COLORREF rgb_u8(unsigned r, unsigned g, unsigned b) {
    return RGB((BYTE)r, (BYTE)g, (BYTE)b);
}

void theme_default(Theme *th) {
    if (!th) return;

    // Base UI colors
    th->col_bg        = rgb_u8(24, 24, 27);    // editor background
    th->col_fg        = rgb_u8(224, 224, 224); // default foreground
    th->col_sel_bg    = rgb_u8(65, 105, 225);  // selection background (royal blue)
    th->col_status_bg = rgb_u8(38, 38, 45);    // status bar bg
    th->col_output_bg = rgb_u8(22, 22, 26);    // output pane bg

    // Syntax colors (index by TokenClass)
    // TK_TEXT, TK_KW, TK_IDENT, TK_NUM, TK_STR, TK_CHAR, TK_COMMENT, TK_PUNCT
    th->syn_colors[TK_TEXT]    = th->col_fg;
    th->syn_colors[TK_KW]      = rgb_u8(189, 147, 249);
    th->syn_colors[TK_IDENT]   = rgb_u8(200, 220, 255);
    th->syn_colors[TK_NUM]     = rgb_u8(255, 121, 198);
    th->syn_colors[TK_STR]     = rgb_u8(241, 250, 140);
    th->syn_colors[TK_CHAR]    = rgb_u8(255, 200, 140);
    th->syn_colors[TK_COMMENT] = rgb_u8(98, 114, 164);
    th->syn_colors[TK_PUNCT]   = rgb_u8(180, 200, 220);

    // Font/metrics — editor_layout_metrics() will create HFONT and compute metrics.
    th->hFont   = NULL;  // force layout to (re)create
    th->font_px = 16;    // request size; layout may adapt for DPI
    th->line_h  = 0;     // computed later
    th->ch_w    = 0;     // computed later
}
