#include "TouchGesture.hpp"

TouchGestureResult TouchGesture::update(bool touching, std::int16_t x, std::int16_t y) {
    TouchGestureResult result{};

    if (touching) {
        if (!tracking_) {
            startX_ = x;
            startY_ = y;
            lastX_ = x;
            lastY_ = y;
            tracking_ = true;
            cancelledByMovement_ = false;
            return result;
        }

        lastX_ = x;
        lastY_ = y;
        const int dx = static_cast<int>(x) - static_cast<int>(startX_);
        const int dy = static_cast<int>(y) - static_cast<int>(startY_);
        const int threshold = kTapMovementThresholdPixels;
        if (dx * dx + dy * dy > threshold * threshold) {
            cancelledByMovement_ = true;
        }
        return result;
    }

    if (!tracking_) {
        return result;
    }

    result.tapped = !cancelledByMovement_;
    result.startX = startX_;
    result.startY = startY_;
    result.endX = lastX_;
    result.endY = lastY_;
    reset();
    return result;
}

void TouchGesture::reset() {
    startX_ = 0;
    startY_ = 0;
    lastX_ = 0;
    lastY_ = 0;
    tracking_ = false;
    cancelledByMovement_ = false;
}

bool TouchGesture::tracking() const { return tracking_; }
bool TouchGesture::cancelledByMovement() const { return cancelledByMovement_; }