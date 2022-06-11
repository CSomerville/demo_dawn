/* This code relies pretty heavily on the scanner in 
 * Crafting Interpreters by Robert Nystrom:
 * https://www.craftinginterpreters.com/ 
 */

#include <string.h>

#include "fe_scanner.h"

void init_scanner(Scanner* scanner, const char* source) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
}

static bool is_at_end(Scanner* scanner) {
    return *(scanner->current) == '\0';
}

static char advance(Scanner* scanner) {
    scanner->current++;
    return scanner->current[-1];
}

static char peek(Scanner* scanner) {
    return *(scanner->current);
}

/*static char peek_next(Scanner* scanner) {*/
    /*if (is_at_end(scanner)) return '\0';*/
    /*return scanner->current[1];*/
/*}*/

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static void skip_whitespace(Scanner* scanner) {
    for (;;) {
        char c = peek(scanner);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(scanner);
                break;
            case '\n':
                scanner->line++;
                advance(scanner);
                break;
            default:
                return;
        }
    }
}

static Token make_token(Scanner* scanner, TokenType token_type) {
    Token token;
    token.type = token_type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    return token;
}

static Token error_token(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;
    return token;
}

static TokenType check_keyword(Scanner* scanner, int start, int length,
        const char* rest, TokenType type) {
    if (scanner->current - scanner->start == start + length &&
            memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Scanner* scanner) {
    switch (scanner->start[0]) {
        case 'S': return check_keyword(scanner, 1, 9, "tateSpace", TOKEN_STATE_SPACE);
        case 'T': return check_keyword(scanner, 1, 11, "ransitionOn", TOKEN_TRANSITION_ON);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Scanner* scanner) {
    while (is_alpha(peek(scanner)) || is_digit(peek(scanner))) advance(scanner);
    return make_token(scanner, identifier_type(scanner));
}

Token scan_token(Scanner* scanner) {
    skip_whitespace(scanner);
    scanner->start = scanner->current;

    if (is_at_end(scanner)) return make_token(scanner, TOKEN_EOF);

    char c = advance(scanner);
    if (is_alpha(c)) return identifier(scanner);

    switch (c) {
        case '{': return make_token(scanner, TOKEN_LEFT_BRACE);
        case '}': return make_token(scanner, TOKEN_RIGHT_BRACE);
        case ';': return make_token(scanner, TOKEN_SEMICOLON);
        case ':': return make_token(scanner, TOKEN_COLON);
        case '|': return make_token(scanner, TOKEN_PIPE);
    }

    return error_token(scanner, "Unexpected token.");
}
