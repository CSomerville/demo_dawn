#include <stdio.h>
#define YYSTYPE MONSTRESTYPE
#include "fe_monstre.tab.h"
#include "fe_monstre.lex.h"
#include "fe_monstre_lib.h"
#include "dd_data.h"

static void init_monstre_data(struct FEMonstreData *dat) {
	DD_INIT_ARRAY(&dat->entities);
	DD_INIT_ARRAY(&dat->relation_types);
	DD_INIT_ARRAY(&dat->relations);
	DD_INIT_ARRAY(&dat->contents);
}

static void init_monstre_state(FEMonstreState *state) {
	DD_INIT_ARRAY(&state->collect);
	state->entity = 0;
}

static void destroy_monstre_state(FEMonstreState *state) {
	dd_arr_dd_twine_destroy(&state->collect);
}

static void copy_entity_by_index(DDTwine *tw, struct FEMonstreData *dat,
		unsigned int index) {
	dd_twine_copy(tw, &dat->entities.elems[index]);
}

static int pick_random_entity(struct FEMonstreData *dat) {
	int r = rand() % dat->entities.size;
	return r;
}

static void filter_relations_by_subject(DDArrInt *indices,
		int subject_index, struct FEMonstreData *dat) {
	int i;
	for (i = 0; i < dat->relations.size; i++)
		if (dat->relations.elems[i].subject == subject_index)
			DD_ADD_ARRAY(indices, i);
}

static void filter_content_by_relation_type(DDArrInt *indices,
		int relation_type_index, struct FEMonstreData *dat) {
	int i;
	for (i = 0; i < dat->contents.size; i++)
		if (dat->contents.elems[i].relation_type == relation_type_index)
			DD_ADD_ARRAY(indices, i);
}

void fe_monstre_init(FEMonstreState *state, struct FEMonstreData *dat) {
	init_monstre_data(dat);
	init_monstre_state(state);
}

void fe_monstre_pick_initial_entity(FEMonstreState *state,
		struct FEMonstreData *dat) {
	DDTwine tmp;
	dd_twine_init(&tmp);
	state->entity = DD_SELECT_ARRAY(&dat->entities);
	copy_entity_by_index(&tmp, dat, state->entity);
	DD_ADD_ARRAY(&state->collect, tmp);
	dd_twine_init(&tmp);
}

void fe_monstre_tick(FEMonstreState *state, struct FEMonstreData *dat) {
	int idx;
	DDArrInt indices;
	DDTwine tmp;

	dd_twine_init(&tmp);
	DD_INIT_ARRAY(&indices);

	filter_relations_by_subject(&indices, state->entity, dat);
	idx = DD_SELECT_ARRAY(&indices);
	idx = indices.elems[idx];
	state->entity = dat->relations.elems[idx].object;
	DD_FREE_ARRAY(&indices);

	filter_content_by_relation_type(&indices,
		dat->relations.elems[idx].relation_type, dat);
	idx = DD_SELECT_ARRAY(&indices);
	idx = indices.elems[idx];
	dd_twine_copy(&tmp, &dat->contents.elems[idx].present_participle);
	DD_ADD_ARRAY(&state->collect, tmp);
	dd_twine_init(&tmp);

	copy_entity_by_index(&tmp, dat, state->entity);
	DD_ADD_ARRAY(&state->collect, tmp);
	dd_twine_init(&tmp);

}

void fe_monstre_parse(struct FEMonstreData *dat, FILE *f) {
	int i;
	yyscan_t scanner;
	if ((i = monstrelex_init(&scanner)) != 0) {
		exit(i);
	}
	monstreset_in(f, scanner);
	int e = monstreparse(dat, scanner);
	if (e != 0)
		exit(e);
	monstrelex_destroy(scanner);
}

void fe_monstre_teardown(FEMonstreState *state,
		struct FEMonstreData *dat) {
	accum_free(dat);
	destroy_monstre_state(state);
}
