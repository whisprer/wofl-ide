// ==================== editor.h (COMPLETE FIXED VERSION) ====================
// This is the complete, working editor.h with all fixes

#ifndef WOFL_EDITOR_H
#define WOFL_EDITOR_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

// ===== Constants =====
#define WOFL_MAX_PATH     1024
#define WOFL_CMD_MAX      2048
#define WOFL_FIND_MAX     512
#define WOFL_MAX_TOKENS   256
#define WOFL_LINE_BUF_MAX 8192
#define WOFL_DEFAULT_TAB  4
#define WOFL_INITIAL_GAP  4096

// ===== Utility Functions (INLINE) =====
static inline bool is_word(wchar_t c) {
    return (c == L'_' || (c >= L'0' && c <= L'9') || 
            (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z'));
}

static inline size_t min_size(size_t a, size_t b) { return a < b ? a : b; }
static inline size_t max_size(size_t a, size_t b) { return a > b ? a : b; }
static inline int min_int(int a, int b) { return a < b ? a : b; }
static inline int max_int(int a, int b) { return a > b ? a : b; }

// ===== Type Definitions =====
typedef enum {
    EOL_LF = 0,
    EOL_CRLF = 1
} EolMode;

typedef enum {
    LANG_NONE = 0,
    LANG_C,
    LANG_CPP,
    LANG_ASM,
    LANG_CSV,
    LANG_PY,
    LANG_JS,
    LANG_HTML,
    LANG_CSS,
    LANG_JSON,
    LANG_MD,
    LANG_GO,
    LANG_RS,
    LANG_SH,
    LANG_LUA,
    LANG_MAX
} Language;

typedef enum {
    TK_TEXT = 0,
    TK_KW,
    TK_IDENT,
    TK_NUM,
    TK_STR,
    TK_CHAR,
    TK_COMMENT,
    TK_PUNCT,
    TK_MAX
} TokenClass;

typedef enum {
    MODE_EDIT = 0,
    MODE_FIND,
    MODE_PALETTE,
    MODE_INPUT,
    MODE_GOTO
} UiMode;

// ===== Data Structures =====
typedef struct {
    wchar_t *data;
    size_t   capacity;
    size_t   gap_start;
    size_t   gap_end;
    EolMode  eol_mode;
    bool     dirty;
} GapBuffer;

typedef struct {
    int    line;
    int    col;
    size_t index;
} Caret;

typedef struct {
    size_t     start;
    size_t     len;
    TokenClass cls;
} TokenSpan;

typedef void (*SyntaxScanFn)(const wchar_t *line, int len, TokenSpan *out, int *out_n);

typedef struct {
    SyntaxScanFn scan_line;
} Syntax;

typedef struct {
    HFONT    hFont;
    int      font_px;
    int      line_h;
    int      ch_w;
    COLORREF col_bg;
    COLORREF col_fg;
    COLORREF col_sel_bg;
    COLORREF col_status_bg;
    COLORREF col_output_bg;
    COLORREF syn_colors[TK_MAX];
} Theme;

typedef struct {
    bool    active;
    wchar_t text[WOFL_FIND_MAX];
    int     len;
    bool    last_dir_down;
} FindState;

typedef struct {
    bool     visible;
    GapBuffer buf;
    int      height_px;
    HANDLE   proc_handle;
    HANDLE   reader_thread;
    HANDLE   pipe_rd;
    CRITICAL_SECTION lock;
} OutputPane;

typedef struct {
    wchar_t  file_path[WOFL_MAX_PATH];
    wchar_t  file_dir[WOFL_MAX_PATH];
    wchar_t  file_name[WOFL_MAX_PATH];
    Language lang;
    
    GapBuffer buf;
    Caret    caret;
    Caret    sel_anchor;
    bool     selecting;
    int      top_line;
    int      left_col;
    int      tab_width;
    bool     overwrite_mode;
    
    UiMode   mode;
    FindState find;
    bool     overlay_active;
    wchar_t  overlay_prompt[128];
    wchar_t  overlay_text[WOFL_CMD_MAX];
    int      overlay_len;
    int      overlay_cursor;
    
    wchar_t  run_cmd[WOFL_CMD_MAX];
    OutputPane out;
    
    HWND     hwnd;
    RECT     client_rc;
    Theme    theme;
    
    int      total_lines_cache;
    bool     need_recount;
} AppState;

// ===== Function Declarations =====

// Buffer operations
void     gb_init(GapBuffer *gb);
void     gb_free(GapBuffer *gb);
size_t   gb_length(const GapBuffer *gb);
void     gb_move_gap(GapBuffer *gb, size_t pos);
void     gb_ensure(GapBuffer *gb, size_t need);
void     gb_insert(GapBuffer *gb, const wchar_t *s, size_t n);
void     gb_insert_ch(GapBuffer *gb, wchar_t ch);
void     gb_delete_range(GapBuffer *gb, size_t pos, size_t n);
wchar_t  gb_char_at(const GapBuffer *gb, size_t pos);
bool     gb_load_from_file(GapBuffer *gb, const wchar_t *path, EolMode *eol);
bool     gb_save_to_file(GapBuffer *gb, const wchar_t *path, EolMode eol);

// Rendering
void     theme_default(Theme *th);
void     editor_layout_metrics(AppState *app, HDC hdc);
void     editor_recount_lines(AppState *app);
int      editor_total_lines(const AppState *app);
size_t   editor_line_start_index(const GapBuffer *gb, int line);
void     editor_index_to_linecol(const GapBuffer *gb, size_t idx, int *line, int *col);
size_t   editor_linecol_to_index(const GapBuffer *gb, int line, int col);
void     editor_paint(AppState *app, HDC hdc);

// Syntax highlighting
Language syntax_detect_language(const wchar_t *path);
const Syntax* syntax_get(Language lang);

// Syntax scanners
void syntax_scan_c(const wchar_t *line, int len, TokenSpan *out, int *out_n);
void syntax_scan_py(const wchar_t *line, int len, TokenSpan *out, int *out_n);
void syntax_scan_js(const wchar_t *line, int len, TokenSpan *out, int *out_n);

// Find operations
bool     find_next(AppState *app, const wchar_t *needle, bool case_ins, bool wrap, bool down);

// Command palette
void     palette_open(AppState *app);
void     palette_handle_char(AppState *app, wchar_t ch);
void     palette_backspace(AppState *app);
void     palette_confirm(AppState *app);
void     palette_cancel(AppState *app);

// Configuration
bool     config_try_load_run_cmd(AppState *app);
void     config_set_default_run_cmd(AppState *app);

// Process execution
bool     run_spawn(AppState *app, const wchar_t *cmdline);
void     run_kill(AppState *app);

#endif // WOFL_EDITOR_H
