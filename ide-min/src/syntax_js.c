#include "editor.h"
#include <wctype.h>

static const wchar_t* kw[] = {
 L"break",L"case",L"catch",L"class",L"const",L"continue",L"debugger",L"default",
 L"delete",L"do",L"else",L"export",L"extends",L"finally",L"for",L"function",
 L"if",L"import",L"in",L"instanceof",L"new",L"return",L"super",L"switch",
 L"this",L"throw",L"try",L"typeof",L"var",L"void",L"while",L"with",L"yield",
 L"let",L"await",L"async",L"true",L"false",L"null",L"undefined"
};
static bool is_kw(const wchar_t *s,int n){
    for(size_t i=0;i<sizeof(kw)/sizeof(kw[0]);++i){ if((int)wcslen(kw[i])==n && wcsncmp(kw[i],s,n)==0) return true; }
    return false;
}
void syntax_scan_js(const wchar_t *l,int n,TokenSpan*out,int*on){
    int i=0,m=0;
    while(i<n){
        wchar_t c=l[i];
        if(iswspace(c)){ int s=i++; while(i<n && iswspace(l[i])) i++; out[m++] = (TokenSpan){s,i-s,TK_TEXT}; }
        else if(c==L'/' && i+1<n && l[i+1]==L'/'){ out[m++] = (TokenSpan){i,n-i,TK_COMMENT}; break; }
        else if(c==L'/' && i+1<n && l[i+1]==L'*'){ int s=i; i+=2; while(i+1<n && !(l[i]==L'*'&&l[i+1]==L'/')) i++; if(i+1<n) i+=2; out[m++] = (TokenSpan){s,i-s,TK_COMMENT}; }
        else if(c==L'"'||c==L'\''||c==L'`'){
            wchar_t q=c; int s=i++; while(i<n){ if(l[i]==L'\\'){ i+=2; } else if(l[i]==q){ i++; break; } else i++; }
            out[m++] = (TokenSpan){s,i-s,TK_STR};
        }
        else if(iswdigit(c)){ int s=i++; while(i<n && (iswdigit(l[i])||l[i]==L'.'||iswalpha(l[i])||l[i]==L'_')) i++; out[m++] = (TokenSpan){s,i-s,TK_NUM}; }
        else if(is_word(c)){ int s=i++; while(i<n && (is_word(l[i])||l[i]==L'$')) i++; TokenSpan t={s,i-s,TK_IDENT}; if(is_kw(l+s,t.len)) t.cls=TK_KW; out[m++]=t; }
        else { out[m++] = (TokenSpan){i++,1,TK_PUNCT}; }
        if(m>=255){ out[m++] = (TokenSpan){i,n-i,TK_TEXT}; break; }
    }
    *on=m;
}
