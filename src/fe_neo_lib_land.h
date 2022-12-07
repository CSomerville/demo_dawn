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
	FE_NLL_GOAL_TYPE_NULL,
	FE_NLL_GOAL_TYPE_FLEE,
	FE_NLL_GOAL_TYPE_CHARGE,
} FENLLGoalType;

typedef struct FENLLander {
	int id;
	DDString name;
	FENLLPoint loc;
	struct {
		double fuse_coin;
		double frux_coin;
	} pocket;
	struct {
		FENLLGoalType goal_type;
		struct FENLLander *lander;
		int invisible_ctr;
	} goal;
	int last_move;
	int sight;
	bool alive;
} FENLLander;

DD_DEF_ARRAY(FENLLander, FENLLander);

typedef enum {
	FE_NLL_COIN_FUSE,
	FE_NLL_COIN_FRUX,
} FENLLCoin;

typedef struct {
	int debtor_id;
	int creditor_id;
	double amt;
	double interest;
	FENLLCoin coin;
} FENLLRelation;

DD_DEF_ARRAY(FENLLRelation, FENLLRelation);

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
 * creditor can accept partial payment
 * creditor can discharge the debt at discount
 * creditor can attack debtor
 * creditor can discharge debt by buying another debt
 *
 * remove possibility of duplicate start locations
 */

void fe_nll_print_map(FENLLWorld *world);
void fe_nll_print_lander(FENLLander *lander);
void fe_nll_init_world(FENLLWorld *world, FENLLConfigureWorld *conf);
void fe_nll_tick(FENLLWorld *world);
void fe_nll_free_world(FENLLWorld *world);

#endif
