/* example

entity: you, workfriend, pres;

relation_type: pmc_equal, subject;

relation:
	(pmc_equal, you, workfriend),
	(pmc_equal, workfriend, you),
	(subject, you, pres),
	(subject, workfriend, pres);

content:
	(pmc_equal, "shaking hands with", "shook lil hands with");

*/

%define api.pure true
%define api.prefix {monstre}
%define parse.error verbose
%code top {
	#include <stdio.h>
	#include "dd_data.h"
	/*#include <stdlib.h>*/
	#include "dd_twine.h"

	#ifndef YYNOMEM
	#define YYNOMEM goto yyexhaustedlab
	#endif
}

%code requires {
	#include "dd_data.h"
	#include "dd_twine.h"

	typedef enum {
		FE_MONSTRE_MODE_ENTITY,
		FE_MONSTRE_MODE_RELATION_TYPE,
		FE_MONSTRE_MODE_RELATION,
		FE_MONSTRE_MODE_CONTENT,
	} FEMonstreMode;

	typedef struct {
		int relation_type;
		int subject;
		int object;
	} FEMonstreRelation;

	typedef struct {
		int relation_type;
		DDTwine present_participle;
		DDTwine past;
	} FEMonstreContent;

	DD_DEF_ARRAY(FEMonstreRelation, FEMonstreRelation);
	DD_DEF_ARRAY(FEMonstreContent, FEMonstreContent);

	struct FEMonstreData {
		FEMonstreMode mode;
		DDArrDDTwine entities;
		DDArrDDTwine relation_types;
		DDArrFEMonstreRelation relations;
		DDArrFEMonstreContent contents;
	};
}

%define api.value.type {DDTwine *}

%parse-param {struct FEMonstreData *accum}

%param {void *scanner}

%code provides {
	void accum_free(struct FEMonstreData *a);
}

%code {
	int monstreerror(void *yylval, const void *s, char const *msg);
	int monstrelex(void *lval, const void *s);
}

%token ENTITIES
%token RELATION_TYPES
%token RELATIONS
%token CONTENT
%token TWI

%%

start :
	  input { return 0; }
;

input :
	%empty
	| input line
;

line :
		subject ':' predicate ';'
;

subject :
		ENTITIES { accum->mode = FE_MONSTRE_MODE_ENTITY; }
	|	RELATION_TYPES { accum->mode =  FE_MONSTRE_MODE_RELATION_TYPE; }
	|	RELATIONS { accum->mode = FE_MONSTRE_MODE_RELATION; }
	|	CONTENT { accum->mode = FE_MONSTRE_MODE_CONTENT; }
;

predicate :
	twines | triples
;

twines :
		twines ',' TWI {  
			switch (accum->mode) {
				case FE_MONSTRE_MODE_ENTITY:
					DD_ADD_ARRAY(&accum->entities, *($3));
					break;
				case FE_MONSTRE_MODE_RELATION_TYPE:
					DD_ADD_ARRAY(&accum->relation_types, *($3));
					break;
			}
			free($3);
		}
	|	TWI { 
			switch (accum->mode) {
				case FE_MONSTRE_MODE_ENTITY:
					DD_ADD_ARRAY(&accum->entities, *($1));
					break;
				case FE_MONSTRE_MODE_RELATION_TYPE:
					DD_ADD_ARRAY(&accum->relation_types, *($1));
					break;
			}
			free($1);
		}
;

triples :
		triples ',' triple
		| triple
;

triple :
	  '(' TWI ',' TWI ',' TWI ')' {
		switch (accum->mode) {
			int n;
			case FE_MONSTRE_MODE_RELATION:
				FEMonstreRelation *rel;
				rel = DD_ALLOCATE(FEMonstreRelation, 1);
				n = dd_arr_dd_twine_index_of(&accum->relation_types, $2);
				rel->relation_type = n;
				n = dd_arr_dd_twine_index_of(&accum->entities, $4);
				rel->subject = n;
				n = dd_arr_dd_twine_index_of(&accum->entities, $6);
				rel->object = n;
				DD_ADD_ARRAY(&accum->relations, *rel);
				dd_twine_destroy($2);
				dd_twine_destroy($4);
				dd_twine_destroy($6);
				free($2);
				free($4);
				free($6);
				free(rel);
				break;
			case FE_MONSTRE_MODE_CONTENT:
				FEMonstreContent *con;
				con = DD_ALLOCATE(FEMonstreContent, 1);
				n = dd_arr_dd_twine_index_of(&accum->relation_types, $2);
				con->relation_type = n;
				con->present_participle = *($4);
				con->past = *($6);
				DD_ADD_ARRAY(&accum->contents, *con);
				dd_twine_destroy($2);
				free($2);
				free($4);
				free($6);
				free(con);
				break;
		}
	}

%%

int monstreerror(void *yylval, const void *s, char const *msg)
{
	(void)yylval;
	(void)s;
	return fprintf(stderr, "msg: %s\n", msg);
}

void accum_free(struct FEMonstreData *a) {
	int i;
	if (!a)
		return;
	dd_arr_dd_twine_destroy(&a->entities);
	dd_arr_dd_twine_destroy(&a->relation_types);
	DD_FREE_ARRAY(&a->relations);
	for (i = 0; i < a->contents.size; i++) {
		dd_twine_destroy(&a->contents.elems[i].present_participle);
		dd_twine_destroy(&a->contents.elems[i].past);
	}
	DD_FREE_ARRAY(&a->contents);
}
