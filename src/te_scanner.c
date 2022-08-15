/* This code relies pretty heavily on the scanner in 
 * Crafting Interpreters by Robert Nystrom:
 * https://www.craftinginterpreters.com/ 
 */

#include <string.h>

#include "te_scanner.h"

void init_te_scanner(TEScanner* scanner, const char* source) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
}

static bool is_at_end(TEScanner* scanner) {
    return *(scanner->current) == '\0';
}

static char advance(TEScanner* scanner) {
    scanner->current++;
    return scanner->current[-1];
}

static char peek(TEScanner* scanner) {
    return *(scanner->current);
}

static char peek_next(TEScanner* scanner) {
	if (is_at_end(scanner)) return '\0';
	return scanner->current[1];
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static void skip_whitespace(TEScanner* scanner) {
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
			case '/':
				if (peek_next(scanner) == '/') {
					while (peek(scanner) != '\n' && !is_at_end(scanner))
						advance(scanner);
				} else {
					return;
				}
				break;
            default:
                return;
        }
    }
}

static TEToken make_token(TEScanner* scanner, TETokenType token_type) {
    TEToken token;
    token.type = token_type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    return token;
}

static TEToken error_token(TEScanner* scanner, const char* message) {
    TEToken token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;
    return token;
}

static TETokenType check_keyword(TEScanner* scanner, int start, int length,
        const char* rest, TETokenType type) {
    if (scanner->current - scanner->start == start + length &&
            memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TETokenType identifier_type(TEScanner* scanner) {
    switch (scanner->start[0]) {
		case 'C': return check_keyword(scanner, 1, 8, "ontentOn",
						  TOKEN_CONTENT_ON);
        case 'S': 
			if (scanner->current - scanner->start > 3 &&
					scanner->start[1] == 't' &&
					scanner->start[2] == 'a') {
				switch (scanner->start[3]) {
					case 't': return check_keyword(scanner, 4, 6,
									  "eSpace", TOKEN_STATE_SPACE);
					case 'r': return check_keyword(scanner, 4, 3,
									  "tOn", TOKEN_START_ON);
				}
			}
			break;
        case 'T': return check_keyword(scanner, 1, 11, "ransitionOn",
						  TOKEN_TRANSITION_ON);
		case 'c':
			if (scanner->current - scanner->start > 1) {
				switch (scanner->start[1]) {
					case 'o':
						return check_keyword(scanner, 2, 5, "ntent",
								TOKEN_CONTENT);
					case 'u':
					  	return check_keyword(scanner, 2, 5, "rrent",
							  TOKEN_CURRENT);
				}
			}
			break;
		case 'm': return check_keyword(scanner, 1, 4, "atch",
						  TOKEN_MATCH);
		case 'n': return check_keyword(scanner, 1, 3, "ext",
						  TOKEN_NEXT);
		case 's': return check_keyword(scanner, 1, 5, "witch",
						  TOKEN_SWITCH);
    }
    return TOKEN_IDENTIFIER;
}

static TEToken identifier(TEScanner* scanner) {
    while (is_alpha(peek(scanner)) || is_digit(peek(scanner))) advance(scanner);
    return make_token(scanner, identifier_type(scanner));
}

static TEToken string(TEScanner *scanner) {
	while (peek(scanner) != '"' && !is_at_end(scanner)) {
		if (peek(scanner) == '\n') scanner->line++;
		advance(scanner);
	}

	if (is_at_end(scanner)) return error_token(scanner, "Unterminated string.");

	advance(scanner);
	return make_token(scanner, TOKEN_STRING);
}

TEToken scan_te_token(TEScanner* scanner) {
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
		case '"': return string(scanner);
    }

    return error_token(scanner, "Unexpected token.");
}
