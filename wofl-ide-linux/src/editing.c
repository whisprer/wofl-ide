#include "editing.h"
#include "gap_buffer.h"
#include "cursor.h"

void insert_text_at_cursor(const char *text, size_t len) {
    size_t cursor_index = get_cursor_index();
    size_t buf_len = gb_length(&g_app.buf);
    size_t new_len = buf_len + len;
    
    if (g_app.buf.gap_end - g_app.buf.gap_start < len) {
        size_t new_cap = g_app.buf.capacity;
        while (new_cap - buf_len < len + 100) {
            new_cap *= 2;
        }
        
        char *new_data = malloc(new_cap);
        
        for (size_t i = 0; i < cursor_index; i++) {
            new_data[i] = gb_char_at(&g_app.buf, i);
        }
        
        memcpy(new_data + cursor_index, text, len);
        
        for (size_t i = cursor_index; i < buf_len; i++) {
            new_data[i + len] = gb_char_at(&g_app.buf, i);
        }
        
        free(g_app.buf.data);
        g_app.buf.data = new_data;
        g_app.buf.capacity = new_cap;
        g_app.buf.gap_start = cursor_index + len;
        g_app.buf.gap_end = new_cap - (buf_len - cursor_index);
    } else {
        gb_ensure(&g_app.buf, len);
        memmove(g_app.buf.data + cursor_index + len, 
                g_app.buf.data + cursor_index, 
                buf_len - cursor_index);
        memcpy(g_app.buf.data + cursor_index, text, len);
        g_app.buf.gap_start += len;
    }
    
    move_cursor_to_index(cursor_index + len);
    g_app.buf.dirty = true;
}

void delete_at_cursor(bool forward) {
    size_t cursor_index = get_cursor_index();
    size_t buf_len = gb_length(&g_app.buf);
    
    if (forward) {
        if (cursor_index < buf_len) {
            g_app.buf.gap_start = cursor_index;
            g_app.buf.gap_end = g_app.buf.gap_start + 1;
            if (g_app.buf.gap_end > g_app.buf.capacity) g_app.buf.gap_end = g_app.buf.capacity;
        }
    } else {
        if (cursor_index > 0) {
            move_cursor_to_index(cursor_index - 1);
            delete_at_cursor(true);
        }
    }
    
    g_app.buf.dirty = true;
}