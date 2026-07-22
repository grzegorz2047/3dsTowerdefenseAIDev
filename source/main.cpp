#include <3ds.h>
#include <citro3d.h>

#include <algorithm>
#include <cstdio>

#include "AudioEvents.hpp"
#include "AudioSystem.hpp"
#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "Campaign.hpp"
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

enum class MissionSessionAction {
    ReturnToCampaign,
    Replay,
    ExitApplication,
};

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

void renderCampaignMenu(PrintConsole& console, const CampaignProgress& progress, std::size_t selected) {
    consoleSelect(&console);
    std::printf("\x1b[2J\x1b[H");
    std::printf("\x1b[36mCITADEL DEFENSE 3D - KAMPANIA\x1b[0m\n\n");

    const auto& missions = CampaignCatalog::missions();
    for (std::size_t index = 0; index < missions.size(); ++index) {
        const bool unlocked = progress.unlocked(index);
        std::printf("%c %zu. %-19s %s %u/3\n",
            index == selected ? '>' : ' ',
            index + 1U,
            unlocked ? missions[index].title : "ZABLOKOWANE",
            unlocked ? "*" : "-",
            static_cast<unsigned int>(progress.bestStars(index)));
    }

    const CampaignMission& mission = missions[selected];
    std::printf("\nCEL: %s\n", mission.objective);
    std::printf("WIEZE: %s\n", mission.availableTowers);
    std::printf("ZAGROZENIA: %s\n", mission.threats);
    std::printf("NAGRODA: %s\n", mission.reward);
    std::printf("\nGWIAZDKI: 1 zwyciestwo, +1 zdrowie bazy >= %u,\n+1 maksymalnie %u wiez. Cele dodatkowe nie blokuja postepu.\n",
        static_cast<unsigned int>(mission.fullHealthThreshold),
        static_cast<unsigned int>(mission.efficientTowerLimit));
    std::printf("\nD-PAD: WYBOR   A: GRAJ   START: WYJSCIE\n");
}

std::size_t selectCampaignMission(PrintConsole& console, const CampaignProgress& progress, std::size_t selected) {
    InputSystem inputSystem;
    const std::size_t selectableCount = progress.unlockedCount();
    if (selectableCount == 0) {
        return kCampaignMissionCount;
    }
    selected = std::min(selected, selectableCount - 1U);

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) {
            return kCampaignMissionCount;
        }
        if (input.pressed(KEY_DUP) || input.pressed(KEY_DLEFT)) {
            selected = selected == 0 ? selectableCount - 1U : selected - 1U;
        }
        if (input.pressed(KEY_DDOWN) || input.pressed(KEY_DRIGHT)) {
            selected = (selected + 1U) % selectableCount;
        }

        renderCampaignMenu(console, progress, selected);
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();

        if (input.pressed(KEY_A)) {
            return selected;
        }
    }
    return kCampaignMissionCount;
}

void renderTouchHud(PrintConsole& console, const CampaignMission& mission, const Wave& wave,
    const BuildSystem& buildSystem, const TutorialFlow& tutorialFlow, const AudioSystem& audioSystem,
    HudMode hudMode, bool paused, int speedMultiplier, const MissionResult& missionResult) {
    consoleSelect(&console);
    std::printf("\x1b[2J\x1b[H");
    std::printf("\x1b[36m%s\x1b[0m\n", mission.title);
    std::printf("[KUSZA]   [MOZDZIERZ]   [MROZ]\n");
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
    if (tutorialFlow.finished()) {
        std::printf("WYNIK: %u/3 GWIAZDKI\n", static_cast<unsigned int>(missionResult.stars));
        std::printf("\n[X: MAPA KAMPANII]       [Y: POWTORZ]\n");
    } else {
        std::printf("Tryb: %s  Predkosc:%dx\n", paused ? "PAUZA" : "GRA", speedMultiplier);
        std::printf("\n[PAUZA/X] [1x-2x/L+R]       [ANULUJ]\n");
        std::printf("\n[BUDUJ/A] [ULEPSZ/B] [SPRZEDAJ/Y] [START/X]\n");
    }

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

MissionSessionAction runMission(PrintConsole& bottomConsole, std::size_t missionIndex, CampaignProgress& progress) {
    const CampaignMission& mission = CampaignCatalog::mission(missionIndex);
    const LevelLoadResult levelResult = LevelLoader::loadFromRomFs(mission.levelPath);
    if (!levelResult.success) {
        consoleSelect(&bottomConsole);
        std::printf("\x1b[2J\x1b[HBLAD POZIOMU: %s\nSTART: WYJSCIE\n", levelResult.error.c_str());
        while (aptMainLoop()) {
            hidScanInput();
            if ((hidKeysDown() & KEY_START) != 0U) break;
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
        return MissionSessionAction::ExitApplication;
    }

    Renderer renderer;
    if (!renderer.initialize(levelResult.level)) {
        renderer.shutdown();
        return MissionSessionAction::ExitApplication;
    }

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
    bool resultRecorded = false;
    MissionResult missionResult{};
    MissionSessionAction action = MissionSessionAction::ExitApplication;

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) {
            action = MissionSessionAction::ExitApplication;
            break;
        }

        if (tutorialFlow.finished()) {
            if (input.pressed(KEY_X)) {
                action = MissionSessionAction::ReturnToCampaign;
                break;
            }
            if (input.pressed(KEY_Y)) {
                action = MissionSessionAction::Replay;
                break;
            }
        }

        const HudMode hudMode = hudModeForSelectHeld(input.isHeld(KEY_SELECT));
        if (allowDiagnosticTone(hudMode, input.pressed(KEY_B))) audioSystem.playDiagnosticTone();

        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        if (gesture.tapped && !tutorialFlow.finished()) {
            applyTouchAction(TouchUiLayout::actionAt(gesture.endX, gesture.endY), buildSystem,
                tutorialFlow, audioSystem, paused, speedMultiplier);
        }

        if (!tutorialFlow.finished() && input.pressed(KEY_X)) {
            if (tutorialFlow.waveRunning()) paused = !paused;
            else {
                (void)tutorialFlow.requestWaveStart(buildSystem.towerCount());
                paused = false;
            }
        }
        if (!tutorialFlow.finished() && input.pressed(KEY_L) && input.pressed(KEY_R)) {
            speedMultiplier = speedMultiplier == 1 ? 2 : 1;
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

        if (tutorialFlow.finished() && !resultRecorded) {
            if (tutorialFlow.phase() == TutorialPhase::Victory) {
                missionResult = progress.complete(missionIndex, wave.baseHealth(), buildSystem.towerCount());
            }
            resultRecorded = true;
        }

        const AudioFrameState audioState{tutorialFlow.phase(), buildSystem.projectiles().activeCount()};
        audioSystem.playMask(audioRouter.update(audioState));
        audioSystem.updateProbe();

        renderTouchHud(bottomConsole, mission, wave, buildSystem, tutorialFlow, audioSystem,
            hudMode, paused, speedMultiplier, missionResult);
        renderer.render(camera, wave, buildSystem, tutorialFlow);
    }

    audioSystem.shutdown();
    renderer.shutdown();
    return action;
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

    PrintConsole bottomConsole{};
    consoleInit(GFX_BOTTOM, &bottomConsole);
    consoleSelect(&bottomConsole);
    consoleClear();

    CampaignProgress progress;
    std::size_t selectedMission = 0;
    bool exitApplication = false;

    while (aptMainLoop() && !exitApplication) {
        selectedMission = selectCampaignMission(bottomConsole, progress, selectedMission);
        if (selectedMission >= kCampaignMissionCount) {
            break;
        }

        bool replay = true;
        while (aptMainLoop() && replay) {
            const MissionSessionAction action = runMission(bottomConsole, selectedMission, progress);
            replay = action == MissionSessionAction::Replay;
            if (action == MissionSessionAction::ExitApplication) {
                exitApplication = true;
                break;
            }
        }
    }

    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
