#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Economy.hpp"
#include "Enemy.hpp"
#include "Level.hpp"
#include "Tower.hpp"
#include "Wave.hpp"

namespace {

[[noreturn]] void fail(const char* message) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
}

void require(bool condition, const char* message) {
    if (!condition) {
        fail(message);
    }
}

LevelData makeLevel() {
    LevelData level{};
    level.id = "host-test";
    level.name = "Host Test";
    level.width = 8;
    level.height = 5;

    for (std::size_t z = 0; z < level.height; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            level.tiles[z * kMaximumMapWidth + x] = TileType::Ground;
        }
    }

    level.tiles[2 * kMaximumMapWidth + 0] = TileType::Spawn;
    for (std::size_t x = 1; x < 7; ++x) {
        level.tiles[2 * kMaximumMapWidth + x] = TileType::Road;
    }
    level.tiles[2 * kMaximumMapWidth + 7] = TileType::Base;
    level.tiles[1 * kMaximumMapWidth + 3] = TileType::BuildSpot;

    level.pathLength = 8;
    for (std::size_t x = 0; x < level.pathLength; ++x) {
        level.path[x] = {static_cast<std::int16_t>(x), 2};
    }
    return level;
}

void advanceEnemy(Enemy& enemy, float step, int count) {
    for (int index = 0; index < count; ++index) {
        enemy.update(step);
    }
}

void testBundledTutorialLevel() {
    const LevelLoadResult result = LevelLoader::loadFromRomFs("romfs/levels/tutorial.lvl");
    require(result.success, result.error.c_str());

    const LevelData& level = result.level;
    require(level.id == "tutorial", "tutorial id should be stable");
    require(level.width == 12 && level.height == 12, "tutorial dimensions should be 12x12");
    require(level.pathLength == 14, "tutorial path length should be 14");

    const GridPoint first = level.path[0];
    const GridPoint second = level.path[1];
    const GridPoint last = level.path[level.pathLength - 1];
    require(first.x == 1 && first.z == 3, "tutorial path should start at spawn");
    require(second.x == 2 && second.z == 3, "tutorial second path point should remain on road");
    require(level.tileAt(2, 3) == TileType::Road, "tutorial grid must mark path point 2,3 as road");
    require(last.x == 8 && last.z == 9, "tutorial path should end at base");
}

void testEnemyTimePartitioning() {
    const LevelData level = makeLevel();
    Enemy fine(level);
    Enemy coarse(level);

    advanceEnemy(fine, 1.0F / 60.0F, 120);
    advanceEnemy(coarse, 1.0F / 20.0F, 40);

    require(std::fabs(fine.x() - coarse.x()) < 0.0001F, "enemy X depends on frame partition");
    require(std::fabs(fine.z() - coarse.z()) < 0.0001F, "enemy Z depends on frame partition");
    require(std::fabs(fine.pathProgress() - coarse.pathProgress()) < 0.0001F, "enemy progress depends on frame partition");
}

void testWaveLossWithoutTowers() {
    const LevelData level = makeLevel();
    Wave wave(level);

    for (int step = 0; step < 60 * 30 && !wave.lost(); ++step) {
        wave.update(1.0F / 60.0F);
    }

    require(wave.lost(), "unopposed wave should destroy the base");
    require(!wave.completed(), "lost wave must not be completed as victory");
    require(wave.baseHealth() == 0, "base health should reach zero exactly");
}

void testTowerCanWinWave() {
    const LevelData level = makeLevel();
    Wave wave(level);
    Tower tower(level, 3, 1);
    require(tower.valid(), "test tower should be placed on BuildSpot");

    for (int step = 0; step < 60 * 30 && !wave.completed() && !wave.lost(); ++step) {
        tower.update(1.0F / 60.0F, wave);
        wave.update(1.0F / 60.0F);
    }

    require(wave.completed(), "tower should defeat the complete test wave");
    require(!wave.lost(), "winning wave must preserve base health");
    require(wave.baseHealth() > 0, "winning wave should leave the base alive");
    require(tower.shotsFired() > 0, "tower should fire during the test");
}

void testTowerPlacementRules() {
    const LevelData level = makeLevel();
    const Tower valid(level, 3, 1);
    const Tower invalid(level, 2, 2);
    require(valid.valid(), "BuildSpot tower should be valid");
    require(!invalid.valid(), "road tower should be invalid");
}

void testEconomySpendsExactlyOnce() {
    Economy economy;
    economy.reset();

    require(economy.trySpend(Economy::kTowerCost), "first tower cost should be accepted");
    require(economy.gold() == Economy::kInitialGold - Economy::kTowerCost, "first cost should be deducted exactly once");
    require(economy.trySpend(Economy::kTowerCost), "second affordable tower cost should be accepted");
    require(economy.gold() == 0, "two tower costs should consume initial gold exactly");
    require(!economy.trySpend(Economy::kTowerCost), "unaffordable tower cost must be rejected");
    require(economy.gold() == 0, "rejected cost must not change gold");
}

void testEconomyRewardsEachEnemyOnce() {
    Economy economy;
    economy.reset();

    require(economy.rewardEnemy(0), "first reward for enemy should be accepted");
    require(economy.gold() == Economy::kInitialGold + Economy::kKillReward, "first reward should be credited once");
    require(!economy.rewardEnemy(0), "duplicate reward for enemy must be rejected");
    require(economy.gold() == Economy::kInitialGold + Economy::kKillReward, "duplicate reward must not change gold");
    require(economy.rewardEnemy(1), "different enemy should grant a reward");
    require(economy.gold() == Economy::kInitialGold + 2 * Economy::kKillReward, "two unique enemies should grant two rewards");
    require(!economy.rewardEnemy(Economy::kMaximumRewardedEnemies), "out-of-range enemy reward must be rejected");
}

}  // namespace

int main() {
    testBundledTutorialLevel();
    testEnemyTimePartitioning();
    testWaveLossWithoutTowers();
    testTowerCanWinWave();
    testTowerPlacementRules();
    testEconomySpendsExactlyOnce();
    testEconomyRewardsEachEnemyOnce();
    std::cout << "All host gameplay tests passed.\n";
    return 0;
}
