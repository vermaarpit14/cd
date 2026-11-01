%option noyywrap
%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>
%}
%%
[0-9]+(\.[0-9]+)? { yylval.text=strdup(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.text=strdup(yytext); return IDENT; }
\"([^\\\"]|\\.)*\" { yylval.text=strdup(yytext); return STRING; }
[ \t\r\n]+ { }
. { return yytext[0]; }
%%
