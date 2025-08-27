#include "cursor.h"
#include "gap_buffer.h"

size_t get_cursor_index(void) {
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

void move_cursor_up(void) {
    if (g_app.caret.line > 0) {
        g_app.caret.line--;
        int line_len = get_line_length(g_app.caret.line);
        if (g_app.caret.col > line_len) {
            g_app.caret.col = line_len;
        }
    }
}

void move_cursor_down(void) {
    g_app.caret.line++;
    int line_len = get_line_length(g_app.caret.line);
    if (g_app.caret.col > line_len) {
        g_app.caret.col = line_len;
    }
}

void move_cursor_left(void) {
    if (g_app.caret.col > 0) {
        g_app.caret.col--;
    } else if (g_app.caret.line > 0) {
        g_app.caret.line--;
        g_app.caret.col = get_line_length(g_app.caret.line);
    }
}

void move_cursor_right(void) {
    int line_len = get_line_length(g_app.caret.line);
    if (g_app.caret.col < line_len) {
        g_app.caret.col++;
    } else {
        g_app.caret.line++;
        g_app.caret.col = 0;
    }
}