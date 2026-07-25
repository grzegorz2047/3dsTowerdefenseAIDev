#include <cassert>
#include <cstddef>

#include "HardwareTelemetry.hpp"

int main() {
    HardwareTelemetrySampler sampler;

    HardwareRenderCounter counters;
    counters.recordSceneDraw(1200U);
    counters.recordSceneDraw(300U);
    counters.recordUiDraw();
    counters.recordTextureUpload(4096U);

    HardwareFrameMetrics first{};
    first.cpuMilliseconds = 4.0F;
    first.leftEyeMilliseconds = 6.0F;
    first.topUiMilliseconds = 1.0F;
    first.bottomUiMilliseconds = 2.0F;
    first.frameWaitMilliseconds = 3.0F;
    first.freeLinearMemoryBytes = 900U * 1024U;
    first.eyeCount = 1U;
    first.deviceProfile = HardwareDeviceProfile::Old3DS;
    first.measured = HardwareMeasurement::Cpu | HardwareMeasurement::LeftEye |
        HardwareMeasurement::TopUi | HardwareMeasurement::BottomUi |
        HardwareMeasurement::FrameWait | HardwareMeasurement::LinearMemory |
        HardwareMeasurement::DeviceProfile;
    counters.applyTo(first);
    sampler.record(first);

    const HardwareTelemetrySnapshot& mono = sampler.snapshot();
    assert(mono.sampleCount == 1U);
    assert(mono.measurementComplete());
    assert(mono.accumulatedSceneDrawCalls == 2U);
    assert(mono.accumulatedUiDrawCalls == 1U);
    assert(mono.accumulatedVertices == 1500U);
    assert(mono.accumulatedTextureUploads == 1U);
    assert(mono.accumulatedTextureUploadBytes == 4096U);
    assert(mono.minimumFreeLinearMemoryBytes == 900U * 1024U);
    assert(mono.last.deviceProfile == HardwareDeviceProfile::Old3DS);

    HardwareFrameMetrics stereo = first;
    stereo.eyeCount = 2U;
    stereo.rightEyeMilliseconds = 5.0F;
    stereo.freeLinearMemoryBytes = 700U * 1024U;
    stereo.measured |= HardwareMeasurement::RightEye;
    sampler.record(stereo);

    const HardwareTelemetrySnapshot& snapshot = sampler.snapshot();
    assert(snapshot.sampleCount == 2U);
    assert(snapshot.stereoObserved);
    assert(snapshot.measurementComplete());
    assert(snapshot.averageCpuMilliseconds == 4.0F);
    assert(snapshot.worstGpuWorkMilliseconds == 14.0F);
    assert(snapshot.minimumFreeLinearMemoryBytes == 700U * 1024U);

    HardwareFrameMetrics incomplete{};
    incomplete.eyeCount = 2U;
    incomplete.measured = HardwareMeasurement::Cpu;
    sampler.reset();
    sampler.record(incomplete);
    assert(!sampler.snapshot().measurementComplete());

    HardwareFrameMetrics invalid{};
    invalid.cpuMilliseconds = -2.0F;
    invalid.leftEyeMilliseconds = -3.0F;
    invalid.frameWaitMilliseconds = -1.0F;
    invalid.measured = HardwareMeasurement::Cpu | HardwareMeasurement::LeftEye |
        HardwareMeasurement::FrameWait;
    sampler.reset();
    sampler.record(invalid);
    assert(sampler.snapshot().last.cpuMilliseconds == 0.0F);
    assert(sampler.snapshot().last.leftEyeMilliseconds == 0.0F);
    assert(sampler.snapshot().last.frameWaitMilliseconds == 0.0F);

    return 0;
}
