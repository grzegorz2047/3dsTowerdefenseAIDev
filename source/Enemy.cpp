#include "Enemy.hpp"

#include <algorithm>
#include <cmath>

namespace {

struct EnemyStats {
    int health;
    int baseDamage;
    float movementSpeed;
};

EnemyStats statsFor(EnemyType type) {
    switch (type) {
        case EnemyType::Scout: return {2, 1, 1.85F};
        case EnemyType::Brute: return {5, 2, 0.82F};
        case EnemyType::Raider:
        default: return {3, 1, 1.35F};
    }
}

float durabilityMultiplierFor(const LevelData& level) {
    if (level.id == "ash_gate") return 1.15F;
    if (level.id == "ruined_village") return 1.30F;
    if (level.id == "stone_bridge") return 1.50F;
    if (level.id == "echo_valley") return 1.70F;
    if (level.id == "flooded_road") return 1.90F;
    if (level.id == "iron_ravine") return 2.10F;
    if (level.id == "storm_ring") return 2.30F;
    if (level.id == "last_citadel") return 2.50F;
    return 1.0F;
}

float worldX(const LevelData& level, std::int16_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::int16_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

}  // namespace

Enemy::Enemy(const LevelData& level, EnemyType type) : level_(&level), type_(type) {
    reset();
}

void Enemy::update(float deltaSeconds) {
    if (dead() || reachedBase_ || level_ == nullptr || level_->pathLength < 2) {
        return;
    }

    const float stepSeconds = std::max(deltaSeconds, 0.0F);
    float remainingDistance = stepSeconds * effectiveMovementSpeed();
    slowRemainingSeconds_ = std::max(slowRemainingSeconds_ - stepSeconds, 0.0F);
    if (slowRemainingSeconds_ <= 0.0F) {
        slowMovementMultiplier_ = 1.0F;
    }

    while (remainingDistance > 0.0F && !reachedBase_) {
        const float available = 1.0F - segmentProgress_;
        const float step = std::min(remainingDistance, available);
        segmentProgress_ += step;
        remainingDistance -= step;

        const GridPoint from = level_->path[segmentIndex_];
        const GridPoint to = level_->path[segmentIndex_ + 1];
        const float fromX = worldX(*level_, from.x);
        const float fromZ = worldZ(*level_, from.z);
        const float toX = worldX(*level_, to.x);
        const float toZ = worldZ(*level_, to.z);
        x_ = fromX + (toX - fromX) * segmentProgress_;
        z_ = fromZ + (toZ - fromZ) * segmentProgress_;

        if (segmentProgress_ >= 1.0F) {
            ++segmentIndex_;
            segmentProgress_ = 0.0F;
            if (segmentIndex_ + 1 >= level_->pathLength) {
                reachedBase_ = true;
                x_ = toX;
                z_ = toZ;
            }
        }
    }
}

void Enemy::reset() {
    segmentIndex_ = 0;
    segmentProgress_ = 0.0F;
    slowRemainingSeconds_ = 0.0F;
    slowMovementMultiplier_ = 1.0F;
    health_ = maxHealth();
    reachedBase_ = level_ == nullptr || level_->pathLength < 2;
    if (!reachedBase_) {
        const GridPoint start = level_->path[0];
        x_ = worldX(*level_, start.x);
        z_ = worldZ(*level_, start.z);
    }
}

void Enemy::takeDamage(int amount) {
    if (amount <= 0 || dead() || reachedBase_) {
        return;
    }
    health_ = std::max(health_ - amount, 0);
}

void Enemy::applySlow(float durationSeconds, float movementMultiplier) {
    if (durationSeconds <= 0.0F || movementMultiplier <= 0.0F || movementMultiplier >= 1.0F || dead() || reachedBase_) {
        return;
    }
    slowRemainingSeconds_ = std::max(slowRemainingSeconds_, durationSeconds);
    slowMovementMultiplier_ = std::min(slowMovementMultiplier_, movementMultiplier);
}

float Enemy::x() const { return x_; }
float Enemy::z() const { return z_; }
bool Enemy::reachedBase() const { return reachedBase_; }
bool Enemy::dead() const { return health_ <= 0; }
int Enemy::health() const { return health_; }
int Enemy::maxHealth() const {
    const EnemyStats stats = statsFor(type_);
    if (level_ == nullptr) return stats.health;
    return std::max(1, static_cast<int>(std::ceil(
        static_cast<float>(stats.health) * durabilityMultiplierFor(*level_))));
}
int Enemy::baseDamage() const { return statsFor(type_).baseDamage; }
float Enemy::movementSpeed() const { return statsFor(type_).movementSpeed; }
float Enemy::effectiveMovementSpeed() const { return movementSpeed() * slowMovementMultiplier_; }
bool Enemy::slowed() const { return slowRemainingSeconds_ > 0.0F; }
EnemyType Enemy::type() const { return type_; }

float Enemy::pathProgress() const {
    if (level_ == nullptr || level_->pathLength < 2) {
        return 1.0F;
    }
    const float completed = static_cast<float>(segmentIndex_) + segmentProgress_;
    return std::min(completed / static_cast<float>(level_->pathLength - 1), 1.0F);
}
