#include "sdl_utils.h"
#include "gap_buffer.h"

bool init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return false;
    }
    
    if (TTF_Init() == -1) {
        printf("TTF init failed: %s\n", TTF_GetError());
        return false;
    }
    
    g_app.window = SDL_CreateWindow("WOFL IDE - SDL2",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    1024, 768,
                                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    if (!g_app.window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    g_app.renderer = SDL_CreateRenderer(g_app.window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_app.renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    g_app.font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14);
    if (!g_app.font) {
        g_app.font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSansMono.ttf", 14);
        if (!g_app.font) {
            g_app.font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 14);
            if (!g_app.font) {
                printf("Font loading failed: %s\n", TTF_GetError());
                return false;
            }
        }
    }
    
    g_app.font_size = 14;
    g_app.line_height = TTF_FontHeight(g_app.font);
    g_app.char_width = 8; // Approximate
    
    return true;
}

void cleanup(void) {
    if (g_app.font) TTF_CloseFont(g_app.font);
    if (g_app.renderer) SDL_DestroyRenderer(g_app.renderer);
    if (g_app.window) SDL_DestroyWindow(g_app.window);
    TTF_Quit();
    SDL_Quit();
    gb_free(&g_app.buf);
}