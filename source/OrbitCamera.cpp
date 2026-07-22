#include "OrbitCamera.hpp"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kTwoPi = kPi * 2.0F;
constexpr float kQuarterTurn = kPi * 0.5F;

}  // namespace

float OrbitCamera::normalizedAxis(int value) {
    const int magnitude = std::abs(value);
    if (magnitude <= kCircleDeadzone) return 0.0F;

    const float usableRange = static_cast<float>(kCircleMaximum - kCircleDeadzone);
    const float normalized = static_cast<float>(magnitude - kCircleDeadzone) / usableRange;
    return std::copysign(std::clamp(normalized, 0.0F, 1.0F), static_cast<float>(value));
}

float OrbitCamera::wrapRadians(float value) {
    while (value >= kPi) value -= kTwoPi;
    while (value < -kPi) value += kTwoPi;
    return value;
}

void OrbitCamera::update(int circleX, int circleY, float deltaSeconds) {
    const float safeDelta = std::clamp(deltaSeconds, 0.0F, 1.0F / 15.0F);
    const float horizontal = normalizedAxis(circleX);
    const float vertical = normalizedAxis(circleY);

    yaw_ = wrapRadians(yaw_ + horizontal * kYawRadiansPerSecond * safeDelta);
    distance_ = std::clamp(distance_ - vertical * kZoomUnitsPerSecond * safeDelta,
        kMinimumDistance, kMaximumDistance);
}

void OrbitCamera::rotateQuarterTurn(int direction) {
    if (direction == 0) return;
    yaw_ = wrapRadians(yaw_ + static_cast<float>(direction) * kQuarterTurn);
}

float OrbitCamera::yaw() const { return yaw_; }
float OrbitCamera::pitch() const { return pitch_; }
float OrbitCamera::distance() const { return distance_; }
