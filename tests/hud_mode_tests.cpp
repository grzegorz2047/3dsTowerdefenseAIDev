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
    const HudMode gameplay = hudModeForSelectHeld(false);
    const HudMode diagnostics = hudModeForSelectHeld(true);

    expect(gameplay == HudMode::Gameplay, "released SELECT must use gameplay HUD");
    expect(diagnostics == HudMode::AudioDiagnostics, "held SELECT must use diagnostic HUD");
    expect(!showAudioDiagnostics(gameplay), "gameplay HUD must hide raw audio diagnostics");
    expect(showAudioDiagnostics(diagnostics), "diagnostic HUD must expose raw audio diagnostics");
    expect(!allowDiagnosticTone(gameplay, true), "B must not play the test tone in normal gameplay");
    expect(!allowDiagnosticTone(diagnostics, false), "diagnostic tone requires B press");
    expect(allowDiagnosticTone(diagnostics, true), "SELECT+B must play the diagnostic tone");

    std::cout << "HUD mode tests passed\n";
    return 0;
}
