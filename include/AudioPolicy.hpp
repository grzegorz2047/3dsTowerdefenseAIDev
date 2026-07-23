#pragma once

#include <cstddef>
#include <cstdint>

#include "AudioEvents.hpp"

enum class AudioPriority : std::uint8_t {
    Combat = 0,
    Action = 1,
    Critical = 2,
};

[[nodiscard]] constexpr AudioPriority audioPriority(AudioCue cue) {
    switch (cue) {
        case AudioCue::Shot:
        case AudioCue::Hit:
            return AudioPriority::Combat;
        case AudioCue::BuildSuccess:
        case AudioCue::BuildFailure:
        case AudioCue::WaveStart:
            return AudioPriority::Action;
        case AudioCue::Victory:
        case AudioCue::Defeat:
        case AudioCue::Count:
            return AudioPriority::Critical;
    }
    return AudioPriority::Critical;
}

[[nodiscard]] constexpr float audioCooldownSeconds(AudioCue cue) {
    switch (cue) {
        case AudioCue::Shot: return 0.09F;
        case AudioCue::Hit: return 0.07F;
        default: return 0.0F;
    }
}

[[nodiscard]] constexpr std::size_t audioPoolIndex(AudioCue cue) {
    return static_cast<std::size_t>(audioPriority(cue));
}
