#include <cstdlib>
#include <cstring>
#include <iostream>

#include "ExtendedControls.hpp"
#include "HudText.hpp"
#include "MotionCamera.hpp"

namespace {

void expectContains(const char* value, const char* needle, const char* message) {
    if (value == nullptr || std::strstr(value, needle) == nullptr) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    ExtendedControls::setRuntimeState(false, ExtendedControlScheme::Camera);
    MotionCameraRuntime::publish(false, false);
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "D-PAD", "Old 3DS build phase must mention D-PAD");
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "A:", "build phase must mention A");
    expectContains(tutorialInstruction(TutorialPhase::ReadyToStart), "X:", "ready phase must mention X");
    expectContains(tutorialInstruction(TutorialPhase::WaveRunning), "SELECT+UP",
        "disabled motion camera must advertise the opt-in shortcut");

    ExtendedControls::setRuntimeState(true, ExtendedControlScheme::Camera);
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "C:KAMERA",
        "New 3DS camera scheme must identify C-Stick camera mapping");
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "ZL/ZR:WIEZA",
        "New 3DS camera scheme must identify tower shortcuts");
    expectContains(tutorialInstruction(TutorialPhase::ReadyToStart), "SELECT+Y",
        "New 3DS ready phase must explain scheme toggle");

    ExtendedControls::setRuntimeState(true, ExtendedControlScheme::Build);
    expectContains(tutorialInstruction(TutorialPhase::BuildFirstTower), "C:POLE",
        "New 3DS build scheme must identify C-Stick cursor mapping");

    MotionCameraRuntime::publish(true, true);
    expectContains(tutorialInstruction(TutorialPhase::WaveRunning), "CENTRUJ",
        "enabled motion camera must show the recenter shortcut");
    MotionCameraRuntime::publish(true, false);
    expectContains(tutorialInstruction(TutorialPhase::WaveRunning), "BRAK SENSORA",
        "failed sensor initialization must be visible");

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
