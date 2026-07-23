#pragma once

#include <citro3d.h>

#include "Input.hpp"
#include "MotionCamera.hpp"
#include "OrbitCamera.hpp"

class Camera {
public:
    Camera();

    void update(const InputSnapshot& input, float deltaSeconds);
    void writeProjection(C3D_Mtx& destination) const;
    void writeStereoProjection(C3D_Mtx& destination, float interocularDistance) const;
    void writeView(C3D_Mtx& destination) const;

    [[nodiscard]] int rotationIndex() const;
    [[nodiscard]] float focusX() const;
    [[nodiscard]] float focusZ() const;
    [[nodiscard]] float yaw() const;
    [[nodiscard]] float distance() const;

private:
    OrbitCamera orbit_{};
    MotionCameraFilter motionFilter_{};
};
