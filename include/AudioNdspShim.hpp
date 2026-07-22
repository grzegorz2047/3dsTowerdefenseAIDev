#pragma once

#include <cstdint>

// libctru returns this exact Result when neither /3ds/dspfirm.cdc nor the
// Homebrew Launcher hb:ndsp handle is available. Only this condition is safe
// to retry with our synthetic component; all other failures must remain real.
constexpr std::uint32_t kNdspComponentNotFoundResult = 0xD8B0A7FAU;

[[nodiscard]] constexpr bool shouldAttemptNdspHleShim(std::uint32_t result) {
    return result == kNdspComponentNotFoundResult;
}
