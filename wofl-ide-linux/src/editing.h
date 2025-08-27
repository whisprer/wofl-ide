#ifndef EDITING_H
#define EDITING_H

#include "app.h"

void insert_text_at_cursor(const char *text, size_t len);
void delete_at_cursor(bool forward);

#endif