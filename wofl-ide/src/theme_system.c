// ==================== theme_system.c ====================
// Theme system implementation

#include "theme_system.h"
#include <shlobj.h>
#include <stdio.h>

/**
 * Load Dark theme (default)
 */
 void theme_load_dark(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Dark");
    wcscpy_s(theme->author, 64, L"WOFL");
    
    // Editor colors
    theme->bg = RGB(30, 30, 30);
    theme->fg = RGB(212, 212, 212);
    theme->line_bg = RGB(37, 37, 38);
    theme->line_num_bg = RGB(30, 30, 30);
    theme->line_num_fg = RGB(133, 133, 133);
    theme->selection_bg = RGB(38, 79, 120);
    theme->selection_fg = RGB(255, 255, 255);
    theme->caret = RGB(255, 255, 255);
    theme->status_bg = RGB(0, 122, 204);
    theme->status_fg = RGB(255, 255, 255);
    theme->output_bg = RGB(22, 22, 22);
    theme->output_fg = RGB(204, 204, 204);
    
    // Syntax colors
    theme->syntax_keyword = RGB(86, 156, 214);
    theme->syntax_ident = RGB(212, 212, 212);
    theme->syntax_number = RGB(181, 206, 168);
    theme->syntax_string = RGB(206, 145, 120);
    theme->syntax_char = RGB(206, 145, 120);
    theme->syntax_comment = RGB(106, 153, 85);
    theme->syntax_punct = RGB(212, 212, 212);
    theme->syntax_preproc = RGB(155, 155, 155);
    theme->syntax_type = RGB(78, 201, 176);
    theme->syntax_function = RGB(220, 220, 170);
    theme->syntax_operator = RGB(212, 212, 212);
    theme->syntax_error = RGB(244, 71, 71);
    
    // UI elements
    theme->overlay_bg = RGB(37, 37, 38);
    theme->overlay_fg = RGB(212, 212, 212);
    theme->overlay_border = RGB(0, 122, 204);
    
    // Font
    wcscpy_s(theme->font_name, 64, L"Consolas");
    theme->font_size = 14;
    theme->font_weight = FW_NORMAL;
    theme->font_italic = false;
}

/**
 * Load Light theme
 */
void theme_load_light(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Light");
    wcscpy_s(theme->author, 64, L"WOFL");
    
    // Editor colors
    theme->bg = RGB(255, 255, 255);
    theme->fg = RGB(0, 0, 0);
    theme->line_bg = RGB(248, 248, 248);
    theme->line_num_bg = RGB(240, 240, 240);
    theme->line_num_fg = RGB(133, 133, 133);
    theme->selection_bg = RGB(173, 214, 255);
    theme->selection_fg = RGB(0, 0, 0);
    theme->caret = RGB(0, 0, 0);
    theme->status_bg = RGB(0, 122, 204);
    theme->status_fg = RGB(255, 255, 255);
    theme->output_bg = RGB(248, 248, 248);
    theme->output_fg = RGB(0, 0, 0);
    
    // Syntax colors
    theme->syntax_keyword = RGB(0, 0, 255);
    theme->syntax_ident = RGB(0, 0, 0);
    theme->syntax_number = RGB(9, 136, 90);
    theme->syntax_string = RGB(163, 21, 21);
    theme->syntax_char = RGB(163, 21, 21);
    theme->syntax_comment = RGB(0, 128, 0);
    theme->syntax_punct = RGB(0, 0, 0);
    theme->syntax_preproc = RGB(111, 0, 138);
    theme->syntax_type = RGB(43, 145, 175);
    theme->syntax_function = RGB(121, 94, 38);
    theme->syntax_operator = RGB(0, 0, 0);
    theme->syntax_error = RGB(255, 0, 0);
    
    // UI elements
    theme->overlay_bg = RGB(248, 248, 248);
    theme->overlay_fg = RGB(0, 0, 0);
    theme->overlay_border = RGB(0, 122, 204);
    
    // Font
    wcscpy_s(theme->font_name, 64, L"Consolas");
    theme->font_size = 14;
    theme->font_weight = FW_NORMAL;
    theme->font_italic = false;
}

/**
 * Load Monokai theme
 */
void theme_load_monokai(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Monokai");
    wcscpy_s(theme->author, 64, L"VS CODE");
    
    theme->bg = RGB(39, 40, 34);
    theme->fg = RGB(248, 248, 242);
    theme->selection_bg = RGB(73, 72, 62);
    theme->syntax_keyword = RGB(249, 38, 114);
    theme->syntax_string = RGB(230, 219, 116);
    theme->syntax_comment = RGB(117, 113, 94);
    theme->syntax_type = RGB(102, 217, 239);
    theme->syntax_function = RGB(166, 226, 46);
    theme->syntax_number = RGB(174, 129, 255);
    
    // Copy other settings from dark theme as base
    ThemeConfig dark;
    theme_load_dark(&dark);
    theme->line_bg = dark.line_bg;
    theme->line_num_bg = dark.line_num_bg;
    theme->line_num_fg = dark.line_num_fg;
    theme->status_bg = dark.status_bg;
    theme->status_fg = dark.status_fg;
    theme->output_bg = dark.output_bg;
    theme->output_fg = dark.output_fg;
    theme->overlay_bg = dark.overlay_bg;
    theme->overlay_fg = dark.overlay_fg;
    theme->overlay_border = dark.overlay_border;
    theme->caret = dark.caret;
    theme->selection_fg = dark.selection_fg;
    theme->syntax_ident = theme->fg;
    theme->syntax_punct = theme->fg;
    theme->syntax_char = theme->syntax_string;
    theme->syntax_preproc = theme->syntax_keyword;
    theme->syntax_operator = theme->fg;
    theme->syntax_error = RGB(249, 38, 114);
    
    wcscpy_s(theme->font_name, 64, L"Consolas");
    theme->font_size = 14;
    theme->font_weight = FW_NORMAL;
    theme->font_italic = false;
}

/**
 * Load Dracula theme
 */
void theme_load_dracula(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Dracula");
    wcscpy_s(theme->author, 64, L"CLAUDE");
    
    theme->bg = RGB(40, 42, 54);
    theme->fg = RGB(248, 248, 242);
    theme->selection_bg = RGB(68, 71, 90);
    theme->syntax_keyword = RGB(255, 121, 198);
    theme->syntax_string = RGB(241, 250, 140);
    theme->syntax_comment = RGB(98, 114, 164);
    theme->syntax_type = RGB(139, 233, 253);
    theme->syntax_function = RGB(80, 250, 123);
    theme->syntax_number = RGB(189, 147, 249);
    
    // Fill in remaining colors
    theme->line_bg = RGB(44, 46, 60);
    theme->line_num_bg = theme->bg;
    theme->line_num_fg = RGB(98, 114, 164);
    theme->status_bg = RGB(68, 71, 90);
    theme->status_fg = theme->fg;
    theme->output_bg = RGB(33, 34, 44);
    theme->output_fg = theme->fg;
    theme->overlay_bg = RGB(44, 46, 60);
    theme->overlay_fg = theme->fg;
    theme->overlay_border = RGB(255, 121, 198);
    theme->caret = theme->fg;
    theme->selection_fg = theme->fg;
    theme->syntax_ident = theme->fg;
    theme->syntax_punct = theme->fg;
    theme->syntax_char = theme->syntax_string;
    theme->syntax_preproc = RGB(255, 184, 108);
    theme->syntax_operator = RGB(255, 121, 198);
    theme->syntax_error = RGB(255, 85, 85);
    
    wcscpy_s(theme->font_name, 64, L"Consolas");
    theme->font_size = 14;
    theme->font_weight = FW_NORMAL;
    theme->font_italic = false;
}

/**
 * Get user theme directory
 */
 bool theme_get_user_dir(wchar_t *path, size_t max_len) {
    // Get AppData/Roaming directory
    wchar_t appdata[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        return false;
    }
    
    // Construct theme directory path
    swprintf_s(path, max_len, L"%ls\\WoflIDE\\themes", appdata);
    
    // Create directory if it doesn't exist
    CreateDirectoryW(path, NULL);
    CreateDirectoryW(L"WoflIDE", NULL);
    
    return true;
}

/**
 * Load theme from INI file
 */
bool theme_load_from_file(ThemeConfig *theme, const wchar_t *path) {
    FILE *file;
    if (_wfopen_s(&file, path, L"r") != 0 || !file) {
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        // Parse key=value pairs
        char key[64], value[192];
        if (sscanf_s(line, "%63[^=]=%191[^\n]", key, sizeof(key), value, sizeof(value)) == 2) {
            // Convert to wide char for comparison
            wchar_t wkey[64], wvalue[192];
            MultiByteToWideChar(CP_UTF8, 0, key, -1, wkey, 64);
            MultiByteToWideChar(CP_UTF8, 0, value, -1, wvalue, 192);
            
            // Parse color values (format: R,G,B)
            int r, g, b;
            if (swscanf_s(wvalue, L"%d,%d,%d", &r, &g, &b) == 3) {
                COLORREF color = RGB(r, g, b);
                
                // Map key to theme field
                if (!_wcsicmp(wkey, L"bg")) theme->bg = color;
                else if (!_wcsicmp(wkey, L"fg")) theme->fg = color;
                else if (!_wcsicmp(wkey, L"selection_bg")) theme->selection_bg = color;
                else if (!_wcsicmp(wkey, L"syntax_keyword")) theme->syntax_keyword = color;
                else if (!_wcsicmp(wkey, L"syntax_string")) theme->syntax_string = color;
                else if (!_wcsicmp(wkey, L"syntax_comment")) theme->syntax_comment = color;
                else if (!_wcsicmp(wkey, L"syntax_number")) theme->syntax_number = color;
                else if (!_wcsicmp(wkey, L"syntax_type")) theme->syntax_type = color;
                else if (!_wcsicmp(wkey, L"syntax_function")) theme->syntax_function = color;
                // ... add more mappings as needed
            }
            // Parse string values
            else if (!_wcsicmp(wkey, L"name")) {
                wcscpy_s(theme->name, 64, wvalue);
            }
            else if (!_wcsicmp(wkey, L"font_name")) {
                wcscpy_s(theme->font_name, 64, wvalue);
            }
            else if (!_wcsicmp(wkey, L"font_size")) {
                theme->font_size = _wtoi(wvalue);
            }
        }
    }
    
    fclose(file);
    return true;
}

/**
 * Save theme to INI file
 */
bool theme_save_to_file(const ThemeConfig *theme, const wchar_t *path) {
    FILE *file;
    if (_wfopen_s(&file, path, L"w") != 0 || !file) {
        return false;
    }
    
    // Write header
    fprintf(file, "# WOFL IDE Theme Configuration\n");
    fprintf(file, "# Format: key=value\n");
    fprintf(file, "# Colors: R,G,B (0-255)\n\n");
    
    // Write metadata
    fwprintf(file, L"name=%ls\n", theme->name);
    fwprintf(file, L"author=%ls\n", theme->author);
    fprintf(file, "\n# Editor Colors\n");
    
    // Write colors
    fprintf(file, "bg=%d,%d,%d\n", GetRValue(theme->bg), GetGValue(theme->bg), GetBValue(theme->bg));
    fprintf(file, "fg=%d,%d,%d\n", GetRValue(theme->fg), GetGValue(theme->fg), GetBValue(theme->fg));
    fprintf(file, "selection_bg=%d,%d,%d\n", GetRValue(theme->selection_bg), GetGValue(theme->selection_bg), GetBValue(theme->selection_bg));
    
    fprintf(file, "\n# Syntax Colors\n");
    fprintf(file, "syntax_keyword=%d,%d,%d\n", GetRValue(theme->syntax_keyword), GetGValue(theme->syntax_keyword), GetBValue(theme->syntax_keyword));
    fprintf(file, "syntax_string=%d,%d,%d\n", GetRValue(theme->syntax_string), GetGValue(theme->syntax_string), GetBValue(theme->syntax_string));
    fprintf(file, "syntax_comment=%d,%d,%d\n", GetRValue(theme->syntax_comment), GetGValue(theme->syntax_comment), GetBValue(theme->syntax_comment));
    fprintf(file, "syntax_number=%d,%d,%d\n", GetRValue(theme->syntax_number), GetGValue(theme->syntax_number), GetBValue(theme->syntax_number));
    fprintf(file, "syntax_type=%d,%d,%d\n", GetRValue(theme->syntax_type), GetGValue(theme->syntax_type), GetBValue(theme->syntax_type));
    fprintf(file, "syntax_function=%d,%d,%d\n", GetRValue(theme->syntax_function), GetGValue(theme->syntax_function), GetBValue(theme->syntax_function));
    
    fprintf(file, "\n# Font Settings\n");
    fwprintf(file, L"font_name=%ls\n", theme->font_name);
    fprintf(file, "font_size=%d\n", theme->font_size);
    fprintf(file, "font_weight=%d\n", theme->font_weight);
    
    fclose(file);
    return true;
}

/**
 * Apply theme to application
 */
void theme_apply(AppState *app, const ThemeConfig *theme) {
    app->theme.col_output_fg = theme->output_fg;

    // Update color scheme
    app->theme.col_bg = theme->bg;
    app->theme.col_fg = theme->fg;
    app->theme.col_sel_bg = theme->selection_bg;
    app->theme.col_status_bg = theme->status_bg;
    app->theme.col_output_bg = theme->output_bg;
   
    // Update syntax colors
    app->theme.syn_colors[TK_TEXT] = theme->fg;
    app->theme.syn_colors[TK_KW] = theme->syntax_keyword;
    app->theme.syn_colors[TK_IDENT] = theme->syntax_ident;
    app->theme.syn_colors[TK_NUM] = theme->syntax_number;
    app->theme.syn_colors[TK_STR] = theme->syntax_string;
    app->theme.syn_colors[TK_CHAR] = theme->syntax_char;
    app->theme.syn_colors[TK_COMMENT] = theme->syntax_comment;
    app->theme.syn_colors[TK_PUNCT] = theme->syntax_punct;
    
    // Update font
    app->theme.font_px = theme->font_size;
    
    // Recreate font with new settings
    if (app->theme.hFont) {
        DeleteObject(app->theme.hFont);
    }

    LOGFONTW lf = {0};
    lf.lfHeight = -theme->font_size;
    lf.lfWeight = theme->font_weight;
    lf.lfItalic = theme->font_italic ? TRUE : FALSE;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, 32, theme->font_name);
    app->theme.hFont = CreateFontIndirectW(&lf);
    
    // Force redraw
    InvalidateRect(app->hwnd, NULL, TRUE);
}
