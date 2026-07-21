#include "Camera.hpp"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kFieldOfView = 55.0F * static_cast<float>(M_PI) / 180.0F;
constexpr float kAspectRatio = 400.0F / 240.0F;
constexpr float kNearPlane = 0.1F;
constexpr float kFarPlane = 100.0F;
constexpr float kCameraPitch = -0.72F;
constexpr float kCameraDistance = -12.0F;
constexpr float kPanSpeed = 0.018F;
constexpr float kMapLimit = 6.0F;
constexpr float kQuarterTurn = static_cast<float>(M_PI) * 0.5F;

}  // namespace

Camera::Camera() = default;

void Camera::update(const InputSnapshot& input, float deltaSeconds) {
    const float frameScale = std::clamp(deltaSeconds * 60.0F, 0.0F, 2.0F);
    const float localX = static_cast<float>(input.circle.dx) * kPanSpeed * frameScale;
    const float localZ = static_cast<float>(input.circle.dy) * kPanSpeed * frameScale;
    const float yaw = static_cast<float>(rotationIndex_) * kQuarterTurn;

    const float worldX = localX * std::cos(yaw) + localZ * std::sin(yaw);
    const float worldZ = -localX * std::sin(yaw) + localZ * std::cos(yaw);

    focusX_ = std::clamp(focusX_ + worldX, -kMapLimit, kMapLimit);
    focusZ_ = std::clamp(focusZ_ + worldZ, -kMapLimit, kMapLimit);

    if (input.pressed(KEY_L)) {
        rotationIndex_ = (rotationIndex_ + 3) % 4;
    }
    if (input.pressed(KEY_R)) {
        rotationIndex_ = (rotationIndex_ + 1) % 4;
    }
}

void Camera::writeProjection(C3D_Mtx& destination) const {
    Mtx_PerspTilt(
        &destination,
        kFieldOfView,
        kAspectRatio,
        kNearPlane,
        kFarPlane,
        false);
}

void Camera::writeView(C3D_Mtx& destination) const {
    Mtx_Identity(&destination);
    Mtx_Translate(&destination, 0.0F, -3.2F, kCameraDistance, true);
    Mtx_RotateX(&destination, kCameraPitch, true);
    Mtx_RotateY(
        &destination,
        static_cast<float>(rotationIndex_) * kQuarterTurn,
        true);
    Mtx_Translate(&destination, -focusX_, 0.0F, -focusZ_, true);
}

int Camera::rotationIndex() const {
    return rotationIndex_;
}

float Camera::focusX() const {
    return focusX_;
}

float Camera::focusZ() const {
    return focusZ_;
}
