#include "Tower.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

struct TowerStats {
    float range;
    float attackIntervalSeconds;
    ProjectilePayload payload;
};

TowerStats baseStatsFor(TowerType type) {
    switch (type) {
        case TowerType::Mortar:
            return {2.85F, 1.25F,
                {ProjectileEffect::Splash, 2, 1.15F, 0.0F, 1.0F,
                    6.0F, 0.0F, 0.0F, DamageType::Explosive}};
        case TowerType::Frost:
            return {3.05F, 0.90F,
                {ProjectileEffect::Frost, 1, 0.85F, 2.20F, 0.55F,
                    6.0F, 0.0F, 0.0F, DamageType::Arcane}};
        case TowerType::Rocket:
            return {4.45F, 2.35F,
                {ProjectileEffect::GuidedRocket, 3, 1.35F, 0.0F, 1.0F,
                    4.5F, 4.5F, 0.45F, DamageType::Explosive}};
        case TowerType::Ballista:
        default:
            return {3.40F, 0.55F,
                {ProjectileEffect::Direct, 1, 0.0F, 0.0F, 1.0F,
                    6.0F, 0.0F, 0.0F, DamageType::Physical}};
    }
}

TowerStats statsFor(TowerType type, std::uint8_t level) {
    TowerStats stats = baseStatsFor(type);
    const int extraLevels = static_cast<int>(level) - 1;
    stats.range += 0.18F * static_cast<float>(extraLevels);
    stats.attackIntervalSeconds *= 1.0F - 0.12F * static_cast<float>(extraLevels);
    stats.payload.damage += extraLevels;
    if (stats.payload.effect == ProjectileEffect::Splash ||
        stats.payload.effect == ProjectileEffect::GuidedRocket) {
        stats.payload.radius += 0.12F * static_cast<float>(extraLevels);
    }
    if (stats.payload.effect == ProjectileEffect::Frost) {
        stats.payload.slowDurationSeconds += 0.35F * static_cast<float>(extraLevels);
        stats.payload.slowMovementMultiplier = std::max(
            0.40F,
            stats.payload.slowMovementMultiplier - 0.05F * static_cast<float>(extraLevels));
    }
    if (stats.payload.effect == ProjectileEffect::GuidedRocket) {
        stats.payload.turnRateRadiansPerSecond += 0.30F * static_cast<float>(extraLevels);
        stats.payload.speed += 0.25F * static_cast<float>(extraLevels);
    }
    return stats;
}

float worldX(const LevelData& level, std::size_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::size_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

constexpr float kMuzzleHeight = 1.05F;

}  // namespace

int towerCost(TowerType type) {
    switch (type) {
        case TowerType::Mortar: return 90;
        case TowerType::Frost: return 75;
        case TowerType::Rocket: return 120;
        case TowerType::Ballista:
        default: return 60;
    }
}

const char* towerName(TowerType type) {
    switch (type) {
        case TowerType::Mortar: return "MOZDZIERZ";
        case TowerType::Frost: return "MROZ";
        case TowerType::Rocket: return "RAKIETY";
        case TowerType::Ballista:
        default: return "KUSZA";
    }
}

Tower::Tower(const LevelData& level, std::size_t gridX, std::size_t gridZ, TowerType type)
    : gridX_(gridX), gridZ_(gridZ), type_(type), investedGold_(towerCost(type)) {
    if (gridX >= level.width || gridZ >= level.height ||
        level.tileAt(gridX, gridZ) != TileType::BuildSpot) {
        investedGold_ = 0;
        return;
    }

    x_ = worldX(level, gridX);
    z_ = worldZ(level, gridZ);
    valid_ = true;
}

void Tower::update(float deltaSeconds, Wave& wave, ProjectilePool& projectiles) {
    if (!valid_ || wave.completed() || wave.lost()) {
        hasTarget_ = false;
        return;
    }

    const TowerStats stats = statsFor(type_, level_);
    const float rangeSquared = stats.range * stats.range;
    cooldown_ = std::max(cooldown_ - std::max(deltaSeconds, 0.0F), 0.0F);

    std::size_t targetIndex = wave.spawnedCount();
    float bestProgress = -1.0F;
    for (std::size_t index = 0; index < wave.spawnedCount(); ++index) {
        const Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead() || enemy.reachedBase()) continue;

        const float dx = enemy.x() - x_;
        const float dz = enemy.z() - z_;
        if (dx * dx + dz * dz > rangeSquared) continue;

        const float progress = enemy.pathProgress();
        if (targetIndex == wave.spawnedCount() || progress > bestProgress) {
            targetIndex = index;
            bestProgress = progress;
        }
    }

    hasTarget_ = targetIndex < wave.spawnedCount();
    if (!hasTarget_) return;

    const Enemy& target = wave.enemyAt(targetIndex);
    aimAngleRadians_ = std::atan2(target.x() - x_, target.z() - z_);
    if (cooldown_ > 0.0F) return;

    if (projectiles.launch(x_, kMuzzleHeight, z_, targetIndex, stats.payload)) {
        cooldown_ = stats.attackIntervalSeconds;
        ++shotsFired_;
    }
}

void Tower::resetCombat() {
    aimAngleRadians_ = 0.0F;
    cooldown_ = 0.0F;
    shotsFired_ = 0;
    hasTarget_ = false;
}

bool Tower::upgrade() {
    if (!canUpgrade()) return false;
    const int cost = upgradeCost();
    ++level_;
    investedGold_ += cost;
    return true;
}

float Tower::x() const { return x_; }
float Tower::z() const { return z_; }
float Tower::aimAngleRadians() const { return aimAngleRadians_; }
bool Tower::hasTarget() const { return hasTarget_; }
std::size_t Tower::gridX() const { return gridX_; }
std::size_t Tower::gridZ() const { return gridZ_; }
TowerType Tower::type() const { return type_; }
std::uint8_t Tower::level() const { return level_; }
int Tower::investedGold() const { return investedGold_; }
int Tower::upgradeCost() const {
    return canUpgrade() ? towerCost(type_) * static_cast<int>(level_ + 1U) / 2 : 0;
}
int Tower::sellValue() const { return investedGold_ * 70 / 100; }
bool Tower::canUpgrade() const { return valid_ && level_ < kMaximumLevel; }
bool Tower::valid() const { return valid_; }
int Tower::shotsFired() const { return shotsFired_; }
