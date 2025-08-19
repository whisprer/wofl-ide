// ==================== cmd_palette.c ====================
// Command palette for quick actions

#include "editor.h"
#include <wchar.h>

// Command definitions
typedef enum {
    CMD_OPEN = 0,
    CMD_SAVE,
    CMD_SAVE_AS,
    CMD_RUN,
    CMD_TOGGLE_OUTPUT,
    CMD_FIND,
    CMD_GOTO_LINE,
    CMD_SET_LANG_C,
    CMD_SET_LANG_PYTHON,
    CMD_SET_LANG_JS,
    CMD_MAX
} CommandId;

typedef struct {
    const wchar_t *name;
    const wchar_t *description;
    CommandId id;
} CommandItem;

static const CommandItem commands[] = {
    {L"Open", L"Open a file", CMD_OPEN},
    {L"Save", L"Save current file", CMD_SAVE},
    {L"Save As", L"Save with new name", CMD_SAVE_AS},
    {L"Run", L"Execute current file", CMD_RUN},
    {L"Toggle Output", L"Show/hide output pane", CMD_TOGGLE_OUTPUT},
    {L"Find", L"Search in file", CMD_FIND},
    {L"Goto Line", L"Jump to line number", CMD_GOTO_LINE},
    {L"Set Language: C", L"C/C++ syntax", CMD_SET_LANG_C},
    {L"Set Language: Python", L"Python syntax", CMD_SET_LANG_PYTHON},
    {L"Set Language: JavaScript", L"JavaScript syntax", CMD_SET_LANG_JS}
};

/**
 * Open command palette
 */
void palette_open(AppState *app) {
    app->overlay_active = true;
    wcscpy_s(app->overlay_prompt, 128, L">");
    app->overlay_text[0] = L'\0';
    app->overlay_len = 0;
    app->overlay_cursor = 0;
    app->mode = MODE_PALETTE;
}

/**
 * Handle character input in palette
 */
void palette_handle_char(AppState *app, wchar_t ch) {
    if (app->overlay_len >= WOFL_CMD_MAX - 1) return;
    
    // Insert at cursor position
    memmove(app->overlay_text + app->overlay_cursor + 1,
            app->overlay_text + app->overlay_cursor,
            (app->overlay_len - app->overlay_cursor) * sizeof(wchar_t));
    
    app->overlay_text[app->overlay_cursor] = ch;
    app->overlay_len++;
    app->overlay_cursor++;
    app->overlay_text[app->overlay_len] = L'\0';
}

/**
 * Handle backspace in palette
 */
void palette_backspace(AppState *app) {
    if (app->overlay_cursor <= 0) return;
    
    memmove(app->overlay_text + app->overlay_cursor - 1,
            app->overlay_text + app->overlay_cursor,
            (app->overlay_len - app->overlay_cursor) * sizeof(wchar_t));
    
    app->overlay_len--;
    app->overlay_cursor--;
    app->overlay_text[app->overlay_len] = L'\0';
}

/**
 * Find matching command
 */
static int find_command(const wchar_t *text) {
    if (!text[0]) return -1;
    
    // Convert search text to uppercase for case-insensitive matching
    wchar_t search[WOFL_CMD_MAX];
    wcscpy_s(search, WOFL_CMD_MAX, text);
    _wcsupr_s(search, WOFL_CMD_MAX);
    
    // Find first matching command
    for (int i = 0; i < CMD_MAX; i++) {
        wchar_t cmd_upper[256];
        wcscpy_s(cmd_upper, 256, commands[i].name);
        _wcsupr_s(cmd_upper, 256);
        
        if (wcsstr(cmd_upper, search)) {
            return i;
        }
    }
    
    return -1;
}

/**
 * Execute selected command
 */
void palette_confirm(AppState *
app) {
    int cmd_idx = find_command(app->overlay_text);
    
    // Close palette
    app->overlay_active = false;
    app->mode = MODE_EDIT;
    
    if (cmd_idx < 0) return;
    
    // Execute command
    switch (commands[cmd_idx].id) {
        case CMD_OPEN:
            PostMessageW(app->hwnd, WM_COMMAND, 1, 0);
            break;
        case CMD_SAVE:
            PostMessageW(app->hwnd, WM_COMMAND, 2, 0);
            break;
        case CMD_SAVE_AS:
            PostMessageW(app->hwnd, WM_COMMAND, 3, 0);
            break;
        case CMD_RUN:
            PostMessageW(app->hwnd, WM_COMMAND, 4, 0);
            break;
        case CMD_TOGGLE_OUTPUT:
            PostMessageW(app->hwnd, WM_COMMAND, 5, 0);
            break;
        case CMD_FIND:
            PostMessageW(app->hwnd, WM_COMMAND, 6, 0);
            break;
        case CMD_GOTO_LINE:
            PostMessageW(app->hwnd, WM_COMMAND, 7, 0);
            break;
        case CMD_SET_LANG_C:
            app->lang = LANG_C;
            break;
        case CMD_SET_LANG_PYTHON:
            app->lang = LANG_PY;
            break;
        case CMD_SET_LANG_JS:
            app->lang = LANG_JS;
            break;
    }
    
    InvalidateRect(app->hwnd, NULL, FALSE);
}

/**
 * Cancel palette
 */
void palette_cancel(AppState *app) {
    app->overlay_active = false;
    app->mode = MODE_EDIT;
    InvalidateRect(app->hwnd, NULL, FALSE);
}

// Command palette (cmd_palette.c)
void     palette_open(AppState *app);
void     palette_handle_char(AppState *app, wchar_t ch);
void     palette_backspace(AppState *app);
void     palette_confirm(AppState *app);
void     palette_cancel(AppState *app);
