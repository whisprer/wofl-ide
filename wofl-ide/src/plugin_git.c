// ==================== plugin_git.c ====================
// Git auto-backup and versioning plugin

#include "editor.h"
#include <time.h>
#include <direct.h>

typedef struct {
    bool enabled;
    bool auto_commit;
    int commit_interval_minutes;
    time_t last_commit_time;
    wchar_t repo_path[WOFL_MAX_PATH];
    wchar_t commit_message[256];
} GitPlugin;

static GitPlugin g_git = {
    .enabled = false,
    .auto_commit = false,
    .commit_interval_minutes = 30,
    .last_commit_time = 0
};

/**
 * Initialize git repository in current directory
 */
bool git_init(const wchar_t *path) {
    wchar_t cmd[WOFL_CMD_MAX];
    swprintf_s(cmd, WOFL_CMD_MAX, L"cd /d \"%ls\" && git init", path);
    
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi = {0};
    
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // Create .gitignore
    wchar_t gitignore_path[WOFL_MAX_PATH];
    swprintf_s(gitignore_path, WOFL_MAX_PATH, L"%ls\\.gitignore", path);
    
    FILE *file;
    if (_wfopen_s(&file, gitignore_path, L"w") == 0 && file) {
        fprintf(file, "*.exe\n*.obj\n*.pdb\n*.ilk\n*.log\n*.tmp\n.vs/\n");
        fclose(file);
    }
    
    return true;
}

/**
 * Check if directory is a git repository
 */
bool git_is_repo(const wchar_t *path) {
    wchar_t git_dir[WOFL_MAX_PATH];
    swprintf_s(git_dir, WOFL_MAX_PATH, L"%ls\\.git", path);
    
    DWORD attrs = GetFileAttributesW(git_dir);
    return (attrs != INVALID_FILE_ATTRIBUTES && 
            (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

/**
 * Stage and commit changes
 */
bool git_commit(const wchar_t *path, const wchar_t *message) {
    wchar_t cmd[WOFL_CMD_MAX];
    
    // Stage all changes
    swprintf_s(cmd, WOFL_CMD_MAX, L"cd /d \"%ls\" && git add -A", path);
    
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi = {0};
    
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // Commit with message
    swprintf_s(cmd, WOFL_CMD_MAX, 
               L"cd /d \"%ls\" && git commit -m \"%ls\"", path, message);
    
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    g_git.last_commit_time = time(NULL);
    return true;
}

/**
 * Auto-commit if enabled and interval passed
 */
void git_auto_commit_check(AppState *app) {
    if (!g_git.enabled || !g_git.auto_commit) return;
    if (!app->file_dir[0]) return;
    if (!git_is_repo(app->file_dir)) return;
    
    time_t now = time(NULL);
    if (now - g_git.last_commit_time < g_git.commit_interval_minutes * 60) {
        return;
    }
    
    // Generate commit message with timestamp
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    wchar_t message[256];
    wcsftime(message, 256, L"Auto-save %Y-%m-%d %H:%M:%S", &timeinfo);
    
    git_commit(app->file_dir, message);
}

/**
 * Get git status
 */
bool git_get_status(const wchar_t *path, wchar_t *status, size_t max_len) {
    wchar_t cmd[WOFL_CMD_MAX];
    swprintf_s(cmd, WOFL_CMD_MAX, L"cd /d \"%ls\" && git status --short", path);
    
    // Create pipes for output
    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
    HANDLE read_pipe, write_pipe;
    
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        return false;
    }
    
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi = {0};
    
    if (!CreateProcessW(NULL, cmd, NULL, NULL, TRUE, 
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        return false;
    }
    
    CloseHandle(write_pipe);
    
    // Read output
    char buffer[4096] = {0};
    DWORD bytes_read = 0;
    ReadFile(read_pipe, buffer, sizeof(buffer) - 1, &bytes_read, NULL);
    
    CloseHandle(read_pipe);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // Convert to wide char
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, status, (int)max_len);
    
    return true;
}

/**
 * Enable git plugin for current project
 */
void git_plugin_enable(AppState *app) {
    if (!app->file_dir[0]) return;
    
    wcscpy_s(g_git.repo_path, WOFL_MAX_PATH, app->file_dir);
    
    if (!git_is_repo(g_git.repo_path)) {
        // Initialize new repository
        git_init(g_git.repo_path);
        
        // Initial commit
        git_commit(g_git.repo_path, L"Initial commit");
    }
    
    g_git.enabled = true;
    g_git.last_commit_time = time(NULL);
}

/**
 * Disable git plugin
 */
void git_plugin_disable(void) {
    g_git.enabled = false;
    g_git.auto_commit = false;
}

/**
 * Toggle auto-commit
 */
void git_toggle_auto_commit(void) {
    g_git.auto_commit = !g_git.auto_commit;
}
