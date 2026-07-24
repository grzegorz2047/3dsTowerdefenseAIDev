#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Economy.hpp"
#include "Enemy.hpp"
#include "Level.hpp"
#include "Projectile.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

LevelData makeLevel() {
    LevelData level{};
    level.id = "armor-test";
    level.name = "Armor Test";
    level.width = 4;
    level.height = 3;
    level.pathLength = 2;
    level.path[0] = {0, 1};
    level.path[1] = {1, 1};
    return level;
}

void testArchetypeRewardsAndArmor() {
    const LevelData level = makeLevel();
    const Enemy scout(level, EnemyType::Scout);
    const Enemy raider(level, EnemyType::Raider);
    const Enemy brute(level, EnemyType::Brute);

    expect(scout.killReward() < raider.killReward(), "scout reward should be lowest");
    expect(raider.killReward() < brute.killReward(), "brute reward should be highest");
    expect(scout.armor(DamageType::Physical) == 0, "scout should not have physical armor");
    expect(raider.armor(DamageType::Physical) == 1, "raider should have light physical armor");
    expect(brute.armor(DamageType::Physical) == 2, "brute should have heavy physical armor");
    expect(brute.armor(DamageType::Explosive) == 0,
        "explosive damage should bypass brute physical armor");
}

void testArmorReductionAndMinimumDamage() {
    const LevelData level = makeLevel();
    Enemy brute(level, EnemyType::Brute);
    const int initial = brute.health();
    brute.takeDamage(3, DamageType::Physical);
    expect(brute.health() == initial - 1, "physical armor should reduce three damage to one");

    Enemy explosiveBrute(level, EnemyType::Brute);
    const int explosiveInitial = explosiveBrute.health();
    explosiveBrute.takeDamage(3, DamageType::Explosive);
    expect(explosiveBrute.health() == explosiveInitial - 3,
        "explosive damage should apply without physical reduction");

    Enemy raider(level, EnemyType::Raider);
    const int raiderInitial = raider.health();
    raider.takeDamage(1, DamageType::Physical);
    expect(raider.health() == raiderInitial - 1,
        "armor must never reduce a valid hit below one damage");
}

void testBruteResistsSlow() {
    const LevelData level = makeLevel();
    Enemy scout(level, EnemyType::Scout);
    Enemy brute(level, EnemyType::Brute);
    scout.applySlow(2.0F, 0.5F);
    brute.applySlow(2.0F, 0.5F);
    expect(scout.slowed() && brute.slowed(), "both enemies should receive a slow state");
    expect(brute.effectiveMovementSpeed() / brute.movementSpeed() >
        scout.effectiveMovementSpeed() / scout.movementSpeed(),
        "brute should retain more movement speed while slowed");
    brute.update(1.2F);
    expect(!brute.slowed(), "brute slow duration should be shortened by resistance");
    expect(scout.slowed(), "scout should still be slowed for the full duration");
}

void testTypedRewardsRemainDeduplicated() {
    Economy economy;
    economy.reset();
    expect(economy.rewardEnemy(0, 8), "scout reward should be accepted");
    expect(economy.gold() == Economy::kInitialGold + 8,
        "scout reward should credit its configured amount");
    expect(!economy.rewardEnemy(0, 28), "same enemy slot must not reward twice");
    economy.beginWave();
    expect(economy.rewardEnemy(0, 28), "same slot should be reusable in a new wave");
    expect(economy.gold() == Economy::kInitialGold + 8 + 28,
        "typed rewards should accumulate across waves");
}

void testProjectileCarriesDamageType() {
    Projectile projectile;
    const ProjectilePayload payload{
        ProjectileEffect::Splash, 3, 1.0F, 0.0F, 1.0F,
        6.0F, 0.0F, 0.0F, DamageType::Explosive};
    projectile.launch(0.0F, 0.5F, 0.0F, 0U, payload);
    expect(projectile.damageType() == DamageType::Explosive,
        "projectile should preserve configured damage category");
}

}  // namespace

int main() {
    testArchetypeRewardsAndArmor();
    testArmorReductionAndMinimumDamage();
    testBruteResistsSlow();
    testTypedRewardsRemainDeduplicated();
    testProjectileCarriesDamageType();
    std::cout << "Enemy economy and armor tests passed\n";
    return 0;
}
