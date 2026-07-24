#include <cmath>
#include <cstdlib>
#include <iostream>

#include "EnemyVisual.hpp"
#include "Level.hpp"
#include "Projectile.hpp"
#include "Tower.hpp"
#include "Wave.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

LevelData makeLevel() {
    LevelData level{};
    level.id = "tower-archetypes";
    level.name = "Tower Archetypes";
    level.width = 8;
    level.height = 5;
    for (std::size_t z = 0; z < level.height; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            level.tiles[z * kMaximumMapWidth + x] = TileType::Ground;
        }
    }
    level.tiles[2 * kMaximumMapWidth] = TileType::Spawn;
    for (std::size_t x = 1; x < 7; ++x) level.tiles[2 * kMaximumMapWidth + x] = TileType::Road;
    level.tiles[2 * kMaximumMapWidth + 7] = TileType::Base;
    level.tiles[1 * kMaximumMapWidth + 2] = TileType::BuildSpot;
    level.tiles[1 * kMaximumMapWidth + 3] = TileType::BuildSpot;
    level.tiles[1 * kMaximumMapWidth + 4] = TileType::BuildSpot;
    level.tiles[3 * kMaximumMapWidth + 3] = TileType::BuildSpot;
    level.pathLength = 8;
    for (std::size_t x = 0; x < level.pathLength; ++x) {
        level.path[x] = {static_cast<std::int16_t>(x), 2};
    }
    level.waveEntryCount = 1;
    level.waveEntries[0] = {EnemyType::Raider, 3, 0.1F};
    level.totalEnemyCount = 3;
    return level;
}

void startAndSpawn(Wave& wave, float seconds = 0.2F) {
    expect(wave.startNextWave(), "tower test wave should start explicitly");
    wave.update(seconds);
    expect(wave.spawnedCount() >= 1, "tower test target should be spawned");
}

void testEnemyVisualProfilesAndHitReaction() {
    const EnemyVisualProfile scout = EnemyVisual::profile(EnemyType::Scout);
    const EnemyVisualProfile raider = EnemyVisual::profile(EnemyType::Raider);
    const EnemyVisualProfile brute = EnemyVisual::profile(EnemyType::Brute);
    expect(scout.halfWidth < raider.halfWidth, "scout silhouette should be narrower than raider");
    expect(raider.halfWidth < brute.halfWidth, "brute silhouette should be widest");
    expect(brute.halfWidth >= scout.halfWidth * 1.5F,
        "brute should be at least one and a half times wider than scout");
    expect(scout.bodyHeight < raider.bodyHeight && raider.bodyHeight < brute.bodyHeight,
        "enemy silhouettes should increase in height by threat class");
    expect(EnemyVisual::typeIndex(EnemyType::Scout) != EnemyVisual::typeIndex(EnemyType::Raider),
        "scout and raider should use different mesh ranges");
    expect(EnemyVisual::typeIndex(EnemyType::Raider) != EnemyVisual::typeIndex(EnemyType::Brute),
        "raider and brute should use different mesh ranges");
    expect(!EnemyVisual::shouldShowHealthBar(EnemyType::Scout, 2, 2),
        "healthy scout should not clutter the battlefield with a health bar");
    expect(EnemyVisual::shouldShowHealthBar(EnemyType::Scout, 1, 2),
        "injured scout should expose its remaining health");
    expect(EnemyVisual::shouldShowHealthBar(EnemyType::Raider, 3, 3),
        "raider should always expose health");
    expect(EnemyVisual::shouldShowHealthBar(EnemyType::Brute, 5, 5),
        "brute should always expose health");
    expect(EnemyVisual::healthBucket(10, 10) == 10U, "full health should fill all segments");
    expect(EnemyVisual::healthBucket(5, 10) == 5U, "half health should fill half the bar");
    expect(EnemyVisual::healthBucket(0, 10) == 0U, "dead enemy should have empty health bar");

    const LevelData level = makeLevel();
    Enemy enemy(level, EnemyType::Raider);
    expect(!enemy.hitFlashActive(), "enemy should not flash before taking damage");
    enemy.takeDamage(1);
    expect(enemy.hitFlashActive(), "real damage should activate hit feedback");
    enemy.update(0.13F);
    expect(!enemy.hitFlashActive(), "hit feedback should expire after a short deterministic window");
}

void testTowerCatalog() {
    expect(towerCost(TowerType::Ballista) < towerCost(TowerType::Frost), "ballista should be cheapest");
    expect(towerCost(TowerType::Frost) < towerCost(TowerType::Mortar), "frost should cost less than mortar");
    expect(towerCost(TowerType::Mortar) < towerCost(TowerType::Rocket), "rocket launcher should be premium defense");
    expect(nextTowerType(TowerType::Ballista) == TowerType::Mortar, "selection should advance to mortar");
    expect(nextTowerType(TowerType::Mortar) == TowerType::Frost, "selection should advance to frost");
    expect(nextTowerType(TowerType::Frost) == TowerType::Rocket, "selection should advance to rockets");
    expect(nextTowerType(TowerType::Rocket) == TowerType::Ballista, "selection should wrap after rockets");
    expect(previousTowerType(TowerType::Ballista) == TowerType::Rocket, "backward selection should wrap to rockets");
}

void testTowerSpecializationProfiles() {
    const TowerCombatProfile ballista1 = towerCombatProfile(TowerType::Ballista, 1);
    const TowerCombatProfile ballista2 = towerCombatProfile(TowerType::Ballista, 2);
    const TowerCombatProfile ballista3 = towerCombatProfile(TowerType::Ballista, 3);
    const float ballistaEfficiency1 = towerSingleTargetDps(TowerType::Ballista, 1) /
        static_cast<float>(towerCost(TowerType::Ballista));
    const float ballistaEfficiency2 = towerSingleTargetDps(TowerType::Ballista, 2) /
        static_cast<float>(towerCost(TowerType::Ballista) * 2);
    const float ballistaEfficiency3 = towerSingleTargetDps(TowerType::Ballista, 3) / 210.0F;
    expect(ballista1.damage < ballista2.damage && ballista2.damage < ballista3.damage,
        "ballista should specialize in single-target damage");
    expect(ballistaEfficiency2 <= ballistaEfficiency1 + 0.0001F,
        "ballista level two must not improve gold-to-DPS efficiency");
    expect(ballistaEfficiency3 <= ballistaEfficiency1 + 0.0001F,
        "ballista level three must not dominate base efficiency");

    const TowerCombatProfile mortar1 = towerCombatProfile(TowerType::Mortar, 1);
    const TowerCombatProfile mortar3 = towerCombatProfile(TowerType::Mortar, 3);
    expect(mortar3.radius >= mortar1.radius + 0.40F,
        "mortar specialization should substantially increase area");
    expect(mortar3.attackIntervalSeconds > mortar1.attackIntervalSeconds,
        "larger mortar impacts should trade away firing speed");

    const TowerCombatProfile frost1 = towerCombatProfile(TowerType::Frost, 1);
    const TowerCombatProfile frost3 = towerCombatProfile(TowerType::Frost, 3);
    expect(frost3.damage == frost1.damage,
        "frost upgrades should not scale direct damage");
    expect(frost3.slowDurationSeconds > frost1.slowDurationSeconds,
        "frost upgrades should extend control duration");
    expect(frost3.slowMovementMultiplier < frost1.slowMovementMultiplier,
        "frost upgrades should deepen the slow");

    const TowerCombatProfile rocket1 = towerCombatProfile(TowerType::Rocket, 1);
    const TowerCombatProfile rocket3 = towerCombatProfile(TowerType::Rocket, 3);
    expect(rocket3.damage >= rocket1.damage * 2,
        "rocket specialization should deliver heavy burst damage");
    expect(rocket3.range > rocket1.range,
        "rocket specialization should extend premium range");
    expect(rocket3.attackIntervalSeconds > rocket1.attackIntervalSeconds,
        "rocket burst should retain a slow firing cycle");

    expect(ballista1.damageType == DamageType::Physical,
        "ballista role should remain physical single target");
    expect(mortar1.damageType == DamageType::Explosive,
        "mortar role should bypass physical armor");
    expect(frost1.damageType == DamageType::Arcane,
        "frost role should remain control and support");
    expect(rocket1.damageType == DamageType::Explosive,
        "rocket role should remain explosive heavy target");
}

void testTowersLaunchDistinctPayloads() {
    const LevelData level = makeLevel();
    Wave wave(level);
    startAndSpawn(wave);

    const TowerType types[] = {
        TowerType::Ballista, TowerType::Mortar, TowerType::Frost, TowerType::Rocket};
    const ProjectileEffect effects[] = {
        ProjectileEffect::Direct, ProjectileEffect::Splash,
        ProjectileEffect::Frost, ProjectileEffect::GuidedRocket};
    for (std::size_t index = 0; index < 4; ++index) {
        Tower tower(level, 2, 1, types[index]);
        ProjectilePool projectiles;
        tower.update(1.0F, wave, projectiles);
        expect(projectiles.activeCount() == 1, "each defense should launch one pooled projectile");
        expect(projectiles.projectileAt(0).effect() == effects[index],
            "defense should launch its own projectile effect");
    }
}

void testTowerAimsAtSelectedTarget() {
    const LevelData level = makeLevel();
    Wave wave(level);
    startAndSpawn(wave, 0.11F);
    Tower tower(level, 2, 1, TowerType::Ballista);
    ProjectilePool projectiles;

    tower.update(1.0F / 60.0F, wave, projectiles);
    expect(tower.hasTarget(), "tower should expose an in-range target");
    Enemy& target = wave.enemyAt(0);
    const float expected = std::atan2(target.x() - tower.x(), target.z() - tower.z());
    expect(std::fabs(tower.aimAngleRadians() - expected) < 0.0001F,
        "tower aim should point at selected enemy");

    target.takeDamage(99);
    tower.update(1.0F / 60.0F, wave, projectiles);
    expect(!tower.hasTarget(), "tower should clear target state when no valid enemy remains");
}

void testSplashDamagesNearbyEnemies() {
    const LevelData level = makeLevel();
    Wave wave(level);
    startAndSpawn(wave);
    const int firstHealth = wave.enemyAt(0).health();
    const int secondHealth = wave.enemyAt(1).health();

    ProjectilePool projectiles;
    const ProjectilePayload splash{ProjectileEffect::Splash, 2, 1.2F, 0.0F, 1.0F};
    expect(projectiles.launch(wave.enemyAt(0).x(), 0.48F, wave.enemyAt(0).z(), 0, splash),
        "splash projectile should launch");
    projectiles.update(1.0F / 60.0F, wave);

    expect(wave.enemyAt(0).health() == firstHealth - 2, "splash should damage its target");
    expect(wave.enemyAt(1).health() == secondHealth - 2, "splash should damage nearby enemy");
}

void testGuidedRocketCurvesAndImpacts() {
    const LevelData level = makeLevel();
    Wave wave(level);
    startAndSpawn(wave);
    const int firstHealth = wave.enemyAt(0).health();
    const int secondHealth = wave.enemyAt(1).health();

    ProjectilePool projectiles;
    const ProjectilePayload rocket{
        ProjectileEffect::GuidedRocket, 2, 1.4F, 0.0F, 1.0F, 4.5F, 4.5F, 0.45F};
    expect(projectiles.launch(-1.5F, 1.05F, -1.5F, 0, rocket), "guided rocket should launch");
    const Projectile& launched = projectiles.projectileAt(0);
    const float initialX = launched.velocityX();
    const float initialY = launched.velocityY();
    const float initialZ = launched.velocityZ();
    expect(initialY > 0.0F, "rocket should initially climb");

    projectiles.update(0.05F, wave);
    const Projectile& turning = projectiles.projectileAt(0);
    const float directionDelta = std::fabs(turning.velocityX() - initialX) +
        std::fabs(turning.velocityY() - initialY) + std::fabs(turning.velocityZ() - initialZ);
    expect(directionDelta > 0.01F, "rocket guidance should bend the velocity vector");
    expect(turning.active(), "rocket should not teleport into the target on first update");

    for (int step = 0; step < 300 && projectiles.activeCount() > 0; ++step) {
        projectiles.update(1.0F / 60.0F, wave);
    }
    expect(projectiles.activeCount() == 0, "rocket should finish its bounded flight");
    expect(projectiles.impactEventCount() == 1U, "guided rocket should register one impact");
    expect(wave.enemyAt(0).health() < firstHealth, "rocket should damage target");
    expect(wave.enemyAt(1).health() < secondHealth, "rocket explosion should damage nearby enemy");
}

void testFrostSlowsAndExpires() {
    const LevelData level = makeLevel();
    Wave wave(level);
    startAndSpawn(wave, 0.11F);
    Enemy& target = wave.enemyAt(0);
    const float normalSpeed = target.movementSpeed();

    wave.applyAreaEffect(target.x(), target.z(), 0.5F, 0, 0.5F, 0.5F);
    expect(target.slowed(), "frost should mark enemy as slowed");
    expect(std::fabs(target.effectiveMovementSpeed() - normalSpeed * 0.5F) < 0.0001F,
        "frost should reduce movement speed");
    target.update(0.6F);
    expect(!target.slowed(), "slow should expire deterministically");
    expect(std::fabs(target.effectiveMovementSpeed() - normalSpeed) < 0.0001F,
        "speed should recover after slow expires");
}

}  // namespace

int main() {
    testEnemyVisualProfilesAndHitReaction();
    testTowerCatalog();
    testTowerSpecializationProfiles();
    testTowersLaunchDistinctPayloads();
    testTowerAimsAtSelectedTarget();
    testSplashDamagesNearbyEnemies();
    testGuidedRocketCurvesAndImpacts();
    testFrostSlowsAndExpires();
    std::cout << "Tower archetype tests passed\n";
    return 0;
}
