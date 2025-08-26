#include "editor.h"
#include <wctype.h>

static const wchar_t* kw[] = {
 L"auto",L"break",L"case",L"char",L"const",L"continue",L"default",L"do",
 L"double",L"else",L"enum",L"extern",L"float",L"for",L"goto",L"if",
 L"inline",L"int",L"long",L"register",L"restrict",L"return",L"short",
 L"signed",L"sizeof",L"static",L"struct",L"switch",L"typedef",L"union",
 L"unsigned",L"void",L"volatile",L"while",L"class",L"namespace",L"template",
 L"typename",L"using",L"new",L"delete",L"bool",L"true",L"false",L"nullptr"
};
static bool is_kw(const wchar_t *s,int n){
    for(size_t i=0;i<sizeof(kw)/sizeof(kw[0]);++i){
        if((int)wcslen(kw[i])==n && wcsncmp(kw[i],s,n)==0) return true;
    }
    return false;
}

void syntax_scan_c(const wchar_t *l,int n,TokenSpan*out,int*on){
    int i=0, m=0;
    while(i<n){
        wchar_t c=l[i];
        if(iswspace(c)){ int s=i++; while(i<n && iswspace(l[i])) i++; out[m++] = (TokenSpan){s, i-s, TK_TEXT}; }
        else if(c==L'/' && i+1<n && l[i+1]==L'/'){ out[m++] = (TokenSpan){i, n-i, TK_COMMENT}; break; }
        else if(c==L'/' && i+1<n && l[i+1]==L'*'){
            int s=i; i+=2;
            while(i+1<n && !(l[i]==L'*'&&l[i+1]==L'/')) i++;
            if(i+1<n) i+=2; out[m++] = (TokenSpan){s, i-s, TK_COMMENT};
        }
        else if(c==L'"'){ int s=i++; while(i<n){ if(l[i]==L'\\'){ i+=2; } else if(l[i]==L'"'){ i++; break; } else i++; } out[m++] = (TokenSpan){s,i-s,TK_STR}; }
        else if(c==L'\''){ int s=i++; while(i<n){ if(l[i]==L'\\'){ i+=2; } else if(l[i]==L'\''){ i++; break; } else i++; } out[m++] = (TokenSpan){s,i-s,TK_CHAR}; }
        else if(iswdigit(c)){ int s=i++; while(i<n && (iswdigit(l[i])||l[i]==L'.'||iswalpha(l[i])||l[i]==L'_')) i++; out[m++] = (TokenSpan){s,i-s,TK_NUM}; }
        else if(is_word(c)){ int s=i++; while(i<n && is_word(l[i])) i++; TokenSpan t={s,i-s, TK_IDENT}; if(is_kw(l+s,t.len)) t.cls=TK_KW; out[m++]=t; }
        else { out[m++] = (TokenSpan){i++,1,TK_PUNCT}; }
        if(m>=255){ out[m++] = (TokenSpan){i,n-i,TK_TEXT}; break; }
    }
    *on=m;
}
