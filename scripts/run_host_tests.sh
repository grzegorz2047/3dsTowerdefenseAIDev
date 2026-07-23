#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build/host-tests"
HOST_CXX="${HOST_CXX:-g++}"

: > "$ROOT/build.log"
exec > >(tee -a "$ROOT/build.log") 2>&1
trap 'status=$?; echo "HOST TEST FAILURE ($status): $BASH_COMMAND"; exit $status' ERR

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
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_channel_pool_tests.cpp" -o "$BUILD_DIR/audio-channel-pool-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_backend_tests.cpp" -o "$BUILD_DIR/audio-backend-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_probe_tests.cpp" -o "$BUILD_DIR/audio-probe-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_ndsp_shim_tests.cpp" -o "$BUILD_DIR/audio-ndsp-shim-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/audio_wave_status_tests.cpp" -o "$BUILD_DIR/audio-wave-status-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/hud_mode_tests.cpp" -o "$BUILD_DIR/hud-mode-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/hud_text_tests.cpp" "$ROOT/source/HudText.cpp" -o "$BUILD_DIR/hud-text-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/performance_budget_tests.cpp" -o "$BUILD_DIR/performance-budget-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/performance_stress_level_tests.cpp" "$ROOT/source/Level.cpp" -o "$BUILD_DIR/performance-stress-level-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/benchmark_config_tests.cpp" "$ROOT/source/Level.cpp" -o "$BUILD_DIR/benchmark-config-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/orbit_camera_tests.cpp" "$ROOT/source/OrbitCamera.cpp" -o "$BUILD_DIR/orbit-camera-tests"
"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/seven_segment_digits_tests.cpp" -o "$BUILD_DIR/seven-segment-digits-tests"

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
"$BUILD_DIR/audio-channel-pool-tests"
"$BUILD_DIR/audio-backend-tests"
"$BUILD_DIR/audio-probe-tests"
"$BUILD_DIR/audio-ndsp-shim-tests"
"$BUILD_DIR/audio-wave-status-tests"
"$BUILD_DIR/hud-mode-tests"
"$BUILD_DIR/hud-text-tests"
"$BUILD_DIR/performance-budget-tests"
"$BUILD_DIR/performance-stress-level-tests" "$ROOT"
"$BUILD_DIR/benchmark-config-tests"
"$BUILD_DIR/orbit-camera-tests"
"$BUILD_DIR/seven-segment-digits-tests"

# One screen, one owner. PrintConsole is allowed only on the top startup-error path.
if grep -R "consoleInit(GFX_BOTTOM" "$ROOT/source" "$ROOT/include"; then
  echo "GFX_BOTTOM must be owned exclusively by Citro2D" >&2
  exit 1
fi
grep -q "consoleInit(GFX_TOP" "$ROOT/source/main.cpp"
grep -q "C2D_Init(C2D_DEFAULT_MAX_OBJECTS)" "$ROOT/source/main.cpp"
grep -q "C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT)" "$ROOT/source/UiRenderer.cpp"
grep -q "C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT)" "$ROOT/source/UiRenderer.cpp"
grep -q "gfxSet3D(false)" "$ROOT/source/UiRenderer.cpp"
grep -q "C2D_Fini();" "$ROOT/source/main.cpp"

# No terminal interception, renderer macros, or CPU writes into GPU-owned framebuffers.
test ! -e "$ROOT/include/MainOverrides.hpp"
test ! -e "$ROOT/source/ConsoleCompat.cpp"
test ! -e "$ROOT/source/TopGoldOverlay.cpp"
if grep -R "gfxGetFramebuffer" "$ROOT/source" "$ROOT/include"; then
  echo "HUD must not write directly to a framebuffer" >&2
  exit 1
fi
if grep -R "#define render\|#define printf\|#define C3D_Init\|#define gfxSwapBuffers" "$ROOT/source" "$ROOT/include"; then
  echo "Render lifecycle must not be replaced with macros" >&2
  exit 1
fi

# Each active path owns exactly one begin/end pair. Standalone rendering composes both screens.
test "$(grep -c "C3D_FrameBegin" "$ROOT/source/Renderer.cpp")" -eq 1
test "$(grep -c "C3D_FrameEnd" "$ROOT/source/Renderer.cpp")" -eq 1
test "$(grep -c "C3D_FrameBegin" "$ROOT/source/UiRenderer.cpp")" -eq 1
test "$(grep -c "C3D_FrameEnd" "$ROOT/source/UiRenderer.cpp")" -eq 1
grep -q "drawStandaloneTop(state)" "$ROOT/source/UiRenderer.cpp"
grep -q "uiRenderer.renderTopOverlay(topLeftTarget_" "$ROOT/source/Renderer.cpp"
grep -q "uiRenderer.renderTopOverlay(topRightTarget_" "$ROOT/source/Renderer.cpp"
grep -q "uiRenderer.renderBottom(uiState)" "$ROOT/source/Renderer.cpp"
test "$(grep -c "C2D_Flush();" "$ROOT/source/UiRenderer.cpp")" -eq 3
test "$(grep -c "C2D_Prepare();" "$ROOT/source/UiRenderer.cpp")" -eq 3

# Typed UI state and physical-device layout contracts.
grep -q "struct UiState" "$ROOT/include/UiState.hpp"
grep -q "UiScreen screen" "$ROOT/include/UiState.hpp"
grep -q "UiRenderer& uiRenderer" "$ROOT/include/Renderer.hpp"
grep -q "missionUiState" "$ROOT/source/main.cpp"
grep -q "campaignUiState" "$ROOT/source/main.cpp"
grep -q "drawWrappedText" "$ROOT/source/UiRenderer.cpp"
grep -q "kMinimumReadableScale = 0.40F" "$ROOT/source/UiRenderer.cpp"
grep -q "std::max(scale, kMinimumReadableScale)" "$ROOT/source/UiRenderer.cpp"
grep -q "drawSegmentNumber(state.gold" "$ROOT/source/UiRenderer.cpp"
grep -q "SevenSegmentDigits::maskFor" "$ROOT/source/UiRenderer.cpp"
grep -q '"X KAMPANIA"' "$ROOT/source/UiRenderer.cpp"
grep -q '"Y POWTORZ"' "$ROOT/source/UiRenderer.cpp"
if grep -q '"L 3D ON"\|"L 3D OFF"' "$ROOT/source/UiRenderer.cpp"; then
  echo "Physical 3D slider must not be duplicated as a campaign button" >&2
  exit 1
fi

# Visible mission controls must be sourced from the same rectangles as touch input.
grep -q "TouchUiLayout::rectFor(TouchUiAction::SelectBallista)" "$ROOT/source/UiRenderer.cpp"
grep -q "TouchUiLayout::rectFor(TouchUiAction::BuildOrSelect)" "$ROOT/source/UiRenderer.cpp"
grep -q "TouchUiLayout::actionAt" "$ROOT/source/main.cpp"

# Stereoscopic top-screen contract.
grep -q "gfxSet3D(true)" "$ROOT/source/Renderer.cpp"
grep -q "osGet3DSliderState" "$ROOT/source/Renderer.cpp"
grep -q "GFX_LEFT" "$ROOT/source/Renderer.cpp"
grep -q "GFX_RIGHT" "$ROOT/source/Renderer.cpp"
grep -q "Mtx_PerspStereoTilt" "$ROOT/source/Camera.cpp"
grep -q "lastStereoPlan_.stereo" "$ROOT/source/Renderer.cpp"
grep -q "drawScene(topRightTarget_" "$ROOT/source/Renderer.cpp"
grep -q "Stereo3D::nextDepthLimit" "$ROOT/source/main.cpp"
if grep -q "settings.stereoEnabled" "$ROOT/source/main.cpp"; then
  echo "Physical 3D slider must be the only stereo switch" >&2
  exit 1
fi
grep -q "BenchmarkProfiles::makeLevel" "$ROOT/source/main.cpp"
grep -q "configureBenchmark" "$ROOT/source/main.cpp"

# Physical-device regression and runtime diagnostics contracts.
grep -q "int speedMultiplier = 1;" "$ROOT/source/main.cpp"
grep -q "OrbitCamera orbit_" "$ROOT/include/Camera.hpp"
grep -q "kDefaultPitch = 0.50F" "$ROOT/include/OrbitCamera.hpp"
grep -q "kDefaultDistance = 13.25F" "$ROOT/include/OrbitCamera.hpp"
grep -q "kCameraHeight = -3.8F" "$ROOT/source/Camera.cpp"
grep -q "orbit_.update" "$ROOT/source/Camera.cpp"
grep -q "Mtx_RotateY(&destination, orbit_.yaw()" "$ROOT/source/Camera.cpp"
grep -q "spawnedCount_ = 0U;" "$ROOT/source/Wave.cpp"
grep -q "C3D_RenderTargetDelete(topLeftTarget_)" "$ROOT/source/Renderer.cpp"
grep -q "C3D_RenderTargetDelete(topRightTarget_)" "$ROOT/source/Renderer.cpp"
grep -q "PerformanceSampler performanceSampler" "$ROOT/source/main.cpp"
grep -q "linearSpaceFree()" "$ROOT/source/main.cpp"
grep -q "drawDiagnostics" "$ROOT/source/UiRenderer.cpp"

# Original Nintendo 3DS XL performance budgets.
grep -q "kTargetFrameMilliseconds = 33.333F" "$ROOT/include/PerformanceBudget.hpp"
grep -q "kMinimumLinearMemoryReserveBytes = 512U \* 1024U" "$ROOT/include/PerformanceBudget.hpp"
grep -q "kMaximumEnemies = 16U" "$ROOT/include/PerformanceBudget.hpp"
grep -q "kMaximumTowers = 16U" "$ROOT/include/PerformanceBudget.hpp"
grep -q "kMaximumProjectiles = 32U" "$ROOT/include/PerformanceBudget.hpp"
grep -q "kMaximumLevelVertices = 4096U" "$ROOT/include/PerformanceBudget.hpp"
grep -q "id=performance_stress" "$ROOT/romfs/levels/performance_stress.lvl"

# Scene composition remains explicit and bounded.
grep -q "void appendEnemy" "$ROOT/source/Renderer.cpp"
grep -q "void appendTower" "$ROOT/source/Renderer.cpp"
grep -q "void appendProjectile" "$ROOT/source/Renderer.cpp"
grep -q "appendEnemy(vertices)" "$ROOT/source/Renderer.cpp"
grep -q "appendTower(vertices)" "$ROOT/source/Renderer.cpp"
grep -q "appendProjectile(vertices)" "$ROOT/source/Renderer.cpp"

# Audio platform, backend fallback, phase music and protected channel contracts.
grep -q "0xD880A7FAU" "$ROOT/include/AudioNdspShim.hpp"
grep -q "resultSummary(result) == kResultSummaryNotFound" "$ROOT/include/AudioNdspShim.hpp"
grep -q "resultModule(result) == kResultModuleDsp" "$ROOT/include/AudioNdspShim.hpp"
grep -q "resultDescription(result) == kResultDescriptionNotFound" "$ROOT/include/AudioNdspShim.hpp"
if grep -q "ndspUseComponent(" "$ROOT/source/AudioSystem.cpp"; then
  echo "Runtime must not inject a synthetic NDSP component" >&2
  exit 1
fi
if grep -q "std::array<std::uint8_t, 0x400>" "$ROOT/source/AudioSystem.cpp"; then
  echo "Runtime must not carry a zero-filled fake DSP component" >&2
  exit 1
fi
if find "$ROOT" -type f \( -iname 'dspfirm.cdc' -o -iname '*.cdc' \) | grep -q .; then
  echo "Proprietary DSP firmware must not be committed" >&2
  exit 1
fi

grep -q "kDiagnosticChannel = 0" "$ROOT/include/AudioSystem.hpp"
grep -q "kPreparationMusicChannel = 7" "$ROOT/include/AudioSystem.hpp"
grep -q "kCombatMusicChannel = 8" "$ROOT/include/AudioSystem.hpp"
grep -q "kAmbientChannel = 9" "$ROOT/include/AudioSystem.hpp"
grep -q "generateMusicLayers" "$ROOT/source/AudioSystem.cpp"
grep -q "generateAmbientLayer" "$ROOT/source/AudioSystem.cpp"
grep -q "approachMusicGain" "$ROOT/source/AudioSystem.cpp"
grep -q "audioSystem.updateMusic" "$ROOT/source/main.cpp"
grep -q "settings.musicEnabled" "$ROOT/source/main.cpp"
grep -q "AudioChannelPool::slotFor" "$ROOT/source/AudioSystem.cpp"
grep -q "poolCursors_" "$ROOT/include/AudioSystem.hpp"
grep -q "kPoolStarts{0U, 3U, 5U}" "$ROOT/include/AudioChannelPool.hpp"
grep -q "kPoolSizes{3U, 2U, 1U}" "$ROOT/include/AudioChannelPool.hpp"
grep -q "audioCooldownSeconds" "$ROOT/include/AudioPolicy.hpp"
grep -q "NDSP_FORMAT_STEREO_PCM16" "$ROOT/source/AudioSystem.cpp"
grep -q "frame \* 2U + 1U" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspChnGetSamplePos" "$ROOT/source/AudioSystem.cpp"
grep -q "diagnosticWaveBuffer_.status" "$ROOT/source/AudioSystem.cpp"
grep -q "ndspSetOutputMode(NDSP_OUTPUT_STEREO)" "$ROOT/source/AudioSystem.cpp"
grep -q "csndInit()" "$ROOT/source/AudioSystem.cpp"
grep -q "SOUND_ENABLE" "$ROOT/source/AudioSystem.cpp"
grep -q "SOUND_REPEAT" "$ROOT/source/AudioSystem.cpp"
grep -q "CSND_SetVol" "$ROOT/source/AudioSystem.cpp"
grep -q "csndExecCmds(true)" "$ROOT/source/AudioSystem.cpp"
grep -q "csndPlaySound" "$ROOT/source/AudioSystem.cpp"
grep -q "dsp::DSP" "$ROOT/config/application.rsf"
grep -q "dsp: 0x0004013000001a02" "$ROOT/config/application.rsf"
grep -q "csnd:SND" "$ROOT/config/application.rsf"
grep -q "csnd: 0x0004013000001802" "$ROOT/config/application.rsf"
