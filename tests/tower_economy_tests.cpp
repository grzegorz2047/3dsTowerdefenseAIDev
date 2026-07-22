#include <cstdlib>
#include <iostream>

#include "Economy.hpp"
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
    level.width = 4;
    level.height = 4;
    level.tiles[1 * kMaximumMapWidth + 1] = TileType::BuildSpot;
    return level;
}

void testUpgradeCostsAndMaximumLevel() {
    const LevelData level = makeLevel();
    Tower tower(level, 1, 1, TowerType::Ballista);
    expect(tower.valid(), "tower fixture should be valid");
    expect(tower.level() == 1, "new tower should start at level one");
    expect(tower.investedGold() == 60, "initial investment should equal build cost");
    expect(tower.upgradeCost() == 60, "level two ballista upgrade should cost base price");

    expect(tower.upgrade(), "first upgrade should succeed");
    expect(tower.level() == 2, "first upgrade should reach level two");
    expect(tower.investedGold() == 120, "first upgrade should increase invested gold");
    expect(tower.upgradeCost() == 90, "level three upgrade should cost 150 percent of base");

    expect(tower.upgrade(), "second upgrade should succeed");
    expect(tower.level() == Tower::kMaximumLevel, "second upgrade should reach maximum level");
    expect(!tower.canUpgrade(), "maximum-level tower must reject further upgrades");
    expect(tower.upgradeCost() == 0, "maximum-level tower should expose no upgrade cost");
    expect(!tower.upgrade(), "upgrade beyond maximum should be idempotently rejected");
}

void testSaleValueUsesTotalInvestment() {
    const LevelData level = makeLevel();
    Tower tower(level, 1, 1, TowerType::Mortar);
    expect(tower.investedGold() == 90, "mortar investment should equal build price");
    expect(tower.sellValue() == 63, "fresh mortar should refund 70 percent");
    expect(tower.upgrade(), "mortar upgrade should succeed");
    expect(tower.investedGold() == 180, "upgraded mortar should track total investment");
    expect(tower.sellValue() == 126, "upgraded mortar refund should use total investment");
}

void testEconomyCreditIsPositiveAndAtomic() {
    Economy economy;
    economy.reset();
    expect(!economy.credit(0), "zero credit should be rejected");
    expect(!economy.credit(-1), "negative credit should be rejected");
    expect(economy.gold() == Economy::kInitialGold, "rejected credit must not change gold");
    expect(economy.trySpend(60), "fixture spend should succeed");
    expect(economy.credit(42), "positive refund should succeed");
    expect(economy.gold() == Economy::kInitialGold - 60 + 42, "refund should be credited exactly once");
}

}  // namespace

int main() {
    testUpgradeCostsAndMaximumLevel();
    testSaleValueUsesTotalInvestment();
    testEconomyCreditIsPositiveAndAtomic();
    std::cout << "Tower economy tests passed\n";
    return 0;
}