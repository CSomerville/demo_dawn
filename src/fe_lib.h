#ifndef fe_lib_h
#define fe_lib_h

#include "dd_data.h"
#include "te_tendril.h"

void run_n_steps(TETendril *tendril, DDArrDDString *result,
		int state, int n_steps);

#endif

