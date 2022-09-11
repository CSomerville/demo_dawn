#include "dd_algo.h"

static DDArrDDArrInt *cart_concat(DDArrDDArrInt *acc, DDArrInt *next) {
	int i, j, k;
	DDArrInt tmp;
	DDArrDDArrInt *final;

	final = DD_ALLOCATE(DDArrDDArrInt, 1);
	DD_INIT_ARRAY(final);

	if (acc->size == 0) {
		for (i = 0; i < next->size; i++) {
			DD_INIT_ARRAY(&tmp);
			DD_ADD_ARRAY(&tmp, next->elems[i]);
			DD_ADD_ARRAY(final, tmp);
		}
		return final;
	}

	for (i = 0; i < acc->size; i++) {
		for (j = 0; j < next->size; j++) {
			DD_INIT_ARRAY(&tmp);
			for (k = 0; k < acc->elems[i].size; k++) {
				DD_ADD_ARRAY(&tmp, acc->elems[i].elems[k]);
			}
			DD_ADD_ARRAY(&tmp, next->elems[j]);
			DD_ADD_ARRAY(final, tmp);
		}
	}
	return final;
}

DDArrDDArrInt *cartesian_product(DDArrDDArrInt *arr) {
	int i, j;
	DDArrDDArrInt *acc;
	DDArrDDArrInt *tmp_acc;

	acc = DD_ALLOCATE(DDArrDDArrInt, 1);
	DD_INIT_ARRAY(acc);

	for (i = 0; i < arr->size; i++) {
		tmp_acc = cart_concat(acc, &arr->elems[i]);
		for (j = 0; j < acc->size; j++) {
			DD_FREE_ARRAY(&(acc->elems[j]));
		}
		DD_FREE_ARRAY(acc);
		free(acc);
		acc = tmp_acc;
	}
	
	return acc;
}

