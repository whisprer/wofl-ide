#include "editor.h"
extern void syntax_scan_c(const wchar_t*,int,TokenSpan*,int*);
void syntax_scan_go(const wchar_t* l,int n,TokenSpan*out,int*on){ syntax_scan_c(l,n,out,on); }
