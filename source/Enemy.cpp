#include "Enemy.hpp"

#include <algorithm>

namespace {

constexpr float kMovementSpeed = 1.35F;

float worldX(const LevelData& level, std::int16_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::int16_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

}  // namespace

Enemy::Enemy(const LevelData& level) : level_(&level) {
    reset();
}

void Enemy::update(float deltaSeconds) {
    if (reachedBase_ || level_ == nullptr || level_->pathLength < 2) {
        return;
    }

    float remainingDistance = std::max(deltaSeconds, 0.0F) * kMovementSpeed;
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
    reachedBase_ = level_ == nullptr || level_->pathLength < 2;
    if (!reachedBase_) {
        const GridPoint start = level_->path[0];
        x_ = worldX(*level_, start.x);
        z_ = worldZ(*level_, start.z);
    }
}

float Enemy::x() const {
    return x_;
}

float Enemy::z() const {
    return z_;
}

bool Enemy::reachedBase() const {
    return reachedBase_;
}

float Enemy::pathProgress() const {
    if (level_ == nullptr || level_->pathLength < 2) {
        return 1.0F;
    }
    const float completed = static_cast<float>(segmentIndex_) + segmentProgress_;
    return std::min(completed / static_cast<float>(level_->pathLength - 1), 1.0F);
}
