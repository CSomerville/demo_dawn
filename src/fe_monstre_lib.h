#ifndef fe_monstre_lib_h
#define fe_monstre_lib_h
#include "fe_monstre.tab.h"

typedef struct {
	DDArrDDTwine collect;
	int entity;
} FEMonstreState;

void fe_monstre_init(FEMonstreState *state, FEMonstreData *dat);
void fe_monstre_pick_initial_entity(FEMonstreState *state,
		FEMonstreData *dat);
void fe_monstre_tick(FEMonstreState *state, FEMonstreData *dat);
void fe_monstre_parse(FEMonstreData *dat, FILE *f);
void fe_monstre_teardown(FEMonstreState *state,
		FEMonstreData *dat);

#endif
