// ==================== Usage Examples ====================

/*
USAGE GUIDE FOR THEMES AND PLUGINS

1. THEME USAGE
==============

// In your main window initialization:
ThemeConfig theme;
theme_load_dark(&theme);  // Or any other theme
theme_apply(&g_app, &theme);

// To save a custom theme:
theme_save_to_file(&theme, L"C:\\Users\\You\\AppData\\Roaming\\WoflIDE\\themes\\MyTheme.theme");

// To load a custom theme:
theme_load_from_file(&theme, L"path\\to\\theme.theme");
theme_apply(&g_app, &theme);


2. THEME FILE FORMAT
====================

Create a .theme file in %APPDATA%\WoflIDE\themes\ with this format:

name=My Theme
author=Your Name

# Colors are R,G,B format (0-255)
bg=30,30,30
fg=220,220,220
selection_bg=50,80,120
syntax_keyword=86,156,214
syntax_string=206,145,120
syntax_comment=106,153,85
syntax_number=181,206,168

font_name=Consolas
font_size=14
font_weight=400


3. PLUGIN USAGE
===============

// Initialize plugin system on startup:
PluginManager pm;
plugin_manager_init(&pm);

// Find and use a plugin:
Plugin *cpp = plugin_manager_find(&pm, L"C++ Enhanced");
if (cpp && cpp->syntax_scan) {
    // Use for syntax highlighting
    TokenSpan tokens[256];
    int token_count;
    cpp->syntax_scan(line_text, line_len, tokens, &token_count);
}

// Handle plugin commands in command palette:
wchar_t plugin_commands[10][64];
int cmd_count = plugin->get_commands(plugin, plugin_commands, 10);
// Add to command palette...

// On file save (for git plugin):
Plugin *git = plugin_manager_find(&pm, L"Git Auto-Backup");
if (git && git->on_file_save) {
    git->on_file_save(git, file_path);
}


4. ADDING TO BUILD.BAT
======================

Add these new source files to your build script:

set THEME_SOURCES=src\theme_system.c
set PLUGIN_SOURCES=src\plugin_system.c src\syntax_cpp.c src\syntax_asm.c src\syntax_csv.c src\plugin_git.c

set ALL_SOURCES=%CORE_SOURCES% %SYNTAX_SOURCES% %FEATURE_SOURCES% %THEME_SOURCES% %PLUGIN_SOURCES%


5. KEYBOARD SHORTCUTS
=====================

Suggested key bindings to add:

Ctrl+Shift+P - Open command palette with plugin commands
Ctrl+K, Ctrl+T - Theme selector
Ctrl+Shift+G - Git commands
F12 - Go to definition (for future plugins)
Ctrl+Space - Autocomplete (for future plugins)


6. FUTURE PLUGIN INTERFACE
==========================

Notes to self, for your file utils plugins, implement these functions:

Plugin* plugin_create_resonant_search(void) {
    Plugin *plugin = HeapAlloc(...);
    plugin->name = L"Resonant Search";
    plugin->capabilities = PLUGIN_CAP_SEARCH;
    plugin->on_key_press = resonant_search_key_handler;
    plugin->execute_command = resonant_search_execute;
    // Your resonant search implementation here
    return plugin;
}

Plugin* plugin_create_file_utils(void) {
    Plugin *plugin = HeapAlloc(...);
    plugin->name = L"File Utils";
    plugin->capabilities = PLUGIN_CAP_COMMAND;
    plugin->get_commands = file_utils_get_commands;
    plugin->execute_command = file_utils_execute;
    // Your file utils implementation here
    return plugin;
}

*/

// ==================== Command Palette Integration ====================
// Modify cmd_palette.c to include plugin commands

/*
typedef struct {
    const wchar_t *name;
    const wchar_t *description;
    CommandId id;
    Plugin *plugin;  // Add plugin reference
    int plugin_cmd_id;  // Plugin-specific command ID
} CommandItem;

// In palette_confirm():
if (commands[cmd_idx].plugin) {
    // Execute plugin command
    commands[cmd_idx].plugin->execute_command(
        commands[cmd_idx].plugin, 
        commands[cmd_idx].plugin_cmd_id
    );
} else {
    // Execute built-in command
    // ... existing code
}
*/

// ==================== Menu System Suggestion ====================

/*
MINIMAL MENU SYSTEM (Optional, for better UX)

typedef struct {
    wchar_t title[32];
    wchar_t items[20][64];
    int item_count;
    int selected;
    bool visible;
} Menu;

typedef struct {
    Menu file_menu;
    Menu edit_menu;
    Menu view_menu;
    Menu plugins_menu;
    Menu themes_menu;
} MenuBar;

// Right-click context menu
typedef struct {
    int x, y;
    bool visible;
    wchar_t items[10][64];
    int item_count;
} ContextMenu;

// Add to main message loop:
case WM_RBUTTONDOWN:
    // Show context menu
    show_context_menu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    break;

// Theme menu items:
- Themes > Dark
- Themes > Light  
- Themes > Monokai
- Themes > Dracula
- Themes > Load Custom...
- Themes > Save Current...

// Plugin menu items:
- Plugins > Manage...
- Plugins > Git > Initialize
- Plugins > Git > Commit
- Plugins > Git > Auto-commit [✓]
- Plugins > Language > C++
- Plugins > Language > Assembly
- Plugins > Language > CSV
*/

// ==================== Configuration File ====================

/*
WOFL IDE CONFIGURATION FILE
Location: %APPDATA%\WoflIDE\config.ini

[General]
theme=Dark
font_name=Consolas
font_size=14
tab_width=4
show_line_numbers=true
highlight_current_line=true
auto_save=true
auto_save_interval=5

[Plugins]
enabled=cpp,asm,csv,git
auto_load=true

[Git]
auto_commit=true
commit_interval=30
default_message=Auto-save

[Editor]
word_wrap=false
show_whitespace=false
match_brackets=true
auto_indent=true

[Recent]
file1=C:\projects\myfile.cpp
file2=C:\projects\data.csv
file3=C:\projects\code.asm

*/