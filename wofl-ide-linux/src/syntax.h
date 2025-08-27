// Create new file: syntax.h
#ifndef SYNTAX_H
#define SYNTAX_H

#include "app.h"

typedef enum {
    TOKEN_NONE = 0,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_COMMENT,  
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_PUNCTUATION
} TokenType;

typedef struct {
    TokenType type;
    int start;
    int length;
    SDL_Color color;
} SyntaxToken;

void highlight_line(const char *line, int line_len, Language lang, SyntaxToken *tokens, int *token_count);

#endif