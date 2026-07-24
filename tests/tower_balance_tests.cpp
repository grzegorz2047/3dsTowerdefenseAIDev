#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Enemy.hpp"
#include "Level.hpp"
#include "Tower.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

LevelData makeLevel() {
    LevelData level{};
    level.id = "tower-balance";
    level.name = "Tower Balance";
    level.width = 4;
    level.height = 3;
    level.pathLength = 2;
    level.path[0] = {0, 1};
    level.path[1] = {3, 1};
    return level;
}

float estimatedKillTime(TowerType towerType, std::uint8_t towerLevel,
    const Enemy& enemy) {
    const TowerCombatProfile profile = towerCombatProfile(towerType, towerLevel);
    const int effectiveDamage = std::max(profile.damage - enemy.armor(profile.damageType), 1);
    const int shots = (enemy.maxHealth() + effectiveDamage - 1) / effectiveDamage;
    return static_cast<float>(shots) * profile.attackIntervalSeconds;
}

void testRepresentativeKillTimes() {
    const LevelData level = makeLevel();
    const Enemy scout(level, EnemyType::Scout);
    const Enemy raider(level, EnemyType::Raider);
    const Enemy brute(level, EnemyType::Brute);

    const float ballistaScout = estimatedKillTime(TowerType::Ballista, 1, scout);
    const float mortarScout = estimatedKillTime(TowerType::Mortar, 1, scout);
    expect(ballistaScout < mortarScout,
        "ballista should answer fast unarmored scouts sooner than mortar");

    const float ballistaRaider = estimatedKillTime(TowerType::Ballista, 1, raider);
    const float mortarRaider = estimatedKillTime(TowerType::Mortar, 1, raider);
    expect(mortarRaider < ballistaRaider,
        "explosive mortar should beat physical ballista against armored raider");

    const float ballistaBrute = estimatedKillTime(TowerType::Ballista, 3, brute);
    const float rocketBrute = estimatedKillTime(TowerType::Rocket, 3, brute);
    expect(rocketBrute < ballistaBrute,
        "heavy rocket specialization should beat upgraded ballista against brute");
}

void testUpgradeEfficiencyAndRoles() {
    const float baseBallistaEfficiency = towerSingleTargetDps(TowerType::Ballista, 1) /
        static_cast<float>(towerCost(TowerType::Ballista));
    const float levelTwoInvestment = static_cast<float>(
        towerCost(TowerType::Ballista) + towerCost(TowerType::Ballista));
    const float levelThreeInvestment = levelTwoInvestment +
        static_cast<float>(towerCost(TowerType::Ballista) * 3 / 2);
    expect(towerSingleTargetDps(TowerType::Ballista, 2) / levelTwoInvestment <=
        baseBallistaEfficiency + 0.0001F,
        "ballista level two should add power without improving gold efficiency");
    expect(towerSingleTargetDps(TowerType::Ballista, 3) / levelThreeInvestment <=
        baseBallistaEfficiency + 0.0001F,
        "ballista level three should not dominate alternatives by gold efficiency");

    const TowerCombatProfile mortar1 = towerCombatProfile(TowerType::Mortar, 1);
    const TowerCombatProfile mortar3 = towerCombatProfile(TowerType::Mortar, 3);
    expect(mortar3.radius > mortar1.radius &&
        mortar3.attackIntervalSeconds > mortar1.attackIntervalSeconds,
        "mortar should trade firing speed for area specialization");

    const TowerCombatProfile frost1 = towerCombatProfile(TowerType::Frost, 1);
    const TowerCombatProfile frost3 = towerCombatProfile(TowerType::Frost, 3);
    expect(frost3.damage == frost1.damage &&
        frost3.slowDurationSeconds > frost1.slowDurationSeconds &&
        frost3.slowMovementMultiplier < frost1.slowMovementMultiplier,
        "frost should specialize in control instead of direct damage");
}

}  // namespace

int main() {
    testRepresentativeKillTimes();
    testUpgradeEfficiencyAndRoles();
    std::cout << "Tower balance tests passed\n";
    return 0;
}
