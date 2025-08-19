// ==================== editor_buffer.c ====================
// Gap buffer implementation for efficient text editing

#include "editor.h"
#include <stdlib.h>
#include <string.h>

/**
 * Initialize a gap buffer with default capacity
 */
void gb_init(GapBuffer *gb) {
    gb->capacity = WOFL_INITIAL_GAP;
    gb->data = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                                    gb->capacity * sizeof(wchar_t));
    gb->gap_start = 0;
    gb->gap_end = gb->capacity;
    gb->eol_mode = EOL_LF;
    gb->dirty = false;
}

/**
 * Free gap buffer resources
 */
void gb_free(GapBuffer *gb) {
    if (gb->data) {
        HeapFree(GetProcessHeap(), 0, gb->data);
        gb->data = NULL;
    }
    gb->capacity = 0;
    gb->gap_start = gb->gap_end = 0;
}

/**
 * Get the logical length of text (excluding gap)
 */
size_t gb_length(const GapBuffer *gb) {
    return gb->capacity - (gb->gap_end - gb->gap_start);
}

/**
 * Ensure buffer has enough capacity for additional characters
 */
void gb_ensure_capacity(GapBuffer *gb, size_t need) {
    size_t gap_size = gb->gap_end - gb->gap_start;
    if (gap_size >= need) return;
    
    // Calculate new capacity with growth factor
    size_t new_capacity = max_size(gb->capacity * 2, gb->capacity + need);
    wchar_t *new_data = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, 
                                             new_capacity * sizeof(wchar_t));
    
    // Copy data before and after gap
    size_t pre_gap = gb->gap_start;
    size_t post_gap = gb->capacity - gb->gap_end;
    
    if (pre_gap > 0) {
        memcpy(new_data, gb->data, pre_gap * sizeof(wchar_t));
    }
    if (post_gap > 0) {
        memcpy(new_data + (new_capacity - post_gap), 
               gb->data + gb->gap_end, post_gap * sizeof(wchar_t));
    }
    
    HeapFree(GetProcessHeap(), 0, gb->data);
    gb->data = new_data;
    gb->gap_end = new_capacity - post_gap;
    gb->capacity = new_capacity;
}

/**
 * Move gap to specified position
 */
void gb_move_gap(GapBuffer *gb, size_t pos) {
    if (pos == gb->gap_start) return;
    
    if (pos < gb->gap_start) {
        // Move gap left
        size_t n = gb->gap_start - pos;
        memmove(gb->data + gb->gap_end - n, gb->data + pos, n * sizeof(wchar_t));
        gb->gap_start = pos;
        gb->gap_end -= n;
    } else {
        // Move gap right
        size_t n = pos - gb->gap_start;
        memmove(gb->data + gb->gap_start, gb->data + gb->gap_end, n * sizeof(wchar_t));
        gb->gap_start = pos;
        gb->gap_end += n;
    }
}

/**
 * Insert text at current gap position
 */
void gb_insert(GapBuffer *gb, const wchar_t *s, size_t n) {
    if (n == 0) return;
    gb_ensure_capacity(gb, n);
    memcpy(gb->data + gb->gap_start, s, n * sizeof(wchar_t));
    gb->gap_start += n;
    gb->dirty = true;
}

/**
 * Insert single character
 */
void gb_insert_char(GapBuffer *gb, wchar_t ch) {
    gb_insert(gb, &ch, 1);
}

/**
 * Delete range of characters
 */
void gb_delete_range(GapBuffer *gb, size_t pos, size_t n) {
    if (n == 0) return;
    gb_move_gap(gb, pos);
    gb->gap_end = min_size(gb->gap_end + n, gb->capacity);
    gb->dirty = true;
}

/**
 * Get character at logical position
 */
wchar_t gb_char_at(const GapBuffer *gb, size_t pos) {
    if (pos >= gb_length(gb)) return L'\0';
    
    if (pos < gb->gap_start) {
        return gb->data[pos];
    } else {
        return gb->data[pos + (gb->gap_end - gb->gap_start)];
    }
}

/**
 * Copy text range to output buffer
 */
void gb_get_text(const GapBuffer *gb, wchar_t *out, size_t start, size_t len) {
    size_t total_len = gb_length(gb);
    if (start >= total_len) {
        out[0] = L'\0';
        return;
    }
    
    len = min_size(len, total_len - start);
    
    for (size_t i = 0; i < len; i++) {
        out[i] = gb_char_at(gb, start + i);
    }
    out[len] = L'\0';
}

/**
 * Normalize line endings in-place
 */
static void normalize_newlines(wchar_t *text, size_t *len, EolMode *eol) {
    size_t read = 0, write = 0;
    bool has_crlf = false;
    
    while (read < *len) {
        if (text[read] == L'\r') {
            if (read + 1 < *len && text[read + 1] == L'\n') {
                text[write++] = L'\n';
                read += 2;
                has_crlf = true;
            } else {
                text[write++] = L'\n';
                read++;
            }
        } else {
            text[write++] = text[read++];
        }
    }
    
    *len = write;
    *eol = has_crlf ? EOL_CRLF : EOL_LF;
}

/**
 * Load file into gap buffer
 */
 bool gb_load_from_file(GapBuffer *gb, const wchar_t *path, EolMode *eol) {
     gb_load_from_utf8_file(gb, path, eol);
     return true;  // Adjust based on actual implementation
    
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return false;
    
    LARGE_INTEGER size;
    GetFileSizeEx(file, &size);
    
    if (size.QuadPart == 0) {
        CloseHandle(file);
        *eol = EOL_LF;
        return true;
    }
    
    // Read file as UTF-8
    char *utf8_buf = (char*)HeapAlloc(GetProcessHeap(), 0, (size_t)size.QuadPart + 1);
    DWORD bytes_read = 0;
    ReadFile(file, utf8_buf, (DWORD)size.QuadPart, &bytes_read, NULL);
    CloseHandle(file);
    utf8_buf[bytes_read] = '\0';
    
    // Convert UTF-8 to wide char
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_buf, bytes_read, NULL, 0);
    wchar_t *wide_buf = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, 
                                             (wide_len + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8_buf, bytes_read, wide_buf, wide_len);
    wide_buf[wide_len] = L'\0';
    HeapFree(GetProcessHeap(), 0, utf8_buf);
    
    // Normalize newlines
    size_t final_len = wide_len;
    normalize_newlines(wide_buf, &final_len, eol);
    
    // Insert into buffer
    gb_ensure_capacity(gb, final_len);
    memcpy(gb->data, wide_buf, final_len * sizeof(wchar_t));
    gb->gap_start = final_len;
    gb->eol_mode = *eol;
    gb->dirty = false;
    
    HeapFree(GetProcessHeap(), 0, wide_buf);
    return true;
}

/**
 * Save gap buffer to file
 */
bool gb_save_to_file(GapBuffer *gb, const wchar_t *path, EolMode eol) {
    return gb_save_to_utf8_file(gb, path, eol);
   
    // Flatten buffer
    wchar_t *flat = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, 
                                         (len + 1) * sizeof(wchar_t));
    gb_get_text(gb, flat, 0, len);
    
    // Convert newlines if needed
    if (eol == EOL_CRLF) {
        // Count newlines
        size_t newline_count = 0;
        for (size_t i = 0; i < len; i++) {
            if (flat[i] == L'\n') newline_count++;
        }
        
        // Allocate buffer with space for CRLF
        wchar_t *crlf_buf = (wchar_t*)HeapAlloc(GetProcessHeap(), 0,
                                                 (len + newline_count + 1) * sizeof(wchar_t));
        size_t write_pos = 0;
        for (size_t i = 0; i < len; i++) {
            if (flat[i] == L'\n') {
                crlf_buf[write_pos++] = L'\r';
                crlf_buf[write_pos++] = L'\n';
            } else {
                crlf_buf[write_pos++] = flat[i];
            }
        }
        crlf_buf[write_pos] = L'\0';
        
        HeapFree(GetProcessHeap(), 0, flat);
        flat = crlf_buf;
        len = write_pos;
    }
    
    // Convert to UTF-8
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, flat, (int)len, 
                                        NULL, 0, NULL, NULL);
    char *utf8_buf = (char*)HeapAlloc(GetProcessHeap(), 0, utf8_len);
    WideCharToMultiByte(CP_UTF8, 0, flat, (int)len, utf8_buf, utf8_len, NULL, NULL);
    
    // Write to file
    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, NULL,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        HeapFree(GetProcessHeap(), 0, utf8_buf);
        HeapFree(GetProcessHeap(), 0, flat);
        return false;
    }
    
    DWORD bytes_written = 0;
    WriteFile(file, utf8_buf, utf8_len, &bytes_written, NULL);
    CloseHandle(file);
    
    HeapFree(GetProcessHeap(), 0, utf8_buf);
    HeapFree(GetProcessHeap(), 0, flat);
    
    gb->dirty = false;
    return true;
}
