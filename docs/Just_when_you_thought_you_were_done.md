// ==================== Usage Examples ====================

/\*
USAGE GUIDE FOR THEMES AND PLUGINS

1. THEME USAGE
   ==============

// In your main window initialization:
ThemeConfig theme;
theme\_load\_dark(\&theme);  // Or any other theme
theme\_apply(\&g\_app, \&theme);

// To save a custom theme:
theme\_save\_to\_file(\&theme, L"C:\\Users\\You\\AppData\\Roaming\\woflIDE\\themes\\MyTheme.theme");

// To load a custom theme:
theme\_load\_from\_file(\&theme, L"path\\to\\theme.theme");
theme\_apply(\&g\_app, \&theme);



2. THEME FILE FORMAT
   ====================

Create a .theme file in %APPDATA%\\WoflIDE\\themes\\ with this format:

name=My Theme
author=Your Name

# Colors are R,G,B format (0-255)

bg=30,30,30
fg=220,220,220
selection\_bg=50,80,120
syntax\_keyword=86,156,214
syntax\_string=206,145,120
syntax\_comment=106,153,85
syntax\_number=181,206,168

font\_name=Consolas
font\_size=14
font\_weight=400



3. PLUGIN USAGE
   ===============

// Initialize plugin system on startup:
PluginManager pm;
plugin\_manager\_init(\&pm);

// Find and use a plugin:
Plugin \*cpp = plugin\_manager\_find(\&pm, L"C++ Enhanced");
if (cpp \&\& cpp->syntax\_scan) {
// Use for syntax highlighting
TokenSpan tokens\[256];
int token\_count;
cpp->syntax\_scan(line\_text, line\_len, tokens, \&token\_count);
}

// Handle plugin commands in command palette:
wchar\_t plugin\_commands\[10]\[64];
int cmd\_count = plugin->get\_commands(plugin, plugin\_commands, 10);
// Add to command palette...

// On file save (for git plugin):
Plugin \*git = plugin\_manager\_find(\&pm, L"Git Auto-Backup");
if (git \&\& git->on\_file\_save) {
git->on\_file\_save(git, file\_path);
}



4. ADDING TO BUILD.BAT
   ======================

Add these new source files to your build script:

set THEME\_SOURCES=src\\theme\_system.c
set PLUGIN\_SOURCES=src\\plugin\_system.c src\\syntax\_cpp.c src\\syntax\_asm.c src\\syntax\_csv.c src\\plugin\_git.c

set ALL\_SOURCES=%CORE\_SOURCES% %SYNTAX\_SOURCES% %FEATURE\_SOURCES% %THEME\_SOURCES% %PLUGIN\_SOURCES%



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

Plugin\* plugin\_create\_resonant\_search(void) {
Plugin \*plugin = HeapAlloc(...);
plugin->name = L"Resonant Search";
plugin->capabilities = PLUGIN\_CAP\_SEARCH;
plugin->on\_key\_press = resonant\_search\_key\_handler;
plugin->execute\_command = resonant\_search\_execute;
// Your resonant search implementation here
return plugin;
}

Plugin\* plugin\_create\_file\_utils(void) {
Plugin \*plugin = HeapAlloc(...);
plugin->name = L"File Utils";
plugin->capabilities = PLUGIN\_CAP\_COMMAND;
plugin->get\_commands = file\_utils\_get\_commands;
plugin->execute\_command = file\_utils\_execute;
// Your file utils implementation here
return plugin;
}

\*/

// ==================== Command Palette Integration ====================
// Modify cmd\_palette.c to include plugin commands

/\*
typedef struct {
const wchar\_t \*name;
const wchar\_t \*description;
CommandId id;
Plugin \*plugin;  // Add plugin reference
int plugin\_cmd\_id;  // Plugin-specific command ID
} CommandItem;

// In palette\_confirm():
if (commands\[cmd\_idx].plugin) {
// Execute plugin command
commands\[cmd\_idx].plugin->execute\_command(
commands\[cmd\_idx].plugin,
commands\[cmd\_idx].plugin\_cmd\_id
);
} else {
// Execute built-in command
// ... existing code
}
\*/

// ==================== Menu System Suggestion ====================

/\*
MINIMAL MENU SYSTEM (Optional, for better UX)

typedef struct {
wchar\_t title\[32];
wchar\_t items\[20]\[64];
int item\_count;
int selected;
bool visible;
} Menu;

typedef struct {
Menu file\_menu;
Menu edit\_menu;
Menu view\_menu;
Menu plugins\_menu;
Menu themes\_menu;
} MenuBar;

// Right-click context menu
typedef struct {
int x, y;
bool visible;
wchar\_t items\[10]\[64];
int item\_count;
} ContextMenu;

// Add to main message loop:
case WM\_RBUTTONDOWN:
// Show context menu
show\_context\_menu(GET\_X\_LPARAM(lParam), GET\_Y\_LPARAM(lParam));
break;

// Theme menu items:

* Themes > Dark
* Themes > Light
* Themes > Monokai
* Themes > Dracula
* Themes > Load Custom...
* Themes > Save Current...

// Plugin menu items:

* Plugins > Manage...
* Plugins > Git > Initialize
* Plugins > Git > Commit
* Plugins > Git > Auto-commit \[✓]
* Plugins > Language > C++
* Plugins > Language > Assembly
* Plugins > Language > CSV
  \*/

// ==================== Configuration File ====================

/\*
WOFL IDE CONFIGURATION FILE
Location: %APPDATA%\\WoflIDE\\config.ini

\[General]
theme=Dark
font\_name=Consolas
font\_size=14
tab\_width=4
show\_line\_numbers=true
highlight\_current\_line=true
auto\_save=true
auto\_save\_interval=5

\[Plugins]
enabled=cpp,asm,csv,git
auto\_load=true

\[Git]
auto\_commit=true
commit\_interval=30
default\_message=Auto-save

\[Editor]
word\_wrap=false
show\_whitespace=false
match\_brackets=true
auto\_indent=true

\[Recent]
file1=C:\\projects\\myfile.cpp
file2=C:\\projects\\data.csv
file3=C:\\projects\\code.asm

\*/

