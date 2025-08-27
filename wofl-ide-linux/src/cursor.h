#ifndef CURSOR_H
#define CURSOR_H

#include "app.h"

size_t get_cursor_index(void);
void move_cursor_to_index(size_t target_index);
int get_line_length(int line_num);
void move_cursor_up(void);
void move_cursor_down(void);
void move_cursor_left(void);
void move_cursor_right(void);

#endif