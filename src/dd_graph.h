#ifndef dd_graph_h
#define dd_graph_h

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "dd_data.h"

typedef struct {
	DDArrDDArrInt edges;
	DDArrInt degree;
	int nvertices;
	int nedges;
	bool directed;
} DDGraph;

void initialize_graph(DDGraph *g, bool directed, int count);
void insert_edge(DDGraph *g, int x, int y, bool directed);
bool edge_in_graph(DDGraph *g, int x, int y);
void free_graph_members(DDGraph *g);
void print_graph(DDGraph *g);

#endif
