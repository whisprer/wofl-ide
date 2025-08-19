// ==================== plugin_system.c ====================
// Plugin system implementation

#include "plugin_system.h"
#include <stdio.h>

static PluginManager g_plugin_manager = {0};

/**
 * Initialize plugin manager
 */
void plugin_manager_init(PluginManager *pm) {
    pm->plugins = NULL;
    pm->plugin_count = 0;
    
    // Get plugin directory
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        swprintf_s(pm->plugin_dir, WOFL_MAX_PATH, L"%ls\\WoflIDE\\plugins", appdata);
        CreateDirectoryW(pm->plugin_dir, NULL);
    }
    
    // Load built-in plugins
    Plugin *cpp_plugin = plugin_create_cpp();
    if (cpp_plugin && cpp_plugin->init(cpp_plugin, NULL)) {
        cpp_plugin->next = pm->plugins;
        pm->plugins = cpp_plugin;
        pm->plugin_count++;
    }
    
    Plugin *asm_plugin = plugin_create_asm();
    if (asm_plugin && asm_plugin->init(asm_plugin, NULL)) {
        asm_plugin->next = pm->plugins;
        pm->plugins = asm_plugin;
        pm->plugin_count++;
    }
    
    Plugin *csv_plugin = plugin_create_csv();
    if (csv_plugin && csv_plugin->init(csv_plugin, NULL)) {
        csv_plugin->next = pm->plugins;
        pm->plugins = csv_plugin;
        pm->plugin_count++;
    }
    
    Plugin *git_plugin = plugin_create_git();
    if (git_plugin && git_plugin->init(git_plugin, NULL)) {
        git_plugin->next = pm->plugins;
        pm->plugins = git_plugin;
        pm->plugin_count++;
    }
}

/**
 * Shutdown plugin manager
 */
void plugin_manager_shutdown(PluginManager *pm) {
    Plugin *current = pm->plugins;
    while (current) {
        Plugin *next = current->next;
        if (current->shutdown) {
            current->shutdown(current);
        }
        HeapFree(GetProcessHeap(), 0, current);
        current = next;
    }
    pm->plugins = NULL;
    pm->plugin_count = 0;
}

/**
 * Find plugin by name
 */
Plugin* plugin_manager_find(PluginManager *pm, const wchar_t *name) {
    Plugin *current = pm->plugins;
    while (current) {
        if (_wcsicmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// ==================== Built-in Plugin Implementations ====================

/**
 * C++ Plugin Implementation
 */
static bool cpp_plugin_init(Plugin *self, AppState *app) {
    (void)app;
    wcscpy_s(self->name, 64, L"C++ Enhanced");
    wcscpy_s(self->author, 64, L"WOFL");
    wcscpy_s(self->version, 16, L"1.0");
    wcscpy_s(self->description, 256, L"Enhanced C++ syntax highlighting with C++20 support");
    self->capabilities = PLUGIN_CAP_SYNTAX;
    return true;
}

Plugin* plugin_create_cpp(void) {
    Plugin *plugin = (Plugin*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Plugin));
    if (plugin) {
        plugin->init = cpp_plugin_init;
        plugin->syntax_scan = syntax_scan_cpp;
    }
    return plugin;
}

/**
 * Assembly Plugin Implementation
 */
static bool asm_plugin_init(Plugin *self, AppState *app) {
    (void)app;
    wcscpy_s(self->name, 64, L"Assembly x86/x64");
    wcscpy_s(self->author, 64, L"WOFL");
    wcscpy_s(self->version, 16, L"1.0");
    wcscpy_s(self->description, 256, L"Assembly language syntax for x86/x64 architectures");
    self->capabilities = PLUGIN_CAP_SYNTAX;
    return true;
}

Plugin* plugin_create_asm(void) {
    Plugin *plugin = (Plugin*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Plugin));
    if (plugin) {
        plugin->init = asm_plugin_init;
        plugin->syntax_scan = syntax_scan_asm;
    }
    return plugin;
}

/**
 * CSV Plugin Implementation
 */
static bool csv_plugin_init(Plugin *self, AppState *app) {
    (void)app;
    wcscpy_s(self->name, 64, L"CSV Viewer");
    wcscpy_s(self->author, 64, L"WOFL");
    wcscpy_s(self->version, 16, L"1.0");
    wcscpy_s(self->description, 256, L"CSV file syntax highlighting with field detection");
    self->capabilities = PLUGIN_CAP_SYNTAX;
    return true;
}

Plugin* plugin_create_csv(void) {
    Plugin *plugin = (Plugin*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Plugin));
    if (plugin) {
        plugin->init = csv_plugin_init;
        plugin->syntax_scan = syntax_scan_csv;
    }
    return plugin;
}

/**
 * Git Plugin Implementation
 */
typedef struct {
    GitPlugin git_state;
} GitPluginData;

static bool git_plugin_init(Plugin *self, AppState *app) {
    (void)app;
    wcscpy_s(self->name, 64, L"Git Auto-Backup");
    wcscpy_s(self->author, 64, L"WOFL");
    wcscpy_s(self->version, 16, L"1.0");
    wcscpy_s(self->description, 256, L"Automatic git commits and version control");
    self->capabilities = PLUGIN_CAP_VCS | PLUGIN_CAP_COMMAND;
    
    // Allocate plugin data
    self->data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(GitPluginData));
    return self->data != NULL;
}

static void git_plugin_shutdown(Plugin *self) {
    if (self->data) {
        HeapFree(GetProcessHeap(), 0, self->data);
        self->data = NULL;
    }
}

static void git_plugin_on_file_save(Plugin *self, const wchar_t *path) {
    GitPluginData *data = (GitPluginData*)self->data;
    if (data && data->git_state.enabled && data->git_state.auto_commit) {
        // Auto-commit on save
        wchar_t dir[WOFL_MAX_PATH];
        wcscpy_s(dir, WOFL_MAX_PATH, path);
        
        // Get directory from path
        wchar_t *last_sep = wcsrchr(dir, L'\\');
        if (last_sep) *last_sep = L'\0';
        
        if (git_is_repo(dir)) {
            git_commit(dir, L"Auto-save commit");
        }
    }
}

static int git_plugin_get_commands(Plugin *self, wchar_t commands[][64], int max) {
    (void)self;
    int count = 0;
    if (count < max) wcscpy_s(commands[count++], 64, L"Git: Initialize Repository");
    if (count < max) wcscpy_s(commands[count++], 64, L"Git: Commit Changes");
    if (count < max) wcscpy_s(commands[count++], 64, L"Git: Toggle Auto-Commit");
    if (count < max) wcscpy_s(commands[count++], 64, L"Git: Show Status");
    return count;
}

static void git_plugin_execute_command(Plugin *self, int command_id) {
    GitPluginData *data = (GitPluginData*)self->data;
    if (!data) return;
    
    switch (command_id) {
        case 0: // Initialize Repository
            // Would need access to current directory
            break;
        case 1: // Commit Changes
            if (data->git_state.enabled) {
                git_commit(data->git_state.repo_path, L"Manual commit");
            }
            break;
        case 2: // Toggle Auto-Commit
            data->git_state.auto_commit = !data->git_state.auto_commit;
            break;
        case 3: // Show Status
            // Would show status in output pane
            break;
    }
}

Plugin* plugin_create_git(void) {
    Plugin *plugin = (Plugin*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Plugin));
    if (plugin) {
        plugin->init = git_plugin_init;
        plugin->shutdown = git_plugin_shutdown;
        plugin->on_file_save = git_plugin_on_file_save;
        plugin->get_commands = git_plugin_get_commands;
        plugin->execute_command = git_plugin_execute_command;
    }
    return plugin;
}
