#ifndef fe_lib_h
#define fe_lib_h

#include "dd_data.h"
#include "te_tendril.h"

/* so uh, collection of entities
 * types of relations
 * relation from to
 * action on relation
 * entities:
 * 		you
 * 		workfriend
 * 		pres
 * 		celeb
 * 		mom
 */

typedef enum {
	FE_REL_TYPE_PMC_EQUAL,
	FE_REL_TYPE_SHOW_EQUAL,
	FE_REL_TYPE_CELEB_PRES,
	FE_REL_TYPE_PRES_CELEB,
	FE_REL_TYPE_HAS_MOM,
	FE_REL_TYPE_IS_MOM,
	FE_REL_TYPE_CELEB_PPL,
	FE_REL_TYPE_PPL_CELEB,
	FE_REL_TYPE_PRES_PPL,
	FE_REL_TYPE_PPL_PRES,
} FERelType;

typedef struct {
	int id;
	DDString name;
} FEEntity;

DD_DEF_ARRAY(FEEntity, FEEntity);

typedef struct {
	FERelType type;
	int sub_id;
	int obj_id;
} FEEntityRelation;

DD_DEF_ARRAY(FEEntityRelation, FEEntityRelation);

typedef struct {
	FERelType type;
	DDString pres_particip;
	DDstring past;
} FERelationContent;

void run_n_steps(TETendril *tendril, DDArrDDString *result,
		int state, int n_steps);

#endif

