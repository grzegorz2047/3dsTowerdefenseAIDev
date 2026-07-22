#include <cstdlib>
#include <iostream>

#include "AudioProbe.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    AudioProbeState state{};

    state = updateAudioProbeState(state, false, true);
    expect(!state.querySucceeded, "failed query must be visible");
    expect(!state.active, "failed query cannot report an active channel");
    expect(!state.everActive, "failed query cannot set ever-active");

    state = updateAudioProbeState(state, true, true);
    expect(state.querySucceeded, "successful query must be visible");
    expect(state.active, "active channel must be reported");
    expect(state.everActive, "active channel must latch ever-active");

    state = updateAudioProbeState(state, true, false);
    expect(!state.active, "inactive sample must clear current activity");
    expect(state.everActive, "ever-active must remain latched");

    std::cout << "Audio probe unit tests passed\n";
    return 0;
}
