#pragma once

#include <3ds.h>

#include "ExtendedControls.hpp"
#include "MotionCamera.hpp"

struct InputSnapshot {
    u32 down = 0;
    u32 held = 0;
    circlePosition circle{};
    touchPosition touch{};
    ExtendedRawInput extendedRaw{};
    ExtendedControlScheme extendedScheme = ExtendedControlScheme::Camera;
    MotionRawInput motionRaw{};
    bool motionRecalibrate = false;

    [[nodiscard]] bool pressed(u32 key) const {
        return (down & key) != 0U;
    }

    [[nodiscard]] bool isHeld(u32 key) const {
        return (held & key) != 0U;
    }

    [[nodiscard]] bool touching() const {
        return isHeld(KEY_TOUCH);
    }

    [[nodiscard]] bool extendedAvailable() const {
        return extendedRaw.available;
    }

    [[nodiscard]] ExtendedMappedInput extended() const {
        return ExtendedControls::map(extendedRaw, extendedScheme);
    }
};

class InputSystem {
public:
    InputSystem();
    ~InputSystem();

    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;

    [[nodiscard]] InputSnapshot poll();
    [[nodiscard]] bool extendedAvailable() const;
    [[nodiscard]] ExtendedControlScheme extendedScheme() const;
    [[nodiscard]] bool configureMotion(bool enabled);
    [[nodiscard]] bool motionAvailable() const;

private:
    bool irrstInitialized_ = false;
    bool extendedAvailable_ = false;
    ExtendedControlScheme extendedScheme_ = ExtendedControlScheme::Camera;
    u32 previousIrrstHeld_ = 0U;
    u32 previousCStickDirections_ = 0U;
    bool motionEnabled_ = false;
    bool gyroscopeInitialized_ = false;
    float gyroscopeRawToDps_ = 0.0F;
};

[[nodiscard]] bool new3dsExtendedControlsAvailable();
[[nodiscard]] ExtendedControlScheme activeExtendedControlScheme();
