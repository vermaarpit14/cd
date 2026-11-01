%option noyywrap
%{
#include "parser.tab.h"
#include <stdlib.h>
%}
%%
[0-9]+(\.[0-9]+)? { yylval.num=atof(yytext); return NUMBER; }
[ \t\r]+          { }
\n                { return EOL; }
.                 { return yytext[0]; }
%%
