// ==================== plugin_system.h ====================
// Plugin system architecture for future extensions

#ifndef WOFL_PLUGIN_SYSTEM_H
#define WOFL_PLUGIN_SYSTEM_H

#include "editor.h"

// Plugin capabilities flags
typedef enum {
    PLUGIN_CAP_SYNTAX     = 0x0001,  // Provides syntax highlighting
    PLUGIN_CAP_COMMAND    = 0x0002,  // Adds commands to palette
    PLUGIN_CAP_KEYBIND    = 0x0004,  // Registers key bindings
    PLUGIN_CAP_MENU       = 0x0008,  // Adds menu items
    PLUGIN_CAP_TOOLBAR    = 0x0010,  // Adds toolbar buttons
    PLUGIN_CAP_AUTOCOMPLETE = 0x0020, // Provides auto-completion
    PLUGIN_CAP_FORMATTER  = 0x0040,  // Code formatting
    PLUGIN_CAP_BUILDER    = 0x0080,  // Build system integration
    PLUGIN_CAP_DEBUGGER   = 0x0100,  // Debugging support
    PLUGIN_CAP_VCS        = 0x0200,  // Version control
    PLUGIN_CAP_SEARCH     = 0x0400,  // Enhanced search
} PluginCapability;

// Plugin interface
typedef struct Plugin {
    // Metadata
    wchar_t name[64];
    wchar_t author[64];
    wchar_t version[16];
    wchar_t description[256];
    PluginCapability capabilities;
    
    // Lifecycle
    bool (*init)(struct Plugin *self, AppState *app);
    void (*shutdown)(struct Plugin *self);
    
    // Event handlers
    void (*on_file_open)(struct Plugin *self, const wchar_t *path);
    void (*on_file_save)(struct Plugin *self, const wchar_t *path);
    void (*on_file_close)(struct Plugin *self);
    void (*on_text_change)(struct Plugin *self, size_t pos, const wchar_t *text, size_t len);
    bool (*on_key_press)(struct Plugin *self, UINT key, bool ctrl, bool shift, bool alt);
    
    // Syntax provider
    void (*syntax_scan)(const wchar_t *line, int len, TokenSpan *out, int *out_n);
    
    // Command provider
    int (*get_commands)(struct Plugin *self, wchar_t commands[][64], int max);
    void (*execute_command)(struct Plugin *self, int command_id);
    
    // Private data
    void *data;
    
    // Next plugin in chain
    struct Plugin *next;
} Plugin;

// Plugin manager
typedef struct {
    Plugin *plugins;
    int plugin_count;
    wchar_t plugin_dir[WOFL_MAX_PATH];
} PluginManager;

// Plugin management functions
void plugin_manager_init(PluginManager *pm);
void plugin_manager_shutdown(PluginManager *pm);
bool plugin_manager_load(PluginManager *pm, const wchar_t *path);
void plugin_manager_unload(PluginManager *pm, const wchar_t *name);
Plugin* plugin_manager_find(PluginManager *pm, const wchar_t *name);

// Built-in plugins
Plugin* plugin_create_cpp(void);
Plugin* plugin_create_asm(void);
Plugin* plugin_create_csv(void);
Plugin* plugin_create_git(void);

// Future plugin placeholders for file utils
Plugin* plugin_create_resonant_search(void);  // For your resonant search
Plugin* plugin_create_file_utils(void);       // For your file utils

#endif // WOFL_PLUGIN_SYSTEM_H