#ifndef FILE_OPS_H
#define FILE_OPS_H

#include "app.h"

bool load_file(const char *filename);
bool gb_save_to_file(GapBuffer *gb, const char *path, EolMode eol);
void save_file();
bool open_file_dialog(char *selected_path, size_t max_len);
bool save_file_dialog(char *selected_path, size_t max_len);

#endif