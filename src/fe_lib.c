#include "fe_lib.h"

/* Relations */
static void init_entities(DDArrFEEntity *entities) {
	DDString tmp_str;
	FEEntity temp_entity;

	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "you", 3);
	temp_entity.id = 0;
	temp_entity.name = tmp_str;
	DD_ADD_ARRAY(entities, temp_entity);

	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "pres", 4);
	temp_entity.id = 1;
	temp_entity.name = tmp_str;
	DD_ADD_ARRAY(entities, temp_entity);

	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "celeb", 5);
	temp_entity.id = 2;
	temp_entity.name = tmp_str;
	DD_ADD_ARRAY(entities, temp_entity);

	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "workfriend", 10);
	temp_entity.id = 3;
	temp_entity.name = tmp_str;
	DD_ADD_ARRAY(entities, temp_entity);

	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, "mom", 3);
	temp_entity.id = 4;
	temp_entity.name = tmp_str;
	DD_ADD_ARRAY(entities, temp_entity);
}

void run_n_steps(TETendril *tendril, DDArrDDString *result,
		int state, int n_steps) {
	int i, j, res, r;
	DDArrDDArrDDString strs;
	for (i = 0; i < n_steps; i++) {
		DD_INIT_ARRAY(&strs);
		state = te_transition(tendril, state);
		res = get_te_tendril_content(&strs, state, tendril);
		if (res != 0) {
			r = rand() % strs.size;
			for (j = 0; j < strs.elems[r].size; j++) {
				DD_ADD_ARRAY(result, strs.elems[r].elems[j]);
			}
		}
	}
}
