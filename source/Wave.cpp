#include "Wave.hpp"

#include <algorithm>

Wave::Wave(const LevelData& level) : level_(&level) {
    enemies_.reserve(level.totalEnemyCount);
    std::size_t enemyIndex = 0;
    for (std::size_t entryIndex = 0; entryIndex < level.waveEntryCount; ++entryIndex) {
        const WaveEntry& entry = level.waveEntries[entryIndex];
        for (std::size_t count = 0; count < entry.count && enemyIndex < kMaximumWaveEnemies; ++count) {
            enemies_.emplace_back(level, entry.type);
            spawnIntervals_[enemyIndex] = entry.spawnIntervalSeconds;
            ++enemyIndex;
        }
    }
    reset();
}

void Wave::update(float deltaSeconds) {
    if (completed() || lost()) {
        return;
    }

    const float step = std::max(deltaSeconds, 0.0F);
    if (spawnedCount_ < enemies_.size()) {
        spawnTimer_ += step;
        while (spawnedCount_ < enemies_.size() &&
               spawnTimer_ >= spawnIntervals_[spawnedCount_]) {
            spawnTimer_ -= spawnIntervals_[spawnedCount_];
            ++spawnedCount_;
        }
    }

    for (std::size_t index = 0; index < spawnedCount_; ++index) {
        if (resolved_[index]) {
            continue;
        }

        Enemy& enemy = enemies_[index];
        if (enemy.dead()) {
            resolved_[index] = true;
            continue;
        }

        enemy.update(step);
        if (enemy.reachedBase()) {
            resolved_[index] = true;
            baseHealth_ = std::max(baseHealth_ - enemy.baseDamage(), 0);
        }
    }
}

void Wave::reset() {
    for (Enemy& enemy : enemies_) {
        enemy.reset();
    }
    resolved_.fill(false);
    spawnedCount_ = 0U;
    spawnTimer_ = 0.0F;
    baseHealth_ = kInitialBaseHealth;
}

void Wave::applyAreaEffect(
    float centerX,
    float centerZ,
    float radius,
    int damage,
    float slowDurationSeconds,
    float slowMovementMultiplier) {
    const float radiusSquared = std::max(radius, 0.0F) * std::max(radius, 0.0F);
    for (std::size_t index = 0; index < spawnedCount_; ++index) {
        Enemy& enemy = enemies_[index];
        if (resolved_[index] || enemy.dead() || enemy.reachedBase()) {
            continue;
        }
        const float dx = enemy.x() - centerX;
        const float dz = enemy.z() - centerZ;
        if (dx * dx + dz * dz > radiusSquared) {
            continue;
        }
        enemy.takeDamage(damage);
        enemy.applySlow(slowDurationSeconds, slowMovementMultiplier);
    }
}

std::size_t Wave::spawnedCount() const { return spawnedCount_; }
std::size_t Wave::enemyCount() const { return enemies_.size(); }
Enemy& Wave::enemyAt(std::size_t index) { return enemies_.at(index); }
const Enemy& Wave::enemyAt(std::size_t index) const { return enemies_.at(index); }
int Wave::baseHealth() const { return baseHealth_; }

bool Wave::completed() const {
    if (enemies_.empty() || spawnedCount_ < enemies_.size()) {
        return false;
    }
    for (std::size_t index = 0; index < enemies_.size(); ++index) {
        if (!resolved_[index]) {
            return false;
        }
    }
    return baseHealth_ > 0;
}

bool Wave::lost() const {
    return baseHealth_ <= 0;
}
