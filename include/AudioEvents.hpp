#pragma once

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
    Victory,
    Defeat,
    Count,
};

[[nodiscard]] AudioCue cueForBuildResult(BuildAttemptResult result);

struct AudioFrameState {
    TutorialPhase phase = TutorialPhase::BuildFirstTower;
    std::size_t activeProjectiles = 0;
};

class AudioEventRouter {
public:
    [[nodiscard]] std::uint32_t update(const AudioFrameState& state);
    void reset(const AudioFrameState& state = {});

private:
    bool initialized_ = false;
    AudioFrameState previous_{};
};

[[nodiscard]] constexpr std::uint32_t audioCueMask(AudioCue cue) {
    return 1U << static_cast<std::uint8_t>(cue);
}
