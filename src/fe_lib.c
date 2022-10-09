#include "fe_lib.h"

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
