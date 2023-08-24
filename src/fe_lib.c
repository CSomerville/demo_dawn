#include <stdio.h>
#include "dd_utils.h"
#include "te_tendril.h"
#include "di_lib.h"
#include "fe_lib.h"
#include "dd_twine.h"
#include "li_lineate.h"

static void prepare_therm(TETendril **therm) {
	TEScanner scanner;
	DDArrTETendril tendrils;

	DD_INIT_ARRAY(&tendrils);

	char *therm_src = read_file("./static/festival/therm.te");
	init_te_scanner(&scanner, therm_src);

	parse_tendrils(&scanner, &tendrils);

	*therm = &tendrils.elems[0];
}

void init_festival(FEstival *festival) {
	festival->therm = DD_ALLOCATE(TETendril, 1);
	prepare_therm(&festival->therm);
	festival->therm_state = 0;
	festival->raw = DD_ALLOCATE(DDTwine, 1);
	dd_twine_init(festival->raw);
	festival->word_bounds = DD_ALLOCATE(DDArrDDTwineWB, 1);
	DD_INIT_ARRAY(festival->word_bounds);
	festival->dict = fopen("./static/cmudict/raw.txt", "r");
	if (!festival->dict) {
		fprintf(stderr, "Could not open dict file");
		exit(1);
	}
	festival->dict_entries = DD_ALLOCATE(DDArrDIIndexedEntry, 1);
	DD_INIT_ARRAY(festival->dict_entries);
	festival->line_indices = DD_ALLOCATE(DDArrInt, 1);
	DD_INIT_ARRAY(festival->line_indices);
}

static void advance_therm(FEstival *festival) {
	DDArrDDArrDDString strs;
	DDTwine tw;
	int i, j, match_found, rand_elem;

	DD_INIT_ARRAY(&strs);
	festival->therm_state = te_transition(festival->therm,
			festival->therm_state);
	match_found = get_te_tendril_content(&strs, festival->therm_state,
			festival->therm);
	if (match_found) {
		rand_elem = rand() % strs.size;
		for (i = 0; i < strs.elems[rand_elem].size; i++) {
			dd_twine_init(&tw);
			dd_twine_from_dd_str(&tw, &strs.elems[rand_elem].elems[i]);
			dd_twine_concat_with_char_mut(festival->raw, &tw, ' ');
			dd_twine_destroy(&tw);
		}
	}

	for (i = 0; i < strs.size; i++) {
		for (j = 0; j < strs.elems[i].size; j++) {
			free_dd_chars(&strs.elems[i].elems[j]);
		}
		DD_FREE_ARRAY(&strs.elems[i]);
	}
	DD_FREE_ARRAY(&strs);
}

static void lineate_and_print(FEstival *festival) {
	int furthest_word_bounds, orig_word_bounds_size,
		dict_offset;
	int *brk_idx = DD_ALLOCATE(int, 1);

	orig_word_bounds_size = festival->word_bounds->size;
	if (orig_word_bounds_size) {
		furthest_word_bounds = festival->word_bounds->elems
			[orig_word_bounds_size - 1].end;
	} else {
		furthest_word_bounds = 0;
	}
	dd_twine_word_bounds_substr(festival->word_bounds, festival->raw,
			furthest_word_bounds, dd_twine_len(festival->raw));

	dict_offset = festival->dict_entries->size;
	di_entries_from_wbs(festival->dict_entries, orig_word_bounds_size,
			festival->word_bounds, festival->raw, festival->dict);

	while (true) {
		li_lineate_trochee_2(brk_idx, dict_offset, festival->dict_entries);

		if (*brk_idx == -1) {
			break;
		}
		dict_offset = *brk_idx;
		DD_ADD_ARRAY(festival->line_indices, 
				festival->dict_entries->elems[*brk_idx].index);
	}
}

void inaugurate_festival(FEstival *festival) {

	int i;
	for (i = 0; i < 15; i++) {
		advance_therm(festival);
	}	
	lineate_and_print(festival);
	for (i = 0; i < 15; i++) {
		advance_therm(festival);
	}	
	lineate_and_print(festival);
}

void destroy_festival(FEstival *festival) {
	int i;
	/* destroy therm */
	dd_twine_destroy(festival->raw);
	free(festival->raw);

	fclose(festival->dict);
	for (i = 0; i < festival->dict_entries->size; i++) {
		free_di_dict_entry(&festival->dict_entries->elems[i].entry);
	}
	DD_FREE_ARRAY(festival->dict_entries);
	free(festival->dict_entries);

	DD_FREE_ARRAY(festival->word_bounds);
	free(festival->word_bounds);

	DD_FREE_ARRAY(festival->line_indices);
	free(festival->line_indices);
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
