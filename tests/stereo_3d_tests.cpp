#include <cstdlib>
#include <iostream>

#include "Stereo3D.hpp"

namespace {
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    const StereoFramePlan mono = Stereo3D::plan(0.0F, 100U);
    expect(!mono.stereo && mono.eyeCount == 1U, "zero physical slider should render one eye");
    const StereoFramePlan stereo = Stereo3D::plan(1.0F, 100U);
    expect(stereo.stereo && stereo.eyeCount == 2U, "raised physical slider should render two eyes");
    expect(stereo.separation > 0.0F, "raised physical slider should create separation");
    const StereoFramePlan limited = Stereo3D::plan(1.0F, 50U);
    expect(limited.separation < stereo.separation, "depth limit should scale separation");
    expect(Stereo3D::nextDepthLimit(100U) == 0U, "depth limit cycle should include zero");
    std::cout << "Stereo 3D tests passed\n";
    return 0;
}
