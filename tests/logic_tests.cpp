#include <cmath>
#include <cstdlib>
#include <iostream>

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

}  // namespace

int main() {
    testEnemyTimePartitioning();
    testWaveLossWithoutTowers();
    testTowerCanWinWave();
    testTowerPlacementRules();
    std::cout << "All host gameplay tests passed.\n";
    return 0;
}
