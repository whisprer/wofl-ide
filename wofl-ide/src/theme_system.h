// ==================== theme_system.h ====================
// Theme configuration and management system

#ifndef WOFL_THEME_SYSTEM_H
#define WOFL_THEME_SYSTEM_H

#include "editor.h"

// Theme configuration structure
typedef struct {
    wchar_t name[64];
    wchar_t author[64];
    
    // Editor colors
    COLORREF bg;              // Background
    COLORREF fg;              // Foreground (default text)
    COLORREF line_bg;         // Current line background
    COLORREF line_num_bg;     // Line number background
    COLORREF line_num_fg;     // Line number foreground
    COLORREF selection_bg;    // Selection background
    COLORREF selection_fg;    // Selection foreground
    COLORREF caret;           // Caret color
    COLORREF status_bg;       // Status bar background
    COLORREF status_fg;       // Status bar foreground
    COLORREF output_bg;       // Output pane background
    COLORREF output_fg;       // Output pane foreground
    
    // Syntax highlighting colors
    COLORREF syntax_keyword;  // Keywords
    COLORREF syntax_ident;    // Identifiers
    COLORREF syntax_number;   // Numbers
    COLORREF syntax_string;   // Strings
    COLORREF syntax_char;     // Character literals
    COLORREF syntax_comment;  // Comments
    COLORREF syntax_punct;    // Punctuation
    COLORREF syntax_preproc;  // Preprocessor directives
    COLORREF syntax_type;     // Type names
    COLORREF syntax_function; // Function names
    COLORREF syntax_operator; // Operators
    COLORREF syntax_error;    // Error highlighting
    
    // UI elements
    COLORREF overlay_bg;      // Command palette background
    COLORREF overlay_fg;      // Command palette foreground
    COLORREF overlay_border;  // Command palette border
    
    // Font settings
    wchar_t font_name[64];
    int font_size;
    int font_weight;          // FW_NORMAL, FW_BOLD, etc.
    bool font_italic;
} ThemeConfig;

// Predefined themes
void theme_load_dark(ThemeConfig *theme);
void theme_load_light(ThemeConfig *theme);
void theme_load_monokai(ThemeConfig *theme);
void theme_load_dracula(ThemeConfig *theme);

// void theme_load_solarized_dark(ThemeConfig *theme);
// void theme_load_solarized_light(ThemeConfig *theme);
// void theme_load_nord(ThemeConfig *theme);

// Theme I/O
bool theme_load_from_file(ThemeConfig *theme, const wchar_t *path);
bool theme_save_to_file(const ThemeConfig *theme, const wchar_t *path);
void theme_apply(AppState *app, const ThemeConfig *theme);

// Theme directory management
bool theme_get_user_dir(wchar_t *path, size_t max_len);
int theme_list_available(wchar_t names[][64], int max_themes);

#endif // WOFL_THEME_SYSTEM_H