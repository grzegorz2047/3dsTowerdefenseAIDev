#include "Tower.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

struct TowerRuntimeStats {
    TowerCombatProfile profile;
    ProjectileEffect effect;
    float projectileSpeed;
    float turnRateRadiansPerSecond;
    float launchClimb;
};

std::uint8_t boundedLevel(std::uint8_t level) {
    return std::max<std::uint8_t>(1U, std::min<std::uint8_t>(level, Tower::kMaximumLevel));
}

TowerRuntimeStats runtimeStatsFor(TowerType type, std::uint8_t level) {
    const TowerCombatProfile profile = towerCombatProfile(type, level);
    switch (type) {
        case TowerType::Mortar:
            return {profile, ProjectileEffect::Splash, 6.0F, 0.0F, 0.0F};
        case TowerType::Frost:
            return {profile, ProjectileEffect::Frost, 6.0F, 0.0F, 0.0F};
        case TowerType::Rocket:
            return {profile, ProjectileEffect::GuidedRocket,
                level >= 3U ? 5.0F : (level >= 2U ? 4.75F : 4.5F),
                level >= 3U ? 5.2F : (level >= 2U ? 4.8F : 4.5F), 0.45F};
        case TowerType::Ballista:
        default:
            return {profile, ProjectileEffect::Direct, 6.0F, 0.0F, 0.0F};
    }
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

TowerCombatProfile towerCombatProfile(TowerType type, std::uint8_t level) {
    const std::uint8_t bounded = boundedLevel(level);
    switch (type) {
        case TowerType::Mortar:
            if (bounded == 1U) return {2.85F, 1.25F, 3, 1.15F, 0.0F, 1.0F,
                DamageType::Explosive, "GRUPY"};
            if (bounded == 2U) return {2.85F, 1.35F, 4, 1.30F, 0.0F, 1.0F,
                DamageType::Explosive, "WIEKSZY OBSZAR"};
            return {2.95F, 1.50F, 5, 1.60F, 0.0F, 1.0F,
                DamageType::Explosive, "OBLEZENIE"};
        case TowerType::Frost:
            if (bounded == 1U) return {3.05F, 0.90F, 1, 0.85F, 2.20F, 0.55F,
                DamageType::Arcane, "SPOWOLNIENIE"};
            if (bounded == 2U) return {3.10F, 0.88F, 1, 0.95F, 2.90F, 0.50F,
                DamageType::Arcane, "DLUZSZA KONTROLA"};
            return {3.20F, 0.85F, 1, 1.10F, 3.70F, 0.40F,
                DamageType::Arcane, "GLEBOKI MROZ"};
        case TowerType::Rocket:
            if (bounded == 1U) return {4.45F, 2.35F, 3, 1.35F, 0.0F, 1.0F,
                DamageType::Explosive, "DUZY CEL"};
            if (bounded == 2U) return {4.70F, 2.60F, 5, 1.45F, 0.0F, 1.0F,
                DamageType::Explosive, "CIEZKI CEL"};
            return {5.05F, 2.85F, 8, 1.65F, 0.0F, 1.0F,
                DamageType::Explosive, "EGZEKUTOR"};
        case TowerType::Ballista:
        default:
            if (bounded == 1U) return {3.40F, 0.55F, 1, 0.0F, 0.0F, 1.0F,
                DamageType::Physical, "SZYBKI CEL"};
            if (bounded == 2U) return {3.40F, 0.65F, 2, 0.0F, 0.0F, 1.0F,
                DamageType::Physical, "PRECYZJA"};
            return {3.50F, 0.60F, 3, 0.0F, 0.0F, 1.0F,
                DamageType::Physical, "LOWCA"};
    }
}

float towerSingleTargetDps(TowerType type, std::uint8_t level) {
    const TowerCombatProfile profile = towerCombatProfile(type, level);
    return profile.attackIntervalSeconds > 0.0F
        ? static_cast<float>(profile.damage) / profile.attackIntervalSeconds : 0.0F;
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

    const TowerRuntimeStats stats = runtimeStatsFor(type_, level_);
    const float rangeSquared = stats.profile.range * stats.profile.range;
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

    const ProjectilePayload payload{
        stats.effect, stats.profile.damage, stats.profile.radius,
        stats.profile.slowDurationSeconds, stats.profile.slowMovementMultiplier,
        stats.projectileSpeed, stats.turnRateRadiansPerSecond, stats.launchClimb,
        stats.profile.damageType};
    if (projectiles.launch(x_, kMuzzleHeight, z_, targetIndex, payload)) {
        cooldown_ = stats.profile.attackIntervalSeconds;
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
TowerCombatProfile Tower::combatProfile() const { return towerCombatProfile(type_, level_); }
TowerCombatProfile Tower::nextCombatProfile() const {
    return towerCombatProfile(type_, canUpgrade() ? static_cast<std::uint8_t>(level_ + 1U) : level_);
}
int Tower::investedGold() const { return investedGold_; }
int Tower::upgradeCost() const {
    return canUpgrade() ? towerCost(type_) * static_cast<int>(level_ + 1U) / 2 : 0;
}
int Tower::sellValue() const { return investedGold_ * 70 / 100; }
bool Tower::canUpgrade() const { return valid_ && level_ < kMaximumLevel; }
bool Tower::valid() const { return valid_; }
int Tower::shotsFired() const { return shotsFired_; }
