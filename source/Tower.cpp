#include "Tower.hpp"

#include <algorithm>
#include <cstddef>

namespace {

struct TowerStats {
    float range;
    float attackIntervalSeconds;
    ProjectilePayload payload;
};

TowerStats statsFor(TowerType type) {
    switch (type) {
        case TowerType::Mortar:
            return {2.85F, 1.25F, {ProjectileEffect::Splash, 2, 1.15F, 0.0F, 1.0F}};
        case TowerType::Frost:
            return {3.05F, 0.90F, {ProjectileEffect::Frost, 1, 0.85F, 2.20F, 0.55F}};
        case TowerType::Ballista:
        default:
            return {3.40F, 0.55F, {ProjectileEffect::Direct, 1, 0.0F, 0.0F, 1.0F}};
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
        case TowerType::Ballista:
        default: return 60;
    }
}

const char* towerName(TowerType type) {
    switch (type) {
        case TowerType::Mortar: return "MOZDZIERZ";
        case TowerType::Frost: return "MROZ";
        case TowerType::Ballista:
        default: return "KUSZA";
    }
}

Tower::Tower(const LevelData& level, std::size_t gridX, std::size_t gridZ, TowerType type)
    : gridX_(gridX), gridZ_(gridZ), type_(type) {
    if (gridX >= level.width || gridZ >= level.height ||
        level.tileAt(gridX, gridZ) != TileType::BuildSpot) {
        return;
    }

    x_ = worldX(level, gridX);
    z_ = worldZ(level, gridZ);
    valid_ = true;
}

void Tower::update(float deltaSeconds, Wave& wave, ProjectilePool& projectiles) {
    if (!valid_ || wave.completed() || wave.lost()) {
        return;
    }

    const TowerStats stats = statsFor(type_);
    const float rangeSquared = stats.range * stats.range;
    cooldown_ = std::max(cooldown_ - std::max(deltaSeconds, 0.0F), 0.0F);
    if (cooldown_ > 0.0F) {
        return;
    }

    std::size_t targetIndex = wave.spawnedCount();
    float bestProgress = -1.0F;
    for (std::size_t index = 0; index < wave.spawnedCount(); ++index) {
        const Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead() || enemy.reachedBase()) {
            continue;
        }

        const float dx = enemy.x() - x_;
        const float dz = enemy.z() - z_;
        if (dx * dx + dz * dz > rangeSquared) {
            continue;
        }

        const float progress = enemy.pathProgress();
        if (targetIndex == wave.spawnedCount() || progress > bestProgress) {
            targetIndex = index;
            bestProgress = progress;
        }
    }

    if (targetIndex < wave.spawnedCount() &&
        projectiles.launch(x_, kMuzzleHeight, z_, targetIndex, stats.payload)) {
        cooldown_ = stats.attackIntervalSeconds;
        ++shotsFired_;
    }
}

void Tower::resetCombat() {
    cooldown_ = 0.0F;
    shotsFired_ = 0;
}

float Tower::x() const { return x_; }
float Tower::z() const { return z_; }
std::size_t Tower::gridX() const { return gridX_; }
std::size_t Tower::gridZ() const { return gridZ_; }
TowerType Tower::type() const { return type_; }
bool Tower::valid() const { return valid_; }
int Tower::shotsFired() const { return shotsFired_; }