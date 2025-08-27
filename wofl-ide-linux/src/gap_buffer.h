#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#include "app.h"

void gb_init(GapBuffer *gb);
void gb_free(GapBuffer *gb);
size_t gb_length(const GapBuffer *gb);
char gb_char_at(const GapBuffer *gb, size_t pos);
void gb_ensure(GapBuffer *gb, size_t need);
void gb_insert(GapBuffer *gb, const char *text, size_t len);

#endif