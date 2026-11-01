# Compiler Design Lab — Expected Questions Practice Pack

This pack contains 22 small lab-exam-style programs based on lexical analysis, CFGs, parsing, Flex/Bison, semantic analysis, and intermediate code generation.

## Requirements

For C++ programs:

```bash
sudo apt update
sudo apt install build-essential
```

For Flex/Bison programs:

```bash
sudo apt install flex bison libfl-dev
```

## Run a C++ question

```bash
cd 02_Validate_Identifier
g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o program
./program < input.txt
```

Some programs accept a source filename as an argument, for example:

```bash
cd 01_Count_Tokens_And_Lexemes
g++ -std=c++17 -O2 main.cpp -o program
./program input.c
```

## Run a Flex-only question

```bash
cd 04_Flex_Token_Recognition
flex lexer.l
gcc lex.yy.c -lfl -o lexer
./lexer < input.c
```

## Run a Flex + Bison question

```bash
cd 16_Flex_Bison_Calculator
bison -d parser.y
flex lexer.l
gcc parser.tab.c lex.yy.c -lfl -o program
./program < input.txt
```

## Notes

- All grammars use single-character symbols where the individual README says so; `#` represents epsilon.
- The programs are intentionally kept small enough for a one-hour lab examination. They are practice implementations, not production compiler components.
- `15_LR0_Items_And_Conflict_Checker` checks LR(0) conflicts. A grammar can be SLR(1) but not LR(0); the program reports LR(0) conflicts rather than silently treating it as LR(0).
- Input files are provided as examples. Change them to practice valid and invalid cases.
