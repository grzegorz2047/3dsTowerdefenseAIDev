#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Stereo3D.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

bool near(float left, float right) {
    return std::fabs(left - right) < 0.0001F;
}

void testSliderZeroUsesMono() {
    const StereoFramePlan plan = Stereo3D::plan(0.0F, true, 100);
    expect(!plan.stereo, "zero slider should use mono");
    expect(plan.eyeCount == 1, "mono should render one eye");
    expect(near(plan.leftEyeIod, 0.0F), "mono left IOD should be zero");
}

void testDisabledSettingUsesMono() {
    const StereoFramePlan plan = Stereo3D::plan(1.0F, false, 100);
    expect(!plan.stereo, "disabled stereo should ignore slider");
    expect(plan.eyeCount == 1, "disabled stereo should render one eye");
}

void testStereoUsesOppositeEyeOffsets() {
    const StereoFramePlan plan = Stereo3D::plan(0.5F, true, 100);
    expect(plan.stereo, "non-zero slider should enable stereo");
    expect(plan.eyeCount == 2, "stereo should render two eyes");
    expect(plan.leftEyeIod < 0.0F, "left eye should use negative IOD");
    expect(plan.rightEyeIod > 0.0F, "right eye should use positive IOD");
    expect(near(-plan.leftEyeIod, plan.rightEyeIod), "eye offsets should be symmetric");
}

void testDepthLimitCapsSeparation() {
    const StereoFramePlan plan = Stereo3D::plan(1.0F, true, 50);
    expect(near(plan.separation, Stereo3D::kMaximumIod * 0.5F), "depth setting should cap separation");
    const StereoFramePlan clamped = Stereo3D::plan(2.0F, true, 200);
    expect(near(clamped.separation, Stereo3D::kMaximumIod), "slider and depth should clamp to safe maximum");
}

}  // namespace

int main() {
    testSliderZeroUsesMono();
    testDisabledSettingUsesMono();
    testStereoUsesOppositeEyeOffsets();
    testDepthLimitCapsSeparation();
    std::cout << "Stereo 3D tests passed\n";
    return 0;
}
