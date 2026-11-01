%option noyywrap
%{
#include <stdio.h>
%}
DIGIT       [0-9]
ID          [a-zA-Z_][a-zA-Z0-9_]*
%%
"int"|"float"|"char"|"double"|"if"|"else"|"while"|"for"|"return"|"void"   { printf("KEYWORD       %s\n", yytext); }
{ID}        { printf("IDENTIFIER    %s\n", yytext); }
{DIGIT}+([.]{DIGIT}+)? { printf("NUMBER        %s\n", yytext); }
\"([^\\\"]|\\.)*\" { printf("STRING        %s\n", yytext); }
"=="|"!="|"<="|">="|"++"|"--"|"+="|"-="|"&&"|"||"|[+*/%=<>!-] { printf("OPERATOR      %s\n", yytext); }
[(){}\[\];,] { printf("SYMBOL        %s\n", yytext); }
"//"[^\n]*  { }
[ \t\r\n]+  { }
.           { printf("UNKNOWN       %s\n", yytext); }
%%
int main(void) { yylex(); return 0; }
