#pragma once

#include <cstdint>

struct StereoFramePlan {
    bool stereo = false;
    std::uint8_t eyeCount = 1;
    float slider = 0.0F;
    float separation = 0.0F;
    float leftEyeIod = 0.0F;
    float rightEyeIod = 0.0F;
};

class Stereo3D {
public:
    static constexpr float kMaximumIod = 0.34F;
    static constexpr std::uint8_t kDefaultDepthPercent = 70;

    // The physical slider is the only on/off control. The legacy enabled flag is
    // retained temporarily for save-format and call-site compatibility, but it
    // cannot disable stereo when the hardware slider is raised.
    [[nodiscard]] static StereoFramePlan plan(
        float sliderState,
        bool legacyEnabled,
        std::uint8_t maximumDepthPercent);
    [[nodiscard]] static std::uint8_t nextDepthLimit(std::uint8_t currentDepthPercent);
};
