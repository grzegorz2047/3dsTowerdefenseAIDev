#include <cmath>
#include <cstdlib>
#include <iostream>

#include "OrbitCamera.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

bool near(float left, float right, float tolerance = 0.0001F) {
    return std::fabs(left - right) < tolerance;
}

void testDeadzonePreventsDrift() {
    OrbitCamera camera;
    camera.update(OrbitCamera::kCircleDeadzone, -OrbitCamera::kCircleDeadzone, 1.0F / 30.0F);
    expect(near(camera.yaw(), OrbitCamera::kDefaultYaw), "deadzone must prevent yaw drift");
    expect(near(camera.distance(), OrbitCamera::kDefaultDistance), "deadzone must prevent zoom drift");
}

void testHorizontalAxisRotates() {
    OrbitCamera camera;
    camera.update(OrbitCamera::kCircleMaximum, 0, 0.5F);
    expect(camera.yaw() > OrbitCamera::kDefaultYaw, "right input must rotate clockwise");
    expect(near(camera.distance(), OrbitCamera::kDefaultDistance), "rotation must not change zoom");
}

void testVerticalAxisZooms() {
    OrbitCamera camera;
    camera.update(0, OrbitCamera::kCircleMaximum, 0.5F);
    expect(camera.distance() < OrbitCamera::kDefaultDistance, "up input must zoom in");
    camera.update(0, -OrbitCamera::kCircleMaximum, 1.0F);
    expect(camera.distance() > OrbitCamera::kMinimumDistance, "down input must zoom out");
}

void testZoomIsClamped() {
    OrbitCamera camera;
    for (int index = 0; index < 100; ++index) camera.update(0, OrbitCamera::kCircleMaximum, 1.0F);
    expect(near(camera.distance(), OrbitCamera::kMinimumDistance), "zoom in must stop at minimum distance");
    for (int index = 0; index < 100; ++index) camera.update(0, -OrbitCamera::kCircleMaximum, 1.0F);
    expect(near(camera.distance(), OrbitCamera::kMaximumDistance), "zoom out must stop at maximum distance");
}

void testShoulderButtonsRotateQuarterTurn() {
    OrbitCamera camera;
    camera.rotateQuarterTurn(1);
    expect(near(camera.yaw(), 3.14159265358979323846F * 0.5F), "right shoulder must rotate by quarter turn");
    camera.rotateQuarterTurn(-1);
    expect(near(camera.yaw(), OrbitCamera::kDefaultYaw), "opposite quarter turns must cancel");
}

}  // namespace

int main() {
    testDeadzonePreventsDrift();
    testHorizontalAxisRotates();
    testVerticalAxisZooms();
    testZoomIsClamped();
    testShoulderButtonsRotateQuarterTurn();
    std::cout << "Orbit camera tests passed\n";
    return 0;
}
