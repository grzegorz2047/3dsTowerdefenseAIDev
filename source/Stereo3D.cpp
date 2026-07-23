#include "Stereo3D.hpp"

#include <algorithm>

StereoFramePlan Stereo3D::plan(float sliderState, bool, std::uint8_t maximumDepthPercent) {
    StereoFramePlan result{};
    result.slider = std::clamp(sliderState, 0.0F, 1.0F);
    const float depthLimit =
        static_cast<float>(std::min<std::uint8_t>(maximumDepthPercent, 100U)) / 100.0F;
    result.separation = result.slider * depthLimit * kMaximumIod;
    result.stereo = result.slider > 0.0001F && result.separation > 0.0001F;
    result.eyeCount = result.stereo ? 2U : 1U;
    if (result.stereo) {
        result.leftEyeIod = -result.separation;
        result.rightEyeIod = result.separation;
    }
    return result;
}

std::uint8_t Stereo3D::nextDepthLimit(std::uint8_t currentDepthPercent) {
    if (currentDepthPercent < 25U) return 25U;
    if (currentDepthPercent < 50U) return 50U;
    if (currentDepthPercent < 75U) return 75U;
    if (currentDepthPercent < 100U) return 100U;
    return 25U;
}
