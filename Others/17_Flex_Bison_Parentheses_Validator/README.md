Bison rejects missing, extra, or misplaced parentheses as syntax errors.
Build: `bison -d parser.y && flex lexer.l && gcc parser.tab.c lex.yy.c -lfl -o validator`
Run: `./validator < input.txt`
