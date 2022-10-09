#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "dd_algo.h"
#include "te_scanner.h"
#include "te_tendril.h"

static void init_te_tendril_legend(TETendrilLegend *leg) {
    DD_INIT_ARRAY(&(leg->keys));
    DD_INIT_ARRAY(&(leg->values));
}

static void free_te_tendril_legend(TETendrilLegend *leg) {
	int i, j;
	for (i = 0; i < leg->keys.size; i++) {
		free_dd_chars(&(leg->keys.elems[i]));
	}
	DD_FREE_ARRAY(&(leg->keys));
	for (i = 0; i < leg->values.size; i++) {
		for (j = 0; j < leg->values.elems[i].size; j++) {
			free_dd_chars(&(
						leg->values.elems[i].elems[j]));
		}
		DD_FREE_ARRAY(&(leg->values.elems[i]));
	}
	DD_FREE_ARRAY(&(leg->values));
}

static void free_te_tendril_content(TEContent *con) {
	int i, j;

	DD_FREE_ARRAY(&con->match);

	for (i = 0; i < con->contents.size; i++) {
		if (con->contents.elems[i].type == TE_CONTENT_ELEM_TYPE_STR) {
			free_dd_chars(&con->contents.elems[i].datum.s.contents);
		} else {
			for (j = 0; j < con->contents.elems[i].datum.i.contents.size; j++) {
				free_dd_chars(
					&con->contents.elems[i].datum.i.contents.elems[j].contents
				);
			}
			DD_FREE_ARRAY(&con->contents.elems[i].datum.i.contents);
		}
	}
	DD_FREE_ARRAY(&con->contents);
}

static void free_te_tendril_contents(DDArrTEContent *cons) {
	int i;
	for (i = 0; i < cons->size; i++) {
		free_te_tendril_content(&cons->elems[i]);
	}
	DD_FREE_ARRAY(cons);
}

void print_tendril_legend(TETendrilLegend *leg) {
	int i, j;
	for (i = 0; i < leg->keys.size; i++) {
		printf("%s: ", leg->keys.elems[i].chars);
		for (j = 0; j < leg->values.elems[i].size; j++) {
			printf("%s ", leg->values.elems[i].elems[j].chars);
		}
		printf("\n");
	}
}

void print_te_state(int state, TETendril *tendril) {
	DDArrInt vals;
	int i;
	DD_INIT_ARRAY(&vals);
	DD_INIT_ARRAY_SIZE(&vals, tendril->legend.keys.size);
	values_from_int(state, &vals, &tendril->legend);
	printf("plaintext for state: %d\n", state);
	for (i = 0; i < tendril->legend.values.size; i++) {
		printf("key: %s, value: %s\n",
			tendril->legend.keys.elems[i].chars,
			tendril->legend.values.elems[i].elems[vals.elems[i]].chars);
	}
	printf("*****\n");
	DD_FREE_ARRAY(&vals);
}

static void init_tendril_content(TEContent *con) {
	DD_INIT_ARRAY(&con->match);
	DD_INIT_ARRAY(&con->contents);
}

static void init_te_tendril(TETendril *tendril) {
    TETendrilLegend leg;
	DDArrTEContent contents;

    init_te_tendril_legend(&leg);
	DD_INIT_ARRAY(&contents);

    tendril->legend = leg;
	tendril->name.chars = NULL;
	give_to_dd_string(&(tendril->name), "", 0);
	tendril->graph = NULL;
	tendril->start = -1;
	tendril->contents = contents;
}

void free_te_tendril(TETendril *tendril) {
	free_dd_chars(&(tendril->name));
	free_te_tendril_legend(&(tendril->legend));
	free_graph_members(tendril->graph);
	free_te_tendril_contents(&tendril->contents);
	free(tendril->graph);
}

void free_te_tendrils(DDArrTETendril *tendrils) {
	int i;
	for (i = 0; i < tendrils->size; i++)
		free_te_tendril(&(tendrils->elems[i]));
}

static void init_tendril_graph(TETendril *tendril) {
	int i, c;
	DDGraph *g;

	c = 1;
	for (i = 0; i < tendril->legend.values.size; i++) {
		c = c * tendril->legend.values.elems[i].size;
	}
	g = DD_ALLOCATE(DDGraph, 1);
	initialize_graph(g, true, c);
	tendril->graph = g;
}

TETendril* lookup_tendril_by_name(DDArrTETendril *tendrils,
		DDString *name) {
    int i;
    for (i = 0; i < tendrils->size; i++) {
        if (strcmp(name->chars, tendrils->elems[i].name.chars) == 0) {
            return &(tendrils->elems[i]);
        }
    }
    return NULL;
}

int key_index_tendril(TETendril *tendril, DDString *key) {
	int i;
	for (i = 0; i < tendril->legend.keys.size; i++) {
		if (strcmp(tendril->legend.keys.elems[i].chars,
					key->chars) == 0)
			return i;
	}
	return -1;
}

int value_index_tendril(TETendril *tendril, DDString *value, int key_idx) {
	int i;
	for (i = 0; i < tendril->legend.values.elems[key_idx].size; i++) {
		if (strcmp(tendril->legend.values.elems[key_idx].elems[i].chars,
					value->chars) == 0)
			return i;
	}
	return -1;
}

int int_from_values(DDArrInt *values, TETendrilLegend *leg) {
	int i, j, n, m;
	n = 0;
	for (i = 0; i < values->size; i++) {
		m = 1;
		for (j = 0; j < i; j++) {
			m = m * leg->values.elems[j].size;
		}
		n += m * values->elems[i];
	}
	return n;
}

/* vals must be sufficiently large to hold leg->keys.size values */
void values_from_int(int x, DDArrInt *vals, TETendrilLegend *leg) {
	int i, j, n, m;

	n = x;
	for (i = leg->keys.size - 1; i >= 0; i--) {
		m = 1;
		for (j = 0; j < i; j++) {
			m = m * leg->values.elems[j].size;
		}
		vals->elems[i] = n / m;
		n = n % m;
	}
}

int te_transition(TETendril *tendril, int current) {
	int r;
	DDArrInt next = tendril->graph->edges.elems[current];
	if (next.size == 0) {
		fprintf(stderr, "no transitions for %d\n", current);
		exit(1);
	}
	r = rand() % next.size;
	return next.elems[r];
}

/* writing out what I think this algo is/should be:
 * convert cur and next to bool arrays?
 * for current, convert bool array to series of states (int array).
 * for each state of current, calculate next and add to graph.
 */ 

static void strs_to_int_vals(TETendrilLegend *legend,
		DDArrDDArrDDString *strs, DDArrDDArrInt *val_list) {
	int i, j, k, idx;
	DDArrInt vals;

	for (i = 0; i < legend->keys.size; i++) {
		DD_INIT_ARRAY(&vals);
		idx = -1;
		for (j = 0; j < strs->size; j++) {
			if (!strcmp(legend->keys.elems[i].chars,
						strs->elems[j].elems[0].chars)) {
				idx = j;
				break;
			}
		}
		if (idx >= 0) {
			for (j = 0; j < legend->values.elems[i].size; j++) {
				for (k = 1; k < strs->elems[idx].size; k++) {
					if (!strcmp(legend->values.elems[i].elems[j].chars,
								strs->elems[idx].elems[k].chars)) {
						DD_ADD_ARRAY(&vals, j);
					}
				}
			}
		}
		DD_ADD_ARRAY(val_list, vals);
	}
}

static void fill_in_cur_vals(TETendrilLegend *legend,
		DDArrDDArrInt *cur_vals) {
	int i, j;
	for (i = 0; i < cur_vals->size; i++) {
		if (cur_vals->elems[i].size == 0) {
			for (j = 0; j < legend->values.elems[i].size; j++) {
				DD_ADD_ARRAY(&cur_vals->elems[i], j);
			}
		}
	}
}

static int next_from_cur(TETendrilLegend *legend, DDArrInt *cur,
		DDArrDDArrInt *next) {
	DDArrInt next_final_arr;
	int i, next_final;

	DD_INIT_ARRAY(&next_final_arr);
	for (i = 0; i < cur->size; i++) {
		if (next->elems[i].size == 0) {
			DD_ADD_ARRAY(&next_final_arr, cur->elems[i]);
		} else {
			DD_ADD_ARRAY(&next_final_arr, next->elems[i].elems[0]);
		}
	}

	next_final = int_from_values(&next_final_arr, legend);
	DD_FREE_ARRAY(&next_final_arr);
	return next_final;
}


static void add_transitions(TETendril *tendril,
		DDArrDDArrDDString *current, DDArrDDArrDDString *next) {
	DDArrDDArrInt vals;
	DDArrDDArrInt *current_states;
	int i, next_int, cur_int;

	DD_INIT_ARRAY(&vals);
	strs_to_int_vals(&tendril->legend, current, &vals);
	fill_in_cur_vals(&tendril->legend, &vals);

	current_states = cartesian_product(&vals);

	for (i = 0; i < vals.size; i++) {
		DD_FREE_ARRAY(&vals.elems[i]);
	}
	DD_FREE_ARRAY(&vals);

	DD_INIT_ARRAY(&vals);
	strs_to_int_vals(&tendril->legend, next, &vals);

	for (i = 0; i < current_states->size; i++) {
		next_int = next_from_cur(&tendril->legend,
				&current_states->elems[i], &vals);
		cur_int = int_from_values(&current_states->elems[i],
				&tendril->legend);
		if (!(edge_in_graph(tendril->graph, cur_int, next_int))) {
			insert_edge(tendril->graph, cur_int, next_int,
					tendril->graph->directed);
		}
	}

	for (i = 0; i < vals.size; i++) {
		DD_FREE_ARRAY(&vals.elems[i]);
	}
	DD_FREE_ARRAY(&vals);
	for (i = 0; i < current_states->size; i++) {
		DD_FREE_ARRAY(&current_states->elems[i]);
	}
	DD_FREE_ARRAY(current_states);
	free(current_states);
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

	tmp_str.chars = NULL;
    DD_INIT_ARRAY(&tmp_dd_arr_str);
    init_te_tendril(&tendril);
    consume(parser, scanner, TOKEN_IDENTIFIER,
			"Expecting state space name");
	give_to_dd_string(&tendril.name, parser->previous.start,
			parser->previous.length);

    consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

    while (parser->current.type != TOKEN_RIGHT_BRACE) {
        consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting key name");
		give_to_dd_string(&tmp_str, parser->previous.start,
				parser->previous.length);
        DD_ADD_ARRAY(&(tendril.legend.keys), tmp_str);
		tmp_str.chars = NULL;

        consume(parser, scanner, TOKEN_COLON, "Expecting colon");
        
        consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value name");
		give_to_dd_string(&tmp_str, parser->previous.start,
				parser->previous.length);
        DD_ADD_ARRAY(&tmp_dd_arr_str, tmp_str);
		tmp_str.chars = NULL;

        while (1) {
            if (parser->current.type == TOKEN_SEMICOLON) {
                advance(parser, scanner);
                break;
            } else {
                consume(parser, scanner, TOKEN_PIPE, "Expecting pipe");
                consume(parser, scanner, TOKEN_IDENTIFIER,
						"Expecting value name");
				give_to_dd_string(&tmp_str, parser->previous.start,
						parser->previous.length);
                DD_ADD_ARRAY(&tmp_dd_arr_str, tmp_str);
				tmp_str.chars = NULL;
            }
        }

        DD_ADD_ARRAY(&(tendril.legend.values), tmp_dd_arr_str);
        DD_INIT_ARRAY(&tmp_dd_arr_str);
    }

    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
    
	init_tendril_graph(&tendril);
    DD_ADD_ARRAY(tendrils, tendril);
}

static void parse_next(TEParser *parser, TEScanner *scanner,
		DDArrDDArrDDString *next) {
	DDArrDDString tmp_line;
	DDString tmp_str;

	tmp_str.chars = NULL;
	DD_INIT_ARRAY(&tmp_line);

    consume(parser, scanner, TOKEN_LEFT_BRACE, "next expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER,
				"next expecting key name");
		give_to_dd_string(&tmp_str, parser->previous.start,
				parser->previous.length);
		// check for key membership
		DD_ADD_ARRAY(&tmp_line, tmp_str);
		tmp_str.chars = NULL;

		consume(parser, scanner, TOKEN_COLON,
				"next expecting colon");

		consume(parser, scanner, TOKEN_IDENTIFIER,
				"next expecting value name");

		give_to_dd_string(&tmp_str, parser->previous.start,
				parser->previous.length);
		// check for value membership
		DD_ADD_ARRAY(&tmp_line, tmp_str);
		tmp_str.chars = NULL;

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
	DDString tmp_str;

	tmp_str.chars = NULL;
	DD_INIT_ARRAY(&tmp_line);

    consume(parser, scanner, TOKEN_LEFT_BRACE,
			"current expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER,
				"current expecting key name");
		give_to_dd_string(&tmp_str, parser->previous.start,
				parser->previous.length);
		// check for key membership
		DD_ADD_ARRAY(&tmp_line, tmp_str);
		tmp_str.chars = NULL;

		consume(parser, scanner, TOKEN_COLON,
				"current expecting colon");

		consume(parser, scanner, TOKEN_IDENTIFIER,
				"current expecting value name");
		give_to_dd_string(&tmp_str, parser->previous.start,
				parser->previous.length);
		// check for value membership
		DD_ADD_ARRAY(&tmp_line, tmp_str);
		tmp_str.chars = NULL;

        while (1) {
            if (parser->current.type == TOKEN_SEMICOLON) {
                advance(parser, scanner);
                break;
            } else {
                consume(parser, scanner, TOKEN_PIPE, "Expecting pipe");
                consume(parser, scanner, TOKEN_IDENTIFIER,
						"Expecting value name");
				give_to_dd_string(&tmp_str, parser->previous.start,
						parser->previous.length);
				// check for value membership
				DD_ADD_ARRAY(&tmp_line, tmp_str);
				tmp_str.chars = NULL;
			}
		}
		DD_ADD_ARRAY(current, tmp_line);
		DD_INIT_ARRAY(&tmp_line);
	}

	consume(parser, scanner, TOKEN_RIGHT_BRACE,
			"current expecting right brace");
}

static TETendril *parse_tendril_name(TEParser *parser, TEScanner *scanner,
		DDArrTETendril *tendrils) {
	TETendril *tendril;
	DDString *tmp_str;
	
	consume(parser, scanner, TOKEN_IDENTIFIER, "expecting tendril name.");
	tmp_str = copy_string(parser->previous.start,
			parser->previous.length);
	tendril = lookup_tendril_by_name(tendrils, tmp_str);

	if (tendril == NULL) {
		error_at(&(parser->previous), "Tendril name not found.");
	}
	free_string(tmp_str);
	return tendril;
}

static void parse_transition_on(TEParser *parser, TEScanner *scanner,
        DDArrTETendril *tendrils) {
    TETendril *tendril;
	DDArrDDArrDDString next;
	DDArrDDArrDDString current;
	int i;

	DD_INIT_ARRAY(&next);
    DD_INIT_ARRAY(&current);

	tendril = parse_tendril_name(parser, scanner, tendrils);

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

	add_transitions(tendril, &current, &next);

	for (i = 0; i < next.size; i++) {
		free_dd_arr_dd_str(&next.elems[i]);
	}
	DD_FREE_ARRAY(&next);
	for (i = 0; i < current.size; i++) {
		free_dd_arr_dd_str(&current.elems[i]);
	}
	DD_FREE_ARRAY(&current);

    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
}

void parse_start_on(TEParser *parser, TEScanner *scanner,
		DDArrTETendril *tendrils) {
    TETendril *tendril;
    DDString *tmp_str;
	DDArrInt vals;
	int i, n, m;

	tendril = parse_tendril_name(parser, scanner, tendrils);

	DD_INIT_ARRAY(&vals);
	DD_INIT_ARRAY_SIZE(&vals, tendril->legend.keys.size);
	for (i = 0; i < tendril->legend.keys.size; i++) vals.elems[i] = -1;

    consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting identifier");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		n = key_index_tendril(tendril, tmp_str);
		free_string(tmp_str);
		if (n == -1) {
			error_at(&(parser->previous), "key not found for tendril");
		}

		consume(parser, scanner, TOKEN_COLON, "Expecting colon");
		consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting identifier");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		m = value_index_tendril(tendril, tmp_str, n);
		free_string(tmp_str);
		if (m == -1) {
			error_at(&(parser->previous), "value not found for tendril");
		}

		vals.elems[n] = m;

		consume(parser, scanner, TOKEN_SEMICOLON, "Expecting semicolon");
	}

	for (i = 0; i < tendril->legend.keys.size; i++) {
		if (vals.elems[i] == -1) {
			fprintf(stderr, "StartOn: missing key");
			exit(1);
		}
	}

	tendril->start = int_from_values(&vals, &(tendril->legend));
	DD_FREE_ARRAY(&vals);
    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
}


static void parse_match(TEParser *parser, TEScanner  *scanner,
		TETendril *tendril, TEContent *content) {
	DDArrDDArrInt match_patterns;
	DDArrDDArrInt *match_patterns_2;
	DDArrInt dd_arr_int;
	DDString *tmp_str;
	int i, j, n, m;

	DD_INIT_ARRAY(&dd_arr_int);
	DD_INIT_ARRAY(&match_patterns);

	for (i = 0; i < tendril->legend.keys.size; i++) {
		DD_ADD_ARRAY(&match_patterns, dd_arr_int);
	}

	DD_INIT_ARRAY(&dd_arr_int);

	consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting key");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		n = key_index_tendril(tendril, tmp_str);
		if (n == -1) {
			error_at(&(parser->previous), "key not found for tendril");
		}
		free_string(tmp_str);

		consume(parser, scanner, TOKEN_COLON, "Expecting colon.");

		consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value.");
		tmp_str = copy_string(parser->previous.start,
				parser->previous.length);
		m = value_index_tendril(tendril, tmp_str, n);
		free_string(tmp_str);
		if (m == -1) {
			error_at(&(parser->previous), "value not found for tendril");
		}
		DD_ADD_ARRAY(&(match_patterns.elems[n]), m);

		while (parser->current.type != TOKEN_SEMICOLON) {
			consume(parser, scanner, TOKEN_PIPE, "Expecting pipe.");
			consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value.");
			tmp_str = copy_string(parser->previous.start,
					parser->previous.length);
			m = value_index_tendril(tendril, tmp_str, n);
			free_string(tmp_str);
			if (m == -1) {
				error_at(&(parser->previous), "value not found for tendril");
			}
			DD_ADD_ARRAY(&(match_patterns.elems[n]), m);
		}
		consume(parser, scanner, TOKEN_SEMICOLON, "Expecting semicolon");
	}

	consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");

	for (i = 0; i < match_patterns.size; i++) {
		if (match_patterns.elems[i].size == 0) {
			for (j = 0; j < tendril->legend.values.elems[i].size; j++) {
				DD_ADD_ARRAY(&(match_patterns.elems[i]), j);
			}
		}
	}

	match_patterns_2 = cartesian_product(&match_patterns);

	DD_INIT_ARRAY(&dd_arr_int);
	for (i = 0; i < match_patterns_2->size; i++) {
		n = int_from_values(&(match_patterns_2->elems[i]), &(tendril->legend));
		DD_ADD_ARRAY(&dd_arr_int, n);
	}

	content->match = dd_arr_int;

	for (i = 0; i < match_patterns.size; i++) {
		DD_FREE_ARRAY(&match_patterns.elems[i]);
	}
	DD_FREE_ARRAY(&match_patterns);

	for (i = 0; i < match_patterns_2->size; i++) {
		DD_FREE_ARRAY(&match_patterns_2->elems[i]);
	}
	DD_FREE_ARRAY(match_patterns_2);
	free(match_patterns_2);
}

/* this just removes the quotes from the string token */
static void parse_str(DDString *str, TEParser *parser) {
	give_to_dd_string(str, parser->previous.start + 1,
			parser->previous.length - 2);
}

static void parse_string(TEParser *parser, TEScanner *scanner,
		TEContent *content) {
	TEContentElem content_elem;
	TEContentStr content_str;

	advance(parser, scanner);

	content_str.contents.chars = NULL;
	parse_str(&content_str.contents, parser);

	content_elem.type = TE_CONTENT_ELEM_TYPE_STR;
	content_elem.datum.s = content_str;

	DD_ADD_ARRAY(&content->contents, content_elem);

	consume(parser, scanner, TOKEN_SEMICOLON, "Expecting semicolon.");
}

static void parse_switch(TEParser *parser, TEScanner *scanner,
		TETendril *tendril, TEContent *content) {
	TEContentElem te_content_elem;
	TEInterpolation te_interpolation;
	TEInterpolationElem te_interpolation_elem;
	int n, m;
	DDString *key;
	DDString *val;

	DD_INIT_ARRAY(&(te_interpolation.contents));

	advance(parser, scanner);
	consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting key identifier");
	key = copy_string(parser->previous.start,
			parser->previous.length);
	n = key_index_tendril(tendril, key);
	free_string(key);
	if (n == -1) {
		error_at(&(parser->previous), "key not found for tendril");
	}
	te_interpolation.key = n;

	consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		consume(parser, scanner, TOKEN_IDENTIFIER, "Expecting value");
		val = copy_string(parser->previous.start,
				parser->previous.length);
		m = value_index_tendril(tendril, val, n);
		free_string(val);
		if (n == -1) {
			error_at(&(parser->previous), "value not found for tendril");
		}
		te_interpolation_elem.value = m;

		consume(parser, scanner, TOKEN_COLON, "Expecting colon");
		consume(parser, scanner, TOKEN_STRING, "Expecting string");

		te_interpolation_elem.contents.chars = NULL;
		parse_str(&te_interpolation_elem.contents, parser);

		DD_ADD_ARRAY(&te_interpolation.contents, te_interpolation_elem);

		consume(parser, scanner, TOKEN_SEMICOLON, "Expecting semicolon");
	}
	// check exhaustiveness

	consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace.");

	te_content_elem.type = TE_CONTENT_ELEM_TYPE_INTER;
	te_content_elem.datum.i = te_interpolation;

	DD_ADD_ARRAY(&content->contents, te_content_elem);
}

static void parse_content(TEParser *parser, TEScanner  *scanner,
		TETendril *tendril, TEContent *content) {

	consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace.");

	while (parser->current.type != TOKEN_RIGHT_BRACE) {
		if (parser->current.type == TOKEN_STRING) {
			parse_string(parser, scanner, content);
		} else if (parser->current.type == TOKEN_SWITCH) {
			parse_switch(parser, scanner, tendril, content);
		} else {
			error_at(&(parser->current), "Expecting string or switch");
		}
	}

	consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");
}

static void parse_content_on(TEParser *parser, TEScanner *scanner,
		DDArrTETendril *tendrils) {
	TETendril *tendril;
	TEContent *content;

	content = DD_ALLOCATE(TEContent, 1);
	init_tendril_content(content);
	tendril = parse_tendril_name(parser, scanner, tendrils);

    consume(parser, scanner, TOKEN_LEFT_BRACE, "Expecting left brace");

	if (parser->current.type == TOKEN_MATCH) {
		advance(parser, scanner);
		parse_match(parser, scanner, tendril, content);
		consume(parser, scanner, TOKEN_CONTENT, "Expecting content");
		parse_content(parser, scanner, tendril, content);
	} else if (parser->current.type == TOKEN_CONTENT) {
		advance(parser, scanner);
		parse_content(parser, scanner, tendril, content);
		consume(parser, scanner, TOKEN_CONTENT, "Expecting match");
		parse_match(parser, scanner, tendril, content);
	} else {
		error_at(&(parser->current), "Expecting match or content");
	}

    consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expecting right brace");

	DD_ADD_ARRAY(&(tendril->contents), *content);
	free(content);
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
			case TOKEN_START_ON: {
				advance(&parser, scanner);
				parse_start_on(&parser, scanner, tendrils);
				break;
			}
			case TOKEN_CONTENT_ON: {
				advance(&parser, scanner);
				parse_content_on(&parser, scanner, tendrils);
				break;
		   	}
            default:
                error_at(&(parser.current), "Uhnexpected token.");
        }
    }
}

static void get_te_tendril_content_matches(DDArrInt *match_indices,
		int state, TETendril *tendril) {
	int i, j;
	for (i = 0; i < tendril->contents.size; i++) {
		for (j = 0; j < tendril->contents.elems[i].match.size; j++) {
			if (tendril->contents.elems[i].match.elems[j] == state) {
				DD_ADD_ARRAY(match_indices, i);
			}
		}
	}
}

static void process_content_elem(DDString *str, TEContentElem *elem,
		DDArrInt *state_values) {
	int i;
	if (elem->type == TE_CONTENT_ELEM_TYPE_STR) {
		give_to_dd_string(str, elem->datum.s.contents.chars,
			elem->datum.s.contents.length);
	} else {
		for (i = 0; i < elem->datum.i.contents.size; i++) {
			if (state_values->elems[elem->datum.i.key] ==
					elem->datum.i.contents.elems[i].value) {
				give_to_dd_string(str, 
					elem->datum.i.contents.elems[i].contents.chars,
					elem->datum.i.contents.elems[i].contents.length);
				break;
			}
		}
	}
}

static void collect_contents(DDArrDDString *collect,
		DDArrInt *state_values, TEContent *content) {
	int i;
	DDString tmp_str;
	
	for (i = 0; i < content->contents.size; i++) {
		tmp_str.chars = NULL;
		process_content_elem(&tmp_str, &content->contents.elems[i],
				state_values);
		DD_ADD_ARRAY(collect, tmp_str);
	}
}


int get_te_tendril_content(DDArrDDArrDDString *result, int state,
		TETendril *tendril) {
	int i;
	DDArrInt match_indices;
	DDArrInt state_values;
	DDArrDDString tmp_str_arr;

	DD_INIT_ARRAY(&match_indices);
	get_te_tendril_content_matches(&match_indices, state, tendril);

	if (match_indices.size == 0)
		return 0;

	DD_INIT_ARRAY(&state_values);
	DD_INIT_ARRAY_SIZE(&state_values, tendril->legend.keys.size);
	values_from_int(state, &state_values, &tendril->legend);

	for (i = 0; i < match_indices.size; i++) {
		DD_INIT_ARRAY(&tmp_str_arr);
		collect_contents(&tmp_str_arr, &state_values, 
				&tendril->contents.elems[match_indices.elems[i]]);
		DD_ADD_ARRAY(result, tmp_str_arr);
	}

	DD_FREE_ARRAY(&state_values);
	DD_FREE_ARRAY(&match_indices);



	return 1;
}
