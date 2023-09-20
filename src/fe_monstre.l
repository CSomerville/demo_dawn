%option noyywrap noinput nounput

%option prefix="monstre"

%option reentrant bison-bridge

%{

#include "fe_monstre.tab.h"
#define YY_EXIT_FAILURE ((void)yyscanner, EXIT_FAILURE)

#define YYSTYPE MONSTRESTYPE

int monstreerror(void *yylval, const void *s, char const *msg);
%}

%%

entities { return ENTITIES; }

relation_types { return RELATION_TYPES; }

relations { return RELATIONS; }

contents { return CONTENT; }

[[:alpha:]][[:alnum:]|_]* {
	DDTwine *tmp_twine = DD_ALLOCATE(DDTwine, 1);
	dd_twine_init(tmp_twine);
	dd_twine_from_chars_dyn(tmp_twine, yytext);
	*yylval = tmp_twine;
	return MON_TWI;
}

\"[^"]*\" {
	int yytext_len;
	DDTwine *tmp_twine = DD_ALLOCATE(DDTwine, 1);
	dd_twine_init(tmp_twine);
	yytext_len = strlen(yytext);
	dd_twine_from_chars_fixed(tmp_twine, &yytext[1], yytext_len - 2);
	*yylval = tmp_twine;
	return MON_TWI;
}

[[:space:]]	;

. { return *yytext; }
