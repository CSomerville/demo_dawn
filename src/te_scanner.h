#ifndef te_scanner_h
#define te_scanner_h

#include <stdbool.h>
#include "dd_data.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} TEScanner;

typedef enum {
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COLON, TOKEN_SEMICOLON,
    TOKEN_PIPE,

    TOKEN_IDENTIFIER,

    TOKEN_STATE_SPACE,
    TOKEN_TRANSITION_ON,

    TOKEN_ERROR, TOKEN_EOF,
} TETokenType;

typedef struct {
    TETokenType type;
    const char* start;
    int length;
    int line;
} TEToken;
DD_DEF_ARRAY(TEToken, Tok);

void init_te_scanner(TEScanner* scanner, const char* source);
TEToken scan_te_token(TEScanner* scanner);

#endif
