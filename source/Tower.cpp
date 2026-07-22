#include "Tower.hpp"

#include <algorithm>
#include <cstddef>

namespace {

constexpr float kRange = 3.4F;
constexpr float kRangeSquared = kRange * kRange;
constexpr float kAttackIntervalSeconds = 0.55F;
constexpr int kDamage = 1;
constexpr float kMuzzleHeight = 1.05F;

float worldX(const LevelData& level, std::size_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::size_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

}  // namespace

Tower::Tower(const LevelData& level, std::size_t gridX, std::size_t gridZ)
    : gridX_(gridX), gridZ_(gridZ) {
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
        if (dx * dx + dz * dz > kRangeSquared) {
            continue;
        }

        const float progress = enemy.pathProgress();
        if (targetIndex == wave.spawnedCount() || progress > bestProgress) {
            targetIndex = index;
            bestProgress = progress;
        }
    }

    if (targetIndex < wave.spawnedCount() &&
        projectiles.launch(x_, kMuzzleHeight, z_, targetIndex, kDamage)) {
        cooldown_ = kAttackIntervalSeconds;
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
bool Tower::valid() const { return valid_; }
int Tower::shotsFired() const { return shotsFired_; }
