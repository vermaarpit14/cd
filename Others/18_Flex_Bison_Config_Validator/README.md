Accepts simple `key = value;` settings where values are identifiers, numbers, or quoted strings.
Build: `bison -d parser.y && flex lexer.l && gcc parser.tab.c lex.yy.c -lfl -o config_check`
Run: `./config_check < input.txt`
