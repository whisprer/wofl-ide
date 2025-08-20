// ==================== syntax_csv.c ====================
// CSV file syntax highlighting

#include "editor.h"
#include <wctype.h>

void syntax_scan_csv(const wchar_t *line, int len, TokenSpan *out, int *out_n) {
    int i = 0, m = 0;
    int field_count = 0;
    
    while (i < len && m < WOFL_MAX_TOKENS - 1) {
        wchar_t ch = line[i];
        
        // Quoted field
        if (ch == L'"') {
            int start = i++;
//            bool escaped = false;
            
            while (i < len) {
                if (line[i] == L'"') {
                    if (i + 1 < len && line[i + 1] == L'"') {
                        // Escaped quote
                        i += 2;
                    } else {
                        // End of quoted field
                        i++;
                        break;
                    }
                } else {
                    i++;
                }
            }
            
            // Alternate colors for fields
            TokenClass cls = (field_count % 2 == 0) ? TK_STR : TK_CHAR;
            out[m++] = (TokenSpan){start, i - start, cls};
            field_count++;
        }
        // Comma separator
        else if (ch == L',' || ch == L';' || ch == L'\t' || ch == L'|') {
            out[m++] = (TokenSpan){i++, 1, TK_PUNCT};
        }
        // Unquoted field
        else if (!iswspace(ch)) {
            int start = i++;
            
            // Read until next separator or end
            while (i < len && line[i] != L',' && line[i] != L';' && 
                   line[i] != L'\t' && line[i] != L'|' && !iswspace(line[i])) {
                i++;
            }
            
            // Check if it's a number
            bool is_number = true;
            bool has_dot = false;
            for (int j = start; j < i; j++) {
                if (line[j] == L'.' || line[j] == L',') {
                    if (has_dot) {
                        is_number = false;
                        break;
                    }
                    has_dot = true;
                } else if (line[j] == L'-' || line[j] == L'+') {
                    if (j != start) {
                        is_number = false;
                        break;
                    }
                } else if (!iswdigit(line[j])) {
                    is_number = false;
                    break;
                }
            }
            
            TokenClass cls;
            if (is_number) {
                cls = TK_NUM;
            } else {
                // Alternate colors for fields
                cls = (field_count % 2 == 0) ? TK_IDENT : TK_KW;
            }
            
            out[m++] = (TokenSpan){start, i - start, cls};
            field_count++;
        }
        // Whitespace
        else {
            int start = i++;
            while (i < len && iswspace(line[i]) && 
                   line[i] != L',' && line[i] != L';' && 
                   line[i] != L'\t' && line[i] != L'|') {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_TEXT};
        }
    }
    
    *out_n = m;
}
