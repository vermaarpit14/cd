%option noyywrap yylineno

DIGIT     [0-9]
LETTER    [A-Za-z_]
EXPONENT  [eE][+-]?{DIGIT}+
FLOATING  (({DIGIT}+"."{DIGIT}*|"."{DIGIT}+){EXPONENT}?|{DIGIT}+{EXPONENT})

%{
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include "parser.tab.h"

void lexicalError(const char *text);
%}

%%

[ \t\r\n]+            ;
"#".*                 ;
"//".*                ;

"print"               { return PRINT; }

{FLOATING}            {
    yylval.value.isFloat = 1;
    yylval.value.integerValue = 0;
    yylval.value.floatValue = std::atof(yytext);
    return FLOAT_LITERAL;
}

{DIGIT}+              {
    errno = 0;
    long long parsed = std::strtoll(yytext, nullptr, 10);
    if (errno == ERANGE) {
        yylval.value.isFloat = 1;
        yylval.value.integerValue = 0;
        yylval.value.floatValue = std::atof(yytext);
        return FLOAT_LITERAL;
    }
    yylval.value.isFloat = 0;
    yylval.value.integerValue = parsed;
    yylval.value.floatValue = 0.0;
    return INTEGER_LITERAL;
}

{LETTER}({LETTER}|{DIGIT})* {
    yylval.name = strdup(yytext);
    return IDENTIFIER;
}

[=;+\-*/()]           { return yytext[0]; }

.                     { lexicalError(yytext); }

%%
