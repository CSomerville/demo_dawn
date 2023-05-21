typedef struct {
	DDArrDDTwine collect;
	int entity;
} FEMonstreState;

void fe_monstre_init(FEMonstreState *state, struct FEMonstreData *dat);
void fe_monstre_pick_initial_entity(FEMonstreState *state,
		struct FEMonstreData *dat);
void fe_monstre_tick(FEMonstreState *state, struct FEMonstreData *dat);
void fe_monstre_parse(struct FEMonstreData *dat, FILE *f);
void fe_monstre_teardown(FEMonstreState *state,
		struct FEMonstreData *dat);
