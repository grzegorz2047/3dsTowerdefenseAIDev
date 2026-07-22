#pragma once

#include <cstdint>

// libctru returns this exact Result when neither /3ds/dspfirm.cdc nor the
// Homebrew Launcher hb:ndsp handle is available. Only this condition is safe
// to retry with our synthetic component; all other failures must remain real.
constexpr std::uint32_t kNdspComponentNotFoundResult = 0xD8B0A7FAU;
constexpr std::uint32_t kAudioResultSuccess = 0x00000000U;
constexpr std::uint32_t kAudioResultNotAttempted = 0xFFFFFFFFU;

[[nodiscard]] constexpr bool audioResultSucceeded(std::uint32_t result) {
    return result == kAudioResultSuccess;
}

[[nodiscard]] constexpr bool shouldAttemptNdspHleShim(std::uint32_t result) {
    return result == kNdspComponentNotFoundResult;
}

[[nodiscard]] constexpr bool ndspBackendReady(
    std::uint32_t initialResult,
    bool shimAttempted,
    std::uint32_t shimResult) {
    return shimAttempted
        ? audioResultSucceeded(shimResult)
        : audioResultSucceeded(initialResult);
}

[[nodiscard]] constexpr bool ndspShimIsActive(bool shimAttempted, std::uint32_t shimResult) {
    return shimAttempted && audioResultSucceeded(shimResult);
}
