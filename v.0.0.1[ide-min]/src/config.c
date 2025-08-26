#include "editor.h"
#include <wchar.h>

static bool read_line(HANDLE f, wchar_t *out, int cap){
    static wchar_t buf[4096]; static int have=0, pos=0;
    int n=0;
    while(1){
        if(pos>=have){
            DWORD rd=0; if(!ReadFile(f, buf, 0, &rd, NULL)){ return false; }
        }
        return false;
    }
}

// Very tiny: look for build.wofl next to file; if present, parse [run] cmd=...
bool config_try_load_run_cmd(AppState *app){
    if(!app->file_dir[0]) return false;
    wchar_t path[WOFL_MAX_PATH];
    swprintf(path, WOFL_MAX_PATH, L"%ls\\build.wofl", app->file_dir);
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(f==INVALID_HANDLE_VALUE) return false;
    DWORD sz = GetFileSize(f, NULL);
    char *u8 = (char*)HeapAlloc(GetProcessHeap(), 0, sz+1);
    DWORD rd=0; ReadFile(f, u8, sz, &rd, NULL); CloseHandle(f); u8[rd]=0;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, u8, rd, NULL, 0);
    wchar_t *wb=(wchar_t*)HeapAlloc(GetProcessHeap(), 0, (wlen+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, u8, rd, wb, wlen); wb[wlen]=0; HeapFree(GetProcessHeap(),0,u8);
    // naive INI parse
    bool in_run=false;
    wchar_t *p=wb;
    while(*p){
        // get line
        wchar_t *start=p; while(*p && *p!=L'\n' && *p!=L'\r') p++; wchar_t ch=*p; *p=0;
        if(start[0]==L'['){ if(!_wcsicmp(start,L"[run]")) in_run=true; else in_run=false; }
        else if(in_run){
            wchar_t *eq = wcschr(start, L'=');
            if(eq){
                *eq=0;
                if(!_wcsicmp(start,L"cmd")){
                    wcsncpy_s(app->run_cmd, WOFL_CMD_MAX, eq+1, _TRUNCATE);
                    HeapFree(GetProcessHeap(),0,wb); return true;
                }
            }
        }
        *p = ch; while(*p==L'\n'||*p==L'\r') p++;
    }
    HeapFree(GetProcessHeap(),0,wb);
    return false;
}

void config_set_default_run_cmd(AppState *app){
    // derive by language
    switch(app->lang){
        case LANG_PY:
            swprintf(app->run_cmd, WOFL_CMD_MAX, L"python \"%ls\"", app->file_path);
            break;
        case LANG_JS:
            swprintf(app->run_cmd, WOFL_CMD_MAX, L"node \"%ls\"", app->file_path);
            break;
        case LANG_C: {
            wchar_t tmp[WOFL_MAX_PATH]; GetTempPathW(WOFL_MAX_PATH, tmp);
            swprintf(app->run_cmd, WOFL_CMD_MAX,
                     L"clang \"%ls\" -O2 -o \"%ls\\a.exe\" && \"%ls\\a.exe\"",
                     app->file_path, tmp, tmp);
        } break;
        default:
            swprintf(app->run_cmd, WOFL_CMD_MAX, L"\"%ls\"", app->file_path);
            break;
    }
}