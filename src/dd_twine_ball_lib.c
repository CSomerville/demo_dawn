#include <stdio.h>
#define YYSTYPE TWINE_BALLSTYPE
#include "dd_twine_ball.tab.h"
#include "dd_twine_ball.lex.h"

void dd_twine_ball_parse(DDTwineBallData *data, FILE *f) {
	int i;
	yyscan_t scanner;

	if ((i = twine_balllex_init(&scanner)) != 0) {
		exit(i);
	}

	twine_ballset_in(f, scanner);
	int e = twine_ballparse(data, scanner);
	if (e != 0)
		exit(e);
	twine_balllex_destroy(scanner);
}

void dd_twine_ball_read(DDTwineBallData *data, const char *fpath) {
	data->stack = DD_ALLOCATE(DDStackDDTwineBallItem, 1);
	int i;
	FILE *f;
	yyscan_t scanner;

	DD_INIT_STACK(data->stack);
	f = fopen(fpath, "r");
	if (!f)
		exit(1);
	if ((i = twine_balllex_init(&scanner)) != 0) {
		exit(i);
	}

	twine_ballset_in(f, scanner);
	int e = twine_ballparse(data, scanner);
	if (e != 0)
		exit(e);
	twine_balllex_destroy(scanner);

	fclose(f);
}

void dd_twine_ball_data_destroy(DDTwineBallData *data) {
	dd_twine_ball_free(data->root);
	free(data->root);
	free(data->stack);
}
