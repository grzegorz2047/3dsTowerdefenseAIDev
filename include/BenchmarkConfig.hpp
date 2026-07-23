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
};

struct BenchmarkConfig {
    std::uint8_t mapSize = 16U;
    std::uint8_t enemies = 16U;
    std::uint8_t towers = 16U;
    std::uint8_t projectiles