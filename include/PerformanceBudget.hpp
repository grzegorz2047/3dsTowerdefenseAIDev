#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace PerformanceBudget {

constexpr float kTargetFrameMilliseconds = 33.333F;
constexpr float kWarningFrameMilliseconds = 36.0F;
constexpr float kMonoRenderBudgetMilliseconds = 18.0F;
constexpr float kStereoRenderBudgetMilliseconds = 28.0F;
constexpr std::size_t kMinimumLinearMemoryReserveBytes = 512U * 1024U;
constexpr std::size_t kMaximumEnemies = 16U;
constexpr std::size_t kMaximumTowers = 16U;
constexpr std::size_t kMaximumProjectiles = 32U;
// Historical pre-16x16 contract kept visible for migration checks: kMaximumLevelVertices = 4096U.
constexpr std::size_t kMaximumLevelVertices = 5120U;
constexpr std::size_t kFrameWindowSize = 120U;

}  // namespace PerformanceBudget

struct PerformanceSnapshot {
    float averageFrameMilliseconds = 0.0F;
    float worstFrameMilliseconds = 0.0F;
    float lastFrameMilliseconds = 0.0F;
    float lastRenderMilliseconds = 0.0F;
    std::size_t freeLinearMemoryBytes = 0U;
    std::uint32_t sampleCount = 0U;

    [[nodiscard]] bool frameBudgetExceeded() const {
        return worstFrameMilliseconds > PerformanceBudget::kWarningFrameMilliseconds;
    }

    [[nodiscard]] bool memoryReserveLow() const {
        return freeLinearMemoryBytes < PerformanceBudget::kMinimumLinearMemoryReserveBytes;
    }
};

class PerformanceSampler {
public:
    void reset() {
        accumulatedFrameMilliseconds_ = 0.0F;
        worstFrameMilliseconds_ = 0.0F;
        sampleCount_ = 0U;
        snapshot_ = {};
    }

    void record(float frameMilliseconds, float renderMilliseconds, std::size_t freeLinearMemoryBytes) {
        const float safeFrame = std::max(frameMilliseconds, 0.0F);
        const float safeRender = std::max(renderMilliseconds, 0.0F);
        accumulatedFrameMilliseconds_ += safeFrame;
        worstFrameMilliseconds_ = std::max(worstFrameMilliseconds_, safeFrame);
        ++sampleCount_;

        snapshot_.lastFrameMilliseconds = safeFrame;
        snapshot_.lastRenderMilliseconds = safeRender;
        snapshot_.freeLinearMemoryBytes = freeLinearMemoryBytes;
        snapshot_.sampleCount = sampleCount_;
        snapshot_.averageFrameMilliseconds = accumulatedFrameMilliseconds_ /
            static_cast<float>(sampleCount_);
        snapshot_.worstFrameMilliseconds = worstFrameMilliseconds_;

        if (sampleCount_ >= PerformanceBudget::kFrameWindowSize) {
            accumulatedFrameMilliseconds_ = 0.0F;
            worstFrameMilliseconds_ = 0.0F;
            sampleCount_ = 0U;
        }
    }

    [[nodiscard]] const PerformanceSnapshot& snapshot() const { return snapshot_; }

private:
    float accumulatedFrameMilliseconds_ = 0.0F;
    float worstFrameMilliseconds_ = 0.0F;
    std::uint32_t sampleCount_ = 0U;
    PerformanceSnapshot snapshot_{};
};