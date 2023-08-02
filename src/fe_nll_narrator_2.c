#include "fe_nll_narrator_2.h"
#include "fe_neo_lib_land.h"
#include "dd_data.h"
#include "dd_twine.h"
#include "dd_twine_ball_lib.h"

static void concat_with_space_mut(DDTwine *twa, DDTwine *twb) {
	DDTwine space;
	dd_twine_init(&space);
	if (twa->length > 0) {
		dd_twine_from_chars_fixed(&space, " ", 1);
		dd_twine_concat_mut(twa, &space);
	}
	dd_twine_concat_mut(twa, twb);
	dd_twine_destroy(&space);
	dd_twine_destroy(twb);
	dd_twine_init(twb);
}

static int coin_comp(double amt) {
	if (amt >= 0.0005)
		return 1;
	else if (amt <= -0.0005)
		return -1;
	else
		return 0;
}

static void twine_from_key(DDTwine *val, FENLLNarrator *narrator,
		const char *key) {
	int i;
	DDTwine tmp_key;
	DDTwineBallItem *item;

	dd_twine_init(&tmp_key);
	dd_twine_from_chars_dyn(&tmp_key, key);

	if (!dd_twine_ball_obj_get(&item, narrator->narrator_conf->value.o,
				&tmp_key) ||
			item->type != DD_TWINE_BALL_LIST) {
		fprintf(stderr, "obj not found for key: %s",
				dd_twine_chars(&tmp_key));
		exit(1);
	}
	i = rand() % item->value.l->items.size;
	if (item->value.l->items.elems[i].type != DD_TWINE_BALL_TWINE) {
		fprintf(stderr, "val not found for key: %s",
				dd_twine_chars(&tmp_key));
		exit(1);
	}
	dd_twine_copy(val, item->value.l->items.elems[i].value.t);

	dd_twine_destroy(&tmp_key);
}

static void indices_of_turn_log_type(DDArrInt *indices,
		DDArrFENLLTurnLogItem *tl_items, FENLLTurnLogItemType tl_type) {
	int i;
	for (i = 0; i < tl_items->size; i++)
		if (tl_items->elems[i].type == tl_type)
			DD_ADD_ARRAY(indices, i);
}

static void print_amt(DDTwine *target, double amt) {
	int written;
	char tmp[10];
	if (amt < 0.0) {
		amt = -amt;
	}
	written = snprintf((char * restrict)&tmp, sizeof(char) * 10, "%.*f", 
			3, amt);
	dd_twine_from_chars_fixed(target, (const char*)&tmp, written);
}

static void print_coin_amt(DDTwine *target, double amt,
		FENLLCoin coin_type) {
	DDTwine tmp;
	dd_twine_init(&tmp);

	print_amt(target, amt);
	if (coin_type == FE_NLL_COIN_FUSE) {
		if (rand() % 2) {
			dd_twine_from_chars_fixed(&tmp, " fuse coin", 10);
		} else {
			dd_twine_from_chars_fixed(&tmp, " fuse", 5);
		}
	} else {
		if (rand() % 2) {
			dd_twine_from_chars_fixed(&tmp, " frux coin", 10);
		} else {
			dd_twine_from_chars_fixed(&tmp, " frux", 5);
		}
	}
	dd_twine_concat_mut(target, &tmp);
	dd_twine_destroy(&tmp);

}

static void narrate_name(FENLLNarrator *narrator) {
	DDTwine name;
	DDTwine description;

	dd_twine_init(&name);
	dd_twine_init(&description);

	dd_twine_from_dd_str(&name, &narrator->lander->name);
	twine_from_key(&description, narrator, "lander_descriptor");

	if (rand() % 2) {
		DD_ADD_ARRAY(narrator->result, name);
		DD_ADD_ARRAY(narrator->result, description);
	} else {
		DD_ADD_ARRAY(narrator->result, description);
		DD_ADD_ARRAY(narrator->result, name);
	}
}

static void narrate_and(FENLLNarrator *narrator) {
	DDTwine and_tw;
	dd_twine_init(&and_tw);
	dd_twine_from_chars_fixed(&and_tw, "and", 3);
	DD_ADD_ARRAY(narrator->result, and_tw);
}

static void narrate_has_in_pocket(FENLLNarrator *narrator,
		FENLLPocket *pocket) {
	DDTwine has_base;
	DDTwine append;

	dd_twine_init(&has_base);
	dd_twine_init(&append);

	dd_twine_from_chars_fixed(&has_base, "has", 3);
	twine_from_key(&append, narrator, "pocket_descriptor");
	concat_with_space_mut(&has_base, &append);

	print_coin_amt(&append, pocket->fuse_coin, FE_NLL_COIN_FUSE);
	concat_with_space_mut(&has_base, &append);

	dd_twine_from_chars_fixed(&append, "and", 3);
	concat_with_space_mut(&has_base, &append);

	print_coin_amt(&append, pocket->frux_coin, FE_NLL_COIN_FRUX);
	concat_with_space_mut(&has_base, &append);

	DD_ADD_ARRAY(narrator->result, has_base);
}

static void print_outstanding_amt(DDTwine *claims_str,
		FENLLNarrator *narrator, double amt, FENLLCoin coin_type,
		bool is_claims) {
	DDTwine append;

	dd_twine_init(&append);
	if (is_claims) {
		twine_from_key(&append, narrator,
				"outstanding_credits_descriptor");
	} else {
		twine_from_key(&append, narrator, "outstanding_debts_descriptor");
	}
	concat_with_space_mut(claims_str, &append);

	print_coin_amt(&append, amt, coin_type);
	concat_with_space_mut(claims_str, &append);
}

static void narrate_has_outstanding(FENLLNarrator *narrator,
		FENLLPocket *outstanding) {
	DDTwine has_base;
	DDTwine append;
	bool fuse_entered = false;

	dd_twine_init(&has_base);
	dd_twine_init(&append);

	if (coin_comp(outstanding->fuse_coin) != 0 &&
			coin_comp(outstanding->frux_coin) != 0) {
		dd_twine_from_chars_fixed(&has_base, "has", 3);
		if (coin_comp(outstanding->fuse_coin) == 1) {
			fuse_entered = true;
			print_outstanding_amt(&has_base, narrator, outstanding->fuse_coin,
					FE_NLL_COIN_FUSE, true);
		} else {
			fuse_entered = true;
			print_outstanding_amt(&has_base, narrator, outstanding->fuse_coin,
					FE_NLL_COIN_FUSE, false);
		}
		if (coin_comp(outstanding->frux_coin) == 1) {
			if (fuse_entered) {
				dd_twine_from_chars_fixed(&append, "and", 3);
				concat_with_space_mut(&has_base, &append);
			}
			print_outstanding_amt(&has_base, narrator, outstanding->frux_coin,
					FE_NLL_COIN_FRUX, true);
		} else {
			if (fuse_entered) {
				dd_twine_from_chars_fixed(&append, "and", 3);
				concat_with_space_mut(&has_base, &append);
			}
			print_outstanding_amt(&has_base, narrator, outstanding->frux_coin,
					FE_NLL_COIN_FRUX, false);
		}
	}
	if (has_base.length)
		DD_ADD_ARRAY(narrator->result, has_base);

}

static void narrate_has(FENLLNarrator *narrator) {
	DDArrInt indices;
	DD_INIT_ARRAY(&indices);
	indices_of_turn_log_type(&indices, &narrator->lander->turn_log,
			FE_NLL_TURN_LOG_ITEM_TYPE_POCKET);
	narrate_has_in_pocket(narrator, 
			&narrator->lander->turn_log.elems[indices.elems[0]].value.p
				.in_pocket);
	narrate_and(narrator);
	narrate_has_outstanding(narrator,
			&narrator->lander->turn_log.elems[indices.elems[0]].value.p
				.outstanding);

	DD_FREE_ARRAY(&indices);
}

static void print_sees_across(DDTwine *base, FENLLNarrator *narrator) {
	DDTwine append;
	dd_twine_init(&append);
	twine_from_key(&append, narrator, "sees_across");
	concat_with_space_mut(base, &append);
}

static void print_sees_nothing(DDTwine *base, FENLLNarrator *narrator) {
	DDTwine append;
	dd_twine_init(&append);
	twine_from_key(&append, narrator, "emptiness");
	concat_with_space_mut(base, &append);
}

static void print_sees_object(DDTwine *base, FENLLNarrator *narrator,
		FENLLTurnLogLanderSees *sees) {
	DDTwine append;
	bool fuse_entered = false;
	dd_twine_init(&append);

	dd_twine_from_dd_str(&append, &sees->seen_name);
	concat_with_space_mut(base, &append);

	twine_from_key(&append, narrator, "seen_lander_descriptor");
	concat_with_space_mut(base, &append);

	if (coin_comp(sees->outstanding.fuse_coin) != 0 &&
			coin_comp(sees->outstanding.frux_coin) != 0) {
		if (coin_comp(sees->outstanding.fuse_coin) == 1) {
			fuse_entered = true;
			dd_twine_from_chars_fixed(&append, "owes", 4);
			concat_with_space_mut(base, &append);
			print_coin_amt(&append, sees->outstanding.fuse_coin,
					FE_NLL_COIN_FUSE);
			concat_with_space_mut(base, &append);
		} else if (coin_comp(sees->outstanding.fuse_coin) == -1) {
			fuse_entered = true;
			dd_twine_from_chars_fixed(&append, "is owed", 7);
			concat_with_space_mut(base, &append);
			print_coin_amt(&append, sees->outstanding.fuse_coin,
					FE_NLL_COIN_FUSE);
			concat_with_space_mut(base, &append);
		}
		if (coin_comp(sees->outstanding.frux_coin) == 1) {
			if (fuse_entered) {
				dd_twine_from_chars_fixed(&append, "and", 3);
				concat_with_space_mut(base, &append);
			}
			dd_twine_from_chars_fixed(&append, "owes", 4);
			concat_with_space_mut(base, &append);
			print_coin_amt(&append, sees->outstanding.frux_coin,
					FE_NLL_COIN_FRUX);
			concat_with_space_mut(base, &append);
		} else if (coin_comp(sees->outstanding.frux_coin) == -1){
			if (fuse_entered) {
				dd_twine_from_chars_fixed(&append, "and", 3);
				concat_with_space_mut(base, &append);

			}
			dd_twine_from_chars_fixed(&append, "is owed", 7);
			concat_with_space_mut(base, &append);
			print_coin_amt(&append, sees->outstanding.frux_coin,
					FE_NLL_COIN_FRUX);
			concat_with_space_mut(base, &append);
		}
	}
}

static void narrate_sees(FENLLNarrator *narrator) {
	DDArrInt indices;
	DDTwine sees_base;
	DDTwine append;
	int i, across_pos;
	int times = (rand() % 3) + 1;

	DD_INIT_ARRAY(&indices);
	dd_twine_init(&sees_base);
	dd_twine_init(&append);

	indices_of_turn_log_type(&indices, &narrator->lander->turn_log,
			FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_SEES);
	DD_SHUFFLE_ARRAY(int, &indices);
	for (i = 0; i < times; i++) {
		if (i == 0) {
			dd_twine_from_chars_fixed(&append, "sees", 4);
		} else {
			dd_twine_from_chars_fixed(&append, "and sees", 8);
		}
		concat_with_space_mut(&sees_base, &append);

		across_pos = rand() % 2;
		if (across_pos) {
			print_sees_across(&sees_base, narrator);
		}
		if (i < indices.size) {
			print_sees_object(&sees_base, narrator,
					&narrator->lander->turn_log.elems[indices.elems[i]]
					.value.s);
		} else {
			print_sees_nothing(&sees_base, narrator);
		}
		if (!across_pos) {
			print_sees_across(&sees_base, narrator);
		}
	}

	DD_ADD_ARRAY(narrator->result, sees_base);
	DD_FREE_ARRAY(&indices);
}

static void print_charge(DDTwine *base, FENLLNarrator *narrator, 
		FENLLTurnLogLanderHasGoal *g) {
	DDTwine append;
	bool fuse_entered = false;
	dd_twine_init(&append);
	twine_from_key(&append, narrator, "charging_lander_descriptor");
	concat_with_space_mut(base, &append);
	twine_from_key(&append, narrator, "charging_lander_does");
	concat_with_space_mut(base, &append);
	dd_twine_from_dd_str(&append, &g->goal_name);
	concat_with_space_mut(base, &append);
	dd_twine_from_chars_fixed(&append, "who owes them", 13);
	concat_with_space_mut(base, &append);
	if (coin_comp(g->outstanding.fuse_coin) == 1) {
		fuse_entered = true;
		print_coin_amt(&append, g->outstanding.fuse_coin,
				FE_NLL_COIN_FUSE);
		concat_with_space_mut(base, &append);
	}
	if (coin_comp(g->outstanding.frux_coin) == 1) {
		if (fuse_entered) {
			dd_twine_from_chars_fixed(&append, "and", 3);
			concat_with_space_mut(base, &append);
		}
		print_coin_amt(&append, g->outstanding.frux_coin,
				FE_NLL_COIN_FRUX);
		concat_with_space_mut(base, &append);
	}
}

static void print_flee(DDTwine *base, FENLLNarrator *narrator, 
		FENLLTurnLogLanderHasGoal *g) {
	DDTwine append;
	bool fuse_entered = false;
	dd_twine_init(&append);
	twine_from_key(&append, narrator, "fleeing_lander_descriptor");
	concat_with_space_mut(base, &append);
	twine_from_key(&append, narrator, "fleeing_lander_does");
	concat_with_space_mut(base, &append);
	dd_twine_from_dd_str(&append, &g->goal_name);
	concat_with_space_mut(base, &append);
	dd_twine_from_chars_fixed(&append, "whom they owe", 13);
	concat_with_space_mut(base, &append);
	if (coin_comp(g->outstanding.fuse_coin) == -1) {
		fuse_entered = true;
		print_coin_amt(&append, g->outstanding.fuse_coin,
				FE_NLL_COIN_FUSE);
		concat_with_space_mut(base, &append);
	}
	if (coin_comp(g->outstanding.frux_coin) == -1) {
		if (fuse_entered) {
			dd_twine_from_chars_fixed(&append, "and", 3);
			concat_with_space_mut(base, &append);
		}
		print_coin_amt(&append, g->outstanding.frux_coin,
				FE_NLL_COIN_FRUX);
		concat_with_space_mut(base, &append);
	}
}

static void print_wants_nothing(DDTwine *base, FENLLNarrator *narrator) {
	DDTwine append;
	dd_twine_init(&append);
	twine_from_key(&append, narrator, "lander_has_no_goal");
	concat_with_space_mut(base, &append);
}

static void narrate_wants(FENLLNarrator *narrator) {
	DDTwine wants_base;
	DDArrInt indices;
	FENLLTurnLogLanderHasGoal *g;

	DD_INIT_ARRAY(&indices);
	dd_twine_init(&wants_base);

	indices_of_turn_log_type(&indices, &narrator->lander->turn_log,
			FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_HAS_GOAL);
	if (indices.size) {
		g = &narrator->lander->turn_log.elems[indices.elems[0]].value.g;
		if (g->goal_type == FE_NLL_GOAL_TYPE_NULL) {
			fprintf(stderr, "Encountered unexpected FE_NLL_GOAL_TYPE_NULL\n");
			exit(1);
		} else if (g->goal_type == FE_NLL_GOAL_TYPE_FLEE) {
			print_flee(&wants_base, narrator, g);
		} else if (g->goal_type == FE_NLL_GOAL_TYPE_CHARGE) {
			print_charge(&wants_base, narrator, g);
		}
	} else {
		print_wants_nothing(&wants_base, narrator);
	}
	
	DD_ADD_ARRAY(narrator->result, wants_base);
	DD_FREE_ARRAY(&indices);
}

void narrate_from_turn_log(DDArrDDTwine *result, FENLLander *lander) {
	FENLLNarrator narrator;
	DDTwineBallData narrator_conf_data;

	dd_twine_ball_read(&narrator_conf_data, "./static/festival/fe_nll.twb");
	narrator.result = result;
	narrator.lander = lander;
	narrator.narrator_conf = narrator_conf_data.root;

	narrate_name(&narrator);
	narrate_has(&narrator);
	narrate_and(&narrator);
	narrate_sees(&narrator);
	narrate_and(&narrator);
	narrate_wants(&narrator);

	dd_twine_ball_data_destroy(&narrator_conf_data);
}
