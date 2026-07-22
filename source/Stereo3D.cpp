#include "Stereo3D.hpp"

#include <algorithm>

StereoFramePlan Stereo3D::plan(float sliderState, bool enabled, std::uint8_t maximumDepthPercent) {
    StereoFramePlan result{};
    result.slider = std::clamp(sliderState, 0.0F, 1.0F);
    const float depthLimit = static_cast<float>(std::min<std::uint8_t>(maximumDepthPercent, 100U)) / 100.0F;
    result.separation = enabled ? result.slider * depthLimit * kMaximumIod : 0.0F;
    result.stereo = result.separation > 0.0001F;
    result.eyeCount = result.stereo ? 2U : 1U;
    if (result.stereo) {
        result.leftEyeIod = -result.separation;
        result.rightEyeIod = result.separation;
    }
    return result;
}
