#include "editor.h"
#include <wctype.h>
void syntax_scan_json(const wchar_t* l,int n,TokenSpan*out,int*on){
    int i=0,m=0;
    while(i<n){
        wchar_t c=l[i];
        if(c==L'"'){ int s=i++; while(i<n){ if(l[i]==L'\\') i+=2; else if(l[i]==L'"'){ i++; break; } else i++; } out[m++] = (TokenSpan){s,(size_t)(i-s),TK_STR}; }
        else if(c==L'/' && i+1<n && l[i+1]==L'/'){ out[m++] = (TokenSpan){i,(size_t)(n-i),TK_COMMENT}; break; }
        else if(iswdigit(c)||c==L'-'){ int s=i++; while(i<n && (iswdigit(l[i])||l[i]==L'.'||l[i]==L'e'||l[i]==L'E'||l[i]==L'+'||l[i]==L'-')) i++; out[m++] = (TokenSpan){s,(size_t)(i-s),TK_NUM}; }
        else if(iswspace(c)){ int s=i++; while(i<n&&iswspace(l[i])) i++; out[m++] = (TokenSpan){s,(size_t)(i-s),TK_TEXT}; }
        else { out[m++] = (TokenSpan){i++,1,TK_PUNCT}; }
        if(m>=255){ out[m++] = (TokenSpan){i,(size_t)(n-i),TK_TEXT}; break; }
    }
    *on=m;
}
