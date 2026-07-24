#include "BuildFeedback.hpp"
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

    flow.update(1, false, false, true);
    require(flow.phase() == TutorialPhase::ReadyToStart, "first tower unlocks wave start");
    require(flow.requestWaveStart(1), "wave starts after tower is built");
    require(flow.waveRunning(), "wave running state is exposed");

    flow.update(1, false, false, true);
    require(flow.phase() == TutorialPhase::ReadyToStart,
        "completed non-final wave returns to preparation");
    require(!flow.finished(), "preparation between waves is not a finished mission");
    require(flow.requestWaveStart(1), "next wave can start after preparation");

    flow.update(1, true, false, false);
    require(flow.phase() == TutorialPhase::Victory, "completed mission produces victory");
    require(flow.finished(), "victory is a finished state");

    flow.reset();
    require(flow.phase() == TutorialPhase::BuildFirstTower, "reset returns to first instruction");
    require(!flow.waveRunning(), "reset pauses the next wave");
    flow.update(1, false, false, true);
    require(flow.requestWaveStart(1), "flow can start after reset");
    flow.update(1, false, true, false);
    require(flow.phase() == TutorialPhase::Defeat, "lost wave produces defeat");
    flow.reset();
    require(!flow.finished(), "reset clears finished state");

    require(
        evaluateBuildAttempt(false, false, true, true) == BuildAttemptResult::NoBuildSpot,
        "missing build spot is reported");
    require(
        evaluateBuildAttempt(true, true, true, true) == BuildAttemptResult::Occupied,
        "occupied build spot is reported");
    require(
        evaluateBuildAttempt(true, false, false, true) == BuildAttemptResult::InsufficientGold,
        "insufficient gold is reported");
    require(
        evaluateBuildAttempt(true, false, true, false) == BuildAttemptResult::TowerLimitReached,
        "tower capacity is reported");
    require(
        evaluateBuildAttempt(true, false, true, true, false) == BuildAttemptResult::InvalidTower,
        "invalid tower is reported");
    require(
        evaluateBuildAttempt(true, false, true, true) == BuildAttemptResult::Built,
        "valid build is accepted");

    std::cout << "Tutorial flow and build feedback tests passed.\n";
    return 0;
}
