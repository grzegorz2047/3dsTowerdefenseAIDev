#include <3ds.h>
#include <citro3d.h>

#include <algorithm>
#include <cstdio>

#include "AudioEvents.hpp"
#include "AudioSystem.hpp"
#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "HudText.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Renderer.hpp"
#include "TutorialFlow.hpp"
#include "Wave.hpp"

namespace {

constexpr float kFixedStepSeconds = 1.0F / 60.0F;
constexpr float kMaximumFrameSeconds = 1.0F / 15.0F;
constexpr float kMaximumAccumulatorSeconds = kFixedStepSeconds * 5.0F;

float calculateFrameSeconds(u64 nowMilliseconds, u64 previousMilliseconds) {
    if (previousMilliseconds == 0U || nowMilliseconds <= previousMilliseconds) {
        return kFixedStepSeconds;
    }

    const float elapsed = static_cast<float>(nowMilliseconds - previousMilliseconds) / 1000.0F;
    return std::min(elapsed, kMaximumFrameSeconds);
}

void renderFallbackHud(
    PrintConsole& console,
    const Wave& wave,
    const BuildSystem& buildSystem,
    const TutorialFlow& tutorialFlow,
    const AudioSystem& audioSystem) {
    consoleSelect(&console);
    std::printf("\x1b[2J\x1b[H");
    std::printf("\x1b[36mCITADEL DEFENSE 3D\x1b[0m\n");
    std::printf("v0.1.13-alpha  AUDIO-PROBE\n");
    std::printf("==============================\n");
    std::printf("AUDIO: %s\n", audioBackendName(audioSystem.backend()));
    std::printf(
        "NDSP:%08lX CSND:%08lX PLAY:%08lX\n",
        static_cast<unsigned long>(audioSystem.ndspResult()),
        static_cast<unsigned long>(audioSystem.csndResult()),
        static_cast<unsigned long>(audioSystem.lastPlayResult()));
    std::printf(
        "CH:%d PROBE:%08lX ACTIVE:%s EVER:%s\n\n",
        audioSystem.lastChannel(),
        static_cast<unsigned long>(audioSystem.probeResult()),
        audioSystem.channelActive() ? "TAK" : "NIE",
        audioSystem.channelEverActive() ? "TAK" : "NIE");

    std::printf("\x1b[33mCO TERAZ:\x1b[0m\n%s\n\n", tutorialInstruction(tutorialFlow.phase()));
    std::printf(
        "BAZA: %-2d  ZLOTO: %-3d  KOSZT: %d\n",
        wave.baseHealth(),
        buildSystem.gold(),
        buildSystem.towerCost());
    std::printf(
        "WIEZE: %-2zu FALA: %zu/%zu  POLE: %zu,%zu\n\n",
        buildSystem.towerCount(),
        wave.spawnedCount(),
        wave.enemyCount(),
        buildSystem.cursorX(),
        buildSystem.cursorZ());

    std::printf("STATUS: %s\n\n", buildAttemptMessage(buildSystem.lastBuildResult()));
    std::printf("D-PAD  wybierz niebieskie pole\n");
    std::printf("A      zbuduj wieze / efekt\n");
    std::printf("B      2 sekundy tonu 880 Hz\n");
    std::printf("X      uruchom fale\n");
    std::printf("Y      restart po wyniku\n");
    std::printf("START  wyjscie\n");
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

    PrintConsole bottomConsole{};
    consoleInit(GFX_BOTTOM, &bottomConsole);
    consoleSelect(&bottomConsole);
    consoleClear();

    InputSystem inputSystem;
    Camera camera;
    Wave wave(levelResult.level);
    BuildSystem buildSystem(levelResult.level);
    TutorialFlow tutorialFlow;
    AudioSystem audioSystem;
    audioSystem.initialize();
    AudioEventRouter audioRouter;
    audioRouter.reset({tutorialFlow.phase(), buildSystem.projectiles().activeCount()});

    float simulationAccumulator = 0.0F;
    u64 previousMilliseconds = osGetTime();

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) {
            break;
        }

        if (input.pressed(KEY_B)) {
            audioSystem.playDiagnosticTone();
        }
        if (input.pressed(KEY_X)) {
            tutorialFlow.requestWaveStart(buildSystem.towerCount());
        }
        if (input.pressed(KEY_Y) && tutorialFlow.finished()) {
            audioSystem.stopAll();
            wave.reset();
            buildSystem.reset();
            tutorialFlow.reset();
            audioRouter.reset({tutorialFlow.phase(), 0U});
            simulationAccumulator = 0.0F;
        }

        const u64 nowMilliseconds = osGetTime();
        const float frameSeconds = calculateFrameSeconds(nowMilliseconds, previousMilliseconds);
        previousMilliseconds = nowMilliseconds;

        camera.update(input, frameSeconds);
        if (!tutorialFlow.finished()) {
            buildSystem.handleInput(input);
            if (input.pressed(KEY_A)) {
                audioSystem.play(cueForBuildResult(buildSystem.lastBuildResult()));
            }
        }
        tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());

        simulationAccumulator = std::min(
            simulationAccumulator + frameSeconds,
            kMaximumAccumulatorSeconds);

        while (simulationAccumulator >= kFixedStepSeconds) {
            if (tutorialFlow.waveRunning()) {
                buildSystem.update(kFixedStepSeconds, wave);
                wave.update(kFixedStepSeconds);
                tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());
            }
            simulationAccumulator -= kFixedStepSeconds;
        }

        const AudioFrameState audioState{
            tutorialFlow.phase(),
            buildSystem.projectiles().activeCount()};
        audioSystem.playMask(audioRouter.update(audioState));
        audioSystem.updateProbe();

        renderFallbackHud(bottomConsole, wave, buildSystem, tutorialFlow, audioSystem);
        renderer.render(camera, wave, buildSystem, tutorialFlow);
    }

    audioSystem.shutdown();
    renderer.shutdown();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
