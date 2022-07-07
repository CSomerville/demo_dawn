#ifndef te_tendril_h
#define te_tendril_h

#include "dd_data.h"
#include "dd_graph.h"
#include "te_scanner.h"

typedef struct {
    TEToken current;
    TEToken previous;
} TEParser;

typedef struct {
    DDArrDDString keys;
    DDArrDDArrDDString values;
} TETendrilLegend;

typedef struct {
    DDString name;
    TETendrilLegend legend;
	DDGraph *graph;
	int start;
} TETendril;

DD_DEF_ARRAY(TETendril, TETendril)

void parse_tendrils(TEScanner *scanner, DDArrTETendril *tendrils);
int int_from_values(DDArrInt *values, TETendrilLegend *leg);
void values_from_int(int x, DDArrInt *vals, TETendrilLegend *leg);

#endif
