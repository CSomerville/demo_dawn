#include <stdio.h>
#include <time.h>
#include "fe_neo_lib_land.h"
#include "dd_utils.h"
#include "te_scanner.h"
#include "te_tendril.h"
#include "fe_nll_narrator.h"

int main(void) {
	srand(time(NULL));
	int i;
	FENLLConfigureWorld conf;
	FENLLWorld world;
	DDString tmp;
	DDArrDDString result;
	DDArrDDString names;
	char *source;
	TEScanner scanner;
	DDArrTETendril tendrils;

	DD_INIT_ARRAY(&tendrils);
	DD_INIT_ARRAY(&result);

	source = read_file("./static/festival/owing.te");
	init_te_scanner(&scanner, source);
	parse_tendrils(&scanner, &tendrils);

	DD_INIT_ARRAY(&names);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Joe", 3);
	DD_ADD_ARRAY(&names, tmp);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Jess", 4);
	DD_ADD_ARRAY(&names, tmp);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Jane", 4);
	DD_ADD_ARRAY(&names, tmp);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Jules", 5);
	DD_ADD_ARRAY(&names, tmp);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Jerr", 4);
	DD_ADD_ARRAY(&names, tmp);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Jill", 4);
	DD_ADD_ARRAY(&names, tmp);
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, "Jack", 4);
	DD_ADD_ARRAY(&names, tmp);

	conf.height = 25;
	conf.width = 25;
	conf.num_population = 20;
	conf.names = names;
	conf.pocket_floor = 20.0;
	conf.pocket_ceil = 100.0;
	conf.amt_floor = 15.0;
	conf.amt_ceil = 75.0;
	conf.interest_floor = 0.01;
	conf.interest_ceil = 0.05;
	conf.mode = FE_NLL_MODE_SILENT;

	fe_nll_init_world(&world, &conf);
	for (i = 0; i < 20; i++) {
		fe_nll_tick(&world);
	}

	narrate_from_turn_log(&result, &world.populace.elems[0], 
			&tendrils.elems[0]);
	for (i = 0; i < result.size; i++) {
		printf("%s\n", result.elems[i].chars);
	}

	fe_nll_free_world(&world);
}
