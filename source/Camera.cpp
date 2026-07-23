#include "Camera.hpp"

#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846F;
constexpr float kFieldOfView = 52.0F * kPi / 180.0F;
constexpr float kAspectRatio = 400.0F / 240.0F;
constexpr float kNearPlane = 0.1F;
constexpr float kFarPlane = 100.0F;
constexpr float kStereoConvergenceDistance = 14.0F;
constexpr float kCameraHeight = -3.8F;
constexpr float kQuarterTurn = kPi * 0.5F;
constexpr float kMotionYawAxisScale = 92.0F;
constexpr float kMotionPitchAxisScale = 54.0F;
}  // namespace

Camera::Camera() = default;

void Camera::update(const InputSnapshot& input, float deltaSeconds) {
    if (input.motionRecalibrate) motionFilter_.calibrate(input.motionRaw);
    const MotionCameraInput motion = motionFilter_.update(input.motionRaw, deltaSeconds);
    const ExtendedMappedInput extended = input.extended();
    orbit_.update(
        static_cast<int>(input.circle.dx) + extended.cameraX + static_cast<int>(motion.yaw * kMotionYawAxisScale),
        static_cast<int>(input.circle.dy) + extended.cameraY + static_cast<int>(motion.pitch * kMotionPitchAxisScale),
        deltaSeconds);
    if (input.pressed(KEY_L)) orbit_.rotateQuarterTurn(-1);
    if (input.pressed(KEY_R)) orbit_.rotateQuarterTurn(1);
}

void Camera::writeProjection(C3D_Mtx& destination) const {
    Mtx_PerspTilt(&destination, kFieldOfView, kAspectRatio, kNearPlane, kFarPlane, false);
}

void Camera::writeStereoProjection(C3D_Mtx& destination, float interocularDistance) const {
    Mtx_PerspStereoTilt(&destination, kFieldOfView, kAspectRatio, kNearPlane, kFarPlane,
        interocularDistance, kStereoConvergenceDistance, false);
}

void Camera::writeView(C3D_Mtx& destination) const {
    Mtx_Identity(&destination);
    Mtx_Translate(&destination, 0.0F, kCameraHeight, 0.0F - orbit_.distance(), true);
    Mtx_RotateX(&destination, orbit_.pitch(), true);
    Mtx_RotateY(&destination, orbit_.yaw(), true);
}

int Camera::rotationIndex() const {
    return static_cast<int>(std::lround(orbit_.yaw() / kQuarterTurn));
}
float Camera::focusX() const { return 0.0F; }
float Camera::focusZ() const { return 0.0F; }
float Camera::yaw() const { return orbit_.yaw(); }
float Camera::distance() const { return orbit_.distance(); }
