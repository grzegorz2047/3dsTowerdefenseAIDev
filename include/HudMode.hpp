#pragma once

#include <cstdint>

enum class HudMode : std::uint8_t {
    Gameplay,
    AudioDiagnostics,
};

[[nodiscard]] constexpr HudMode toggleHudMode(HudMode current, bool selectPressed) {
    if (!selectPressed) {
        return current;
    }
    return current == HudMode::Gameplay ? HudMode::AudioDiagnostics : HudMode::Gameplay;
}

[[nodiscard]] constexpr bool showAudioDiagnostics(HudMode mode) {
    return mode == HudMode::AudioDiagnostics;
}

[[nodiscard]] constexpr bool allowDiagnosticTone(HudMode mode, bool bPressed) {
    return showAudioDiagnostics(mode) && bPressed;
}
