#include "editor.h"
#include <wctype.h>
void syntax_scan_css(const wchar_t* l,int n,TokenSpan*out,int*on){
    int i=0,m=0;
    while(i<n){
        if(l[i]==L'/' && i+1<n && l[i+1]==L'*'){ int s=i; i+=2; while(i+1<n && !(l[i]==L'*'&&l[i+1]==L'/')) i++; if(i+1<n) i+=2; out[m++] = (TokenSpan){s,(size_t)(i-s),TK_COMMENT}; }
        else if(l[i]==L'"'||l[i]==L'\''){ wchar_t q=l[i++]; int s=i-1; while(i<n){ if(l[i]==L'\\') i+=2; else if(l[i]==q){ i++; break; } else i++; } out[m++] = (TokenSpan){s,(size_t)(i-s),TK_STR}; }
        else if(iswdigit(l[i])){ int s=i++; while(i<n && (iswdigit(l[i])||l[i]==L'.'||iswalpha(l[i])||l[i]==L'%')) i++; out[m++] = (TokenSpan){s,(size_t)(i-s),TK_NUM}; }
        else { out[m++] = (TokenSpan){i++,1,TK_TEXT}; }
        if(m>=255){ out[m++] = (TokenSpan){i,(size_t)(n-i),TK_TEXT}; break; }
    }
    *on=m;
}
