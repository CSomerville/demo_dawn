#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "dd_data.h"
#include "fe_neo_lib_land.h"
#include "te_tendril.h"
#include "fe_nll_narrator.h"

static void init_fe_nll_narrator(FENLLNarrator *narrator, 
		DDArrDDString *result, FENLLander *lander, TETendril *tendril) {
	int i;
	DD_INIT_ARRAY(&narrator->unused_sees_indices);

	narrator->result = result;
	narrator->lander = lander;
	narrator->tendril = tendril;
	narrator->state = tendril->start;

	for (i = 0; i < lander->turn_log.size; i++) {
		if (lander->turn_log.elems[i].type ==
				FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_SEES) {
			DD_ADD_ARRAY(&narrator->unused_sees_indices, i);
		}
	}
}

static void narrator_te_transition(FENLLNarrator *narrator) {
	narrator->state = te_transition(narrator->tendril, narrator->state);
}

static void pick_one_string(DDString *target, int count, ...) {
	va_list str_pt;
	char *str;
	int i, j;

	va_start(str_pt, count);
	j = rand() % count;
	for (i = 0; i <= j; i++) {
		str = va_arg(str_pt, char *);
	}
	give_to_dd_string(target, (const char *)str, 
			strlen((const char *)str));
	va_end(str_pt);
}

static void print_coin(DDString *target, double amt) {
	int written;
	char tmp[10];
	written = snprintf((char * restrict)&tmp, sizeof(char) * 10, "%.*f", 
			3, amt);
	give_to_dd_string(target, (const char*)&tmp, written);
}

static void narrate_name(FENLLNarrator *narrator) {
	DDString name_str;
	init_dd_string(&name_str);

	pick_one_string(&name_str, 2,
			"exhalation crackling past sunk-in-mucous windpipe ",
			"crudely trauma-laced as urban beech bark ");

	dd_string_concat_mutate(&name_str, &narrator->lander->name);

	DD_ADD_ARRAY(narrator->result, name_str);
}

static void narrate_and(FENLLNarrator *narrator) {
	DDString and_str;
	init_dd_string(&and_str);
	give_to_dd_string(&and_str, "and", 3);
	DD_ADD_ARRAY(narrator->result, and_str);
}

static void get_first_tl_pocket(FENLLTurnLogPocket *tl_pocket,
		FENLLNarrator *narrator) {
	int i;
	for (i = 0; i < narrator->lander->turn_log.size; i++) {
		if (narrator->lander->turn_log.elems[i].type ==
				FE_NLL_TURN_LOG_ITEM_TYPE_POCKET) {
			*tl_pocket = narrator->lander->turn_log.elems[i].value.p;
			break;
		}
	}
}

static void narrate_has(FENLLNarrator *narrator) {
	DDString has_str;
	DDString tmp;
	FENLLTurnLogPocket tl_pocket;

	init_dd_string(&has_str);
	init_dd_string(&tmp);
	get_first_tl_pocket(&tl_pocket, narrator);

	give_to_dd_string(&has_str, "has ", 4);

	pick_one_string(&tmp, 1,
			"to their dad-pronounced and debt-market-comprehended name ");
	dd_string_concat_mutate(&has_str, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	print_coin(&tmp, tl_pocket.in_pocket.fuse_coin);
	dd_string_concat_mutate(&has_str, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	give_to_dd_string(&tmp, " fuse coin ", 11);
	dd_string_concat_mutate(&has_str, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	give_to_dd_string(&tmp, " and ", 5);
	dd_string_concat_mutate(&has_str, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	print_coin(&tmp, tl_pocket.in_pocket.frux_coin);
	dd_string_concat_mutate(&has_str, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	give_to_dd_string(&tmp, " frux coin ", 11);
	dd_string_concat_mutate(&has_str, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	DD_ADD_ARRAY(narrator->result, has_str);
}

static FENLLTurnLogLanderSees *pick_turn_log_sees(
		FENLLNarrator *narrator) {
	int n, m;
	n = rand() % narrator->unused_sees_indices.size;
	m = narrator->unused_sees_indices.elems[n];
	DD_REMOVE_ARRAY(&narrator->unused_sees_indices, n);
	return &narrator->lander->turn_log.elems[m].value.s;
}

static void narrate_sees_across(FENLLNarrator *narrator) {
	DDString str;
	init_dd_string(&str);
	pick_one_string(&str, 1,
			"across song-corrugated commercial spaces");
	
	DD_ADD_ARRAY(narrator->result, str);
}

static void narrate_sees_nothing(FENLLNarrator *narrator) {
	DDString str;
	init_dd_string(&str);
	pick_one_string(&str, 2,
			"vaporous emptiness",
			"economically useless material");
	DD_ADD_ARRAY(narrator->result, str);
}

static void narrate_sees_object(FENLLNarrator *narrator) {
	DDString sees_str;
	DDString add_str;
	FENLLTurnLogLanderSees *sees;

	if (narrator->unused_sees_indices.size == 0) {
		narrate_sees_nothing(narrator);
	} else {
		sees = NULL;
		init_dd_string(&sees_str);
		init_dd_string(&add_str);
		sees = pick_turn_log_sees(narrator);
		assert(sees != NULL);

		dd_string_concat_mutate(&sees_str, &sees->seen_name);

		give_to_dd_string(&add_str, " sweat-streaked ", 16);
		dd_string_concat_mutate(&sees_str, &add_str);
		free_dd_chars(&add_str);
		init_dd_string(&add_str);

		print_coin(&add_str, sees->outstanding.fuse_coin);
		dd_string_concat_mutate(&sees_str, &add_str);
		free_dd_chars(&add_str);
		init_dd_string(&add_str);

		give_to_dd_string(&add_str, " fuse", 5);
		dd_string_concat_mutate(&sees_str, &add_str);
		free_dd_chars(&add_str);
		init_dd_string(&add_str);

		DD_ADD_ARRAY(narrator->result, sees_str);
	}
}

static void narrate_sees_null(FENLLNarrator *narrator) {
	DDString sees_str;
	init_dd_string(&sees_str);

	give_to_dd_string(&sees_str, "sees", 4);
	DD_ADD_ARRAY(narrator->result, sees_str);
}

static void narrate_sees(FENLLNarrator *narrator) {
	DDString key;
	DDString value;

	init_dd_string(&key);
	init_dd_string(&value);

	give_to_dd_string(&key, "SeesCurrent", 11);
	get_value_from_state(&value, narrator->tendril, narrator->state, &key);

	if (!strcmp(value.chars, "Null")) {
		narrate_sees_null(narrator);
	} else if (!strcmp(value.chars, "Across")) {
		narrate_sees_across(narrator);
	} else if (!strcmp(value.chars, "Object")) {
		narrate_sees_object(narrator);
	} else {
		abort();
	}
}

static void narrate_wants(FENLLNarrator *narrator) {
	DDString wants_str;
	init_dd_string(&wants_str);
	give_to_dd_string(&wants_str, "wants goes here", 15);
	DD_ADD_ARRAY(narrator->result, wants_str);
}

static void narrate_state(FENLLNarrator *narrator) {
	DDString key;
	DDString value;

	init_dd_string(&key);
	init_dd_string(&value);

	give_to_dd_string(&key, "Seq", 3);
	get_value_from_state(&value, narrator->tendril, narrator->state, &key);

	if (!strcmp(value.chars, "Name")) {
		narrate_name(narrator);
	} else if (!strcmp(value.chars, "And")) {
		narrate_and(narrator);
	} else if (!strcmp(value.chars, "Has")) {
		narrate_has(narrator);
	} else if (!strcmp(value.chars, "Sees")) {
		narrate_sees(narrator);
	} else if (!strcmp(value.chars, "Wants")) {
		narrate_wants(narrator);
	} else {
		abort();
	}
}

void narrate_from_turn_log(DDArrDDString *result, FENLLander *lander,
		TETendril *tendril) {
	int i;
	FENLLNarrator narrator;
	init_fe_nll_narrator(&narrator, result, lander, tendril);

	narrate_state(&narrator);
	for (i = 0; i < 20; i++) {
		narrator_te_transition(&narrator);
		narrate_state(&narrator);
	}
}
