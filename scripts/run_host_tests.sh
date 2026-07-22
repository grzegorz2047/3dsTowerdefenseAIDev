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

"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/logic_tests.cpp" "$ROOT/source/Economy.cpp" "$ROOT/source/Enemy.cpp" "$ROOT/source/Level.cpp" "$ROOT/source/Projectile.cpp" "$ROOT/source/Tower.cpp" "$ROOT/source/Wave.cpp" -o "$BUILD_DIR/gameplay-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/tower_archetype_tests.cpp" "$ROOT/source/Enemy.cpp" "$ROOT/source/Level.cpp" "$ROOT/source/Projectile.cpp" "$ROOT/source/Tower.cpp" "$ROOT/source/Wave.cpp" -o "$BUILD_DIR/tower-archetype-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/tower_economy_tests.cpp" "$ROOT/source/Economy.cpp" "$ROOT/source/Enemy.cpp" "$ROOT/source/Level.cpp" "$ROOT/source/Projectile.cpp" "$ROOT/source/Tower.cpp" "$ROOT/source/Wave.cpp" -o "$BUILD_DIR/tower-economy-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/touch_gesture_tests.cpp" "$ROOT/source/TouchGesture.cpp" -o "$BUILD_DIR/touch-gesture-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/touch_ui_layout_tests.cpp" "$ROOT/source/TouchUiLayout.cpp" -o "$BUILD_DIR/touch-ui-layout-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/tutorial_flow_tests.cpp" "$ROOT/source/BuildFeedback.cpp" "$ROOT/source/TutorialFlow.cpp" -o "$BUILD_DIR/tutorial-flow-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/campaign_tests.cpp" "$ROOT/source/Campaign.cpp" -o "$BUILD_DIR/campaign-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/campaign_level_files_tests.cpp" "$ROOT/source/Campaign.cpp" "$ROOT/source/Level.cpp" -o "$BUILD_DIR/campaign-level-files-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/save_data_tests.cpp" "$ROOT/source/Campaign.cpp" "$ROOT/source/SaveData.cpp" "$ROOT/source/Stereo3D.cpp" -o "$BUILD_DIR/save-data-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/stereo_3d_tests.cpp" "$ROOT/source/Stereo3D.cpp" -o "$BUILD_DIR/stereo-3d-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_events_tests.cpp" "$ROOT/source/AudioEvents.cpp" -o "$BUILD_DIR/audio-events-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_backend_tests.cpp" -o "$BUILD_DIR/audio-backend-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_probe_tests.cpp" -o "$BUILD_DIR/audio-probe-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_ndsp_shim_tests.cpp" -o "$BUILD_DIR/audio-ndsp-shim-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_wave_status_tests.cpp" -o "$BUILD_DIR/audio-wave-status-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/hud_mode_tests.cpp" -o "$BUILD_DIR/hud-mode-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/hud_text_tests.cpp" "$ROOT/source/HudText.cpp" -o "$BUILD_DIR/hud-text-tests"

"$BUILD_DIR/gameplay-tests"
"$BUILD_DIR/tower-archetype-tests"
"$BUILD_DIR/tower-economy-tests"
"$BUILD_DIR/touch-gesture-tests"
"$BUILD_DIR/touch-ui-layout-tests"
"$BUILD_DIR/tutorial-flow-tests"
"$BUILD_DIR/campaign-tests"
"$BUILD_DIR/campaign-level-files-tests" "$ROOT"
"$BUILD_DIR/save-data-tests" "$ROOT"
"$BUILD_DIR/stereo-3d-tests"
"$BUILD_DIR/audio-events-tests"
"$BUILD_DIR/audio-backend-tests"
"$BUILD_DIR/audio-probe-tests"
"$BUILD_DIR/audio-ndsp-shim-tests"
"$BUILD_DIR/audio-wave-status-tests"
"$BUILD_DIR/hud-mode-tests"
"$BUILD_DIR/hud-text-tests"

grep -q "consoleInit(GFX_BOTTOM" "$ROOT/source/main.cpp"
grep -q "TouchGesture touchGesture" "$ROOT/source/main.cpp"
grep -q "TouchUiLayout::actionAt" "$ROOT/source/main.cpp"
grep -q "SelectBallista" "$ROOT/source/main.cpp"
grep -q "TogglePause" "$ROOT/source/main.cpp"
grep -q "ToggleSpeed" "$ROOT/source/main.cpp"
grep -q "input.pressed(KEY_L) && input.pressed(KEY_R)" "$ROOT/source/main.cpp"
grep -q "hudModeForSelectHeld" "$ROOT/source/main.cpp"
grep -q "allowDiagnosticTone" "$ROOT/source/main.cpp"
grep -q "renderAudioDiagnostics" "$ROOT/source/main.cpp"
grep -q "playDiagnosticTone" "$ROOT/source/main.cpp"

# Stereoscopic top-screen contract: physical slider, two eye targets, asymmetric
# projections, settings controls, runtime diagnostics, and a one-eye mono fallback.
grep -q "gfxSet3D(true)" "$ROOT/source/Renderer.cpp"
grep -q "osGet3DSliderState" "$ROOT/source/Renderer.cpp"
grep -q "GFX_LEFT" "$ROOT/source/Renderer.cpp"
grep -q "GFX_RIGHT" "$ROOT/source/Renderer.cpp"
grep -q "Mtx_PerspStereoTilt" "$ROOT/source/Camera.cpp"
grep -q "lastStereoPlan_.stereo" "$ROOT/source/Renderer.cpp"
grep -q "drawScene(topRightTarget_" "$ROOT/source/Renderer.cpp"
grep -q "Stereo3D::nextDepthLimit" "$ROOT/source/main.cpp"
grep -q "input.pressed(KEY_L)" "$ROOT/source/main.cpp"
grep -q "input.pressed(KEY_R)" "$ROOT/source/main.cpp"
grep -q "renderStereoDiagnostics" "$ROOT/source/main.cpp"
grep -q "lastEyeCount" "$ROOT/source/main.cpp"
grep -q "lastStereoSlider" "$ROOT/source/main.cpp"
grep -q "lastStereoSeparation" "$ROOT/source/main.cpp"

# Model-composition contract: gameplay entities must remain recognizable
# multi-part geometry, not regress to one generic placeholder box.
grep -q "void appendEnemy" "$ROOT/source/Renderer.cpp"
grep -q "void appendTower" "$ROOT/source/Renderer.cpp"
grep -q "void appendProjectile" "$ROOT/source/Renderer.cpp"
grep -q "appendEnemy(vertices)" "$ROOT/source/Renderer.cpp"
grep -q "appendTower(vertices)" "$ROOT/source/Renderer.cpp"
grep -q "appendProjectile(vertices)" "$ROOT/source/Renderer.cpp"
enemy_parts=$(sed -n '/void appendEnemy/,/^}/p' "$ROOT/source/Renderer.cpp" | grep -c "appendBoxAt")
tower_parts=$(sed -n '/void appendTower/,/^}/p' "$ROOT/source/Renderer.cpp" | grep -c "appendBoxAt")
projectile_parts=$(sed -n '/void appendProjectile/,/^}/p' "$ROOT/source/Renderer.cpp" | grep -c "appendBoxAt")
test "$enemy_parts" -ge 8
test "$tower_parts" -ge 11
test "$projectile_parts" -ge 2

grep -q "0xD880A7FAU" "$ROOT/include/AudioNdspShim.hpp"
grep -q "resultSummary(result) == kResultSummaryNotFound" "$ROOT/include/AudioNdspShim.hpp"
grep -q "resultModule(result) == kResultModuleDsp" "$ROOT/include/AudioNdspShim.hpp"
grep -q "resultDescription(result) == kResultDescriptionNotFound" "$ROOT/include/AudioNdspShim.hpp"
grep -q "kSyntheticHleComponent" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspUseComponent" "$ROOT/source/AudioSystem.cpp"
grep -q "std::array<std::uint8_t, 0x400>" "$ROOT/source/AudioSystem.cpp"
if find "$ROOT" -type f \( -iname 'dspfirm.cdc' -o -iname '*.cdc' \) | grep -q .; then
  echo "Proprietary DSP firmware must not be committed" >&2
  exit 1
fi

grep -q "kDiagnosticChannel = 0" "$ROOT/include/AudioSystem.hpp"
grep -q "NDSP_FORMAT_STEREO_PCM16" "$ROOT/source/AudioSystem.cpp"
grep -q "frame \* 2U + 1U" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspChnGetSamplePos" "$ROOT/source/AudioSystem.cpp"
grep -q "diagnosticWaveBuffer_.status" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspSetOutputMode(NDSP_OUTPUT_STEREO)" "$ROOT/source/AudioSystem.cpp"
grep -q "csndPlaySound" "$ROOT/source/AudioSystem.cpp"
grep -q "dsp::DSP" "$ROOT/config/application.rsf"
grep -q "dsp: 0x0004013000001a02" "$ROOT/config/application.rsf"
grep -q "csnd:SND" "$ROOT/config/application.rsf"
grep -q "csnd: 0x0004013000001802" "$ROOT/config/application.rsf"
