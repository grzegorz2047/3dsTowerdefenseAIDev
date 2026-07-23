#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "BuildFeedback.hpp"
#include "TutorialFlow.hpp"

enum class AudioCue : std::uint8_t {
    BuildSuccess,
    BuildFailure,
    WaveStart,
    Shot,
    Hit,
    EnemyDeath,
    BaseDamage,
    Victory,
    Defeat,
    Count,
};

[[nodiscard]] AudioCue cueForBuildResult(BuildAttemptResult result);

struct AudioFrameState {
    TutorialPhase phase = TutorialPhase::BuildFirstTower;
    std::size_t activeProjectiles = 0;
    std::size_t defeatedEnemies = 0;
    int baseHealth = 5;
};

class AudioEventRouter {
public:
    [[nodiscard]] std::uint32_t update(const AudioFrameState& state, float deltaSeconds = 1.0F / 30.0F);
    void reset(const AudioFrameState& state = {});

private:
    [[nodiscard]] bool allow(AudioCue cue);
    void advanceCooldowns(float deltaSeconds);

    bool initialized_ = false;
    AudioFrameState previous_{};
    std::array<float, static_cast<std::size_t>(AudioCue::Count)> cooldowns_{};
};

[[nodiscard]] constexpr std::uint32_t audioCueMask(AudioCue cue) {
    return 1U << static_cast<std::uint8_t>(cue);
}
