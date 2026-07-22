#include "Input.hpp"

InputSnapshot InputSystem::poll() const {
    hidScanInput();

    InputSnapshot snapshot{};
    snapshot.down = hidKeysDown();
    snapshot.held = hidKeysHeld();
    hidCircleRead(&snapshot.circle);
    hidTouchRead(&snapshot.touch);
    return snapshot;
}