#include "gap_buffer.h"

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