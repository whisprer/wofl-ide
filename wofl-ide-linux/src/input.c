#include "input.h"
#include "file_ops.h"
#include "language.h"
#include "find.h"
#include "cursor.h"
#include "editing.h"
#include "gap_buffer.h"

void handle_key(SDL_Keycode key, Uint16 mod) {
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
        return;
    }
    
    if (mod & KMOD_CTRL) {
        switch (key) {
            case SDLK_q:
                g_app.running = false;
                break;
            case SDLK_o: {
                char file_path[WOFL_MAX_PATH];
                if (open_file_dialog(file_path, sizeof(file_path))) {
                    if (load_file(file_path)) {
                        g_app.lang = detect_language(file_path);
                        g_app.caret.line = 0;
                        g_app.caret.col = 0;
                        snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                                "Opened: %s", g_app.file_name);
                        g_app.show_overlay = true;
                    }
                }
                break;
            }
            case SDLK_s:
                if (mod & KMOD_SHIFT) {
                    // Save As
                    char file_path[WOFL_MAX_PATH];
                    if (save_file_dialog(file_path, sizeof(file_path))) {
                        strcpy(g_app.file_path, file_path);
                        const char *name = strrchr(file_path, '/');
                        strcpy(g_app.file_name, name ? name + 1 : file_path);
                        save_file();
                    }
                } else {
                    // Regular save
                    save_file();
                }
                break;
            case SDLK_f:
                start_find();
                break;
            case SDLK_p:
                strcpy(g_app.overlay_text, "Command palette: Ctrl+O (open), Ctrl+S (save), Ctrl+F (find)");
                g_app.show_overlay = true;
                break;
        }
    } else {
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
                insert_text_at_cursor("    ", 4);
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

void handle_mouse_click(int x, int y) {
    // Don't handle clicks in overlay area
    if (g_app.show_overlay && y < 30) {
        return;
    }
    
    // Convert screen coordinates to text position
    int clicked_line = (y - 10) / g_app.line_height;
    int clicked_col = (x - 10) / g_app.char_width;
    
    if (clicked_line < 0) clicked_line = 0;
    if (clicked_col < 0) clicked_col = 0;
    
    // Find the actual line in the buffer
    size_t buf_len = gb_length(&g_app.buf);
    int current_line = 0;
    int line_start_pos = 0;
    int line_length = 0;
    
    // Scan through buffer to find the clicked line
    for (size_t i = 0; i < buf_len; i++) {
        char ch = gb_char_at(&g_app.buf, i);
        
        if (current_line == clicked_line) {
            if (ch == '\n') {
                line_length = i - line_start_pos;
                break;
            }
        } else if (ch == '\n') {
            current_line++;
            if (current_line == clicked_line) {
                line_start_pos = i + 1;
            }
        }
    }
    
    // Handle click past end of buffer
    if (current_line < clicked_line) {
        // Clicked below last line - move to end of buffer
        g_app.caret.line = current_line;
        g_app.caret.col = line_length;
    } else {
        // Clicked on valid line
        g_app.caret.line = clicked_line;
        g_app.caret.col = clicked_col;
        
        // Clamp column to line length
        int actual_line_len = get_line_length(clicked_line);
        if (g_app.caret.col > actual_line_len) {
            g_app.caret.col = actual_line_len;
        }
    }
    
    // Clear overlay when clicking in editor
    g_app.show_overlay = false;
}

void handle_text_input(const char* text) {
    if (g_app.find_active) {
        handle_find_input(text);
    } else if (text && text[0] && text[0] != '\b' && text[0] != '\n' && text[0] != '\t') {
        insert_text_at_cursor(text, strlen(text));
        g_app.show_overlay = false;
    }
}