#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

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

struct BenchmarkConfig {
    std::uint8_t mapSize = 16U;
    std::uint8_t enemies = 16U;
    std::uint8_t towers = 16U;
    std::uint8_t projectiles = 32U;
    std::uint8_t decorations = 24U;
    std::uint8_t rocketSharePercent = 25U;
    bool automaticRamp = false;

    [[nodiscard]] bool operator==(const BenchmarkConfig& other) const {
        return mapSize == other.mapSize && enemies == other.enemies &&
            towers == other.towers && projectiles == other.projectiles &&
            decorations == other.decorations &&
            rocketSharePercent == other.rocketSharePercent &&
            automaticRamp == other.automaticRamp;
    }
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

}  // namespace BenchmarkProfiles
