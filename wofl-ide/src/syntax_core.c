// ==================== syntax_core.c (FIXED) ====================
// Fixed version with proper syntax definitions

#include "editor.h"
#include <string.h>
#include <wctype.h>

// Forward declarations for syntax scanners
extern void syntax_scan_c(const wchar_t*, int, TokenSpan*, int*);
extern void syntax_scan_py(const wchar_t*, int, TokenSpan*, int*);
extern void syntax_scan_js(const wchar_t*, int, TokenSpan*, int*);

// Plain text scanner
static void syntax_scan_plain(const wchar_t *line, int len, TokenSpan *out, int *out_n) {
    out[0] = (TokenSpan){0, len, TK_TEXT};
    *out_n = 1;
}

// Static syntax definitions
static Syntax syntax_plain = { syntax_scan_plain };
static Syntax syntax_c = { syntax_scan_c };
static Syntax syntax_python = { syntax_scan_py };
static Syntax syntax_js_obj = { syntax_scan_js };

// Language detection from file extension
Language syntax_detect_language(const wchar_t *path) {
    const wchar_t *dot = wcsrchr(path, L'.');
    if (!dot) return LANG_NONE;
    
    // C/C++
    if (!_wcsicmp(dot, L".c") || !_wcsicmp(dot, L".h")) {
        return LANG_C;
    }
    else if (!_wcsicmp(dot, L".cpp") || !_wcsicmp(dot, L".hpp") ||
             !_wcsicmp(dot, L".cc") || !_wcsicmp(dot, L".cxx")) {
        return LANG_CPP;
    }
    // Python
    else if (!_wcsicmp(dot, L".py") || !_wcsicmp(dot, L".pyw")) {
        return LANG_PY;
    }
    // JavaScript
    else if (!_wcsicmp(dot, L".js") || !_wcsicmp(dot, L".ts") ||
             !_wcsicmp(dot, L".jsx") || !_wcsicmp(dot, L".tsx")) {
        return LANG_JS;
    }
    // Others (add as needed)
    else if (!_wcsicmp(dot, L".go")) return LANG_GO;
    else if (!_wcsicmp(dot, L".rs")) return LANG_RS;
    else if (!_wcsicmp(dot, L".sh") || !_wcsicmp(dot, L".bash")) return LANG_SH;
    else if (!_wcsicmp(dot, L".lua")) return LANG_LUA;
    else if (!_wcsicmp(dot, L".md")) return LANG_MD;
    else if (!_wcsicmp(dot, L".json")) return LANG_JSON;
    else if (!_wcsicmp(dot, L".html") || !_wcsicmp(dot, L".htm")) return LANG_HTML;
    else if (!_wcsicmp(dot, L".css")) return LANG_CSS;
    else if (!_wcsicmp(dot, L".csv")) return LANG_CSV;
    else if (!_wcsicmp(dot, L".asm") || !_wcsicmp(dot, L".s")) return LANG_ASM;
    
    return LANG_NONE;
}

// Get syntax for language
const Syntax* syntax_get(Language lang) {
    switch (lang) {
        case LANG_C:
        case LANG_CPP:
            return &syntax_c;
        case LANG_PY:
            return &syntax_python;
        case LANG_JS:
            return &syntax_js_obj;
        case LANG_GO:
        case LANG_RS:
            return &syntax_c;  // Use C syntax as fallback
        default:
            return &syntax_plain;
    }
}
