#include "editor.h"
#include <process.h>

static unsigned __stdcall reader_thread(void *p){
    AppState *app = (AppState*)p;
    HANDLE h = app->out.pipe_rd;
    char buf[2048];
    DWORD rd=0;
    for(;;){
        if(!ReadFile(h, buf, sizeof(buf), &rd, NULL) || rd==0) break;
        int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, (int)rd, NULL, 0);
        wchar_t *wb = (wchar_t*)HeapAlloc(GetProcessHeap(),0,(wlen+1)*sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, buf, (int)rd, wb, wlen); wb[wlen]=0;
        EnterCriticalSection(&app->out.lock);
        gb_move_gap(&app->out.buf, gb_length(&app->out.buf));
        gb_insert(&app->out.buf, wb, wlen);
        LeaveCriticalSection(&app->out.lock);
        HeapFree(GetProcessHeap(),0,wb);
        InvalidateRect(app->hwnd, NULL, FALSE);
    }
    _endthreadex(0);
    return 0;
}

bool run_spawn(AppState *app, const wchar_t *cmdline){
    if(app->out.proc_handle){ run_kill(app); }
    SECURITY_ATTRIBUTES sa={ sizeof(sa), NULL, TRUE };
    HANDLE rd=NULL, wr=NULL;
    if(!CreatePipe(&rd,&wr,&sa, 0)) return false;
    if(!SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0)){ CloseHandle(rd); CloseHandle(wr); return false; }

    STARTUPINFOW si={0}; si.cb=sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = NULL; si.hStdOutput=wr; si.hStdError=wr;
    PROCESS_INFORMATION pi={0};

    wchar_t cmd[WOFL_CMD_MAX];
    wcsncpy_s(cmd, WOFL_CMD_MAX, cmdline, _TRUNCATE);

    // working dir
    wchar_t wd[WOFL_MAX_PATH]; wcsncpy_s(wd, WOFL_MAX_PATH, app->file_dir[0]?app->file_dir:L".", _TRUNCATE);

    BOOL ok = CreateProcessW(NULL, cmd, NULL,NULL, TRUE, CREATE_NO_WINDOW, NULL, wd, &si, &pi);
    CloseHandle(wr);
    if(!ok){ CloseHandle(rd); return false; }

    app->out.pipe_rd = rd;
    app->out.proc_handle = pi.hProcess;
    CloseHandle(pi.hThread);

    if(!app->out.buf.data) gb_init(&app->out.buf);
    InitializeCriticalSection(&app->out.lock);
    app->out.visible = true;
    app->out.height_px = 200;

    uintptr_t th = _beginthreadex(NULL, 0, reader_thread, app, 0, NULL);
    app->out.reader_thread = (HANDLE)th;
    return true;
}

void run_kill(AppState *app){
    if(app->out.proc_handle){
        TerminateProcess(app->out.proc_handle, 1);
        CloseHandle(app->out.proc_handle);
        app->out.proc_handle=NULL;
    }
    if(app->out.reader_thread){ WaitForSingleObject(app->out.reader_thread, 1000); CloseHandle(app->out.reader_thread); app->out.reader_thread=NULL; }
    if(app->out.pipe_rd){ CloseHandle(app->out.pipe_rd); app->out.pipe_rd=NULL; }
    DeleteCriticalSection(&app->out.lock);
}
