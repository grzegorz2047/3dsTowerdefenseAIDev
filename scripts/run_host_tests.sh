#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build/host-tests"
HOST_CXX="${HOST_CXX:-g++}"

mkdir -p "$BUILD_DIR"

COMMON_FLAGS=(
  -std=c++17
  -O2
  -Wall
  -Wextra
  -Werror
  -I"$ROOT/include"
)

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/logic_tests.cpp" \
  "$ROOT/source/Economy.cpp" \
  "$ROOT/source/Enemy.cpp" \
  "$ROOT/source/Level.cpp" \
  "$ROOT/source/Projectile.cpp" \
  "$ROOT/source/Tower.cpp" \
  "$ROOT/source/Wave.cpp" \
  -o "$BUILD_DIR/gameplay-tests"

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/tutorial_flow_tests.cpp" \
  "$ROOT/source/BuildFeedback.cpp" \
  "$ROOT/source/TutorialFlow.cpp" \
  -o "$BUILD_DIR/tutorial-flow-tests"

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/audio_events_tests.cpp" \
  "$ROOT/source/AudioEvents.cpp" \
  -o "$BUILD_DIR/audio-events-tests"

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/hud_text_tests.cpp" \
  "$ROOT/source/HudText.cpp" \
  -o "$BUILD_DIR/hud-text-tests"

"$BUILD_DIR/gameplay-tests"
"$BUILD_DIR/tutorial-flow-tests"
"$BUILD_DIR/audio-events-tests"
"$BUILD_DIR/hud-text-tests"

# The fallback UI must stay independent from the custom GPU pipeline and expose
# enough diagnostics to distinguish a DSP permission failure from muted output.
grep -q "consoleInit(GFX_BOTTOM" "$ROOT/source/main.cpp"
grep -q "v0.1.11-alpha  AUDIO-DSP" "$ROOT/source/main.cpp"
grep -q "audioStatusMessage(audioAvailable)" "$ROOT/source/main.cpp"
grep -q "dsp::DSP" "$ROOT/config/application.rsf"
grep -q "dsp: 0x0004013000001a02" "$ROOT/config/application.rsf"
