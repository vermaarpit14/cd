#!/usr/bin/env bash
set -euo pipefail
for dir in */; do
  if [[ -f "${dir}main.cpp" ]]; then
    echo "Building ${dir%/}"
    (cd "$dir" && g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o program)
  fi
done
echo "All C++ programs built. Flex/Bison programs have separate build commands in their README files."
