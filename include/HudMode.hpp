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

class HudModeController {
public:
    [[nodiscard]] HudMode update(bool selectHeld) {
        const bool pressed = selectHeld && !selectHeldLastFrame_;
        mode_ = toggleHudMode(mode_, pressed);
        selectHeldLastFrame_ = selectHeld;
        return mode_;
    }

    [[nodiscard]] HudMode mode() const { return mode_; }

    void reset() {
        mode_ = HudMode::Gameplay;
        selectHeldLastFrame_ = false;
    }

private:
    HudMode mode_ = HudMode::Gameplay;
    bool selectHeldLastFrame_ = false;
};

// Compatibility entrypoint for the current mission loop. State is intentionally
// local to the UI helper and changes only on the rising edge of SELECT.
[[nodiscard]] inline HudMode hudModeForSelectHeld(bool selectHeld) {
    static HudModeController controller;
    return controller.update(selectHeld);
}

[[nodiscard]] constexpr bool showAudioDiagnostics(HudMode mode) {
    return mode == HudMode::AudioDiagnostics;
}

[[nodiscard]] constexpr bool allowDiagnosticTone(HudMode mode, bool bPressed) {
    return showAudioDiagnostics(mode) && bPressed;
}
