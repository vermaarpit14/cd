%{
#include <stdio.h>
#include <stdlib.h>
int yylex(void);
void yyerror(const char *s);
%}
%union { double num; }
%token <num> NUMBER
%token EOL
%type <num> expr
%left '+' '-'
%left '*' '/'
%right UMINUS
%%
input: %empty | input line;
line: EOL | expr EOL { printf("Result = %.10g\n", $1); };
expr: NUMBER { $$=$1; }
    | expr '+' expr { $$=$1+$3; }
    | expr '-' expr { $$=$1-$3; }
    | expr '*' expr { $$=$1*$3; }
    | expr '/' expr { if($3==0){yyerror("division by zero");YYERROR;} $$=$1/$3; }
    | '-' expr %prec UMINUS { $$=-$2; }
    | '(' expr ')' { $$=$2; }
    ;
%%
void yyerror(const char *s){fprintf(stderr,"Error: %s\n",s);}
int main(void){return yyparse();}
