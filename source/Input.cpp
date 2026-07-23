#include "Input.hpp"

namespace {

constexpr u32 kCStickLeft = BIT(0);
constexpr u32 kCStickRight = BIT(1);
constexpr u32 kCStickUp = BIT(2);
constexpr u32 kCStickDown = BIT(3);

bool gExtendedControlsAvailable = false;
ExtendedControlScheme gExtendedControlScheme = ExtendedControlScheme::Camera;

bool keyDown(u32 down, u32 key) {
    return (down & key) != 0U;
}

bool keyHeld(u32 held, u32 key) {
    return (held & key) != 0U;
}

u32 cStickDirections(const circlePosition& position) {
    u32 directions = 0U;
    if (position.dx < -ExtendedControls::kAxisDeadzone) directions |= kCStickLeft;
    if (position.dx > ExtendedControls::kAxisDeadzone) directions |= kCStickRight;
    if (position.dy > ExtendedControls::kAxisDeadzone) directions |= kCStickUp;
    if (position.dy < -ExtendedControls::kAxisDeadzone) directions |= kCStickDown;
    return directions;
}

void publishRuntimeState(bool available, ExtendedControlScheme scheme) {
    gExtendedControlsAvailable = available;
    gExtendedControlScheme = scheme;
    ExtendedControls::setRuntimeState(available, scheme);
}

}  // namespace

InputSystem::InputSystem() {
    bool isNew3DS = false;
    const Result modelResult = APT_CheckNew3DS(&isNew3DS);
    if (R_SUCCEEDED(modelResult) && isNew3DS) {
        const Result irrstResult = irrstInit();
        irrstInitialized_ = R_SUCCEEDED(irrstResult);
    }
    extendedAvailable_ = irrstInitialized_;
    publishRuntimeState(extendedAvailable_, extendedScheme_);
    MotionCameraRuntime::publish(false, false);
}

InputSystem::~InputSystem() {
    (void)configureMotion(false);
    if (irrstInitialized_) {
        irrstExit();
        irrstInitialized_ = false;
    }
    publishRuntimeState(false, ExtendedControlScheme::Camera);
    MotionCameraRuntime::publish(false, false);
}

bool InputSystem::configureMotion(bool enabled) {
    if (!enabled) {
        if (gyroscopeInitialized_) {
            (void)HIDUSER_DisableGyroscope();
            gyroscopeInitialized_ = false;
        }
        motionEnabled_ = false;
        gyroscopeRawToDps_ = 0.0F;
        MotionCameraRuntime::publish(false, false);
        return true;
    }

    if (gyroscopeInitialized_) {
        motionEnabled_ = true;
        MotionCameraRuntime::publish(true, true);
        return true;
    }

    if (R_FAILED(HIDUSER_EnableGyroscope())) {
        motionEnabled_ = false;
        MotionCameraRuntime::publish(true, false);
        return false;
    }
    float coefficient = 0.0F;
    if (R_FAILED(HIDUSER_GetGyroscopeRawToDpsCoefficient(&coefficient)) || coefficient <= 0.0F) {
        (void)HIDUSER_DisableGyroscope();
        motionEnabled_ = false;
        MotionCameraRuntime::publish(true, false);
        return false;
    }
    gyroscopeInitialized_ = true;
    motionEnabled_ = true;
    gyroscopeRawToDps_ = coefficient;
    MotionCameraRuntime::publish(true, true);
    return true;
}

InputSnapshot InputSystem::poll() {
    hidScanInput();

    InputSnapshot snapshot{};
    snapshot.down = hidKeysDown();
    snapshot.held = hidKeysHeld();
    hidCircleRead(&snapshot.circle);
    hidTouchRead(&snapshot.touch);

    const bool selectHeld = keyHeld(snapshot.held, KEY_SELECT);
    if (selectHeld && keyDown(snapshot.down, KEY_DUP)) {
        (void)configureMotion(!motionEnabled_);
        snapshot.down &= ~KEY_DUP;
    }

    u32 irrstHeld = 0U;
    u32 irrstDown = 0U;
    u32 directionDown = 0U;
    circlePosition cStick{};
    if (irrstInitialized_) {
        irrstScanInput();
        irrstHeld = irrstKeysHeld();
        irrstDown = irrstHeld & ~previousIrrstHeld_;
        hidCstickRead(&cStick);
        const u32 directions = cStickDirections(cStick);
        directionDown = directions & ~previousCStickDirections_;
        previousIrrstHeld_ = irrstHeld;
        previousCStickDirections_ = directions;
    }

    if (extendedAvailable_ && selectHeld && keyDown(snapshot.down, KEY_Y)) {
        extendedScheme_ = ExtendedControls::nextScheme(extendedScheme_);
        publishRuntimeState(extendedAvailable_, extendedScheme_);
    }

    snapshot.extendedScheme = extendedScheme_;
    snapshot.extendedRaw.available = extendedAvailable_;
    snapshot.extendedRaw.cX = static_cast<int>(cStick.dx);
    snapshot.extendedRaw.cY = static_cast<int>(cStick.dy);
    snapshot.extendedRaw.cLeftDown = keyDown(directionDown, kCStickLeft);
    snapshot.extendedRaw.cRightDown = keyDown(directionDown, kCStickRight);
    snapshot.extendedRaw.cUpDown = keyDown(directionDown, kCStickUp);
    snapshot.extendedRaw.cDownDown = keyDown(directionDown, kCStickDown);
    snapshot.extendedRaw.zlHeld = keyHeld(irrstHeld, KEY_ZL);
    snapshot.extendedRaw.zrHeld = keyHeld(irrstHeld, KEY_ZR);
    snapshot.extendedRaw.zlDown = keyDown(irrstDown, KEY_ZL);
    snapshot.extendedRaw.zrDown = keyDown(irrstDown, KEY_ZR);

    snapshot.motionRecalibrate = motionEnabled_ && selectHeld && keyDown(snapshot.down, KEY_B);
    if (motionEnabled_ && gyroscopeInitialized_) {
        angularRate rate{};
        hidGyroRead(&rate);
        snapshot.motionRaw.available = true;
        snapshot.motionRaw.yawDegreesPerSecond = static_cast<float>(rate.z) * gyroscopeRawToDps_;
        snapshot.motionRaw.pitchDegreesPerSecond = static_cast<float>(rate.y) * gyroscopeRawToDps_;
    }
    return snapshot;
}

bool InputSystem::extendedAvailable() const {
    return extendedAvailable_;
}

ExtendedControlScheme InputSystem::extendedScheme() const {
    return extendedScheme_;
}

bool InputSystem::motionAvailable() const {
    return motionEnabled_ && gyroscopeInitialized_;
}

bool new3dsExtendedControlsAvailable() {
    return gExtendedControlsAvailable;
}

ExtendedControlScheme activeExtendedControlScheme() {
    return gExtendedControlScheme;
}
