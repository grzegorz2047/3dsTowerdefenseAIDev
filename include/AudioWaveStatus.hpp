#pragma once

#include <cstdint>

enum class AudioWaveStatus : std::uint8_t {
    Free,
    Queued,
    Playing,
    Done,
    Unknown,
};

[[nodiscard]] constexpr AudioWaveStatus audioWaveStatusFromRaw(std::uint8_t value) {
    switch (value) {
        case 0: return AudioWaveStatus::Free;
        case 1: return AudioWaveStatus::Queued;
        case 2: return AudioWaveStatus::Playing;
        case 3: return AudioWaveStatus::Done;
        default: return AudioWaveStatus::Unknown;
    }
}

[[nodiscard]] constexpr const char* audioWaveStatusName(AudioWaveStatus status) {
    switch (status) {
        case AudioWaveStatus::Free: return "FREE";
        case AudioWaveStatus::Queued: return "QUEUED";
        case AudioWaveStatus::Playing: return "PLAYING";
        case AudioWaveStatus::Done: return "DONE";
        case AudioWaveStatus::Unknown:
        default: return "UNKNOWN";
    }
}

[[nodiscard]] constexpr bool audioWaveStatusActive(AudioWaveStatus status) {
    return status == AudioWaveStatus::Queued || status == AudioWaveStatus::Playing;
}
