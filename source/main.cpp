#include <3ds.h>
#include <citro3d.h>

#include <algorithm>
#include <cstdio>

#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Renderer.hpp"
#include "Wave.hpp"

namespace {

constexpr float kFixedStepSeconds = 1.0F / 60.0F;
constexpr float kMaximumFrameSeconds = 1.0F / 15.0F;
constexpr float kMaximumAccumulatorSeconds = kFixedStepSeconds * 5.0F;
constexpr float kRestartDelaySeconds = 1.5F;

float calculateFrameSeconds(u64 nowMilliseconds, u64 previousMilliseconds) {
    if (previousMilliseconds == 0U || nowMilliseconds <= previousMilliseconds) {
        return kFixedStepSeconds;
    }

    const float elapsed = static_cast<float>(nowMilliseconds - previousMilliseconds) / 1000.0F;
    return std::min(elapsed, kMaximumFrameSeconds);
}

int showStartupError(const char* stage, const char* detail, bool citro3dInitialized, bool romfsInitialized) {
    if (citro3dInitialized) {
        C3D_Fini();
    }
    if (romfsInitialized) {
        romfsExit();
    }

    consoleInit(GFX_TOP, nullptr);
    consoleClear();
    std::printf("CITADEL DEFENSE 3D\n");
    std::printf("==================\n\n");
    std::printf("BLAD STARTU: %s\n\n", stage != nullptr ? stage : "nieznany etap");
    if (detail != nullptr && detail[0] != '\0') {
        std::printf("%s\n\n", detail);
    }
    std::printf("Nacisnij START, aby zamknac.\n");
    std::printf("Zachowaj ten komunikat do diagnozy.\n");

    while (aptMainLoop()) {
        hidScanInput();
        if ((hidKeysDown() & KEY_START) != 0U) {
            break;
        }
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 1;
}

}  // namespace

int main() {
    gfxInitDefault();

    const Result romfsResult = romfsInit();
    if (R_FAILED(romfsResult)) {
        char detail[96];
        std::snprintf(detail, sizeof(detail), "romfsInit() = 0x%08lX", static_cast<unsigned long>(romfsResult));
        return showStartupError("ROMFS", detail, false, false);
    }

    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        return showStartupError(
            "CITRO3D",
            "C3D_Init nie przydzielil bufora polecen GPU.",
            false,
            true);
    }

    const LevelLoadResult levelResult = LevelLoader::loadFromRomFs("romfs:/levels/tutorial.lvl");
    if (!levelResult.success) {
        return showStartupError(
            "LADOWANIE POZIOMU",
            levelResult.error.c_str(),
            true,
            true);
    }

    Renderer renderer;
    if (!renderer.initialize(levelResult.level)) {
        renderer.shutdown();
        return showStartupError(
            "RENDERER",
            "Nie udalo sie utworzyc ekranow, shaderow lub bufora wierzcholkow.",
            true,
            true);
    }

    InputSystem inputSystem;
    Camera camera;
    Wave wave(levelResult.level);
    BuildSystem buildSystem(levelResult.level);
    float restartTimer = 0.0F;
    float simulationAccumulator = 0.0F;
    u64 previousMilliseconds = osGetTime();

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) {
            break;
        }

        const u64 nowMilliseconds = osGetTime();
        const float frameSeconds = calculateFrameSeconds(nowMilliseconds, previousMilliseconds);
        previousMilliseconds = nowMilliseconds;

        camera.update(input, frameSeconds);
        buildSystem.handleInput(input);

        simulationAccumulator = std::min(
            simulationAccumulator + frameSeconds,
            kMaximumAccumulatorSeconds);

        while (simulationAccumulator >= kFixedStepSeconds) {
            if (wave.completed() || wave.lost()) {
                restartTimer += kFixedStepSeconds;
                if (restartTimer >= kRestartDelaySeconds) {
                    wave.reset();
                    buildSystem.reset();
                    restartTimer = 0.0F;
                }
            } else {
                buildSystem.update(kFixedStepSeconds, wave);
                wave.update(kFixedStepSeconds);
            }
            simulationAccumulator -= kFixedStepSeconds;
        }

        renderer.render(camera, wave, buildSystem);
    }

    renderer.shutdown();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
