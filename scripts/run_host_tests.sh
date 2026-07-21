#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT="$ROOT/build/host-tests/gameplay-tests"
HOST_CXX="${HOST_CXX:-g++}"

mkdir -p "$(dirname "$OUTPUT")"

"$HOST_CXX" \
  -std=c++17 \
  -O2 \
  -Wall \
  -Wextra \
  -Werror \
  -I"$ROOT/include" \
  "$ROOT/tests/logic_tests.cpp" \
  "$ROOT/source/Enemy.cpp" \
  "$ROOT/source/Level.cpp" \
  "$ROOT/source/Tower.cpp" \
  "$ROOT/source/Wave.cpp" \
  -o "$OUTPUT"

"$OUTPUT"
