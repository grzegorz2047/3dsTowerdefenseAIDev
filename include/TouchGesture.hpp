#pragma once

#include <cstdint>

struct TouchGestureResult {
    bool tapped = false;
    std::int16_t startX = 0;
    std::int16_t startY = 0;
    std::int16_t endX = 0;
    std::int16_t endY = 0;
};

class TouchGesture {
public:
    static constexpr int kTapMovementThresholdPixels = 8;

    [[nodiscard]] TouchGestureResult update(bool touching, std::int16_t x, std::int16_t y);
    void reset();

    [[nodiscard]] bool tracking() const;
    [[nodiscard]] bool cancelledByMovement() const;

private:
    std::int16_t startX_ = 0;
    std::int16_t startY_ = 0;
    std::int16_t lastX_ = 0;
    std::int16_t lastY_ = 0;
    bool tracking_ = false;
    bool cancelledByMovement_ = false;
};