%{
#include <stdio.h>
#include <stdlib.h>
int yylex(void); void yyerror(const char *s);
%}
%union { char *text; }
%token <text> IDENT NUMBER STRING
%%
config: %empty | config statement;
statement: IDENT '=' value ';' { printf("Valid setting: %s\n",$1); free($1); };
value: IDENT {free($1);} | NUMBER {free($1);} | STRING {free($1);};
%%
void yyerror(const char*s){fprintf(stderr,"Configuration syntax error: %s\n",s);}
int main(void){int r=yyparse();if(!r)puts("Configuration file accepted.");return r;}
