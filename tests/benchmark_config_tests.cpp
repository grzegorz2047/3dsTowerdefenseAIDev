#include <cstdlib>
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
