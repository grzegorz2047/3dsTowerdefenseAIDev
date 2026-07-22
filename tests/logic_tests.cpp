#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Economy.hpp"
#include "Enemy.hpp"
#include "Level.hpp"
#include "Projectile.hpp"
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

    level.waveEntryCount = 1;
    level.waveEntries[0] = {EnemyType::Raider, 5, 1.15F};
    level.totalEnemyCount = 5;
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
    require(level.waveEntryCount == 3, "tutorial should define three wave entries");
    require(level.totalEnemyCount == 5, "tutorial should define five enemies");
    require(level.waveEntries[0].type == EnemyType::Scout, "tutorial should start with scouts");
    require(level.waveEntries[1].type == EnemyType::Raider, "tutorial should include raiders");
    require(level.waveEntries[2].type == EnemyType::Brute, "tutorial should end with a brute");

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

void testEnemyClassesHaveDistinctStats() {
    const LevelData level = makeLevel();
    const Enemy scout(level, EnemyType::Scout);
    const Enemy raider(level, EnemyType::Raider);
    const Enemy brute(level, EnemyType::Brute);

    require(scout.movementSpeed() > raider.movementSpeed(), "scout should be faster than raider");
    require(raider.movementSpeed() > brute.movementSpeed(), "raider should be faster than brute");
    require(scout.maxHealth() < raider.maxHealth(), "scout should have less health than raider");
    require(raider.maxHealth() < brute.maxHealth(), "brute should have the most health");
    require(brute.baseDamage() == 2, "brute should deal two base damage");
}

void testWaveUsesLevelDefinitions() {
    LevelData level = makeLevel();
    level.waveEntryCount = 3;
    level.waveEntries[0] = {EnemyType::Scout, 2, 0.25F};
    level.waveEntries[1] = {EnemyType::Raider, 1, 0.50F};
    level.waveEntries[2] = {EnemyType::Brute, 1, 0.75F};
    level.totalEnemyCount = 4;

    Wave wave(level);
    require(wave.enemyCount() == 4, "wave enemy count should come from level data");
    require(wave.enemyAt(0).type() == EnemyType::Scout, "first enemy should be a scout");
    require(wave.enemyAt(1).type() == EnemyType::Scout, "second enemy should be a scout");
    require(wave.enemyAt(2).type() == EnemyType::Raider, "third enemy should be a raider");
    require(wave.enemyAt(3).type() == EnemyType::Brute, "fourth enemy should be a brute");

    wave.update(0.24F);
    require(wave.spawnedCount() == 1, "second scout should respect its spawn interval");
    wave.update(0.01F);
    require(wave.spawnedCount() == 2, "second scout should spawn from level timing");
    wave.update(0.50F);
    require(wave.spawnedCount() == 3, "raider should spawn from level timing");
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
    ProjectilePool projectiles;
    require(tower.valid(), "test tower should be placed on BuildSpot");

    for (int step = 0; step < 60 * 30 && !wave.completed() && !wave.lost(); ++step) {
        tower.update(1.0F / 60.0F, wave, projectiles);
        projectiles.update(1.0F / 60.0F, wave);
        wave.update(1.0F / 60.0F);
    }

    require(wave.completed(), "tower should defeat the complete test wave");
    require(!wave.lost(), "winning wave must preserve base health");
    require(wave.baseHealth() > 0, "winning wave should leave the base alive");
    require(tower.shotsFired() > 0, "tower should fire during the test");
}

void testProjectileDamagesOnlyOnImpact() {
    const LevelData level = makeLevel();
    Wave wave(level);
    ProjectilePool projectiles;
    Enemy& target = wave.enemyAt(0);
    const int initialHealth = target.health();

    require(projectiles.launch(target.x() - 2.0F, 0.48F, target.z(), 0, 1), "projectile launch should reserve a slot");
    require(target.health() == initialHealth, "launch must not apply immediate damage");
    projectiles.update(1.0F / 60.0F, wave);
    require(target.health() == initialHealth, "projectile must not damage before reaching target");

    for (int step = 0; step < 120 && projectiles.activeCount() > 0; ++step) {
        projectiles.update(1.0F / 60.0F, wave);
    }
    require(projectiles.activeCount() == 0, "projectile should resolve after impact");
    require(target.health() == initialHealth - 1, "projectile should apply damage exactly on impact");
}

void testProjectileDropsLostTarget() {
    const LevelData level = makeLevel();
    Wave wave(level);
    ProjectilePool projectiles;
    Enemy& target = wave.enemyAt(0);

    require(projectiles.launch(target.x() - 2.0F, 0.48F, target.z(), 0, 1), "projectile launch should succeed");
    target.takeDamage(99);
    projectiles.update(1.0F / 60.0F, wave);
    require(projectiles.activeCount() == 0, "projectile should deactivate when target is already dead");
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
    testEnemyClassesHaveDistinctStats();
    testWaveUsesLevelDefinitions();
    testWaveLossWithoutTowers();
    testTowerCanWinWave();
    testProjectileDamagesOnlyOnImpact();
    testProjectileDropsLostTarget();
    testTowerPlacementRules();
    testEconomySpendsExactlyOnce();
    testEconomyRewardsEachEnemyOnce();
    std::cout << "All host gameplay tests passed.\n";
    return 0;
}
