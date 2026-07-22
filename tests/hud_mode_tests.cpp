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
    HudMode mode = HudMode::Gameplay;
    expect(!showAudioDiagnostics(mode), "gameplay HUD must hide raw audio diagnostics");

    mode = toggleHudMode(mode, false);
    expect(mode == HudMode::Gameplay, "no SELECT press must preserve gameplay HUD");

    mode = toggleHudMode(mode, true);
    expect(mode == HudMode::AudioDiagnostics, "first SELECT press must enable diagnostics");
    expect(showAudioDiagnostics(mode), "diagnostic HUD must expose raw audio diagnostics");

    mode = toggleHudMode(mode, false);
    expect(mode == HudMode::AudioDiagnostics, "released SELECT must preserve diagnostic mode");

    expect(!allowDiagnosticTone(HudMode::Gameplay, true), "B must not play the test tone in normal gameplay");
    expect(!allowDiagnosticTone(HudMode::AudioDiagnostics, false), "diagnostic tone requires B press");
    expect(allowDiagnosticTone(HudMode::AudioDiagnostics, true), "diagnostic mode plus B must play the tone");

    mode = toggleHudMode(mode, true);
    expect(mode == HudMode::Gameplay, "second SELECT press must return to gameplay HUD");

    std::cout << "HUD mode tests passed\n";
    return 0;
}
