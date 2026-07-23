#pragma once

#include <cstddef>
#include <cstdint>

#include "Level.hpp"
#include "Projectile.hpp"
#include "Wave.hpp"

enum class TowerType : std::uint8_t {
    Ballista,
    Mortar,
    Frost,
    Rocket,
};

[[nodiscard]] constexpr TowerType nextTowerType(TowerType type) {
    switch (type) {
        case TowerType::Ballista: return TowerType::Mortar;
        case TowerType::Mortar: return TowerType::Frost;
        case TowerType::Frost: return TowerType::Rocket;
        case TowerType::Rocket:
        default: return TowerType::Ballista;
    }
}

[[nodiscard]] constexpr TowerType previousTowerType(TowerType type) {
    switch (type) {
        case TowerType::Ballista: return TowerType::Rocket;
        case TowerType::Mortar: return TowerType::Ballista;
        case TowerType::Frost: return TowerType::Mortar;
        case TowerType::Rocket:
        default: return TowerType::Frost;
    }
}

[[nodiscard]] int towerCost(TowerType type);
[[nodiscard]] const char* towerName(TowerType type);

class Tower {
public:
    static constexpr std::uint8_t kMaximumLevel = 3;

    Tower() = default;
    Tower(const LevelData& level, std::size_t gridX, std::size_t gridZ, TowerType type = TowerType::Ballista);

    void update(float deltaSeconds, Wave& wave, ProjectilePool& projectiles);
    void resetCombat();
    [[nodiscard]] bool upgrade();

    [[nodiscard]] float x() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] float aimAngleRadians() const;
    [[nodiscard]] bool hasTarget() const;
    [[nodiscard]] std::size_t gridX() const;
    [[nodiscard]] std::size_t gridZ() const;
    [[nodiscard]] TowerType type() const;
    [[nodiscard]] std::uint8_t level() const;
    [[nodiscard]] int investedGold() const;
    [[nodiscard]] int upgradeCost() const;
    [[nodiscard]] int sellValue() const;
    [[nodiscard]] bool canUpgrade() const;
    [[nodiscard]] bool valid() const;
    [[nodiscard]] int shotsFired() const;

private:
    float x_ = 0.0F;
    float z_ = 0.0F;
    float aimAngleRadians_ = 0.0F;
    float cooldown_ = 0.0F;
    std::size_t gridX_ = 0;
    std::size_t gridZ_ = 0;
    TowerType type_ = TowerType::Ballista;
    std::uint8_t level_ = 1;
    int investedGold_ = 0;
    int shotsFired_ = 0;
    bool hasTarget_ = false;
    bool valid_ = false;
};
