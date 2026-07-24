#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected one match, found {count}: {old[:80]!r}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


main = ROOT / "source/main.cpp"
replace_once(main, '#include "Narrative.hpp"\n', '#include "Narrative.hpp"\n#include "MissionPause.hpp"\n')
replace_once(
    main,
    '    bool& paused, int& speedMultiplier) {\n    switch (action) {',
    '    bool& paused, int& speedMultiplier) {\n'
    '    if (!MissionPause::gameplayInputAllowed(paused) &&\n'
    '        action != TouchUiAction::TogglePause) {\n'
    '        return;\n'
    '    }\n'
    '    switch (action) {'
)
replace_once(
    main,
    '        case TouchUiAction::TogglePause:\n'
    '            if (tutorialFlow.waveRunning()) paused = !paused;\n'
    '            break;',
    '        case TouchUiAction::TogglePause:\n'
    '            if (tutorialFlow.waveRunning()) {\n'
    '                paused = MissionPause::toggled(paused, false, false);\n'
    '            }\n'
    '            break;'
)
replace_once(
    main,
    '        if (input.pressed(KEY_START)) {\n'
    '            action = benchmarkConfig != nullptr\n'
    '                ? MissionSessionAction::ReturnToCampaign\n'
    '                : MissionSessionAction::ExitApplication;\n'
    '            break;\n'
    '        }\n'
    '        const bool benchmarkFinished = benchmarkConfig != nullptr &&\n'
    '            benchmarkElapsed >= kBenchmarkDurationSeconds;\n'
    '        const bool sessionFinished = benchmarkFinished || tutorialFlow.finished();',
    '        const bool benchmarkFinished = benchmarkConfig != nullptr &&\n'
    '            benchmarkElapsed >= kBenchmarkDurationSeconds;\n'
    '        const bool sessionFinished = benchmarkFinished || tutorialFlow.finished();\n'
    '        if (input.pressed(KEY_START)) {\n'
    '            if (benchmarkConfig != nullptr) {\n'
    '                action = MissionSessionAction::ReturnToCampaign;\n'
    '                break;\n'
    '            }\n'
    '            paused = MissionPause::toggled(paused, sessionFinished, false);\n'
    '            simulationAccumulator = 0.0F;\n'
    '            previousMilliseconds = osGetTime();\n'
    '        }'
)
replace_once(
    main,
    '        if (sessionFinished) {\n'
    '            if (input.pressed(KEY_X) || touch == TouchUiAction::ResultCampaign) {\n'
    '                action = MissionSessionAction::ReturnToCampaign;\n'
    '                break;\n'
    '            }\n'
    '            if (input.pressed(KEY_Y) || touch == TouchUiAction::ResultReplay) {\n'
    '                action = MissionSessionAction::Replay;\n'
    '                break;\n'
    '            }\n'
    '        }',
    '        if (sessionFinished) {\n'
    '            if (input.pressed(KEY_X) || touch == TouchUiAction::ResultCampaign) {\n'
    '                action = MissionSessionAction::ReturnToCampaign;\n'
    '                break;\n'
    '            }\n'
    '            if (input.pressed(KEY_Y) || touch == TouchUiAction::ResultReplay) {\n'
    '                action = MissionSessionAction::Replay;\n'
    '                break;\n'
    '            }\n'
    '        } else if (paused) {\n'
    '            if (input.pressed(KEY_X)) {\n'
    '                action = MissionSessionAction::ReturnToCampaign;\n'
    '                break;\n'
    '            }\n'
    '            if (input.pressed(KEY_Y)) {\n'
    '                action = MissionSessionAction::Replay;\n'
    '                break;\n'
    '            }\n'
    '        }'
)
replace_once(
    main,
    '        if (!sessionFinished && benchmarkConfig == nullptr && input.pressed(KEY_X)) {',
    '        if (!sessionFinished && !paused && benchmarkConfig == nullptr && input.pressed(KEY_X)) {'
)
replace_once(
    main,
    '        if (!sessionFinished && input.pressed(KEY_L) && input.pressed(KEY_R)) {',
    '        if (!sessionFinished && !paused && input.pressed(KEY_L) && input.pressed(KEY_R)) {'
)
replace_once(
    main,
    '        if (!sessionFinished && benchmarkConfig == nullptr) {\n'
    '            buildSystem.handleInput(input);',
    '        if (!sessionFinished && !paused && benchmarkConfig == nullptr) {\n'
    '            buildSystem.handleInput(input);'
)
replace_once(
    main,
    '        if (!paused && !sessionFinished) {\n'
    '            simulationAccumulator = std::min(\n'
    '                simulationAccumulator + frameSeconds * speedMultiplier,\n'
    '                kMaximumAccumulatorSeconds);\n'
    '            if (benchmarkConfig != nullptr) benchmarkElapsed += frameSeconds;\n'
    '        }',
    '        if (paused) {\n'
    '            simulationAccumulator = 0.0F;\n'
    '        } else if (!sessionFinished) {\n'
    '            simulationAccumulator = std::min(\n'
    '                simulationAccumulator + MissionPause::simulationSeconds(\n'
    '                    frameSeconds, speedMultiplier, paused),\n'
    '                kMaximumAccumulatorSeconds);\n'
    '            if (benchmarkConfig != nullptr) benchmarkElapsed += frameSeconds;\n'
    '        }'
)

ui = ROOT / "source/UiRenderer.cpp"
replace_once(
    ui,
    '    const TouchRect pause = TouchUiLayout::rectFor(TouchUiAction::TogglePause);',
    '    if (state.paused) {\n'
    '        C2D_DrawRectSolid(8.0F, 124.0F, 0.2F, 304.0F, 108.0F, kPanelStrong);\n'
    '        drawText("PAUZA", 118.0F, 134.0F, 0.62F, kGold);\n'
    '        drawText("START  WZNOW", 91.0F, 164.0F, 0.44F, kText);\n'
    '        drawText("X  KAMPANIA    Y  POWTORZ", 48.0F, 187.0F, 0.40F, kText);\n'
    '        drawText("Kamera i 3D pozostaja aktywne", 43.0F, 211.0F, 0.40F, kMuted);\n'
    '        return;\n'
    '    }\n\n'
    '    const TouchRect pause = TouchUiLayout::rectFor(TouchUiAction::TogglePause);'
)

header = ROOT / "include/MissionPause.hpp"
header.write_text('''#pragma once\n\nnamespace MissionPause {\n\nconstexpr bool toggled(bool paused, bool sessionFinished, bool benchmarkMode) {\n    return (!sessionFinished && !benchmarkMode) ? !paused : paused;\n}\n\nconstexpr bool gameplayInputAllowed(bool paused) {\n    return !paused;\n}\n\nconstexpr float simulationSeconds(float frameSeconds, int speedMultiplier, bool paused) {\n    if (paused || frameSeconds <= 0.0F || speedMultiplier <= 0) return 0.0F;\n    return frameSeconds * static_cast<float>(speedMultiplier);\n}\n\n}  // namespace MissionPause\n''', encoding="utf-8")

test = ROOT / "tests/mission_pause_tests.cpp"
test.write_text('''#include <cassert>\n\n#include "MissionPause.hpp"\n\nint main() {\n    assert(MissionPause::toggled(false, false, false));\n    assert(!MissionPause::toggled(true, false, false));\n    assert(!MissionPause::toggled(false, true, false));\n    assert(!MissionPause::toggled(false, false, true));\n\n    assert(MissionPause::gameplayInputAllowed(false));\n    assert(!MissionPause::gameplayInputAllowed(true));\n\n    assert(MissionPause::simulationSeconds(1.0F / 60.0F, 1, false) == 1.0F / 60.0F);\n    assert(MissionPause::simulationSeconds(1.0F / 60.0F, 2, false) == 2.0F / 60.0F);\n    assert(MissionPause::simulationSeconds(10.0F, 2, true) == 0.0F);\n    assert(MissionPause::simulationSeconds(-1.0F, 2, false) == 0.0F);\n    assert(MissionPause::simulationSeconds(1.0F, 0, false) == 0.0F);\n    return 0;\n}\n''', encoding="utf-8")

host = ROOT / "scripts/run_host_tests.sh"
replace_once(
    host,
    '"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/seven_segment_digits_tests.cpp" -o "$BUILD_DIR/seven-segment-digits-tests"\n',
    '"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/seven_segment_digits_tests.cpp" -o "$BUILD_DIR/seven-segment-digits-tests"\n'
    '"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/mission_pause_tests.cpp" -o "$BUILD_DIR/mission-pause-tests"\n'
)
replace_once(
    host,
    '"$BUILD_DIR/seven-segment-digits-tests"\n',
    '"$BUILD_DIR/seven-segment-digits-tests"\n'
    '"$BUILD_DIR/mission-pause-tests"\n'
)
replace_once(
    host,
    'grep -q "int speedMultiplier = 1;" "$ROOT/source/main.cpp"\n',
    'grep -q "int speedMultiplier = 1;" "$ROOT/source/main.cpp"\n'
    'grep -q "MissionPause::toggled" "$ROOT/source/main.cpp"\n'
    'grep -q "MissionPause::gameplayInputAllowed" "$ROOT/source/main.cpp"\n'
    'grep -q "START  WZNOW" "$ROOT/source/UiRenderer.cpp"\n'
)

# Remove the one-shot transformer and its workflow before committing the product patch.
(ROOT / "scripts/apply_issue_185.py").unlink()
workflow = ROOT / ".github/workflows/apply-issue-185.yml"
if workflow.exists():
    workflow.unlink()
