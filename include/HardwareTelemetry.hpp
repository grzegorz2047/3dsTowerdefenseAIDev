#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>

struct HardwareFrameMetrics {
    float cpuMilliseconds = 0.0F;
    float leftEyeMilliseconds = 0.0F;
    float rightEyeMilliseconds = 0.0F;
    float topUiMilliseconds = 0.0F;
    float bottomUiMilliseconds = 0.0F;
    float frameWaitMilliseconds = 0.0F;
    std::uint32_t sceneDrawCalls = 0U;
    std::uint32_t uiDrawCalls = 0U;
    std::uint32_t submittedVertices = 0U;
    std::uint32_t textureUploads = 0U;
    std::size_t textureUploadBytes = 0U;
    std::size_t freeLinearMemoryBytes = 0U;
    std::uint8_t eyeCount = 1U;
    bool new3DS = false;
    bool speedupEnabled = false;
};

struct HardwareTelemetrySnapshot {
    HardwareFrameMetrics last{};
    float averageCpuMilliseconds = 0.0F;
    float worstCpuMilliseconds = 0.0F;
    float averageGpuWorkMilliseconds = 0.0F;
    float worstGpuWorkMilliseconds = 0.0F;
    std::size_t minimumFreeLinearMemoryBytes = 0U;
    std::uint64_t accumulatedSceneDrawCalls = 0U;
    std::uint64_t accumulatedUiDrawCalls = 0U;
    std::uint64_t accumulatedVertices = 0U;
    std::uint64_t accumulatedTextureUploads = 0U;
    std::uint64_t accumulatedTextureUploadBytes = 0U;
    std::uint32_t sampleCount = 0U;
};

class HardwareTelemetrySampler {
public:
    void reset() {
        snapshot_ = {};
        accumulatedCpuMilliseconds_ = 0.0F;
        accumulatedGpuWorkMilliseconds_ = 0.0F;
    }

    void record(const HardwareFrameMetrics& metrics) {
        const float safeCpu = std::max(metrics.cpuMilliseconds, 0.0F);
        const float safeGpu = std::max(metrics.leftEyeMilliseconds, 0.0F) +
            std::max(metrics.rightEyeMilliseconds, 0.0F) +
            std::max(metrics.topUiMilliseconds, 0.0F) +
            std::max(metrics.bottomUiMilliseconds, 0.0F);

        snapshot_.last = metrics;
        snapshot_.last.cpuMilliseconds = safeCpu;
        snapshot_.last.leftEyeMilliseconds = std::max(metrics.leftEyeMilliseconds, 0.0F);
        snapshot_.last.rightEyeMilliseconds = std::max(metrics.rightEyeMilliseconds, 0.0F);
        snapshot_.last.topUiMilliseconds = std::max(metrics.topUiMilliseconds, 0.0F);
        snapshot_.last.bottomUiMilliseconds = std::max(metrics.bottomUiMilliseconds, 0.0F);
        snapshot_.last.frameWaitMilliseconds = std::max(metrics.frameWaitMilliseconds, 0.0F);

        ++snapshot_.sampleCount;
        accumulatedCpuMilliseconds_ += safeCpu;
        accumulatedGpuWorkMilliseconds_ += safeGpu;
        snapshot_.averageCpuMilliseconds = accumulatedCpuMilliseconds_ /
            static_cast<float>(snapshot_.sampleCount);
        snapshot_.averageGpuWorkMilliseconds = accumulatedGpuWorkMilliseconds_ /
            static_cast<float>(snapshot_.sampleCount);
        snapshot_.worstCpuMilliseconds = std::max(snapshot_.worstCpuMilliseconds, safeCpu);
        snapshot_.worstGpuWorkMilliseconds = std::max(snapshot_.worstGpuWorkMilliseconds, safeGpu);

        if (snapshot_.sampleCount == 1U) {
            snapshot_.minimumFreeLinearMemoryBytes = metrics.freeLinearMemoryBytes;
        } else {
            snapshot_.minimumFreeLinearMemoryBytes = std::min(
                snapshot_.minimumFreeLinearMemoryBytes, metrics.freeLinearMemoryBytes);
        }

        snapshot_.accumulatedSceneDrawCalls += metrics.sceneDrawCalls;
        snapshot_.accumulatedUiDrawCalls += metrics.uiDrawCalls;
        snapshot_.accumulatedVertices += metrics.submittedVertices;
        snapshot_.accumulatedTextureUploads += metrics.textureUploads;
        snapshot_.accumulatedTextureUploadBytes += metrics.textureUploadBytes;
    }

    [[nodiscard]] const HardwareTelemetrySnapshot& snapshot() const { return snapshot_; }

private:
    HardwareTelemetrySnapshot snapshot_{};
    float accumulatedCpuMilliseconds_ = 0.0F;
    float accumulatedGpuWorkMilliseconds_ = 0.0F;
};
