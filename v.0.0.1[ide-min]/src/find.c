#include "editor.h"
#include <wchar.h>
#include <wctype.h>

static bool match_here(const GapBuffer *gb, size_t pos, const wchar_t *needle, bool case_ins){
    for(int i=0; needle[i]; ++i){
        wchar_t c = (pos+i<gb_length(gb)) ? gb_char_at(gb, pos+i) : 0;
        wchar_t n = needle[i];
        if(case_ins){ c=towupper(c); n=towupper(n); }
        if(c!=n) return false;
    }
    return true;
}

bool find_next(AppState *app, const wchar_t *needle, bool case_ins, bool wrap, bool down){
    if(!needle || !needle[0]) return false;
    size_t start = editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
    size_t len = gb_length(&app->buf);
    if(down){
        for(size_t i=start+1;i<len;i++){ if(match_here(&app->buf,i,needle,case_ins)){ editor_index_to_linecol(&app->buf,i,&app->caret.line,&app->caret.col); app->selecting=false; return true; } }
        if(wrap){
            for(size_t i=0;i<start;i++){ if(match_here(&app->buf,i,needle,case_ins)){ editor_index_to_linecol(&app->buf,i,&app->caret.line,&app->caret.col); app->selecting=false; return true; } }
        }
    }else{
        if(start>0){
            for(size_t i=start-1; i>0; i--){ if(match_here(&app->buf,i,needle,case_ins)){ editor_index_to_linecol(&app->buf,i,&app->caret.line,&app->caret.col); app->selecting=false; return true; } }
            if(match_here(&app->buf,0,needle,case_ins)){ editor_index_to_linecol(&app->buf,0,&app->caret.line,&app->caret.col); app->selecting=false; return true; }
        }
        if(wrap){
            for(size_t i=len; i>start; i--){ if(match_here(&app->buf,i,needle,case_ins)){ editor_index_to_linecol(&app->buf,i,&app->caret.line,&app->caret.col); app->selecting=false; return true; } }
        }
    }
    return false;
}
