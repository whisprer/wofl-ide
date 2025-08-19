#include "editor.h"
void syntax_scan_md(const wchar_t* l,int n,TokenSpan*out,int*on){
    if(n>0 && (l[0]==L'#'||l[0]==L'>'||l[0]==L'-'||l[0]==L'*')){
        out[0]=(TokenSpan){0,(size_t)n,TK_KW}; *on=1; return;
    }
    out[0]=(TokenSpan){0,(size_t)n,TK_TEXT}; *on=1;
}
