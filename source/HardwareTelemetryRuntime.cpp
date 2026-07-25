#include "HardwareTelemetryRuntime.hpp"

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "PerformanceBudget.hpp"

namespace {

HardwareTelemetrySampler gSampler;
HardwareRenderCounter gCounters;
HardwareFrameMetrics gCurrent{};
HardwareFrameMetrics gPending{};
HardwareDeviceProfile gDeviceProfile = HardwareDeviceProfile::Unknown;
C3D_RenderTarget* gCurrentTarget = nullptr;
C2D_TextBuf gOverlayTextBuffer = nullptr;
u64 gPreviousFrameEndTicks = 0U;
bool gDeviceProfileInitialized = false;
bool gFrameActive = false;
bool gPendingReady = false;
bool gRightEyeObserved = false;
bool gSceneObserved = false;
bool gOverlayDrawing = false;

constexpr u32 kOverlayBackground = 0xF5261D14U;
constexpr u32 kOverlayText = 0xFFF4F0ECU;
constexpr u32 kOverlayMuted = 0xFFD5CABEU;
constexpr u32 kOverlayPass = 0xFF8FE0A0U;
constexpr u32 kOverlayWarn = 0xFF70DEFFU;
constexpr u32 kOverlayFail = 0xFF8484FFU;

float elapsedMilliseconds(u64 startTicks, u64 endTicks) {
    if (endTicks <= startTicks) return 0.0F;
    return static_cast<float>(endTicks - startTicks) /
        static_cast<float>(CPU_TICKS_PER_MSEC);
}

void initializeDeviceProfile() {
    if (gDeviceProfileInitialized) return;
    bool isNew3DS = false;
    const Result result = APT_CheckNew3DS(&isNew3DS);
    gDeviceProfile = R_SUCCEEDED(result)
        ? (isNew3DS ? HardwareDeviceProfile::New3DS : HardwareDeviceProfile::Old3DS)
        : HardwareDeviceProfile::Unknown;
    gDeviceProfileInitialized = true;
}

void beginCurrentFrame(float frameWaitMilliseconds, float gameCpuMilliseconds, bool cpuMeasured) {
    initializeDeviceProfile();
    gCurrent = {};
    gCounters.reset();
    gCurrentTarget = nullptr;
    gRightEyeObserved = false;
    gSceneObserved = false;
    gCurrent.cpuMilliseconds = gameCpuMilliseconds;
    gCurrent.frameWaitMilliseconds = frameWaitMilliseconds;
    gCurrent.deviceProfile = gDeviceProfile;
    gCurrent.speedupEnabled = false;
    gCurrent.measured = HardwareMeasurement::FrameWait |
        HardwareMeasurement::DeviceProfile;
    if (cpuMeasured) gCurrent.measured |= HardwareMeasurement::Cpu;
    gFrameActive = true;
}

void addSceneTime(float milliseconds) {
    if (gCurrentTarget == nullptr || gCurrentTarget->screen != GFX_TOP) return;
    if (gCurrentTarget->side == GFX_RIGHT) {
        gRightEyeObserved = true;
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
        if (gCurrentTarget->side == GFX_RIGHT) gRightEyeObserved = true;
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

void finishCurrentFrame() {
    if (!gFrameActive) return;
    gCurrent.citroProcessingMilliseconds = std::max(C3D_GetProcessingTime(), 0.0F);
    gCurrent.freeLinearMemoryBytes = static_cast<std::size_t>(linearSpaceFree());
    gCurrent.eyeCount = gRightEyeObserved ? 2U : 1U;
    gCurrent.measured |= HardwareMeasurement::CitroProcessing |
        HardwareMeasurement::LinearMemory;
    gCounters.applyTo(gCurrent);
    gPending = gCurrent;
    gPendingReady = true;
    gFrameActive = false;
    gCurrentTarget = nullptr;
}

const char* hardwareVerdict(const HardwareTelemetrySnapshot& snapshot) {
    if (snapshot.sampleCount == 0U || !snapshot.measurementComplete()) return "WARN";
    const float gpuBudget = snapshot.stereoObserved
        ? PerformanceBudget::kStereoRenderBudgetMilliseconds
        : PerformanceBudget::kMonoRenderBudgetMilliseconds;
    const float worstCpuWork = snapshot.worstCpuMilliseconds +
        snapshot.worstCitroProcessingMilliseconds;
    if (snapshot.minimumFreeLinearMemoryBytes <
            PerformanceBudget::kMinimumLinearMemoryReserveBytes ||
        snapshot.worstGpuDrawingMilliseconds > gpuBudget ||
        worstCpuWork > PerformanceBudget::kWarningFrameMilliseconds) {
        return "FAIL";
    }
    if (worstCpuWork > PerformanceBudget::kTargetFrameMilliseconds) return "WARN";
    return "PASS";
}

u32 hardwareVerdictColor(const char* verdict) {
    if (verdict[0] == 'P') return kOverlayPass;
    if (verdict[0] == 'F') return kOverlayFail;
    return kOverlayWarn;
}

bool shouldDrawOverlay() {
    return gFrameActive && gSceneObserved && !gOverlayDrawing &&
        gCurrentTarget != nullptr && gCurrentTarget->screen == GFX_BOTTOM &&
        (hidKeysHeld() & KEY_SELECT) != 0U;
}

bool ensureOverlayTextBuffer() {
    if (gOverlayTextBuffer != nullptr) return true;
    gOverlayTextBuffer = C2D_TextBufNew(1024U);
    return gOverlayTextBuffer != nullptr;
}

void drawOverlayLine(const char* text, float y, u32 color) {
    C2D_Text parsed{};
    C2D_TextParse(&parsed, gOverlayTextBuffer, text);
    C2D_TextOptimize(&parsed);
    C2D_DrawText(&parsed, C2D_WithColor, 12.0F, y, 0.98F, 0.39F, 0.39F, color);
}

void drawHardwareOverlay() {
    if (!ensureOverlayTextBuffer()) return;
    gOverlayDrawing = true;
    C2D_TextBufClear(gOverlayTextBuffer);
    C2D_DrawRectSolid(8.0F, 124.0F, 0.96F, 304.0F, 110.0F, kOverlayBackground);

    const HardwareTelemetrySnapshot& snapshot = gSampler.snapshot();
    const HardwareFrameMetrics& last = snapshot.last;
    const char* verdict = hardwareVerdict(snapshot);
    char line[96]{};

    std::snprintf(line, sizeof(line), "HW %s %s %s", hardwareDeviceProfileName(last.deviceProfile),
        verdict, snapshot.measurementComplete() ? "KOMPLET" : "NIEPELNY");
    drawOverlayLine(line, 130.0F, hardwareVerdictColor(verdict));

    std::snprintf(line, sizeof(line), "CPU %.1f C3D %.1f GPU %.1f WAIT %.1f",
        last.cpuMilliseconds, last.citroProcessingMilliseconds,
        last.gpuDrawingMilliseconds, last.frameWaitMilliseconds);
    drawOverlayLine(line, 150.0F, kOverlayText);

    std::snprintf(line, sizeof(line), "L %.1f R %.1f UI %.1f/%.1f O %u",
        last.leftEyeMilliseconds, last.rightEyeMilliseconds,
        last.topUiMilliseconds, last.bottomUiMilliseconds,
        static_cast<unsigned int>(last.eyeCount));
    drawOverlayLine(line, 170.0F, kOverlayText);

    std::snprintf(line, sizeof(line), "DRAW %lu/%lu VTX %lu",
        static_cast<unsigned long>(last.sceneDrawCalls),
        static_cast<unsigned long>(last.uiDrawCalls),
        static_cast<unsigned long>(last.submittedVertices));
    drawOverlayLine(line, 190.0F, kOverlayMuted);

    std::snprintf(line, sizeof(line), "MEM MIN %luK TEX %lu/%luK N %lu",
        static_cast<unsigned long>(snapshot.minimumFreeLinearMemoryBytes / 1024U),
        static_cast<unsigned long>(last.textureUploads),
        static_cast<unsigned long>(last.textureUploadBytes / 1024U),
        static_cast<unsigned long>(snapshot.sampleCount));
    drawOverlayLine(line, 210.0F, kOverlayMuted);
    gOverlayDrawing = false;
}

void releaseOverlayTextBuffer() {
    if (gOverlayTextBuffer == nullptr) return;
    C2D_TextBufDelete(gOverlayTextBuffer);
    gOverlayTextBuffer = nullptr;
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
extern "C" void __real_C2D_Flush(void);
extern "C" void __real_C2D_Fini(void);

extern "C" bool __wrap_C3D_FrameBegin(u8 flags) {
    const u64 start = svcGetSystemTick();
    const bool result = __real_C3D_FrameBegin(flags);
    const u64 end = svcGetSystemTick();
    if (result) {
        publishCompletedGpuFrame();
        const bool cpuMeasured = gPreviousFrameEndTicks != 0U;
        const float gameCpu = cpuMeasured
            ? elapsedMilliseconds(gPreviousFrameEndTicks, start)
            : 0.0F;
        beginCurrentFrame(elapsedMilliseconds(start, end), gameCpu, cpuMeasured);
    }
    return result;
}

extern "C" bool __wrap_C3D_FrameDrawOn(C3D_RenderTarget* target) {
    gCurrentTarget = target;
    if (target != nullptr && target->screen == GFX_TOP && target->side == GFX_RIGHT) {
        gRightEyeObserved = true;
    }
    return __real_C3D_FrameDrawOn(target);
}

extern "C" void __wrap_C3D_FrameEnd(u8 flags) {
    __real_C3D_FrameEnd(flags);
    const u64 end = svcGetSystemTick();
    finishCurrentFrame();
    gPreviousFrameEndTicks = end;
}

extern "C" void __wrap_C3D_DrawArrays(
    GPU_Primitive_t primitive, int first, int size) {
    const u64 start = svcGetSystemTick();
    __real_C3D_DrawArrays(primitive, first, size);
    const u64 end = svcGetSystemTick();
    if (!gFrameActive) return;
    const std::uint32_t vertices = size > 0 ? static_cast<std::uint32_t>(size) : 0U;
    gCounters.recordSceneDraw(vertices);
    gSceneObserved = true;
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

extern "C" void __wrap_C2D_Flush(void) {
    if (shouldDrawOverlay()) drawHardwareOverlay();
    __real_C2D_Flush();
}

extern "C" void __wrap_C2D_Fini(void) {
    releaseOverlayTextBuffer();
    __real_C2D_Fini();
}

void hardwareTelemetryRecordGameCpu(float milliseconds) {
    if (!gFrameActive) return;
    gCurrent.cpuMilliseconds = std::max(milliseconds, 0.0F);
    gCurrent.measured |= HardwareMeasurement::Cpu;
}

const HardwareTelemetrySnapshot& hardwareTelemetrySnapshot() {
    return gSampler.snapshot();
}
