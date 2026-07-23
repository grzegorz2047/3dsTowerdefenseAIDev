#include "AudioEvents.hpp"

#include <algorithm>

#include "AudioPolicy.hpp"

AudioCue cueForBuildResult(BuildAttemptResult result) {
    return result == BuildAttemptResult::Built ? AudioCue::BuildSuccess : AudioCue::BuildFailure;
}

void AudioEventRouter::advanceCooldowns(float deltaSeconds) {
    const float step = std::max(deltaSeconds, 0.0F);
    for (float& remaining : cooldowns_) remaining = std::max(remaining - step, 0.0F);
}

bool AudioEventRouter::allow(AudioCue cue) {
    const std::size_t index = static_cast<std::size_t>(cue);
    if (cooldowns_[index] > 0.0F) return false;
    cooldowns_[index] = audioCooldownSeconds(cue);
    return true;
}

std::uint32_t AudioEventRouter::update(const AudioFrameState& state, float deltaSeconds) {
    advanceCooldowns(deltaSeconds);
    if (!initialized_) {
        previous_ = state;
        initialized_ = true;
        return 0U;
    }

    std::uint32_t cues = 0U;
    if (previous_.phase != state.phase) {
        if (state.phase == TutorialPhase::WaveRunning) cues |= audioCueMask(AudioCue::WaveStart);
        else if (state.phase == TutorialPhase::Victory) cues |= audioCueMask(AudioCue::Victory);
        else if (state.phase == TutorialPhase::Defeat) cues |= audioCueMask(AudioCue::Defeat);
    }

    if (state.baseDamageEvents > previous_.baseDamageEvents) {
        cues |= audioCueMask(AudioCue::BaseDamage);
    }
    if (state.deathEvents > previous_.deathEvents) {
        cues |= audioCueMask(AudioCue::EnemyDeath);
    }
    if (state.shotEvents > previous_.shotEvents && allow(AudioCue::Shot)) {
        cues |= audioCueMask(AudioCue::Shot);
    }
    if (state.impactEvents > previous_.impactEvents && allow(AudioCue::Hit)) {
        cues |= audioCueMask(AudioCue::Hit);
    }

    previous_ = state;
    return cues;
}

void AudioEventRouter::reset(const AudioFrameState& state) {
    previous_ = state;
    cooldowns_.fill(0.0F);
    initialized_ = true;
}
