#include <cstdlib>
#include <iostream>
#include <string>

#include "Level.hpp"
#include "PerformanceBudget.hpp"

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    const std::string path = std::string(argv[1]) + "/romfs/levels/performance_stress.lvl";
    const LevelLoadResult result = LevelLoader::loadFromRomFs(path.c_str());

    expect(result.success, std::string("stress level should load: ") + result.error);
    expect(result.level.id == "performance_stress", "stress level id should be stable");
    expect(result.level.totalEnemyCount == kMaximumWaveEnemies,
        "stress level should use the parser enemy capacity");
    expect(result.level.totalEnemyCount == PerformanceBudget::kMaximumEnemies,
        "performance budget should match runtime enemy capacity");
    expect(result.level.waveEntryCount == kMaximumWaveEntries,
        "stress level should use every wave entry");

    std::size_t buildSpots = 0;
    for (std::size_t z = 0; z < result.level.height; ++z) {
        for (std::size_t x = 0; x < result.level.width; ++x) {
            buildSpots += result.level.tileAt(x, z) == TileType::BuildSpot ? 1U : 0U;
        }
    }
    expect(buildSpots >= PerformanceBudget::kMaximumTowers,
        "stress level should expose enough build spots for the tower budget");

    std::cout << "Performance stress level tests passed\n";
    return 0;
}
