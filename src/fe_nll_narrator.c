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

static void easy_concat(DDString *target, DDString *b) {
	DDString tmp;
	init_dd_string(&tmp);
	give_to_dd_string(&tmp, " ", 1);
	dd_string_concat_mutate(target, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);
	dd_string_concat_mutate(target, b);
	free_dd_chars(b);
	init_dd_string(b);
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

static int coin_comp(double amt) {
	if (amt >= 0.0005)
		return 1;
	else if (amt <= -0.0005)
		return -1;
	else
		return 0;
}

static void print_coin(DDString *target, double amt) {
	int written;
	char tmp[10];
	if (amt < 0.0) {
		amt = -amt;
	}
	written = snprintf((char * restrict)&tmp, sizeof(char) * 10, "%.*f", 
			3, amt);
	give_to_dd_string(target, (const char*)&tmp, written);
}

static void narrate_name(FENLLNarrator *narrator) {
	DDString name_str;
	DDString add_str;
	DDString descrip_str;
	init_dd_string(&name_str);
	init_dd_string(&add_str);
	init_dd_string(&descrip_str);

	pick_one_string(&descrip_str, 3,
				"exhalation crackling past sunk-in-mucous windpipe",
				"as roughly semantized as citified beech bark",
				"their specialized by evolution ass skin itchy");

	if (rand() % 2) {
		dd_copy_dd_string(&name_str, &descrip_str);
		dd_copy_dd_string(&add_str, &narrator->lander->name);
		easy_concat(&name_str, &add_str);
	} else {
		dd_copy_dd_string(&name_str, &narrator->lander->name);
		easy_concat(&name_str, &descrip_str);
	}

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

static void narrate_has_outstanding(DDString *aggregate,
		FENLLTurnLogPocket *tl_pocket) {
	DDString tmp;
	bool fuse_entered;
	init_dd_string(&tmp);
	fuse_entered = false;

	if (coin_comp(tl_pocket->outstanding.fuse_coin) != 0 &&
			coin_comp(tl_pocket->outstanding.frux_coin) != 0) {
		give_to_dd_string(&tmp, "and has", 7);
		easy_concat(aggregate, &tmp);

		if (coin_comp(tl_pocket->outstanding.fuse_coin) == 1) {
			fuse_entered = true;
			pick_one_string(&tmp, 2,
					"illiquid assets of",
					"claims valued at");
			easy_concat(aggregate, &tmp);
			
			print_coin(&tmp, tl_pocket->outstanding.fuse_coin);
			easy_concat(aggregate, &tmp);

			give_to_dd_string(&tmp, "fuse coin", 9);
			easy_concat(aggregate, &tmp);
		} else if (coin_comp(tl_pocket->outstanding.fuse_coin) == -1) {
			fuse_entered = true;
			pick_one_string(&tmp, 1,
					"liabilities of");
			easy_concat(aggregate, &tmp);
			
			print_coin(&tmp, tl_pocket->outstanding.fuse_coin);
			easy_concat(aggregate, &tmp);

			give_to_dd_string(&tmp, "fuse coin", 9);
			easy_concat(aggregate, &tmp);
		}
		if (coin_comp(tl_pocket->outstanding.frux_coin) == 1) {
			if (fuse_entered) {
				give_to_dd_string(&tmp, "and", 3);
				easy_concat(aggregate, &tmp);
			}
			pick_one_string(&tmp, 2,
					"illiquid assets of",
					"claims valued at");
			easy_concat(aggregate, &tmp);
			
			print_coin(&tmp, tl_pocket->outstanding.frux_coin);
			easy_concat(aggregate, &tmp);

			give_to_dd_string(&tmp, "frux coin", 9);
			easy_concat(aggregate, &tmp);
		} else if (coin_comp(tl_pocket->outstanding.frux_coin) == -1) {
			if (fuse_entered) {
				give_to_dd_string(&tmp, "and", 3);
				easy_concat(aggregate, &tmp);
			}
			pick_one_string(&tmp, 1,
					"liabilities of");
			easy_concat(aggregate, &tmp);
			
			print_coin(&tmp, tl_pocket->outstanding.frux_coin);
			easy_concat(aggregate, &tmp);

			give_to_dd_string(&tmp, "frux coin", 9);
			easy_concat(aggregate, &tmp);
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

	give_to_dd_string(&has_str, "has", 3);

	pick_one_string(&tmp, 2,
			"to their dad-pronounced and debt-market-comprehended name",
			"ajangle in blank fear birthed musk stanked pocket");
	easy_concat(&has_str, &tmp);

	print_coin(&tmp, tl_pocket.in_pocket.fuse_coin);
	easy_concat(&has_str, &tmp);

	give_to_dd_string(&tmp, "fuse coin", 9);
	easy_concat(&has_str, &tmp);

	give_to_dd_string(&tmp, "and", 3);
	easy_concat(&has_str, &tmp);

	print_coin(&tmp, tl_pocket.in_pocket.frux_coin);
	easy_concat(&has_str, &tmp);

	give_to_dd_string(&tmp, "frux coin", 9);
	easy_concat(&has_str, &tmp);

	narrate_has_outstanding(&has_str, &tl_pocket);

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
	pick_one_string(&str, 3,
			"across song-corrugated commercial spaces",
			"beyond veilingly interwoven diode-clustering smart monitors",
			"past interstate treelines, where ghost and cryptid stage their conflict");
	
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

static void add_seen_liabilities(DDString *target,
		FENLLTurnLogLanderSees *sees) {
	DDString tmp;
	bool fuse_entry;

	init_dd_string(&tmp);
	fuse_entry = false;

	give_to_dd_string(&tmp, " who ", 5);
	dd_string_concat_mutate(target, &tmp);
	free_dd_chars(&tmp);
	init_dd_string(&tmp);

	if (sees->outstanding.fuse_coin > 0.0004) {
		fuse_entry = true;
		give_to_dd_string(&tmp, "is owed ", 8);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		print_coin(&tmp, sees->outstanding.fuse_coin);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		give_to_dd_string(&tmp, " fuse", 5);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

	} else if (sees->outstanding.fuse_coin < -0.0004) {
		fuse_entry = true;
		give_to_dd_string(&tmp, "owes ", 5);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		print_coin(&tmp, sees->outstanding.fuse_coin);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		give_to_dd_string(&tmp, " fuse", 5);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);
	} 
	
	if (sees->outstanding.frux_coin > 0.0004) {
		if (fuse_entry) {
			give_to_dd_string(&tmp, " and ", 5);
			dd_string_concat_mutate(target, &tmp);
			free_dd_chars(&tmp);
			init_dd_string(&tmp);
		}
		give_to_dd_string(&tmp, "is owed ", 8);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		print_coin(&tmp, sees->outstanding.frux_coin);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		give_to_dd_string(&tmp, " frux", 5);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);
	} else if (sees->outstanding.frux_coin < -0.0004) {
		if (fuse_entry) {
			give_to_dd_string(&tmp, " and ", 5);
			dd_string_concat_mutate(target, &tmp);
			free_dd_chars(&tmp);
			init_dd_string(&tmp);
		}
		give_to_dd_string(&tmp, "owes ", 5);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		print_coin(&tmp, sees->outstanding.frux_coin);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);

		give_to_dd_string(&tmp, " frux", 5);
		dd_string_concat_mutate(target, &tmp);
		free_dd_chars(&tmp);
		init_dd_string(&tmp);
	} 
}

/* "distant youth deligitimized by later market movements"*/
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

		pick_one_string(&add_str, 3,
				"gland-wrought sweat shot gridwards",
				"memories deligitimized by later market movements",
				"naked, no clod but the coin to their name");
		easy_concat(&sees_str, &add_str);

		add_seen_liabilities(&add_str, sees);
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

static bool get_goal(FENLLTurnLogLanderHasGoal **g,
		FENLLNarrator *narrator) {
	int i;
	for (i = 0; i < narrator->lander->turn_log.size; i++) {
		if (narrator->lander->turn_log.elems[i].type ==
				FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_HAS_GOAL) {
			*g = &narrator->lander->turn_log.elems[i].value.g;
			return true;
		}
	}
	return false;
}

static void narrate_charge(FENLLTurnLogLanderHasGoal *g, 
		DDString *wants_str) {
	DDString tmp_str;
	init_dd_string(&tmp_str);

	pick_one_string(&tmp_str, 1,
			"mirage-singed loved-by-crook photoreceptors a-tracking");
	easy_concat(wants_str, &tmp_str);

	pick_one_string(&tmp_str, 1,
			"they hungrily envisage a collapse of options for");
	easy_concat(wants_str, &tmp_str);

	dd_copy_dd_string(&tmp_str, &g->goal_name);
	easy_concat(wants_str, &tmp_str);
}

static void narrate_flee(FENLLTurnLogLanderHasGoal *g, 
		DDString *wants_str) {
	DDString tmp_str;
	bool fuse_entered = false;

	init_dd_string(&tmp_str);

	pick_one_string(&tmp_str, 2,
			"their thin shoes sharply dampened by snow",
			"their weary-of-horror glands sputtering",
			"theirs as quiveringly explosive as a mouse's heart");
	easy_concat(wants_str, &tmp_str);

	pick_one_string(&tmp_str, 2,
			"they drive in desperate emission shocked flight from",
			"they sprint fearfully between towered-high palettes from");
	easy_concat(wants_str, &tmp_str);

	dd_copy_dd_string(&tmp_str, &g->goal_name);
	easy_concat(wants_str, &tmp_str);

	give_to_dd_string(&tmp_str, "whom they owe", 13);
	easy_concat(wants_str, &tmp_str);

	if (g->outstanding.fuse_coin < -0.0004) {
		fuse_entered = true;
		print_coin(&tmp_str, g->outstanding.fuse_coin);
		easy_concat(wants_str, &tmp_str);

		give_to_dd_string(&tmp_str, "fuse", 4);
		easy_concat(wants_str, &tmp_str);
	}

	if (g->outstanding.frux_coin < -0.0004) {
		if (fuse_entered) {
			give_to_dd_string(&tmp_str, "and", 3);
			easy_concat(wants_str, &tmp_str);
		}
		print_coin(&tmp_str, g->outstanding.frux_coin);
		easy_concat(wants_str, &tmp_str);

		give_to_dd_string(&tmp_str, "frux", 4);
		easy_concat(wants_str, &tmp_str);
	}


}

static void narrate_wants(FENLLNarrator *narrator) {
	DDString wants_str;
	FENLLTurnLogLanderHasGoal *g;

	g = NULL;
	init_dd_string(&wants_str);

	if (get_goal(&g, narrator)) {
		if (g->goal_type == FE_NLL_GOAL_TYPE_NULL) {
			give_to_dd_string(&wants_str, "null goal", 9);
		} else if (g->goal_type == FE_NLL_GOAL_TYPE_FLEE) {
			narrate_flee(g, &wants_str);
		} else if (g->goal_type == FE_NLL_GOAL_TYPE_CHARGE) {
			narrate_charge(g, &wants_str);
		}
	} else {
		pick_one_string(&wants_str, 1,
				"moves utterly motivationlessly");
	}


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
