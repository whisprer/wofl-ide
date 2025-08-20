// ==================== syntax_sh.c ====================
// Shell script syntax highlighting

#include "editor.h"
#include <wctype.h>

/**
 * Shell script syntax scanner
 * Handles: comments (#), strings (single/double quotes), variables ($)
 */
void syntax_scan_shell(const wchar_t *line, int len, TokenSpan *out, int *out_n) {
    int i = 0, m = 0;
    
    while (i < len && m < WOFL_MAX_TOKENS - 1) {
        wchar_t ch = line[i];
        
        // Comment (# to end of line)
        if (ch == L'#') {
            out[m++] = (TokenSpan){i, len - i, TK_COMMENT};
            break;
        }
        // String literals (single or double quotes)
        else if (ch == L'"' || ch == L'\'') {
            wchar_t quote = ch;
            int start = i++;
            
            while (i < len) {
                if (line[i] == L'\\' && i + 1 < len) {
                    // Handle escape sequences
                    i += 2;
                } else if (line[i] == quote) {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            
            out[m++] = (TokenSpan){start, i - start, TK_STR};
        }
        // Variables ($VAR or ${VAR})
        else if (ch == L'$') {
            int start = i++;
            
            if (i < len && line[i] == L'{') {
                // ${VAR} format
                i++;
                while (i < len && line[i] != L'}') {
                    i++;
                }
                if (i < len && line[i] == L'}') {
                    i++;
                }
            } else if (i < len && (iswalpha(line[i]) || line[i] == L'_')) {
                // $VAR format
                while (i < len && (is_word(line[i]) || iswdigit(line[i]))) {
                    i++;
                }
            }
            
            out[m++] = (TokenSpan){start, i - start, TK_IDENT};
        }
        // Numbers
        else if (iswdigit(ch)) {
            int start = i++;
            while (i < len && (iswdigit(line[i]) || line[i] == L'.')) {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_NUM};
        }
        // Keywords and identifiers
        else if (is_word(ch)) {
            int start = i++;
            while (i < len && is_word(line[i])) {
                i++;
            }
            
            // Check for common shell keywords
            TokenSpan token = {start, i - start, TK_IDENT};
            
            // Common shell keywords
            static const wchar_t* sh_keywords[] = {
                L"if", L"then", L"else", L"elif", L"fi", L"for", L"while", L"do", L"done",
                L"case", L"esac", L"function", L"return", L"break", L"continue", L"exit",
                L"export", L"local", L"readonly", L"echo", L"printf", L"read", L"cd",
                L"pwd", L"test", L"true", L"false", L"shift", L"set", L"unset"
            };
            
            for (size_t k = 0; k < sizeof(sh_keywords) / sizeof(sh_keywords[0]); k++) {
                if ((int)wcslen(sh_keywords[k]) == token.len &&
                    wcsncmp(sh_keywords[k], line + start, token.len) == 0) {
                    token.cls = TK_KW;
                    break;
                }
            }
            
            out[m++] = token;
        }
        // Whitespace
        else if (iswspace(ch)) {
            int start = i++;
            while (i < len && iswspace(line[i])) {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_TEXT};
        }
        // Operators and punctuation
        else {
            out[m++] = (TokenSpan){i++, 1, TK_PUNCT};
        }
    }
    
    *out_n = m;
}

// Export for syntax system
void syntax_scan_sh(const wchar_t *line, int len, TokenSpan *out, int *out_n) {
    syntax_scan_shell(line, len, out, out_n);
}
