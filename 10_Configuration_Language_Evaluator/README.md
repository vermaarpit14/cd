# 10 Configuration Language Evaluator (Flex + Bison)

`lexer.l` is the Flex scanner and `parser.y` is the Bison grammar. A symbol table (`std::map`) keeps variable values.

## Language

```
name = expression;     assignment, prints "name = value"
print expression;      prints "Result = value"
```

* Integers and floating-point numbers, `+ - * /`, unary minus, parentheses.
* Integer arithmetic stays integer (`7 / 2` is `3`); mixing in a float gives a float.
* `#` and `//` start line comments.
* Reported errors: undefined variable, division by zero, syntax errors, unexpected characters. After an error the evaluator skips to the next `;` and continues.

## Build

```bash
bison -d parser.y
flex lexer.l
g++ -std=c++17 parser.tab.c lex.yy.c -o config_eval
```

Required tools: `bison`, `flex`, `g++`. `-lfl` is not needed because the scanner uses `%option noyywrap`.

## Run

```bash
./config_eval input.txt
printf 'x = 4 / (2 - 2);\nprint q;\ny = 3 +;\nprint 2.5 * 2;\n' | ./config_eval
```

The program exits with status 2 when any error was reported.
