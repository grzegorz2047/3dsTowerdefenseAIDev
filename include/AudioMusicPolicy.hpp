#pragma once

#include <algorithm>
#include <cstdint>

#include "AudioBackend.hpp"
#include "TutorialFlow.hpp"

enum class MusicPhase : std::uint8_t {
    Silent,
    Preparation,
    Combat,
};

struct MusicGainTargets {
    float preparation = 0.0F;
    float combat = 0.0F;
    float ambient = 0.0F;
};

[[nodiscard]] constexpr MusicPhase musicPhaseFor(TutorialPhase phase) {
    switch (phase) {
        case TutorialPhase::BuildFirstTower:
        case TutorialPhase::ReadyToStart:
            return MusicPhase::Preparation;
        case TutorialPhase::WaveRunning:
            return MusicPhase::Combat;
        case TutorialPhase::Victory:
        case TutorialPhase::Defeat:
            return MusicPhase::Silent;
    }
    return MusicPhase::Silent;
}

[[nodiscard]] constexpr MusicGainTargets targetMusicGains(MusicPhase phase, bool enabled) {
    if (!enabled) return {};
    switch (phase) {
        case MusicPhase::Preparation: return {1.0F, 0.0F, 1.0F};
        case MusicPhase::Combat: return {0.0F, 1.0F, 1.0F};
        case MusicPhase::Silent: return {};
    }
    return {};
}

[[nodiscard]] inline float approachMusicGain(float current, float target, float deltaSeconds) {
    constexpr float kFadeSeconds = 0.45F;
    const float step = std::max(deltaSeconds, 0.0F) / kFadeSeconds;
    if (current < target) return std::min(current + step, target);
    return std::max(current - step, target);
}

[[nodiscard]] constexpr bool shouldStartMissionMusic(
    AudioBackend backend,
    bool alreadyPlaying,
    bool hasSamples) {
    return backend != AudioBackend::None && !alreadyPlaying && hasSamples;
}

[[nodiscard]] constexpr bool usesCsndMusicLoop(AudioBackend backend) {
    return backend == AudioBackend::Csnd;
}
