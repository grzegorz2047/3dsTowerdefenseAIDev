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
  "$ROOT/tests/audio_ndsp_shim_tests.cpp" \
  -o "$BUILD_DIR/audio-ndsp-shim-tests"

"$HOST_CXX" \
  "${COMMON_FLAGS[@]}" \
  "$ROOT/tests/audio_wave_status_tests.cpp" \
  -o "$BUILD_DIR/audio-wave-status-tests"

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
"$BUILD_DIR/audio-ndsp-shim-tests"
"$BUILD_DIR/audio-wave-status-tests"
"$BUILD_DIR/hud-text-tests"

# The fallback UI must expose both NDSP attempts and retain the manual tone.
grep -q "consoleInit(GFX_BOTTOM" "$ROOT/source/main.cpp"
grep -q "v0.1.16-alpha  NDSP-WBUF" "$ROOT/source/main.cpp"
grep -q "HLE-SHIM" "$ROOT/source/main.cpp"
grep -q "WBUF:%s POS:%lu/%lu" "$ROOT/source/main.cpp"
grep -q "KEY_B" "$ROOT/source/main.cpp"
grep -q "playDiagnosticTone" "$ROOT/source/main.cpp"

# The shim is permitted only for libctru's exact missing-component result and
# must contain no external firmware data.
grep -q "0xD8B0A7FAU" "$ROOT/include/AudioNdspShim.hpp"
grep -q "kSyntheticHleComponent" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspUseComponent" "$ROOT/source/AudioSystem.cpp"
grep -q "std::array<std::uint8_t, 0x400>" "$ROOT/source/AudioSystem.cpp"
if find "$ROOT" -type f \( -iname 'dspfirm.cdc' -o -iname '*.cdc' \) | grep -q .; then
  echo "Proprietary DSP firmware must not be committed" >&2
  exit 1
fi

# Diagnostic path must match the official devkitPro shape: channel 0, stereo
# PCM16, interleaved L/R and an observable wave-buffer status.
grep -q "kDiagnosticChannel = 0" "$ROOT/include/AudioSystem.hpp"
grep -q "NDSP_FORMAT_STEREO_PCM16" "$ROOT/source/AudioSystem.cpp"
grep -q "frame \* 2U + 1U" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspChnGetSamplePos" "$ROOT/source/AudioSystem.cpp"
grep -q "diagnosticWaveBuffer_.status" "$ROOT/source/AudioSystem.cpp"

grep -q "ndspSetOutputMode(NDSP_OUTPUT_STEREO)" "$ROOT/source/AudioSystem.cpp"
grep -q "csndPlaySound" "$ROOT/source/AudioSystem.cpp"

# CIA builds must be allowed to use both services. NDSP remains preferred,
# while CSND remains a hardware fallback when the synthetic component fails.
grep -q "dsp::DSP" "$ROOT/config/application.rsf"
grep -q "dsp: 0x0004013000001a02" "$ROOT/config/application.rsf"
grep -q "csnd:SND" "$ROOT/config/application.rsf"
grep -q "csnd: 0x0004013000001802" "$ROOT/config/application.rsf"
