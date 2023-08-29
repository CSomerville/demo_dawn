#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "dd_data.h"
#include "dd_graph.h"
#include "dd_algo.h"
#include "dd_twine.h"

void dd_array(void) {
	printf("testing dd_array... ");
	int i, x;
	DDArrInt dd_arr_int;

	/* init array, add one element */
	DD_INIT_ARRAY(&dd_arr_int);
	x = 5;
	DD_ADD_ARRAY(&dd_arr_int, x);
	assert(dd_arr_int.size == 1);
	assert(dd_arr_int.capacity == 8);
	assert(dd_arr_int.elems[0] == 5);

	/* grow array */
	for (i = 1; i < 10; i++) {
		DD_ADD_ARRAY(&dd_arr_int, i);
	}
	assert(dd_arr_int.size == 10);
	assert(dd_arr_int.capacity == 16);
	assert(dd_arr_int.elems[9] == 9);

	/* remove array */
	DD_REMOVE_ARRAY(&dd_arr_int, 3);
	assert(dd_arr_int.size == 9);
	assert(dd_arr_int.elems[2] == 2);
	assert(dd_arr_int.elems[3] == 4);

	/* free array, init array, confirm empty */
	DD_FREE_ARRAY(&dd_arr_int);
	DD_INIT_ARRAY(&dd_arr_int);
	assert(dd_arr_int.size == 0);
	assert(dd_arr_int.capacity == 0);
	assert(dd_arr_int.elems == NULL);

	/* init with size */
	DD_INIT_ARRAY_SIZE(&dd_arr_int, 7);
	assert(dd_arr_int.size == 7);
	assert(dd_arr_int.capacity == 7);
	for (i = 0; i < dd_arr_int.size; i++) dd_arr_int.elems[i] = i;
	assert(dd_arr_int.elems[6] == 6);

	/* init with size, resize */
	DD_INIT_ARRAY_SIZE(&dd_arr_int, 5);
	assert(dd_arr_int.size == 5);
	assert(dd_arr_int.capacity == 5);
	assert(dd_arr_int.elems[3] == 3);
	DD_INIT_ARRAY_SIZE(&dd_arr_int, 10);
	assert(dd_arr_int.size == 10);
	assert(dd_arr_int.capacity == 10);
	for (i = 0; i < dd_arr_int.size; i++) dd_arr_int.elems[i] = i;
	assert(dd_arr_int.elems[6] == 6);
	DD_FREE_ARRAY(&dd_arr_int);


	
	printf("success.\n");
}

void dd_queue(void) {
	printf("testing dd_queue... ");
	int i, x;
	DDQInt dd_queue_int;

	DD_INIT_QUEUE(&dd_queue_int);
	assert(DD_QUEUE_EMPTY(&dd_queue_int));

	for (i = 0; i < 10; i++) {
		DD_ENQUEUE(Int, &dd_queue_int, i);
	}
	assert(!DD_QUEUE_EMPTY(&dd_queue_int));
	for (i = 0; i < 10; i++) {
		DD_DEQUEUE(Int, &dd_queue_int, x);
		assert(x == i);
	}
	assert(DD_QUEUE_EMPTY(&dd_queue_int));
	printf("success.\n");
}

void dd_stack(void) {
	printf("testing dd_stack... ");
	DD_DEF_STACK(int, Int);

	int i, x;
	DDStackInt dd_stack_int;

	DD_INIT_STACK(&dd_stack_int);
	for (i = 0; i < 10; i++) {
		DD_PUSH_STACK(Int, &dd_stack_int, i);
	}
	assert(!DD_STACK_EMPTY(&dd_stack_int));
	assert(DD_PEEK_STACK(&dd_stack_int) == 9);
	for (i = 0; i < 10; i++) {
		DD_POP_STACK(Int, &dd_stack_int, x);
		assert(x == 9 - i);
	}
	assert(DD_STACK_EMPTY(&dd_stack_int));
	printf("success.\n");
}

/* TODO: test dd_string_equal */
void dd_string(void) {
	printf("testing dd_string... ");
	/* copy string */
	const char *test = "Demo dawn through fronds";
	int len = 24;
	DDString *str;
	str = copy_string(test, len);
	assert(str->length == len);
	assert(!strcmp(test, str->chars));
	free_string(str);

	/* concat string */
	const char *test1 = "abc";
	const char *test2 = "def";
	DDString *str_a, *str_b, *result;
	str_a = copy_string(test1, 3);
	str_b = copy_string(test2, 3);
	result = dd_string_concat(str_a, str_b);
	assert(result->length == 6);
	assert(!strcmp("abcdef", result->chars));
	free_string(str_a);
	free_string(str_b);
	free_string(result);

	/* mutate concat */
	const char *test3 = "abc";
	const char *test4 = "def";
	DDString *str_c, *str_d;
	str_c = copy_string(test3, 3);
	str_d = copy_string(test4, 3);
	dd_string_concat_mutate(str_c, str_d);
	assert(str_c->length == 6);
	assert(!strcmp("abcdef", str_c->chars));
	free_string(str_c);
	free_string(str_d);

	/* give to dd_string */
	DDString dd_str;
	dd_str.chars = NULL;
	const char *test_give = "Demo dawn through fronds";
	int len_give = 24;
	give_to_dd_string(&dd_str, test_give, len_give);
	assert(dd_str.length == len_give);
	assert(!strcmp(test_give, dd_str.chars));
	free_dd_chars(&dd_str);

	/* dd copy dd_string */
	DDString target;
	DDString to_copy;
	init_dd_string(&target);
	init_dd_string(&to_copy);
	give_to_dd_string(&to_copy, "blarg", 5);
	dd_copy_dd_string(&target, &to_copy);
	assert(!strcmp("blarg", target.chars));
	assert(target.length == 5);
	free_dd_chars(&to_copy);
	free_dd_chars(&target);

	/* dd_repeat_dd_string */
	DDString target_2;
	DDString to_repeat;
	init_dd_string(&target_2);
	init_dd_string(&to_repeat);
	give_to_dd_string(&to_repeat, "blarg", 5);
	dd_repeat_dd_string(&target_2, &to_repeat, 3);
	assert(!strcmp("blargblargblarg", target_2.chars));
	assert(target_2.length == 15);
	free_dd_chars(&target_2);
	free_dd_chars(&to_repeat);

	printf("success.\n");
}

void dd_word_bounds(void) {
	printf("testing dd_word_bounds... ");
	DDString dd_str;
	int start, word_start, word_end, result;
	init_dd_string(&dd_str);
	give_to_dd_string(&dd_str, "two ghosts in thermodynamism's foamy", 36);
	start = 0;
	result = get_next_dd_string_word_bounds(&dd_str, start, &word_start,
			&word_end);
	assert(result == 0);
	assert(word_start == 0);
	assert(word_end == 3);
	start = word_end;
	result = get_next_dd_string_word_bounds(&dd_str, start, &word_start,
			&word_end);
	assert(result == 0);
	assert(word_start == 4);
	assert(word_end == 10);

	free_dd_chars(&dd_str);
	printf("success.\n");
}

void dd_graph(void) {
	printf("testing dd_graph... ");
	/* simple graph */
	DDGraph graph;
	initialize_graph(&graph, true, 4);
	insert_edge(&graph, 0, 1, true);
	insert_edge(&graph, 1, 2, true);
	insert_edge(&graph, 3, 0, true);
	assert(edge_in_graph(&graph, 3, 0));
	assert(edge_in_graph(&graph, 1, 2));
	assert(!edge_in_graph(&graph, 0, 3));
	free_graph_members(&graph);

	printf("success.\n");
}

void dd_cart_concat(void) {
	printf("testing dd_cart_concat... ");
	int i, j;
	DDArrInt tmp;
	DDArrDDArrInt test;
	DDArrDDArrInt *next;

	DD_INIT_ARRAY(&test);

	for (i = 1; i <= 3; i++) {
		DD_INIT_ARRAY(&tmp);
		for (j = 1; j <= 3; j++) {
			DD_ADD_ARRAY(&tmp, i * j);
		}	
		DD_ADD_ARRAY(&test, tmp);
	}

	next = cartesian_product(&test);

	assert(next->size == 27);

	for (i = 0; i < next->size; i++) {
		DD_FREE_ARRAY(&next->elems[i]);
	}
	DD_FREE_ARRAY(next);
	free(next);

	for (i = 0; i < test.size; i++) {
		DD_FREE_ARRAY(&test.elems[i]);
	}
	DD_FREE_ARRAY(&test);

	printf("success.\n");
}

void dd_twine(void) {
	printf("testing dd_twine... ");
	DDTwine twa;
	DDTwine twb;
	DDTwine twc;
	DDArrDDTwineWB wb_arr;

	dd_twine_init(&twa);
	dd_twine_init(&twb);
	dd_twine_init(&twc);

	dd_twine_from_chars_fixed(&twa, "blarg", 5);
	assert(!strcmp(dd_twine_chars(&twa), "blarg"));
	dd_twine_destroy(&twa);
	dd_twine_init(&twa);

	dd_twine_from_chars_dyn(&twa, "blarg");
	assert(!strcmp(dd_twine_chars(&twa), "blarg"));
	assert(dd_twine_char_at(&twa, 1) == 'l');
	dd_twine_destroy(&twa);
	dd_twine_init(&twa);

	dd_twine_from_chars_fixed(&twa, "unk", 3);
	dd_twine_from_chars_fixed(&twb, "unk", 3);
	assert(dd_twine_len(&twa) == 3);
	assert(dd_twine_eq(&twa, &twb));
	dd_twine_destroy(&twb);
	dd_twine_init(&twb);
	dd_twine_from_chars_fixed(&twb, "blarg", 5);
	assert(!dd_twine_eq(&twa, &twb));
	dd_twine_destroy(&twa);
	dd_twine_destroy(&twb);
	dd_twine_init(&twa);
	dd_twine_init(&twb);

	dd_twine_from_chars_fixed(&twa, "blarg", 5);
	dd_twine_from_chars_fixed(&twb, "stror", 5);
	dd_twine_concat(&twc, &twa, &twb);
	assert(!strcmp(dd_twine_chars(&twc), "blargstror"));
	dd_twine_destroy(&twa);
	dd_twine_destroy(&twb);
	dd_twine_destroy(&twc);
	dd_twine_init(&twa);
	dd_twine_init(&twb);
	dd_twine_init(&twc);

	dd_twine_from_chars_fixed(&twa, "blarg", 5);
	dd_twine_from_chars_fixed(&twb, "stror", 5);
	dd_twine_concat_mut(&twa, &twb);
	assert(!strcmp(dd_twine_chars(&twa), "blargstror"));
	dd_twine_destroy(&twa);
	dd_twine_destroy(&twb);
	dd_twine_init(&twa);
	dd_twine_init(&twb);

	DD_INIT_ARRAY(&wb_arr);
	dd_twine_from_chars_fixed(&twa, "blarg unk nuu", 13);
	dd_twine_word_bounds_substr(&wb_arr, &twa, 5, twa.length);
	assert(wb_arr.elems[0].start == 6);
	assert(wb_arr.elems[0].end == 9);
	assert(wb_arr.elems[1].start == 10);
	assert(wb_arr.elems[1].end == 13);
	DD_FREE_ARRAY(&wb_arr);
	dd_twine_destroy(&twa);
	dd_twine_init(&twa);

	DD_INIT_ARRAY(&wb_arr);
	dd_twine_from_chars_fixed(&twa, "blarg 123.456 nuu", 17);
	dd_twine_word_bounds_substr(&wb_arr, &twa, 5, twa.length);
	assert(wb_arr.elems[0].start == 6);
	assert(wb_arr.elems[0].end == 13);
	assert(wb_arr.elems[1].start == 14);
	assert(wb_arr.elems[1].end == 17);
	DD_FREE_ARRAY(&wb_arr);
	dd_twine_destroy(&twa);
	dd_twine_init(&twa);
	printf("success.\n");
}

int main(void) {
	dd_array();
	dd_queue();
	dd_stack();
	dd_string();
	dd_word_bounds();
	dd_graph();
	dd_cart_concat();
	dd_twine();
}
