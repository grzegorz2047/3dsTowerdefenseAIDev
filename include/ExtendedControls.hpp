#pragma once

#include <cstdint>

enum class ExtendedControlScheme : std::uint8_t {
    Camera,
    Build,
};

struct ExtendedRawInput {
    bool available = false;
    int cX = 0;
    int cY = 0;
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
constexpr int kAxisDeadzone = 24;
inline bool runtimeAvailable = false;
inline ExtendedControlScheme runtimeScheme = ExtendedControlScheme::Camera;

[[nodiscard]] constexpr ExtendedControlScheme nextScheme(ExtendedControlScheme scheme) {
    return scheme == ExtendedControlScheme::Camera
        ? ExtendedControlScheme::Build
        : ExtendedControlScheme::Camera;
}

[[nodiscard]] constexpr int clampAxis(int value) {
    return value < -kAxisMaximum ? -kAxisMaximum
        : (value > kAxisMaximum ? kAxisMaximum : value);
}

[[nodiscard]] constexpr int filteredAxis(int value) {
    const int clamped = clampAxis(value);
    return clamped >= -kAxisDeadzone && clamped <= kAxisDeadzone ? 0 : clamped;
}

[[nodiscard]] constexpr ExtendedMappedInput map(
    const ExtendedRawInput& input,
    ExtendedControlScheme scheme) {
    if (!input.available) return {};

    ExtendedMappedInput output{};
    if (scheme == ExtendedControlScheme::Camera) {
        output.cameraX = filteredAxis(input.cX);
        output.cameraY = filteredAxis(input.cY);
        output.legacyShoulderDelta =
            (input.zrDown ? 1 : 0) - (input.zlDown ? 1 : 0);
        return output;
    }

    const int previous = (input.cLeftDown || input.cUpDown) ? 1 : 0;
    const int next = (input.cRightDown || input.cDownDown) ? 1 : 0;
    output.cursorDelta = next - previous;
    output.cameraX = (input.zrHeld ? kAxisMaximum : 0) -
        (input.zlHeld ? kAxisMaximum : 0);
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
