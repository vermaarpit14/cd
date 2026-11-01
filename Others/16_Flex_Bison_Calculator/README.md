Build: `bison -d parser.y && flex lexer.l && gcc parser.tab.c lex.yy.c -lfl -o calculator`
Run: `./calculator < input.txt`
