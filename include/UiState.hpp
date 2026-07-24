#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "BenchmarkConfig.hpp"
#include "Campaign.hpp"
#include "PerformanceBudget.hpp"
#include "Tower.hpp"
#include "TutorialFlow.hpp"

enum class UiScreen : std::uint8_t {
    Campaign,
    Briefing,
    BenchmarkConfig,
    Loading,
    Mission,
};

struct UiState {
    UiScreen screen = UiScreen::Campaign;
    std::size_t selectedMission = 0U;
    int gold = 0;
    int baseHealth = 0;
    std::size_t spawnedEnemies = 0U;
    std::size_t activeEnemies = 0U;
    std::size_t totalEnemies = 0U;
    std::size_t waveNumber = 0U;
    std::size_t waveCount = 0U;
    std::size_t completedWaves = 0U;
    bool waveRunning = false;
    bool awaitingNextWave = false;
    int waveCompletionReward = 0;
    TowerType selectedTower = TowerType::Ballista;
    TutorialPhase tutorialPhase = TutorialPhase::BuildFirstTower;
    bool paused = false;
    int speedMultiplier = 1;
    bool diagnosticsVisible = false;

    const char* title = "";
    const char* objective = "";
    const char* availableTowers = "";
    const char* threats = "";
    const char* instruction = "";
    const char* statusMessage = "";
    const char* selectedTowerName = "";

    const char* narrativeSpeaker = "";
    const char* narrativeText = "";
    std::size_t narrativePage = 0U;
    std::size_t narrativePageCount = 0U;
    bool narrativeCanGoBack = false;

    std::array<const char*, kCampaignMissionCount> campaignTitles{};
    std::array<bool, kCampaignMissionCount> missionUnlocked{};
    std::array<std::uint8_t, kCampaignMissionCount> missionStars{};
    std::uint8_t fullHealthThreshold = 0U;
    std::uint8_t efficientTowerLimit = 0U;
    std::uint8_t difficulty = 1U;
    bool saveProblem = false;

    bool soundEnabled = true;
    bool musicEnabled = true;
    std::uint8_t maximum3DDepthPercent = 0U;

    std::size_t towerCount = 0U;
    std::size_t buildSpotCount = 0U;
    std::size_t availableBuildSpotCount = 0U;
    std::size_t activeProjectiles = 0U;
    int towerCost = 0;
    std::size_t cursorX = 0U;
    std::size_t cursorZ = 0U;
    bool cursorOccupied = false;
    std::uint8_t cursorTowerLevel = 0U;
    int cursorTowerUpgradeCost = 0;
    int cursorTowerSellValue = 0;

    bool missionFinished = false;
    bool recordsCampaignProgress = true;
    std::uint8_t resultStars = 0U;

    bool benchmarkMode = false;
    BenchmarkConfig benchmark{};
    BenchmarkVerdict benchmarkVerdict = BenchmarkVerdict::Warn;

    PerformanceSnapshot performance{};
    std::uint8_t stereoEyeCount = 0U;
    float stereoSlider = 0.0F;
    float stereoSeparation = 0.0F;

    bool audioAvailable = false;
    const char* audioBackend = "";
    bool ndspShimActive = false;
    bool ndspShimAttempted = false;
    std::uint32_t ndspInitialResult = 0U;
    std::uint32_t ndspShimResult = 0U;
    std::uint32_t csndResult = 0U;
    std::uint32_t lastPlayResult = 0U;
    int audioChannel = -1;
    const char* audioWaveStatus = "";
    std::size_t audioSamplePosition = 0U;
    std::size_t audioSampleCount = 0U;
    std::uint32_t audioProbeResult = 0U;
    bool audioChannelActive = false;
    bool audioChannelEverActive = false;
};
