#include "Wave.hpp"

#include <algorithm>

namespace {

constexpr float kSpawnIntervalSeconds = 1.15F;

}  // namespace

Wave::Wave(const LevelData& level) : level_(&level) {
    enemies_.reserve(kEnemyCount);
    for (std::size_t index = 0; index < kEnemyCount; ++index) {
        enemies_.emplace_back(level);
    }
    reset();
}

void Wave::update(float deltaSeconds) {
    if (completed() || lost()) {
        return;
    }

    const float step = std::max(deltaSeconds, 0.0F);
    if (spawnedCount_ < kEnemyCount) {
        spawnTimer_ += step;
        while (spawnedCount_ < kEnemyCount && spawnTimer_ >= kSpawnIntervalSeconds) {
            spawnTimer_ -= kSpawnIntervalSeconds;
            ++spawnedCount_;
        }
    }

    for (std::size_t index = 0; index < spawnedCount_; ++index) {
        if (resolved_[index]) {
            continue;
        }

        Enemy& enemy = enemies_[index];
        enemy.update(step);
        if (enemy.reachedBase()) {
            resolved_[index] = true;
            baseHealth_ = std::max(baseHealth_ - 1, 0);
        }
    }
}

void Wave::reset() {
    for (Enemy& enemy : enemies_) {
        enemy.reset();
    }
    resolved_.fill(false);
    spawnedCount_ = 1;
    spawnTimer_ = 0.0F;
    baseHealth_ = kInitialBaseHealth;
}

std::size_t Wave::spawnedCount() const {
    return spawnedCount_;
}

const Enemy& Wave::enemyAt(std::size_t index) const {
    return enemies_.at(index);
}

int Wave::baseHealth() const {
    return baseHealth_;
}

bool Wave::completed() const {
    if (spawnedCount_ < kEnemyCount) {
        return false;
    }
    for (bool resolved : resolved_) {
        if (!resolved) {
            return false;
        }
    }
    return baseHealth_ > 0;
}

bool Wave::lost() const {
    return baseHealth_ <= 0;
}
