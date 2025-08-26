#include "editor.h"
#include <stdio.h>
#include <stdlib.h>

static size_t min_size(size_t a, size_t b){ return a<b?a:b; }
static size_t max_size(size_t a, size_t b){ return a>b?a:b; }

void gb_init(GapBuffer *gb){
    gb->cap = 4096;
    gb->data = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, gb->cap*sizeof(wchar_t));
    gb->gap_start = 0;
    gb->gap_end   = gb->cap;
    gb->eol_mode = EOL_LF;
    gb->dirty = false;
}

void gb_free(GapBuffer *gb){
    if(gb->data) HeapFree(GetProcessHeap(), 0, gb->data);
    gb->data=NULL; gb->cap=0; gb->gap_start=gb->gap_end=0;
}

size_t gb_length(const GapBuffer *gb){ return gb->cap - (gb->gap_end - gb->gap_start); }

void gb_ensure(GapBuffer *gb, size_t need){
    size_t gap_sz = gb->gap_end - gb->gap_start;
    if(gap_sz >= need) return;
    size_t newcap = gb->cap * 2 + need;
    wchar_t *nd = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, newcap*sizeof(wchar_t));
    size_t pre = gb->gap_start;
    size_t post = gb->cap - gb->gap_end;
    memcpy(nd, gb->data, pre*sizeof(wchar_t));
    memcpy(nd+(newcap-post), gb->data+gb->gap_end, post*sizeof(wchar_t));
    HeapFree(GetProcessHeap(), 0, gb->data);
    gb->data = nd;
    gb->gap_end = newcap - post;
    gb->cap = newcap;
}

void gb_move_gap(GapBuffer *gb, size_t pos){
    size_t cur = gb->gap_start;
    if(pos == cur) return;
    if(pos < cur){
        size_t n = cur - pos;
        memmove(gb->data + gb->gap_end - n, gb->data + pos, n*sizeof(wchar_t));
        gb->gap_start -= n;
        gb->gap_end   -= n;
    }else{
        size_t n = pos - cur;
        memmove(gb->data + gb->gap_start, gb->data + gb->gap_end, n*sizeof(wchar_t));
        gb->gap_start += n;
        gb->gap_end   += n;
    }
}

void gb_insert(GapBuffer *gb, const wchar_t *s, size_t n){
    gb_ensure(gb, n);
    memcpy(gb->data + gb->gap_start, s, n*sizeof(wchar_t));
    gb->gap_start += n;
    gb->dirty = true;
}

void gb_insert_ch(GapBuffer *gb, wchar_t ch){ gb_insert(gb, &ch, 1); }

void gb_delete_range(GapBuffer *gb, size_t pos, size_t n){
    if(n==0) return;
    gb_move_gap(gb, pos);
    gb->gap_end += n;
    gb->dirty = true;
}

wchar_t gb_char_at(const GapBuffer *gb, size_t pos){
    size_t pre = gb->gap_start;
    if(pos < pre) return gb->data[pos];
    return gb->data[pos + (gb->gap_end - gb->gap_start)];
}

// --- UTF-8 file I/O helpers ---
static void normalize_newlines_inplace(wchar_t *w, size_t *len, EolMode *eol){
    // detect CRLF; normalize to LF in memory
    size_t i=0, j=0; bool saw_crlf=false;
    while(i<*len){
        if(w[i]==L'\r' && (i+1<*len) && w[i+1]==L'\n'){ w[j++]=L'\n'; i+=2; saw_crlf=true; }
        else { w[j++]=w[i++]; }
    }
    *len=j;
    *eol = saw_crlf?EOL_CRLF:EOL_LF;
}

void gb_load_from_utf8_file(GapBuffer *gb, const wchar_t *path, EolMode *eol){
    gb_init(gb);
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(f==INVALID_HANDLE_VALUE) return;
    LARGE_INTEGER sz; GetFileSizeEx(f, &sz);
    if(sz.QuadPart==0){ CloseHandle(f); *eol=EOL_LF; return; }
    char *buf = (char*)HeapAlloc(GetProcessHeap(), 0, (size_t)sz.QuadPart+1);
    DWORD rd=0; ReadFile(f, buf, (DWORD)sz.QuadPart, &rd, NULL); CloseHandle(f);
    buf[rd]=0;

    size_t wlen = MultiByteToWideChar(CP_UTF8, 0, buf, (int)rd, NULL, 0);
    wchar_t *wb = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (wlen+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, buf, (int)rd, wb, (int)wlen);
    wb[wlen]=0;
    HeapFree(GetProcessHeap(), 0, buf);

    normalize_newlines_inplace(wb, &wlen, eol);
    gb_ensure(gb, wlen);
    memcpy(gb->data, wb, wlen*sizeof(wchar_t));
    gb->gap_start = wlen;
    HeapFree(GetProcessHeap(), 0, wb);
    gb->dirty=false;
    gb->eol_mode=*eol;
}

bool gb_save_to_utf8_file(GapBuffer *gb, const wchar_t *path, EolMode eol){
    // flatten to contiguous memory
    size_t len = gb_length(gb);
    wchar_t *flat = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (len+1)*sizeof(wchar_t));
    size_t pre=gb->gap_start, post=gb->cap - gb->gap_end;
    memcpy(flat, gb->data, pre*sizeof(wchar_t));
    memcpy(flat+pre, gb->data+gb->gap_end, post*sizeof(wchar_t));
    len = pre+post; flat[len]=0;

    // convert LF->CRLF if needed
    size_t i=0, count_crlf=0;
    if(eol==EOL_CRLF){
        for(i=0;i<len;i++) if(flat[i]==L'\n') count_crlf++;
        wchar_t *out = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (len+count_crlf+1)*sizeof(wchar_t));
        size_t j=0;
        for(i=0;i<len;i++){
            if(flat[i]==L'\n'){ out[j++]=L'\r'; out[j++]=L'\n'; } else out[j++]=flat[i];
        }
        out[j]=0;
        HeapFree(GetProcessHeap(), 0, flat);
        flat=out; len=j;
    }

    int u8len = WideCharToMultiByte(CP_UTF8, 0, flat, (int)len, NULL, 0, NULL, NULL);
    char *u8 = (char*)HeapAlloc(GetProcessHeap(), 0, u8len);
    WideCharToMultiByte(CP_UTF8, 0, flat, (int)len, u8, u8len, NULL, NULL);

    HANDLE f = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(f==INVALID_HANDLE_VALUE){ HeapFree(GetProcessHeap(),0,u8); HeapFree(GetProcessHeap(),0,flat); return false; }
    DWORD wr=0; WriteFile(f, u8, u8len, &wr, NULL); CloseHandle(f);

    HeapFree(GetProcessHeap(),0,u8); HeapFree(GetProcessHeap(),0,flat);
    gb->dirty=false; return true;
}
