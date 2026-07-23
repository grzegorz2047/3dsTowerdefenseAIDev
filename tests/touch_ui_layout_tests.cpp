#include <cstdlib>
#include <iostream>

#include "TouchUiLayout.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void expectCenter(TouchUiAction action) {
    const TouchRect rect = TouchUiLayout::rectFor(action);
    expect(rect.height >= TouchUiLayout::kMinimumButtonHeight, "button should meet minimum touch height");
    expect(
        TouchUiLayout::actionAt(
            static_cast<std::int16_t>(rect.x + rect.width / 2),
            static_cast<std::int16_t>(rect.y + rect.height / 2)) == action,
        "button center should map to its action");
}

void testAllActionsHaveUsableHitboxes() {
    const TouchUiAction actions[] = {
        TouchUiAction::SelectBallista,
        TouchUiAction::SelectMortar,
        TouchUiAction::SelectFrost,
        TouchUiAction::SelectRocket,
        TouchUiAction::BuildOrSelect,
        TouchUiAction::Upgrade,
        TouchUiAction::Sell,
        TouchUiAction::StartWave,
        TouchUiAction::TogglePause,
        TouchUiAction::ToggleSpeed,
        TouchUiAction::Cancel,
    };
    for (const TouchUiAction action : actions) expectCenter(action);
}

void testEdgesAreHalfOpen() {
    const TouchRect rect = TouchUiLayout::rectFor(TouchUiAction::SelectBallista);
    expect(TouchUiLayout::actionAt(rect.x, rect.y) == TouchUiAction::SelectBallista, "top-left edge should be active");
    expect(TouchUiLayout::actionAt(static_cast<std::int16_t>(rect.x + rect.width), rect.y) != TouchUiAction::SelectBallista,
        "right edge should belong outside the button");
    expect(TouchUiLayout::actionAt(rect.x, static_cast<std::int16_t>(rect.y + rect.height)) != TouchUiAction::SelectBallista,
        "bottom edge should belong outside the button");
}

void testGapsAndOutsideScreenDoNothing() {
    expect(TouchUiLayout::actionAt(82, 20) == TouchUiAction::None, "gap between tower buttons should be inert");
    expect(TouchUiLayout::actionAt(20, 100) == TouchUiAction::None, "central information area should be inert");
    expect(TouchUiLayout::actionAt(168, 145) == TouchUiAction::None, "gap beside speed button should be inert");
    expect(TouchUiLayout::actionAt(-1, 20) == TouchUiAction::None, "negative coordinates should be ignored");
    expect(TouchUiLayout::actionAt(320, 20) == TouchUiAction::None, "coordinates beyond screen width should be ignored");
    expect(TouchUiLayout::actionAt(20, 240) == TouchUiAction::None, "coordinates beyond screen height should be ignored");
}

}  // namespace

int main() {
    testAllActionsHaveUsableHitboxes();
    testEdgesAreHalfOpen();
    testGapsAndOutsideScreenDoNothing();
    std::cout << "Touch UI layout tests passed\n";
    return 0;
}
