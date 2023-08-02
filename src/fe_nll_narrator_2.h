#include "dd_data.h"
#include "dd_twine.h"
#include "dd_twine_ball.tab.h"
#include "fe_neo_lib_land.h"

typedef struct {
	DDArrDDTwine *result;
	FENLLander *lander;
	DDTwineBallItem *narrator_conf;
} FENLLNarrator;

void narrate_from_turn_log(DDArrDDTwine *result, FENLLander *lander);
