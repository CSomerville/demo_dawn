#include <stdio.h>
#include <time.h>
#define YYSTYPE MONSTRESTYPE
#include "fe_monstre.tab.h"
#include "fe_monstre.lex.h"
#include "fe_monstre_lib.h"
#include "dd_data.h"
#include "dd_twine.h"
#include "di_lib.h"
#include "li_lineate.h"

static void lineate_and_print(DDArrDDTwine *collect) {
	DDTwine all, between;
	DDArrDIIndexedEntry dict_entries;
	DDArrDDTwine lines;
	DDArrInt line_indices;
	int i;

	dd_twine_init(&all);
	dd_twine_init(&between);
	DD_INIT_ARRAY(&dict_entries);
	DD_INIT_ARRAY(&lines);
	DD_INIT_ARRAY(&line_indices);

	dd_twine_from_chars_fixed(&between, " ", 1);

	dd_twine_join(&all, collect, &between);
	di_entries_for_string(&all, &dict_entries, "./static/cmudict/raw.txt");
	li_lineate_trochee(&line_indices, &dict_entries, &all);
	li_lineate_to_arr(&lines, &line_indices, &all);

	for (i = 0; i < lines.size; i++) {
		printf("%s\n", dd_twine_chars(&lines.elems[i]));
	}

	dd_twine_destroy(&all);
	dd_twine_destroy(&between);
	for (i = 0; i < dict_entries.size; i++) {
		free_di_dict_entry(&dict_entries.elems[i].entry);
	}
	DD_FREE_ARRAY(&dict_entries);
	dd_arr_dd_twine_destroy(&lines);
	DD_FREE_ARRAY(&line_indices);
}

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
	for (i = 0; i < 20; i++) {
		fe_monstre_tick(&state, &dat);
	}

	lineate_and_print(&state.collect);

	fclose(f);
	return 0;
}
