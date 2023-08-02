#include <stdio.h>
#include <time.h>
#include "dd_twine.h"
#include "fe_neo_lib_land.h"
#include "fe_nll_narrator_2.h"

int main(void) {
	srand(time(NULL));
	int i;
	FENLLConfigureWorld conf;
	FENLLWorld world;
	DDString tmp;
	DDArrDDString names;
	DDArrInt counts;
	DDArrDDTwine narrator_result; 

	DD_INIT_ARRAY(&narrator_result);
	DD_INIT_ARRAY(&counts);
	for (i = 0; i < 10; i++) {
		DD_ADD_ARRAY(&counts, 0);
	}

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
	conf.num_population = 25;
	conf.names = names;
	conf.pocket_floor = 20.0;
	conf.pocket_ceil = 100.0;
	conf.amt_floor = 15.0;
	conf.amt_ceil = 75.0;
	conf.interest_floor = 0.01;
	conf.interest_ceil = 0.05;
	conf.mode = FE_NLL_MODE_SILENT;

	fe_nll_init_world(&world, &conf);
	for (i = 0; i < 100; i++) {
		fe_nll_tick(&world);
		/*for (j = 0; j < world.populace.size; j++) {*/
			/*for (k = 0; k < world.populace.elems[j].turn_log.size; k++) {*/
				/*counts.elems[*/
					/*world.populace.elems[j].turn_log.elems[k].type*/
				/*]++;*/
			/*}*/
		/*}*/
	}

	narrate_from_turn_log(&narrator_result, &world.populace.elems[0]);
	for (i = 0; i < narrator_result.size; i++) {
		printf("%s ", narrator_result.elems[i].chars);
	}
	printf("\n");


	/*for (i = 0; i < counts.size; i++) {*/
		/*printf("%d: %d\n", i, counts.elems[i]);*/
	/*}*/

	fe_nll_free_world(&world);
	for (i = 0; i < names.size; i++) {
		free_dd_chars(&names.elems[i]);
	}
	dd_arr_dd_twine_destroy(&narrator_result);
	DD_FREE_ARRAY(&names);
	DD_FREE_ARRAY(&counts);
}
