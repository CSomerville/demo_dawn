#include "dd_twine.h"
#include "di_lib.h"

void li_lineate_trochee(DDArrInt *indices,
		DDArrDIIndexedEntry *dict_entries, DDTwine *raw_txt) {
	int i, j, state;
	DDArrDISyllable *pron;

	state = 0;
	for (i = 0; i < dict_entries->size; i++) {
		pron = &dict_entries->elems[i].entry.pronunciation;
		for (j = 0; j < pron->size; j++) {
			if (pron->elems[j].stress > 0) {
				state = 1;
			} else if (pron->elems[j].stress == 0) {
				if (state == 1) {
					state = 2;
				} else if (state == 2) {
					state = 0;
				}
			}
		}
		if (state == 2) {
			state = 0;
			if (i + 1 < dict_entries->size) {
				DD_ADD_ARRAY(indices, dict_entries->elems[i+1].index);
			}
		}
	}
}

void li_lineate_trochee_2(int *brk_idx, int dict_offset,
		DDArrDIIndexedEntry *dict_entries) {
	int i, j, state;
	DDArrDISyllable *pron;
	
	state = 0;
	for (i = dict_offset; i < dict_entries->size; i++) {
		pron = &dict_entries->elems[i].entry.pronunciation;
		for (j = 0; j < pron->size; j++) {
			if (pron->elems[j].stress > 0) {
				state = 1;
			} else if (pron->elems[j].stress == 0) {
				if (state == 1) {
					state = 2;
				} else if (state == 2) {
					state = 0;
				}
			}
		}
		if (state == 2 && (i+1) < dict_entries->size) {
			*brk_idx = i+1;
			return;
		}
	}
	*brk_idx = -1;
}

void li_lineate_to_arr(DDArrDDTwine *lines, DDArrInt *indices,
		DDTwine *raw_txt) {
	int i, base;
	DDTwine tmp;

	base = 0;
	for (i = 0; i < indices->size; i++) {
		dd_twine_init(&tmp);
		dd_twine_from_chars_fixed(&tmp, &raw_txt->chars[base],
				indices->elems[i] - base);
		DD_ADD_ARRAY(lines, tmp);
		base = indices->elems[i];
	}
	dd_twine_init(&tmp);
	dd_twine_from_chars_dyn(&tmp, &raw_txt->chars[base]);
	DD_ADD_ARRAY(lines, tmp);
}
