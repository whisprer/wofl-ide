#include "editor.h"
#include <wchar.h>

typedef struct { const wchar_t* name; } CmdItem;

static CmdItem items[] = {
    {L"Open"}, {L"Save"}, {L"Save As"}, {L"Run"}, {L"Toggle Output"},
    {L"Find"}, {L"Goto Line"}, {L"Set Language: C"}, {L"Set Language: Python"}, {L"Set Language: JavaScript"}
};
static int n_items = sizeof(items)/sizeof(items[0]);

void palette_open(AppState *app){
    app->overlay_active=true;
    wcscpy_s(app->overlay_prompt, 128, L">");
    app->overlay_text[0]=0; app->overlay_len=0; app->overlay_cursor=0;
    app->mode = MODE_PALETTE;
}

void palette_handle_char(AppState *app, wchar_t ch){
    if(app->overlay_len+1 >= WOFL_CMD_MAX) return;
    // insert at cursor
    memmove(app->overlay_text + app->overlay_cursor + 1,
            app->overlay_text + app->overlay_cursor,
            (app->overlay_len - app->overlay_cursor)*sizeof(wchar_t));
    app->overlay_text[app->overlay_len+1]=0;
    app->overlay_text[app->overlay_cursor]=ch;
    app->overlay_len++; app->overlay_cursor++;
}

void palette_backspace(AppState *app){
    if(app->overlay_cursor<=0) return;
    memmove(app->overlay_text + app->overlay_cursor - 1,
            app->overlay_text + app->overlay_cursor,
            (app->overlay_len - app->overlay_cursor)*sizeof(wchar_t));
    app->overlay_len--; app->overlay_cursor--;
    app->overlay_text[app->overlay_len]=0;
}

static int palette_pick(const wchar_t *s){
    // naive substring match; pick first
    for(int i=0;i<n_items;i++){ if(wcsstr(items[i].name, s)) return i; }
    return -1;
}

void palette_confirm(AppState *app){
    int idx = palette_pick(app->overlay_text);
    app->overlay_active=false; app->mode=MODE_EDIT;
    if(idx<0) return;
    switch(idx){
        case 0: // Open
            PostMessageW(app->hwnd, WM_COMMAND, 1, 0); break;
        case 1: // Save
            PostMessageW(app->hwnd, WM_COMMAND, 2, 0); break;
        case 2: // Save As
            PostMessageW(app->hwnd, WM_COMMAND, 3, 0); break;
        case 3: // Run
            PostMessageW(app->hwnd, WM_COMMAND, 4, 0); break;
        case 4: // Toggle Output
            PostMessageW(app->hwnd, WM_COMMAND, 5, 0); break;
        case 5: // Find
            PostMessageW(app->hwnd, WM_COMMAND, 6, 0); break;
        case 6: // Goto
            PostMessageW(app->hwnd, WM_COMMAND, 7, 0); break;
        case 7: app->lang=LANG_C; break;
        case 8: app->lang=LANG_PY; break;
        case 9: app->lang=LANG_JS; break;
    }
    InvalidateRect(app->hwnd,NULL,FALSE);
}
