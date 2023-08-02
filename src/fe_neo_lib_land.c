#include <stdio.h>
#include "dd_data.h"
#include "dd_utils.h"
#include "fe_neo_lib_land.h"

/***********************
 * Relations ***********
 * *********************
 */

static void lander_by_id(FENLLander **lander, FENLLWorld *world, int id) {
	int i;
	for (i = 0; i < world->populace.size; i++) {
		if (id == world->populace.elems[i].id) {
			*lander = &world->populace.elems[i];
			break;
		}
	}
}

// world->populace must be set.
static void set_relations(FENLLWorld *world, FENLLConfigureWorld *conf) {
	int i, j;
	double tmp;
	bool i_is_debtor;
	FENLLRelation relation;
	for (i = 0; i < world->populace.size; i++) {
		for (j = i + 1; j < world->populace.size; j++) {
			i_is_debtor = rand() % 2;
			relation.debtor_id = i_is_debtor ? i : j;
			relation.creditor_id = i_is_debtor ? j : i;
			tmp = rand_double() * (conf->amt_ceil - conf->amt_floor)
				+ conf->amt_floor;
			relation.amt = tmp;
			tmp = rand_double() * 
				(conf->interest_ceil - conf->interest_floor)
				+ conf->interest_floor;
			relation.interest = tmp;
			relation.coin = rand() % 2 ? FE_NLL_COIN_FUSE :
				FE_NLL_COIN_FRUX;
			DD_ADD_ARRAY(&world->relations, relation);
		}
	}
}

static void add_new_relation(FENLLWorld *world, int debtor_id, int
		creditor_id, double amt, double interest, FENLLCoin coin) {
	FENLLRelation rel;
	rel.debtor_id = debtor_id;
	rel.creditor_id = creditor_id;
	rel.amt = amt;
	rel.interest = interest;
	rel.coin = coin;
	DD_ADD_ARRAY(&world->relations, rel);
}

static void get_joint_relations(DDArrFENLLRelation *found,
		DDArrFENLLRelation *relations, int id_a, int id_b) {
	int i;
	for (i = 0; i < relations->size; i++) {
		if (((relations->elems[i].debtor_id == id_a &&
				relations->elems[i].creditor_id == id_b) ||
				(relations->elems[i].debtor_id == id_b &&
				relations->elems[i].creditor_id == id_a)) &&
				relations->elems[i].amt != 0.0)
			DD_ADD_ARRAY(found, relations->elems[i]);
	}
}

static void get_relations(DDArrFENLLRelation *found,
		DDArrFENLLRelation *relations, int id) {
	int i;
	for (i = 0; i < relations->size; i++) {
		if (relations->elems[i].debtor_id == id ||
				relations->elems[i].creditor_id == id) {
			DD_ADD_ARRAY(found, relations->elems[i]);
		}
	}
}

static void get_creditor_relations(DDArrInt *indices,
		DDArrFENLLRelation *relations, int id) {
	int i;
	for (i = 0; i < relations->size; i++)
		if (relations->elems[i].creditor_id == id &&
				relations->elems[i].amt > 0.0)
			DD_ADD_ARRAY(indices, i);
}


static void sum_relations(FENLLPocket *summed,
		DDArrFENLLRelation *relations, int id) {
	int i;
	summed->fuse_coin = 0.0;
	summed->frux_coin = 0.0;
	for (i = 0; i < relations->size; i++) {
		if (relations->elems[i].debtor_id == id) {
			if (relations->elems[i].coin == FE_NLL_COIN_FUSE) {
				summed->fuse_coin -= relations->elems[i].amt;
			} else {
				summed->frux_coin -= relations->elems[i].amt;
			}
		} else {
			if (relations->elems[i].coin == FE_NLL_COIN_FUSE) {
				summed->fuse_coin += relations->elems[i].amt;
			} else {
				summed->frux_coin += relations->elems[i].amt;
			}
		}	
	}
}

static void get_outstanding(FENLLPocket *outstanding, FENLLWorld *world,
		int id_self, int id_other) {
	DDArrFENLLRelation relations;
	DD_INIT_ARRAY(&relations);
	get_joint_relations(&relations, &world->relations, id_self, id_other);
	sum_relations(outstanding, &relations, id_self);
	DD_FREE_ARRAY(&relations);
}

static void apply_interest(FENLLWorld *world) {
	int i;
	for (i = 0; i < world->relations.size; i++) {
		world->relations.elems[i].amt +=
			world->relations.elems[i].amt * 
			world->relations.elems[i].interest;
	}
}

static void make_payment(DDArrFENLLRelation *relations, double amt,
		FENLLCoin coin_type, int id_self, int id_other) {
	int i;
	for (i = 0; i < relations->size; i++) {
		if (relations->elems[i].coin == coin_type &&
				relations->elems[i].debtor_id == id_self &&
				relations->elems[i].creditor_id == id_other) {
			if (amt > relations->elems[i].amt) {
				relations->elems[i].amt = 0.0;
				amt -= relations->elems[i].amt;
			} else {
				relations->elems[i].amt -= amt;
				amt = 0.0;
			}
		}
	}
}

void debug_print_relations(FENLLWorld *world) {
	int i;
	printf("relations:\n");
	for (i = 0; i < world->relations.size; i++) {
		printf("debtor: %d\tcreditor: %d\tamt: %f\t",
				world->relations.elems[i].debtor_id,
				world->relations.elems[i].creditor_id,
				world->relations.elems[i].amt);
		printf("interest: %f\tcoin: %d",
				world->relations.elems[i].interest,
				world->relations.elems[i].coin);
	}
}

/***********************
 * Cooldowns ***********
 * *********************
 */

static void reduce_cooldowns(DDArrFENLLCooldown *cooldowns) {
	int i = 0;
	while (i < cooldowns->size) {
		if (cooldowns->elems[i].turns_left <= 1) {
			DD_REMOVE_ARRAY(cooldowns, i);
		} else {
			cooldowns->elems[i].turns_left--;
			i++;
		}
	}
}

static bool id_in_cooldowns(DDArrFENLLCooldown *cooldowns, int id) {
	int i;
	for (i = 0; i < cooldowns->size; i++)
		if (cooldowns->elems[i].id == id)
			return true;
	return false;
}

static void add_cooldown(DDArrFENLLCooldown *cooldowns, int turns,
		int id) {
	FENLLCooldown cooldown;
	cooldown.id = id;
	cooldown.turns_left = turns;
	DD_ADD_ARRAY(cooldowns, cooldown);
}


/***********************
 * Turn Log ************
 * *********************
 */

static void create_turn_log_pocket(FENLLTurnLogPocket *p,
		FENLLPocket in_pocket, FENLLPocket outstanding) {
	p->in_pocket = in_pocket;
	p->outstanding = outstanding;
}

static void init_turn_log_lander_sees(FENLLTurnLogLanderSees *s) {
	init_dd_string(&s->seen_name);
	DD_INIT_ARRAY(&s->relations);
}

static void create_turn_log_lander_sees(FENLLTurnLogLanderSees *s,
		int seen_id, DDString *seen_name, FENLLPoint *seen_point,
		DDArrFENLLRelation *relations, int seer_id) {
	int i;
	s->seen_id = seen_id;
	dd_copy_dd_string(&s->seen_name, seen_name);
	s->seen_loc = *seen_point;
	for (i = 0; i < relations->size; i++) {
		DD_ADD_ARRAY(&s->relations, relations->elems[i]);
	}
	sum_relations(&s->outstanding, &s->relations, seer_id);
}

static void destroy_turn_log_lander_sees(FENLLTurnLogLanderSees *s) {
	free_dd_chars(&s->seen_name);
	DD_FREE_ARRAY(&s->relations);
}

static void init_turn_log_lander_has_goal(FENLLTurnLogLanderHasGoal *g) {
	init_dd_string(&g->goal_name);
	DD_INIT_ARRAY(&g->relations);
}

static void create_turn_log_lander_has_goal(FENLLTurnLogLanderHasGoal *g,
		int goal_id, DDString *goal_name, FENLLPoint *goal_loc,
		FENLLGoalType goal_type, DDArrFENLLRelation *relations,
		int lander_id) {
	int i;
	g->goal_id = goal_id;
	dd_copy_dd_string(&g->goal_name, goal_name);
	g->goal_loc = *goal_loc;
	g->goal_type = goal_type;
	for (i = 0; i < relations->size; i++) {
		DD_ADD_ARRAY(&g->relations, relations->elems[i]);
	}
	sum_relations(&g->outstanding, &g->relations, lander_id);
}

static void destroy_turn_log_lander_has_goal(
		FENLLTurnLogLanderHasGoal *g) {
	free_dd_chars(&g->goal_name);
	DD_FREE_ARRAY(&g->relations);
}

static void init_turn_log_lander_moves(FENLLTurnLogLanderMoves *m) {
	DD_INIT_ARRAY(&m->wanted_dirs);
}

static void create_turn_log_lander_moves(FENLLTurnLogLanderMoves *m,
		FENLLPoint start_loc, FENLLPoint end_loc,
		DDArrFENLLDirection *wanted_dirs, FENLLDirection actual_dir) {
	int i;
	m->start_loc = start_loc;
	m->end_loc = end_loc;
	for (i = 0; i < wanted_dirs->size; i++) {
		DD_ADD_ARRAY(&m->wanted_dirs, wanted_dirs->elems[i]);
	}
	m->actual_dir = actual_dir;
}

static void destroy_turn_log_lander_moves(FENLLTurnLogLanderMoves *m) {
	DD_FREE_ARRAY(&m->wanted_dirs);
}

static void init_turn_log_attempts_recoup(FENLLTurnLogAttemptsRecoup *r) {
	init_dd_string(&r->creditor_name);
	init_dd_string(&r->debtor_name);
}

static void create_turn_log_attempts_recoup(
		FENLLTurnLogAttemptsRecoup *r, int creditor_id, int debtor_id,
		FENLLPoint creditor_loc, FENLLPoint debtor_loc,
		DDString *creditor_name, DDString *debtor_name) {
	r->creditor_id = creditor_id;
	r->debtor_id = debtor_id;
	r->creditor_loc = creditor_loc;
	r->debtor_loc = debtor_loc;
	dd_copy_dd_string(&r->creditor_name, creditor_name);
	dd_copy_dd_string(&r->debtor_name, debtor_name);
}

static void destroy_turn_log_attempts_recoup(
		FENLLTurnLogAttemptsRecoup *r) {
	free_dd_chars(&r->creditor_name);
	free_dd_chars(&r->debtor_name);
}

static void init_turn_log_full_repayment(FENLLTurnLogFullRepayment *f) {
	init_dd_string(&f->creditor_name);
	init_dd_string(&f->debtor_name);
}

static void create_turn_log_full_repayment(
		FENLLTurnLogFullRepayment *f, int creditor_id, int debtor_id,
		double amt, FENLLCoin coin_type, DDString *creditor_name, 
		DDString *debtor_name) {
	f->creditor_id = creditor_id;
	f->debtor_id = debtor_id;
	f->amt = amt;
	f->coin_type = coin_type;
	dd_copy_dd_string(&f->debtor_name, debtor_name);
	dd_copy_dd_string(&f->creditor_name, creditor_name);
}

static void destroy_turn_log_full_repayment(
		FENLLTurnLogFullRepayment *f) {
	free_dd_chars(&f->creditor_name);
	free_dd_chars(&f->debtor_name);
}

static void init_turn_log_sells_debt(
	FENLLTurnLogSellsDebt *d) {
	init_dd_string(&d->old_creditor_name);
	init_dd_string(&d->new_creditor_name);
	init_dd_string(&d->debtor_name);
}

static void create_turn_log_sells_debt( FENLLTurnLogSellsDebt *d,
		int new_creditor_id, int old_creditor_id, int debtor_id,
		DDString *new_creditor_name, DDString *old_creditor_name,
		DDString *debtor_name, double amt, FENLLCoin coin_type) {
	d->new_creditor_id = new_creditor_id;
	d->old_creditor_id = old_creditor_id;
	d->debtor_id = debtor_id;
	dd_copy_dd_string(&d->old_creditor_name, old_creditor_name);
	dd_copy_dd_string(&d->new_creditor_name, new_creditor_name);
	dd_copy_dd_string(&d->debtor_name, debtor_name);
	d->amt = amt;
	d->coin_type = coin_type;
}

static void destroy_turn_log_sells_debt(
	FENLLTurnLogSellsDebt *d) {
	free_dd_chars(&d->old_creditor_name);
	free_dd_chars(&d->new_creditor_name);
	free_dd_chars(&d->debtor_name);
}

static void init_turn_log_sets_up_plan(FENLLTurnLogSetsUpPlan *u) {
	init_dd_string(&u->creditor_name);
	init_dd_string(&u->debtor_name);
}

static void create_turn_log_sets_up_plan(FENLLTurnLogSetsUpPlan *u,
		int creditor_id, int debtor_id, DDString *creditor_name,
		DDString *debtor_name, double total, double monthly,
		FENLLCoin coin_type) {
	u->creditor_id = creditor_id;
	u->debtor_id = debtor_id;
	dd_copy_dd_string(&u->creditor_name, creditor_name);
	dd_copy_dd_string(&u->debtor_name, debtor_name);
	u->total = total;
	u->monthly = monthly;
	u->coin_type = coin_type;
}

static void destroy_turn_log_sets_up_plan(FENLLTurnLogSetsUpPlan *u) {
	free_dd_chars(&u->creditor_name);
	free_dd_chars(&u->debtor_name);
}

static void init_turn_log_misses_payment(FENLLTurnLogMissesPayment *i) {
	init_dd_string(&i->creditor_name);
	init_dd_string(&i->debtor_name);
}

static void create_turn_log_misses_payment(FENLLTurnLogMissesPayment *i,
		int creditor_id, int debtor_id, DDString *creditor_name,
		DDString *debtor_name, double amt, FENLLCoin coin_type) {
	i->creditor_id = creditor_id;
	i->debtor_id = debtor_id;
	dd_copy_dd_string(&i->creditor_name, creditor_name);
	dd_copy_dd_string(&i->debtor_name, debtor_name);
	i->amt = amt;
	i->coin_type = coin_type;
}

static void destroy_turn_log_misses_payment(FENLLTurnLogMissesPayment *i) {
	free_dd_chars(&i->creditor_name);
	free_dd_chars(&i->debtor_name);
}

static void init_turn_log_makes_payment(FENLLTurnLogMakesPayment *a) {
	init_dd_string(&a->creditor_name);
	init_dd_string(&a->debtor_name);
}

static void create_turn_log_makes_payment(FENLLTurnLogMakesPayment *a,
		int creditor_id, int debtor_id, DDString *creditor_name,
		DDString *debtor_name, double amt, FENLLCoin coin_type) {
	a->creditor_id = creditor_id;
	a->debtor_id = debtor_id;
	dd_copy_dd_string(&a->creditor_name, creditor_name);
	dd_copy_dd_string(&a->debtor_name, debtor_name);
	a->amt = amt;
	a->coin_type = coin_type;
}

static void destroy_turn_log_makes_payment(FENLLTurnLogMakesPayment *a) {
	free_dd_chars(&a->creditor_name);
	free_dd_chars(&a->debtor_name);
}

static void empty_turn_log(FENLLander *lander) {
	int i;
	for (i = 0; i < lander->turn_log.size; i++) {
		switch (lander->turn_log.elems[i].type) {
			case FE_NLL_TURN_LOG_ITEM_TYPE_POCKET:
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_SEES:
				destroy_turn_log_lander_sees(
						&lander->turn_log.elems[i].value.s);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_HAS_GOAL:
				destroy_turn_log_lander_has_goal(
						&lander->turn_log.elems[i].value.g);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_MOVES:
				destroy_turn_log_lander_moves(
						&lander->turn_log.elems[i].value.m);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_ATTEMPTS_RECOUP:
				destroy_turn_log_attempts_recoup(
						&lander->turn_log.elems[i].value.r);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_FULL_REPAYMENT:
				destroy_turn_log_full_repayment(
						&lander->turn_log.elems[i].value.f);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_SELLS_DEBT:
				destroy_turn_log_sells_debt(
						&lander->turn_log.elems[i].value.d);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_SETS_UP_PLAN:
				destroy_turn_log_sets_up_plan(
						&lander->turn_log.elems[i].value.u);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_MISSES_PAYMENT:
				destroy_turn_log_misses_payment(
						&lander->turn_log.elems[i].value.i);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_MAKES_PAYMENT:
				destroy_turn_log_makes_payment(
						&lander->turn_log.elems[i].value.a);
				break;
		}
	}
	lander->turn_log.size = 0;
}

static void add_turn_log_pocket(FENLLander *lander, FENLLWorld *world) {
	FENLLTurnLogPocket item_val;
	FENLLTurnLogItem item;
	DDArrFENLLRelation lander_relations;
	FENLLPocket outstanding;

	DD_INIT_ARRAY(&lander_relations);
	get_relations(&lander_relations, &world->relations, lander->id);
	sum_relations(&outstanding, &lander_relations, lander->id);
	create_turn_log_pocket(&item_val, lander->pocket, outstanding);

	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_POCKET;
	item.value.p = item_val;

	DD_ADD_ARRAY(&lander->turn_log, item);
	DD_FREE_ARRAY(&lander_relations);
}

static void add_turn_log_lander_sees(FENLLander *seer,
		FENLLWorld *world, FENLLander *seen) {
	FENLLTurnLogLanderSees item_val;
	FENLLTurnLogItem item;
	DDArrFENLLRelation relations;

	init_turn_log_lander_sees(&item_val);
	DD_INIT_ARRAY(&relations);

	get_joint_relations(&relations, &world->relations, seen->id,
			seer->id);
	create_turn_log_lander_sees(&item_val, seen->id, &seen->name,
			&seen->loc, &relations, seer->id);

	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_SEES;
	item.value.s = item_val;

	DD_ADD_ARRAY(&seer->turn_log, item);
	DD_FREE_ARRAY(&relations);
}

static void add_turn_log_lander_has_goal(FENLLander *goal_haver,
		FENLLWorld *world, FENLLander *goal_obj) {
	FENLLTurnLogLanderHasGoal item_val;
	FENLLTurnLogItem item;
	DDArrFENLLRelation relations;

	init_turn_log_lander_has_goal(&item_val);
	DD_INIT_ARRAY(&relations);
	get_joint_relations(&relations, &world->relations, goal_haver->id,
			goal_obj->id);
	create_turn_log_lander_has_goal(&item_val, goal_obj->id,
			&goal_obj->name, &goal_obj->loc, goal_haver->goal.goal_type,
			&relations, goal_haver->id);

	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_HAS_GOAL;
	item.value.g = item_val;

	DD_ADD_ARRAY(&goal_haver->turn_log, item);
	DD_FREE_ARRAY(&relations);
}

static void add_turn_log_has_goal(FENLLander *lander, FENLLWorld *world) {
	if (lander->goal.goal_type != FE_NLL_GOAL_TYPE_NULL) {
		add_turn_log_lander_has_goal(lander, world,
				lander->goal.lander);
	}
}

static void add_turn_log_lander_moves(FENLLander *lander, FENLLPoint last,
		FENLLPoint next, DDArrFENLLDirection *wanted_dirs,
		FENLLDirection chosen_dir) {
	FENLLTurnLogLanderMoves item_val;
	FENLLTurnLogItem item;

	init_turn_log_lander_moves(&item_val);
	create_turn_log_lander_moves(&item_val, last, next, wanted_dirs,
			chosen_dir);

	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_MOVES;
	item.value.m = item_val;

	DD_ADD_ARRAY(&lander->turn_log, item);
}

static void add_turn_log_attempts_recoup(FENLLander *lander) {
	FENLLTurnLogAttemptsRecoup item_val;
	FENLLTurnLogItem item;
	FENLLTurnLogAttemptsRecoup item_val_2;
	FENLLTurnLogItem item_2;

	init_turn_log_attempts_recoup(&item_val);
	create_turn_log_attempts_recoup(&item_val, lander->id,
			lander->goal.lander->id, lander->loc,
			lander->goal.lander->loc, &lander->name,
			&lander->goal.lander->name);
	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_ATTEMPTS_RECOUP;
	item.value.r = item_val;

	init_turn_log_attempts_recoup(&item_val_2);
	create_turn_log_attempts_recoup(&item_val_2, lander->id,
			lander->goal.lander->id, lander->loc,
			lander->goal.lander->loc, &lander->name,
			&lander->goal.lander->name);
	item_2.type = FE_NLL_TURN_LOG_ITEM_TYPE_ATTEMPTS_RECOUP;
	item_2.value.r = item_val_2;

	DD_ADD_ARRAY(&lander->goal.lander->turn_log, item);
	DD_ADD_ARRAY(&lander->turn_log, item_2);
}

static void add_turn_log_full_repayment(FENLLander *lander, double amt,
		FENLLCoin coin_type) {
	FENLLTurnLogFullRepayment item_val;
	FENLLTurnLogItem item;
	FENLLTurnLogFullRepayment item_val_2;
	FENLLTurnLogItem item_2;

	init_turn_log_full_repayment(&item_val);
	create_turn_log_full_repayment(&item_val, lander->id,
			lander->goal.lander->id, amt, coin_type,
			&lander->name, &lander->goal.lander->name);
	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_FULL_REPAYMENT;
	item.value.f = item_val;

	init_turn_log_full_repayment(&item_val_2);
	create_turn_log_full_repayment(&item_val_2, lander->id,
			lander->goal.lander->id, amt, coin_type,
			&lander->name, &lander->goal.lander->name);
	item_2.type = FE_NLL_TURN_LOG_ITEM_TYPE_FULL_REPAYMENT;
	item_2.value.f = item_val_2;

	DD_ADD_ARRAY(&lander->turn_log, item);
	DD_ADD_ARRAY(&lander->goal.lander->turn_log, item_2);
}

static void add_turn_log_sells_debt(FENLLander *lander,
		FENLLander *debtor, FENLLRelation *rel) {
	FENLLTurnLogSellsDebt item_val;
	FENLLTurnLogItem item;
	FENLLTurnLogSellsDebt item_val_2;
	FENLLTurnLogItem item_2;

	init_turn_log_sells_debt(&item_val);
	create_turn_log_sells_debt(&item_val, lander->id,
			lander->goal.lander->id, debtor->id, &lander->name,
			&lander->goal.lander->name, &debtor->name, rel->amt,
			rel->coin);
	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_SELLS_DEBT;
	item.value.d = item_val;

	init_turn_log_sells_debt(&item_val_2);
	create_turn_log_sells_debt(&item_val_2, lander->id,
			lander->goal.lander->id, debtor->id, &lander->name,
			&lander->goal.lander->name, &debtor->name, rel->amt,
			rel->coin);
	item_2.type = FE_NLL_TURN_LOG_ITEM_TYPE_SELLS_DEBT;
	item_2.value.d = item_val_2;
	DD_ADD_ARRAY(&lander->turn_log, item);
	DD_ADD_ARRAY(&debtor->turn_log, item_2);
}

static void add_turn_log_sets_up_plan(FENLLander *creditor,
		FENLLander *debtor, FENLLPaymentPlan plan) {
	FENLLTurnLogSetsUpPlan item_val;
	FENLLTurnLogItem item;
	FENLLTurnLogSetsUpPlan item_val_2;
	FENLLTurnLogItem item_2;

	init_turn_log_sets_up_plan(&item_val);
	create_turn_log_sets_up_plan(&item_val, creditor->id,
			debtor->id, &creditor->name, &debtor->name,
			plan.total, plan.monthly, plan.coin_type);
	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_SETS_UP_PLAN;
	item.value.u = item_val;

	init_turn_log_sets_up_plan(&item_val_2);
	create_turn_log_sets_up_plan(&item_val_2, creditor->id,
			debtor->id, &creditor->name, &debtor->name,
			plan.total, plan.monthly, plan.coin_type);
	item_2.type = FE_NLL_TURN_LOG_ITEM_TYPE_SETS_UP_PLAN;
	item_2.value.u = item_val_2;

	DD_ADD_ARRAY(&creditor->turn_log, item);
	DD_ADD_ARRAY(&debtor->turn_log, item_2);
}

static void add_turn_log_misses_payment(FENLLander *creditor,
		FENLLander *debtor, FENLLPaymentPlan *plan) {
	FENLLTurnLogMissesPayment item_val;
	FENLLTurnLogItem item;
	FENLLTurnLogMissesPayment item_val_2;
	FENLLTurnLogItem item_2;

	init_turn_log_misses_payment(&item_val);
	create_turn_log_misses_payment(&item_val, creditor->id, debtor->id, 
			&creditor->name, &debtor->name, plan->monthly,
			plan->coin_type);
	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_MISSES_PAYMENT;
	item.value.i = item_val;

	init_turn_log_misses_payment(&item_val_2);
	create_turn_log_misses_payment(&item_val_2, creditor->id, debtor->id, 
			&creditor->name, &debtor->name, plan->monthly,
			plan->coin_type);
	item_2.type = FE_NLL_TURN_LOG_ITEM_TYPE_MISSES_PAYMENT;
	item_2.value.i = item_val_2;
	DD_ADD_ARRAY(&creditor->turn_log, item);
	DD_ADD_ARRAY(&debtor->turn_log, item_2);
}

static void add_turn_log_makes_payment(FENLLander *creditor,
		FENLLander *debtor, FENLLPaymentPlan *plan) {
	FENLLTurnLogMakesPayment item_val;
	FENLLTurnLogItem item;
	FENLLTurnLogMakesPayment item_val_2;
	FENLLTurnLogItem item_2;

	init_turn_log_makes_payment(&item_val);
	create_turn_log_makes_payment(&item_val, creditor->id, debtor->id, 
			&creditor->name, &debtor->name, plan->monthly,
			plan->coin_type);
	item.type = FE_NLL_TURN_LOG_ITEM_TYPE_MAKES_PAYMENT;
	item.value.a = item_val;
	init_turn_log_makes_payment(&item_val_2);
	create_turn_log_makes_payment(&item_val_2, creditor->id, debtor->id, 
			&creditor->name, &debtor->name, plan->monthly,
			plan->coin_type);
	item_2.type = FE_NLL_TURN_LOG_ITEM_TYPE_MAKES_PAYMENT;
	item_2.value.a = item_val_2;
	DD_ADD_ARRAY(&creditor->turn_log, item);
	DD_ADD_ARRAY(&debtor->turn_log, item_2);
}

static void print_pocket(FENLLTurnLogPocket *p) {
	printf("pocket at start:\n");
	printf("fuse: %f, frux: %f\n", p->in_pocket.fuse_coin,
			p->in_pocket.frux_coin);
	printf("outstanding:\n");
	printf("fuse: %f, frux: %f\n", p->outstanding.fuse_coin,
			p->outstanding.frux_coin);
}

static void print_lander_sees(FENLLTurnLogLanderSees *s,
		FENLLander *lander) {
	FENLLPocket summed;
	sum_relations(&summed, &s->relations, lander->id);
	printf("lander sees:\n");
	printf("id: %d\tname:%s\n", s->seen_id, s->seen_name.chars);
	printf("at: (%d,%d)\n", s->seen_loc.x, s->seen_loc.y);
	printf("relationship: fuse: %f, frux: %f\n", summed.fuse_coin,
			summed.frux_coin);
}

static void print_lander_has_goal(FENLLTurnLogLanderHasGoal *g,
		FENLLander *lander) {
	DDString str;
	FENLLPocket summed;
	init_dd_string(&str);
	switch (g->goal_type) {
		case FE_NLL_GOAL_TYPE_NULL:
			give_to_dd_string(&str, "none", 4);
			break;
		case FE_NLL_GOAL_TYPE_FLEE:
			give_to_dd_string(&str, "flee", 4);
			break;
		case FE_NLL_GOAL_TYPE_CHARGE:
			give_to_dd_string(&str, "charge", 6);
			break;
	}
	sum_relations(&summed, &g->relations, lander->id);

	printf("lander has goal: %s\n", str.chars);
	free_dd_chars(&str);
	printf("id: %d\tname:%s\n", g->goal_id, g->goal_name.chars);
	printf("at: (%d,%d)\n", g->goal_loc.x, g->goal_loc.y);
	printf("relationship: fuse: %f, frux: %f\n", summed.fuse_coin,
			summed.frux_coin);
}

static void print_lander_moves(FENLLTurnLogLanderMoves *m) {
	int i;
	printf("lander moved from (%d,%d)", m->start_loc.x, m->start_loc.y);
	printf(" to (%d,%d)\n", m->end_loc.x, m->end_loc.y);
	printf("wanted dirs: ");
	for (i = 0; i < m->wanted_dirs.size; i++) {
		printf("%d ", m->wanted_dirs.elems[i]);
	}
	printf("\n");
	printf("chosen dir: %d\n", m->actual_dir);
}

static void print_attempts_recoup(FENLLTurnLogAttemptsRecoup *r) {
	printf("%s (id: %d) ", r->creditor_name.chars, r->creditor_id);
	printf("at (%d,%d)", r->creditor_loc.x, r->creditor_loc.y);
	printf(" tries to recoup debt from %s ", r->debtor_name.chars);
	printf("(id: %d) at (%d,%d)\n", r->debtor_id,
			r->debtor_loc.x, r->debtor_loc.y);
}

static void print_full_repayment(FENLLTurnLogFullRepayment *f) {
	printf("%s (id: %d)", f->creditor_name.chars, f->creditor_id);
	printf(" fully collects %f", f->amt);
	if (f->coin_type == FE_NLL_COIN_FUSE) {
		printf(" fuse ");
	} else {
		printf(" frux ");
	}
	printf("from %s (id: %d)\n", f->debtor_name.chars, f->debtor_id);
}

static void print_sells_debt(FENLLTurnLogSellsDebt *d) {
	printf("%s (id: %d)", d->old_creditor_name.chars, d->old_creditor_id);
	printf(" sells debt to %s (id: %d) ", d->new_creditor_name.chars,
			d->new_creditor_id);
	printf("owed by %s (id: %d)", d->debtor_name.chars, d->debtor_id);
	printf(" for the amount of %f ", d->amt);
	if (d->coin_type == FE_NLL_COIN_FUSE) {
		printf("fuse\n");
	} else {
		printf("frux\n");
	}
}

static void print_sets_up_plan(FENLLTurnLogSetsUpPlan *u) {
	printf("%s (id: %d)", u->creditor_name.chars, u->creditor_id);
	printf(" sets up a payment plan with ");
	printf("%s (id: %d)", u->debtor_name.chars, u->debtor_id);
	printf("for a total of %f, at a monthly rate of %f ", u->total,
			u->monthly);
	if (u->coin_type == FE_NLL_COIN_FUSE) {
		printf("fuse\n");
	} else {
		printf("frux\n");
	}
}

static void print_misses_payment(FENLLTurnLogMissesPayment *i) {
	printf("%s (id: %d)", i->debtor_name.chars, i->debtor_id);
	printf(" misses a payment of %f ", i->amt);
	if (i->coin_type == FE_NLL_COIN_FUSE) {
		printf("fuse");
	} else {
		printf("frux");
	}
	printf(" owed to %s (id: %d)\n", i->creditor_name.chars,
			i->creditor_id);
	
}

static void print_makes_payment(FENLLTurnLogMakesPayment *a) {
	printf("%s (id: %d)", a->debtor_name.chars, a->debtor_id);
	printf(" misses a payment of %f ", a->amt);
	if (a->coin_type == FE_NLL_COIN_FUSE) {
		printf("fuse");
	} else {
		printf("frux");
	}
	printf(" owed to %s (id: %d)\n", a->creditor_name.chars,
			a->creditor_id);
}

void print_turn_log(FENLLander *lander) {
	int i;
	for (i = 0; i < lander->turn_log.size; i++) {
		switch (lander->turn_log.elems[i].type) {
			case FE_NLL_TURN_LOG_ITEM_TYPE_POCKET:
				print_pocket(&lander->turn_log.elems[i].value.p);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_SEES:
				print_lander_sees(&lander->turn_log.elems[i].value.s,
						lander);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_HAS_GOAL:
				print_lander_has_goal(&lander->turn_log.elems[i].value.g,
						lander);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_MOVES:
				print_lander_moves(&lander->turn_log.elems[i].value.m);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_ATTEMPTS_RECOUP:
				print_attempts_recoup(&lander->turn_log.elems[i].value.r);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_FULL_REPAYMENT:
				print_full_repayment(&lander->turn_log.elems[i].value.f);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_SELLS_DEBT:
				print_sells_debt(&lander->turn_log.elems[i].value.d);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_SETS_UP_PLAN:
				print_sets_up_plan(&lander->turn_log.elems[i].value.u);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_MISSES_PAYMENT:
				print_misses_payment(&lander->turn_log.elems[i].value.i);
				break;
			case FE_NLL_TURN_LOG_ITEM_TYPE_MAKES_PAYMENT:
				print_makes_payment(&lander->turn_log.elems[i].value.a);
				break;
		}
	}
}

/***********************
 * Payment Plan ********
 * *********************
 */

static void create_payment_plan(FENLLWorld *world, FENLLander *lander,
		int creditor_id, double total, FENLLCoin coin_type) {
	FENLLander *creditor;
	FENLLPaymentPlan payment_plan;
	payment_plan.creditor_id = creditor_id;
	payment_plan.total = total;
	payment_plan.monthly = total / ((rand() % 24) + 24);
	payment_plan.interest = rand_double() * 0.4 + 0.11;
	payment_plan.coin_type = coin_type;
	DD_ADD_ARRAY(&lander->payment_plans, payment_plan);
	lander_by_id(&creditor, world, creditor_id);
	add_turn_log_sets_up_plan(creditor, lander, payment_plan);
}

static bool can_pay(FENLLander *lander, FENLLPaymentPlan *plan) {
	if (plan->coin_type == FE_NLL_COIN_FUSE) {
		return lander->pocket.fuse_coin >= plan->monthly;
	}
	return lander->pocket.frux_coin >= plan->monthly;
}

static void pay_plan(FENLLWorld *world, FENLLander *lander,
		FENLLPaymentPlan *plan) {
	FENLLander *creditor;
	lander_by_id(&creditor, world, plan->creditor_id);
	add_turn_log_makes_payment(creditor, lander, plan);
	if (plan->coin_type == FE_NLL_COIN_FUSE) {
		lander->pocket.fuse_coin -= plan->monthly;
	} else {
		lander->pocket.frux_coin -= plan->monthly;
	}
	plan->total -= plan->monthly;
}

static void handle_missed_payment(FENLLWorld *world, FENLLander *lander,
		FENLLPaymentPlan *plan) {
	FENLLander *creditor;
	lander_by_id(&creditor, world, plan->creditor_id);
	add_turn_log_misses_payment(creditor, lander, plan);
	add_new_relation(world, lander->id, plan->creditor_id,
			plan->monthly, plan->interest, plan->coin_type);
}

static void handle_payment_plan(FENLLWorld *world, FENLLander *lander,
		FENLLPaymentPlan *plan) {
	if (can_pay(lander, plan)) {
		pay_plan(world, lander, plan);
	} else {
		handle_missed_payment(world, lander, plan);
	}
}

static void handle_payment_plans(FENLLWorld *world, FENLLander *lander) {
	int i;
	for (i = 0; i < lander->payment_plans.size; i++) {
		if (lander->payment_plans.elems[i].total > 0.0) {
			handle_payment_plan(world, lander,
					&lander->payment_plans.elems[i]);
		}
	}
}

/***********************
 * Map Utils ***********
 * *********************
 */

static FENLLPoint shift_point_in_dir(FENLLPoint point,
		FENLLDirection dir) {
	switch (dir) {
		case FE_NLL_DIR_NORTH:
			point.y--;
			return point;
		case FE_NLL_DIR_EAST:
			point.x++;
			return point;
		case FE_NLL_DIR_SOUTH:
			point.y++;
			return point;
		case FE_NLL_DIR_WEST:
			point.x--;
			return point;
		case FE_NLL_DIR_NONE:
			return point;
	}
}

static int get_manhattan_distance(FENLLPoint *a, FENLLPoint *b) {
	return abs(a->x - b->x) + abs(a->y - b->y);
}

static FENLLPoint invert_point(FENLLPoint point) {
	point.x = -point.x;
	point.y = -point.y;
	return point;
}

static int point_to_tile_index(FENLLWorld *world, FENLLPoint pt) {
	return pt.y * world->width + pt.x;
}

static FENLLMapTile *map_tile_at(FENLLWorld *world, FENLLPoint pt) {
	int index = point_to_tile_index(world, pt);
	return &world->map.elems[index];
}

static bool point_on_map(FENLLWorld *world, FENLLPoint pt) {
	return pt.x >= 0 && pt.x < world->width && 
		pt.y >= 0 && pt.y < world->height;
}

static bool point_available(FENLLWorld *world, FENLLPoint *point) {
	int i;
	for (i = 0; i < world->populace.size; i++) {
		if (point->x == world->populace.elems[i].loc.x &&
				point->y == world->populace.elems[i].loc.y)
			return false;
	}
	return true;
}

static void init_tile(FENLLMapTile *tile) {
	tile->inhabitant = NULL;
}

// world->populace and world->height/width must be set.
static void set_map(FENLLWorld *world) {
	int i;
	FENLLMapTile *tile;

	DD_INIT_ARRAY_SIZE(&world->map, world->height * world->width);
	for (i = 0; i < world->map.size; i++)
		init_tile(&world->map.elems[i]);

	for (i = 0; i < world->populace.size; i++) {
		tile = map_tile_at(world, world->populace.elems[i].loc);
		tile->inhabitant = &world->populace.elems[i];
	}
}

void fe_nll_print_map(FENLLWorld *world) {
	int i,j;
	FENLLPoint point;
	FENLLMapTile *tile;
	for (i = 0; i < world->height; i++) {
		for (j = 0; j < world->width; j++) {
			point.x = j;
			point.y = i;
			tile = map_tile_at(world, point);
			if (tile->inhabitant) {
				if (tile->inhabitant->id == world->player_id) {
					printf("@");
				} else {
					printf("*");
				}
			} else {
				printf(".");
			}
		}
		printf("\n");
	}
}

static void stringify_dir(DDString *str, FENLLDirection dir) {
	switch (dir) {
		case FE_NLL_DIR_NORTH:
			give_to_dd_string(str, "North", 5);
			break;
		case FE_NLL_DIR_EAST:
			give_to_dd_string(str, "East", 4);
			break;
		case FE_NLL_DIR_SOUTH:
			give_to_dd_string(str, "South", 5);
			break;
		case FE_NLL_DIR_WEST:
			give_to_dd_string(str, "West", 4);
			break;
		case FE_NLL_DIR_NONE:
			give_to_dd_string(str, "None", 4);
			break;
	}
}

/***********************
 * Event Log ***********
 * *********************
 */

static void add_log_item(FENLLWorld *world, DDString *str) {
	FENLLogItem item;
	item.turn = world->turn;
	item.description = *str;
	DD_ADD_ARRAY(&world->event_log, item);
}

static void lander_sees_lander_description(DDString *target,
		FENLLander *seer, FENLLander *seen) {
	DDString tmp_str;
	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, " sees ", 6);
	dd_string_concat_mutate(target, &seer->name);
	dd_string_concat_mutate(target, &tmp_str);
	dd_string_concat_mutate(target, &seen->name);
	free_dd_chars(&tmp_str);
}

static void log_new_goal(FENLLWorld *world, FENLLander *lander) {
	DDString tmp_str;
	DDString target;
	init_dd_string(&tmp_str);
	init_dd_string(&target);

	dd_string_concat_mutate(&target, &lander->name);
	give_to_dd_string(&tmp_str, " has new goal: ", 15);
	dd_string_concat_mutate(&target, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);
	if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_FLEE) {
		give_to_dd_string(&tmp_str, "flee ", 5);
		dd_string_concat_mutate(&target, &tmp_str);
		dd_string_concat_mutate(&target, &lander->goal.lander->name);
	} else if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_CHARGE) {
		give_to_dd_string(&tmp_str, "charge ", 7);
		dd_string_concat_mutate(&target, &tmp_str);
		dd_string_concat_mutate(&target, &lander->goal.lander->name);
	} else {
		give_to_dd_string(&tmp_str, "none", 4);
		dd_string_concat_mutate(&target, &tmp_str);
	}
	add_log_item(world, &target);
	free_dd_chars(&tmp_str);
}

void fe_nll_print_event_log(FENLLWorld *world) {
	int i;
	for (i = 0; i < world->event_log.size; i++) {
		if (world->event_log.elems[i].turn == world->turn)
			printf("%s\n", world->event_log.elems[i].description.chars);
	}
}

static void free_log_item(FENLLogItem *log_item) {
	free_dd_chars(&log_item->description);
}

/***********************
 * Lander Mgmnt ********
 * *********************
 */

/* map isn't set yet so we have to iterate through landers
 * to determine whether a map tile is already occupied
 */
static void get_lander_start_loc(FENLLander *lander, 
		FENLLConfigureWorld *conf, FENLLWorld *world) {
	FENLLPoint point;
	lander->loc.x = -1;
	lander->loc.y = -1;
	while (lander->loc.x < 0 && lander->loc.y < 0) {
		point.y = rand() % conf->height;
		point.x = rand() % conf->width;
		if (point_available(world, &point)) {
			lander->loc = point;
		}
	}
}

static void get_lander_start_pocket(FENLLander *lander,
		FENLLConfigureWorld *conf) {
	double tmp;
	tmp = rand_double() * (conf->pocket_ceil - conf->pocket_floor)
		+ conf->pocket_floor;
	lander->pocket.fuse_coin = tmp;
	tmp = rand_double() * (conf->pocket_ceil - conf->pocket_floor)
		+ conf->pocket_floor;
	lander->pocket.frux_coin = tmp;
}

static void init_lander(FENLLander *lander) {
	init_dd_string(&lander->name);
}

static void create_lander(FENLLander *lander, int id, DDString *name,
	FENLLConfigureWorld *conf, FENLLWorld *world) {
	lander->id = id;
	dd_copy_dd_string(&lander->name, name);
	get_lander_start_loc(lander, conf, world);
	get_lander_start_pocket(lander, conf);
	lander->goal.goal_type = FE_NLL_GOAL_TYPE_NULL;
	lander->goal.lander = NULL;
	lander->goal.invisible_ctr = 0;
	lander->last_move = -1;
	lander->sight = 5;
	lander->alive = true;
	DD_INIT_ARRAY(&lander->turn_log);
	DD_INIT_ARRAY(&lander->cooldowns);
	DD_INIT_ARRAY(&lander->payment_plans);
}

static void free_lander(FENLLander *lander) {
	free_dd_chars(&lander->name);
	empty_turn_log(lander);
	DD_FREE_ARRAY(&lander->turn_log);
	DD_FREE_ARRAY(&lander->cooldowns);
	DD_FREE_ARRAY(&lander->payment_plans);
}

/***********************
 * Lander Look *********
 * *********************
 */

static void lander_look(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	int i, j, d, index;
	FENLLMapTile *tile;
	FENLLPoint pt;
	for (i = -lander->sight; i <= lander->sight; i++) {
		d = lander->sight - abs(i);
		for (j = -d; j <= d; j++) {
			pt.x = lander->loc.x + j;
			pt.y = lander->loc.y + i;
			if (point_on_map(world, pt)) {
				tile = map_tile_at(world, pt);
				if (tile->inhabitant) {
					index = point_to_tile_index(world, pt);
					DD_ADD_ARRAY(tile_indices, index);
				}
			}
		}
	}
}

static void lander_see(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	FENLLMapTile *tile;
	int i;
	DDString tmp_str;
	bool goal_seen;
	goal_seen = false;

	for (i = 0; i < tile_indices->size; i++) {
		tile = &world->map.elems[tile_indices->elems[i]];
		if (tile->inhabitant && tile->inhabitant != lander) {
			if (lander->goal.goal_type != FE_NLL_GOAL_TYPE_NULL &&
					tile->inhabitant == lander->goal.lander) {
				goal_seen = true;
			}
			init_dd_string(&tmp_str);
			lander_sees_lander_description(&tmp_str, lander,
					tile->inhabitant);
			add_log_item(world, &tmp_str);
			add_turn_log_lander_sees(lander, world, tile->inhabitant);
		}
	}

	if (lander->goal.goal_type != FE_NLL_GOAL_TYPE_NULL &&
		   goal_seen == false) {
		lander->goal.invisible_ctr++;
	}
}

static void lander_look_and_see(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	lander_look(tile_indices, world, lander);
	lander_see(tile_indices, world, lander);
}

/***********************
 * Lander Goal *********
 * *********************
 */

static void set_goal(DDArrFENLLRelation *relations, FENLLander *lander,
		FENLLander *tile_inhabitant) {
	double rel;
	FENLLPocket outstanding;
	sum_relations(&outstanding, relations, lander->id);
	rel = outstanding.fuse_coin + outstanding.frux_coin;

	if (rel < 0.0) {
		lander->goal.goal_type = FE_NLL_GOAL_TYPE_FLEE;
		lander->goal.lander = tile_inhabitant;
		lander->goal.invisible_ctr = 0;
	} else {
		lander->goal.goal_type = FE_NLL_GOAL_TYPE_CHARGE;
		lander->goal.lander = tile_inhabitant;
		lander->goal.invisible_ctr = 0;
	}
}

static void evaluate_one_tile(int tile_index, FENLLWorld *world,
		FENLLander *lander) {
	FENLLMapTile *tile;
	DDArrFENLLRelation relations;
	int dist_a;
	int dist_b;

	DD_INIT_ARRAY(&relations);

	tile = &world->map.elems[tile_index];

	if (tile->inhabitant != lander && 
			tile->inhabitant != lander->goal.lander &&
			!id_in_cooldowns(&lander->cooldowns, tile->inhabitant->id)) {
		get_joint_relations(&relations, &world->relations, lander->id,
				tile->inhabitant->id);

		if (relations.size) {
			if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_NULL) {
				set_goal(&relations, lander, tile->inhabitant);
				log_new_goal(world, lander);
			} else {
				dist_a = get_manhattan_distance(&lander->loc,
						&lander->goal.lander->loc);
				dist_b = get_manhattan_distance(&lander->loc,
						&tile->inhabitant->loc);
				if (dist_a / dist_b >= 1.5) {
					set_goal(&relations, lander, tile->inhabitant);
					log_new_goal(world, lander);
				}
			}
		}
	}
	DD_FREE_ARRAY(&relations);
}

static void lander_pick_goal(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	int i;
	if (lander->goal.invisible_ctr > 2) {
		lander->goal.goal_type = FE_NLL_GOAL_TYPE_NULL;
		lander->goal.lander = NULL;
		lander->goal.invisible_ctr = 0;
	}
	for (i = 0; i < tile_indices->size; i++) {
		evaluate_one_tile(tile_indices->elems[i], world, lander);
	}
	add_turn_log_has_goal(lander, world);
}

static void remove_goal(FENLLander *lander) {
	lander->goal.goal_type = FE_NLL_GOAL_TYPE_NULL;
	lander->goal.lander = NULL;
}

/***********************
 * Lander Encounter ****
 * *********************
 */

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

static void log_full_payment(FENLLWorld *world, FENLLCoin coin_type,
		double amt, FENLLander *creditor, FENLLander *debtor) {
	DDString log_str;
	DDString tmp_str;
	init_dd_string(&log_str);
	init_dd_string(&tmp_str);

	dd_string_concat_mutate(&log_str, &creditor->name);
	give_to_dd_string(&tmp_str, " collects ", 10);
	dd_string_concat_mutate(&log_str, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);
	print_coin(&tmp_str, amt);
	dd_string_concat_mutate(&log_str, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);
	if (coin_type == FE_NLL_COIN_FUSE) {
		give_to_dd_string(&tmp_str, " fuse coin from ", 16);
	} else {
		give_to_dd_string(&tmp_str, " frux coin from ", 16);
	}
	dd_string_concat_mutate(&log_str, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);
	dd_string_concat_mutate(&log_str, &debtor->name);

	add_log_item(world, &log_str);
}

static void log_debt_purchase(FENLLWorld *world, FENLLRelation *relation,
		FENLLCoin coin_type) {
	DDString log_str;
	DDString tmp_str;;
	init_dd_string(&log_str);
	init_dd_string(&tmp_str);
	int i;

	give_to_dd_string(&log_str, " via the transfer of ", 21);
	print_coin(&tmp_str, relation->amt);
	dd_string_concat_mutate(&log_str, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);

	if (coin_type == FE_NLL_COIN_FUSE) {
		give_to_dd_string(&tmp_str, " in fuse coin debt owed by ", 27);
	} else {
		give_to_dd_string(&tmp_str, " in frux coin debt owed by ", 27);
	}

	dd_string_concat_mutate(&log_str, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);

	for (i = 0; i < world->populace.size; i++) {
		if (world->populace.elems[i].id == relation->debtor_id) {
			dd_string_concat_mutate(&log_str,
					&world->populace.elems[i].name);
		}
	}

	add_log_item(world, &log_str);
}

static bool attempt_full_payment(FENLLWorld *world, FENLLander *lander) {
	FENLLPocket outstanding;

	get_outstanding(&outstanding, world, lander->id,
			lander->goal.lander->id);

	if (outstanding.fuse_coin > 0.0 &&
			outstanding.fuse_coin < lander->goal.lander->pocket.fuse_coin) {
		make_payment(&world->relations, outstanding.fuse_coin,
				FE_NLL_COIN_FUSE, lander->goal.lander->id, lander->id);
		log_full_payment(world, FE_NLL_COIN_FUSE, outstanding.fuse_coin,
				lander->goal.lander, lander);
		add_turn_log_full_repayment(lander, outstanding.fuse_coin,
				FE_NLL_COIN_FUSE);
		return true;
	} else if (outstanding.frux_coin > 0.0 &&
			outstanding.frux_coin < lander->goal.lander->pocket.frux_coin) {
		make_payment(&world->relations, outstanding.frux_coin,
				FE_NLL_COIN_FRUX, lander->goal.lander->id, lander->id);
		log_full_payment(world, FE_NLL_COIN_FRUX, outstanding.frux_coin,
				lander->goal.lander, lander);
		add_turn_log_full_repayment(lander, outstanding.frux_coin,
				FE_NLL_COIN_FRUX);
		return true;
	}
	return false;
}

static bool attempt_buy_debt(FENLLWorld *world, FENLLander *lander) {
	DDArrInt rel_indices;
	FENLLPocket outstanding;

	DD_INIT_ARRAY(&rel_indices);
	get_creditor_relations(&rel_indices, &world->relations,
		lander->goal.lander->id);
	if (rel_indices.size == 0) {
		return false;
	}
	get_outstanding(&outstanding, world, lander->id,
			lander->goal.lander->id);

	world->relations.elems[rel_indices.elems[0]].creditor_id =
		lander->id;
	if (outstanding.fuse_coin > 0.0) {
		make_payment(&world->relations,
				world->relations.elems[rel_indices.elems[0]].amt,
				FE_NLL_COIN_FUSE, lander->goal.lander->id, lander->id);
		log_full_payment(world, FE_NLL_COIN_FUSE,
				world->relations.elems[rel_indices.elems[0]].amt,
				lander->goal.lander, lander);
		log_debt_purchase(world, 
				&world->relations.elems[rel_indices.elems[0]],
				FE_NLL_COIN_FUSE);
		add_turn_log_sells_debt(lander, lander->goal.lander,
			&world->relations.elems[rel_indices.elems[0]]);

		DD_FREE_ARRAY(&rel_indices);
		return true;
	} else if (outstanding.frux_coin > 0.0) {
		make_payment(&world->relations,
				world->relations.elems[rel_indices.elems[0]].amt,
				FE_NLL_COIN_FRUX, lander->goal.lander->id, lander->id);
		log_full_payment(world, FE_NLL_COIN_FRUX,
				world->relations.elems[rel_indices.elems[0]].amt,
				lander->goal.lander, lander);
		log_debt_purchase(world, 
				&world->relations.elems[rel_indices.elems[0]],
				FE_NLL_COIN_FRUX);
		add_turn_log_sells_debt(lander, lander->goal.lander,
			&world->relations.elems[rel_indices.elems[0]]);
		DD_FREE_ARRAY(&rel_indices);
		return true;
	}
	DD_FREE_ARRAY(&rel_indices);
	return false;
}

static void setup_payment_plan(FENLLWorld *world, FENLLander *lander) {
	FENLLPocket outstanding;

	get_outstanding(&outstanding, world, lander->id,
			lander->goal.lander->id);
	if (outstanding.fuse_coin > 0.0) {
		create_payment_plan(world, lander->goal.lander, lander->id,
				outstanding.fuse_coin, FE_NLL_COIN_FUSE);
		make_payment(&world->relations, outstanding.fuse_coin,
				FE_NLL_COIN_FUSE, lander->goal.lander->id, lander->id);
	} else if (outstanding.frux_coin > 0.0) {
		create_payment_plan(world, lander->goal.lander, lander->id,
				outstanding.frux_coin, FE_NLL_COIN_FRUX);
		make_payment(&world->relations, outstanding.frux_coin,
				FE_NLL_COIN_FRUX, lander->goal.lander->id, lander->id);
	} 
}

static void lander_recoup(FENLLWorld *world, FENLLander *lander) {
	bool pay;
	add_turn_log_attempts_recoup(lander);
	pay = attempt_full_payment(world, lander);
	if (!pay) {
		pay = attempt_buy_debt(world, lander);
		if (!pay) {
			setup_payment_plan(world, lander);
		}
	}
	add_cooldown(&lander->cooldowns, 3, lander->goal.lander->id);
	remove_goal(lander);
}

/***********************
 * Lander Move *********
 * *********************
 */

static void move_lander(FENLLWorld *world, FENLLPoint point,
		FENLLander *lander) {
	FENLLMapTile *tile;
	tile = map_tile_at(world, lander->loc);
	tile->inhabitant = NULL;
	lander->loc = point;
	tile = map_tile_at(world, lander->loc);
	tile->inhabitant = lander;
}

static void find_available_dirs(DDArrFENLLDirection *available,
		FENLLWorld *world, FENLLander *lander) {
	FENLLMapTile *tile;
	FENLLPoint point;
	int i;

	for (i = FE_NLL_DIR_NORTH; i <= FE_NLL_DIR_WEST; i++) {
		point = shift_point_in_dir(lander->loc, i);
		if (point_on_map(world, point)) {
			tile = map_tile_at(world, point);
			if (!tile->inhabitant)
				DD_ADD_ARRAY(available, i);
		}
	}
}

static void rand_pick_two(DDArrFENLLDirection *dir_arr,
		FENLLDirection dir_1, FENLLDirection dir_2) {
	if ((rand() % 2) == 0) {
		DD_ADD_ARRAY(dir_arr, dir_1);
		DD_ADD_ARRAY(dir_arr, dir_2);
	} else {
		DD_ADD_ARRAY(dir_arr, dir_2);
		DD_ADD_ARRAY(dir_arr, dir_1);
	}
}

static void set_priority_of_dirs(DDArrFENLLDirection *priority_dirs,
		FENLLander *lander) {
	FENLLPoint goal_point;
	if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_CHARGE) {
		goal_point = lander->goal.lander->loc;
	} else {
		goal_point = invert_point(lander->goal.lander->loc);
	}

	if (goal_point.x == lander->loc.x) {
		if (goal_point.y > lander->loc.y) {
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_SOUTH);
			rand_pick_two(priority_dirs, FE_NLL_DIR_EAST, FE_NLL_DIR_WEST);
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_NORTH);
		} else {
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_NORTH);
			rand_pick_two(priority_dirs, FE_NLL_DIR_EAST, FE_NLL_DIR_WEST);
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_SOUTH);
		}
	} else if (goal_point.x > lander->loc.x) {
		if (goal_point.y == lander->loc.y) {
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_EAST);
			rand_pick_two(priority_dirs, FE_NLL_DIR_SOUTH, FE_NLL_DIR_NORTH);
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_WEST);
		} else if (goal_point.y > lander->loc.y) {
			rand_pick_two(priority_dirs, FE_NLL_DIR_SOUTH, FE_NLL_DIR_EAST);
			rand_pick_two(priority_dirs, FE_NLL_DIR_NORTH, FE_NLL_DIR_WEST);
		} else {
			rand_pick_two(priority_dirs, FE_NLL_DIR_NORTH, FE_NLL_DIR_EAST);
			rand_pick_two(priority_dirs, FE_NLL_DIR_WEST, FE_NLL_DIR_SOUTH);
		}
	} else {
		if (goal_point.y == lander->loc.y) {
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_WEST);
			rand_pick_two(priority_dirs, FE_NLL_DIR_SOUTH, FE_NLL_DIR_NORTH);
			DD_ADD_ARRAY(priority_dirs, FE_NLL_DIR_EAST);
		} else if (goal_point.y > lander->loc.y) {
			rand_pick_two(priority_dirs, FE_NLL_DIR_SOUTH, FE_NLL_DIR_WEST);
			rand_pick_two(priority_dirs, FE_NLL_DIR_NORTH, FE_NLL_DIR_EAST);
		} else {
			rand_pick_two(priority_dirs, FE_NLL_DIR_NORTH, FE_NLL_DIR_WEST);
			rand_pick_two(priority_dirs, FE_NLL_DIR_SOUTH, FE_NLL_DIR_EAST);
		}
	}
}

static bool dir_in_arr(FENLLDirection dir, DDArrFENLLDirection *arr) {
	int i;
	for (i = 0; i < arr->size; i++) {
		if (dir == arr->elems[i])
			return true;
	}
	return false;
}

static FENLLDirection pick_from_wanted(DDArrFENLLDirection *wanted_dirs,
		DDArrFENLLDirection *available_dirs) {
	int i;
	for (i = 0; i < wanted_dirs->size; i++) {
		if (dir_in_arr(wanted_dirs->elems[i], available_dirs)) {
			return wanted_dirs->elems[i];
		}
	}
	return available_dirs->elems[rand() % available_dirs->size];
}

static void lander_move(FENLLWorld *world, FENLLander *lander) {
	DDArrFENLLDirection available_dirs;
	DDArrFENLLDirection wanted_dirs;
	FENLLPoint last_pt;
	FENLLDirection chosen_dir;
	FENLLPoint next_pt;

	DD_INIT_ARRAY(&available_dirs);
	DD_INIT_ARRAY(&wanted_dirs);
	last_pt = lander->loc;

	find_available_dirs(&available_dirs, world, lander);
	
	if (available_dirs.size == 0) {
		chosen_dir = FE_NLL_DIR_NONE;
	} else if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_NULL) {
		chosen_dir = available_dirs.elems[rand() % available_dirs.size];
	} else {
		set_priority_of_dirs(&wanted_dirs, lander);
		chosen_dir = pick_from_wanted(&wanted_dirs, &available_dirs);
	}
	
	next_pt = shift_point_in_dir(lander->loc, chosen_dir);
	move_lander(world, next_pt, lander);
	add_turn_log_lander_moves(lander, last_pt, next_pt, &wanted_dirs,
		chosen_dir);

	DD_FREE_ARRAY(&available_dirs);
	DD_FREE_ARRAY(&wanted_dirs);
}

/***********************
 * Player Turn *********
 * *********************
 */

static FENLLDirection get_player_dir_selection(
		DDArrFENLLDirection *available_dirs) {
	int i, c, d;
	DDString dir_str;

	init_dd_string(&dir_str);
	
	printf("Select direction:\n");
	for (i = 0; i < available_dirs->size; i++) {
		stringify_dir(&dir_str, available_dirs->elems[i]);
		printf("%d) %s\n", available_dirs->elems[i]+1, dir_str.chars);
		free_dd_chars(&dir_str);
		init_dd_string(&dir_str);
	}

	while (1) {
		c = getchar();
		d = c;
		while (d != '\n') {
			d = getchar();
		}

		for (i = 0; i < available_dirs->size; i++) {
			if (c == available_dirs->elems[i]+49) {
				return available_dirs->elems[i];
			}
		}
	}
}

static void player_move(FENLLWorld *world, FENLLander *player) {
	FENLLDirection chosen_dir;
	FENLLPoint next_pt;
	DDArrFENLLDirection available_dirs;
	DD_INIT_ARRAY(&available_dirs);
	
	find_available_dirs(&available_dirs, world, player);

	if (available_dirs.size == 0) {
		printf("You are unable to move this turn.\n");
		getchar();
	}

	chosen_dir = get_player_dir_selection(&available_dirs);
	next_pt = shift_point_in_dir(player->loc, chosen_dir);

	move_lander(world, next_pt, player);
}

static void player_take_turn(FENLLWorld *world, FENLLander *player) {
	DDArrInt interesting_tile_indices;

	DD_INIT_ARRAY(&interesting_tile_indices);

	add_turn_log_pocket(player, world);

	lander_look_and_see(&interesting_tile_indices, world, player);

	fe_nll_print_map(world);
	print_turn_log(player);
	player_move(world, player);

	DD_FREE_ARRAY(&interesting_tile_indices);
}

/***********************
 * Tick ****************
 * *********************
 */

static bool can_try_collection(FENLLander *lander) {
	return lander->goal.goal_type == FE_NLL_GOAL_TYPE_CHARGE &&
		get_manhattan_distance(&lander->goal.lander->loc,
				&lander->loc) == 1;
}

static void lander_take_turn(FENLLWorld *world, FENLLander *lander) {
	DDArrInt interesting_tile_indices;
	DD_INIT_ARRAY(&interesting_tile_indices);


	add_turn_log_pocket(lander, world);
	handle_payment_plans(world, lander);

	lander_look_and_see(&interesting_tile_indices, world, lander);
	DD_SHUFFLE_ARRAY(int, &interesting_tile_indices);
	lander_pick_goal(&interesting_tile_indices, world, lander);

	if (can_try_collection(lander)) {
		lander_recoup(world, lander);
	} else {
		lander_move(world, lander);
	}

	reduce_cooldowns(&lander->cooldowns);
	DD_FREE_ARRAY(&interesting_tile_indices);
}

void fe_nll_init_world(FENLLWorld *world, FENLLConfigureWorld *conf) { 
	int i;
	DDString tmp_name;
	FENLLander lander;
	world->height = conf->height;
	world->width = conf->width;
	DD_INIT_ARRAY(&world->populace);
	DD_INIT_ARRAY(&world->relations);
	DD_INIT_ARRAY(&world->map);
	DD_INIT_ARRAY(&world->event_log);

	world->turn = 0;
	for (i = 0; i < conf->num_population; i++) {
		init_dd_string(&tmp_name);
		init_lander(&lander);
		dd_repeat_dd_string(&tmp_name, 
			&conf->names.elems[i%conf->names.size], 
			(i / conf->names.size) + 1);
		create_lander(&lander, i, &tmp_name, conf, world);
		DD_ADD_ARRAY(&world->populace, lander);

		free_dd_chars(&tmp_name);
	}
	set_relations(world, conf);
	set_map(world);
	world->mode = conf->mode;

	if (world->mode == FE_NLL_MODE_PLAY) {
		world->player_id = world->populace.elems[0].id;
	} else {
		world->player_id = -1;
	}
}

void fe_nll_tick(FENLLWorld *world) {
	int i, j, k;

	for (i = 0; i < world->populace.size; i++) {
		empty_turn_log(&world->populace.elems[i]);
	}

	for (i = 0; i < world->populace.size; i++) {
		j = rand() % 3 + 1;
		if (world->populace.elems[i].id == world->player_id) {
			for (k = 0; k < j; k++) {
				player_take_turn(world, &world->populace.elems[i]);
			}
		} else {
			for (k = 0; k < j; k++) {
				lander_take_turn(world, &world->populace.elems[i]);
			}
		}
	}

	if (world->mode == FE_NLL_MODE_WATCH) {
		fe_nll_print_map(world);
		fe_nll_print_event_log(world);
		getchar();
	}

	apply_interest(world);
	world->turn++;
}

void fe_nll_free_world(FENLLWorld *world) {
	int i;
	for (i = 0; i < world->populace.size; i++)
		free_lander(&world->populace.elems[i]);
	DD_FREE_ARRAY(&world->populace);
	DD_FREE_ARRAY(&world->relations);
	for (i = 0; i < world->event_log.size; i++)
		free_log_item(&world->event_log.elems[i]);
	DD_FREE_ARRAY(&world->event_log);
	DD_FREE_ARRAY(&world->map);
}
