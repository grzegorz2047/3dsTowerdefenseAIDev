#include "TutorialFlow.hpp"

void TutorialFlow::reset() {
    phase_ = TutorialPhase::BuildFirstTower;
}

void TutorialFlow::update(std::size_t towerCount, bool missionCompleted, bool missionLost,
    bool awaitingNextWave) {
    if (missionLost) {
        phase_ = TutorialPhase::Defeat;
        return;
    }
    if (missionCompleted) {
        phase_ = TutorialPhase::Victory;
        return;
    }
    if (phase_ == TutorialPhase::BuildFirstTower && towerCount > 0U) {
        phase_ = TutorialPhase::ReadyToStart;
        return;
    }
    if (awaitingNextWave && phase_ == TutorialPhase::WaveRunning) {
        phase_ = TutorialPhase::ReadyToStart;
    }
}

bool TutorialFlow::requestWaveStart(std::size_t towerCount) {
    if (phase_ != TutorialPhase::ReadyToStart || towerCount == 0U) {
        return false;
    }
    phase_ = TutorialPhase::WaveRunning;
    return true;
}

TutorialPhase TutorialFlow::phase() const {
    return phase_;
}

bool TutorialFlow::waveRunning() const {
    return phase_ == TutorialPhase::WaveRunning;
}

bool TutorialFlow::finished() const {
    return phase_ == TutorialPhase::Victory || phase_ == TutorialPhase::Defeat;
}
