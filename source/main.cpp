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
#include "BenchmarkConfig.hpp"
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
    AdvanceBenchmark,
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
    TouchGesture touchGesture;
    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        const TouchUiAction touch = gesture.tapped
            ? TouchUiLayout::narrativeActionAt(gesture.endX, gesture.endY) : TouchUiAction::None;
        if (input.pressed(KEY_START) || input.pressed(KEY_X) || touch == TouchUiAction::NarrativeSkip) return true;
        if (input.pressed(KEY_B) || input.pressed(KEY_DLEFT) || touch == TouchUiAction::NarrativeBack) {
            if (page == 0U) return false;
            --page;
        }
        if (input.pressed(KEY_A) || input.pressed(KEY_DRIGHT) || touch == TouchUiAction::NarrativeNext) {
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
    TouchGesture touchGesture;
    selected = std::min(selected, progress.unlockedCount() - 1U);

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        const TouchUiAction touch = gesture.tapped
            ? TouchUiLayout::campaignActionAt(gesture.endX, gesture.endY) : TouchUiAction::None;
        if (input.pressed(KEY_START) || touch == TouchUiAction::Exit) return kExitCampaignSelection;
        if (saveProblem && input.isHeld(KEY_SELECT) && input.pressed(KEY_Y)) {
            (void)SaveDataStore::reset(kSavePath);
            progress.reset();
            saveData = {};
            saveProblem = false;
            saveMessage = "Zapis zresetowany.";
            selected = 0U;
        } else if ((input.isHeld(KEY_SELECT) && input.pressed(KEY_X)) ||
            touch == TouchUiAction::CampaignBenchmark) {
            return kPerformanceStressSelection;
        } else if (input.pressed(KEY_X) || touch == TouchUiAction::ToggleSound) {
            saveData.settings.soundEnabled = !saveData.settings.soundEnabled;
            saveProblem = !persistSave(progress, saveData, saveMessage);
        } else if (input.pressed(KEY_B) || touch == TouchUiAction::ToggleMusic) {
            saveData.settings.musicEnabled = !saveData.settings.musicEnabled;
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
        if (touch >= TouchUiAction::CampaignRow0 && touch <= TouchUiAction::CampaignRow5) {
            const std::size_t first = selected < 6U ? 0U : std::min(selected - 5U, kCampaignMissionCount - 6U);
            const std::size_t row = static_cast<std::size_t>(touch) -
                static_cast<std::size_t>(TouchUiAction::CampaignRow0);
            const std::size_t candidate = first + row;
            if (candidate < selectableCount) selected = candidate;
        }
        if (input.pressed(KEY_DUP) || input.pressed(KEY_DLEFT)) {
            selected = selected == 0U ? selectableCount - 1U : selected - 1U;
        }
        if (input.pressed(KEY_DDOWN) || input.pressed(KEY_DRIGHT)) {
            selected = (selected + 1U) % selectableCount;
        }

        uiRenderer.renderStandalone(campaignUiState(progress, saveData, selected, saveProblem, saveMessage));
        if (input.pressed(KEY_A) || touch == TouchUiAction::CampaignPlay) return selected;
    }
    return kExitCampaignSelection;
}

void applyTouchAction(TouchUiAction action, BuildSystem& buildSystem, TutorialFlow& tutorialFlow,
    AudioSystem& audioSystem, bool soundEnabled, bool& paused, int& speedMultiplier) {
    switch (action) {
        case TouchUiAction::SelectBallista: buildSystem.selectTowerType(TowerType::Ballista); break;
        case TouchUiAction::SelectMortar: buildSystem.selectTowerType(TowerType::Mortar); break;
        case TouchUiAction::SelectFrost: buildSystem.selectTowerType(TowerType::Frost); break;
        case TouchUiAction::SelectRocket: buildSystem.selectTowerType(TowerType::Rocket); break;
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

bool configureBenchmark(UiRenderer& uiRenderer, BenchmarkConfig& config) {
    InputSystem inputSystem;
    TouchGesture touchGesture;
    std::size_t selected = 0U;
    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        const TouchUiAction touch = gesture.tapped
            ? TouchUiLayout::benchmarkActionAt(gesture.endX, gesture.endY) : TouchUiAction::None;
        if (input.pressed(KEY_B) || touch == TouchUiAction::BenchmarkBack) return false;
        if (input.pressed(KEY_A) || touch == TouchUiAction::BenchmarkStart) return true;
        if (input.pressed(KEY_DUP)) selected = selected == 0U ? 6U : selected - 1U;
        if (input.pressed(KEY_DDOWN)) selected = (selected + 1U) % 7U;
        if (input.pressed(KEY_DLEFT)) config = BenchmarkProfiles::adjusted(
            config, static_cast<BenchmarkSetting>(selected), -1);
        if (input.pressed(KEY_DRIGHT)) config = BenchmarkProfiles::adjusted(
            config, static_cast<BenchmarkSetting>(selected), 1);

        const std::array<std::pair<TouchUiAction, BenchmarkSetting>, 12U> adjustments{{
            {TouchUiAction::BenchmarkMapDown, BenchmarkSetting::MapSize},
            {TouchUiAction::BenchmarkMapUp, BenchmarkSetting::MapSize},
            {TouchUiAction::BenchmarkEnemiesDown, BenchmarkSetting::Enemies},
            {TouchUiAction::BenchmarkEnemiesUp, BenchmarkSetting::Enemies},
            {TouchUiAction::BenchmarkTowersDown, BenchmarkSetting::Towers},
            {TouchUiAction::BenchmarkTowersUp, BenchmarkSetting::Towers},
            {TouchUiAction::BenchmarkProjectilesDown, BenchmarkSetting::Projectiles},
            {TouchUiAction::BenchmarkProjectilesUp, BenchmarkSetting::Projectiles},
            {TouchUiAction::BenchmarkDecorationsDown, BenchmarkSetting::Decorations},
            {TouchUiAction::BenchmarkDecorationsUp, BenchmarkSetting::Decorations},
            {TouchUiAction::BenchmarkRocketsDown, BenchmarkSetting::RocketShare},
            {TouchUiAction::BenchmarkRocketsUp, BenchmarkSetting::RocketShare},
        }};
        for (std::size_t index = 0U; index < adjustments.size(); ++index) {
            if (touch != adjustments[index].first) continue;
            config = BenchmarkProfiles::adjusted(config, adjustments[index].second,
                index % 2U == 0U ? -1 : 1);
        }
        if (touch == TouchUiAction::BenchmarkAutomatic) {
            config = BenchmarkProfiles::adjusted(config, BenchmarkSetting::AutomaticRamp, 1);
        }

        UiState state{};
        state.screen = UiScreen::BenchmarkConfig;
        state.title = "LABORATORIUM WYDAJNOSCI";
        state.objective = "Ustaw obciazenie, potem uruchom deterministyczny pomiar 12 s.";
        state.benchmark = config;
        state.selectedMission = selected;
        uiRenderer.renderStandalone(state);
    }
    return false;
}

UiState missionUiState(const CampaignMission& mission, const Wave& wave,
    const BuildSystem& buildSystem, const TutorialFlow& tutorialFlow,
    const AudioSystem& audioSystem, const Renderer& renderer, const GameSettings& settings,
    const PerformanceSnapshot& performance, HudMode hudMode, bool paused, int speedMultiplier,
    const MissionResult& missionResult, bool recordsCampaignProgress, bool sessionFinished,
    const BenchmarkConfig* benchmarkConfig) {
    UiState state{};
    state.screen = UiScreen::Mission;
    state.title = mission.title;
    state.objective = mission.objective;
    state.gold = buildSystem.gold();
    state.baseHealth = wave.baseHealth();
    state.spawnedEnemies = wave.spawnedCount();
    state.activeEnemies = wave.activeCount();
    state.totalEnemies = wave.enemyCount();
    state.selectedTower = buildSystem.selectedTowerType();
    state.selectedTowerName = buildSystem.selectedTowerName();
    state.towerCost = buildSystem.towerCost();
    state.towerCount = buildSystem.towerCount();
    state.activeProjectiles = buildSystem.projectiles().activeCount();
    state.cursorX = buildSystem.cursorX();
    state.cursorZ = buildSystem.cursorZ();
    state.cursorOccupied = buildSystem.cursorOccupied();
    state.tutorialPhase = tutorialFlow.phase();
    state.instruction = benchmarkConfig != nullptr ? "Pomiar automatyczny: SELECT pokazuje diagnostyke" :
        tutorialInstruction(tutorialFlow.phase());
    state.paused = paused;
    state.speedMultiplier = speedMultiplier;
    state.diagnosticsVisible = showAudioDiagnostics(hudMode);
    state.missionFinished = sessionFinished;
    state.recordsCampaignProgress = recordsCampaignProgress;
    state.resultStars = missionResult.stars;
    state.soundEnabled = settings.soundEnabled;
    state.musicEnabled = settings.musicEnabled;
    state.maximum3DDepthPercent = settings.maximum3DDepthPercent;
    state.performance = performance;
    state.stereoEyeCount = renderer.lastEyeCount();
    state.stereoSlider = renderer.lastStereoSlider();
    state.stereoSeparation = renderer.lastStereoSeparation();
    if (benchmarkConfig != nullptr) {
        state.benchmarkMode = true;
        state.benchmark = *benchmarkConfig;
        state.benchmarkVerdict = BenchmarkProfiles::verdict(performance);
    }

    const Tower* tower = buildSystem.cursorTower();
    if (tower != nullptr) {
        state.cursorTowerLevel = tower->level();
        state.cursorTowerUpgradeCost = tower->upgradeCost();
        state.cursorTowerSellValue = tower->sellValue();
        state.statusMessage = towerName(tower->type());
    } else if (sessionFinished) {
        state.statusMessage = benchmarkConfig != nullptr ? BenchmarkProfiles::verdictName(state.benchmarkVerdict) :
            (recordsCampaignProgress ? "X kampania, Y powtorz" : "Test zakonczony bez zapisu");
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
    const MissionNarrative* narrative, const BenchmarkConfig* benchmarkConfig) {
    UiState loading{};
    loading.screen = UiScreen::Loading;
    loading.title = mission.title;
    uiRenderer.renderStandalone(loading);

    LevelLoadResult levelResult{};
    if (benchmarkConfig != nullptr) {
        levelResult.success = true;
        levelResult.level = BenchmarkProfiles::makeLevel(*benchmarkConfig);
    } else {
        levelResult = LevelLoader::loadFromRomFs(mission.levelPath);
    }
    if (!levelResult.success) {
        saveMessage = "BLAD POZIOMU: " + levelResult.error;
        return MissionSessionAction::ReturnToCampaign;
    }

    Renderer renderer;
    if (!renderer.initialize(levelResult.level,
        benchmarkConfig != nullptr ? benchmarkConfig->decorations : 0U)) {
        saveMessage = "BLAD GRAFIKI: brak pamieci.";
        return MissionSessionAction::ReturnToCampaign;
    }

    InputSystem inputSystem;
    TouchGesture touchGesture;
    Camera camera;
    Wave wave(levelResult.level);
    BuildSystem buildSystem(levelResult.level);
    if (benchmarkConfig != nullptr) {
        buildSystem.prepareBenchmarkLayout(benchmarkConfig->towers,
            benchmarkConfig->rocketSharePercent);
        buildSystem.setProjectileLimit(benchmarkConfig->projectiles);
    }
    TutorialFlow tutorialFlow;
    if (benchmarkConfig != nullptr) (void)tutorialFlow.requestWaveStart(buildSystem.towerCount());
    AudioSystem audioSystem;
    (void)audioSystem.initialize();
    AudioEventRouter audioRouter;
    audioRouter.reset({tutorialFlow.phase(), buildSystem.projectiles().shotEventCount(),
        buildSystem.projectiles().impactEventCount(), wave.deathEventCount(), wave.baseDamageEventCount()});
    PerformanceSampler performanceSampler;
    PerformanceSnapshot performanceSnapshot{};

    float simulationAccumulator = 0.0F;
    float benchmarkElapsed = 0.0F;
    float benchmarkResultElapsed = 0.0F;
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
        const bool benchmarkFinished = benchmarkConfig != nullptr && benchmarkElapsed >= 12.0F;
        const bool sessionFinished = benchmarkFinished || tutorialFlow.finished();

        const TouchGestureResult gesture = touchGesture.update(input.touching(),
            static_cast<std::int16_t>(input.touch.px), static_cast<std::int16_t>(input.touch.py));
        const TouchUiAction touch = gesture.tapped ? (sessionFinished
            ? TouchUiLayout::resultActionAt(gesture.endX, gesture.endY)
            : TouchUiLayout::actionAt(gesture.endX, gesture.endY)) : TouchUiAction::None;
        if (sessionFinished) {
            if (input.pressed(KEY_X) || touch == TouchUiAction::ResultCampaign) {
                action = MissionSessionAction::ReturnToCampaign; break;
            }
            if (input.pressed(KEY_Y) || touch == TouchUiAction::ResultReplay) {
                action = MissionSessionAction::Replay; break;
            }
        }

        const HudMode hudMode = hudModeForSelectHeld(input.isHeld(KEY_SELECT));
        if (saveData.settings.soundEnabled && allowDiagnosticTone(hudMode, input.pressed(KEY_B))) {
            audioSystem.playDiagnosticTone();
        }
        if (gesture.tapped && !sessionFinished) {
            applyTouchAction(touch, buildSystem, tutorialFlow, audioSystem,
                saveData.settings.soundEnabled, paused, speedMultiplier);
        }
        if (!sessionFinished && benchmarkConfig == nullptr && input.pressed(KEY_X)) {
            if (tutorialFlow.waveRunning()) paused = !paused;
            else { (void)tutorialFlow.requestWaveStart(buildSystem.towerCount()); paused = false; }
        }
        if (!sessionFinished && input.pressed(KEY_L) && input.pressed(KEY_R)) {
            speedMultiplier = speedMultiplier == 1 ? 2 : 1;
        }

        const u64 nowMilliseconds = osGetTime();
        const float frameSeconds = calculateFrameSeconds(nowMilliseconds, previousMilliseconds);
        previousMilliseconds = nowMilliseconds;
        camera.update(input, frameSeconds);
        if (!sessionFinished && benchmarkConfig == nullptr) {
            buildSystem.handleInput(input);
            if (saveData.settings.soundEnabled && input.pressed(KEY_A)) {
                audioSystem.play(cueForBuildResult(buildSystem.lastBuildResult()));
            }
        }
        tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());
        if (!paused && !sessionFinished) {
            simulationAccumulator = std::min(simulationAccumulator + frameSeconds * speedMultiplier,
                kMaximumAccumulatorSeconds);
            if (benchmarkConfig != nullptr) benchmarkElapsed += frameSeconds;
        }
        while (simulationAccumulator >= kFixedStepSeconds) {
            if (tutorialFlow.waveRunning()) {
                buildSystem.update(kFixedStepSeconds, wave);
                wave.update(kFixedStepSeconds);
                tutorialFlow.update(buildSystem.towerCount(), wave.completed(), wave.lost());
            }
            simulationAccumulator -= kFixedStepSeconds;
        }

        if (tutorialFlow.finished() && benchmarkConfig == nullptr && !resultRecorded) {
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
        if (benchmarkFinished && benchmarkConfig->automaticRamp) {
            benchmarkResultElapsed += frameSeconds;
            if (benchmarkResultElapsed >= 2.0F && !BenchmarkProfiles::maximumReached(*benchmarkConfig)) {
                action = MissionSessionAction::AdvanceBenchmark;
                break;
            }
        }

        const AudioFrameState audioState{tutorialFlow.phase(),
            buildSystem.projectiles().shotEventCount(), buildSystem.projectiles().impactEventCount(),
            wave.deathEventCount(), wave.baseDamageEventCount()};
        audioSystem.updateMusic(tutorialFlow.phase(), saveData.settings.musicEnabled, frameSeconds);
        if (saveData.settings.soundEnabled) audioSystem.playMask(audioRouter.update(audioState, frameSeconds));
        audioSystem.updateProbe();

        UiState uiState = missionUiState(mission, wave, buildSystem, tutorialFlow, audioSystem,
            renderer, saveData.settings, performanceSnapshot, hudMode, paused, speedMultiplier,
            missionResult, recordsCampaignProgress, sessionFinished, benchmarkConfig);
        const u64 renderStartMilliseconds = osGetTime();
        renderer.render(camera, wave, buildSystem, tutorialFlow,
            saveData.settings.maximum3DDepthPercent, uiRenderer, uiState);
        const u64 frameEndMilliseconds = osGetTime();
        performanceSampler.record(elapsedMilliseconds(frameStartMilliseconds, frameEndMilliseconds),
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
        BenchmarkConfig benchmarkConfig = BenchmarkProfiles::kBalanced;
        if (stressTest && !configureBenchmark(uiRenderer, benchmarkConfig)) continue;
        if (stressTest && benchmarkConfig.automaticRamp) benchmarkConfig = BenchmarkProfiles::kAutomatic;
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
                narrative.success ? &narrative.narrative : nullptr,
                stressTest ? &benchmarkConfig : nullptr);
            if (action == MissionSessionAction::AdvanceBenchmark) {
                benchmarkConfig = BenchmarkProfiles::nextAutomaticStage(benchmarkConfig);
                replay = true;
            } else {
                replay = action == MissionSessionAction::Replay;
            }
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
