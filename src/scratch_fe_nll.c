#include "fe_neo_lib_land.h"

int main(void) {
	srand(201);
	int i;
	FENLLConfigureWorld conf;
	FENLLWorld world;
	DDString tmp;
	DDArrDDString names;

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
	conf.mode = FE_NLL_MODE_WATCH;

	fe_nll_init_world(&world, &conf);
	for (i = 0; i < 300; i++) {
		fe_nll_tick(&world);
	}

	fe_nll_free_world(&world);
}
