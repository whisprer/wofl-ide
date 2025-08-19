// ==================== run_exec_win32.c ====================
// Process execution and output capture

#include "editor.h"
#include <process.h>

/**
 * Thread function to read process output
 */
static unsigned __stdcall output_reader_thread(void *param) {
    AppState *app = (AppState*)param;
    HANDLE pipe = app->out.pipe_rd;
    char buffer[4096];
    DWORD bytes_read = 0;
    
    while (ReadFile(pipe, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read > 0) {
        // Convert to wide char
        int wide_len = MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, NULL, 0);
        wchar_t *wide_buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), 0,
                                                    (wide_len + 1) * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, wide_buffer, wide_len);
        wide_buffer[wide_len] = L'\0';
        
        // Append to output buffer
        EnterCriticalSection(&app->out.lock);
        size_t current_pos = gb_length(&app->out.buf);
        gb_move_gap(&app->out.buf, current_pos);
        gb_insert(&app->out.buf, wide_buffer, wide_len);
        LeaveCriticalSection(&app->out.lock);
        
        HeapFree(GetProcessHeap(), 0, wide_buffer);
        
        // Trigger repaint
        InvalidateRect(app->hwnd, NULL, FALSE);
    }
    
    _endthreadex(0);
    return 0;
}

/**
 * Spawn process and capture output
 */
bool run_spawn(AppState *app, const wchar_t *cmdline) {
    // Kill existing process if running
    if (app->out.proc_handle) {
        run_kill(app);
    }
    
    // Create pipe for output capture
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE read_pipe = NULL, write_pipe = NULL;
    
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        return false;
    }
    
    if (!SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        return false;
    }
    
    // Setup process creation
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = NULL;
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;
    
    PROCESS_INFORMATION pi = {0};
    
    // Make command line copy (CreateProcess may modify it)
    wchar_t cmd_copy[WOFL_CMD_MAX];
    wcscpy_s(cmd_copy, WOFL_CMD_MAX, cmdline);
    
    // Use file directory as working directory
    const wchar_t *working_dir = app->file_dir[0] ? app->file_dir : NULL;
    
    // Create process
    BOOL success = CreateProcessW(NULL, cmd_copy, NULL, NULL, TRUE,
                                   CREATE_NO_WINDOW, NULL, working_dir,
                                   &si, &pi);
    
    CloseHandle(write_pipe);
    
    if (!success) {
        CloseHandle(read_pipe);
        return false;
    }
    
    // Initialize output pane
    app->out.pipe_rd = read_pipe;
    app->out.proc_handle = pi.hProcess;
    CloseHandle(pi.hThread);
    
    if (!app->out.buf.data) {
        gb_init(&app->out.buf);
    } else {
        // Clear existing output
        gb_free(&app->out.buf);
        gb_init(&app->out.buf);
    }
    
    InitializeCriticalSection(&app->out.lock);
    app->out.visible = true;
    app->out.height_px = 200;
    
    // Start reader thread
    uintptr_t thread = _beginthreadex(NULL, 0, output_reader_thread, app, 0, NULL);
    app->out.reader_thread = (HANDLE)thread;
    
    return true;
}

/**
 * Kill running process
 */
void run_kill(AppState *app) {
    if (app->out.proc_handle) {
        TerminateProcess(app->out.proc_handle, 1);
        CloseHandle(app->out.proc_handle);
        app->out.proc_handle = NULL;
    }
    
    if (app->out.reader_thread) {
        WaitForSingleObject(app->out.reader_thread, 1000);
        CloseHandle(app->out.reader_thread);
        app->out.reader_thread = NULL;
    }
    
    if (app->out.pipe_rd) {
        CloseHandle(app->out.pipe_rd);
        app->out.pipe_rd = NULL;
    }
    
    DeleteCriticalSection(&app->out.lock);
}

/**
 * Check if process is running
 */
bool run_is_active(const AppState *app) {
    if (!app->out.proc_handle) return false;
    
    DWORD exit_code;
    if (GetExitCodeProcess(app->out.proc_handle, &exit_code)) {
        return exit_code == STILL_ACTIVE;
    }
    
    return false;
}
