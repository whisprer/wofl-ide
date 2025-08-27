// ==================== main_sdl2.c ====================
// WOFL IDE - SDL2 cross-platform port from Windows version

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <dirent.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>

#define WOFL_MAX_PATH     1024
#define WOFL_CMD_MAX      2048
#define WOFL_INITIAL_GAP  4096

typedef enum {
    EOL_LF = 0,
    EOL_CRLF = 1
} EolMode;

typedef enum {
    LANG_NONE = 0, LANG_C, LANG_CPP, LANG_PY, LANG_JS, LANG_SH, LANG_MAX
} Language;

typedef struct {
    char *data;
    size_t capacity;
    size_t gap_start;
    size_t gap_end;
    bool dirty;
} GapBuffer;

typedef struct {
    int line, col;
} Caret;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    bool running;
    
    char file_path[WOFL_MAX_PATH];
    char file_name[WOFL_MAX_PATH];
    Language lang;
    GapBuffer buf;
    Caret caret;
    int scroll_y;
    
    // Find state
    bool find_active;
    char find_text[256];
    int find_cursor;
    size_t last_find_pos;
    bool find_case_sensitive;
    
    // UI state
    bool show_overlay;
    char overlay_text[512];
    
    int font_size;
    int line_height;
    int char_width;
} AppState;

static AppState g_app = {0};

// Gap buffer implementation
void gb_init(GapBuffer *gb) {
    gb->capacity = WOFL_INITIAL_GAP;
    gb->data = malloc(gb->capacity);
    gb->gap_start = 0;
    gb->gap_end = gb->capacity;
    gb->dirty = false;
}

void gb_free(GapBuffer *gb) {
    if (gb->data) {
        free(gb->data);
        gb->data = NULL;
    }
}

size_t gb_length(const GapBuffer *gb) {
    return gb->capacity - (gb->gap_end - gb->gap_start);
}

char gb_char_at(const GapBuffer *gb, size_t pos) {
    if (pos >= gb_length(gb)) return '\0';
    if (pos < gb->gap_start) {
        return gb->data[pos];
    } else {
        return gb->data[pos + (gb->gap_end - gb->gap_start)];
    }
}

void gb_ensure(GapBuffer *gb, size_t need) {
    size_t gap = gb->gap_end - gb->gap_start;
    if (gap >= need) return;
    
    size_t new_cap = gb->capacity * 2;
    while (new_cap - gb_length(gb) < need + 100) {
        new_cap *= 2;
    }
    
    char *new_data = malloc(new_cap);
    
    // Copy pre-gap data
    memcpy(new_data, gb->data, gb->gap_start);
    // Copy post-gap data to end of new buffer
    memcpy(new_data + (new_cap - (gb->capacity - gb->gap_end)),
           gb->data + gb->gap_end,
           gb->capacity - gb->gap_end);
    
    free(gb->data);
    gb->data = new_data;
    gb->gap_end = new_cap - (gb->capacity - gb->gap_end);
    gb->capacity = new_cap;
}

void gb_insert(GapBuffer *gb, const char *text, size_t len) {
    if (gb->gap_end - gb->gap_start < len) {
        gb_ensure(gb, len);
    }
    
    memcpy(gb->data + gb->gap_start, text, len);
    gb->gap_start += len;
    gb->dirty = true;
}

// File operations
bool load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    gb_free(&g_app.buf);
    gb_init(&g_app.buf);
    
    char buffer[4096];
    size_t read_size;
    while ((read_size = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        gb_insert(&g_app.buf, buffer, read_size);
    }
    
    fclose(f);
    strcpy(g_app.file_path, filename);
    
    const char *name = strrchr(filename, '/');
    strcpy(g_app.file_name, name ? name + 1 : filename);
    
    g_app.buf.dirty = false;
    return true;
}

bool gb_save_to_file(GapBuffer *gb, const char *path, EolMode eol) {
    FILE *f = fopen(path, "w");
    if (!f) return false;

    const size_t len = gb_length(gb);
    for (size_t i = 0; i < len; i++) {
        char ch = gb_char_at(gb, i);
        if (ch == '\n' && eol == EOL_CRLF) {
            fputc('\r', f);
        }
        fputc(ch, f);
    }

    fclose(f);
    gb->dirty = false;
    return true;
}

// File save functionality
void save_file() {
    if (!g_app.file_path[0]) {
        // No filename - use default
        strcpy(g_app.file_path, "untitled.txt");
        strcpy(g_app.file_name, "untitled.txt");
    }
    
    if (gb_save_to_file(&g_app.buf, g_app.file_path, EOL_LF)) {
        printf("Saved: %s\n", g_app.file_path);
        g_app.buf.dirty = false;
    } else {
        printf("Save failed: %s\n", g_app.file_path);
    }
}

// Language detection
Language detect_language(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return LANG_NONE;
    
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) return LANG_C;
    if (strcmp(ext, ".py") == 0) return LANG_PY;
    if (strcmp(ext, ".js") == 0) return LANG_JS;
    if (strcmp(ext, ".sh") == 0) return LANG_SH;
    
    return LANG_NONE;
}

// Text rendering
void render_text(const char *text, int x, int y, SDL_Color color) {
    if (!text || !*text) return;
    
    SDL_Surface *surface = TTF_RenderText_Solid(g_app.font, text, color);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(g_app.renderer, surface);
    if (texture) {
        SDL_Rect dest = {x, y, surface->w, surface->h};
        SDL_RenderCopy(g_app.renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

// Main rendering
void render_editor() {
    SDL_SetRenderDrawColor(g_app.renderer, 20, 20, 20, 255);
    SDL_RenderClear(g_app.renderer);
    
    // Simple text display
    size_t len = gb_length(&g_app.buf);
    char line_buf[1024];
    int line_pos = 0;
    int y = 10;
    int line_num = 0;
    
    for (size_t i = 0; i < len; i++) {
        char ch = gb_char_at(&g_app.buf, i);
        
        if (ch == '\n' || line_pos >= 1023) {
            line_buf[line_pos] = '\0';
            
            // Simple syntax coloring
            SDL_Color color = {220, 220, 220, 255}; // Default white
            if (strstr(line_buf, "def ") || strstr(line_buf, "import ")) {
                color = (SDL_Color){100, 150, 255, 255}; // Blue for keywords
            }
            
            render_text(line_buf, 10, y, color);
            y += g_app.line_height;
            line_pos = 0;
            line_num++;
            
            if (y > 700) break; // Don't render off screen
        } else {
            line_buf[line_pos++] = ch;
        }
    }
    
    // Status bar
    char status[256];
    snprintf(status, sizeof(status), "%s | Lines: %d", 
             g_app.file_name[0] ? g_app.file_name : "untitled", line_num);
    render_text(status, 10, 740, (SDL_Color){180, 180, 180, 255});
}

// Cursor and text position utilities
size_t get_cursor_index() {
    size_t len = gb_length(&g_app.buf);
    size_t index = 0;
    int current_line = 0;
    int current_col = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (current_line == g_app.caret.line && current_col == g_app.caret.col) {
            return i;
        }
        
        if (gb_char_at(&g_app.buf, i) == '\n') {
            current_line++;
            current_col = 0;
        } else {
            current_col++;
        }
    }
    
    return len;  // End of buffer
}

void move_cursor_to_index(size_t target_index) {
    size_t len = gb_length(&g_app.buf);
    if (target_index > len) target_index = len;
    
    int line = 0, col = 0;
    for (size_t i = 0; i < target_index; i++) {
        if (gb_char_at(&g_app.buf, i) == '\n') {
            line++;
            col = 0;
        } else {
            col++;
        }
    }
    
    g_app.caret.line = line;
    g_app.caret.col = col;
}

int get_line_length(int line_num) {
    size_t len = gb_length(&g_app.buf);
    int current_line = 0;
    int line_length = 0;
    
    for (size_t i = 0; i < len; i++) {
        char ch = gb_char_at(&g_app.buf, i);
        if (current_line == line_num) {
            if (ch == '\n') break;
            line_length++;
        } else if (ch == '\n') {
            current_line++;
        }
    }
    
    return line_length;
}

void move_cursor_up() {
    if (g_app.caret.line > 0) {
        g_app.caret.line--;
        int line_len = get_line_length(g_app.caret.line);
        if (g_app.caret.col > line_len) {
            g_app.caret.col = line_len;
        }
    }
}

void move_cursor_down() {
    g_app.caret.line++;
    int line_len = get_line_length(g_app.caret.line);
    if (g_app.caret.col > line_len) {
        g_app.caret.col = line_len;
    }
    // Note: we don't check if we're past end of file - that's handled in rendering
}

void move_cursor_left() {
    if (g_app.caret.col > 0) {
        g_app.caret.col--;
    } else if (g_app.caret.line > 0) {
        g_app.caret.line--;
        g_app.caret.col = get_line_length(g_app.caret.line);
    }
}

void move_cursor_right() {
    int line_len = get_line_length(g_app.caret.line);
    if (g_app.caret.col < line_len) {
        g_app.caret.col++;
    } else {
        // Move to start of next line
        g_app.caret.line++;
        g_app.caret.col = 0;
    }
}

// Proper text insertion at cursor
void insert_text_at_cursor(const char *text, size_t len) {
    size_t cursor_index = get_cursor_index();
    
    // Move gap to cursor position and insert
    char *old_data = g_app.buf.data;
    size_t old_gap_start = g_app.buf.gap_start;
    size_t old_gap_end = g_app.buf.gap_end;
    
    // Simple approach: rebuild buffer with insertion
    size_t buf_len = gb_length(&g_app.buf);
    size_t new_len = buf_len + len;
    
    if (g_app.buf.gap_end - g_app.buf.gap_start < len) {
        // Need more space
        size_t new_cap = g_app.buf.capacity;
        while (new_cap - (buf_len) < len + 100) {
            new_cap *= 2;
        }
        
        char *new_data = malloc(new_cap);
        
        // Copy pre-insertion data
        for (size_t i = 0; i < cursor_index; i++) {
            new_data[i] = gb_char_at(&g_app.buf, i);
        }
        
        // Copy inserted text
        memcpy(new_data + cursor_index, text, len);
        
        // Copy post-insertion data
        for (size_t i = cursor_index; i < buf_len; i++) {
            new_data[i + len] = gb_char_at(&g_app.buf, i);
        }
        
        free(g_app.buf.data);
        g_app.buf.data = new_data;
        g_app.buf.capacity = new_cap;
        g_app.buf.gap_start = cursor_index + len;
        g_app.buf.gap_end = new_cap - (buf_len - cursor_index);
    } else {
        // Have space, use proper gap buffer insertion
        gb_ensure(&g_app.buf, len);
        // This is simplified - proper gap buffer would move gap to cursor_index first
        memmove(g_app.buf.data + cursor_index + len, 
                g_app.buf.data + cursor_index, 
                buf_len - cursor_index);
        memcpy(g_app.buf.data + cursor_index, text, len);
        g_app.buf.gap_start += len;
    }
    
    // Update cursor position
    move_cursor_to_index(cursor_index + len);
    g_app.buf.dirty = true;
}

// Find functionality
bool find_text_in_buffer(const char* needle, size_t start_pos, bool case_sensitive, size_t* found_pos) {
    if (!needle || !needle[0]) return false;
    
    size_t buf_len = gb_length(&g_app.buf);
    size_t needle_len = strlen(needle);
    
    for (size_t i = start_pos; i <= buf_len - needle_len; i++) {
        bool match = true;
        for (size_t j = 0; j < needle_len; j++) {
            char buf_char = gb_char_at(&g_app.buf, i + j);
            char needle_char = needle[j];
            
            if (!case_sensitive) {
                buf_char = tolower(buf_char);
                needle_char = tolower(needle_char);
            }
            
            if (buf_char != needle_char) {
                match = false;
                break;
            }
        }
        
        if (match) {
            *found_pos = i;
            return true;
        }
    }
    
    return false;
}

void find_next() {
    if (!g_app.find_text[0]) return;
    
    size_t start_pos = g_app.last_find_pos + 1;
    size_t found_pos;
    
    if (find_text_in_buffer(g_app.find_text, start_pos, g_app.find_case_sensitive, &found_pos)) {
        move_cursor_to_index(found_pos);
        g_app.last_find_pos = found_pos;
        
        // Show found text info
        snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                "Found: %s at position %zu", g_app.find_text, found_pos);
        g_app.show_overlay = true;
    } else {
        // Wrap around search
        if (find_text_in_buffer(g_app.find_text, 0, g_app.find_case_sensitive, &found_pos)) {
            move_cursor_to_index(found_pos);
            g_app.last_find_pos = found_pos;
            snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                    "Found: %s (wrapped)", g_app.find_text);
            g_app.show_overlay = true;
        } else {
            snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                    "Not found: %s", g_app.find_text);
            g_app.show_overlay = true;
        }
    }
}

void start_find() {
    g_app.find_active = true;
    g_app.find_text[0] = '\0';
    g_app.find_cursor = 0;
    g_app.last_find_pos = 0;
    strcpy(g_app.overlay_text, "Find: ");
    g_app.show_overlay = true;
}

void handle_find_input(const char* text) {
    size_t len = strlen(g_app.find_text);
    if (len < sizeof(g_app.find_text) - 1) {
        strncat(g_app.find_text, text, sizeof(g_app.find_text) - len - 1);
        g_app.find_cursor = strlen(g_app.find_text);
        
        snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                "Find: %s", g_app.find_text);
    }
}

void handle_find_backspace() {
    if (g_app.find_cursor > 0) {
        g_app.find_text[--g_app.find_cursor] = '\0';
        snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                "Find: %s", g_app.find_text);
    }
}

void finish_find() {
    g_app.find_active = false;
    g_app.show_overlay = false;
    if (g_app.find_text[0]) {
        // Perform the search
        size_t cursor_pos = get_cursor_index();
        size_t found_pos;
        if (find_text_in_buffer(g_app.find_text, cursor_pos, g_app.find_case_sensitive, &found_pos)) {
            move_cursor_to_index(found_pos);
            g_app.last_find_pos = found_pos;
        }
    }
}
    size_t cursor_index = get_cursor_index();
    size_t buf_len = gb_length(&g_app.buf);
    
    if (forward) {
        if (cursor_index < buf_len) {
            // Delete character at cursor
            for (size_t i = cursor_index; i < buf_len - 1; i++) {
                // This is a hack - should use proper gap buffer deletion
                char next_char = gb_char_at(&g_app.buf, i + 1);
                // Can't easily modify gap buffer char by char, need proper implementation
            }
            g_app.buf.gap_start = cursor_index;
            g_app.buf.gap_end = g_app.buf.gap_start + 1;
            if (g_app.buf.gap_end > g_app.buf.capacity) g_app.buf.gap_end = g_app.buf.capacity;
        }
    } else {
        // Backspace
        if (cursor_index > 0) {
            move_cursor_to_index(cursor_index - 1);
            delete_at_cursor(true);
        }
    }
    
    g_app.buf.dirty = true;
}

// Input handling
void handle_key(SDL_Keycode key, Uint16 mod) {
    // Handle find mode input first
    if (g_app.find_active) {
        switch (key) {
            case SDLK_ESCAPE:
                g_app.find_active = false;
                g_app.show_overlay = false;
                return;
            case SDLK_RETURN:
                finish_find();
                return;
            case SDLK_BACKSPACE:
                handle_find_backspace();
                return;
            case SDLK_F3:
                if (g_app.find_text[0]) {
                    find_next();
                }
                return;
        }
        return; // In find mode, only handle find-specific keys
    }
    
    if (mod & KMOD_CTRL) {
        switch (key) {
            case SDLK_q:
                g_app.running = false;
                break;
            case SDLK_o: {
                // Simple file open - for now just try test.py
                if (load_file("test.py")) {
                    g_app.lang = detect_language(g_app.file_path);
                    g_app.caret.line = 0;
                    g_app.caret.col = 0;
                }
                break;
            }
            case SDLK_s:
                save_file();
                break;
            case SDLK_f:
                start_find();
                break;
            case SDLK_p:
                // Command palette - placeholder
                strcpy(g_app.overlay_text, "Command palette: Ctrl+O (open), Ctrl+S (save), Ctrl+F (find)");
                g_app.show_overlay = true;
                break;
        }
    } else {
        // Handle regular keys
        switch (key) {
            case SDLK_UP:
                move_cursor_up();
                g_app.show_overlay = false;
                break;
            case SDLK_DOWN:
                move_cursor_down();
                g_app.show_overlay = false;
                break;
            case SDLK_LEFT:
                move_cursor_left();
                g_app.show_overlay = false;
                break;
            case SDLK_RIGHT:
                move_cursor_right();
                g_app.show_overlay = false;
                break;
            case SDLK_RETURN:
                insert_text_at_cursor("\n", 1);
                g_app.show_overlay = false;
                break;
            case SDLK_BACKSPACE:
                delete_at_cursor(false);
                g_app.show_overlay = false;
                break;
            case SDLK_DELETE:
                delete_at_cursor(true);
                g_app.show_overlay = false;
                break;
            case SDLK_TAB:
                insert_text_at_cursor("    ", 4);  // 4 spaces for tab
                g_app.show_overlay = false;
                break;
            case SDLK_F3:
                if (g_app.find_text[0]) {
                    find_next();
                }
                break;
            case SDLK_ESCAPE:
                g_app.show_overlay = false;
                break;
        }
    }
}

// Handle text input events
void handle_text_input(const char* text) {
    if (g_app.find_active) {
        handle_find_input(text);
    } else if (text && text[0] && text[0] != '\b' && text[0] != '\n' && text[0] != '\t') {
        insert_text_at_cursor(text, strlen(text));
        g_app.show_overlay = false;
    }
}

// File save functionality
void save_file() {
    if (!g_app.file_path[0]) {
        // No filename - use default
        strcpy(g_app.file_path, "untitled.txt");
        strcpy(g_app.file_name, "untitled.txt");
    }
    
    if (gb_save_to_file(&g_app.buf, g_app.file_path, EOL_LF)) {
        printf("Saved: %s\n", g_app.file_path);
        g_app.buf.dirty = false;
    } else {
        printf("Save failed: %s\n", g_app.file_path);
    }
}

// SDL2 initialization
bool init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return false;
    }
    
    if (TTF_Init() == -1) {
        printf("TTF init failed: %s\n", TTF_GetError());
        return false;
    }
    
    g_app.window = SDL_CreateWindow("WOFL IDE - SDL2",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    1024, 768,
                                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    if (!g_app.window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    g_app.renderer = SDL_CreateRenderer(g_app.window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_app.renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    // Try to load a monospace font
    g_app.font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14);
    if (!g_app.font) {
        g_app.font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSansMono.ttf", 14);
        if (!g_app.font) {
            g_app.font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 14);
            if (!g_app.font) {
                printf("Font loading failed: %s\n", TTF_GetError());
                return false;
            }
        }
    }
    
    // Get font metrics
    g_app.font_size = 14;
    g_app.line_height = TTF_FontHeight(g_app.font);
    g_app.char_width = 8; // Approximate
    
    return true;
}

void cleanup() {
    if (g_app.font) TTF_CloseFont(g_app.font);
    if (g_app.renderer) SDL_DestroyRenderer(g_app.renderer);
    if (g_app.window) SDL_DestroyWindow(g_app.window);
    TTF_Quit();
    SDL_Quit();
    gb_free(&g_app.buf);
}

int main(int argc, char *argv[]) {
    // Initialize
    gb_init(&g_app.buf);
    g_app.running = true;
    
    // Load file if provided
    if (argc > 1) {
        if (load_file(argv[1])) {
            g_app.lang = detect_language(argv[1]);
        }
    } else {
        // Default content for testing
        const char *test_content = "# WOFL IDE - SDL2 Version\nprint('Hello from SDL2!')\n\ndef test_function():\n    return 42\n";
        gb_insert(&g_app.buf, test_content, strlen(test_content));
        strcpy(g_app.file_name, "test.py");
    }
    
    if (!init_sdl()) {
        return 1;
    }
    
    // Enable text input
    SDL_StartTextInput();
    
    printf("WOFL IDE SDL2 - Controls:\n");
    printf("Ctrl+Q - Quit\n");
    printf("Ctrl+O - Open test.py\n");
    printf("Type to add text, Backspace to delete, Enter for newline\n");
    
    // Main loop
    SDL_Event e;
    while (g_app.running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    g_app.running = false;
                    break;
                    
                case SDL_KEYDOWN:
                    handle_key(e.key.keysym.sym, e.key.keysym.mod);
                    break;
                    
                case SDL_TEXTINPUT:
                    handle_text_input(e.text.text);
                    break;
            }
        }
        
        render_editor();
        SDL_RenderPresent(g_app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
    
    SDL_StopTextInput();
    
    cleanup();
    return 0;
}