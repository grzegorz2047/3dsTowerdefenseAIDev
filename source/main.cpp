#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <sys/stat.h>

#include "AudioEvents.hpp"
#include "AudioSystem.hpp"
#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "Campaign.hpp"
#include "HudMode.hpp"
#include "HudText.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Narrative.hpp"
#include "PerformanceBudget.hpp"
#include "Renderer.hpp"
#include "SaveData.hpp"
#include "TouchGesture.hpp"
#include "TouchUiLayout.hpp"
#include "TutorialFlow.hpp"
#include "UiRenderer.hpp"
#include "UiState.hpp"
#include "Wave.hpp"

namespace {

constexpr float kFixedStepSeconds = 1.0F / 60.0F;
constexpr float kMaximumFrameSeconds = 1.0F / 15.0F;
constexpr float kMaximumAccumulatorSeconds = kFixedStepSeconds * 5.0F;
constexpr const char* kSaveDirectory = "sdmc:/3ds/CitadelDefense3D";
constexpr const char* kSavePath = "sdmc:/3ds/CitadelDefense3D/campaign.sav";
constexpr std::size_t kExitCampaignSelection = kCampaignMissionCount;
constexpr std::size_t kPerformanceStressSelection = kCampaignMissionCount + 1U;

constexpr CampaignMission kPerformanceStressMission{
    "performance_stress",
    "romfs:/levels/performance_stress.lvl",
    "TEST WYDAJNOSCI",
    "Zmierz maksymalne obciazenie Old 3DS XL",
    "Wszystkie typy, 16 miejsc",
    "16 wrogow, szybki spawn",
    "Brak zapisu postepu",
    3,
    16,
};

enum class MissionSessionAction {
    ReturnToCampaign,
    Replay,
    ExitApplication,
};

float calculateFrameSeconds(u64 nowMilliseconds, u64 previousMilliseconds) {
    if (previousMilliseconds == 0U || nowMilliseconds <= previousMilliseconds) return kFixedStepSeconds;
    const float elapsed = static_cast<float>(nowMilliseconds - previousMilliseconds) / 1000.0F;
    return std::min(elapsed, kMaximumFrameSeconds);
}

float elapsedMilliseconds(u64 startMilliseconds, u64 endMilliseconds) {
    if (endMilliseconds <= startMilliseconds) return 0.0F;
    return static_cast<float>(endMilliseconds - startMilliseconds);
}

bool persistSave(const CampaignProgress& progress, SaveData& saveData, std::string& saveMessage) {
    saveData.campaign = progress.snapshot();
    std::string error;
    if (!SaveDataStore::saveAtomically(kSavePath, saveData, error)) {
        saveMessage = "BLAD ZAPISU: " + error;
        return false;
    }
    saveMessage = "Postep zapisany.";
    return true;
}

NarrativeLoadResult loadMissionNarrative(const CampaignMission& mission) {
    const std::string path = NarrativeLoader::pathFor("pl", mission.id);
    NarrativeLoadResult result = NarrativeLoader::load(path.c_str());
    if (result.success && result.narrative.missionId != mission.id) {
        result.success = false;
        result.error = "Narracja nie pasuje do misji";
    }
    return result;
}

bool showNarrativeCards(UiRenderer& uiRenderer, InputSystem& inputSystem, const char* title,
    const NarrativeCard* const* cards, std::size_t count) {
    if (cards == nullptr || count == 0U) return true;
    std::size_t page = 0U;
    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START) || input.pressed(KEY_X)) return true;
        if (input.pressed(KEY_B) || input.pressed(KEY_DLEFT)) {
            if (page == 0U) return false;
            --page;
        }
        if (input.pressed(KEY_A) || input.pressed(KEY_DRIGHT)) {
            if (page + 1U >= count) return true;
            ++page;
        }

        UiState state{};
        state.screen = UiScreen::Briefing;
        state.title = title;
        state.narrativeSpeaker = cards[page]->speaker.c_str();
        state.narrativeText = cards[page]->text.c_str();
        state.narrativePage = page;
        state.narrativePageCount = count;
        state.narrativeCanGoBack = page > 0U;
        uiRenderer.renderNarrative(state);
    }
    return false;
}

bool showMissionBriefing(UiRenderer& uiRenderer, InputSystem& inputSystem,
    const CampaignMission& mission, const MissionNarrative& narrative) {
    const std::array<const NarrativeCard*, 3U> cards{
        &narrative.briefing[0], &narrative.briefing[1], &narrative.mechanic};
    return showNarrativeCards(uiRenderer, inputSystem, mission.title, cards.data(), cards.size());
}

void showMissionOutcome(UiRenderer& uiRenderer, InputSystem& inputSystem,
    const CampaignMission& mission, const MissionNarrative& narrative, bool victory) {
    const NarrativeCard* card = victory ? &narrative.victory : &narrative.defeat;
    const std::array<const NarrativeCard*, 1U> cards{card};
    (void)showNarrativeCards(uiRenderer, inputSystem, mission.title, cards.data(), cards.size());
}

UiState campaignUiState(const CampaignProgress& progress, const SaveData& saveData,
    std::size_t selected, bool saveProblem, const std::string& saveMessage) {
    UiState state{};
    state.screen = UiScreen::Campaign;
    state.selectedMission = selected;
    state.saveProblem = saveProblem;
    state.statusMessage = saveMessage.c_str();
    state.soundEnabled = saveData.settings.soundEnabled;
    state.musicEnabled = saveData.settings.musicEnabled;
    state.stereoEnabled = saveData.settings.stereoEnabled;
    state.maximum3DDepthPercent = saveData.settings.maximum3DDepthPercent;

    const auto& missions = CampaignCatalog::missions();
    for (std::size_t index = 0; index < missions.size(); ++index) {
        state.campaignTitles[index] = missions[index].title;
        state.missionUnlocked[index] = progress.unlocked(index);
        state.missionStars[index] = progress.bestStars(index);
    }
    const CampaignMission& mission = missions[selected];
    state.title = mission.title;
    state.objective = mission.objective;
    state.availableTowers = mission.availableTowers;
    state.threats = mission.threats;
    state.fullHealthThreshold = mission.fullHealthThreshold;
    state.efficientTowerLimit = mission.efficientTowerLimit;
    return state;
}

std::size_t selectCampaignMission(UiRenderer& uiRenderer, CampaignProgress& progress,
    SaveData& saveData, std::size_t selected, bool& saveProblem, std::string& saveMessage) {
    InputSystem inputSystem;
    selected = std::min(selected, progress.unlockedCount() - 1U);

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) return kExitCampaignSelection;
        if (saveProblem && input.isHeld(KEY_SELECT) && input.pressed(KEY_Y)) {
            (void)SaveDataStore::reset(kSavePath);
            progress.reset();
            saveData = {};
            saveProblem = false;
            saveMessage = "Zapis zresetowany.";
            selected = 0U;
        } else if (input.isHeld(KEY_SELECT) && input.pressed(KEY_X)) {
            return kPerformanceStressSelection;
        } else if (input.pressed(KEY_X)) {
            saveData.settings.soundEnabled = !saveData.settings.soundEnabled;
            saveProblem = !persistSave(progress, saveData, saveMessage);
        } else if (input.pressed(KEY_B)) {
            saveData.settings.musicEnabled = !saveData.settings.musicEnabled;
            saveProblem = !persistSave(progress, saveData, saveMessage);
        } else if (input.pressed(KEY_L)) {
            saveData.settings.stereoEnabled = !saveData.settings.stereoEnabled;
            saveProblem = !persistSave(progress, saveData, saveMessage);
        } else if (input.pressed(KEY_R)) {
            saveData.settings.maximum3DDepthPercent =
                Stereo3D::nextDepthLimit(saveData.settings.maximum3DDepthPercent);
            saveProblem = !persistSave(progress, saveData, saveMessage);
        } else if (!input.isHeld(KEY_SELECT) && input.pressed(KEY_Y)) {
            const CampaignMission& mission = CampaignCatalog::mission(selected);
            const NarrativeLoadResult narrative = loadMissionNarrative(mission);
            if (narrative.success) {
                (void)showMissionBriefing(uiRenderer, inputSystem, mission, narrative.narrative);
            } else {
                saveMessage = "BLAD ODPRAWY: " + narrative.error;
                saveProblem = true;
            }
        }

        const std::size_t selectableCount = progress.unlockedCount();
        if (input.pressed(KEY_DUP) || input.pressed(KEY_DLEFT)) {
            selected = selected == 0U ? selectableCount - 1U : selected - 1U;
        }
        if (input.pressed(KEY_DDOWN) || input.pressed(KEY_DRIGHT)) {
            selected = (selected + 1U) % selectableCount;
        }

        uiRenderer.renderStandalone(campaignUiState(progress, saveData, selected, saveProblem, saveMessage));
        if (input.pressed(KEY_A)) return selected;
    }
    return kExitCampaignSelection;
}

void applyTouchAction(TouchUiAction action, BuildSystem& buildSystem, TutorialFlow& tutorialFlow,
    AudioSystem& audioSystem, bool soundEnabled, bool& paused, int& speedMultiplier) {
    switch (action) {
        case TouchUiAction::SelectBallista: buildSystem.selectTowerType(TowerType::Ballista); break;
        case TouchUiAction::SelectMortar: buildSystem.selectTowerType(TowerType::Mortar); break;
        case TouchUiAction::SelectFrost: buildSystem.selectTowerType(TowerType::Frost); break;
        case TouchUiAction::BuildOrSelect:
            buildSystem.buildOrSelectCursor();
            if (soundEnabled) audioSystem.play(cueForBuildResult(buildSystem.lastBuildResult()));
            break;
        case TouchUiAction::Upgrade: (void)buildSystem.upgradeCursorTower(); break;
        case TouchUiAction::Sell: (void)buildSystem.sellCursorTower(); break;
        case TouchUiAction::StartWave:
            (void)tutorialFlow.requestWaveStart(buildSystem.towerCount());
            paused = false;
            break;
        case TouchUiAction::TogglePause:
            if (tutorialFlow.waveRunning()) paused = !paused;
            break;
        case TouchUiAction::ToggleSpeed: speedMultiplier = speedMultiplier == 1 ? 2 : 1; break;
        case TouchUiAction::Cancel: buildSystem.cancelAction(); break;
        case TouchUiAction::None: default: break;
    }
}

UiState missionUiState(const CampaignMission& mission, const Wave& wave,
    const BuildSystem& buildSystem, const TutorialFlow& tutorialFlow,
    const AudioSystem& audioSystem, const Renderer& renderer, const GameSettings& settings,
    const PerformanceSnapshot& performance, HudMode hudMode, bool paused, int speedMultiplier,
    const MissionResult& missionResult, bool recordsCampaignProgress) {
    UiState state{};
    state.screen = UiScreen::Mission;
    state.title = mission.title;
    state.objective = mission.objective;
    state.gold = buildSystem.gold();
    state.baseHealth = wave.baseHealth();
    state.spawnedEnemies = wave.spawnedCount();
    state.totalEnemies = wave.enemyCount();
    state.selectedTower = buildSystem.selectedTowerType();
    state.selectedTowerName = buildSystem.selectedTowerName();
    state.towerCost = buildSystem.towerCost();
    state.towerCount = buildSystem.towerCount();
    state.cursorX = buildSystem.cursorX();
    state.cursorZ = buildSystem.cursorZ();
    state.cursorOccupied = buildSystem.cursorOccupied();
    state.tutorialPhase = tutorialFlow.phase();
    state.instruction = tutorialInstruction(tutorialFlow.phase());
    state.paused = paused;
    state.speedMultiplier = speedMultiplier;
    state.diagnosticsVisible = showAudioDiagnostics(hudMode);
    state.missionFinished = tutorialFlow.finished();
    state.recordsCampaignProgress = recordsCampaignProgress;
    state.resultStars = missionResult.stars;
    state.soundEnabled = settings.soundEnabled;
    state.musicEnabled = settings.musicEnabled;
    state.stereoEnabled = settings.stereoEnabled;
    state.maximum3DDepthPercent = settings.maximum3DDepthPercent;
    state.performance = performance;
    state.stereoEyeCount = renderer.lastEyeCount();
    state.stereoSlider = renderer.lastStereoSlider();
    state.stereoSeparation = renderer.lastStereoSeparation();

    const Tower* tower = buildSystem.cursorTower();
    if (tower != nullptr) {
        state.cursorTowerLevel = tower->level();
        state.cursorTowerUpgradeCost = tower->upgradeCost();
        state.cursorTowerSellValue = tower->sellValue();
        state.statusMessage = towerName(tower->type());
    } else if (tutorialFlow.finished()) {
        state.statusMessage = recordsCampaignProgress ? "X kampania, Y powtorz" : "Test zakonczony bez zapisu";
    } else {
        state.statusMessage = buildAttemptMessage(buildSystem.lastBuildResult());
    }

    state.audioAvailable = audioSystem.available();
    state.audioBackend = audioBackendName(audioSystem.backend());
    state.ndspShimActive = audioSystem.ndspShimActive();
    state.ndspShimAttempted = audioSystem.ndspShimAttempted();
    state.ndspInitialResult = audioSystem.ndspInitialResult();
    state.ndspShimResult = audioSystem.ndspShimResult();
    state.csndResult = audioSystem.csndResult();
    state.lastPlayResult = audioSystem.lastPlayResult();
    state.audioChannel = audioSystem.lastChannel();
    state.audioWaveStatus = audioWaveStatusName(audioSystem.diagnosticWaveStatus());
    state.audioSamplePosition = audioSystem.diagnosticSamplePosition();
    state.audioSampleCount = audioSystem.diagnosticSampleCount();
    state.audioProbeResult = audioSystem.probeResult();
    state.audioChannelActive = audioSystem.channelActive();
    state.audioChannelEverActive = audioSystem.channelEverActive();
    return state;
}

MissionSessionAction runMission(UiRenderer& uiRenderer, const CampaignMission& mission,
    std::size_t missionIndex, bool recordsCampaignProgress, CampaignProgress& progress,
    SaveData& saveData, bool& saveProblem, std::string& saveMessage,
    const MissionNarrative* narrative) {
    UiState loading{};
    loading.screen = UiScreen::Loading;
    loading.title = mission.title;
    uiRenderer.renderStandalone(loading);

    const LevelLoadResult levelResult = LevelLoader::loadFromRomFs(mission.levelPath);
    if (!levelResult.success) {
        saveMessage = "BLAD POZIOMU: " + levelResult.error;
        return MissionSessionAction::ReturnToCampaign;
    }

    Renderer renderer;
    if (!renderer.initialize(levelResult.level)) {
        saveMessage = "BLAD GRAFIKI: brak pamieci.";
        return MissionSessionAction::ReturnToCampaign;
    }

    InputSystem inputSystem;
    TouchGesture touchGesture;
    Camera camera;
    Wave wave(levelResult.level);
    BuildSystem buildSystem(levelResult.level);
    TutorialFlow tutorialFlow;
    AudioSystem audioSystem;
    (void)audioSystem.initialize();
    AudioEventRouter audioRouter;
    audioRouter.reset({
        tutorialFlow.phase(),
        buildSystem.projectiles().shotEventCount(),
        buildSystem.projectiles().impactEventCount(),
        wave.deathEventCount(),
        wave.baseDamageEventCount(),
    });
    PerformanceSampler performanceSampler;
    PerformanceSnapshot performanceSnapshot{};

    float simulationAccumulator = 0.0F;
    u64 previousMilliseconds = osGetTime();
    bool paused = false;
    int speedMultiplier = 1;
    bool resultRecorded = false;
    bool resultNarrativeShown = false;
    MissionResult missionResult{};
    MissionSessionAction action = MissionSessionAction::ExitApplication;

    while (aptMainLoop()) {
        const u64 frameStartMilliseconds = osGetTime();
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) { action = MissionSessionAction::ExitApplication; break; }
        if (tutorialFlow.finished()) {
            if (input.pressed(KEY_X)) { action = MissionSessionAction::ReturnToCampaign; break; }
            if (input.pressed(KEY_Y)) { action = MissionSessionAction::Replay; break; }
        }

        const HudMode hudMode = hudModeForSelectHeld(input.isHeld(KEY_SELECT));
        if (saveData.settings.soundEnabled && allowDiagnosticTone(hudMode, input.pressed(KEY_B))) {
            audioSystem.playDiagnosticTone();
        }
        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        if (gesture.tapped && !tutorialFlow.finished()) {
            applyTouchAction(TouchUiLayout::actionAt(gesture.endX, gesture.endY), buildSystem,
                tutorialFlow, audioSystem, saveData.settings.soundEnabled, paused, speedMultiplier);
        }
        if (!tutorialFlow.finished() && input.pressed(KEY_X)) {
            if (tutorialFlow.waveRunning()) paused = !paused;
            else { (void)tutorialFlow.requestWaveStart(buildSystem.towerCount()); paused = false; }
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
            if (saveData.settings.soundEnabled && input.pressed(KEY_A)) {
                audioSystem.play(cueForBuildResult(buildSystem.lastBuildResult()));
            }
        }
        tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());
        if (!paused) {
            simulationAccumulator = std::min(simulationAccumulator + frameSeconds * speedMultiplier,
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
            if (recordsCampaignProgress && tutorialFlow.phase() == TutorialPhase::Victory) {
                missionResult = progress.complete(missionIndex, wave.baseHealth(), buildSystem.towerCount());
                saveProblem = !persistSave(progress, saveData, saveMessage);
            }
            resultRecorded = true;
        }
        if (tutorialFlow.finished() && resultRecorded && !resultNarrativeShown && narrative != nullptr) {
            showMissionOutcome(uiRenderer, inputSystem, mission, *narrative,
                tutorialFlow.phase() == TutorialPhase::Victory);
            resultNarrativeShown = true;
            previousMilliseconds = osGetTime();
        }

        const AudioFrameState audioState{
            tutorialFlow.phase(),
            buildSystem.projectiles().shotEventCount(),
            buildSystem.projectiles().impactEventCount(),
            wave.deathEventCount(),
            wave.baseDamageEventCount(),
        };
        audioSystem.updateMusic(tutorialFlow.phase(), saveData.settings.musicEnabled, frameSeconds);
        if (saveData.settings.soundEnabled) audioSystem.playMask(audioRouter.update(audioState, frameSeconds));
        audioSystem.updateProbe();

        UiState uiState = missionUiState(mission, wave, buildSystem, tutorialFlow, audioSystem,
            renderer, saveData.settings, performanceSnapshot, hudMode, paused, speedMultiplier,
            missionResult, recordsCampaignProgress);
        const u64 renderStartMilliseconds = osGetTime();
        renderer.render(camera, wave, buildSystem, tutorialFlow,
            saveData.settings.stereoEnabled, saveData.settings.maximum3DDepthPercent,
            uiRenderer, uiState);
        const u64 frameEndMilliseconds = osGetTime();
        performanceSampler.record(
            elapsedMilliseconds(frameStartMilliseconds, frameEndMilliseconds),
            elapsedMilliseconds(renderStartMilliseconds, frameEndMilliseconds),
            static_cast<std::size_t>(linearSpaceFree()));
        performanceSnapshot = performanceSampler.snapshot();
    }

    audioSystem.shutdown();
    renderer.shutdown();
    return action;
}

int showStartupError(const char* stage, const char* detail, bool citro2dInitialized,
    bool citro3dInitialized, bool romfsInitialized) {
    if (citro2dInitialized) C2D_Fini();
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
        char detail[96]{};
        std::snprintf(detail, sizeof(detail), "romfsInit() = 0x%08lX", static_cast<unsigned long>(romfsResult));
        return showStartupError("ROMFS", detail, false, false, false);
    }
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        return showStartupError("CITRO3D", "C3D_Init nie przydzielil bufora polecen GPU.", false, false, true);
    }
    if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)) {
        return showStartupError("CITRO2D", "C2D_Init nie przydzielil zasobow UI.", false, true, true);
    }
    C2D_Prepare();

    UiRenderer uiRenderer;
    if (!uiRenderer.initialize()) {
        return showStartupError("UI", "Nie udalo sie utworzyc dolnego targetu lub bufora tekstu.", true, true, true);
    }

    (void)mkdir("sdmc:/3ds", 0777);
    (void)mkdir(kSaveDirectory, 0777);
    CampaignProgress progress;
    SaveData saveData{};
    bool saveProblem = false;
    std::string saveMessage;
    const SaveLoadResult loaded = SaveDataStore::load(kSavePath);
    if (loaded.status == SaveLoadStatus::Loaded && progress.restore(loaded.data.campaign)) {
        saveData = loaded.data;
        saveMessage = loaded.migrated ? "Zapis zmigrowany do v4." : "Wczytano zapis.";
        if (loaded.migrated) saveProblem = !persistSave(progress, saveData, saveMessage);
    } else if (loaded.status == SaveLoadStatus::Corrupt || loaded.status == SaveLoadStatus::UnsupportedVersion) {
        saveProblem = true;
        saveMessage = loaded.error;
    }

    std::size_t selectedMission = 0U;
    bool exitApplication = false;
    while (aptMainLoop() && !exitApplication) {
        selectedMission = selectCampaignMission(uiRenderer, progress, saveData, selectedMission,
            saveProblem, saveMessage);
        if (selectedMission == kExitCampaignSelection) break;

        const bool stressTest = selectedMission == kPerformanceStressSelection;
        const CampaignMission& mission = stressTest ? kPerformanceStressMission : CampaignCatalog::mission(selectedMission);
        const std::size_t progressMissionIndex = stressTest ? 0U : selectedMission;

        NarrativeLoadResult narrative{};
        if (!stressTest) {
            narrative = loadMissionNarrative(mission);
            if (narrative.success) {
                InputSystem briefingInput;
                if (!showMissionBriefing(uiRenderer, briefingInput, mission, narrative.narrative)) continue;
            } else {
                saveMessage = "BLAD ODPRAWY: " + narrative.error;
                saveProblem = true;
            }
        }

        bool replay = true;
        while (aptMainLoop() && replay) {
            const MissionSessionAction action = runMission(uiRenderer, mission, progressMissionIndex,
                !stressTest, progress, saveData, saveProblem, saveMessage,
                narrative.success ? &narrative.narrative : nullptr);
            replay = action == MissionSessionAction::Replay;
            if (action == MissionSessionAction::ExitApplication) { exitApplication = true; break; }
        }
    }

    uiRenderer.shutdown();
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
