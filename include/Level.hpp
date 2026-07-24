#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

constexpr std::size_t kMaximumMapWidth = 16;
constexpr std::size_t kMaximumMapHeight = 16;
constexpr std::size_t kMaximumPathPoints = 64;
constexpr std::size_t kMaximumWaveEntries = 8;
constexpr std::size_t kMaximumWaveEnemies = 16;
constexpr std::size_t kMaximumMissionEnemies = kMaximumWaveEntries * kMaximumWaveEnemies;

enum class TileType : std::uint8_t {
    Ground,
    Road,
    BuildSpot,
    Blocked,
    Spawn,
    Base,
};

enum class EnemyType : std::uint8_t {
    Scout,
    Raider,
    Brute,
};

struct GridPoint {
    std::int16_t x = 0;
    std::int16_t z = 0;
};

struct WaveEntry {
    EnemyType type = EnemyType::Raider;
    std::uint8_t count = 0;
    float spawnIntervalSeconds = 1.0F;
};

struct LevelData {
    std::string id;
    std::string name;
    std::uint8_t width = 0;
    std::uint8_t height = 0;
    std::array<TileType, kMaximumMapWidth * kMaximumMapHeight> tiles{};
    std::array<GridPoint, kMaximumPathPoints> path{};
    std::size_t pathLength = 0;
    std::array<WaveEntry, kMaximumWaveEntries> waveEntries{};
    std::size_t waveEntryCount = 0;
    std::size_t totalEnemyCount = 0;

    [[nodiscard]] TileType tileAt(std::size_t x, std::size_t z) const;
};

struct LevelLoadResult {
    bool success = false;
    std::string error;
    LevelData level;
};

class LevelLoader {
public:
    [[nodiscard]] static LevelLoadResult loadFromRomFs(const char* path);
};
