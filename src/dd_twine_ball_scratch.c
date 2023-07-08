#include <stdio.h>
#define YYSTYPE TWINE_BALLSTYPE
#include "dd_twine_ball.tab.h"
#include "dd_twine_ball.lex.h"

int main(void) {
	DDTwineBallData data;
	data.stack = DD_ALLOCATE(DDStackDDTwineBallItem, 1);
	int i;
	FILE *f;
	yyscan_t scanner;

	DD_INIT_STACK(data.stack);
	f = fopen("./static/test.twb", "r");
	if (!f)
		return 1;
	if ((i = twine_balllex_init(&scanner)) != 0) {
		exit(i);
	}

	twine_ballset_in(f, scanner);
	int e = twine_ballparse(&data, scanner);
	if (e != 0)
		exit(e);
	twine_balllex_destroy(scanner);
	print_twine_ball(data.root);
	dd_twine_ball_free(data.root);
	free(data.root);
	free(data.stack);
	fclose(f);
}
