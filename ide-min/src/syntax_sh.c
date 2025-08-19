#include "editor.h"
void syntax_scan_sh(const wchar_t* l,int n,TokenSpan*out,int*on){
    // simple: comments & strings
    int i=0,m=0;
    while(i<n){
        if(l[i]==L'#'){ out[m++]=(TokenSpan){i,(size_t)(n-i),TK_COMMENT}; break; }
        else if(l[i]==L'"'||l[i]==L'\''){ wchar_t q=l[i++]; int s=i-1; while(i<n){ if(l[i]==L'\\') i+=2; else if(l[i]==q){ i++; break; } else i++; } out[m++]=(TokenSpan){s,(size_t)(i-s),TK_STR}; }
        else { out[m++]=(TokenSpan){i++,1,TK_TEXT}; }
        if(m>=255){ out[m++]=(TokenSpan){i,(size_t)(n-i),TK_TEXT}; break; }
    }
    *on=m;
}
