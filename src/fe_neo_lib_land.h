#ifndef fe_neo_lib_land_h
#define fe_neo_lib_land_h

#include <stdbool.h>
#include "dd_data.h"

typedef struct {
	int x;
	int y;
} FENLLPoint;

DD_DEF_QUEUE(FENLLPoint, FENLLPoint);
DD_DEF_ARRAY(FENLLPoint, FENLLPoint);

typedef enum {
	FE_NLL_COIN_FUSE,
	FE_NLL_COIN_FRUX,
} FENLLCoin;

typedef struct {
	double fuse_coin;
	double frux_coin;
} FENLLPocket;

typedef enum {
	FE_NLL_DIR_NORTH,
	FE_NLL_DIR_EAST,
	FE_NLL_DIR_SOUTH,
	FE_NLL_DIR_WEST,
	FE_NLL_DIR_NONE,
} FENLLDirection;

DD_DEF_ARRAY(FENLLDirection, FENLLDirection);

typedef struct {
	int debtor_id;
	int creditor_id;
	double amt;
	double interest;
	FENLLCoin coin;
} FENLLRelation;

DD_DEF_ARRAY(FENLLRelation, FENLLRelation);

typedef enum {
	FE_NLL_GOAL_TYPE_NULL,
	FE_NLL_GOAL_TYPE_FLEE,
	FE_NLL_GOAL_TYPE_CHARGE,
} FENLLGoalType;

typedef struct {
	FENLLPocket in_pocket;
	FENLLPocket outstanding;
} FENLLTurnLogPocket;

typedef struct {
	int seen_id;
	DDString seen_name;
	FENLLPoint seen_loc;
	DDArrFENLLRelation relations;
	FENLLPocket outstanding;
} FENLLTurnLogLanderSees;

typedef struct {
	int goal_id;
	DDString goal_name;
	FENLLPoint goal_loc;
	FENLLGoalType goal_type;
	DDArrFENLLRelation relations;
	FENLLPocket outstanding;
} FENLLTurnLogLanderHasGoal;

typedef struct {
	FENLLPoint start_loc;
	FENLLPoint end_loc;
	DDArrFENLLDirection wanted_dirs;
	FENLLDirection actual_dir;
} FENLLTurnLogLanderMoves;

typedef struct {
	int creditor_id;
	int debtor_id;
	FENLLPoint creditor_loc;
	FENLLPoint debtor_loc;
	DDString creditor_name;
	DDString debtor_name;
} FENLLTurnLogAttemptsRecoup;

typedef struct {
	int creditor_id;
	int debtor_id;
	DDString creditor_name;
	DDString debtor_name;
	double amt;
	FENLLCoin coin_type;
} FENLLTurnLogFullRepayment;

typedef struct {
	int new_creditor_id;
	int old_creditor_id;
	int debtor_id;
	DDString new_creditor_name;
	DDString old_creditor_name;
	DDString debtor_name;
	double amt;
	FENLLCoin coin_type;
} FENLLTurnLogSellsDebt;

typedef struct {
	int creditor_id;
	int debtor_id;
	DDString creditor_name;
	DDString debtor_name;
	double total;
	double monthly;
	FENLLCoin coin_type;
} FENLLTurnLogSetsUpPlan;

/* additional turn log item types:
 * attempts recoupment;
 * full payment;
 * sells debt;
 * sets up payment plan;
 * makes payment;
 * misses payment;
 */

typedef enum {
	FE_NLL_TURN_LOG_ITEM_TYPE_POCKET,
	FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_SEES,
	FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_HAS_GOAL,
	FE_NLL_TURN_LOG_ITEM_TYPE_LANDER_MOVES,
	FE_NLL_TURN_LOG_ITEM_TYPE_ATTEMPTS_RECOUP,
	FE_NLL_TURN_LOG_ITEM_TYPE_FULL_REPAYMENT,
	FE_NLL_TURN_LOG_ITEM_TYPE_SELLS_DEBT,
	FE_NLL_TURN_LOG_ITEM_TYPE_SETS_UP_PLAN,
} FENLLTurnLogItemType;

typedef union {
	FENLLTurnLogPocket p;
	FENLLTurnLogLanderSees s;
	FENLLTurnLogLanderHasGoal g;
	FENLLTurnLogLanderMoves m;
	FENLLTurnLogAttemptsRecoup r;
	FENLLTurnLogFullRepayment f;
	FENLLTurnLogSellsDebt d;
	FENLLTurnLogSetsUpPlan u;
} FENLLTurnLogItemValue;

typedef struct {
	FENLLTurnLogItemType type;
	FENLLTurnLogItemValue value;
} FENLLTurnLogItem;

DD_DEF_ARRAY(FENLLTurnLogItem, FENLLTurnLogItem);

typedef struct FENLLGoalCooldown {
	int id;
	int turns_left;
} FENLLCooldown;

DD_DEF_ARRAY(FENLLCooldown, FENLLCooldown);

typedef struct FENLLPaymentPlan {
	int creditor_id;
	double total;
	double monthly;
	double interest;
	FENLLCoin coin_type;
} FENLLPaymentPlan;

DD_DEF_ARRAY(FENLLPaymentPlan, FENLLPaymentPlan);

typedef struct FENLLander {
	int id;
	DDString name;
	FENLLPoint loc;
	FENLLPocket pocket;
	struct {
		FENLLGoalType goal_type;
		struct FENLLander *lander;
		int invisible_ctr;
	} goal;
	int last_move;
	int sight;
	bool alive;
	DDArrFENLLTurnLogItem turn_log;
	DDArrFENLLCooldown cooldowns;
	DDArrFENLLPaymentPlan payment_plans;
} FENLLander;

DD_DEF_ARRAY(FENLLander, FENLLander);

typedef struct{
	FENLLander *inhabitant;
} FENLLMapTile;

DD_DEF_ARRAY(FENLLMapTile, FENLLMapTile);

typedef struct {
	int turn;
	DDString description;
} FENLLogItem;

DD_DEF_ARRAY(FENLLogItem, FENLLogItem);

typedef enum {
	FE_NLL_MODE_WATCH,
	FE_NLL_MODE_SILENT,
	FE_NLL_MODE_PLAY,
} FENLLMode;

typedef struct {
	int height;
	int width;
	int turn;
	DDArrFENLLander populace;
	DDArrFENLLRelation relations;
	DDArrFENLLMapTile map;
	DDArrFENLLogItem event_log;
	FENLLMode mode;
	int player_id;
} FENLLWorld;

typedef struct {
	int height;
	int width;
	int num_population;
	DDArrDDString names;
	double pocket_floor;
	double pocket_ceil;
	double amt_floor;
	double amt_ceil;
	double interest_floor;
	double interest_ceil;
	FENLLMode mode;
} FENLLConfigureWorld;

/* encounter:
 * creditor can accept full payment
 * creditor can discharge debt by buying another debt
 * creditor can impose payment plan
 *
 */

void debug_print_relations(FENLLWorld *world);
void fe_nll_print_map(FENLLWorld *world);
void fe_nll_print_lander(FENLLander *lander);
void print_turn_log(FENLLander *lander);
void fe_nll_init_world(FENLLWorld *world, FENLLConfigureWorld *conf);
void fe_nll_tick(FENLLWorld *world);
void fe_nll_free_world(FENLLWorld *world);

#endif
