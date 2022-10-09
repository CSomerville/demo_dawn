#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "dd_data.h"
#include "te_scanner.h"
#include "te_tendril.h"

void te_tendril_legend(void) {
	printf("testing te_tendril_legend... ");
	TEScanner scanner;
	TETendril tendril;
	DDArrTETendril tendrils;
	DDString *name = copy_string("Test", 4);
	const char *source =
		"StateSpace Test {\n"
		"	shape: Oval | Square | Rectangle | Circle;\n"
		"	color: Yellow | Fuschia | Lime;\n"
		"	texture: Bumpy | Smooth;\n"
		"}";

	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);

	parse_tendrils(&scanner, &tendrils);
	tendril = *(lookup_tendril_by_name(&tendrils, name));
	assert(tendril.legend.keys.size == 3);
	assert(!strcmp(tendril.legend.keys.elems[0].chars, "shape"));
	assert(!strcmp(tendril.legend.keys.elems[1].chars, "color"));
	assert(!strcmp(tendril.legend.keys.elems[2].chars, "texture"));
	assert(!strcmp(tendril.legend.values.elems[0].elems[0].chars,
				"Oval"));
	assert(!strcmp(tendril.legend.values.elems[0].elems[1].chars,
				"Square"));
	assert(!strcmp(tendril.legend.values.elems[0].elems[2].chars,
				"Rectangle"));
	assert(!strcmp(tendril.legend.values.elems[0].elems[3].chars,
				"Circle"));
	assert(!strcmp(tendril.legend.values.elems[1].elems[0].chars,
				"Yellow"));
	assert(!strcmp(tendril.legend.values.elems[1].elems[1].chars,
				"Fuschia"));
	assert(!strcmp(tendril.legend.values.elems[1].elems[2].chars,
				"Lime"));
	assert(!strcmp(tendril.legend.values.elems[2].elems[0].chars,
				"Bumpy"));
	assert(!strcmp(tendril.legend.values.elems[2].elems[1].chars,
				"Smooth"));

	DDString *dd_str;
	int x;
	dd_str = copy_string("shape", 5);
	x = key_index_tendril(&tendril, dd_str);
	assert(x == 0);
	free_string(dd_str);

	dd_str = copy_string("color", 5);
	x = key_index_tendril(&tendril, dd_str);
	assert(x == 1);
	free_string(dd_str);

	dd_str = copy_string("Circle",  6);
	x = value_index_tendril(&tendril, dd_str, 0);
	assert(x == 3);
	free_string(dd_str);

	dd_str = copy_string("Bumpy",  5);
	x = value_index_tendril(&tendril, dd_str, 2);
	assert(x == 0);
	free_string(dd_str);

	free_string(name);
	free_te_tendrils(&tendrils);
	DD_FREE_ARRAY(&tendrils);
	printf("success\n");
}

void te_tendril_start(void) {
	printf("testing te_tendril_start... ");
	TEScanner scanner;
	TETendril tendril;
	DDArrTETendril tendrils;
	DDString *name = copy_string("Test", 4);
	const char *source =
		"StateSpace Test {\n"
		"	shape: Oval | Square | Rectangle | Circle;\n"
		"	color: Yellow | Fuschia | Lime;\n"
		"	texture: Bumpy | Smooth;\n"
		"}\n"
		"StartOn Test {\n"
		"	shape: Circle;\n"
		"	color: Lime;\n"
		"	texture: Smooth;\n"
		"}\n"
		"StateSpace Test2 {\n"
		"	blarg: Stror | Nuuu;\n"
		"	unk: Naar | Gaar;\n"
		"}\n"
		"StartOn Test2 {\n"
		"	blarg: Nuuu;\n"
		"	unk: Naar;\n"
		"}";

	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);

	parse_tendrils(&scanner, &tendrils);
	tendril = *(lookup_tendril_by_name(&tendrils, name));
	free_string(name);
	assert(tendril.start == 23);
	
	name = copy_string("Test2", 5);
	tendril = *(lookup_tendril_by_name(&tendrils, name));
	free_string(name);
	assert(tendril.start == 1);

	free_te_tendrils(&tendrils);
	DD_FREE_ARRAY(&tendrils);
	printf("success\n");
}

void te_tendril_transition(void) {
	printf("testing te_tendril_transition... ");
	TEScanner scanner;
	TETendril tendril;
	DDArrTETendril tendrils;
	DDArrInt values;
	int x, y;
	DDString *name = copy_string("Test", 4);
	const char *source =
		"StateSpace Test {\n"
		"	shape: Oval | Square | Rectangle | Circle;\n"
		"	color: Yellow | Fuschia | Lime;\n"
		"	texture: Bumpy | Smooth;\n"
		"}\n"
		"TransitionOn Test {\n"
		"	current {\n"
		"		shape: Oval;\n"
		"	}\n"
		"	next {\n"
		"		shape: Square;\n"
		"	}\n"
		"}\n";

	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);
	DD_INIT_ARRAY(&values);

	parse_tendrils(&scanner, &tendrils);
	tendril = *(lookup_tendril_by_name(&tendrils, name));
	free_string(name);

	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 1);
	DD_ADD_ARRAY(&values, 1);
	x = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	DD_ADD_ARRAY(&values, 1);
	DD_ADD_ARRAY(&values, 1);
	DD_ADD_ARRAY(&values, 1);
	y = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	assert(edge_in_graph(tendril.graph, x, y));

	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 0);
	x = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	DD_ADD_ARRAY(&values, 1);
	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 0);
	y = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	assert(edge_in_graph(tendril.graph, x, y));

	free_te_tendrils(&tendrils);
	DD_FREE_ARRAY(&tendrils);
	printf("success\n");
}

void te_tendril_transition_case_2(void) {
	printf("testing te_tendril_transition_case_2... ");
	TEScanner scanner;
	TETendril tendril;
	DDArrTETendril tendrils;
	DDArrInt values;
	int x, y;
	DDString *name = copy_string("Test", 4);
	const char *source =
		"StateSpace Test {\n"
		"	shape: Oval | Square | Rectangle | Circle;\n"
		"	color: Yellow | Fuschia | Lime;\n"
		"	texture: Bumpy | Smooth;\n"
		"}\n"
		"TransitionOn Test {\n"
		"	current {\n"
		"		shape: Oval;\n"
		"		color: Fuschia;\n"
		"	}\n"
		"	next {\n"
		"		color: Lime;\n"
		"	}\n"
		"}\n";

	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);
	DD_INIT_ARRAY(&values);

	parse_tendrils(&scanner, &tendrils);
	tendril = *(lookup_tendril_by_name(&tendrils, name));
	free_string(name);

	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 1);
	DD_ADD_ARRAY(&values, 0);
	x = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 2);
	DD_ADD_ARRAY(&values, 0);
	y = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	assert(edge_in_graph(tendril.graph, x, y));

	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 1);
	DD_ADD_ARRAY(&values, 1);
	x = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	DD_ADD_ARRAY(&values, 0);
	DD_ADD_ARRAY(&values, 2);
	DD_ADD_ARRAY(&values, 1);
	y = int_from_values(&values, &tendril.legend);
	DD_FREE_ARRAY(&values);
	assert(edge_in_graph(tendril.graph, x, y));

	free_te_tendrils(&tendrils);
	DD_FREE_ARRAY(&tendrils);
	printf("success\n");
}

void te_tendril_content(void) {
	printf("testing te_tendril_content... ");
	int i, j;
	TEScanner scanner;
	DDArrTETendril tendrils;
	DDArrDDArrDDString strs;
	DD_INIT_ARRAY(&strs);
	const char *source =
		"StateSpace Test {\n"
		"	shape: Oval | Square | Rectangle | Circle;\n"
		"	color: Yellow | Fuschia | Lime;\n"
		"	texture: Bumpy | Smooth;\n"
		"}\n"
		"ContentOn Test {\n"
			"match {\n"
				"shape: Oval | Square;\n"
			"}\n"
			"content {\n"
				"\"blarg for uuuuu\";\n"
				"switch color {\n"
					"Yellow: \"in yellow\";\n"
					"Fuschia: \"in fuschia\";\n"
					"Lime: \"in lime\";\n"
				"}\n"
				"\"pls\";\n"
			"}\n"
		"}\n";
	init_te_scanner(&scanner, source);
	DD_INIT_ARRAY(&tendrils);

	parse_tendrils(&scanner, &tendrils);

	for (i = 0; i < tendrils.elems[0].contents.elems[0].match.size; i++) {
		j = tendrils.elems[0].contents.elems[0].match.elems[i];
		assert(j % 4 == 0 || j % 4 == 1);
	}

	get_te_tendril_content(&strs, 16, &tendrils.elems[0]);
	assert(!strcmp(strs.elems[0].elems[0].chars, "blarg for uuuuu"));
	assert(!strcmp(strs.elems[0].elems[1].chars, "in fuschia"));
	assert(!strcmp(strs.elems[0].elems[2].chars, "pls"));

	free_te_tendrils(&tendrils);
	DD_FREE_ARRAY(&tendrils);

	for (i = 0; i < strs.size; i++) {
		for (j = 0; j < strs.elems[i].size; j++) {
			free_dd_chars(&strs.elems[i].elems[j]);
		}
		DD_FREE_ARRAY(&strs.elems[i]);
	}
	DD_FREE_ARRAY(&strs);

	printf("success\n");
}

int main(void) {
	te_tendril_legend();
	te_tendril_start();
	te_tendril_transition();
	te_tendril_transition_case_2();
	te_tendril_content();
}
