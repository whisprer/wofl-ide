#include "editor.h"
#include <wctype.h>

static const wchar_t* kw[] = {
 L"and",L"as",L"assert",L"break",L"class",L"continue",L"def",L"del",L"elif",
 L"else",L"except",L"False",L"finally",L"for",L"from",L"global",L"if",
 L"import",L"in",L"is",L"lambda",L"None",L"nonlocal",L"not",L"or",L"pass",
 L"raise",L"return",L"True",L"try",L"while",L"with",L"yield"
};
static bool is_kw(const wchar_t *s,int n){
    for(size_t i=0;i<sizeof(kw)/sizeof(kw[0]);++i){ if((int)wcslen(kw[i])==n && wcsncmp(kw[i],s,n)==0) return true; }
    return false;
}

void syntax_scan_py(const wchar_t *l,int n,TokenSpan*out,int*on){
    int i=0, m=0;
    while(i<n){
        wchar_t c=l[i];
        if(iswspace(c)){ int s=i++; while(i<n && iswspace(l[i])) i++; out[m++] = (TokenSpan){s, i-s, TK_TEXT}; }
        else if(c==L'#'){ out[m++] = (TokenSpan){i, n-i, TK_COMMENT}; break; }
        else if(c==L'"' || c==L'\''){
            wchar_t q=c; int s=i++; bool triple=false;
            if(i+1<n && l[i]==q && l[i+1]==q){ triple=true; i+=2; }
            if(triple){ while(i+2<n && !(l[i]==q && l[i+1]==q && l[i+2]==q)){ if(l[i]==L'\\') i+=2; else i++; } if(i+2<n) i+=3; }
            else{ while(i<n){ if(l[i]==L'\\'){ i+=2; } else if(l[i]==q){ i++; break; } else i++; } }
            out[m++] = (TokenSpan){s,i-s,TK_STR};
        }
        else if(iswdigit(c)){ int s=i++; while(i<n && (iswdigit(l[i])||l[i]==L'.'||iswalpha(l[i])||l[i]==L'_')) i++; out[m++] = (TokenSpan){s,i-s,TK_NUM}; }
        else if(is_word(c)){ int s=i++; while(i<n && (is_word(l[i])||l[i]==L'.')) i++; TokenSpan t={s,i-s,TK_IDENT}; if(is_kw(l+s,t.len)) t.cls=TK_KW; out[m++]=t; }
        else { out[m++] = (TokenSpan){i++,1,TK_PUNCT}; }
        if(m>=255){ out[m++] = (TokenSpan){i,n-i,TK_TEXT}; break; }
    }
    *on=m;
}
