# 03 Flex Token Recognition

Genuine Flex scanner (`lexer.l`) with the macros `DIGIT`, `LETTER`, `ID` and `NUMBER`.
Recognised: keywords (`int float char if else while for break continue return void`), identifiers, integers, floating-point numbers, operators, string and character literals, separators. Comments are skipped; malformed input is reported as an error.

## Build

```bash
flex lexer.l
gcc lex.yy.c -o flex_lexer
```

## Run

```bash
./flex_lexer input.c
./flex_lexer < input.c
```

Required tools: `flex`, `gcc`. No `-lfl` is needed because the scanner uses `%option noyywrap`.
