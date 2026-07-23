#pragma once

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
