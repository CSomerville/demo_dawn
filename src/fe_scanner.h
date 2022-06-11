#ifndef fe_scanner_h
#define fe_scanner_h

#include <stdbool.h>
#include "dd_data.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

typedef enum {
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COLON, TOKEN_SEMICOLON,
    TOKEN_PIPE,

    TOKEN_IDENTIFIER,

    TOKEN_STATE_SPACE,
    TOKEN_TRANSITION_ON,

    TOKEN_ERROR, TOKEN_EOF,
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;
DD_DEF_ARRAY(Token, Tok);

void init_scanner(Scanner* scanner, const char* source);
Token scan_token(Scanner* scanner);

#endif
