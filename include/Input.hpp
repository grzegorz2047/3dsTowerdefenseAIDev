#pragma once

#include <3ds.h>

struct InputSnapshot {
    u32 down = 0;
    u32 held = 0;
    circlePosition circle{};

    [[nodiscard]] bool pressed(u32 key) const {
        return (down & key) != 0U;
    }

    [[nodiscard]] bool isHeld(u32 key) const {
        return (held & key) != 0U;
    }
};

class InputSystem {
public:
    [[nodiscard]] InputSnapshot poll() const;
};
