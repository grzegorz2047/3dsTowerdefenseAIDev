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
  "$ROOT/tests/audio_backend_tests.cpp" \
  -o "$BUILD_DIR/audio-backend-tests"

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/audio_probe_tests.cpp" \
  -o "$BUILD_DIR/audio-probe-tests"

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/hud_text_tests.cpp" \
  "$ROOT/source/HudText.cpp" \
  -o "$BUILD_DIR/hud-text-tests"

"$BUILD_DIR/gameplay-tests"
"$BUILD_DIR/tutorial-flow-tests"
"$BUILD_DIR/audio-events-tests"
"$BUILD_DIR/audio-backend-tests"
"$BUILD_DIR/audio-probe-tests"
"$BUILD_DIR/hud-text-tests"

# The fallback UI must expose the active backend and raw service results.
grep -q "consoleInit(GFX_BOTTOM" "$ROOT/source/main.cpp"
grep -q "v0.1.13-alpha  AUDIO-PROBE" "$ROOT/source/main.cpp"
grep -q "KEY_B" "$ROOT/source/main.cpp"
grep -q "playDiagnosticTone" "$ROOT/source/main.cpp"
grep -q "channelEverActive" "$ROOT/source/main.cpp"
grep -q "csndGetState" "$ROOT/source/AudioSystem.cpp"
grep -q "kDurationSeconds = 2.0F" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspSetOutputMode(NDSP_OUTPUT_STEREO)" "$ROOT/source/AudioSystem.cpp"
grep -q "csndPlaySound" "$ROOT/source/AudioSystem.cpp"

# CIA builds must be allowed to use both services. NDSP remains preferred,
# while CSND provides an emulator and firmware compatibility fallback.
grep -q "dsp::DSP" "$ROOT/config/application.rsf"
grep -q "dsp: 0x0004013000001a02" "$ROOT/config/application.rsf"
grep -q "csnd:SND" "$ROOT/config/application.rsf"
grep -q "csnd: 0x0004013000001802" "$ROOT/config/application.rsf"
