#include <stdio.h>
#include "dd_utils.h"
#include "te_tendril.h"
#include "di_lib.h"
#include "fe_lib.h"
#include "dd_twine.h"
#include "li_lineate.h"
#include "fe_neo_lib_land.h"
#include "fe_nll_narrator_2.h"

static void prepare_therm(TETendril **therm) {
	TEScanner scanner;
	DDArrTETendril tendrils;

	DD_INIT_ARRAY(&tendrils);

	char *therm_src = read_file("./static/festival/therm.te");
	init_te_scanner(&scanner, therm_src);

	parse_tendrils(&scanner, &tendrils);

	*therm = &tendrils.elems[0];
}

static void prepare_nll_world(FENLLWorld *nll_world) {
	FENLLConfigureWorld conf;
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
	conf.num_population = 25;
	conf.names = names;
	conf.pocket_floor = 20.0;
	conf.pocket_ceil = 100.0;
	conf.amt_floor = 15.0;
	conf.amt_ceil = 75.0;
	conf.interest_floor = 0.01;
	conf.interest_ceil = 0.05;
	conf.mode = FE_NLL_MODE_SILENT;

	fe_nll_init_world(nll_world, &conf);
}

void init_festival(FEstival *festival) {
	festival->therm = DD_ALLOCATE(TETendril, 1);
	prepare_therm(&festival->therm);
	festival->therm_state = 0;
	festival->nll_world = DD_ALLOCATE(FENLLWorld, 1);
	prepare_nll_world(festival->nll_world);
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

static void advance_nll(FEstival *festival, int nsteps, int lander_index) {
	int i;
	DDArrDDTwine result_arr;

	DD_INIT_ARRAY(&result_arr);
	for (i = 0; i < nsteps; i++) {
		fe_nll_tick(festival->nll_world);
	}
	narrate_from_turn_log(&result_arr, 
			&festival->nll_world->populace.elems[lander_index]);
	for (i = 0; i < result_arr.size; i++) {
		dd_twine_concat_with_char_mut(festival->raw,
				&result_arr.elems[i], ' ');
	}

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
	advance_nll(festival, 10, 3);
}

void destroy_festival(FEstival *festival) {
	int i;
	/* destroy therm */
	fe_nll_free_world(festival->nll_world);
	free(festival->nll_world);

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
