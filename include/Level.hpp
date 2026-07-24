#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

constexpr std::size_t kMaximumMapWidth = 16;
constexpr std::size_t kMaximumMapHeight = 16;
constexpr std::size_t kMaximumPaths = 4;
constexpr std::size_t kMaximumPathPoints = 64;
constexpr std::size_t kMaximumWaveEntries = 8;
constexpr std::size_t kMaximumWaveEnemies = 16;
constexpr std::size_t kMaximumMissionEnemies = kMaximumWaveEntries * kMaximum