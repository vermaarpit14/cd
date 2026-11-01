#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$ROOT/bin"
BUILD="$ROOT/build"

CPP_PROJECTS=(
    01_Two_Buffer_Input_System
    02_Source_Code_Lexical_Analyzer
    04_Maximal_Munch_Lexical_Analyzer
    05_CFG_Language_Pattern
    06_CFG_AnBn_Pattern_Matching
    07_LL1_Predictive_Parser
    08_Left_Recursion_Elimination
    09_LR0_Parsing_Table
    12_AST_Semantic_Analyzer
    13_Three_Address_Code_Generator
)

require_tool() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing required tool: $1" >&2
        exit 1
    fi
}

if [ "${1:-}" = "clean" ]; then
    rm -rf "$BIN" "$BUILD"
    echo "Removed bin/ and build/"
    exit 0
fi

require_tool g++
require_tool gcc
require_tool flex
require_tool bison

mkdir -p "$BIN" "$BUILD"

for project in "${CPP_PROJECTS[@]}"; do
    echo "Compiling $project"
    g++ -std=c++17 -Wall -Wextra -O2 "$ROOT/$project/main.cpp" -o "$BIN/$project"
done

echo "Compiling 03_Flex_Token_Recognition"
mkdir -p "$BUILD/03"
flex -o "$BUILD/03/lex.yy.c" "$ROOT/03_Flex_Token_Recognition/lexer.l"
gcc -O2 "$BUILD/03/lex.yy.c" -o "$BIN/03_Flex_Token_Recognition"

for project in 10_Configuration_Language_Evaluator 11_Nested_Parentheses_Validator; do
    echo "Compiling $project"
    mkdir -p "$BUILD/$project"
    bison -d -o "$BUILD/$project/parser.tab.c" "$ROOT/$project/parser.y"
    flex -o "$BUILD/$project/lex.yy.c" "$ROOT/$project/lexer.l"
    g++ -std=c++17 -O2 -I"$BUILD/$project" "$BUILD/$project/parser.tab.c" "$BUILD/$project/lex.yy.c" -o "$BIN/$project"
done

echo "All programs compiled into $BIN"

if [ "${1:-}" = "--run" ]; then
    run_section() {
        echo
        echo "################ $1 ################"
    }
    run_section 01_Two_Buffer_Input_System
    (cd "$BUILD" && "$BIN/01_Two_Buffer_Input_System")
    run_section 02_Source_Code_Lexical_Analyzer
    "$BIN/02_Source_Code_Lexical_Analyzer" "$ROOT/02_Source_Code_Lexical_Analyzer/input.c"
    run_section 03_Flex_Token_Recognition
    "$BIN/03_Flex_Token_Recognition" "$ROOT/03_Flex_Token_Recognition/input.c"
    run_section 04_Maximal_Munch_Lexical_Analyzer
    "$BIN/04_Maximal_Munch_Lexical_Analyzer" "$ROOT/04_Maximal_Munch_Lexical_Analyzer/input.c" || true
    for project in 05_CFG_Language_Pattern 06_CFG_AnBn_Pattern_Matching 07_LL1_Predictive_Parser \
                   08_Left_Recursion_Elimination 09_LR0_Parsing_Table; do
        run_section "$project"
        "$BIN/$project"
    done
    run_section 10_Configuration_Language_Evaluator
    "$BIN/10_Configuration_Language_Evaluator" "$ROOT/10_Configuration_Language_Evaluator/input.txt"
    run_section 11_Nested_Parentheses_Validator
    "$BIN/11_Nested_Parentheses_Validator" "$ROOT/11_Nested_Parentheses_Validator/input.txt" || true
    for project in 12_AST_Semantic_Analyzer 13_Three_Address_Code_Generator; do
        run_section "$project"
        "$BIN/$project"
    done
fi
