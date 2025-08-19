#include "editor.h"
void syntax_scan_html(const wchar_t* l,int n,TokenSpan*out,int*on){
    // very light: tags/punct vs text
    int i=0,m=0;
    while(i<n){
        if(l[i]==L'<'){ int s=i++; while(i<n && l[i]!=L'>') i++; if(i<n) i++;
            out[m++] = (TokenSpan){s, (size_t)(i-s), TK_PUNCT};
        } else {
            int s=i++; while(i<n && l[i]!=L'<') i++;
            out[m++] = (TokenSpan){s, (size_t)(i-s), TK_TEXT};
        }
        if(m>=255){ out[m++] = (TokenSpan){i,(size_t)(n-i),TK_TEXT}; break; }
    }
    *on=m;
}
