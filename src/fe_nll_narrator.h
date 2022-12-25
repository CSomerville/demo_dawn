#include "dd_data.h"
#include "fe_neo_lib_land.h"
#include "te_tendril.h"

typedef struct {
	DDArrDDString *result;
	FENLLander *lander;
	TETendril *tendril;
	DDArrInt unused_sees_indices;
	int state;
} FENLLNarrator;

void narrate_from_turn_log(DDArrDDString *result, FENLLander *lander,
		TETendril *tendril);
