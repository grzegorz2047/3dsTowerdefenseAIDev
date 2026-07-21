#pragma once

#include <citro3d.h>

#include "Input.hpp"

class Camera {
public:
    Camera();

    void update(const InputSnapshot& input, float deltaSeconds);
    void writeProjection(C3D_Mtx& destination) const;
    void writeView(C3D_Mtx& destination) const;

    [[nodiscard]] int rotationIndex() const;
    [[nodiscard]] float focusX() const;
    [[nodiscard]] float focusZ() const;

private:
    float focusX_ = 0.0F;
    float focusZ_ = 0.0F;
    int rotationIndex_ = 0;
};
