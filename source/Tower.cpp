#include "Tower.hpp"

#include <algorithm>
#include <cstddef>

namespace {

constexpr float kRange = 3.4F;
constexpr float kRangeSquared = kRange * kRange;
constexpr float kAttackIntervalSeconds = 0.55F;
constexpr int kDamage = 1;

float worldX(const LevelData& level, std::size_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::size_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

}  // namespace

Tower::Tower(const LevelData& level) {
    for (std::size_t z = 0; z < level.height && !valid_; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            if (level.tileAt(x, z) == TileType::BuildSpot) {
                x_ = worldX(level, x);
                z_ = worldZ(level, z);
                valid_ = true;
                break;
            }
        }
    }
}

void Tower::update(float deltaSeconds, Wave& wave) {
    if (!valid_ || wave.completed() || wave.lost()) {
        return;
    }

    cooldown_ = std::max(cooldown_ - std::max(deltaSeconds, 0.0F), 0.0F);
    if (cooldown_ > 0.0F) {
        return;
    }

    Enemy* target = nullptr;
    float bestProgress = -1.0F;
    for (std::size_t index = 0; index < wave.spawnedCount(); ++index) {
        Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead() || enemy.reachedBase()) {
            continue;
        }

        const float dx = enemy.x() - x_;
        const float dz = enemy.z() - z_;
        if (dx * dx + dz * dz > kRangeSquared) {
            continue;
        }

        const float progress = enemy.pathProgress();
        if (target == nullptr || progress > bestProgress) {
            target = &enemy;
            bestProgress = progress;
        }
    }

    if (target != nullptr) {
        target->takeDamage(kDamage);
        cooldown_ = kAttackIntervalSeconds;
        ++shotsFired_;
    }
}

void Tower::reset() {
    cooldown_ = 0.0F;
    shotsFired_ = 0;
}

float Tower::x() const {
    return x_;
}

float Tower::z() const {
    return z_;
}

bool Tower::valid() const {
    return valid_;
}

int Tower::shotsFired() const {
    return shotsFired_;
}
