#pragma once

#include <cstdint>

constexpr std::uint32_t kAudioResultSuccess = 0x00000000U;
constexpr std::uint32_t kAudioResultNotAttempted = 0xFFFFFFFFU;

// Values from libctru's 3ds/result.h. Keep this host-testable without pulling
// the platform headers into unit tests.
constexpr std::uint32_t kResultSummaryNotFound = 4U;
constexpr std::uint32_t kResultModuleDsp = 41U;
constexpr std::uint32_t kResultDescriptionNotFound = 1018U;
constexpr std::uint32_t kObservedNdspComponentNotFoundResult = 0xD880A7FAU;

[[nodiscard]] constexpr std::uint32_t resultSummary(std::uint32_t result) {
    return (result >> 21U) & 0x3FU;
}

[[nodiscard]] constexpr std::uint32_t resultModule(std::uint32_t result) {
    return (result >> 10U) & 0xFFU;
}

[[nodiscard]] constexpr std::uint32_t resultDescription(std::uint32_t result) {
    return result & 0x3FFU;
}

[[nodiscard]] constexpr bool audioResultSucceeded(std::uint32_t result) {
    return result == kAudioResultSuccess;
}

// A missing DSP component can be reported with different failure levels, so
// match the semantic Result fields rather than one complete 32-bit value.
[[nodiscard]] constexpr bool shouldAttemptNdspHleShim(std::uint32_t result) {
    return resultSummary(result) == kResultSummaryNotFound &&
        resultModule(result) == kResultModuleDsp &&
        resultDescription(result) == kResultDescriptionNotFound;
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
