// ==================== syntax_cpp.c ====================
// C++ specific syntax highlighting

#include "editor.h"
#include <wctype.h>

// C++ keywords (includes C keywords plus C++ specific)
static const wchar_t* cpp_keywords[] = {
    // C keywords
    L"auto", L"break", L"case", L"char", L"const", L"continue", L"default", L"do",
    L"double", L"else", L"enum", L"extern", L"float", L"for", L"goto", L"if",
    L"inline", L"int", L"long", L"register", L"restrict", L"return", L"short",
    L"signed", L"sizeof", L"static", L"struct", L"switch", L"typedef", L"union",
    L"unsigned", L"void", L"volatile", L"while",
    // C++ specific
    L"alignas", L"alignof", L"and", L"and_eq", L"asm", L"bitand", L"bitor",
    L"bool", L"catch", L"char16_t", L"char32_t", L"class", L"compl", L"concept",
    L"const_cast", L"constexpr", L"consteval", L"constinit", L"co_await", L"co_return",
    L"co_yield", L"decltype", L"delete", L"dynamic_cast", L"explicit", L"export",
    L"false", L"friend", L"mutable", L"namespace", L"new", L"noexcept", L"not",
    L"not_eq", L"nullptr", L"operator", L"or", L"or_eq", L"private", L"protected",
    L"public", L"reinterpret_cast", L"requires", L"static_assert", L"static_cast",
    L"template", L"this", L"thread_local", L"throw", L"true", L"try", L"typeid",
    L"typename", L"using", L"virtual", L"wchar_t", L"xor", L"xor_eq"
};

// C++ type keywords
static const wchar_t* cpp_types[] = {
    L"std", L"string", L"vector", L"map", L"set", L"list", L"deque", L"queue",
    L"stack", L"pair", L"unique_ptr", L"shared_ptr", L"weak_ptr", L"function",
    L"array", L"tuple", L"optional", L"variant", L"any", L"byte", L"size_t",
    L"ptrdiff_t", L"nullptr_t", L"int8_t", L"int16_t", L"int32_t", L"int64_t",
    L"uint8_t", L"uint16_t", L"uint32_t", L"uint64_t"
};

static bool is_cpp_keyword(const wchar_t *word, int len) {
    for (size_t i = 0; i < sizeof(cpp_keywords) / sizeof(cpp_keywords[0]); i++) {
        if ((int)wcslen(cpp_keywords[i]) == len && 
            wcsncmp(cpp_keywords[i], word, len) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_cpp_type(const wchar_t *word, int len) {
    for (size_t i = 0; i < sizeof(cpp_types) / sizeof(cpp_types[0]); i++) {
        if ((int)wcslen(cpp_types[i]) == len && 
            wcsncmp(cpp_types[i], word, len) == 0) {
            return true;
        }
    }
    return false;
}

void syntax_scan_cpp(const wchar_t *line, int len, TokenSpan *out, int *out_n) {
    int i = 0, m = 0;
    
    while (i < len && m < WOFL_MAX_TOKENS - 1) {
        wchar_t ch = line[i];
        
        // Whitespace
        if (iswspace(ch)) {
            int start = i++;
            while (i < len && iswspace(line[i])) i++;
            out[m++] = (TokenSpan){start, i - start, TK_TEXT};
        }
        // Preprocessor directive
        else if (ch == L'#' && (i == 0 || iswspace(line[i-1]))) {
            out[m++] = (TokenSpan){i, len - i, TK_COMMENT};  // Use comment color for preprocessor
            break;
        }
        // Single-line comment
        else if (ch == L'/' && i + 1 < len && line[i + 1] == L'/') {
            out[m++] = (TokenSpan){i, len - i, TK_COMMENT};
            break;
        }
        // Multi-line comment
        else if (ch == L'/' && i + 1 < len && line[i + 1] == L'*') {
            int start = i;
            i += 2;
            while (i + 1 < len && !(line[i] == L'*' && line[i + 1] == L'/')) {
                i++;
            }
            if (i + 1 < len) i += 2;
            out[m++] = (TokenSpan){start, i - start, TK_COMMENT};
        }
        // String literal (including raw strings R"()")
        else if (ch == L'R' && i + 1 < len && line[i + 1] == L'"') {
            // Raw string literal
            int start = i;
            i += 2;
            if (i < len && line[i] == L'(') {
                i++;
                // Find closing )"
                while (i + 1 < len && !(line[i] == L')' && line[i + 1] == L'"')) {
                    i++;
                }
                if (i + 1 < len) i += 2;
            }
            out[m++] = (TokenSpan){start, i - start, TK_STR};
        }
        else if (ch == L'"') {
            int start = i++;
            while (i < len) {
                if (line[i] == L'\\') {
                    i += 2;
                } else if (line[i] == L'"') {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            out[m++] = (TokenSpan){start, i - start, TK_STR};
        }
        // Character literal
        else if (ch == L'\'') {
            int start = i++;
            while (i < len) {
                if (line[i] == L'\\') {
                    i += 2;
                } else if (line[i] == L'\'') {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            out[m++] = (TokenSpan){start, i - start, TK_CHAR};
        }
        // Number (including C++14 digit separators and binary literals)
        else if (iswdigit(ch) || (ch == L'0' && i + 1 < len && 
                 (line[i + 1] == L'x' || line[i + 1] == L'X' || 
                  line[i + 1] == L'b' || line[i + 1] == L'B'))) {
            int start = i++;
            if (ch == L'0' && i < len && (line[i] == L'x' || line[i] == L'X' || 
                                          line[i] == L'b' || line[i] == L'B')) {
                i++; // Skip x/X/b/B
            }
            while (i < len && (iswdigit(line[i]) || line[i] == L'.' || 
                   iswalpha(line[i]) || line[i] == L'_' || line[i] == L'\'')) {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_NUM};
        }
        // Identifier or keyword
        else if (iswalpha(ch) || ch == L'_') {
            int start = i++;
            while (i < len && (iswalnum(line[i]) || line[i] == L'_')) {
                i++;
            }
            
            // Check for function call pattern
            int end_word = i;
            while (i < len && iswspace(line[i])) i++;
            bool is_function = (i < len && line[i] == L'(');
            i = end_word; // Reset position
            
            TokenSpan token = {start, i - start, TK_IDENT};
            
            // Categorize token
            if (is_cpp_keyword(line + start, token.len)) {
                token.cls = TK_KW;
            } else if (is_cpp_type(line + start, token.len)) {
                token.cls = TK_KW; // Use keyword color for types
            } else if (is_function) {
                token.cls = TK_IDENT; // Could use a special color for functions
            }
            
            out[m++] = token;
        }
        
        // Operators and punctuation
        else {
            // Check for multi-character operators
            if (i + 1 < len) {
                wchar_t next = line[i + 1];
                // :: -> ++ -- << >> <= >= == != && || += -= *= /=
                if ((ch == L':' && next == L':') ||
                    (ch == L'-' && next == L'>') ||
                    (ch == L'+' && next == L'+') ||
                    (ch == L'-' && next == L'-') ||
                    (ch == L'<' && next == L'<') ||
                    (ch == L'>' && next == L'>') ||
                    (ch == L'<' && next == L'=') ||
                    (ch == L'>' && next == L'=') ||
                    (ch == L'=' && next == L'=') ||
                    (ch == L'!' && next == L'=') ||
                    (ch == L'&' && next == L'&') ||
                    (ch == L'|' && next == L'|') ||
                    (ch == L'+' && next == L'=') ||
                    (ch == L'-' && next == L'=') ||
                    (ch == L'*' && next == L'=') ||
                    (ch == L'/' && next == L'=')) {
                    out[m++] = (TokenSpan){i, 2, TK_PUNCT};
                    i += 2;
                    continue;
                }
            }
            out[m++] = (TokenSpan){i++, 1, TK_PUNCT};
        }
    }
    
    *out_n = m;
}