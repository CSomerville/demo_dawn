%option noyywrap noinput nounput

%option prefix="twine_ball"

%option reentrant bison-bridge

%{

#include "dd_twine_ball.tab.h"
#define YY_EXIT_FAILURE ((void)yyscanner, EXIT_FAILURE)

#define YYSTYPE TWINE_BALLSTYPE

int monstreerror(void *yylval, const void *s, char const *msg);
%}

%%

[[:alpha:]][[:alnum:]|_]* {
	DDTwineBallItem *item = DD_ALLOCATE(DDTwineBallItem, 1);
	DDTwine *tmp_twine = DD_ALLOCATE(DDTwine, 1);
	dd_twine_init(tmp_twine);
	dd_twine_from_chars_dyn(tmp_twine, yytext);
	item->type = DD_TWINE_BALL_TWINE;
	item->value.t = tmp_twine;
	*yylval = item;
	return TWI;
}

\"[^"]*\" {
	int yytext_len;
	DDTwineBallItem *item = DD_ALLOCATE(DDTwineBallItem, 1);
	DDTwine *tmp_twine = DD_ALLOCATE(DDTwine, 1);
	dd_twine_init(tmp_twine);
	yytext_len = strlen(yytext);
	dd_twine_from_chars_fixed(tmp_twine, &yytext[1], yytext_len - 2);
	item->type = DD_TWINE_BALL_TWINE;
	item->value.t = tmp_twine;
	*yylval = item;
	return TWI;
}

"//".* ;

[[:space:]]	;

. { return *yytext; }
