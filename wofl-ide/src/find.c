// ==================== find.c ====================
// Find and replace functionality

#include "editor.h"
#include <wchar.h>
#include <wctype.h>

/**
 * Check if text matches at position
 */
static bool match_at_position(const GapBuffer *gb, size_t pos, 
                              const wchar_t *needle, bool case_insensitive) {
    size_t needle_len = wcslen(needle);
    size_t buf_len = gb_length(gb);
    
    if (pos + needle_len > buf_len) return false;
    
    for (size_t i = 0; i < needle_len; i++) {
        wchar_t buf_char = gb_char_at(gb, pos + i);
        wchar_t needle_char = needle[i];
        
        if (case_insensitive) {
            buf_char = towupper(buf_char);
            needle_char = towupper(needle_char);
        }
        
        if (buf_char != needle_char) return false;
    }
    
    return true;
}

/**
 * Find next occurrence
 */
bool find_next(AppState *app, const wchar_t *needle, bool case_ins, bool wrap, bool down) {
    if (!needle || !needle[0]) return false;
    
    size_t start = editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
    size_t len = gb_length(&app->buf);
    
    if (down) {
        // Search forward
        for (size_t i = start + 1; i < len; i++) {
            if (match_at_position(&app->buf, i, needle, case_ins)) {
                editor_index_to_linecol(&app->buf, i, &app->caret.line, &app->caret.col);
                app->selecting = false;
                return true;
            }
        }
        
        // Wrap around if enabled
        if (wrap) {
            for (size_t i = 0; i <= start; i++) {
                if (match_at_position(&app->buf, i, needle, case_ins)) {
                    editor_index_to_linecol(&app->buf, i, &app->caret.line, &app->caret.col);
                    app->selecting = false;
                    return true;
                }
            }
        }
    } else {
        // Search backward
        if (start > 0) {
            for (size_t i = start - 1; i > 0; i--) {
                if (match_at_position(&app->buf, i, needle, case_ins)) {
                    editor_index_to_linecol(&app->buf, i, &app->caret.line, &app->caret.col);
                    app->selecting = false;
                    return true;
                }
            }
            
            // Check position 0
            if (match_at_position(&app->buf, 0, needle, case_ins)) {
                editor_index_to_linecol(&app->buf, 0, &app->caret.line, &app->caret.col);
                app->selecting = false;
                return true;
            }
        }
        
        // Wrap around if enabled
        if (wrap) {
            for (size_t i = len - 1; i > start; i--) {
                if (match_at_position(&app->buf, i, needle, case_ins)) {
                    editor_index_to_linecol(&app->buf, i, &app->caret.line, &app->caret.col);
                    app->selecting = false;
                    return true;
                }
            }
        }
    }
    
    return false;
}

/**
 * Find previous occurrence
 */
bool find_previous(AppState *app, const wchar_t *needle, bool case_ins, bool wrap) {
    return find_next(app, needle, case_ins, wrap, false);
}

/**
 * Replace current selection with replacement text
 */
void find_replace(AppState *app, const wchar_t *needle, const wchar_t *replacement) {
    size_t start = editor_linecol_to_index(&app->buf, app->caret.line, app->caret.col);
    
    if (match_at_position(&app->buf, start, needle, app->find.case_insensitive)) {
        size_t needle_len = wcslen(needle);
        
        // Delete the needle
        gb_delete_range(&app->buf, start, needle_len);
        
        // Insert replacement
        gb_move_gap(&app->buf, start);
        gb_insert(&app->buf, replacement, wcslen(replacement));
        
        // Update caret position
        editor_index_to_linecol(&app->buf, start + wcslen(replacement), 
                               &app->caret.line, &app->caret.col);
        
        app->need_recount = true;
    }
}
