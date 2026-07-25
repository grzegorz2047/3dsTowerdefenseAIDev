#include <cassert>
#include <cstddef>

#include "HardwareTelemetry.hpp"

int main() {
    HardwareTelemetrySampler sampler;

    HardwareFrameMetrics first{};
    first.cpuMilliseconds = 4.0F;
    first.leftEyeMilliseconds = 6.0F;
    first.rightEyeMilliseconds = 5.0F;
    first.topUiMilliseconds = 1.0F;
    first.bottomUiMilliseconds = 2.0F;
    first.sceneDrawCalls = 20U;
    first.uiDrawCalls = 8U;
    first.submittedVertices = 1500U;
    first.textureUploads = 1U;
    first.textureUploadBytes = 4096U;
    first.freeLinearMemoryBytes = 900U * 1024U;
    first.eyeCount = 2U;
    sampler.record(first);

    HardwareFrameMetrics second{};
    second.cpuMilliseconds = 8.0F;
    second.leftEyeMilliseconds = 7.0F;
    second.rightEyeMilliseconds = 0.0F;
    second.topUiMilliseconds = 1.5F;
    second.bottomUiMilliseconds = 2.5F;
    second.sceneDrawCalls = 12U;
    second.uiDrawCalls = 7U;
    second.submittedVertices = 900U;
    second.freeLinearMemoryBytes = 700U * 1024U;
    second.eyeCount = 1U;
    sampler.record(second);

    const HardwareTelemetrySnapshot& snapshot = sampler.snapshot();
    assert(snapshot.sampleCount == 2U);
    assert(snapshot.averageCpuMilliseconds == 6.0F);
    assert(snapshot.worstCpuMilliseconds == 8.0F);
    assert(snapshot.averageGpuWorkMilliseconds == 12.5F);
    assert(snapshot.worstGpuWorkMilliseconds == 14.0F);
    assert(snapshot.minimumFreeLinearMemoryBytes == 700U * 1024U);
    assert(snapshot.accumulatedSceneDrawCalls == 32U);
    assert(snapshot.accumulatedUiDrawCalls == 15U);
    assert(snapshot.accumulatedVertices == 2400U);
    assert(snapshot.accumulatedTextureUploads == 1U);
    assert(snapshot.accumulatedTextureUploadBytes == 4096U);
    assert(snapshot.last.eyeCount == 1U);

    HardwareFrameMetrics invalid{};
    invalid.cpuMilliseconds = -2.0F;
    invalid.leftEyeMilliseconds = -3.0F;
    invalid.frameWaitMilliseconds = -1.0F;
    invalid.freeLinearMemoryBytes = 800U * 1024U;
    sampler.reset();
    sampler.record(invalid);
    assert(sampler.snapshot().last.cpuMilliseconds == 0.0F);
    assert(sampler.snapshot().last.leftEyeMilliseconds == 0.0F);
    assert(sampler.snapshot().last.frameWaitMilliseconds == 0.0F);
    assert(sampler.snapshot().minimumFreeLinearMemoryBytes == 800U * 1024U);

    return 0;
}
