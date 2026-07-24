#pragma once

#include <cstdint>

struct StereoFramePlan {
    bool stereo = false;
    std::uint8_t eyeCount = 1U;
    float slider = 0.0F;
    float separation = 0.0F;
    float leftEyeIod = 0.0F;
    float rightEyeIod = 0.0F;
};

class Stereo3D {
public:
    static constexpr float kMaximumIod = 0.44F;
    static constexpr std::uint8_t kDefaultDepthPercent = 70;
    static constexpr float kMinimumConvergenceDistance = 8.0F;
    static constexpr float kMaximumConvergenceDistance = 22.0F;

    [[nodiscard]] static StereoFramePlan plan(
        float sliderState, std::uint8_t maximumDepthPercent);
    [[nodiscard]] static float convergenceDistance(float cameraDistance, float cameraHeight);
    [[nodiscard]] static std::uint8_t nextDepthLimit(std::uint8_t currentDepthPercent);
};
