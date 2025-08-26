#ifndef WOFL_EDITOR_H
#define WOFL_EDITOR_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

#define WOFL_MAX_PATH 1024
#define WOFL_CMD_MAX  2048
#define WOFL_FIND_MAX 512

typedef enum { EOL_LF=0, EOL_CRLF=1 } EolMode;

typedef struct {
    wchar_t *data;      // buffer with a gap [gap_start, gap_end)
    size_t   cap;
    size_t   gap_start;
    size_t   gap_end;
    EolMode  eol_mode;
    bool     dirty;
} GapBuffer;

typedef struct {
    int line;
    int col;
    size_t index; // absolute char index in logical text (excluding gap)
} Caret;

typedef enum {
    LANG_NONE=0, LANG_C, LANG_PY, LANG_JS, LANG_HTML, LANG_CSS, LANG_JSON,
    LANG_MD, LANG_GO, LANG_RS, LANG_SH, LANG_LUA
} Language;

typedef struct {
    HFONT hFont;
    int   font_px;
    int   line_h;
    int   ch_w;
    COLORREF col_bg;
    COLORREF col_fg;
    COLORREF col_sel_bg;
    COLORREF col_status_bg;
    COLORREF col_output_bg;
    COLORREF syn_colors[8]; // token classes
} Theme;

typedef struct {
    bool  active;
    wchar_t text[WOFL_FIND_MAX];
    int   len;
    bool  last_dir_down;
} FindState;

typedef struct {
    bool   visible;
    GapBuffer buf; // output text buffer (append-only)
    int    height_px; // fixed split height when visible
    HANDLE proc_handle;
    HANDLE reader_thread;
    HANDLE pipe_rd; // reading from child stdout/stderr
    CRITICAL_SECTION lock;
} OutputPane;

typedef enum {
    MODE_EDIT=0, MODE_FIND, MODE_PALETTE, MODE_INPUT, MODE_GOTO
} UiMode;

typedef struct {
    wchar_t file_path[WOFL_MAX_PATH];
    wchar_t file_dir[WOFL_MAX_PATH];
    wchar_t file_name[WOFL_MAX_PATH];
    Language lang;
    GapBuffer buf;
    Caret caret;
    Caret sel_anchor;
    bool   selecting;
    int    top_line;   // first visible line
    int    left_col;   // horizontal scroll in columns
    int    tab_w;
    bool   overwrite;
    UiMode mode;

    // overlays
    FindState find;

    // command palette / input line
    bool   overlay_active;
    wchar_t overlay_prompt[128];
    wchar_t overlay_text[WOFL_CMD_MAX];
    int    overlay_len;
    int    overlay_cursor;

    // run config
    wchar_t run_cmd[WOFL_CMD_MAX];

    // window + theme
    HWND   hwnd;
    RECT   client_rc;
    Theme  theme;

    // status/metrics
    int    total_lines_cache;
    bool   need_recount;

    // output pane
    OutputPane out;
} AppState;

// ===== editor_buffer.c =====
void gb_init(GapBuffer *gb);
void gb_free(GapBuffer *gb);
size_t gb_length(const GapBuffer *gb);
void gb_move_gap(GapBuffer *gb, size_t pos);
void gb_ensure(GapBuffer *gb, size_t need);
void gb_insert(GapBuffer *gb, const wchar_t *s, size_t n);
void gb_insert_ch(GapBuffer *gb, wchar_t ch);
void gb_delete_range(GapBuffer *gb, size_t pos, size_t n);
wchar_t gb_char_at(const GapBuffer *gb, size_t pos);
void gb_load_from_utf8_file(GapBuffer *gb, const wchar_t *path, EolMode *eol);
bool gb_save_to_utf8_file(GapBuffer *gb, const wchar_t *path, EolMode eol);

// ===== editor_render.c =====
void theme_default(Theme *th);
void editor_layout_metrics(AppState *app, HDC hdc);
void editor_recount_lines(AppState *app);
int  editor_total_lines(const AppState *app);
size_t editor_line_start_index(const GapBuffer *gb, int line);
void editor_index_to_linecol(const GapBuffer *gb, size_t idx, int *out_line, int *out_col);
size_t editor_linecol_to_index(const GapBuffer *gb, int line, int col);
void editor_paint(AppState *app, HDC hdc);

// ===== syntax_core.c =====
typedef enum {
    TK_TEXT=0, TK_KW, TK_IDENT, TK_NUM, TK_STR, TK_CHAR, TK_COMMENT, TK_PUNCT
} TokenClass;

typedef struct {
    size_t start; size_t len; TokenClass cls;
} TokenSpan;

typedef struct {
    void (*scan_line)(const wchar_t *line, int len, TokenSpan *out, int *out_n);
} Syntax;

Language lang_from_ext(const wchar_t *path);
const Syntax* syntax_get(Language lang);

// ===== find.c =====
bool find_next(AppState *app, const wchar_t *needle, bool case_ins, bool wrap, bool down);

// ===== cmd_palette.c =====
void palette_open(AppState *app);
void palette_handle_char(AppState *app, wchar_t ch);
void palette_backspace(AppState *app);
void palette_confirm(AppState *app);

// ===== config.c =====
bool config_try_load_run_cmd(AppState *app);
void config_set_default_run_cmd(AppState *app);

// ===== run_exec_win32.c =====
bool run_spawn(AppState *app, const wchar_t *cmdline);
void run_kill(AppState *app);

// ===== utils =====
static inline bool is_word(wchar_t c) { return (c==L'_' || (c>=L'0'&&c<=L'9') || (c>=L'a'&&c<=L'z') || (c>=L'A'&&c<=L'Z')); }

#endif
