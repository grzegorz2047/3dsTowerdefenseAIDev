#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "HardwareTelemetry.hpp"

int main() {
    HardwareTelemetrySampler sampler;
    HardwareRenderCounter counter;

    HardwareFrameMetrics first{};
    first.cpuMilliseconds = 4.0F;
    first.leftEyeMilliseconds = 6.0F;
    first.rightEyeMilliseconds = 5.0F;
    first.topUiMilliseconds = 1.0F;
    first.bottomUiMilliseconds = 2.0F;
    first.frameWaitMilliseconds = 0.5F;
    first.citroProcessingMilliseconds = 8.0F;
    first.gpuDrawingMilliseconds = 12.0F;
    first.freeLinearMemoryBytes = 900U * 1024U;
    first.eyeCount = 2U;
    first.deviceProfile = HardwareDeviceProfile::Old3DS;
    first.measured = HardwareMeasurement::Cpu | HardwareMeasurement::LeftEye |
        HardwareMeasurement::RightEye | HardwareMeasurement::TopUi |
        HardwareMeasurement::BottomUi | HardwareMeasurement::FrameWait |
        HardwareMeasurement::LinearMemory | HardwareMeasurement::DeviceProfile |
        HardwareMeasurement::CitroProcessing | HardwareMeasurement::GpuDrawing;
    counter.recordSceneDraw(1000U);
    counter.recordSceneDraw(500U);
    counter.recordUiDraw();
    counter.recordTextureUpload(4096U);
    counter.applyTo(first);
    sampler.record(first);

    HardwareFrameMetrics second{};
    second.cpuMilliseconds = 8.0F;
    second.leftEyeMilliseconds = 7.0F;
    second.topUiMilliseconds = 1.5F;
    second.bottomUiMilliseconds = 2.5F;
    second.frameWaitMilliseconds = 1.0F;
    second.citroProcessingMilliseconds = 7.0F;
    second.gpuDrawingMilliseconds = 10.0F;
    second.freeLinearMemoryBytes = 700U * 1024U;
    second.eyeCount = 1U;
    second.deviceProfile = HardwareDeviceProfile::Old3DS;
    second.measured = HardwareMeasurement::Cpu | HardwareMeasurement::LeftEye |
        HardwareMeasurement::TopUi | HardwareMeasurement::BottomUi |
        HardwareMeasurement::FrameWait | HardwareMeasurement::LinearMemory |
        HardwareMeasurement::DeviceProfile | HardwareMeasurement::CitroProcessing |
        HardwareMeasurement::GpuDrawing;
    counter.reset();
    counter.recordSceneDraw(900U);
    counter.recordUiDraw();
    counter.applyTo(second);
    sampler.record(second);

    const HardwareTelemetrySnapshot& snapshot = sampler.snapshot();
    assert(snapshot.sampleCount == 2U);
    assert(snapshot.cpuSampleCount == 2U);
    assert(snapshot.renderStageSampleCount == 2U);
    assert(snapshot.citroProcessingSampleCount == 2U);
    assert(snapshot.gpuDrawingSampleCount == 2U);
    assert(snapshot.averageCpuMilliseconds == 6.0F);
    assert(snapshot.worstCpuMilliseconds == 8.0F);
    assert(snapshot.averageRenderStageMilliseconds == 12.5F);
    assert(snapshot.worstRenderStageMilliseconds == 14.0F);
    assert(snapshot.averageCitroProcessingMilliseconds == 7.5F);
    assert(snapshot.worstCitroProcessingMilliseconds == 8.0F);
    assert(snapshot.averageGpuDrawingMilliseconds == 11.0F);
    assert(snapshot.worstGpuDrawingMilliseconds == 12.0F);
    assert(snapshot.minimumFreeLinearMemoryBytes == 700U * 1024U);
    assert(snapshot.accumulatedSceneDrawCalls == 3U);
    assert(snapshot.accumulatedUiDrawCalls == 2U);
    assert(snapshot.accumulatedVertices == 2400U);
    assert(snapshot.accumulatedTextureUploads == 1U);
    assert(snapshot.accumulatedTextureUploadBytes == 4096U);
    assert(snapshot.last.eyeCount == 1U);
    assert(snapshot.stereoObserved);
    assert(snapshot.measurementComplete());
    assert(snapshot.last.deviceProfile == HardwareDeviceProfile::Old3DS);

    HardwareTelemetrySampler incompleteSampler;
    HardwareFrameMetrics incomplete{};
    incomplete.eyeCount = 2U;
    incomplete.measured = HardwareMeasurement::BaseRequired;
    incompleteSampler.record(incomplete);
    assert(!incompleteSampler.snapshot().measurementComplete());
    incomplete.measured |= HardwareMeasurement::RightEye;
    incompleteSampler.record(incomplete);
    assert(incompleteSampler.snapshot().measurementComplete());

    HardwareTelemetrySampler missingCpuCompletenessSampler;
    HardwareFrameMetrics missingCpuCompleteness = second;
    missingCpuCompleteness.measured &= static_cast<std::uint16_t>(~HardwareMeasurement::Cpu);
    missingCpuCompletenessSampler.record(missingCpuCompleteness);
    assert(!missingCpuCompletenessSampler.snapshot().measurementComplete());

    HardwareTelemetrySampler unbiasedSampler;
    HardwareFrameMetrics missingCpu{};
    missingCpu.leftEyeMilliseconds = 3.0F;
    missingCpu.topUiMilliseconds = 1.0F;
    missingCpu.bottomUiMilliseconds = 1.0F;
    missingCpu.measured = HardwareMeasurement::LeftEye | HardwareMeasurement::TopUi |
        HardwareMeasurement::BottomUi;
    unbiasedSampler.record(missingCpu);
    HardwareFrameMetrics measuredCpu{};
    measuredCpu.cpuMilliseconds = 10.0F;
    measuredCpu.measured = HardwareMeasurement::Cpu;
    unbiasedSampler.record(measuredCpu);
    assert(unbiasedSampler.snapshot().cpuSampleCount == 1U);
    assert(unbiasedSampler.snapshot().averageCpuMilliseconds == 10.0F);
    assert(unbiasedSampler.snapshot().renderStageSampleCount == 1U);
    assert(unbiasedSampler.snapshot().averageRenderStageMilliseconds == 5.0F);

    HardwareFrameMetrics invalid{};
    invalid.cpuMilliseconds = -2.0F;
    invalid.leftEyeMilliseconds = -3.0F;
    invalid.frameWaitMilliseconds = -1.0F;
    invalid.citroProcessingMilliseconds = -4.0F;
    invalid.gpuDrawingMilliseconds = -5.0F;
    invalid.freeLinearMemoryBytes = 800U * 1024U;
    invalid.measured = HardwareMeasurement::Cpu | HardwareMeasurement::LeftEye |
        HardwareMeasurement::FrameWait | HardwareMeasurement::CitroProcessing |
        HardwareMeasurement::GpuDrawing | HardwareMeasurement::LinearMemory;
    sampler.reset();
    sampler.record(invalid);
    assert(sampler.snapshot().last.cpuMilliseconds == 0.0F);
    assert(sampler.snapshot().last.leftEyeMilliseconds == 0.0F);
    assert(sampler.snapshot().last.frameWaitMilliseconds == 0.0F);
    assert(sampler.snapshot().last.citroProcessingMilliseconds == 0.0F);
    assert(sampler.snapshot().last.gpuDrawingMilliseconds == 0.0F);
    assert(sampler.snapshot().minimumFreeLinearMemoryBytes == 800U * 1024U);

    HardwareRenderCounter saturated;
    saturated.recordSceneDraw(std::numeric_limits<std::uint32_t>::max());
    saturated.recordSceneDraw(1U);
    saturated.recordTextureUpload(std::numeric_limits<std::size_t>::max());
    saturated.recordTextureUpload(1U);
    HardwareFrameMetrics saturationMetrics{};
    saturated.applyTo(saturationMetrics);
    assert(saturationMetrics.submittedVertices == std::numeric_limits<std::uint32_t>::max());
    assert(saturationMetrics.textureUploadBytes == std::numeric_limits<std::size_t>::max());

    return 0;
}
