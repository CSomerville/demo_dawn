/* example

{
	key1: bare_val;
	key2: "quoted val";
	key3: (bare_val, "quoted val", (nested), { nested_key: any_val });
	key4: { nested_key_2: bare_val, nested_key_3: () },
}

*/

%define api.pure true
%define api.prefix {twine_ball}
%define parse.error verbose
%code top {
	#ifndef YYNOMEM
	#define YYNOMEM goto yyexhaustedlab
	#endif
}

%code requires {
	#include <stdio.h>
	#include "dd_data.h"
	#include "dd_twine.h"

	struct DDTwineBallList;
	struct DDTwineBallObj;

	typedef enum {
		DD_TWINE_BALL_LIST,
		DD_TWINE_BALL_OBJ,
		DD_TWINE_BALL_TWINE,
	} DDTwineBallType; 

	typedef union {
		struct DDTwineBallList *l;
		struct DDTwineBallObj *o;
		DDTwine *t;
	} DDTwineBallValue;

	typedef struct {
		DDTwineBallType type;
		DDTwineBallValue value;
	} DDTwineBallItem;

	DD_DEF_ARRAY(DDTwineBallItem, DDTwineBallItem);
	DD_DEF_STACK(DDTwineBallItem *, DDTwineBallItem);

	typedef struct DDTwineBallList {
		DDArrDDTwineBallItem items;
	} DDTwineBallList;

	typedef struct DDTwineBallObj {
		DDArrDDTwine keys;
		DDArrDDTwineBallItem items;
	} DDTwineBallObj;

	typedef struct DDTwineBallData {
		DDTwineBallItem *root;
		DDStackDDTwineBallItem *stack;
	} DDTwineBallData;
}

%define api.value.type {DDTwineBallItem *}

%parse-param {DDTwineBallData *data}

%param {void *scanner}

%code provides {
	void dd_twine_ball_free(DDTwineBallItem *item);
	void print_twine_ball(DDTwineBallItem *item);
	bool dd_twine_ball_obj_get(DDTwineBallItem **item, DDTwineBallObj *o,
		DDTwine *key);
}

%code {
	int twine_ballerror(void *yylval, const void *s, char const *msg);
	int twine_balllex(void *lval, const void *s);
}

%token TWI

%%

start :
	  twine_ball_item { return 0; }
;

twine_ball_item :
	  list_begin list_items ')' {
		DD_POP_STACK(DDTwineBallItem, data->stack, $$);
		if (DD_STACK_EMPTY(data->stack))
			data->root = $$;
}
	| obj_begin keys_values '}' {
		DD_POP_STACK(DDTwineBallItem, data->stack, $$);
		if (DD_STACK_EMPTY(data->stack))
			data->root = $$;
}
	| TWI
;

list_begin :
	'(' {
		DDTwineBallItem *item = DD_ALLOCATE(DDTwineBallItem, 1);
		DDTwineBallList *list = DD_ALLOCATE(DDTwineBallList, 1);
		DD_INIT_ARRAY(&list->items);
		item->type = DD_TWINE_BALL_LIST;
		item->value.l = list;
		DD_PUSH_STACK(DDTwineBallItem, data->stack, item);
}

obj_begin :
	'{' {
		DDTwineBallItem *item = DD_ALLOCATE(DDTwineBallItem, 1);
		DDTwineBallObj *obj = DD_ALLOCATE(DDTwineBallObj, 1);
		DD_INIT_ARRAY(&obj->keys);
		DD_INIT_ARRAY(&obj->items);
		item->type = DD_TWINE_BALL_OBJ;
		item->value.o = obj;
		DD_PUSH_STACK(DDTwineBallItem, data->stack, item);
}

list_items :
	list_items ',' twine_ball_item {
				DD_ADD_ARRAY(&DD_PEEK_STACK(data->stack)->value.l->items, 
						*($3));
				free($3);
	}
	| twine_ball_item {
				DD_ADD_ARRAY(&DD_PEEK_STACK(data->stack)->value.l->items, 
						*($1));
				free($1);
	}
;

keys_values :
	keys_values key_value
	| key_value
;

key_value :
		  TWI ':' twine_ball_item ';' {
				DDTwine tmp_str;
				dd_twine_init(&tmp_str);
				dd_twine_copy(&tmp_str, ($1)->value.t);
				DD_ADD_ARRAY(&DD_PEEK_STACK(data->stack)->value.o->keys,
					tmp_str);
				DD_ADD_ARRAY(&DD_PEEK_STACK(data->stack)->value.o->items, 
						*($3));
				dd_twine_destroy(($1)->value.t);
				free(($1)->value.t);
				free($1);
				free($3);
	}
;

%%

int twine_ballerror(void *yylval, const void *s, char const *msg)
{
	(void)yylval;
	(void)s;
	return fprintf(stderr, "msg: %s\n", msg);
}

void dd_twine_ball_free(DDTwineBallItem *item) {
	int i;
	if (!item)
		return;
	switch (item->type) {
		case DD_TWINE_BALL_LIST:
			for (i = 0; i < item->value.l->items.size; i++) {
				dd_twine_ball_free(&item->value.l->items.elems[i]);
			}
			DD_FREE_ARRAY(&item->value.l->items);
			free(item->value.l);
			break;
		case DD_TWINE_BALL_OBJ:
			for (i = 0; i < item->value.o->items.size; i++) {
				dd_twine_ball_free(&item->value.o->items.elems[i]);
			}
			DD_FREE_ARRAY(&item->value.o->items);
			dd_arr_dd_twine_destroy(&item->value.o->keys);
			free(item->value.o);
			break;
		case DD_TWINE_BALL_TWINE:
			dd_twine_destroy(item->value.t);
			free(item->value.t);
			break;
	}
}

void print_twine_ball(DDTwineBallItem *item) {
	int i;
	switch (item->type) {
		case DD_TWINE_BALL_LIST:
			printf("(\n");
			for (i = 0; i < item->value.l->items.size; i++) {
				print_twine_ball(&item->value.l->items.elems[i]);
				printf(",\n");
			}
			printf(")\n");
			break;
		case DD_TWINE_BALL_OBJ:
			printf("{\n");
			for (i = 0; i < item->value.o->items.size; i++) {
				printf("%s: ", item->value.o->keys.elems[i].chars);
				print_twine_ball(&item->value.o->items.elems[i]);
				printf(",\n");
			}
			printf("}\n");
			break;
		case DD_TWINE_BALL_TWINE:
			printf("%s", item->value.t->chars);
			break;
	}
}

bool dd_twine_ball_obj_get(DDTwineBallItem **item, DDTwineBallObj *o,
		DDTwine *key) {
	int i;
	for (i = 0; i < o->keys.size; i++) {
		if (dd_twine_eq(&o->keys.elems[i], key)) {
			*item = &o->items.elems[i];
			return true;
		}
	}
	return false;
}

