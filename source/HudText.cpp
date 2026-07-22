#include "HudText.hpp"

const char* tutorialInstruction(TutorialPhase phase) {
    switch (phase) {
        case TutorialPhase::BuildFirstTower:
            return "D-PAD: wybierz niebieskie pole\nA: zbuduj pierwsza wieze";
        case TutorialPhase::ReadyToStart:
            return "Wieza gotowa\nX: uruchom fale";
        case TutorialPhase::WaveRunning:
            return "Bron cytadeli\nWieze strzelaja automatycznie";
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
