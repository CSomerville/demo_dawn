#include <stdio.h>
#include <stdlib.h>
#include "dd_graph.h"

void initialize_graph(DDGraph *g, bool directed, int count) {
	int i;

	g->edges.elems = NULL;
	DD_INIT_ARRAY_SIZE(&(g->edges), count);
	for (i = 0; i < count; i++) DD_INIT_ARRAY(&(g->edges.elems[i]));

	g->degree.elems = NULL;
	DD_INIT_ARRAY_SIZE(&(g->degree), count);
	for (i = 0; i < count; i++) g->degree.elems[i] = 0;

	g->nvertices = count;
	g->nedges = 0;
	g->directed = directed;
}

void free_graph_members(DDGraph *g) {
	int i;
	for (i = 0; i < g->nvertices; i++) 
		DD_FREE_ARRAY(&(g->edges.elems[i]));
	DD_FREE_ARRAY(&(g->edges));
	DD_FREE_ARRAY(&(g->degree));
}

void insert_edge(DDGraph *g, int x, int y, bool directed) {
	DD_ADD_ARRAY(&(g->edges.elems[x]), y);
	g->degree.elems[x]++;

	if (directed == false) {
		insert_edge(g, y, x, true);
	} else {
		g->nedges++;
	}
}

bool edge_in_graph(DDGraph *g, int x, int y) {
	int i;
	for (i = 0; i < g->edges.elems[x].size; i++) {
		if (g->edges.elems[x].elems[i] == y) return true;
	}
	return false;
}


void print_graph(DDGraph *g) {
	int i, j;

	for (i = 0; i < g->nvertices; i++) {
		printf("%d: ", i);
		for (j = 0; j < g->edges.elems[i].size; j++) {
			printf(" %d", g->edges.elems[i].elems[j]);
		}
		printf("\n");
	}
}
