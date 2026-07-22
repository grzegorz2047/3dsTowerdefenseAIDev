#include "TutorialFlow.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    TutorialFlow flow;
    require(flow.phase() == TutorialPhase::BuildFirstTower, "tutorial starts with build instruction");
    require(!flow.waveRunning(), "wave is paused on startup");
    require(!flow.requestWaveStart(0), "wave cannot start without a tower");

    flow.update(1, false, false);
    require(flow.phase() == TutorialPhase::ReadyToStart, "first tower unlocks wave start");
    require(flow.requestWaveStart(1), "wave starts after tower is built");
    require(flow.waveRunning(), "wave running state is exposed");

    flow.update(1, true, false);
    require(flow.phase() == TutorialPhase::Victory, "completed wave produces victory");
    require(flow.finished(), "victory is a finished state");

    flow.reset();
    flow.update(1, false, false);
    require(flow.requestWaveStart(1), "flow can start after reset");
    flow.update(1, false, true);
    require(flow.phase() == TutorialPhase::Defeat, "lost wave produces defeat");

    std::cout << "Tutorial flow tests passed.\n";
    return 0;
}
