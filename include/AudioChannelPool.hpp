#pragma once

#include <array>
#include <cstddef>

#include "AudioPolicy.hpp"

namespace AudioChannelPool {

constexpr std::array<std::size_t, 3U> kPoolStarts{0U, 3U, 5U};
constexpr std::array<std::size_t, 3U> kPoolSizes{3U, 2U, 1U};

[[nodiscard]] constexpr std::size_t slotFor(AudioCue cue, std::size_t cursor) {
    const std::size_t pool = audioPoolIndex(cue);
    return kPoolStarts[pool] + cursor % kPoolSizes[pool];
}

[[nodiscard]] constexpr std::size_t nextCursor(AudioCue cue, std::size_t cursor) {
    const std::size_t pool = audioPoolIndex(cue);
    return (cursor + 1U) % kPoolSizes[pool];
}

}  // namespace AudioChannelPool
