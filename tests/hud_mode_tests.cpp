#include <cstdlib>
#include <iostream>

#include "HudMode.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    HudModeController controller;
    expect(controller.mode() == HudMode::Gameplay, "HUD must start in gameplay mode");
    expect(controller.update(false) == HudMode::Gameplay, "released SELECT must preserve gameplay HUD");
    expect(controller.update(true) == HudMode::AudioDiagnostics, "first SELECT press must enable diagnostics");
    expect(controller.update(true) == HudMode::AudioDiagnostics, "holding SELECT must not toggle repeatedly");
    expect(controller.update(false) == HudMode::AudioDiagnostics, "releasing SELECT must preserve diagnostics");
    expect(controller.update(true) == HudMode::Gameplay, "second SELECT press must return to gameplay HUD");

    controller.reset();
    expect(controller.mode() == HudMode::Gameplay, "reset must restore gameplay HUD");
    expect(!showAudioDiagnostics(HudMode::Gameplay), "gameplay HUD must hide raw diagnostics");
    expect(showAudioDiagnostics(HudMode::AudioDiagnostics), "diagnostic HUD must expose raw diagnostics");
    expect(!allowDiagnosticTone(HudMode::Gameplay, true), "B must not play the test tone in gameplay mode");
    expect(allowDiagnosticTone(HudMode::AudioDiagnostics, true), "diagnostic mode plus B must play the tone");

    std::cout << "HUD mode tests passed\n";
    return 0;
}
