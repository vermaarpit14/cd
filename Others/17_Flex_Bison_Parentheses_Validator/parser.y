%{
#include <stdio.h>
int yylex(void);
void yyerror(const char *s);
%}
%union { double num; }
%token <num> NUMBER
%token EOL
%type <num> expr
%left '+' '-'
%left '*' '/'
%%
input: %empty | input line;
line: EOL | expr EOL { printf("Valid expression; value = %.10g\n",$1); };
expr: NUMBER {$$=$1;} | expr '+' expr {$$=$1+$3;} | expr '-' expr {$$=$1-$3;} | expr '*' expr {$$=$1*$3;} | expr '/' expr {if($3==0){yyerror("division by zero");YYERROR;}$$=$1/$3;} | '(' expr ')' {$$=$2;};
%%
void yyerror(const char*s){fprintf(stderr,"Invalid expression: %s\n",s);}
int main(void){return yyparse();}
