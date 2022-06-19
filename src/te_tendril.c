#include <stdio.h>

#include "te_scanner.h"
#include "te_tendril.h"

static void init_te_tendril_legend(TETendrilLegend *leg) {
    DD_INIT_ARRAY(&(leg->keys));
    DD_INIT_ARRAY(&(leg->values));
}

static void init_te_tendril(TETendril *tendril) {
    TETendrilLegend leg;
    init_te_tendril_legend(&leg);
    tendril->legend = leg;
    tendril->name = *copy_string("", 0);
}

static void error_at(TEToken *token, const char *msg) {
    fprintf(stderr, "[line %d] Error", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", msg);
}

static void advance(TEParser *parser, TEScanner *scanner) {
    parser->previous = parser->current;

    while (1) {
        parser->current = scan_te_token(scanner);
        if (parser->current.type != TOKEN_ERROR) break;

        error_at(&(parser->current), "scanner encountered an error");
    }
}

static void consume(TEParser *parser, TEScanner *scanner,
        TETokenType type, const char* msg) {
    if (parser->current.type == type) {
        advance(parser, scanner);
        return;
    }

    error_at(&(parser->current), msg);
}

static void parse_state_space(TEParser *parser, TEScanner *scanner,
        DDArrTETendril *tendrils) {
    TETendril tendril;
    DDString tmp_str;
    DDArrDDString tmp_dd_arr_str;

    DD_INIT_ARRAY(&tmp_dd_arr_str);
    init_te_tendril(&tendril);
    consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting state space name");
    tendril.name = *copy_string(parser->previous.start, parser->previous.length);

    consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

    while (parser->current.type != TOKEN_RIGHT_BRACE) {
        consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting key name");
        tmp_str = *copy_string(parser->previous.start, parser->previous.length);
        DD_ADD_ARRAY(&(tendril.legend.keys), tmp_str);

        consume(parser, scanner, TOKEN_COLON, "Expecting colon");
        
        consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value name");
        tmp_str = *copy_string(parser->previous.start, parser->previous.length);
        DD_ADD_ARRAY(&tmp_dd_arr_str, tmp_str);

        while (1) {
            if (parser->current.type == TOKEN_SEMICOLON) {
                advance(parser, scanner);
                break;
            } else {
                consume(parser, scanner, TOKEN_PIPE, "Expecting pipe");
                consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value name");
                tmp_str = *copy_string(parser->previous.start, parser->previous.length);
                DD_ADD_ARRAY(&tmp_dd_arr_str, tmp_str);
            }
        }

        DD_ADD_ARRAY(&(tendril.legend.values), tmp_dd_arr_str);
        DD_INIT_ARRAY(&tmp_dd_arr_str);
    }

    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
    
    DD_ADD_ARRAY(tendrils, tendril);
}

void parse_tendrils(TEScanner *scanner, DDArrTETendril *tendrils) {
    TEParser parser;

    advance(&parser, scanner);

    while (parser.current.type != TOKEN_EOF) {
        switch (parser.current.type) {
            case TOKEN_STATE_SPACE: {
                advance(&parser, scanner);
                parse_state_space(&parser, scanner, tendrils);
                break;
            }
            default:
                error_at(&(parser.current), "Unexpected token.");
        }
    }
}
