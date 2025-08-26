// ==================== main_ncurses.c ====================
// WOFL IDE - ncurses terminal version for Linux

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <locale.h>

// ===== Constants =====
#define WOFL_MAX_PATH     1024
#define WOFL_CMD_MAX      2048
#define WOFL_FIND_MAX     512
#define WOFL_LINE_BUF_MAX 8192
#define WOFL_DEFAULT_TAB  4
#define WOFL_INITIAL_GAP  4096

// ===== Types =====
typedef enum {
    LANG_NONE = 0,
    LANG_C, LANG_CPP, LANG_PY, LANG_JS, LANG_SH, LANG_MAX
} Language;

typedef enum {
    TK_TEXT = 0, TK_KW, TK_IDENT, TK_NUM, TK_STR, TK_CHAR, TK_COMMENT, TK_PUNCT, TK_MAX
} TokenClass;

typedef enum {
    MODE_EDIT = 0, MODE_FIND, MODE_PALETTE, MODE_GOTO
} UiMode;

typedef struct {
    char *data;
    size_t capacity;
    size_t gap_start;
    size_t gap_end;
    bool dirty;
} GapBuffer;

typedef struct {
    int line;
    int col;
    size_t index;
} Caret;

typedef struct {
    size_t start;
    size_t len;
    TokenClass cls;
} TokenSpan;

typedef struct {
    bool visible;
    GapBuffer buf;
    int height;
    pid_t proc_pid;
    int pipe_fd;
    pthread_t reader_thread;
    pthread_mutex_t lock;
} OutputPane;

typedef struct {
    char file_path[WOFL_MAX_PATH];
    char file_name[WOFL_MAX_PATH];
    Language lang;
    
    GapBuffer buf;
    Caret caret;
    Caret sel_anchor;
    bool selecting;
    int top_line;
    int left_col;
    int tab_width;
    
    UiMode mode;
    bool overlay_active;
    char overlay_prompt[128];
    char overlay_text[WOFL_CMD_MAX];
    int overlay_len;
    int overlay_cursor;
    
    char run_cmd[WOFL_CMD_MAX];
    OutputPane out;
    
    int screen_rows;
    int screen_cols;
    int total_lines_cache;
    bool need_recount;
} AppState;

// Global state
static AppState g_app;
static volatile bool g_running = true;

// ===== Forward Declarations =====
static void init_ncurses(void);
static void cleanup_ncurses(void);
static void handle_resize(int sig);
static void update_screen_size(void);
static Language detect_language(const char *path);
static void syntax_highlight_line(const char *line, int len, Language lang, TokenSpan *tokens, int *count);
static void gb_init(GapBuffer *gb);
static void gb_free(GapBuffer *gb);
static size_t gb_length(const GapBuffer *gb);
static void gb_insert(GapBuffer *gb, const char *text, size_t len);
static void gb_delete_range(GapBuffer *gb, size_t pos, size_t len);
static char gb_char_at(const GapBuffer *gb, size_t pos);
static bool load_file(const char *path);
static bool save_file(void);
static void render_screen(void);
static void render_editor(void);
static void render_status(void);
static void render_overlay(void);
static void render_output_pane(void);
static void handle_key(int ch);
static void move_cursor(int dy, int dx);
static void insert_char(char ch);
static void delete_char(void);
static void run_current_file(void);
static void open_find_dialog(void);
static void open_palette(void);
static void* output_reader_thread(void* arg);

// ===== Gap Buffer Implementation =====
static void gb_init(GapBuffer *gb) {
    gb->capacity = WOFL_INITIAL_GAP;
    gb->data = malloc(gb->capacity);
    gb->gap_start = 0;
    gb->gap_end = gb->capacity;
    gb->dirty = false;
}

static void gb_free(GapBuffer *gb) {
    if (gb->data) {
        free(gb->data);
        gb->data = NULL;
    }
}

static size_t gb_length(const GapBuffer *gb) {
    return gb->capacity - (gb->gap_end - gb->gap_start);
}

static void gb_ensure(GapBuffer *gb, size_t need) {
    size_t gap = gb->gap_end - gb->gap_start;
    if (gap >= need) return;
    
    size_t new_cap = gb->capacity * 2;
    char *new_data = malloc(new_cap);
    
    // Copy pre-gap data
    memcpy(new_data, gb->data, gb->gap_start);
    // Copy post-gap data
    memcpy(new_data + (new_cap - (gb->capacity - gb->gap_end)),
           gb->data + gb->gap_end,
           gb->capacity - gb->gap_end);
    
    free(gb->data);
    gb->data = new_data;
    gb->gap_end = new_cap - (gb->capacity - gb->gap_end);
    gb->capacity = new_cap;
}

static void gb_insert(GapBuffer *gb, const char *text, size_t len) {
    gb_ensure(gb, len);
    memcpy(gb->data + gb->gap_start, text, len);
    gb->gap_start += len;
    gb->dirty = true;
}

static char gb_char_at(const GapBuffer *gb, size_t pos) {
    if (pos >= gb_length(gb)) return '\0';
    if (pos < gb->gap_start) {
        return gb->data[pos];
    } else {
        return gb->data[pos + (gb->gap_end - gb->gap_start)];
    }
}

// ===== Language Detection =====
static Language detect_language(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return LANG_NONE;
    
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) return LANG_C;
    if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".hpp") == 0) return LANG_CPP;
    if (strcmp(ext, ".py") == 0) return LANG_PY;
    if (strcmp(ext, ".js") == 0) return LANG_JS;
    if (strcmp(ext, ".sh") == 0) return LANG_SH;
    
    return LANG_NONE;
}

// ===== Simple Syntax Highlighting =====
static bool is_keyword(const char *word, Language lang) {
    static const char *c_keywords[] = {"if", "else", "for", "while", "return", "int", "char", "void", NULL};
    static const char *py_keywords[] = {"if", "else", "for", "while", "return", "def", "import", "class", NULL};
    static const char *js_keywords[] = {"if", "else", "for", "while", "return", "function", "var", "let", "const", NULL};
    
    const char **keywords = NULL;
    switch (lang) {
        case LANG_C:
        case LANG_CPP:
            keywords = c_keywords;
            break;
        case LANG_PY:
            keywords = py_keywords;
            break;
        case LANG_JS:
            keywords = js_keywords;
            break;
        default:
            return false;
    }
    
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(word, keywords[i]) == 0) return true;
    }
    return false;
}

static void syntax_highlight_line(const char *line, int len, Language lang, TokenSpan *tokens, int *count) {
    *count = 0;
    if (len == 0) return;
    
    int i = 0;
    while (i < len && *count < 64) {
        // Skip whitespace
        while (i < len && (line[i] == ' ' || line[i] == '\t')) i++;
        if (i >= len) break;
        
        int start = i;
        TokenClass cls = TK_TEXT;
        
        // Comments
        if (line[i] == '#' || (line[i] == '/' && i + 1 < len && line[i+1] == '/')) {
            cls = TK_COMMENT;
            i = len; // Rest of line is comment
        }
        // Strings
        else if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i++];
            while (i < len && line[i] != quote) {
                if (line[i] == '\\' && i + 1 < len) i += 2;
                else i++;
            }
            if (i < len) i++; // Skip closing quote
            cls = TK_STR;
        }
        // Numbers
        else if (line[i] >= '0' && line[i] <= '9') {
            while (i < len && ((line[i] >= '0' && line[i] <= '9') || line[i] == '.')) i++;
            cls = TK_NUM;
        }
        // Identifiers/Keywords
        else if ((line[i] >= 'a' && line[i] <= 'z') || (line[i] >= 'A' && line[i] <= 'Z') || line[i] == '_') {
            while (i < len && ((line[i] >= 'a' && line[i] <= 'z') || 
                              (line[i] >= 'A' && line[i] <= 'Z') || 
                              (line[i] >= '0' && line[i] <= '9') || 
                              line[i] == '_')) i++;
            
            // Check if it's a keyword
            char word[64];
            int word_len = i - start;
            if (word_len < 63) {
                strncpy(word, line + start, word_len);
                word[word_len] = '\0';
                cls = is_keyword(word, lang) ? TK_KW : TK_IDENT;
            }
        }
        // Punctuation
        else {
            while (i < len && !((line[i] >= 'a' && line[i] <= 'z') || 
                               (line[i] >= 'A' && line[i] <= 'Z') || 
                               (line[i] >= '0' && line[i] <= '9') || 
                               line[i] == '_' || line[i] == ' ' || line[i] == '\t')) i++;
            cls = TK_PUNCT;
        }
        
        tokens[*count].start = start;
        tokens[*count].len = i - start;
        tokens[*count].cls = cls;
        (*count)++;
    }
}

// ===== File Operations =====
static bool load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    gb_free(&g_app.buf);
    gb_init(&g_app.buf);
    
    char buffer[4096];
    size_t read;
    while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        gb_insert(&g_app.buf, buffer, read);
    }
    
    fclose(f);
    
    strncpy(g_app.file_path, path, WOFL_MAX_PATH - 1);
    const char *filename = strrchr(path, '/');
    strncpy(g_app.file_name, filename ? filename + 1 : path, WOFL_MAX_PATH - 1);
    g_app.lang = detect_language(path);
    g_app.caret.line = g_app.caret.col = 0;
    g_app.top_line = g_app.left_col = 0;
    g_app.buf.dirty = false;
    
    return true;
}

static bool save_file(void) {
    if (!g_app.file_path[0]) return false;
    
    FILE *f = fopen(g_app.file_path, "w");
    if (!f) return false;
    
    size_t len = gb_length(&g_app.buf);
    for (size_t i = 0; i < len; i++) {
        fputc(gb_char_at(&g_app.buf, i), f);
    }
    
    fclose(f);
    g_app.buf.dirty = false;
    return true;
}

// ===== ncurses Initialization =====
static void init_ncurses(void) {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_BLUE, -1);    // Keywords
        init_pair(2, COLOR_GREEN, -1);   // Strings
        init_pair(3, COLOR_YELLOW, -1);  // Numbers
        init_pair(4, COLOR_RED, -1);     // Comments
        init_pair(5, COLOR_CYAN, -1);    // Status bar
        init_pair(6, COLOR_WHITE, COLOR_BLUE); // Overlay
    }
    
    signal(SIGWINCH, handle_resize);
    update_screen_size();
}

static void cleanup_ncurses(void) {
    endwin();
}

static void handle_resize(int sig) {
    (void)sig;
    endwin();
    refresh();
    update_screen_size();
}

static void update_screen_size(void) {
    getmaxyx(stdscr, g_app.screen_rows, g_app.screen_cols);
    g_app.out.height = g_app.out.visible ? g_app.screen_rows / 4 : 0;
}

// ===== Rendering =====
static void render_screen(void) {
    clear();
    render_editor();
    render_output_pane();
    render_status();
    render_overlay();
    refresh();
}

static void render_editor(void) {
    size_t len = gb_length(&g_app.buf);
    int editor_height = g_app.screen_rows - 1 - g_app.out.height;
    
    // Count lines for proper scrolling
    int total_lines = 1;
    for (size_t i = 0; i < len; i++) {
        if (gb_char_at(&g_app.buf, i) == '\n') total_lines++;
    }
    
    // Build lines for display
    char line_buf[WOFL_LINE_BUF_MAX];
    int line_num = 0;
    int line_pos = 0;
    int y = 0;
    
    for (size_t i = 0; i < len && y < editor_height; i++) {
        char ch = gb_char_at(&g_app.buf, i);
        
        if (ch == '\n' || line_pos >= WOFL_LINE_BUF_MAX - 1) {
            line_buf[line_pos] = '\0';
            
            if (line_num >= g_app.top_line && y < editor_height) {
                // Syntax highlight the line
                TokenSpan tokens[64];
                int token_count;
                syntax_highlight_line(line_buf, line_pos, g_app.lang, tokens, &token_count);
                
                int x = 0;
                for (int t = 0; t < token_count; t++) {
                    int color = 0;
                    switch (tokens[t].cls) {
                        case TK_KW: color = COLOR_PAIR(1); break;
                        case TK_STR: color = COLOR_PAIR(2); break;
                        case TK_NUM: color = COLOR_PAIR(3); break;
                        case TK_COMMENT: color = COLOR_PAIR(4); break;
                        default: color = 0; break;
                    }
                    
                    attron(color);
                    for (size_t j = 0; j < tokens[t].len; j++) {
                        if (x >= g_app.left_col && x - g_app.left_col < g_app.screen_cols) {
                            mvaddch(y, x - g_app.left_col, line_buf[tokens[t].start + j]);
                        }
                        x++;
                    }
                    attroff(color);
                }
                y++;
            }
            
            line_num++;
            line_pos = 0;
        } else {
            line_buf[line_pos++] = ch;
        }
    }
    
    // Draw cursor
    int cursor_y = g_app.caret.line - g_app.top_line;
    int cursor_x = g_app.caret.col - g_app.left_col;
    if (cursor_y >= 0 && cursor_y < editor_height && 
        cursor_x >= 0 && cursor_x < g_app.screen_cols) {
        mvaddch(cursor_y, cursor_x, ' ' | A_REVERSE);
    }
}

static void render_status(void) {
    int status_y = g_app.screen_rows - 1 - g_app.out.height;
    char status[1024];
    snprintf(status, sizeof(status), "%s%s | Ln %d, Col %d | %s",
             g_app.file_name[0] ? g_app.file_name : "(untitled)",
             g_app.buf.dirty ? "*" : "",
             g_app.caret.line + 1, g_app.caret.col + 1,
             g_app.out.visible ? "OUT:ON" : "OUT:OFF");
    
    attron(COLOR_PAIR(5) | A_REVERSE);
    mvhline(status_y, 0, ' ', g_app.screen_cols);
    mvprintw(status_y, 0, "%.80s", status);
    attroff(COLOR_PAIR(5) | A_REVERSE);
}

static void render_overlay(void) {
    if (!g_app.overlay_active) return;
    
    attron(COLOR_PAIR(6));
    mvhline(0, 0, ' ', g_app.screen_cols);
    mvprintw(0, 0, "%s %s", g_app.overlay_prompt, g_app.overlay_text);
    
    // Draw cursor
    int cursor_x = strlen(g_app.overlay_prompt) + 1 + g_app.overlay_cursor;
    if (cursor_x < g_app.screen_cols) {
        mvaddch(0, cursor_x, ' ' | A_REVERSE);
    }
    attroff(COLOR_PAIR(6));
}

static void render_output_pane(void) {
    if (!g_app.out.visible) return;
    
    int start_y = g_app.screen_rows - g_app.out.height;
    
    pthread_mutex_lock(&g_app.out.lock);
    
    // Draw output buffer (simple implementation)
    size_t len = gb_length(&g_app.out.buf);
    char line_buf[1024];
    int line_pos = 0;
    int y = 0;
    
    for (size_t i = 0; i < len && y < g_app.out.height; i++) {
        char ch = gb_char_at(&g_app.out.buf, i);
        
        if (ch == '\n' || line_pos >= 1023) {
            line_buf[line_pos] = '\0';
            mvprintw(start_y + y, 0, "%.80s", line_buf);
            y++;
            line_pos = 0;
        } else {
            line_buf[line_pos++] = ch;
        }
    }
    
    pthread_mutex_unlock(&g_app.out.lock);
}

// ===== Process Execution =====
static void* output_reader_thread(void* arg) {
    int pipe_fd = *(int*)arg;
    char buffer[4096];
    ssize_t bytes_read;
    
    while ((bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        
        pthread_mutex_lock(&g_app.out.lock);
        gb_insert(&g_app.out.buf, buffer, bytes_read);
        pthread_mutex_unlock(&g_app.out.lock);
    }
    
    close(pipe_fd);
    return NULL;
}

static void run_current_file(void) {
    if (!g_app.file_path[0]) return;
    
    if (g_app.buf.dirty) save_file();
    
    // Build run command based on file type
    switch (g_app.lang) {
        case LANG_PY:
            snprintf(g_app.run_cmd, sizeof(g_app.run_cmd), "python3 \"%s\"", g_app.file_path);
            break;
        case LANG_SH:
            snprintf(g_app.run_cmd, sizeof(g_app.run_cmd), "bash \"%s\"", g_app.file_path);
            break;
        default:
            snprintf(g_app.run_cmd, sizeof(g_app.run_cmd), "\"%s\"", g_app.file_path);
            break;
    }
    
    // Create pipe for output
    int pipefd[2];
    if (pipe(pipefd) == -1) return;
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execl("/bin/sh", "sh", "-c", g_app.run_cmd, NULL);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(pipefd[1]);
        
        g_app.out.proc_pid = pid;
        g_app.out.pipe_fd = pipefd[0];
        g_app.out.visible = true;
        
        // Clear output buffer
        pthread_mutex_lock(&g_app.out.lock);
        gb_free(&g_app.out.buf);
        gb_init(&g_app.out.buf);
        pthread_mutex_unlock(&g_app.out.lock);
        
        // Start reader thread
        pthread_create(&g_app.out.reader_thread, NULL, output_reader_thread, &g_app.out.pipe_fd);
        pthread_detach(g_app.out.reader_thread);
    } else {
        close(pipefd[0]);
        close(pipefd[1]);
    }
}

// ===== Input Handling =====
static void handle_key(int ch) {
    if (g_app.overlay_active) {
        // Handle overlay input
        switch (ch) {
            case '\n':
            case '\r':
            case KEY_ENTER:
                // Process overlay command
                g_app.overlay_active = false;
                g_app.mode = MODE_EDIT;
                break;
            case 27: // ESC
                g_app.overlay_active = false;
                g_app.mode = MODE_EDIT;
                break;
            case KEY_BACKSPACE:
            case 127:
                if (g_app.overlay_cursor > 0) {
                    memmove(g_app.overlay_text + g_app.overlay_cursor - 1,
                           g_app.overlay_text + g_app.overlay_cursor,
                           g_app.overlay_len - g_app.overlay_cursor + 1);
                    g_app.overlay_cursor--;
                    g_app.overlay_len--;
                }
                break;
            default:
                if (ch >= 32 && ch < 127 && g_app.overlay_len < WOFL_CMD_MAX - 1) {
                    memmove(g_app.overlay_text + g_app.overlay_cursor + 1,
                           g_app.overlay_text + g_app.overlay_cursor,
                           g_app.overlay_len - g_app.overlay_cursor + 1);
                    g_app.overlay_text[g_app.overlay_cursor] = ch;
                    g_app.overlay_cursor++;
                    g_app.overlay_len++;
                }
                break;
        }
        return;
    }
    
    // Normal editing mode
    switch (ch) {
        case KEY_UP:    move_cursor(-1, 0); break;
        case KEY_DOWN:  move_cursor(1, 0); break;
        case KEY_LEFT:  move_cursor(0, -1); break;
        case KEY_RIGHT: move_cursor(0, 1); break;
        
        case KEY_F(3):  open_find_dialog(); break;
        case KEY_F(5):  run_current_file(); break;
        case KEY_F(6):  
            g_app.out.visible = !g_app.out.visible; 
            update_screen_size();
            break;
        
        case 16: // Ctrl+P
            open_palette();
            break;
            
        case 19: // Ctrl+S
            save_file();
            break;
            
        case 17: // Ctrl+Q
            g_running = false;
            break;
            
        case KEY_BACKSPACE:
        case 127:
            delete_char();
            break;
            
        case '\n':
        case '\r':
            insert_char('\n');
            break;
            
        case '\t':
            for (int i = 0; i < g_app.tab_width; i++) insert_char(' ');
            break;
            
        default:
            if (ch >= 32 && ch < 127) {
                insert_char(ch);
            }
            break;
    }
}

static void move_cursor(int dy, int dx) {
    g_app.caret.line += dy;
    g_app.caret.col += dx;
    
    if (g_app.caret.line < 0) g_app.caret.line = 0;
    if (g_app.caret.col < 0) g_app.caret.col = 0;
    
    // Adjust scrolling
    if (g_app.caret.line < g_app.top_line) {
        g_app.top_line = g_app.caret.line;
    }
    if (g_app.caret.line >= g_app.top_line + g_app.screen_rows - 2 - g_app.out.height) {
        g_app.top_line = g_app.caret.line - g_app.screen_rows + 3 + g_app.out.height;
    }
}

static void insert_char(char ch) {
    // Simple insertion at cursor (simplified)
    gb_insert(&g_app.buf, &ch, 1);
    if (ch == '\n') {
        g_app.caret.line++;
        g_app.caret.col = 0;
    } else {
        g_app.caret.col++;
    }
}

static void delete_char(void) {
    // Simple deletion (simplified)
    if (g_app.caret.col > 0) {
        g_app.caret.col--;
        // Would need proper gap buffer deletion here
    }
}

static void open_find_dialog(void) {
    g_app.mode = MODE_FIND;
    g_app.overlay_active = true;
    strcpy(g_app.overlay_prompt, "Find:");
    g_app.overlay_text[0] = '\0';
    g_app.overlay_len = 0;
    g_app.overlay_cursor = 0;
}

static void open_palette(void) {
    g_app.mode = MODE_PALETTE;
    g_app.overlay_active = true;
    strcpy(g_app.overlay_prompt, ">");
    g_app.overlay_text[0] = '\0';
    g_app.overlay_len = 0;
    g_app.overlay_cursor = 0;
}

// ===== Main =====
int main(int argc, char *argv[]) {
    // Initialize application state
    memset(&g_app, 0, sizeof(g_app));
    gb_init(&g_app.buf);
    gb_init(&g_app.out.buf);
    pthread_mutex_init(&g_app.out.lock, NULL);
    g_app.tab_width = WOFL_DEFAULT_TAB;
    
    // Load file if provided
    if (argc > 1) {
        load_file(argv[1]);
    }
    
    init_ncurses();
    
    // Main loop
    while (g_running) {
        render_screen();
        int ch = getch();
        handle_key(ch);
    }
    
    cleanup_ncurses();
    gb_free(&g_app.buf);
    gb_free(&g_app.out.buf);
    pthread_mutex_destroy(&g_app.out.lock);
    
    return 0;
}