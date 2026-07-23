#include "Input.hpp"

namespace {

bool gExtendedControlsAvailable = false;
ExtendedControlScheme gExtendedControlScheme = ExtendedControlScheme::Camera;

bool keyDown(u32 down, u32 key) {
    return (down & key) != 0U;
}

bool keyHeld(u32 held, u32 key) {
    return (held & key) != 0U;
}

void publishRuntimeState(bool available, ExtendedControlScheme scheme) {
    gExtendedControlsAvailable = available;
    gExtendedControlScheme = scheme;
    ExtendedControls::setRuntimeState(available, scheme);
}

}  // namespace

InputSystem::InputSystem() {
    bool isNew3DS = false;
    const Result result = APT_CheckNew3DS(&isNew3DS);
    extendedAvailable_ = R_SUCCEEDED(result) && isNew3DS;
    publishRuntimeState(extendedAvailable_, extendedScheme_);
}

InputSnapshot InputSystem::poll() {
    hidScanInput();

    InputSnapshot snapshot{};
    snapshot.down = hidKeysDown();
    snapshot.held = hidKeysHeld();
    hidCircleRead(&snapshot.circle);
    hidTouchRead(&snapshot.touch);

    if (extendedAvailable_ && keyHeld(snapshot.held, KEY_SELECT) && keyDown(snapshot.down, KEY_Y)) {
        extendedScheme_ = ExtendedControls::nextScheme(extendedScheme_);
        publishRuntimeState(extendedAvailable_, extendedScheme_);
    }

    snapshot.extendedScheme = extendedScheme_;
    snapshot.extendedRaw.available = extendedAvailable_;
    snapshot.extendedRaw.cLeftHeld = keyHeld(snapshot.held, KEY_CSTICK_LEFT);
    snapshot.extendedRaw.cRightHeld = keyHeld(snapshot.held, KEY_CSTICK_RIGHT);
    snapshot.extendedRaw.cUpHeld = keyHeld(snapshot.held, KEY_CSTICK_UP);
    snapshot.extendedRaw.cDownHeld = keyHeld(snapshot.held, KEY_CSTICK_DOWN);
    snapshot.extendedRaw.cLeftDown = keyDown(snapshot.down, KEY_CSTICK_LEFT);
    snapshot.extendedRaw.cRightDown = keyDown(snapshot.down, KEY_CSTICK_RIGHT);
    snapshot.extendedRaw.cUpDown = keyDown(snapshot.down, KEY_CSTICK_UP);
    snapshot.extendedRaw.cDownDown = keyDown(snapshot.down, KEY_CSTICK_DOWN);
    snapshot.extendedRaw.zlHeld = keyHeld(snapshot.held, KEY_ZL);
    snapshot.extendedRaw.zrHeld = keyHeld(snapshot.held, KEY_ZR);
    snapshot.extendedRaw.zlDown = keyDown(snapshot.down, KEY_ZL);
    snapshot.extendedRaw.zrDown = keyDown(snapshot.down, KEY_ZR);
    return snapshot;
}

bool InputSystem::extendedAvailable() const {
    return extendedAvailable_;
}

ExtendedControlScheme InputSystem::extendedScheme() const {
    return extendedScheme_;
}

bool new3dsExtendedControlsAvailable() {
    return gExtendedControlsAvailable;
}

ExtendedControlScheme activeExtendedControlScheme() {
    return gExtendedControlScheme;
}
