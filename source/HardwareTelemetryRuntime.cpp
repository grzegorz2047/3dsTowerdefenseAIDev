#include "HardwareTelemetryRuntime.hpp"

#include <3ds.h>
#include <citro3d.h>

#include <algorithm>
#include <cstdint>

namespace {

HardwareTelemetrySampler gSampler;
HardwareRenderCounter gCounters;
HardwareFrameMetrics gCurrent{};
HardwareFrameMetrics gPending{};
HardwareDeviceProfile gDeviceProfile = HardwareDeviceProfile::Unknown;
C3D_RenderTarget* gCurrentTarget = nullptr;
bool gFrameActive = false;
bool gPendingReady = false;

float elapsedMilliseconds(u64 startTicks, u64 endTicks) {
    if (endTicks <= startTicks) return 0.0F;
    return static_cast<float>(endTicks - startTicks) /
        static_cast<float>(CPU_TICKS_PER_MSEC);
}

void addSceneTime(float milliseconds) {
    if (gCurrentTarget == nullptr || gCurrentTarget->screen != GFX_TOP) return;
    if (gCurrentTarget->side == GFX_RIGHT) {
        gCurrent.rightEyeMilliseconds += milliseconds;
        gCurrent.measured |= HardwareMeasurement::RightEye;
    } else {
        gCurrent.leftEyeMilliseconds += milliseconds;
        gCurrent.measured |= HardwareMeasurement::LeftEye;
    }
}

void addUiTime(float milliseconds) {
    if (gCurrentTarget == nullptr) return;
    if (gCurrentTarget->screen == GFX_BOTTOM) {
        gCurrent.bottomUiMilliseconds += milliseconds;
        gCurrent.measured |= HardwareMeasurement::BottomUi;
    } else if (gCurrentTarget->screen == GFX_TOP) {
        gCurrent.topUiMilliseconds += milliseconds;
        gCurrent.measured |= HardwareMeasurement::TopUi;
    }
}

void publishCompletedGpuFrame() {
    if (!gPendingReady) return;
    gPending.gpuDrawingMilliseconds = std::max(C3D_GetDrawingTime(), 0.0F);
    gPending.measured |= HardwareMeasurement::GpuDrawing;
    gSampler.record(gPending);
    gPendingReady = false;
}

}  // namespace

extern "C" bool __real_C3D_FrameBegin(u8 flags);
extern "C" bool __real_C3D_FrameDrawOn(C3D_RenderTarget* target);
extern "C" void __real_C3D_FrameEnd(u8 flags);
extern "C" void __real_C3D_DrawArrays(GPU_Primitive_t primitive, int first, int size);
extern "C" void __real_C3D_DrawElements(
    GPU_Primitive_t primitive, int count, int type, const void* indices);
extern "C" void __real_C3D_SyncTextureCopy(
    u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags);

extern "C" bool __wrap_C3D_FrameBegin(u8 flags) {
    const u64 start = svcGetSystemTick();
    const bool result = __real_C3D_FrameBegin(flags);
    const u64 end = svcGetSystemTick();
    if (result && gFrameActive) {
        gCurrent.frameWaitMilliseconds = elapsedMilliseconds(start, end);
        gCurrent.measured |= HardwareMeasurement::FrameWait;
        publishCompletedGpuFrame();
    }
    return result;
}

extern "C" bool __wrap_C3D_FrameDrawOn(C3D_RenderTarget* target) {
    gCurrentTarget = target;
    return __real_C3D_FrameDrawOn(target);
}

extern "C" void __wrap_C3D_FrameEnd(u8 flags) {
    __real_C3D_FrameEnd(flags);
    if (!gFrameActive) return;
    gCurrent.citroProcessingMilliseconds = std::max(C3D_GetProcessingTime(), 0.0F);
    gCurrent.measured |= HardwareMeasurement::CitroProcessing;
}

extern "C" void __wrap_C3D_DrawArrays(
    GPU_Primitive_t primitive, int first, int size) {
    const u64 start = svcGetSystemTick();
    __real_C3D_DrawArrays(primitive, first, size);
    const u64 end = svcGetSystemTick();
    if (!gFrameActive) return;
    const std::uint32_t vertices = size > 0 ? static_cast<std::uint32_t>(size) : 0U;
    gCounters.recordSceneDraw(vertices);
    addSceneTime(elapsedMilliseconds(start, end));
}

extern "C" void __wrap_C3D_DrawElements(
    GPU_Primitive_t primitive, int count, int type, const void* indices) {
    const u64 start = svcGetSystemTick();
    __real_C3D_DrawElements(primitive, count, type, indices);
    const u64 end = svcGetSystemTick();
    if (!gFrameActive) return;
    gCounters.recordUiDraw();
    addUiTime(elapsedMilliseconds(start, end));
}

extern "C" void __wrap_C3D_SyncTextureCopy(
    u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags) {
    __real_C3D_SyncTextureCopy(inadr, indim, outadr, outdim, size, flags);
    if (gFrameActive) gCounters.recordTextureUpload(static_cast<std::size_t>(size));
}

void hardwareTelemetryInitializeDeviceProfile() {
    bool isNew3DS = false;
    const Result result = APT_CheckNew3DS(&isNew3DS);
    gDeviceProfile = R_SUCCEEDED(result)
        ? (isNew3DS ? HardwareDeviceProfile::New3DS : HardwareDeviceProfile::Old3DS)
        : HardwareDeviceProfile::Unknown;
}

void hardwareTelemetryBeginFrame(float gameCpuMilliseconds) {
    gCurrent = {};
    gCounters.reset();
    gCurrentTarget = nullptr;
    gCurrent.cpuMilliseconds = std::max(gameCpuMilliseconds, 0.0F);
    gCurrent.deviceProfile = gDeviceProfile;
    gCurrent.speedupEnabled = false;
    gCurrent.measured = HardwareMeasurement::Cpu | HardwareMeasurement::DeviceProfile;
    gFrameActive = true;
}

void hardwareTelemetryFinishFrame(std::size_t freeLinearMemoryBytes, std::uint8_t eyeCount) {
    if (!gFrameActive) return;
    gCurrent.freeLinearMemoryBytes = freeLinearMemoryBytes;
    gCurrent.eyeCount = eyeCount > 1U ? 2U : 1U;
    gCurrent.measured |= HardwareMeasurement::LinearMemory;
    gCounters.applyTo(gCurrent);
    gPending = gCurrent;
    gPendingReady = true;
    gFrameActive = false;
    gCurrentTarget = nullptr;
}

const HardwareTelemetrySnapshot& hardwareTelemetrySnapshot() {
    return gSampler.snapshot();
}
