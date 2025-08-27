#ifndef INPUT_H
#define INPUT_H

#include "app.h"

void handle_key(SDL_Keycode key, Uint16 mod);
void handle_text_input(const char* text);
void handle_mouse_click(int x, int y);

#endif