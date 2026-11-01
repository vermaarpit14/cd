# Compiler Design Lab

Solutions to the 13 Compiler Design lab questions, one self-contained project per question. Standalone programs are C++17. The three questions that name Flex or Bison use genuine `.l` and `.y` files (no hand-written parser stands in for them).

## Folder structure

```
Compiler-Design-Lab/
├── 01_Two_Buffer_Input_System/          main.cpp
├── 02_Source_Code_Lexical_Analyzer/     main.cpp, input.c
├── 03_Flex_Token_Recognition/           lexer.l, input.c, README.md
├── 04_Maximal_Munch_Lexical_Analyzer/   main.cpp, input.c
├── 05_CFG_Language_Pattern/             main.cpp
├── 06_CFG_AnBn_Pattern_Matching/        main.cpp
├── 07_LL1_Predictive_Parser/            main.cpp
├── 08_Left_Recursion_Elimination/       main.cpp
├── 09_LR0_Parsing_Table/                main.cpp
├── 10_Configuration_Language_Evaluator/ lexer.l, parser.y, input.txt, README.md
├── 11_Nested_Parentheses_Validator/     lexer.l, parser.y, input.txt, README.md
├── 12_AST_Semantic_Analyzer/            main.cpp
├── 13_Three_Address_Code_Generator/     main.cpp
├── README.md
└── build_all.sh
```

## Question mapping

Assignment 2 Question 1 (Flex token recognition) and Assignment 6 Questions 1 and 2 (the two Flex + Bison programs) are fixed by the lab brief. The other assignment numbers below follow the order of the questions and should be adjusted if your numbering differs.

| Folder | Topic | Assignment |
|---|---|---|
| 01_Two_Buffer_Input_System | Two-buffer input with sentinels | Assignment 1 |
| 02_Source_Code_Lexical_Analyzer | Table-printing lexical analyzer | Assignment 1 |
| 03_Flex_Token_Recognition | Flex token recognition | Assignment 2, Question 1 |
| 04_Maximal_Munch_Lexical_Analyzer | Longest-match lexer with rule priority | Assignment 2 |
| 05_CFG_Language_Pattern | CFG for `(aab)^n a^n` | Assignment 3 |
| 06_CFG_AnBn_Pattern_Matching | `matchesAnB` for `A^n B^n` | Assignment 4 |
| 07_LL1_Predictive_Parser | FIRST/FOLLOW, LL(1) table, parser | Assignment 5 |
| 08_Left_Recursion_Elimination | Left recursion removal | Assignment 5 |
| 09_LR0_Parsing_Table | LR(0) item sets, conflict, SLR(1) | Assignment 5 |
| 10_Configuration_Language_Evaluator | Flex + Bison evaluator | Assignment 6, Question 1 |
| 11_Nested_Parentheses_Validator | Flex + Bison validator | Assignment 6, Question 2 |
| 12_AST_Semantic_Analyzer | AST, symbol table, type checking | Assignment 7 |
| 13_Three_Address_Code_Generator | TAC, quadruples, triples | Assignment 8 |

## Required software

* `g++` with C++17 support
* `gcc`
* `flex` (tested with 2.6.4)
* `bison` (tested with 3.8)

On Ubuntu or Debian: `sudo apt-get install g++ gcc flex bison`

## Compilation

Build everything at once (binaries go to `bin/`, generated Flex/Bison sources to `build/`):

```bash
./build_all.sh
./build_all.sh --run
./build_all.sh clean
```

Or build a single question by hand:

```bash
g++ -std=c++17 -Wall -Wextra 07_LL1_Predictive_Parser/main.cpp -o ll1_parser
```

Flex-only (question 03):

```bash
cd 03_Flex_Token_Recognition
flex lexer.l
gcc lex.yy.c -o flex_lexer
```

Flex + Bison (questions 10 and 11):

```bash
cd 10_Configuration_Language_Evaluator
bison -d parser.y
flex lexer.l
g++ -std=c++17 parser.tab.c lex.yy.c -o config_eval
```

## Execution

All programs run with no arguments or with the sample input shown. Run them from inside their own folder.

| Folder | Command |
|---|---|
| 01 | `./two_buffer [source_file] [buffer_size]` (writes and uses `sample_input.c` when no file is given) |
| 02 | `./lexer input.c` |
| 03 | `./flex_lexer input.c` |
| 04 | `./maximal_munch input.c` |
| 05 | `./cfg_pattern [string]` (default `aabaabaa`) |
| 06 | `./anbn [S A B n]` (default `aabaabaa aab a 2`) |
| 07 | `./ll1 [strings...]` |
| 08 | `./left_recursion [grammar_file]` |
| 09 | `./lr0 [strings...]` (default `abb` plus extra tests) |
| 10 | `./config_eval input.txt` |
| 11 | `./paren_validator input.txt` |
| 12 | `./semantic [source_file]` |
| 13 | `./tac [statements...]` or `./tac -` to read statements from standard input |

Notes:

* Question 09: the grammar `S -> A, A -> Ab | a` is not LR(0). State 2 holds the completed item `S -> A.` and the item `A -> A.b`, giving a shift-reduce conflict on `b`. The program reports it and then builds a separate SLR(1) table (reduce only on FOLLOW) to parse `abb`. The two tables are labelled separately.
* Questions 02 to 04: unrecognised input is reported as `ERROR` tokens and the program exits with status 2.
