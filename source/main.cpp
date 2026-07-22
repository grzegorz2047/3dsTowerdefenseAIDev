#include <3ds.h>
#include <citro3d.h>

#include <algorithm>
#include <cstdio>

#include "AudioEvents.hpp"
#include "AudioSystem.hpp"
#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "HudMode.hpp"
#include "HudText.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Renderer.hpp"
#include "TouchGesture.hpp"
#include "TouchUiLayout.hpp"
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

void renderAudioDiagnostics(const AudioSystem& audioSystem) {
    std::printf("AUDIO:%s HLE-SHIM:%s\n", audioBackendName(audioSystem.backend()),
        audioSystem.ndspShimActive() ? "AKTYWNY" : (audioSystem.ndspShimAttempted() ? "ODRZUCONY" : "NIE"));
    std::printf("NDSP1:%08lX NDSP2:%08lX\n", static_cast<unsigned long>(audioSystem.ndspInitialResult()),
        static_cast<unsigned long>(audioSystem.ndspShimResult()));
    std::printf("CSND:%08lX PLAY:%08lX\n", static_cast<unsigned long>(audioSystem.csndResult()),
        static_cast<unsigned long>(audioSystem.lastPlayResult()));
    std::printf("CH:%d WBUF:%s POS:%lu/%lu\n", audioSystem.lastChannel(),
        audioWaveStatusName(audioSystem.diagnosticWaveStatus()),
        static_cast<unsigned long>(audioSystem.diagnosticSamplePosition()),
        static_cast<unsigned long>(audioSystem.diagnosticSampleCount()));
    std::printf("PROBE:%08lX ACTIVE:%s EVER:%s\n\n", static_cast<unsigned long>(audioSystem.probeResult()),
        audioSystem.channelActive() ? "TAK" : "NIE", audioSystem.channelEverActive() ? "TAK" : "NIE");
}

void renderTouchHud(PrintConsole& console, const Wave& wave, const BuildSystem& buildSystem,
    const TutorialFlow& tutorialFlow, const AudioSystem& audioSystem, HudMode hudMode,
    bool paused, int speedMultiplier) {
    consoleSelect(&console);
    std::printf("\x1b[2J\x1b[H");
    std::printf("\x1b[36m[KUSZA]   [MOZDZIERZ]   [MROZ]\x1b[0m\n");
    std::printf("Wybrano: %-10s Koszt:%-3d Zloto:%-3d\n", buildSystem.selectedTowerName(),
        buildSystem.towerCost(), buildSystem.gold());
    std::printf("Baza:%-2d Wieze:%-2zu Fala:%zu/%zu\n", wave.baseHealth(), buildSystem.towerCount(),
        wave.spawnedCount(), wave.enemyCount());
    std::printf("Pole:%zu,%zu  %s\n", buildSystem.cursorX(), buildSystem.cursorZ(),
        buildSystem.cursorOccupied() ? "WIEZA ZAZNACZONA" : "MIEJSCE BUDOWY");

    const Tower* tower = buildSystem.cursorTower();
    if (tower != nullptr) {
        std::printf("%s  POZIOM:%u  ULEPSZ:%d  SPRZEDAJ:%d\n", towerName(tower->type()),
            static_cast<unsigned int>(tower->level()), tower->upgradeCost(), tower->sellValue());
    } else {
        std::printf("Status: %s\n", buildAttemptMessage(buildSystem.lastBuildResult()));
    }

    std::printf("\nCO TERAZ: %s\n", tutorialInstruction(tutorialFlow.phase()));
    std::printf("Tryb: %s  Predkosc:%dx\n", paused ? "PAUZA" : "GRA", speedMultiplier);
    std::printf("\n[PAUZA/X] [1x-2x/L+R]       [ANULUJ]\n");
    std::printf("\n[BUDUJ/A] [ULEPSZ/B] [SPRZEDAJ/Y] [START/X]\n");

    if (showAudioDiagnostics(hudMode)) {
        std::printf("\n");
        renderAudioDiagnostics(audioSystem);
    } else {
        std::printf("\nDZWIEK: %s  SELECT: diagnostyka\n", audioSystem.available() ? "WLACZONY" : "WYLACZONY");
    }
}

void applyTouchAction(TouchUiAction action, BuildSystem& buildSystem, TutorialFlow& tutorialFlow,
    AudioSystem& audioSystem, bool& paused, int& speedMultiplier) {
    switch (action) {
        case TouchUiAction::SelectBallista:
            buildSystem.selectTowerType(TowerType::Ballista);
            break;
        case TouchUiAction::SelectMortar:
            buildSystem.selectTowerType(TowerType::Mortar);
            break;
        case TouchUiAction::SelectFrost:
            buildSystem.selectTowerType(TowerType::Frost);
            break;
        case TouchUiAction::BuildOrSelect:
            buildSystem.buildOrSelectCursor();
            audioSystem.play(cueForBuildResult(buildSystem.lastBuildResult()));
            break;
        case TouchUiAction::Upgrade:
            (void)buildSystem.upgradeCursorTower();
            break;
        case TouchUiAction::Sell:
            (void)buildSystem.sellCursorTower();
            break;
        case TouchUiAction::StartWave:
            (void)tutorialFlow.requestWaveStart(buildSystem.towerCount());
            paused = false;
            break;
        case TouchUiAction::TogglePause:
            if (tutorialFlow.waveRunning()) paused = !paused;
            break;
        case TouchUiAction::ToggleSpeed:
            speedMultiplier = speedMultiplier == 1 ? 2 : 1;
            break;
        case TouchUiAction::Cancel:
            buildSystem.cancelAction();
            break;
        case TouchUiAction::None:
        default:
            break;
    }
}

int showStartupError(const char* stage, const char* detail, bool citro3dInitialized, bool romfsInitialized) {
    if (citro3dInitialized) C3D_Fini();
    if (romfsInitialized) romfsExit();
    consoleInit(GFX_TOP, nullptr);
    consoleClear();
    std::printf("CITADEL DEFENSE 3D\n==================\n\nBLAD STARTU: %s\n\n",
        stage != nullptr ? stage : "nieznany etap");
    if (detail != nullptr && detail[0] != '\0') std::printf("%s\n\n", detail);
    std::printf("Nacisnij START, aby zamknac.\n");
    while (aptMainLoop()) {
        hidScanInput();
        if ((hidKeysDown() & KEY_START) != 0U) break;
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
        return showStartupError("CITRO3D", "C3D_Init nie przydzielil bufora polecen GPU.", false, true);
    }

    const LevelLoadResult levelResult = LevelLoader::loadFromRomFs("romfs:/levels/tutorial.lvl");
    if (!levelResult.success) return showStartupError("LADOWANIE POZIOMU", levelResult.error.c_str(), true, true);

    Renderer renderer;
    if (!renderer.initialize(levelResult.level)) {
        renderer.shutdown();
        return showStartupError("RENDERER", "Nie udalo sie utworzyc ekranow, shaderow lub bufora wierzcholkow.", true, true);
    }

    PrintConsole bottomConsole{};
    consoleInit(GFX_BOTTOM, &bottomConsole);
    consoleSelect(&bottomConsole);
    consoleClear();

    InputSystem inputSystem;
    TouchGesture touchGesture;
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
    bool paused = false;
    int speedMultiplier = 1;

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) break;

        const HudMode hudMode = hudModeForSelectHeld(input.isHeld(KEY_SELECT));
        if (allowDiagnosticTone(hudMode, input.pressed(KEY_B))) audioSystem.playDiagnosticTone();

        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        if (gesture.tapped && !tutorialFlow.finished()) {
            applyTouchAction(TouchUiLayout::actionAt(gesture.endX, gesture.endY), buildSystem,
                tutorialFlow, audioSystem, paused, speedMultiplier);
        }

        if (input.pressed(KEY_X)) {
            if (tutorialFlow.waveRunning()) paused = !paused;
            else {
                (void)tutorialFlow.requestWaveStart(buildSystem.towerCount());
                paused = false;
            }
        }
        if (input.pressed(KEY_L) && input.pressed(KEY_R)) speedMultiplier = speedMultiplier == 1 ? 2 : 1;

        if (input.pressed(KEY_Y) && tutorialFlow.finished()) {
            audioSystem.stopAll();
            wave.reset();
            buildSystem.reset();
            tutorialFlow.reset();
            audioRouter.reset({tutorialFlow.phase(), 0U});
            simulationAccumulator = 0.0F;
            paused = false;
            speedMultiplier = 1;
        }

        const u64 nowMilliseconds = osGetTime();
        const float frameSeconds = calculateFrameSeconds(nowMilliseconds, previousMilliseconds);
        previousMilliseconds = nowMilliseconds;

        camera.update(input, frameSeconds);
        if (!tutorialFlow.finished()) {
            buildSystem.handleInput(input);
            if (input.pressed(KEY_A)) audioSystem.play(cueForBuildResult(buildSystem.lastBuildResult()));
        }
        tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());

        if (!paused) {
            simulationAccumulator = std::min(simulationAccumulator + frameSeconds * static_cast<float>(speedMultiplier),
                kMaximumAccumulatorSeconds);
        }
        while (simulationAccumulator >= kFixedStepSeconds) {
            if (tutorialFlow.waveRunning()) {
                buildSystem.update(kFixedStepSeconds, wave);
                wave.update(kFixedStepSeconds);
                tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());
            }
            simulationAccumulator -= kFixedStepSeconds;
        }

        const AudioFrameState audioState{tutorialFlow.phase(), buildSystem.projectiles().activeCount()};
        audioSystem.playMask(audioRouter.update(audioState));
        audioSystem.updateProbe();

        renderTouchHud(bottomConsole, wave, buildSystem, tutorialFlow, audioSystem, hudMode, paused, speedMultiplier);
        renderer.render(camera, wave, buildSystem, tutorialFlow);
    }

    audioSystem.shutdown();
    renderer.shutdown();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
