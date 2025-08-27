#include "find.h"
#include "gap_buffer.h"
#include "cursor.h"

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

void find_next(void) {
    if (!g_app.find_text[0]) return;
    
    size_t start_pos = g_app.last_find_pos + 1;
    size_t found_pos;
    
    if (find_text_in_buffer(g_app.find_text, start_pos, g_app.find_case_sensitive, &found_pos)) {
        move_cursor_to_index(found_pos);
        g_app.last_find_pos = found_pos;
        
        snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                "Found: %s at position %zu", g_app.find_text, found_pos);
        g_app.show_overlay = true;
    } else {
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

void start_find(void) {
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

void handle_find_backspace(void) {
    if (g_app.find_cursor > 0) {
        g_app.find_text[--g_app.find_cursor] = '\0';
        snprintf(g_app.overlay_text, sizeof(g_app.overlay_text), 
                "Find: %s", g_app.find_text);
    }
}

void finish_find(void) {
    g_app.find_active = false;
    g_app.show_overlay = false;
    if (g_app.find_text[0]) {
        size_t cursor_pos = get_cursor_index();
        size_t found_pos;
        if (find_text_in_buffer(g_app.find_text, cursor_pos, g_app.find_case_sensitive, &found_pos)) {
            move_cursor_to_index(found_pos);
            g_app.last_find_pos = found_pos;
        }
    }
}