from pathlib import Path
import re


def write(path: str, content: str) -> None:
    Path(path).write_text(content.rstrip() + "\n")


def replace_once(path: str, old: str, new: str) -> None:
    file = Path(path)
    text = file.read_text()
    if text.count(old) != 1:
        raise RuntimeError(f"expected one match in {path}: {old[:80]!r}, got {text.count(old)}")
    file.write_text(text.replace(old, new, 1))


def replace_span(path: str, start: str, end: str, replacement: str) -> None:
    file = Path(path)
    text = file.read_text()
    first = text.find(start)
    if first < 0:
        raise RuntimeError(f"start marker missing in {path}: {start!r}")
    last = text.find(end, first)
    if last < 0:
        raise RuntimeError(f"end marker missing in {path}: {end!r}")
    file.write_text(text[:first] + replacement + text[last:])


write("include/BenchmarkConfig.hpp", r'''#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "Level.hpp"
#include "PerformanceBudget.hpp"

enum class BenchmarkSetting : std::uint8_t {
    MapSize,
    Enemies,
    Towers,
    Projectiles,
    Decorations,
    RocketShare,
    AutomaticRamp,
    Count,
};

enum class BenchmarkVerdict : std::uint8_t {
    Pass,
    Warn,
    Fail,
};

struct BenchmarkConfig {
    std::uint8_t mapSize = 12U;
    std::uint8_t enemies = 12U;
    std::uint8_t towers = 12U;
    std::uint8_t projectiles = 24U;
    std::uint8_t decorations = 16U;
    std::uint8_t rocketSharePercent = 25U;
    bool automaticRamp = false;

    [[nodiscard]] bool operator==(const BenchmarkConfig& other) const {
        return mapSize == other.mapSize && enemies == other.enemies &&
            towers == other.towers && projectiles == other.projectiles &&
            decorations == other.decorations &&
            rocketSharePercent == other.rocketSharePercent &&
            automaticRamp == other.automaticRamp;
    }
    [[nodiscard]] bool operator!=(const BenchmarkConfig& other) const { return !(*this == other); }
};

namespace BenchmarkProfiles {

constexpr std::array<std::uint8_t, 3U> kMapSizes{8U, 12U, 16U};
constexpr std::array<std::uint8_t, 4U> kEnemyCounts{4U, 8U, 12U, 16U};
constexpr std::array<std::uint8_t, 4U> kTowerCounts{4U, 8U, 12U, 16U};
constexpr std::array<std::uint8_t, 4U> kProjectileCounts{8U, 16U, 24U, 32U};
constexpr std::array<std::uint8_t, 4U> kDecorationCounts{0U, 8U, 16U, 24U};
constexpr std::array<std::uint8_t, 4U> kRocketShares{0U, 25U, 50U, 100U};

constexpr BenchmarkConfig kBalanced{12U, 12U, 12U, 24U, 16U, 25U, false};
constexpr BenchmarkConfig kMaximum{16U, 16U, 16U, 32U, 24U, 100U, false};
constexpr BenchmarkConfig kAutomatic{8U, 4U, 4U, 8U, 0U, 0U, true};

template <std::size_t N>
[[nodiscard]] constexpr std::uint8_t steppedValue(
    const std::array<std::uint8_t, N>& values, std::uint8_t current, int delta) {
    std::size_t index = 0U;
    for (std::size_t candidate = 0U; candidate < N; ++candidate) {
        if (values[candidate] == current) { index = candidate; break; }
    }
    if (delta < 0) index = index == 0U ? N - 1U : index - 1U;
    else if (delta > 0) index = (index + 1U) % N;
    return values[index];
}

[[nodiscard]] constexpr BenchmarkConfig adjusted(
    BenchmarkConfig config, BenchmarkSetting setting, int delta) {
    switch (setting) {
        case BenchmarkSetting::MapSize:
            config.mapSize = steppedValue(kMapSizes, config.mapSize, delta); break;
        case BenchmarkSetting::Enemies:
            config.enemies = steppedValue(kEnemyCounts, config.enemies, delta); break;
        case BenchmarkSetting::Towers:
            config.towers = steppedValue(kTowerCounts, config.towers, delta); break;
        case BenchmarkSetting::Projectiles:
            config.projectiles = steppedValue(kProjectileCounts, config.projectiles, delta); break;
        case BenchmarkSetting::Decorations:
            config.decorations = steppedValue(kDecorationCounts, config.decorations, delta); break;
        case BenchmarkSetting::RocketShare:
            config.rocketSharePercent = steppedValue(kRocketShares, config.rocketSharePercent, delta); break;
        case BenchmarkSetting::AutomaticRamp:
            if (delta != 0) config.automaticRamp = !config.automaticRamp;
            break;
        case BenchmarkSetting::Count:
        default: break;
    }
    return config;
}

[[nodiscard]] constexpr BenchmarkConfig nextAutomaticStage(BenchmarkConfig config) {
    if (!config.automaticRamp) return config;
    if (config.enemies < 16U) config.enemies = steppedValue(kEnemyCounts, config.enemies, 1);
    else if (config.towers < 16U) config.towers = steppedValue(kTowerCounts, config.towers, 1);
    else if (config.projectiles < 32U) config.projectiles = steppedValue(kProjectileCounts, config.projectiles, 1);
    else if (config.decorations < 24U) config.decorations = steppedValue(kDecorationCounts, config.decorations, 1);
    else if (config.rocketSharePercent < 100U) config.rocketSharePercent = steppedValue(kRocketShares, config.rocketSharePercent, 1);
    else if (config.mapSize < 16U) config.mapSize = steppedValue(kMapSizes, config.mapSize, 1);
    return config;
}

[[nodiscard]] constexpr bool maximumReached(const BenchmarkConfig& config) {
    return config.mapSize == 16U && config.enemies == 16U && config.towers == 16U &&
        config.projectiles == 32U && config.decorations == 24U &&
        config.rocketSharePercent == 100U;
}

[[nodiscard]] inline LevelData makeLevel(const BenchmarkConfig& config) {
    LevelData level{};
    level.id = "performance_stress";
    level.name = "Konfigurowalne laboratorium";
    level.width = std::min<std::uint8_t>(config.mapSize, static_cast<std::uint8_t>(kMaximumMapWidth));
    level.height = std::min<std::uint8_t>(config.mapSize, static_cast<std::uint8_t>(kMaximumMapHeight));
    level.tiles.fill(TileType::Ground);

    const std::size_t roadZ = static_cast<std::size_t>(level.height / 2U);
    for (std::size_t x = 0U; x < level.width; ++x) {
        level.path[level.pathLength++] = {
            static_cast<std::int16_t>(x), static_cast<std::int16_t>(roadZ)};
        level.tiles[roadZ * kMaximumMapWidth + x] = TileType::Road;
    }
    level.tiles[roadZ * kMaximumMapWidth] = TileType::Spawn;
    level.tiles[roadZ * kMaximumMapWidth + level.width - 1U] = TileType::Base;

    std::size_t spots = 0U;
    for (std::size_t distance = 1U; distance < level.height && spots < config.towers; ++distance) {
        const std::array<int, 2U> rows{
            static_cast<int>(roadZ) - static_cast<int>(distance),
            static_cast<int>(roadZ) + static_cast<int>(distance)};
        for (const int row : rows) {
            if (row < 0 || row >= static_cast<int>(level.height)) continue;
            for (std::size_t x = 1U; x + 1U < level.width && spots < config.towers; x += 2U) {
                level.tiles[static_cast<std::size_t>(row) * kMaximumMapWidth + x] = TileType::BuildSpot;
                ++spots;
            }
        }
    }

    level.waveEntries[0] = {
        EnemyType::Raider,
        std::min<std::uint8_t>(config.enemies, static_cast<std::uint8_t>(kMaximumWaveEnemies)),
        0.12F};
    level.waveEntryCount = 1U;
    level.totalEnemyCount = level.waveEntries[0].count;
    return level;
}

[[nodiscard]] inline BenchmarkVerdict verdict(const PerformanceSnapshot& snapshot) {
    if (snapshot.memoryReserveLow() || snapshot.worstFrameMilliseconds > 50.0F) {
        return BenchmarkVerdict::Fail;
    }
    if (snapshot.frameBudgetExceeded() || snapshot.lastRenderMilliseconds >
        PerformanceBudget::kStereoRenderBudgetMilliseconds) {
        return BenchmarkVerdict::Warn;
    }
    return BenchmarkVerdict::Pass;
}

[[nodiscard]] constexpr const char* verdictName(BenchmarkVerdict value) {
    switch (value) {
        case BenchmarkVerdict::Pass: return "PASS";
        case BenchmarkVerdict::Warn: return "WARN";
        case BenchmarkVerdict::Fail: return "FAIL";
        default: return "WARN";
    }
}

}  // namespace BenchmarkProfiles
''')

write("include/TouchUiLayout.hpp", r'''#pragma once

#include <cstdint>

enum class TouchUiAction : std::uint8_t {
    None,
    SelectBallista, SelectMortar, SelectFrost, SelectRocket,
    BuildOrSelect, Upgrade, Sell, StartWave, TogglePause, ToggleSpeed, Cancel,
    CampaignRow0, CampaignRow1, CampaignRow2, CampaignRow3, CampaignRow4, CampaignRow5,
    CampaignPlay, CampaignBenchmark, ToggleSound, ToggleMusic, Exit,
    NarrativeBack, NarrativeNext, NarrativeSkip,
    BenchmarkMapDown, BenchmarkMapUp,
    BenchmarkEnemiesDown, BenchmarkEnemiesUp,
    BenchmarkTowersDown, BenchmarkTowersUp,
    BenchmarkProjectilesDown, BenchmarkProjectilesUp,
    BenchmarkDecorationsDown, BenchmarkDecorationsUp,
    BenchmarkRocketsDown, BenchmarkRocketsUp,
    BenchmarkAutomatic, BenchmarkStart, BenchmarkBack,
    ResultCampaign, ResultReplay,
};

struct TouchRect {
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t width = 0;
    std::int16_t height = 0;

    [[nodiscard]] constexpr bool contains(std::int16_t pointX, std::int16_t pointY) const {
        return pointX >= x && pointY >= y &&
            pointX < static_cast<std::int16_t>(x + width) &&
            pointY < static_cast<std::int16_t>(y + height);
    }
};

class TouchUiLayout {
public:
    static constexpr std::int16_t kScreenWidth = 320;
    static constexpr std::int16_t kScreenHeight = 240;
    static constexpr std::int16_t kMinimumButtonHeight = 40;

    [[nodiscard]] static TouchUiAction actionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction campaignActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction narrativeActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction benchmarkActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction resultActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchRect rectFor(TouchUiAction action);
};
''')

write("source/TouchUiLayout.cpp", r'''#include "TouchUiLayout.hpp"

#include <array>

namespace {

constexpr TouchRect kBallista{8, 8, 72, 44};
constexpr TouchRect kMortar{85, 8, 72, 44};
constexpr TouchRect kFrost{162, 8, 72, 44};
constexpr TouchRect kRocket{239, 8, 72, 44};
constexpr TouchRect kCancel{240, 128, 72, 40};
constexpr TouchRect kPause{8, 128, 72, 40};
constexpr TouchRect kSpeed{88, 128, 72, 40};
constexpr TouchRect kBuild{8, 184, 72, 48};
constexpr TouchRect kUpgrade{88, 184, 72, 48};
constexpr TouchRect kSell{168, 184, 64, 48};
constexpr TouchRect kStart{240, 184, 72, 48};
constexpr TouchRect kResultCampaign{8, 190, 144, 42};
constexpr TouchRect kResultReplay{160, 190, 152, 42};

constexpr std::array<TouchRect, 6U> kCampaignRows{{
    {6, 36, 144, 24}, {6, 60, 144, 24}, {6, 84, 144, 24},
    {6, 108, 144, 24}, {6, 132, 144, 24}, {6, 156, 144, 24},
}};
constexpr TouchRect kCampaignPlay{6, 201, 58, 33};
constexpr TouchRect kCampaignBenchmark{68, 201, 58, 33};
constexpr TouchRect kCampaignSound{130, 201, 58, 33};
constexpr TouchRect kCampaignMusic{192, 201, 58, 33};
constexpr TouchRect kCampaignExit{254, 201, 60, 33};

constexpr TouchRect kNarrativeBack{8, 194, 96, 40};
constexpr TouchRect kNarrativeNext{112, 194, 96, 40};
constexpr TouchRect kNarrativeSkip{216, 194, 96, 40};

constexpr std::array<TouchRect, 6U> kBenchmarkDown{{
    {202, 34, 44, 24}, {202, 61, 44, 24}, {202, 88, 44, 24},
    {202, 115, 44, 24}, {202, 142, 44, 24}, {202, 169, 44, 24},
}};
constexpr std::array<TouchRect, 6U> kBenchmarkUp{{
    {252, 34, 60, 24}, {252, 61, 60, 24}, {252, 88, 60, 24},
    {252, 115, 60, 24}, {252, 142, 60, 24}, {252, 169, 60, 24},
}};
constexpr TouchRect kBenchmarkAutomatic{8, 199, 144, 34};
constexpr TouchRect kBenchmarkStart{160, 199, 72, 34};
constexpr TouchRect kBenchmarkBack{240, 199, 72, 34};

bool outside(std::int16_t x, std::int16_t y) {
    return x < 0 || y < 0 || x >= TouchUiLayout::kScreenWidth || y >= TouchUiLayout::kScreenHeight;
}

}  // namespace

TouchUiAction TouchUiLayout::actionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    const std::array<std::pair<TouchRect, TouchUiAction>, 11U> actions{{
        {kBallista, TouchUiAction::SelectBallista}, {kMortar, TouchUiAction::SelectMortar},
        {kFrost, TouchUiAction::SelectFrost}, {kRocket, TouchUiAction::SelectRocket},
        {kPause, TouchUiAction::TogglePause}, {kSpeed, TouchUiAction::ToggleSpeed},
        {kCancel, TouchUiAction::Cancel}, {kBuild, TouchUiAction::BuildOrSelect},
        {kUpgrade, TouchUiAction::Upgrade}, {kSell, TouchUiAction::Sell},
        {kStart, TouchUiAction::StartWave},
    }};
    for (const auto& entry : actions) if (entry.first.contains(x, y)) return entry.second;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::campaignActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    constexpr std::array<TouchUiAction, 6U> rows{
        TouchUiAction::CampaignRow0, TouchUiAction::CampaignRow1, TouchUiAction::CampaignRow2,
        TouchUiAction::CampaignRow3, TouchUiAction::CampaignRow4, TouchUiAction::CampaignRow5};
    for (std::size_t index = 0U; index < rows.size(); ++index) {
        if (kCampaignRows[index].contains(x, y)) return rows[index];
    }
    if (kCampaignPlay.contains(x, y)) return TouchUiAction::CampaignPlay;
    if (kCampaignBenchmark.contains(x, y)) return TouchUiAction::CampaignBenchmark;
    if (kCampaignSound.contains(x, y)) return TouchUiAction::ToggleSound;
    if (kCampaignMusic.contains(x, y)) return TouchUiAction::ToggleMusic;
    if (kCampaignExit.contains(x, y)) return TouchUiAction::Exit;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::narrativeActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    if (kNarrativeBack.contains(x, y)) return TouchUiAction::NarrativeBack;
    if (kNarrativeNext.contains(x, y)) return TouchUiAction::NarrativeNext;
    if (kNarrativeSkip.contains(x, y)) return TouchUiAction::NarrativeSkip;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::benchmarkActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    constexpr std::array<TouchUiAction, 6U> down{
        TouchUiAction::BenchmarkMapDown, TouchUiAction::BenchmarkEnemiesDown,
        TouchUiAction::BenchmarkTowersDown, TouchUiAction::BenchmarkProjectilesDown,
        TouchUiAction::BenchmarkDecorationsDown, TouchUiAction::BenchmarkRocketsDown};
    constexpr std::array<TouchUiAction, 6U> up{
        TouchUiAction::BenchmarkMapUp, TouchUiAction::BenchmarkEnemiesUp,
        TouchUiAction::BenchmarkTowersUp, TouchUiAction::BenchmarkProjectilesUp,
        TouchUiAction::BenchmarkDecorationsUp, TouchUiAction::BenchmarkRocketsUp};
    for (std::size_t index = 0U; index < down.size(); ++index) {
        if (kBenchmarkDown[index].contains(x, y)) return down[index];
        if (kBenchmarkUp[index].contains(x, y)) return up[index];
    }
    if (kBenchmarkAutomatic.contains(x, y)) return TouchUiAction::BenchmarkAutomatic;
    if (kBenchmarkStart.contains(x, y)) return TouchUiAction::BenchmarkStart;
    if (kBenchmarkBack.contains(x, y)) return TouchUiAction::BenchmarkBack;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::resultActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    if (kResultCampaign.contains(x, y)) return TouchUiAction::ResultCampaign;
    if (kResultReplay.contains(x, y)) return TouchUiAction::ResultReplay;
    return TouchUiAction::None;
}

TouchRect TouchUiLayout::rectFor(TouchUiAction action) {
    switch (action) {
        case TouchUiAction::SelectBallista: return kBallista;
        case TouchUiAction::SelectMortar: return kMortar;
        case TouchUiAction::SelectFrost: return kFrost;
        case TouchUiAction::SelectRocket: return kRocket;
        case TouchUiAction::BuildOrSelect: return kBuild;
        case TouchUiAction::Upgrade: return kUpgrade;
        case TouchUiAction::Sell: return kSell;
        case TouchUiAction::StartWave: return kStart;
        case TouchUiAction::TogglePause: return kPause;
        case TouchUiAction::ToggleSpeed: return kSpeed;
        case TouchUiAction::Cancel: return kCancel;
        case TouchUiAction::CampaignPlay: return kCampaignPlay;
        case TouchUiAction::CampaignBenchmark: return kCampaignBenchmark;
        case TouchUiAction::ToggleSound: return kCampaignSound;
        case TouchUiAction::ToggleMusic: return kCampaignMusic;
        case TouchUiAction::Exit: return kCampaignExit;
        case TouchUiAction::NarrativeBack: return kNarrativeBack;
        case TouchUiAction::NarrativeNext: return kNarrativeNext;
        case TouchUiAction::NarrativeSkip: return kNarrativeSkip;
        case TouchUiAction::BenchmarkAutomatic: return kBenchmarkAutomatic;
        case TouchUiAction::BenchmarkStart: return kBenchmarkStart;
        case TouchUiAction::BenchmarkBack: return kBenchmarkBack;
        case TouchUiAction::ResultCampaign: return kResultCampaign;
        case TouchUiAction::ResultReplay: return kResultReplay;
        default: return {};
    }
}
''')

write("include/UiState.hpp", r'''#pragma once

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
''')

replace_once("include/UiRenderer.hpp", "    void drawCampaign(const UiState& state);\n    void drawMission(const UiState& state);", "    void drawCampaign(const UiState& state);\n    void drawBenchmarkConfig(const UiState& state);\n    void drawMission(const UiState& state);")

write("include/Stereo3D.hpp", r'''#pragma once

#include <cstdint>

struct StereoFramePlan {
    bool stereo = false;
    std::uint8_t eyeCount = 1U;
    float slider = 0.0F;
    float separation = 0.0F;
    float leftEyeIod = 0.0F;
    float rightEyeIod = 0.0F;
};

class Stereo3D {
public:
    static constexpr float kMaximumIod = 0.34F;
    static constexpr std::uint8_t kDefaultDepthPercent = 70;

    [[nodiscard]] static StereoFramePlan plan(
        float sliderState, std::uint8_t maximumDepthPercent);
    [[nodiscard]] static std::uint8_t nextDepthLimit(std::uint8_t currentDepthPercent);
};
''')

write("source/Stereo3D.cpp", r'''#include "Stereo3D.hpp"

#include <algorithm>

StereoFramePlan Stereo3D::plan(float sliderState, std::uint8_t maximumDepthPercent) {
    StereoFramePlan result{};
    result.slider = std::clamp(sliderState, 0.0F, 1.0F);
    const float depthLimit =
        static_cast<float>(std::min<std::uint8_t>(maximumDepthPercent, 100U)) / 100.0F;
    result.separation = result.slider * depthLimit * kMaximumIod;
    result.stereo = result.slider > 0.0001F && result.separation > 0.0001F;
    result.eyeCount = result.stereo ? 2U : 1U;
    if (result.stereo) {
        result.leftEyeIod = -result.separation;
        result.rightEyeIod = result.separation;
    }
    return result;
}

std::uint8_t Stereo3D::nextDepthLimit(std::uint8_t currentDepthPercent) {
    if (currentDepthPercent < 50U) return 50U;
    if (currentDepthPercent < 70U) return 70U;
    if (currentDepthPercent < 100U) return 100U;
    return 0U;
}
''')

replace_once("include/Projectile.hpp", "    void update(float deltaSeconds, Wave& wave);\n    void reset();", "    void update(float deltaSeconds, Wave& wave);\n    void reset();\n    void setActiveLimit(std::size_t limit);")
replace_once("include/Projectile.hpp", "    [[nodiscard]] std::size_t activeCount() const;", "    [[nodiscard]] std::size_t activeCount() const;\n    [[nodiscard]] std::size_t activeLimit() const;")
replace_once("include/Projectile.hpp", "    std::array<Projectile, kCapacity> projectiles_{};", "    std::array<Projectile, kCapacity> projectiles_{};\n    std::size_t activeLimit_ = kCapacity;")

replace_once("source/Projectile.cpp", "    for (Projectile& projectile : projectiles_) {\n        if (!projectile.active()) {", "    for (std::size_t index = 0U; index < activeLimit_; ++index) {\n        Projectile& projectile = projectiles_[index];\n        if (!projectile.active()) {")
replace_once("source/Projectile.cpp", "void ProjectilePool::reset() {", "void ProjectilePool::setActiveLimit(std::size_t limit) {\n    activeLimit_ = std::max<std::size_t>(1U, std::min(limit, kCapacity));\n    for (std::size_t index = activeLimit_; index < kCapacity; ++index) projectiles_[index].reset();\n}\n\nvoid ProjectilePool::reset() {")
replace_once("source/Projectile.cpp", "std::uint32_t ProjectilePool::shotEventCount() const", "std::size_t ProjectilePool::activeLimit() const { return activeLimit_; }\n\nstd::uint32_t ProjectilePool::shotEventCount() const")

replace_once("include/BuildSystem.hpp", "    void prepareBenchmarkLayout();", "    void prepareBenchmarkLayout(std::size_t requestedTowers = kMaximumTowers,\n        std::uint8_t rocketSharePercent = 25U);\n    void setProjectileLimit(std::size_t limit);")
replace_once("source/BuildSystem.cpp", "void BuildSystem::prepareBenchmarkLayout() {\n    reset();\n    if (level_ == nullptr) return;\n    const std::array<TowerType, 4U> pattern{\n        TowerType::Ballista, TowerType::Mortar, TowerType::Frost, TowerType::Rocket};\n    for (std::size_t index = 0U; index < buildSpotCount_ && towerCount_ < kMaximumTowers; ++index) {\n        const GridPoint spot = buildSpots_[index];\n        Tower tower(*level_, static_cast<std::size_t>(spot.x), static_cast<std::size_t>(spot.z),\n            pattern[index % pattern.size()]);\n        if (!tower.valid()) continue;\n        while (tower.canUpgrade()) (void)tower.upgrade();\n        towers_[towerCount_++] = tower;\n    }\n    cursorIndex_ = 0U;\n    selectedTowerType_ = TowerType::Rocket;\n    cancelAction();\n}", "void BuildSystem::prepareBenchmarkLayout(std::size_t requestedTowers,\n    std::uint8_t rocketSharePercent) {\n    reset();\n    if (level_ == nullptr) return;\n    const std::size_t target = std::min({requestedTowers, buildSpotCount_, kMaximumTowers});\n    const std::size_t rocketCount = (target * std::min<std::uint8_t>(rocketSharePercent, 100U) + 99U) / 100U;\n    const std::array<TowerType, 3U> conventional{\n        TowerType::Ballista, TowerType::Mortar, TowerType::Frost};\n    for (std::size_t index = 0U; index < target; ++index) {\n        const GridPoint spot = buildSpots_[index];\n        const TowerType type = index < rocketCount ? TowerType::Rocket :\n            conventional[(index - rocketCount) % conventional.size()];\n        Tower tower(*level_, static_cast<std::size_t>(spot.x), static_cast<std::size_t>(spot.z), type);\n        if (!tower.valid()) continue;\n        while (tower.canUpgrade()) (void)tower.upgrade();\n        towers_[towerCount_++] = tower;\n    }\n    cursorIndex_ = 0U;\n    selectedTowerType_ = rocketSharePercent > 0U ? TowerType::Rocket : TowerType::Ballista;\n    cancelAction();\n}\n\nvoid BuildSystem::setProjectileLimit(std::size_t limit) { projectiles_.setActiveLimit(limit); }")

replace_once("include/Renderer.hpp", "    [[nodiscard]] bool initialize(const LevelData& level);", "    [[nodiscard]] bool initialize(const LevelData& level, std::size_t benchmarkDecorations = 0U);")
replace_once("include/Renderer.hpp", "        const TutorialFlow& tutorialFlow,\n        bool stereoEnabled,\n        std::uint8_t maximum3DDepthPercent,", "        const TutorialFlow& tutorialFlow,\n        std::uint8_t maximum3DDepthPercent,")
replace_once("include/Renderer.hpp", "    StereoFramePlan lastStereoPlan_{};", "    StereoFramePlan lastStereoPlan_{};\n    std::size_t benchmarkDecorations_ = 0U;")
replace_once("source/Renderer.cpp", "bool Renderer::initialize(const LevelData& level) {", "bool Renderer::initialize(const LevelData& level, std::size_t benchmarkDecorations) {")
replace_once("source/Renderer.cpp", "    level_ = &level;\n    gfxSet3D(true);", "    level_ = &level;\n    benchmarkDecorations_ = benchmarkDecorations;\n    gfxSet3D(true);")
replace_once("source/Renderer.cpp", "    bool stereoEnabled, std::uint8_t maximum3DDepthPercent,\n    UiRenderer& uiRenderer, const UiState& uiState) {\n    lastStereoPlan_ = Stereo3D::plan(osGet3DSliderState(), stereoEnabled, maximum3DDepthPercent);", "    std::uint8_t maximum3DDepthPercent,\n    UiRenderer& uiRenderer, const UiState& uiState) {\n    lastStereoPlan_ = Stereo3D::plan(osGet3DSliderState(), maximum3DDepthPercent);")
replace_once("source/Renderer.cpp", "    lastStereoPlan_ = {};", "    lastStereoPlan_ = {};\n    benchmarkDecorations_ = 0U;")

write("source/UiRendererNarrative.cpp", r'''#include "UiRenderer.hpp"

#include <cstdio>

#include "TouchUiLayout.hpp"

void UiRenderer::renderNarrative(const UiState& state) {
    if (topTarget_ == nullptr || bottomTarget_ == nullptr || textBuffer_ == nullptr) return;

    const u32 background = C2D_Color32(12, 18, 28, 255);
    const u32 panel = C2D_Color32(20, 29, 40, 255);
    const u32 panelStrong = C2D_Color32(24, 43, 62, 255);
    const u32 text = C2D_Color32(236, 240, 244, 255);
    const u32 accent = C2D_Color32(112, 225, 255, 255);
    const u32 gold = C2D_Color32(255, 222, 112, 255);

    gfxSet3D(false);
    beginFrame();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    C2D_Prepare();
    C2D_TargetClear(topTarget_, background);
    C2D_SceneBegin(topTarget_);
    C2D_DrawRectSolid(16.0F, 14.0F, 0.1F, 368.0F, 212.0F, panel);
    C2D_DrawRectSolid(16.0F, 14.0F, 0.2F, 368.0F, 42.0F, panelStrong);
    drawText("OSTATNIA CYTADELA", 76.0F, 25.0F, 0.72F, accent);
    drawWrappedText(state.title, 28.0F, 76.0F, 0.66F, gold, 28U, 2U);
    drawText(state.narrativeSpeaker, 28.0F, 126.0F, 0.48F, accent);
    drawWrappedText(state.narrativeText, 28.0F, 151.0F, 0.48F, text, 38U, 4U);
    C2D_Flush();

    C2D_Prepare();
    C2D_TargetClear(bottomTarget_, background);
    C2D_SceneBegin(bottomTarget_);
    C2D_DrawRectSolid(8.0F, 18.0F, 0.1F, 304.0F, 166.0F, panel);
    drawText("ODPRAWA", 106.0F, 31.0F, 0.66F, accent);
    drawWrappedText(state.narrativeText, 18.0F, 67.0F, 0.48F, text, 34U, 6U);
    char page[24]{};
    std::snprintf(page, sizeof(page), "%zu/%zu", state.narrativePage + 1U, state.narrativePageCount);
    drawText(page, 142.0F, 178.0F, 0.44F, gold);
    const TouchRect back = TouchUiLayout::rectFor(TouchUiAction::NarrativeBack);
    const TouchRect next = TouchUiLayout::rectFor(TouchUiAction::NarrativeNext);
    const TouchRect skip = TouchUiLayout::rectFor(TouchUiAction::NarrativeSkip);
    drawButton(back.x, back.y, back.width, back.height,
        state.narrativeCanGoBack ? "WSTECZ" : "ZAMKNIJ", false);
    drawButton(next.x, next.y, next.width, next.height, "DALEJ", true);
    drawButton(skip.x, skip.y, skip.width, skip.height, "POMIN", false);
    C2D_Flush();

    C3D_FrameEnd(0);
    composedFrameActive_ = false;
}
''')

# Main runtime flow.
replace_once("source/main.cpp", '#include "AudioSystem.hpp"\n', '#include "AudioSystem.hpp"\n#include "BenchmarkConfig.hpp"\n')

replace_span("source/main.cpp", "bool showNarrativeCards(", "bool showMissionBriefing(", r'''bool showNarrativeCards(UiRenderer& uiRenderer, InputSystem& inputSystem, const char* title,
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

''')

replace_once("source/main.cpp", "    state.stereoEnabled = saveData.settings.stereoEnabled;\n", "")

replace_span("source/main.cpp", "std::size_t selectCampaignMission(", "void applyTouchAction(", r'''std::size_t selectCampaignMission(UiRenderer& uiRenderer, CampaignProgress& progress,
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

''')

replace_once("source/main.cpp", "        case TouchUiAction::SelectFrost: buildSystem.selectTowerType(TowerType::Frost); break;\n        case TouchUiAction::BuildOrSelect:", "        case TouchUiAction::SelectFrost: buildSystem.selectTowerType(TowerType::Frost); break;\n        case TouchUiAction::SelectRocket: buildSystem.selectTowerType(TowerType::Rocket); break;\n        case TouchUiAction::BuildOrSelect:")

# Add benchmark configuration loop before mission state construction.
marker = "UiState missionUiState("
insert = r'''bool configureBenchmark(UiRenderer& uiRenderer, BenchmarkConfig& config) {
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

'''
text = Path("source/main.cpp").read_text()
if marker not in text:
    raise RuntimeError("missionUiState marker missing")
Path("source/main.cpp").write_text(text.replace(marker, insert + marker, 1))

replace_span("source/main.cpp", "UiState missionUiState(", "MissionSessionAction runMission(", r'''UiState missionUiState(const CampaignMission& mission, const Wave& wave,
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

''')

# Add automatic-ramp action.
replace_once("source/main.cpp", "    Replay,\n    ExitApplication,", "    Replay,\n    AdvanceBenchmark,\n    ExitApplication,")

replace_span("source/main.cpp", "MissionSessionAction runMission(", "int showStartupError(", r'''MissionSessionAction runMission(UiRenderer& uiRenderer, const CampaignMission& mission,
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

''')

replace_once("source/main.cpp", "        const bool stressTest = selectedMission == kPerformanceStressSelection;\n        const CampaignMission& mission = stressTest ? kPerformanceStressMission : CampaignCatalog::mission(selectedMission);", "        const bool stressTest = selectedMission == kPerformanceStressSelection;\n        BenchmarkConfig benchmarkConfig = BenchmarkProfiles::kBalanced;\n        if (stressTest && !configureBenchmark(uiRenderer, benchmarkConfig)) continue;\n        if (stressTest && benchmarkConfig.automaticRamp) benchmarkConfig = BenchmarkProfiles::kAutomatic;\n        const CampaignMission& mission = stressTest ? kPerformanceStressMission : CampaignCatalog::mission(selectedMission);")
replace_once("source/main.cpp", "                narrative.success ? &narrative.narrative : nullptr);\n            replay = action == MissionSessionAction::Replay;", "                narrative.success ? &narrative.narrative : nullptr,\n                stressTest ? &benchmarkConfig : nullptr);\n            if (action == MissionSessionAction::AdvanceBenchmark) {\n                benchmarkConfig = BenchmarkProfiles::nextAutomaticStage(benchmarkConfig);\n                replay = true;\n            } else {\n                replay = action == MissionSessionAction::Replay;\n            }")

# UiRenderer standalone/config/results.
replace_once("source/UiRenderer.cpp", "    if (state.screen == UiScreen::Loading) {", "    if (state.screen == UiScreen::BenchmarkConfig) {\n        drawText(\"LABORATORIUM WYDAJNOSCI\", 60.0F, 82.0F, 0.68F, kGold);\n        drawWrappedText(state.objective, 52.0F, 126.0F, 0.48F, kText, 32U, 3U);\n        drawText(\"Dotyk: -/+  A start  B powrot\", 63.0F, 194.0F, 0.42F, kMuted);\n    } else if (state.screen == UiScreen::Loading) {")
replace_once("source/UiRenderer.cpp", "    if (state.screen == UiScreen::Campaign) drawCampaign(state);\n    else if (state.screen == UiScreen::Loading)", "    if (state.screen == UiScreen::Campaign) drawCampaign(state);\n    else if (state.screen == UiScreen::BenchmarkConfig) drawBenchmarkConfig(state);\n    else if (state.screen == UiScreen::Loading)")
replace_once("source/UiRenderer.cpp", "    drawButton(6.0F, 201.0F, 72.0F, 33.0F, \"A GRAJ\", false);\n    drawButton(82.0F, 201.0F, 72.0F, 33.0F,\n        state.soundEnabled ? \"X SFX ON\" : \"X SFX OFF\", false);\n    drawButton(158.0F, 201.0F, 72.0F, 33.0F,\n        state.musicEnabled ? \"B MUZ ON\" : \"B MUZ OFF\", false);\n    drawButton(234.0F, 201.0F, 80.0F, 33.0F, \"START WYJ\", false);", "    drawButton(6.0F, 201.0F, 58.0F, 33.0F, \"GRAJ\", false);\n    drawButton(68.0F, 201.0F, 58.0F, 33.0F, \"LAB\", false);\n    drawButton(130.0F, 201.0F, 58.0F, 33.0F, state.soundEnabled ? \"SFX ON\" : \"SFX OFF\", false);\n    drawButton(192.0F, 201.0F, 58.0F, 33.0F, state.musicEnabled ? \"MUZ ON\" : \"MUZ OFF\", false);\n    drawButton(254.0F, 201.0F, 60.0F, 33.0F, \"WYJ\", false);")

insert_marker = "void UiRenderer::drawMission(const UiState& state) {"
benchmark_draw = r'''void UiRenderer::drawBenchmarkConfig(const UiState& state) {
    C2D_DrawRectSolid(0.0F, 0.0F, 0.1F, 320.0F, 28.0F, kPanelStrong);
    drawText("KONFIGURATOR BENCHMARKU", 18.0F, 6.0F, 0.52F, kAccent);
    const std::array<const char*, 6U> labels{
        "MAPA", "WROGOWIE", "OBRONY", "POCISKI", "DEKORACJE", "RAKIETY"};
    const std::array<unsigned int, 6U> values{
        state.benchmark.mapSize, state.benchmark.enemies, state.benchmark.towers,
        state.benchmark.projectiles, state.benchmark.decorations,
        state.benchmark.rocketSharePercent};
    for (std::size_t row = 0U; row < labels.size(); ++row) {
        const float y = 34.0F + static_cast<float>(row) * 27.0F;
        char value[40]{};
        if (row == 0U) std::snprintf(value, sizeof(value), "%ux%u", values[row], values[row]);
        else if (row == 5U) std::snprintf(value, sizeof(value), "%u%%", values[row]);
        else std::snprintf(value, sizeof(value), "%u", values[row]);
        drawText(labels[row], 12.0F, y + 4.0F, 0.44F, kText);
        drawText(value, 126.0F, y + 4.0F, 0.44F, kGold);
        drawButton(202.0F, y, 44.0F, 24.0F, "-", false);
        drawButton(252.0F, y, 60.0F, 24.0F, "+", false);
    }
    drawButton(8.0F, 199.0F, 144.0F, 34.0F,
        state.benchmark.automaticRamp ? "AUTO ON" : "AUTO OFF",
        state.benchmark.automaticRamp);
    drawButton(160.0F, 199.0F, 72.0F, 34.0F, "START", true);
    drawButton(240.0F, 199.0F, 72.0F, 34.0F, "WSTECZ", false);
}

'''
text = Path("source/UiRenderer.cpp").read_text()
if insert_marker not in text:
    raise RuntimeError("drawMission marker missing")
Path("source/UiRenderer.cpp").write_text(text.replace(insert_marker, benchmark_draw + insert_marker, 1))

replace_once("source/UiRenderer.cpp", "    if (state.diagnosticsVisible) { drawDiagnostics(state); return; }\n    if (state.missionFinished) {\n        drawButton(8.0F, 128.0F, 144.0F, 40.0F, \"X KAMPANIA\", true);\n        drawButton(160.0F, 128.0F, 152.0F, 40.0F, \"Y POWTORZ\", false);", "    if (state.diagnosticsVisible && !state.missionFinished) { drawDiagnostics(state); return; }\n    if (state.missionFinished) {\n        if (state.benchmarkMode) {\n            C2D_DrawRectSolid(8.0F, 124.0F, 0.2F, 304.0F, 62.0F, kPanelStrong);\n            char result[96]{};\n            const float fps = state.performance.averageFrameMilliseconds > 0.0F\n                ? 1000.0F / state.performance.averageFrameMilliseconds : 0.0F;\n            std::snprintf(result, sizeof(result), \"%s FPS %.1f AVG %.1f MAX %.1f\",\n                BenchmarkProfiles::verdictName(state.benchmarkVerdict), fps,\n                state.performance.averageFrameMilliseconds, state.performance.worstFrameMilliseconds);\n            drawText(result, 12.0F, 130.0F, 0.40F,\n                state.benchmarkVerdict == BenchmarkVerdict::Fail ? kDanger : kGold);\n            std::snprintf(result, sizeof(result), \"R %.1f MEM %luK E%zu T%zu P%zu D%u O%u\",\n                state.performance.lastRenderMilliseconds,\n                static_cast<unsigned long>(state.performance.freeLinearMemoryBytes / 1024U),\n                state.activeEnemies, state.towerCount, state.activeProjectiles,\n                static_cast<unsigned int>(state.benchmark.decorations),\n                static_cast<unsigned int>(state.stereoEyeCount));\n            drawText(result, 12.0F, 151.0F, 0.40F, kText);\n            std::snprintf(result, sizeof(result), \"MAP %ux%u RAK %u%% SEP %.3f\",\n                state.benchmark.mapSize, state.benchmark.mapSize,\n                state.benchmark.rocketSharePercent, state.stereoSeparation);\n            drawText(result, 12.0F, 171.0F, 0.40F, kMuted);\n        }\n        drawButton(8.0F, 190.0F, 144.0F, 42.0F, \"KAMPANIA\", true);\n        drawButton(160.0F, 190.0F, 152.0F, 42.0F, \"POWTORZ\", false);")
replace_once("source/UiRenderer.cpp", "    std::snprintf(line, sizeof(line), \"3D SUWAK %.2f LIMIT %u%% OCZY %u\",\n        state.stereoSlider,\n        static_cast<unsigned int>(state.maximum3DDepthPercent),\n        static_cast<unsigned int>(state.stereoEyeCount));", "    std::snprintf(line, sizeof(line), \"3D %.2f SEP %.3f OCZY %u LIM %u%%\",\n        state.stereoSlider, state.stereoSeparation,\n        static_cast<unsigned int>(state.stereoEyeCount),\n        static_cast<unsigned int>(state.maximum3DDepthPercent));")

# Host tests for config and all touch surfaces.
write("tests/benchmark_config_tests.cpp", r'''#include <cstdlib>
#include <iostream>

#include "BenchmarkConfig.hpp"

namespace {
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    BenchmarkConfig config = BenchmarkProfiles::kBalanced;
    config = BenchmarkProfiles::adjusted(config, BenchmarkSetting::MapSize, 1);
    expect(config.mapSize == 16U, "map size should step to 16");
    config = BenchmarkProfiles::kAutomatic;
    const BenchmarkConfig next = BenchmarkProfiles::nextAutomaticStage(config);
    expect(next.enemies == 8U, "automatic ramp should increase enemies first");
    const LevelData level = BenchmarkProfiles::makeLevel(BenchmarkProfiles::kMaximum);
    expect(level.width == 16U && level.height == 16U, "maximum map should be 16x16");
    expect(level.totalEnemyCount == 16U, "configured enemy count should reach level");
    std::size_t buildSpots = 0U;
    for (std::size_t z = 0U; z < level.height; ++z) {
        for (std::size_t x = 0U; x < level.width; ++x) {
            if (level.tileAt(x, z) == TileType::BuildSpot) ++buildSpots;
        }
    }
    expect(buildSpots == 16U, "generator should create requested tower spots");
    std::cout << "Benchmark config tests passed\n";
    return 0;
}
''')

replace_once("scripts/run_host_tests.sh", '"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/performance_stress_level_tests.cpp" "$ROOT/source/Level.cpp" -o "$BUILD_DIR/performance-stress-level-tests"\n', '"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/performance_stress_level_tests.cpp" "$ROOT/source/Level.cpp" -o "$BUILD_DIR/performance-stress-level-tests"\n"$HOST_CXX" "${COMMON_FLAGS[@]}" "$ROOT/tests/benchmark_config_tests.cpp" "$ROOT/source/Level.cpp" -o "$BUILD_DIR/benchmark-config-tests"\n')
replace_once("scripts/run_host_tests.sh", '"$BUILD_DIR/performance-stress-level-tests" "$ROOT"\n', '"$BUILD_DIR/performance-stress-level-tests" "$ROOT"\n"$BUILD_DIR/benchmark-config-tests"\n')
replace_once("scripts/run_host_tests.sh", 'grep -q "Stereo3D::nextDepthLimit" "$ROOT/source/main.cpp"\n', 'grep -q "Stereo3D::nextDepthLimit" "$ROOT/source/main.cpp"\nif grep -q "settings.stereoEnabled" "$ROOT/source/main.cpp"; then\n  echo "Physical 3D slider must be the only stereo switch" >&2\n  exit 1\nfi\ngrep -q "BenchmarkProfiles::makeLevel" "$ROOT/source/main.cpp"\ngrep -q "configureBenchmark" "$ROOT/source/main.cpp"\n')

write("tests/stereo_3d_tests.cpp", r'''#include <cstdlib>
#include <iostream>

#include "Stereo3D.hpp"

namespace {
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    const StereoFramePlan mono = Stereo3D::plan(0.0F, 100U);
    expect(!mono.stereo && mono.eyeCount == 1U, "zero physical slider should render one eye");
    const StereoFramePlan stereo = Stereo3D::plan(1.0F, 100U);
    expect(stereo.stereo && stereo.eyeCount == 2U, "raised physical slider should render two eyes");
    expect(stereo.separation > 0.0F, "raised physical slider should create separation");
    const StereoFramePlan limited = Stereo3D::plan(1.0F, 50U);
    expect(limited.separation < stereo.separation, "depth limit should scale separation");
    expect(Stereo3D::nextDepthLimit(100U) == 0U, "depth limit cycle should include zero");
    std::cout << "Stereo 3D tests passed\n";
    return 0;
}
''')

# Remove obsolete one-shot workflow and this patch script after the workflow commits changes.
for obsolete in [
    ".github/workflows/issue-157-integrate.yml",
    ".github/scripts/issue157_runtime_patch.py",
]:
    file = Path(obsolete)
    if file.exists(): file.unlink()
