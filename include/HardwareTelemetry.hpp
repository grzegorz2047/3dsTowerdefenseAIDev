#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace HardwareMeasurement {
constexpr std::uint16_t Cpu = 1U << 0U;
constexpr std::uint16_t LeftEye = 1U << 1U;
constexpr std::uint16_t RightEye = 1U << 2U;
constexpr std::uint16_t TopUi = 1U << 3U;
constexpr std::uint16_t BottomUi = 1U << 4U;
constexpr std::uint16_t FrameWait = 1U << 5U;
constexpr std::uint16_t SceneCounters = 1U << 6U;
constexpr std::uint16_t UiCounters = 1U << 7U;
constexpr std::uint16_t TextureCounters = 1U << 8U;
constexpr std::uint16_t LinearMemory = 1U << 9U;
constexpr std::uint16_t DeviceProfile = 1U << 10U;
constexpr std::uint16_t BaseRequired = Cpu | LeftEye | TopUi | BottomUi | FrameWait |
    SceneCounters | UiCounters | TextureCounters | LinearMemory | DeviceProfile;
}  // namespace HardwareMeasurement

enum class HardwareDeviceProfile : std::uint8_t {
    Unknown,
    Old3DS,
    New3DS,
};

inline const char* hardwareDeviceProfileName(HardwareDeviceProfile profile) {
    switch (profile) {
        case HardwareDeviceProfile::Old3DS: return "OLD 3DS";
        case HardwareDeviceProfile::New3DS: return "NEW 3DS";
        case HardwareDeviceProfile::Unknown:
        default: return "NIEZNANY";
    }
}

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
    std::uint16_t measured = 0U;
    std::uint8_t eyeCount = 1U;
    HardwareDeviceProfile deviceProfile = HardwareDeviceProfile::Unknown;
    bool speedupEnabled = false;

    [[nodiscard]] bool has(std::uint16_t measurement) const {
        return (measured & measurement) == measurement;
    }

    [[nodiscard]] float gpuWorkMilliseconds() const {
        return leftEyeMilliseconds + rightEyeMilliseconds +
            topUiMilliseconds + bottomUiMilliseconds;
    }
};

class HardwareRenderCounter {
public:
    void reset() {
        sceneDrawCalls_ = 0U;
        uiDrawCalls_ = 0U;
        submittedVertices_ = 0U;
        textureUploads_ = 0U;
        textureUploadBytes_ = 0U;
    }

    void recordSceneDraw(std::uint32_t vertices) {
        ++sceneDrawCalls_;
        submittedVertices_ += vertices;
    }

    void recordUiDraw() { ++uiDrawCalls_; }

    void recordTextureUpload(std::size_t bytes) {
        ++textureUploads_;
        textureUploadBytes_ += bytes;
    }

    void applyTo(HardwareFrameMetrics& metrics) const {
        metrics.sceneDrawCalls = sceneDrawCalls_;
        metrics.uiDrawCalls = uiDrawCalls_;
        metrics.submittedVertices = submittedVertices_;
        metrics.textureUploads = textureUploads_;
        metrics.textureUploadBytes = textureUploadBytes_;
        metrics.measured |= HardwareMeasurement::SceneCounters |
            HardwareMeasurement::UiCounters | HardwareMeasurement::TextureCounters;
    }

private:
    std::uint32_t sceneDrawCalls_ = 0U;
    std::uint32_t uiDrawCalls_ = 0U;
    std::uint32_t submittedVertices_ = 0U;
    std::uint32_t textureUploads_ = 0U;
    std::size_t textureUploadBytes_ = 0U;
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
    std::uint16_t observedMeasurements = 0U;
    std::uint32_t sampleCount = 0U;
    bool stereoObserved = false;

    [[nodiscard]] bool measurementComplete() const {
        const std::uint16_t required = stereoObserved
            ? static_cast<std::uint16_t>(
                HardwareMeasurement::BaseRequired | HardwareMeasurement::RightEye)
            : HardwareMeasurement::BaseRequired;
        return (observedMeasurements & required) == required;
    }
};

class HardwareTelemetrySampler {
public:
    void reset() {
        snapshot_ = {};
        accumulatedCpuMilliseconds_ = 0.0F;
        accumulatedGpuWorkMilliseconds_ = 0.0F;
        hasLinearMemorySample_ = false;
    }

    void record(const HardwareFrameMetrics& metrics) {
        HardwareFrameMetrics normalized = metrics;
        normalized.cpuMilliseconds = std::max(metrics.cpuMilliseconds, 0.0F);
        normalized.leftEyeMilliseconds = std::max(metrics.leftEyeMilliseconds, 0.0F);
        normalized.rightEyeMilliseconds = std::max(metrics.rightEyeMilliseconds, 0.0F);
        normalized.topUiMilliseconds = std::max(metrics.topUiMilliseconds, 0.0F);
        normalized.bottomUiMilliseconds = std::max(metrics.bottomUiMilliseconds, 0.0F);
        normalized.frameWaitMilliseconds = std::max(metrics.frameWaitMilliseconds, 0.0F);
        normalized.eyeCount = metrics.eyeCount > 1U ? 2U : 1U;

        snapshot_.last = normalized;
        ++snapshot_.sampleCount;
        snapshot_.observedMeasurements |= normalized.measured;
        snapshot_.stereoObserved = snapshot_.stereoObserved || normalized.eyeCount == 2U;

        accumulatedCpuMilliseconds_ += normalized.cpuMilliseconds;
        accumulatedGpuWorkMilliseconds_ += normalized.gpuWorkMilliseconds();
        snapshot_.averageCpuMilliseconds = accumulatedCpuMilliseconds_ /
            static_cast<float>(snapshot_.sampleCount);
        snapshot_.averageGpuWorkMilliseconds = accumulatedGpuWorkMilliseconds_ /
            static_cast<float>(snapshot_.sampleCount);
        snapshot_.worstCpuMilliseconds = std::max(
            snapshot_.worstCpuMilliseconds, normalized.cpuMilliseconds);
        snapshot_.worstGpuWorkMilliseconds = std::max(
            snapshot_.worstGpuWorkMilliseconds, normalized.gpuWorkMilliseconds());

        if (normalized.has(HardwareMeasurement::LinearMemory)) {
            if (!hasLinearMemorySample_) {
                snapshot_.minimumFreeLinearMemoryBytes = normalized.freeLinearMemoryBytes;
                hasLinearMemorySample_ = true;
            } else {
                snapshot_.minimumFreeLinearMemoryBytes = std::min(
                    snapshot_.minimumFreeLinearMemoryBytes, normalized.freeLinearMemoryBytes);
            }
        }

        snapshot_.accumulatedSceneDrawCalls += normalized.sceneDrawCalls;
        snapshot_.accumulatedUiDrawCalls += normalized.uiDrawCalls;
        snapshot_.accumulatedVertices += normalized.submittedVertices;
        snapshot_.accumulatedTextureUploads += normalized.textureUploads;
        snapshot_.accumulatedTextureUploadBytes += normalized.textureUploadBytes;
    }

    [[nodiscard]] const HardwareTelemetrySnapshot& snapshot() const { return snapshot_; }

private:
    HardwareTelemetrySnapshot snapshot_{};
    float accumulatedCpuMilliseconds_ = 0.0F;
    float accumulatedGpuWorkMilliseconds_ = 0.0F;
    bool hasLinearMemorySample_ = false;
};
