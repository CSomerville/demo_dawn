#include <stdio.h>
#include <time.h>
#define YYSTYPE MONSTRESTYPE
#include "fe_monstre.tab.h"
#include "fe_monstre.lex.h"
#include "fe_monstre_lib.h"
#include "dd_data.h"

int main(void) {
	srand(time(NULL));
	int i;
	struct FEMonstreData dat;
	FEMonstreState state;
	FILE *f;

	fe_monstre_init(&state, &dat);
	f = fopen("./static/festival/gram.monst", "r");
	if (!f)
		return 1;

	fe_monstre_parse(&dat, f);
	fe_monstre_pick_initial_entity(&state, &dat);
	for (i = 0; i < 10; i++) {
		fe_monstre_tick(&state, &dat);
	}

	for (i = 0; i < state.collect.size; i++) {
		printf("%s ", dd_twine_chars(&state.collect.elems[i]));
	}
	printf("\n");
	fclose(f);
	return 0;
}
