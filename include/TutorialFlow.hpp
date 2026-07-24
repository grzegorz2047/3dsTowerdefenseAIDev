#pragma once

#include <cstddef>

enum class TutorialPhase {
    BuildFirstTower,
    ReadyToStart,
    WaveRunning,
    Victory,
    Defeat,
};

class TutorialFlow {
public:
    void reset();
    void update(std::size_t towerCount, bool missionCompleted, bool missionLost,
        bool awaitingNextWave = false);
    [[nodiscard]] bool requestWaveStart(std::size_t towerCount);

    [[nodiscard]] TutorialPhase phase() const;
    [[nodiscard]] bool waveRunning() const;
    [[nodiscard]] bool finished() const;

private:
    TutorialPhase phase_ = TutorialPhase::BuildFirstTower;
};
