#include "editor.h"
#include <string.h>

extern void syntax_scan_c(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_py(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_js(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_html(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_css(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_json(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_md(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_go(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_rs(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_sh(const wchar_t*,int,TokenSpan*,int*);
extern void syntax_scan_lua(const wchar_t*,int,TokenSpan*,int*);

static void scan_plain(const wchar_t *l,int n,TokenSpan*out,int*on){
    out[0].start=0; out[0].len=n; out[0].cls=TK_TEXT; *on=1;
}
static Syntax SY_PLAIN = { scan_plain };
static Syntax SY_C     = { syntax_scan_c };
static Syntax SY_PY    = { syntax_scan_py };
static Syntax SY_JS    = { syntax_scan_js };
static Syntax SY_HTML  = { syntax_scan_html };
static Syntax SY_CSS   = { syntax_scan_css };
static Syntax SY_JSON  = { syntax_scan_json };
static Syntax SY_MD    = { syntax_scan_md };
static Syntax SY_GO    = { syntax_scan_go };
static Syntax SY_RS    = { syntax_scan_rs };
static Syntax SY_SH    = { syntax_scan_sh };
static Syntax SY_LUA   = { syntax_scan_lua };

Language lang_from_ext(const wchar_t *path){
    const wchar_t *dot = wcsrchr(path, L'.');
    if(!dot) return LANG_NONE;
    if(!_wcsicmp(dot,L".c")||!_wcsicmp(dot,L".h")||!_wcsicmp(dot,L".cpp")||!_wcsicmp(dot,L".hpp")) return LANG_C;
    if(!_wcsicmp(dot,L".py")) return LANG_PY;
    if(!_wcsicmp(dot,L".js")||!_wcsicmp(dot,L".ts")) return LANG_JS;
    if(!_wcsicmp(dot,L".html")||!_wcsicmp(dot,L".htm")) return LANG_HTML;
    if(!_wcsicmp(dot,L".css")) return LANG_CSS;
    if(!_wcsicmp(dot,L".json")) return LANG_JSON;
    if(!_wcsicmp(dot,L".md")) return LANG_MD;
    if(!_wcsicmp(dot,L".go")) return LANG_GO;
    if(!_wcsicmp(dot,L".rs")) return LANG_RS;
    if(!_wcsicmp(dot,L".sh")||!_wcsicmp(dot,L".bash")) return LANG_SH;
    if(!_wcsicmp(dot,L".lua")) return LANG_LUA;
    return LANG_NONE;
}

const Syntax* syntax_get(Language lang){
    switch(lang){
        case LANG_C: return &SY_C;
        case LANG_PY: return &SY_PY;
        case LANG_JS: return &SY_JS;
        case LANG_HTML: return &SY_HTML;
        case LANG_CSS: return &SY_CSS;
        case LANG_JSON: return &SY_JSON;
        case LANG_MD: return &SY_MD;
        case LANG_GO: return &SY_GO;
        case LANG_RS: return &SY_RS;
        case LANG_SH: return &SY_SH;
        case LANG_LUA: return &SY_LUA;
        default: return &SY_PLAIN;
    }
}
