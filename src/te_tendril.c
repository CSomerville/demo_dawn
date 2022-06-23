#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

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

static TETendril* lookup_tendril_by_name(DDArrTETendril *tendrils,
		DDString *name) {
    int i;
    for (i = 0; i < tendrils->size; i++) {
        if (strcmp(name->chars, tendrils->elems[i].name.chars) == 0) {
            return &(tendrils->elems[i]);
        }
    }
    return NULL;
}

static int key_index_tendril(TETendril *tendril, DDString *key) {
	int i;
	for (i = 0; i < tendril->legend.keys.size; i++) {
		if (strcmp(tendril->legend.keys.elems[i].chars,
					key->chars) == 0)
			return i;
	}
	return -1;
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
	// TODO: do something better.
	exit(1);
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
    consume(parser, scanner, TOKEN_IDENTIFIER,
			"Expecting state space name");
    tendril.name = *copy_string(parser->previous.start,
			parser->previous.length);

    consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

    while (parser->current.type != TOKEN_RIGHT_BRACE) {
        consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting key name");
        tmp_str = *copy_string(parser->previous.start,
				parser->previous.length);
        DD_ADD_ARRAY(&(tendril.legend.keys), tmp_str);

        consume(parser, scanner, TOKEN_COLON, "Expecting colon");
        
        consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value name");
        tmp_str = *copy_string(parser->previous.start,
				parser->previous.length);
        DD_ADD_ARRAY(&tmp_dd_arr_str, tmp_str);

        while (1) {
            if (parser->current.type == TOKEN_SEMICOLON) {
                advance(parser, scanner);
                break;
            } else {
                consume(parser, scanner, TOKEN_PIPE, "Expecting pipe");
                consume(parser, scanner, TOKEN_IDENTIFIER,
						"Expecting value name");
                tmp_str = *copy_string(parser->previous.start,
						parser->previous.length);
                DD_ADD_ARRAY(&tmp_dd_arr_str, tmp_str);
            }
        }

        DD_ADD_ARRAY(&(tendril.legend.values), tmp_dd_arr_str);
        DD_INIT_ARRAY(&tmp_dd_arr_str);
    }

    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
    
    DD_ADD_ARRAY(tendrils, tendril);
}

static void parse_next(TEParser *parser, TEScanner *scanner,
		DDArrDDArrDDString *next) {
	DDArrDDString tmp_line;
	DDString *tmp_str;

	DD_INIT_ARRAY(&tmp_line);

    consume(parser, scanner, TOKEN_LEFT_BRACE, "next expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER,
				"next expecting key name");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		// check for key membership
		DD_ADD_ARRAY(&tmp_line, *tmp_str);

		consume(parser, scanner, TOKEN_COLON,
				"next expecting colon");

		consume(parser, scanner, TOKEN_IDENTIFIER,
				"next expecting value name");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		// check for value membership
		DD_ADD_ARRAY(&tmp_line, *tmp_str);

		consume(parser, scanner, TOKEN_SEMICOLON,
				"next expecting semicolon");

		DD_ADD_ARRAY(next, tmp_line);
		DD_INIT_ARRAY(&tmp_line);
	}

	consume(parser, scanner, TOKEN_RIGHT_BRACE,
			"next expecting right brace");
}

static void parse_current(TEParser *parser, TEScanner *scanner,
		DDArrDDArrDDString *current) {
	DDArrDDString tmp_line;
	DDString *tmp_str;

	DD_INIT_ARRAY(&tmp_line);

    consume(parser, scanner, TOKEN_LEFT_BRACE,
			"current expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER,
				"current expecting key name");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		// check for key membership
		DD_ADD_ARRAY(&tmp_line, *tmp_str);

		consume(parser, scanner, TOKEN_COLON,
				"current expecting colon");

		consume(parser, scanner, TOKEN_IDENTIFIER,
				"current expecting value name");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		// check for value membership
		DD_ADD_ARRAY(&tmp_line, *tmp_str);

        while (1) {
            if (parser->current.type == TOKEN_SEMICOLON) {
                advance(parser, scanner);
                break;
            } else {
                consume(parser, scanner, TOKEN_PIPE, "Expecting pipe");
                consume(parser, scanner, TOKEN_IDENTIFIER,
						"Expecting value name");
				tmp_str = copy_string(parser->previous.start,
						parser->previous.length);
				// check for value membership
				DD_ADD_ARRAY(&tmp_line, *tmp_str);
			}
		}
		DD_ADD_ARRAY(current, tmp_line);
		DD_INIT_ARRAY(&tmp_line);
	}

	consume(parser, scanner, TOKEN_RIGHT_BRACE,
			"current expecting right brace");
}

static void parse_transition_on(TEParser *parser, TEScanner *scanner,
        DDArrTETendril *tendrils) {
    TETendril *tendril;
    DDString *name_to_find;
	DDArrDDArrDDString next;
	DDArrDDArrDDString current;

	DD_INIT_ARRAY(&next);
    DD_INIT_ARRAY(&current);

    consume(parser, scanner, TOKEN_IDENTIFIER,
			"Transition expecting tendril name");
    name_to_find = copy_string(parser->previous.start,
			parser->previous.length);
    tendril = lookup_tendril_by_name(tendrils, name_to_find);

    if (tendril == NULL) {
        // lol do something.
    }

    consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

	if (parser->current.type == TOKEN_NEXT) {
		advance(parser, scanner);
		parse_next(parser, scanner, &next);
		consume(parser, scanner, TOKEN_CURRENT, "Expecting current");
		parse_current(parser, scanner, &current);
	} else if (parser->current.type == TOKEN_CURRENT) {
		advance(parser, scanner);
		parse_current(parser, scanner, &current);
		consume(parser, scanner, TOKEN_NEXT, "Expecting next");
		parse_next(parser, scanner, &next);
	} else {
		error_at(&(parser->current), "Expecting current or next");
	}

	// tmp test
	for (int i = 0; i < current.size; i++) {
		for (int j = 0; j < current.elems[i].size; j++) {
			printf("%s ", current.elems[i].elems[j].chars);
		}
		printf("\n");
	}

	for (int i = 0; i < next.size; i++) {
		for (int j = 0; j < next.elems[i].size; j++) {
			printf("%s ", next.elems[i].elems[j].chars);
		}
		printf("\n");
	}
	
	/*add_transitions(tendril, &intermediate_representation);*/

    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
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
            case TOKEN_TRANSITION_ON: {
                advance(&parser, scanner);
                parse_transition_on(&parser, scanner, tendrils);
                break;
            }
            default:
                error_at(&(parser.current), "Unexpected token.");
        }
    }
}
