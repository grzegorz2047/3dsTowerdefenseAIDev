#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "ExtendedControls.hpp"
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

void testOld3dsFallbackIsNeutral() {
    ExtendedRawInput raw{};
    raw.cX = ExtendedControls::kAxisMaximum;
    raw.zrDown = true;
    const ExtendedMappedInput mapped = ExtendedControls::map(raw, ExtendedControlScheme::Camera);
    expect(mapped.cameraX == 0, "unavailable C-Stick must not move camera");
    expect(mapped.legacyShoulderDelta == 0, "unavailable ZR must not change tower");
}

void testNew3dsCameraSchemeUsesAnalogValues() {
    ExtendedRawInput raw{};
    raw.available = true;
    raw.cX = 100;
    raw.cY = ExtendedControls::kAxisMaximum;
    raw.zrDown = true;
    const ExtendedMappedInput mapped = ExtendedControls::map(raw, ExtendedControlScheme::Camera);
    expect(mapped.cameraX == 100, "C-Stick must preserve partial analog rotation");
    expect(mapped.cameraY == ExtendedControls::kAxisMaximum, "C-Stick up zooms camera");
    expect(mapped.legacyShoulderDelta == 1, "ZR selects next tower in camera scheme");
}

void testAnalogDeadzoneAndClamp() {
    ExtendedRawInput raw{};
    raw.available = true;
    raw.cX = ExtendedControls::kAxisDeadzone;
    raw.cY = ExtendedControls::kAxisMaximum + 100;
    const ExtendedMappedInput mapped = ExtendedControls::map(raw, ExtendedControlScheme::Camera);
    expect(mapped.cameraX == 0, "C-Stick deadzone must prevent drift");
    expect(mapped.cameraY == ExtendedControls::kAxisMaximum, "C-Stick input must be clamped");
}

void testNew3dsBuildScheme() {
    ExtendedRawInput raw{};
    raw.available = true;
    raw.cLeftDown = true;
    raw.zrHeld = true;
    const ExtendedMappedInput mapped = ExtendedControls::map(raw, ExtendedControlScheme::Build);
    expect(mapped.cursorDelta == -1, "C-Stick left selects previous build spot");
    expect(mapped.cameraX == ExtendedControls::kAxisMaximum, "ZR rotates camera in build scheme");
}

void testOppositeBuildDirectionsCancelAndSchemeToggles() {
    ExtendedRawInput raw{};
    raw.available = true;
    raw.cLeftDown = true;
    raw.cRightDown = true;
    const ExtendedMappedInput mapped = ExtendedControls::map(raw, ExtendedControlScheme::Build);
    expect(mapped.cursorDelta == 0, "opposite C-Stick cursor directions must cancel");
    expect(ExtendedControls::nextScheme(ExtendedControlScheme::Camera) == ExtendedControlScheme::Build,
        "camera scheme toggles to build");
    expect(ExtendedControls::nextScheme(ExtendedControlScheme::Build) == ExtendedControlScheme::Camera,
        "build scheme toggles to camera");
}

void testHardwareSpecificHints() {
    expect(std::string(ExtendedControls::hint(false, ExtendedControlScheme::Camera)).find("CPAD") != std::string::npos,
        "Old 3DS hint must mention standard Circle Pad controls");
    expect(std::string(ExtendedControls::hint(true, ExtendedControlScheme::Camera)).find("C:KAMERA") != std::string::npos,
        "New 3DS camera hint must mention C-Stick camera");
    expect(std::string(ExtendedControls::hint(true, ExtendedControlScheme::Build)).find("C:KURSOR") != std::string::npos,
        "New 3DS build hint must mention C-Stick cursor");
}

}  // namespace

int main() {
    testDeadzonePreventsDrift();
    testHorizontalAxisRotates();
    testVerticalAxisZooms();
    testZoomIsClamped();
    testShoulderButtonsRotateQuarterTurn();
    testOld3dsFallbackIsNeutral();
    testNew3dsCameraSchemeUsesAnalogValues();
    testAnalogDeadzoneAndClamp();
    testNew3dsBuildScheme();
    testOppositeBuildDirectionsCancelAndSchemeToggles();
    testHardwareSpecificHints();
    std::cout << "Orbit camera and extended control tests passed\n";
    return 0;
}
