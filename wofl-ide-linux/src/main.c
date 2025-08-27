#include "app.h"
#include "gap_buffer.h"
#include "file_ops.h"
#include "language.h"
#include "rendering.h"
#include "input.h"
#include "sdl_utils.h"

AppState g_app = {0};

int main(int argc, char *argv[]) {
    gb_init(&g_app.buf);
    g_app.running = true;
    
    if (argc > 1) {
        if (load_file(argv[1])) {
            g_app.lang = detect_language(argv[1]);
        }
    } else {
        const char *test_content = "# WOFL IDE - SDL2 Version\nprint('Hello from SDL2!')\n\ndef test_function():\n    return 42\n";
        gb_insert(&g_app.buf, test_content, strlen(test_content));
        strcpy(g_app.file_name, "test.py");
    }
    
    if (!init_sdl()) {
        return 1;
    }
    
    SDL_StartTextInput();
    
    printf("WOFL IDE SDL2 - Controls:\n");
    printf("Ctrl+Q - Quit\n");
    printf("Ctrl+O - Open file\n"); 
    printf("Ctrl+F - Find text\n");
    printf("F3 - Find next\n");
    printf("Arrow keys - Move cursor\n");
    
    SDL_Event e;
    while (g_app.running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    g_app.running = false;
                    break;
                case SDL_KEYDOWN:
                    handle_key(e.key.keysym.sym, e.key.keysym.mod);
                    break;
                case SDL_TEXTINPUT:
                    handle_text_input(e.text.text);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        handle_mouse_click(e.button.x, e.button.y);
                    }
                    break;
            }
        }
        
        render_editor();
        SDL_RenderPresent(g_app.renderer);
        SDL_Delay(16); // ~60 FPS
    }
    
    SDL_StopTextInput();
    cleanup();
    return 0;
}