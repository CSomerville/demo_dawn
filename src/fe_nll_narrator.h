#include "dd_data.h"
#include "fe_neo_lib_land.h"
#include "te_tendril.h"

/* we want to move a lot of the actual writing out of here and into
 * a filed that can be parsed, leaving just the logic in this module
 * so what sorts of elems are those, right now?
 * - lander descriptor
 * - outstanding debts descriptor
 * - outstanding credits descriptor
 * - pocket descriptor
 * - sees across
 * - emptiness
 * - seen lander descriptor
 * - charging lander descriptor
 * - charging lander does
 * - fleeing lander descriptor
 * - fleeing lander does
 * - lander has no goal
 */

typedef struct {
	DDArrDDString *result;
	FENLLander *lander;
	TETendril *tendril;
	DDArrInt unused_sees_indices;
	int state;
} FENLLNarrator;

void narrate_from_turn_log(DDArrDDString *result, FENLLander *lander,
		TETendril *tendril);
