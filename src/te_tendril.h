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

typedef enum {
	TE_CONTENT_ELEM_TYPE_STR,
	TE_CONTENT_ELEM_TYPE_INTER,
} TEContentElemType;

typedef struct {
	DDString contents;
} TEContentStr;

typedef struct {
	int value;
	DDString contents;
} TEInterpolationElem;

DD_DEF_ARRAY(TEInterpolationElem, TEInterpolationElem);

typedef struct {
	int key;
	DDArrTEInterpolationElem contents;
} TEInterpolation;

typedef union {
	TEContentStr s;
	TEInterpolation i;
} TEContentDatum;

typedef struct {
	TEContentElemType type;
	TEContentDatum datum;
} TEContentElem;

DD_DEF_ARRAY(TEContentElem, TEContentElem);

typedef struct {
	DDArrInt match;
	DDArrTEContentElem contents;
} TEContent;

DD_DEF_ARRAY(TEContent, TEContent);

typedef struct {
    DDString name;
    TETendrilLegend legend;
	DDGraph *graph;
	int start;
	DDArrTEContent contents;
} TETendril;

DD_DEF_ARRAY(TETendril, TETendril)

void parse_tendrils(TEScanner *scanner, DDArrTETendril *tendrils);
int int_from_values(DDArrInt *values, TETendrilLegend *leg);
void values_from_int(int x, DDArrInt *vals, TETendrilLegend *leg);
int te_transition(TETendril *tendril, int current);
void print_tendril_legend(TETendrilLegend *leg);
TETendril* lookup_tendril_by_name(DDArrTETendril *tendrils, DDString *name);
int key_index_tendril(TETendril *tendril, DDString *key);
int value_index_tendril(TETendril *tendril, DDString *value, int key_idx);
void free_te_tendril(TETendril *tendril);
void free_te_tendrils(DDArrTETendril *tendrils);
int get_te_tendril_content(DDArrDDArrDDString *result, int state,
		TETendril *tendril);
void print_te_state(int state, TETendril *tendril);

#endif
