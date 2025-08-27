// Create new file: syntax.c
#include "syntax.h"
#include <ctype.h>

// Language keywords
static const char* python_keywords[] = {
    "def", "class", "import", "from", "if", "else", "elif", "for", "while", 
    "return", "break", "continue", "pass", "try", "except", "finally",
    "with", "as", "lambda", "yield", "global", "nonlocal", "assert",
    "True", "False", "None", "and", "or", "not", "in", "is", NULL
};

static const char* c_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    "#include", "#define", "#ifdef", "#ifndef", "#endif", "#pragma", NULL
};

static const char* js_keywords[] = {
    "function", "var", "let", "const", "if", "else", "for", "while", "do",
    "break", "continue", "return", "switch", "case", "default", "try", "catch",
    "finally", "throw", "new", "this", "typeof", "instanceof", "in", "delete",
    "true", "false", "null", "undefined", NULL
};

static bool is_keyword(const char *word, int len, Language lang) {
    const char **keywords = NULL;
    
    switch (lang) {
        case LANG_PY:
            keywords = python_keywords;
            break;
        case LANG_C:
        case LANG_CPP:
            keywords = c_keywords;
            break;
        case LANG_JS:
            keywords = js_keywords;
            break;
        default:
            return false;
    }
    
    if (!keywords) return false;
    
    for (int i = 0; keywords[i]; i++) {
        if (strlen(keywords[i]) == len && strncmp(word, keywords[i], len) == 0) {
            return true;
        }
    }
    return false;
}

static SDL_Color get_token_color(TokenType type) {
    switch (type) {
        case TOKEN_KEYWORD:     return (SDL_Color){100, 150, 255, 255}; // Blue
        case TOKEN_STRING:      return (SDL_Color){150, 255, 150, 255}; // Green
        case TOKEN_COMMENT:     return (SDL_Color){128, 128, 128, 255}; // Gray
        case TOKEN_NUMBER:      return (SDL_Color){255, 200, 100, 255}; // Orange
        case TOKEN_OPERATOR:    return (SDL_Color){255, 150, 150, 255}; // Pink
        case TOKEN_IDENTIFIER:  return (SDL_Color){220, 220, 220, 255}; // White
        case TOKEN_PUNCTUATION: return (SDL_Color){200, 200, 200, 255}; // Light gray
        default:               return (SDL_Color){220, 220, 220, 255}; // Default white
    }
}

void highlight_line(const char *line, int line_len, Language lang, SyntaxToken *tokens, int *token_count) {
    *token_count = 0;
    if (line_len == 0) return;
    
    int pos = 0;
    int max_tokens = 64; // Reasonable limit per line
    
    while (pos < line_len && *token_count < max_tokens) {
        char ch = line[pos];
        
        // Skip whitespace
        if (isspace(ch)) {
            pos++;
            continue;
        }
        
        TokenType type = TOKEN_NONE;
        int token_start = pos;
        int token_len = 1;
        
        // Comments
        if (ch == '#' && lang == LANG_PY) {
            type = TOKEN_COMMENT;
            token_len = line_len - pos; // Rest of line
        } else if (pos < line_len - 1 && ch == '/' && line[pos + 1] == '/' && 
                   (lang == LANG_C || lang == LANG_CPP || lang == LANG_JS)) {
            type = TOKEN_COMMENT;
            token_len = line_len - pos; // Rest of line
        }
        // Strings
        else if (ch == '"' || ch == '\'') {
            type = TOKEN_STRING;
            char quote = ch;
            pos++; // Skip opening quote
            while (pos < line_len && line[pos] != quote) {
                if (line[pos] == '\\' && pos + 1 < line_len) {
                    pos += 2; // Skip escaped character
                } else {
                    pos++;
                }
            }
            if (pos < line_len) pos++; // Skip closing quote
            token_len = pos - token_start;
        }
        // Numbers
        else if (isdigit(ch)) {
            type = TOKEN_NUMBER;
            while (pos < line_len && (isdigit(line[pos]) || line[pos] == '.')) {
                pos++;
            }
            token_len = pos - token_start;
        }
        // Operators and punctuation
        else if (strchr("+-*/=<>!&|(){}[];,.", ch)) {
            type = (strchr("+-*/=<>!&|", ch)) ? TOKEN_OPERATOR : TOKEN_PUNCTUATION;
            // Handle multi-character operators
            if (pos < line_len - 1) {
                char next = line[pos + 1];
                if ((ch == '=' && next == '=') || 
                    (ch == '!' && next == '=') || 
                    (ch == '<' && next == '=') || 
                    (ch == '>' && next == '=') ||
                    (ch == '&' && next == '&') ||
                    (ch == '|' && next == '|')) {
                    token_len = 2;
                    pos++;
                }
            }
            pos++;
        }
        // Identifiers and keywords
        else if (isalpha(ch) || ch == '_') {
            while (pos < line_len && (isalnum(line[pos]) || line[pos] == '_')) {
                pos++;
            }
            token_len = pos - token_start;
            
            // Check if it's a keyword
            if (is_keyword(line + token_start, token_len, lang)) {
                type = TOKEN_KEYWORD;
            } else {
                type = TOKEN_IDENTIFIER;
            }
        } else {
            // Unknown character, skip
            pos++;
            continue;
        }
        
        // Add token
        if (type != TOKEN_NONE) {
            tokens[*token_count].type = type;
            tokens[*token_count].start = token_start;
            tokens[*token_count].length = token_len;
            tokens[*token_count].color = get_token_color(type);
            (*token_count)++;
        }
        
        pos = token_start + token_len;
    }
}