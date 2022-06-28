#ifndef dd_graph_h
#define dd_graph_h

#include <stdbool.h>
#include "dd_data.h"

typedef struct {
	DDArrDDArrInt edges;
	DDArrInt degree;
	int nvertices;
	int nedges;
	bool directed;
} graph;

void initialize_graph(graph *g, bool directed);
void insert_edge(graph *, int x, int y, bool directed);

#endif
