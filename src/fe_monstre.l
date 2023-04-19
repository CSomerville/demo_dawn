%option noyywrap noinput nounput

%option prefix="monstre"

%option reentrant bison-bridge

%{

#include "fe_monstre.tab.h"
#define YY_EXIT_FAILURE ((void)yyscanner, EXIT_FAILURE)

#define YYSTYPE MONSTRESTYPE

int monstreerror(char const *msg);
%}

%%

[-+]?[[:digit:]]+ {
	long n = strtol(yytext, NULL, 10);
	*yylval = n;
	return NUM;
}

[[:space:]]	;

. { return *yytext; }
