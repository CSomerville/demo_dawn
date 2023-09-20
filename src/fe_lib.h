#ifndef fe_lib_h
#define fe_lib_h

#include "dd_data.h"
#include "dd_twine.h"
#include "di_lib.h"
#include "te_tendril.h"
#include "fe_neo_lib_land.h"
#include "fe_monstre_lib.h"
#include "fe_monstre.tab.h"

typedef struct FEstival {
	TETendril *therm;
	int therm_state;
	FENLLWorld *nll_world;
	FEMonstreData *monstre_dat;
	FEMonstreState *monstre_state;
	int last_monstre_read;

	DDTwine *raw;
	DDArrDDTwineWB *word_bounds;
	FILE *dict;
	DDArrDIIndexedEntry *dict_entries;
	DDArrInt *line_indices;
	int last_lineated_index;
} FEstival;

void init_festival(FEstival *festival);
void inaugurate_festival(FEstival *festival);
void destroy_festival(FEstival *festival);
void run_n_steps(TETendril *tendril, DDArrDDString *result,
		int state, int n_steps);

#endif
