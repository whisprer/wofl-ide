// ==================== config.c ====================
// Configuration management

#include "editor.h"
#include <wchar.h>
#include <stdio.h>

/**
 * Parse simple INI-style configuration file
 */
static bool parse_config_line(const wchar_t *line, wchar_t *key, wchar_t *value) {
    // Skip whitespace and comments
    while (*line && iswspace(*line)) line++;
    if (!*line || *line == L'#' || *line == L';') return false;
    
    // Parse key
    int key_len = 0;
    while (*line && *line != L'=' && !iswspace(*line) && key_len < 255) {
        key[key_len++] = *line++;
    }
    key[key_len] = L'\0';
    
    // Skip whitespace and '='
    while (*line && iswspace(*line)) line++;
    if (*line != L'=') return false;
    line++;
    while (*line && iswspace(*line)) line++;
    
    // Parse value
    int value_len = 0;
    bool in_quotes = (*line == L'"');
    if (in_quotes) line++;
    
    while (*line && value_len < WOFL_CMD_MAX - 1) {
        if (in_quotes && *line == L'"') break;
        if (!in_quotes && (*line == L'\r' || *line == L'\n')) break;
        value[value_len++] = *line++;
    }
    value[value_len] = L'\0';
    
    // Trim trailing whitespace from value
    while (value_len > 0 && iswspace(value[value_len - 1])) {
        value[--value_len] = L'\0';
    }
    
    return key_len > 0 && value_len > 0;
}

/**
 * Load run command from build configuration
 */
bool config_try_load_run_cmd(AppState *app) {
    if (!app->file_dir[0]) return false;
    
    // Look for build.wofl in project directory
    wchar_t config_path[WOFL_MAX_PATH];
    swprintf_s(config_path, WOFL_MAX_PATH, L"%ls\\build.wofl", app->file_dir);
    
    HANDLE file = CreateFileW(config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return false;
    
    // Read file
    DWORD size = GetFileSize(file, NULL);
    if (size == 0 || size > 65536) {  // Limit config file size
        CloseHandle(file);
        return false;
    }
    
    char *buffer = (char*)HeapAlloc(GetProcessHeap(), 0, size + 1);
    DWORD bytes_read = 0;
    ReadFile(file, buffer, size, &bytes_read, NULL);
    CloseHandle(file);
    buffer[bytes_read] = '\0';
    
    // Convert to wide char
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, NULL, 0);
    wchar_t *wide_buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, 
                                                (wide_len + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, wide_buffer, wide_len);
    wide_buffer[wide_len] = L'\0';
    HeapFree(GetProcessHeap(), 0, buffer);
    
    // Parse configuration
    bool in_run_section = false;
    bool found = false;
    wchar_t *line = wide_buffer;
    wchar_t key[256], value[WOFL_CMD_MAX];
    
    while (*line) {
        // Get line
        wchar_t *line_end = line;
        while (*line_end && *line_end != L'\n' && *line_end != L'\r') line_end++;
        
        wchar_t saved = *line_end;
        *line_end = L'\0';
        
        // Check for section header
        if (line[0] == L'[') {
            in_run_section = !_wcsicmp(line, L"[run]");
        }
        // Parse key-value pair
        else if (in_run_section && parse_config_line(line, key, value)) {
            if (!_wcsicmp(key, L"cmd")) {
                wcscpy_s(app->run_cmd, WOFL_CMD_MAX, value);
                found = true;
            }
        }
        
        *line_end = saved;
        line = line_end;
        while (*line == L'\n' || *line == L'\r') line++;
    }
    
    HeapFree(GetProcessHeap(), 0, wide_buffer);
    return found;
}

/**
 * Set default run command based on file type
 */
void config_set_default_run_cmd(AppState *app) {
    if (!app->file_path[0]) {
        app->run_cmd[0] = L'\0';
        return;
    }
    
    switch (app->lang) {
        case LANG_PY:
            swprintf_s(app->run_cmd, WOFL_CMD_MAX, L"python \"%ls\"", app->file_path);
            break;
            
        case LANG_JS:
            swprintf_s(app->run_cmd, WOFL_CMD_MAX, L"node \"%ls\"", app->file_path);
            break;
            
        case LANG_C: {
            wchar_t temp_dir[WOFL_MAX_PATH];
            GetTempPathW(WOFL_MAX_PATH, temp_dir);
            swprintf_s(app->run_cmd, WOFL_CMD_MAX,
                      L"clang \"%ls\" -O2 -o \"%ls\\a.exe\" && \"%ls\\a.exe\"",
                      app->file_path, temp_dir, temp_dir);
            break;
        }
        
        case LANG_GO:
            swprintf_s(app->run_cmd, WOFL_CMD_MAX, L"go run \"%ls\"", app->file_path);
            break;
            
        case LANG_RS: {
            wchar_t temp_dir[WOFL_MAX_PATH];
            GetTempPathW(WOFL_MAX_PATH, temp_dir);
            swprintf_s(app->run_cmd, WOFL_CMD_MAX,
                      L"rustc \"%ls\" -O -o \"%ls\\a.exe\" && \"%ls\\a.exe\"",
                      app->file_path, temp_dir, temp_dir);
            break;
        }
        
        default:
            swprintf_s(app->run_cmd, WOFL_CMD_MAX, L"\"%ls\"", app->file_path);
            break;
    }
}