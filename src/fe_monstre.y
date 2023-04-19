/* start with something simple. blarg: ID* ; stror: ID*
	IDs are comma separated. blarg and stror can be in any
	order and occur any number of times */

%define api.pure true
%define api.prefix {monstre}
%define parse.error verbose
%code top {
	#include <stdio.h>
	#include "dd_data.h"
	/*#include <stdlib.h>*/
	/*#include "dd_twine.h"*/

	#ifndef YYNOMEM
	#define YYNOMEM goto yyexhaustedlab
	#endif
}

%code requires {
	#include "dd_data.h"
	DD_DEF_ARRAY(long int, LongInt)

	struct accum {
		long int cur;
		DDArrLongInt acc;
	};
}
%define api.value.type {long}

%parse-param {struct accum *accum}

%param {void *scanner}

%code provides {
	void accum_free(struct accum *a);
}

%code {
	int monstreerror(void *foo, char const *msg, const void *s);
	int monstrelex(void *lval, const void *s);
}

%token NUM

%%

start :
	  input { return 0; }
;

input :
	%empty
	| input line
;

line :
	 '\n'
	| numbers ';' { DD_ADD_ARRAY(&accum->acc, accum->cur); }
;

numbers :
		numbers ',' NUM { accum->cur += $3; }
	|	NUM { accum->cur = $1; }
;

%%

int monstreerror(void *yylval, char const *msg, const void *s)
{
	(void)yylval;
	(void)s;
	return fprintf(stderr, "%s\n", msg);
}

void accum_free(struct accum *a) {
	if (!a)
		return;
	DD_FREE_ARRAY(&a->acc);
	free(a);
}
