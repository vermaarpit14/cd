%option noyywrap

%{
#include <cstdlib>
#include <string>
#include "parser.tab.h"

extern int openParenthesisCount;
extern int unmatchedCloseCount;
extern std::string lexicalMessage;
%}

DIGIT   [0-9]

%%

[ \t\r\n]+                          ;
({DIGIT}+("."{DIGIT}*)?|"."{DIGIT}+) { yylval.number = std::atof(yytext); return NUMBER; }
"("                                  { ++openParenthesisCount; return '('; }
")"                                  {
    if (openParenthesisCount > 0) {
        --openParenthesisCount;
    } else {
        ++unmatchedCloseCount;
    }
    return ')';
}
[-+*/]                               { return yytext[0]; }
.                                    {
    if (lexicalMessage.empty()) {
        lexicalMessage = std::string("invalid character '") + yytext + "'";
    }
    return INVALID;
}

%%
