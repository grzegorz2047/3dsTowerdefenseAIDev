#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

#include "Economy.hpp"
#include "Enemy.hpp"
#include "Level.hpp"

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

std::size_t heavyEnemyCount(const LevelData& level) {
    std::size_t count = 0U;
    for (std::size_t index = 0U; index < level.waveEntryCount; ++index) {
        if (level.waveEntries[index].type != EnemyType::Scout) {
            count += level.waveEntries[index].count;
        }
    }
    return count;
}

int rewardBudget(const LevelData& level) {
    int reward = 0;
    for (std::size_t index = 0U; index < level.waveEntryCount; ++index) {
        const WaveEntry& wave = level.waveEntries[index];
        int perEnemy = 8;
        if (wave.type == EnemyType::Raider) perEnemy = 15;
        if (wave.type == EnemyType::Brute) perEnemy = 28;
        reward += static_cast<int>(wave.count) * perEnemy;
    }
    reward += static_cast<int>(level.waveEntryCount) * Economy::kWaveCompletionReward;
    return reward;
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    const std::string root = argv[1];
    constexpr std::array<const char*, 9U> ids{{
        "tutorial", "ash_gate", "ruined_village", "stone_bridge", "echo_valley",
        "flooded_road", "iron_ravine", "storm_ring", "last_citadel"}};
    constexpr std::array<std::size_t, 9U> expectedEnemies{{18U, 24U, 28U, 32U, 36U, 40U, 44U, 48U, 52U}};

    std::size_t previousEnemies = 0U;
    std::size_t previousHeavy = 0U;
    int previousRewardBudget = 0;
    for (std::size_t mission = 0U; mission < ids.size(); ++mission) {
        const std::string path = root + "/romfs/levels/" + ids[mission] + ".lvl";
        const LevelLoadResult loaded = LevelLoader::loadFromRomFs(path.c_str());
        expect(loaded.success, "campaign level should load: " + std::string(ids[mission]) +
            " (" + loaded.error + ")");
        const LevelData& level = loaded.level;
        expect(level.totalEnemyCount == expectedEnemies[mission],
            "campaign enemy count should match designed curve: " + std::string(ids[mission]));
        expect(level.totalEnemyCount <= kMaximumMissionEnemies,
            "mission should fit total enemy budget: " + std::string(ids[mission]));
        expect(level.waveEntryCount >= 5U && level.waveEntryCount <= kMaximumWaveEntries,
            "mission should contain five to eight waves: " + std::string(ids[mission]));
        for (std::size_t wave = 0U; wave < level.waveEntryCount; ++wave) {
            expect(level.waveEntries[wave].count <= kMaximumWaveEnemies,
                "wave should fit active enemy budget: " + std::string(ids[mission]));
        }

        const std::size_t heavy = heavyEnemyCount(level);
        const int rewards = rewardBudget(level);
        expect(level.totalEnemyCount > previousEnemies,
            "total pressure should increase every mission: " + std::string(ids[mission]));
        if (mission >= 2U) {
            expect(heavy >= previousHeavy,
                "armored and heavy presence should not regress: " + std::string(ids[mission]));
        }
        expect(rewards > previousRewardBudget,
            "economy budget should grow with mission pressure: " + std::string(ids[mission]));
        previousEnemies = level.totalEnemyCount;
        previousHeavy = heavy;
        previousRewardBudget = rewards;
    }

    LevelData base{};
    base.id = "tutorial";
    base.width = 2U;
    base.height = 1U;
    base.pathLength = 2U;
    base.path[0] = {0, 0};
    base.path[1] = {1, 0};
    expect(Enemy(base, EnemyType::Scout).maxHealth() >= 5,
        "scout should survive several basic hits");
    expect(Enemy(base, EnemyType::Raider).maxHealth() >= 12,
        "raider should be a visibly tougher armored target");
    expect(Enemy(base, EnemyType::Brute).maxHealth() >= 30,
        "brute should require sustained focused fire");

    std::cout << "Campaign balance tests passed\n";
    return 0;
}
