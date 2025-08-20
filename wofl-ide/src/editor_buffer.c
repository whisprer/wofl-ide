// ==================== editor_buffer.c ====================
// Gap buffer implementation aligned to editor.h

#include "editor.h"
#include <windows.h>
#include <string.h>

static void normalize_newlines(wchar_t *text, size_t *len, EolMode *eol) {
    size_t r = 0, w = 0;
    bool has_crlf = false;
    while (r < *len) {
        if (text[r] == L'\r') {
            if (r + 1 < *len && text[r + 1] == L'\n') {
                text[w++] = L'\n';
                r += 2;
                has_crlf = true;
            } else {
                text[w++] = L'\n';
                r += 1;
            }
        } else {
            text[w++] = text[r++];
        }
    }
    *len = w;
    *eol = has_crlf ? EOL_CRLF : EOL_LF;
}

void gb_init(GapBuffer *gb) {
    gb->capacity  = WOFL_INITIAL_GAP;
    gb->data      = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                        gb->capacity * sizeof(wchar_t));
    gb->gap_start = 0;
    gb->gap_end   = gb->capacity;
    gb->eol_mode  = EOL_LF;
    gb->dirty     = false;
}

void gb_free(GapBuffer *gb) {
    if (gb->data) {
        HeapFree(GetProcessHeap(), 0, gb->data);
        gb->data = NULL;
    }
    gb->capacity = 0;
    gb->gap_start = gb->gap_end = 0;
    gb->dirty = false;
}

size_t gb_length(const GapBuffer *gb) {
    const size_t gap = (gb->gap_end > gb->gap_start) ? (gb->gap_end - gb->gap_start) : 0;
    return gb->capacity - gap;
}

void gb_ensure(GapBuffer *gb, size_t need) {
    const size_t gap = gb->gap_end - gb->gap_start;
    if (gap >= need) return;

    const size_t pre  = gb->gap_start;
    const size_t post = gb->capacity - gb->gap_end;

    size_t new_cap = gb->capacity * 2;
    const size_t extra = need + 64;
    if (new_cap < gb->capacity + extra) new_cap = gb->capacity + extra;

    wchar_t *nd = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, new_cap * sizeof(wchar_t));
    if (!nd) return; // out-of-memory: fail safe (no growth)

    // Copy left of gap
    if (pre) memcpy(nd, gb->data, pre * sizeof(wchar_t));
    // Copy right of gap to the end of new buffer
    if (post) memcpy(nd + (new_cap - post), gb->data + gb->gap_end, post * sizeof(wchar_t));

    HeapFree(GetProcessHeap(), 0, gb->data);
    gb->data      = nd;
    gb->capacity  = new_cap;
    gb->gap_start = pre;
    gb->gap_end   = new_cap - post;
}

void gb_move_gap(GapBuffer *gb, size_t pos) {
    size_t len = gb_length(gb);
    if (pos > len) pos = len;
    if (pos == gb->gap_start) return;

    // Move gap left in chunks
    while (pos < gb->gap_start) {
        const size_t delta = gb->gap_start - pos;
        const size_t chunk = delta; // limited by available pre-gap anyway
        memmove(gb->data + (gb->gap_end - chunk),
                gb->data + (gb->gap_start - chunk),
                chunk * sizeof(wchar_t));
        gb->gap_start -= chunk;
        gb->gap_end   -= chunk;
    }

    // Move gap right in chunks (bounded by post-gap)
    while (pos > gb->gap_start) {
        const size_t delta    = pos - gb->gap_start;
        const size_t post_gap = gb->capacity - gb->gap_end;
        const size_t chunk    = (delta < post_gap) ? delta : post_gap;
        if (chunk == 0) break; // nothing to move (shouldn't happen often)
        memmove(gb->data + gb->gap_start,
                gb->data + gb->gap_end,
                chunk * sizeof(wchar_t));
        gb->gap_start += chunk;
        gb->gap_end   += chunk;
    }
}

void gb_insert(GapBuffer *gb, const wchar_t *s, size_t n) {
    if (!s || n == 0) return;
    gb_ensure(gb, n);
    memcpy(gb->data + gb->gap_start, s, n * sizeof(wchar_t));
    gb->gap_start += n;
    gb->dirty = true;
}

void gb_insert_ch(GapBuffer *gb, wchar_t ch) {
    gb_insert(gb, &ch, 1);
}

void gb_delete_range(GapBuffer *gb, size_t pos, size_t n) {
    size_t len = gb_length(gb);
    if (pos > len || n == 0) return;
    if (pos + n > len) n = len - pos;
    gb_move_gap(gb, pos);
    // expand gap by n (delete)
    size_t grow_to = gb->gap_end + n;
    if (grow_to > gb->capacity) grow_to = gb->capacity;
    gb->gap_end = grow_to;
    gb->dirty = true;
}

wchar_t gb_char_at(const GapBuffer *gb, size_t pos) {
    const size_t len = gb_length(gb);
    if (pos >= len) return L'\0';
    if (pos < gb->gap_start) {
        return gb->data[pos];
    } else {
        return gb->data[pos + (gb->gap_end - gb->gap_start)];
    }
}

bool gb_load_from_file(GapBuffer *gb, const wchar_t *path, EolMode *eol) {
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER sz;
    if (!GetFileSizeEx(f, &sz)) { CloseHandle(f); return false; }

    if (sz.QuadPart == 0) {
        CloseHandle(f);
        if (eol) *eol = EOL_LF;
        gb->eol_mode = EOL_LF;
        gb->dirty = false;
        return true;
    }

    char *utf8 = (char*)HeapAlloc(GetProcessHeap(), 0, (size_t)sz.QuadPart + 1);
    if (!utf8) { CloseHandle(f); return false; }
    DWORD rd = 0;
    if (!ReadFile(f, utf8, (DWORD)sz.QuadPart, &rd, NULL)) {
        HeapFree(GetProcessHeap(), 0, utf8);
        CloseHandle(f);
        return false;
    }
    CloseHandle(f);
    utf8[rd] = '\0';

    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, rd, NULL, 0);
    wchar_t *wbuf = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (wlen + 1) * sizeof(wchar_t));
    if (!wbuf) { HeapFree(GetProcessHeap(), 0, utf8); return false; }
    MultiByteToWideChar(CP_UTF8, 0, utf8, rd, wbuf, wlen);
    wbuf[wlen] = L'\0';
    HeapFree(GetProcessHeap(), 0, utf8);

    size_t final_len = (size_t)wlen;
    EolMode mode = EOL_LF;
    normalize_newlines(wbuf, &final_len, &mode);

    // Prepare gap: put all text before the gap
    gb_ensure(gb, final_len);
    memcpy(gb->data, wbuf, final_len * sizeof(wchar_t));
    gb->gap_start = final_len;
    gb->gap_end   = gb->capacity;
    gb->eol_mode  = mode;
    gb->dirty     = false;

    if (eol) *eol = mode;
    HeapFree(GetProcessHeap(), 0, wbuf);
    return true;
}

bool gb_save_to_file(GapBuffer *gb, const wchar_t *path, EolMode eol) {
    HANDLE f = CreateFileW(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return false;

    const size_t len = gb_length(gb);
    if (len == 0) {
        CloseHandle(f);
        gb->dirty = false;
        return true;
    }

    // Allocate worst-case (CRLF doubles LF; UTF-8 up to 3 bytes for BMP codepoints)
    size_t out_cap = len * 2 * 3;
    char *out = (char*)HeapAlloc(GetProcessHeap(), 0, out_cap);
    if (!out) { CloseHandle(f); return false; }

    int pos = 0;
    for (size_t i = 0; i < len; i++) {
        wchar_t ch = gb_char_at(gb, i);
        if (ch == L'\n' && eol == EOL_CRLF) {
            out[pos++] = '\r';
            out[pos++] = '\n';
        } else {
            char tmp[4];
            const int n = WideCharToMultiByte(CP_UTF8, 0, &ch, 1, tmp, 4, NULL, NULL);
            for (int j = 0; j < n; j++) out[pos++] = tmp[j];
        }
    }

    DWORD wr = 0;
    const BOOL ok = WriteFile(f, out, (DWORD)pos, &wr, NULL);
    HeapFree(GetProcessHeap(), 0, out);
    CloseHandle(f);
    if (!ok) return false;

    gb->dirty = false;
    return true;
}
