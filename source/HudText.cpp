#include "HudText.hpp"

#include "ExtendedControls.hpp"
#include "MotionCamera.hpp"

const char* tutorialInstruction(TutorialPhase phase) {
    if (phase == TutorialPhase::WaveRunning) {
        return MotionCameraRuntime::hint();
    }

    if (phase != TutorialPhase::Victory && phase != TutorialPhase::Defeat &&
        ExtendedControls::runtimeAvailable) {
        switch (phase) {
            case TutorialPhase::BuildFirstTower:
                return ExtendedControls::runtimeScheme == ExtendedControlScheme::Camera
                    ? "N3DS C:KAMERA ZL/ZR:WIEZA A:BUDUJ"
                    : "N3DS C:POLE ZL/ZR:KAMERA A:BUDUJ";
            case TutorialPhase::ReadyToStart:
                return "X:START SELECT+Y:TRYB N3DS";
            case TutorialPhase::WaveRunning:
            case TutorialPhase::Victory:
            case TutorialPhase::Defeat:
                break;
        }
    }

    switch (phase) {
        case TutorialPhase::BuildFirstTower:
            return "D-PAD: wybierz niebieskie pole\nA: zbuduj pierwsza wieze";
        case TutorialPhase::ReadyToStart:
            return "Wieza gotowa\nX: uruchom fale";
        case TutorialPhase::WaveRunning:
            return MotionCameraRuntime::hint();
        case TutorialPhase::Victory:
            return "ZWYCIESTWO\nY: zagraj ponownie";
        case TutorialPhase::Defeat:
            return "PORAZKA\nY: sprobuj ponownie";
    }
    return "";
}

const char* buildAttemptMessage(BuildAttemptResult result) {
    switch (result) {
        case BuildAttemptResult::Built: return "Wieza zbudowana";
        case BuildAttemptResult::NoBuildSpot: return "Brak pola budowy";
        case BuildAttemptResult::Occupied: return "Pole jest zajete";
        case BuildAttemptResult::InsufficientGold: return "Za malo zlota";
        case BuildAttemptResult::TowerLimitReached: return "Limit wiez osiagniety";
        case BuildAttemptResult::InvalidTower: return "Nie mozna tu budowac";
        case BuildAttemptResult::None:
        default: return "Niebieskie: budowa | Brazowe: droga";
    }
}

const char* audioStatusMessage(bool available) {
    return available ? "AUDIO: OK" : "AUDIO: BLAD DSP";
}
