#ifndef FIND_H
#define FIND_H

#include "app.h"

bool find_text_in_buffer(const char* needle, size_t start_pos, bool case_sensitive, size_t* found_pos);
void find_next(void);
void start_find(void);
void handle_find_input(const char* text);
void handle_find_backspace(void);
void finish_find(void);

#endif