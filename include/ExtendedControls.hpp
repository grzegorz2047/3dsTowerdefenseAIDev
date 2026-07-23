#pragma once

#include <cstdint>

enum class ExtendedControlScheme : std::uint8_t {
    Camera,
    Build,
};

struct ExtendedRawInput {
    bool available = false;
    bool cLeftHeld = false;
    bool cRightHeld = false;
    bool cUpHeld = false;
    bool cDownHeld = false;
    bool cLeftDown = false;
    bool cRightDown = false;
    bool cUpDown = false;
    bool cDownDown = false;
    bool zlHeld = false;
    bool zrHeld = false;
    bool zlDown = false;
    bool zrDown = false;
};

struct ExtendedMappedInput {
    int cameraX = 0;
    int cameraY = 0;
    int cursorDelta = 0;
    int legacyShoulderDelta = 0;
};

namespace ExtendedControls {

constexpr int kAxisMaximum = 156;
inline bool runtimeAvailable = false;
inline ExtendedControlScheme runtimeScheme = ExtendedControlScheme::Camera;

[[nodiscard]] constexpr ExtendedControlScheme nextScheme(ExtendedControlScheme scheme) {
    return scheme == ExtendedControlScheme::Camera
        ? ExtendedControlScheme::Build
        : ExtendedControlScheme::Camera;
}

[[nodiscard]] constexpr int axis(bool negative, bool positive) {
    return negative == positive ? 0 : (positive ? kAxisMaximum : -kAxisMaximum);
}

[[nodiscard]] constexpr ExtendedMappedInput map(
    const ExtendedRawInput& input,
    ExtendedControlScheme scheme) {
    if (!input.available) return {};

    ExtendedMappedInput output{};
    if (scheme == ExtendedControlScheme::Camera) {
        output.cameraX = axis(input.cLeftHeld, input.cRightHeld);
        output.cameraY = axis(input.cDownHeld, input.cUpHeld);
        output.legacyShoulderDelta =
            (input.zrDown ? 1 : 0) - (input.zlDown ? 1 : 0);
        return output;
    }

    const int previous = (input.cLeftDown || input.cUpDown) ? 1 : 0;
    const int next = (input.cRightDown || input.cDownDown) ? 1 : 0;
    output.cursorDelta = next - previous;
    output.cameraX = axis(input.zlHeld, input.zrHeld);
    return output;
}

[[nodiscard]] constexpr const char* hint(
    bool available,
    ExtendedControlScheme scheme) {
    if (!available) return "CPAD:KAMERA L/R:WIEZA";
    return scheme == ExtendedControlScheme::Camera
        ? "N3DS C:KAMERA ZL/ZR:WIEZA SEL+Y"
        : "N3DS C:KURSOR ZL/ZR:KAMERA SEL+Y";
}

inline void setRuntimeState(bool available, ExtendedControlScheme scheme) {
    runtimeAvailable = available;
    runtimeScheme = scheme;
}

[[nodiscard]] inline const char* runtimeHint() {
    return hint(runtimeAvailable, runtimeScheme);
}

}  // namespace ExtendedControls
