#include "AudioEvents.hpp"

AudioCue cueForBuildResult(BuildAttemptResult result) {
    return result == BuildAttemptResult::Built ? AudioCue::BuildSuccess : AudioCue::BuildFailure;
}

std::uint32_t AudioEventRouter::update(const AudioFrameState& state) {
    if (!initialized_) {
        previous_ = state;
        initialized_ = true;
        return 0U;
    }

    std::uint32_t cues = 0U;
    if (previous_.phase != state.phase) {
        if (state.phase == TutorialPhase::WaveRunning) {
            cues |= audioCueMask(AudioCue::WaveStart);
        } else if (state.phase == TutorialPhase::Victory) {
            cues |= audioCueMask(AudioCue::Victory);
        } else if (state.phase == TutorialPhase::Defeat) {
            cues |= audioCueMask(AudioCue::Defeat);
        }
    }

    if (state.activeProjectiles > previous_.activeProjectiles) {
        cues |= audioCueMask(AudioCue::Shot);
    } else if (state.activeProjectiles < previous_.activeProjectiles) {
        cues |= audioCueMask(AudioCue::Hit);
    }

    previous_ = state;
    return cues;
}

void AudioEventRouter::reset(const AudioFrameState& state) {
    previous_ = state;
    initialized_ = true;
}
