#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Stereo3D.hpp"

namespace {
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

bool near(float left, float right, float epsilon = 0.0001F) {
    return std::fabs(left - right) <= epsilon;
}
}

int main() {
    const StereoFramePlan mono = Stereo3D::plan(0.0F, 100U);
    expect(!mono.stereo && mono.eyeCount == 1U, "zero physical slider should render one eye");

    const StereoFramePlan stereo = Stereo3D::plan(1.0F, 100U);
    expect(stereo.stereo && stereo.eyeCount == 2U, "raised physical slider should render two eyes");
    expect(near(stereo.separation, Stereo3D::kMaximumIod),
        "maximum slider should use the tuned maximum separation");
    expect(near(stereo.leftEyeIod, -stereo.rightEyeIod),
        "left and right eye offsets should stay symmetric");

    const StereoFramePlan halfSlider = Stereo3D::plan(0.5F, 100U);
    expect(halfSlider.separation > stereo.separation * 0.5F,
        "shaped slider should make the upper range more expressive");
    expect(halfSlider.separation < stereo.separation,
        "partial slider should remain below maximum separation");

    const StereoFramePlan limited = Stereo3D::plan(1.0F, 50U);
    expect(near(limited.separation, stereo.separation * 0.5F),
        "depth limit should scale separation predictably");

    const float nearConvergence = Stereo3D::convergenceDistance(9.0F, -3.8F);
    const float farConvergence = Stereo3D::convergenceDistance(21.0F, -3.8F);
    expect(farConvergence > nearConvergence,
        "convergence should follow camera distance");
    expect(nearConvergence >= Stereo3D::kMinimumConvergenceDistance,
        "convergence should respect the safe minimum");
    expect(farConvergence <= Stereo3D::kMaximumConvergenceDistance,
        "convergence should respect the safe maximum");

    expect(Stereo3D::nextDepthLimit(100U) == 0U, "depth limit cycle should include zero");
    std::cout << "Stereo 3D tests passed\n";
    return 0;
}
