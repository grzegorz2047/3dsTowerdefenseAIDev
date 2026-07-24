#include "Stereo3D.hpp"

#include <algorithm>
#include <cmath>

StereoFramePlan Stereo3D::plan(float sliderState, std::uint8_t maximumDepthPercent) {
    StereoFramePlan result{};
    result.slider = std::clamp(sliderState, 0.0F, 1.0F);
    const float depthLimit =
        static_cast<float>(std::min<std::uint8_t>(maximumDepthPercent, 100U)) / 100.0F;
    const float shapedSlider = result.slider * (1.35F - 0.35F * result.slider);
    result.separation = shapedSlider * depthLimit * kMaximumIod;
    result.stereo = result.slider > 0.0001F && result.separation > 0.0001F;
    result.eyeCount = result.stereo ? 2U : 1U;
    if (result.stereo) {
        result.leftEyeIod = -result.separation;
        result.rightEyeIod = result.separation;
    }
    return result;
}

float Stereo3D::convergenceDistance(float cameraDistance, float cameraHeight) {
    const float distanceToBoard = std::hypot(cameraDistance, cameraHeight);
    return std::clamp(distanceToBoard, kMinimumConvergenceDistance,
        kMaximumConvergenceDistance);
}

std::uint8_t Stereo3D::nextDepthLimit(std::uint8_t currentDepthPercent) {
    if (currentDepthPercent < 50U) return 50U;
    if (currentDepthPercent < 70U) return 70U;
    if (currentDepthPercent < 100U) return 100U;
    return 0U;
}
