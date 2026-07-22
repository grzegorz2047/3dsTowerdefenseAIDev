#include <cstdlib>
#include <cstring>
#include <iostream>

#include "HudText.hpp"

namespace {

void expectContains(const char* value, const char* needle, const char* message) {
    if (value == nullptr || std::strstr(value, needle) == nullptr) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "D-PAD", "build phase must mention D-PAD");
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "A:", "build phase must mention A");
    expectContains(tutorialInstruction(TutorialPhase::ReadyToStart), "X:", "ready phase must mention X");
    expectContains(tutorialInstruction(TutorialPhase::Victory), "Y:", "victory phase must mention Y");
    expectContains(tutorialInstruction(TutorialPhase::Defeat), "Y:", "defeat phase must mention Y");

    expectContains(buildAttemptMessage(BuildAttemptResult::Occupied), "zajete", "occupied feedback must explain the reason");
    expectContains(buildAttemptMessage(BuildAttemptResult::InsufficientGold), "zlota", "gold feedback must explain the reason");
    expectContains(buildAttemptMessage(BuildAttemptResult::TowerLimitReached), "Limit", "limit feedback must explain the reason");

    expectContains(audioStatusMessage(true), "OK", "available audio must be reported as ready");
    expectContains(audioStatusMessage(false), "BLAD", "failed audio initialization must be visible");

    std::cout << "HUD text unit tests passed\n";
    return 0;
}
