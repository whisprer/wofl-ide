#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <dirent.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>

#define WOFL_MAX_PATH     1024
#define WOFL_CMD_MAX      2048
#define WOFL_INITIAL_GAP  4096

typedef enum {
    EOL_LF = 0,
    EOL_CRLF = 1
} EolMode;

typedef enum {
    LANG_NONE = 0, LANG_C, LANG_CPP, LANG_PY, LANG_JS, LANG_SH, LANG_MAX
} Language;

typedef struct {
    char *data;
    size_t capacity;
    size_t gap_start;
    size_t gap_end;
    bool dirty;
} GapBuffer;

typedef struct {
    int line, col;
} Caret;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    bool running;
    
    char file_path[WOFL_MAX_PATH];
    char file_name[WOFL_MAX_PATH];
    Language lang;
    GapBuffer buf;
    Caret caret;
    int scroll_y;
    
    // Find state
    bool find_active;
    char find_text[256];
    int find_cursor;
    size_t last_find_pos;
    bool find_case_sensitive;
    
    // UI state
    bool show_overlay;
    char overlay_text[512];
    
    int font_size;
    int line_height;
    int char_width;
} AppState;

extern AppState g_app;

#endif