#pragma once

#include <algorithm>
#include <cmath>

struct MotionRawInput {
    bool available = false;
    float yawDegreesPerSecond = 0.0F;
    float pitchDegreesPerSecond = 0.0F;
};

struct MotionCameraInput {
    float yaw = 0.0F;
    float pitch = 0.0F;
};

class MotionCameraFilter {
public:
    static constexpr float kDeadzoneDegreesPerSecond = 2.5F;
    static constexpr float kFullScaleDegreesPerSecond = 45.0F;
    static constexpr float kSmoothingPerSecond = 10.0F;
    static constexpr float kSettledThreshold = 0.004F;

    void reset() {
        calibrated_ = false;
        biasYaw_ = 0.0F;
        biasPitch_ = 0.0F;
        smoothedYaw_ = 0.0F;
        smoothedPitch_ = 0.0F;
    }

    void calibrate(const MotionRawInput& input) {
        if (!input.available) {
            reset();
            return;
        }
        calibrated_ = true;
        biasYaw_ = input.yawDegreesPerSecond;
        biasPitch_ = input.pitchDegreesPerSecond;
        smoothedYaw_ = 0.0F;
        smoothedPitch_ = 0.0F;
    }

    [[nodiscard]] MotionCameraInput update(const MotionRawInput& input, float deltaSeconds) {
        if (!input.available) {
            reset();
            return {};
        }
        if (!calibrated_) {
            calibrate(input);
            return {};
        }

        const float safeDelta = std::clamp(deltaSeconds, 0.0F, 1.0F / 15.0F);
        const float blend = std::clamp(safeDelta * kSmoothingPerSecond, 0.0F, 1.0F);
        const float targetYaw = normalized(input.yawDegreesPerSecond - biasYaw_);
        const float targetPitch = normalized(input.pitchDegreesPerSecond - biasPitch_);
        smoothedYaw_ += (targetYaw - smoothedYaw_) * blend;
        smoothedPitch_ += (targetPitch - smoothedPitch_) * blend;
        if (targetYaw == 0.0F && std::fabs(smoothedYaw_) < kSettledThreshold) smoothedYaw_ = 0.0F;
        if (targetPitch == 0.0F && std::fabs(smoothedPitch_) < kSettledThreshold) smoothedPitch_ = 0.0F;
        return {smoothedYaw_, smoothedPitch_};
    }

    [[nodiscard]] bool calibrated() const { return calibrated_; }

private:
    [[nodiscard]] static float normalized(float value) {
        const float magnitude = std::fabs(value);
        if (magnitude <= kDeadzoneDegreesPerSecond) return 0.0F;
        const float usable = kFullScaleDegreesPerSecond - kDeadzoneDegreesPerSecond;
        const float scaled = (magnitude - kDeadzoneDegreesPerSecond) / usable;
        return std::copysign(std::clamp(scaled, 0.0F, 1.0F), value);
    }

    bool calibrated_ = false;
    float biasYaw_ = 0.0F;
    float biasPitch_ = 0.0F;
    float smoothedYaw_ = 0.0F;
    float smoothedPitch_ = 0.0F;
};
