%option noyywrap yylineno
%x BLOCK_COMMENT

DIGIT     [0-9]
LETTER    [a-zA-Z_]
ID        {LETTER}({LETTER}|{DIGIT})*
NUMBER    {DIGIT}+
EXPONENT  [eE][+-]?{DIGIT}+
FLOATING  (({DIGIT}+"."{DIGIT}*|"."{DIGIT}+){EXPONENT}?|{DIGIT}+{EXPONENT})[fFlL]?
HEX       0[xX][0-9a-fA-F]+

%{
#include <stdio.h>

static int tokenCount = 0;
static int errorCount = 0;
static int commentStartLine = 0;

static void report(const char *type, const char *lexeme) {
    ++tokenCount;
    printf("%-6d %-28s %s\n", yylineno, lexeme, type);
}

static void reportError(const char *reason, const char *lexeme) {
    ++tokenCount;
    ++errorCount;
    printf("%-6d %-28s ERROR (%s)\n", yylineno, lexeme, reason);
}
%}

%%

"int"|"float"|"char"|"if"|"else"|"while"|"for"|"break"|"continue"|"return"|"void" {
    report("KEYWORD", yytext);
}

{ID}        { report("IDENTIFIER", yytext); }
{HEX}       { report("INTEGER", yytext); }
{FLOATING}  { report("FLOAT", yytext); }
{NUMBER}    { report("INTEGER", yytext); }

{DIGIT}+{LETTER}({LETTER}|{DIGIT})* { reportError("malformed number", yytext); }

\"([^"\\\n]|\\.)*\"  { report("STRING_LITERAL", yytext); }
\"([^"\\\n]|\\.)*    { reportError("unterminated string", yytext); }
'([^'\\\n]|\\.)'     { report("CHAR_LITERAL", yytext); }
'                    { reportError("invalid character literal", yytext); }

"<<="|">>="|"++"|"--"|"+="|"-="|"*="|"/="|"%="|"=="|"!="|"<="|">="|"&&"|"||"|"<<"|">>"|"->"|[-+*/%=<>!&|^~?:.] {
    report("OPERATOR", yytext);
}

[(){}\[\];,]  { report("SEPARATOR", yytext); }

"//".*        ;
"/*"          { commentStartLine = yylineno; BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"   { BEGIN(INITIAL); }
<BLOCK_COMMENT>.|\n   ;
<BLOCK_COMMENT><<EOF>> {
    yylineno = commentStartLine;
    reportError("unterminated comment", "/*");
    BEGIN(INITIAL);
    yyterminate();
}

[ \t\r\n]+    ;

.             { reportError("unrecognized character", yytext); }

%%

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [source_file]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        yyin = fopen(argv[1], "r");
        if (yyin == NULL) {
            fprintf(stderr, "Error: cannot open source file '%s'\n", argv[1]);
            return 1;
        }
    }
    printf("%-6s %-28s %s\n", "Line", "Lexeme", "Token Type");
    printf("------------------------------------------------------------\n");
    yylex();
    printf("\nTotal tokens: %d, errors: %d\n", tokenCount, errorCount);
    if (yyin != stdin) {
        fclose(yyin);
    }
    return errorCount > 0 ? 2 : 0;
}
