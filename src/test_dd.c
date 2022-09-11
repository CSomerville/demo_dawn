#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "dd_data.h"
#include "dd_graph.h"
#include "dd_algo.h"

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

	/* give to dd_string */
	DDString dd_str;
	dd_str.chars = NULL;
	const char *test_give = "Demo dawn through fronds";
	int len_give = 24;
	give_to_dd_string(&dd_str, test_give, len_give);
	assert(dd_str.length == len_give);
	assert(!strcmp(test_give, dd_str.chars));
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

int main(void) {
	dd_array();
	dd_queue();
	dd_string();
	dd_graph();
	dd_cart_concat();
}
