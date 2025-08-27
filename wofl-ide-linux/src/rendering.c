#include "rendering.h"
#include "gap_buffer.h"
#include "cursor.h"
#include "syntax.h"

void render_text(const char *text, int x, int y, SDL_Color color) {
    if (!text || !text[0]) return;
    
    SDL_Surface *surface = TTF_RenderText_Solid(g_app.font, text, color);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(g_app.renderer, surface);
    if (texture) {
        SDL_Rect dest = {x, y, surface->w, surface->h};
        SDL_RenderCopy(g_app.renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

void render_editor() {
    SDL_SetRenderDrawColor(g_app.renderer, 20, 20, 20, 255);
    SDL_RenderClear(g_app.renderer);
    
    // Render text with cursor tracking
    size_t len = gb_length(&g_app.buf);
    char line_buf[1024];
    int line_pos = 0;
    int y = 10;
    int line_num = 0;
    int cursor_x = 10, cursor_y = 10;
    bool cursor_found = false;
    
    for (size_t i = 0; i <= len; i++) {
        char ch = (i < len) ? gb_char_at(&g_app.buf, i) : '\n'; // Force final line render
        
        // Check if this is cursor position
        if (line_num == g_app.caret.line && line_pos == g_app.caret.col && !cursor_found) {
            cursor_x = 10 + (line_pos * g_app.char_width);
            cursor_y = y;
            cursor_found = true;
        }
        
        if (ch == '\n' || line_pos >= 1023 || i == len) {
            line_buf[line_pos] = '\0';
            
            if (line_buf[0] != '\0' || i == len) { // Render line if not empty or if it's the end
                // Advanced syntax highlighting
                SyntaxToken tokens[64];
                int token_count = 0;
                highlight_line(line_buf, line_pos, g_app.lang, tokens, &token_count);
                
                // Render each token with its specific color
                int render_x = 10;
                for (int t = 0; t < token_count; t++) {
                    SyntaxToken *token = &tokens[t];
                    char token_text[256];
                    int copy_len = (token->length < 255) ? token->length : 255;
                    strncpy(token_text, line_buf + token->start, copy_len);
                    token_text[copy_len] = '\0';
                    
                    render_text(token_text, render_x, y, token->color);
                    render_x += copy_len * g_app.char_width;
                }
                
                // Fallback for unhighlighted content
                if (token_count == 0) {
                    SDL_Color default_color = {220, 220, 220, 255};
                    render_text(line_buf, 10, y, default_color);
                }
            }
            
            y += g_app.line_height;
            line_pos = 0;
            line_num++;
            
            if (y > 700) break; // Don't render off screen
        } else {
            line_buf[line_pos++] = ch;
        }
    }
    
    // Draw cursor
    if (cursor_found || (g_app.caret.line == line_num - 1)) {
        if (!cursor_found) {
            // Cursor is at end of last line
            cursor_x = 10 + (g_app.caret.col * g_app.char_width);
            cursor_y = y - g_app.line_height;
        }
        
        SDL_SetRenderDrawColor(g_app.renderer, 255, 255, 255, 255); // White cursor
        SDL_Rect cursor_rect = {cursor_x, cursor_y, 2, g_app.line_height};
        SDL_RenderFillRect(g_app.renderer, &cursor_rect);
    }
    
    // Status bar
    char status[512];
    const char *display_name = g_app.file_name[0] ? g_app.file_name : "untitled";
    snprintf(status, sizeof(status), "%.200s%s | Ln %d, Col %d | SDL2", 
             display_name,
             g_app.buf.dirty ? "*" : "",
             g_app.caret.line + 1, 
             g_app.caret.col + 1);
    render_text(status, 10, 740, (SDL_Color){180, 180, 180, 255});
    
    // Overlay for find/command palette
    if (g_app.show_overlay) {
        // Draw semi-transparent background
        SDL_SetRenderDrawColor(g_app.renderer, 0, 0, 50, 200);
        SDL_Rect overlay_bg = {0, 0, 1024, 30};
        SDL_RenderFillRect(g_app.renderer, &overlay_bg);
        
        // Draw overlay text
        render_text(g_app.overlay_text, 10, 5, (SDL_Color){255, 255, 255, 255});
        
        // Draw cursor if in find mode
        if (g_app.find_active) {
            int cursor_x = 10 + (6 * g_app.char_width) + (g_app.find_cursor * g_app.char_width);
            SDL_SetRenderDrawColor(g_app.renderer, 255, 255, 255, 255);
            SDL_Rect cursor_rect = {cursor_x, 5, 2, g_app.line_height};
            SDL_RenderFillRect(g_app.renderer, &cursor_rect);
        }
    }
}