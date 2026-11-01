# 11 Nested Parentheses Validator (Flex + Bison)

`lexer.l` is the Flex scanner and `parser.y` is the Bison grammar. Each line of the input is treated as one expression.

For every expression the program reports:

* whether the parentheses are balanced (the scanner counts unmatched `(` and `)`),
* whether the syntax is valid (the Bison grammar with `+ - * /`, unary minus, nesting and the usual precedence),
* the evaluated result, or a division-by-zero / syntax error.

## Build

```bash
bison -d parser.y
flex lexer.l
g++ -std=c++17 parser.tab.c lex.yy.c -o paren_validator
```

Required tools: `bison`, `flex`, `g++`. `-lfl` is not needed because the scanner uses `%option noyywrap`.

## Run

```bash
./paren_validator input.txt
echo '((10 + 5) * (8 - 3)) / 5' | ./paren_validator
echo '((10 + 5) * 3' | ./paren_validator
```

The program exits with status 2 when any expression is invalid.
