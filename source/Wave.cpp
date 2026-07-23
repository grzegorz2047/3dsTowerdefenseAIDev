#include "Wave.hpp"

#include <algorithm>

namespace {

constexpr float kSpawnTimeEpsilon = 0.00001F;

}  // namespace

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
    if (level.id == "performance_stress") setBenchmarkLoop(true);
}

void Wave::update(float deltaSeconds) {
    if (!benchmarkLoop_ && (completed() || lost())) return;

    const float step = std::max(deltaSeconds, 0.0F);
    if (spawnedCount_ < enemies_.size()) {
        spawnTimer_ += step;
        while (spawnedCount_ < enemies_.size() &&
               spawnTimer_ + kSpawnTimeEpsilon >= spawnIntervals_[spawnedCount_]) {
            spawnTimer_ = std::max(spawnTimer_ - spawnIntervals_[spawnedCount_], 0.0F);
            ++spawnedCount_;
        }
    }

    for (std::size_t index = 0; index < spawnedCount_; ++index) {
        if (resolved_[index]) continue;

        Enemy& enemy = enemies_[index];
        if (enemy.dead()) {
            ++deathEventCount_;
            if (benchmarkLoop_) enemy.reset();
            else resolved_[index] = true;
            continue;
        }

        enemy.update(step);
        if (enemy.reachedBase()) {
            ++baseDamageEventCount_;
            if (benchmarkLoop_) enemy.reset();
            else {
                resolved_[index] = true;
                baseHealth_ = std::max(baseHealth_ - enemy.baseDamage(), 0);
            }
        }
    }
}

void Wave::reset() {
    for (Enemy& enemy : enemies_) enemy.reset();
    resolved_.fill(false);
    spawnedCount_ = 0U;
    spawnTimer_ = 0.0F;
    baseHealth_ = kInitialBaseHealth;
    deathEventCount_ = 0U;
    baseDamageEventCount_ = 0U;
}

void Wave::setBenchmarkLoop(bool enabled) {
    benchmarkLoop_ = enabled;
    if (enabled) baseHealth_ = kInitialBaseHealth;
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
        if (resolved_[index] || enemy.dead() || enemy.reachedBase()) continue;
        const float dx = enemy.x() - centerX;
        const float dz = enemy.z() - centerZ;
        if (dx * dx + dz * dz > radiusSquared) continue;
        enemy.takeDamage(damage);
        enemy.applySlow(slowDurationSeconds, slowMovementMultiplier);
    }
}

std::size_t Wave::spawnedCount() const { return spawnedCount_; }

std::size_t Wave::activeCount() const {
    std::size_t count = 0U;
    for (std::size_t index = 0U; index < spawnedCount_; ++index) {
        if (!resolved_[index] && !enemies_[index].dead() && !enemies_[index].reachedBase()) ++count;
    }
    return count;
}

std::size_t Wave::enemyCount() const { return enemies_.size(); }
std::size_t Wave::defeatedCount() const {
    std::size_t count = 0U;
    for (std::size_t index = 0; index < spawnedCount_; ++index) {
        if (resolved_[index] && enemies_[index].dead()) ++count;
    }
    return count;
}
std::uint32_t Wave::deathEventCount() const { return deathEventCount_; }
std::uint32_t Wave::baseDamageEventCount() const { return baseDamageEventCount_; }
Enemy& Wave::enemyAt(std::size_t index) { return enemies_.at(index); }
const Enemy& Wave::enemyAt(std::size_t index) const { return enemies_.at(index); }
int Wave::baseHealth() const { return baseHealth_; }

bool Wave::completed() const {
    if (benchmarkLoop_) return false;
    if (enemies_.empty() || spawnedCount_ < enemies_.size()) return false;
    for (std::size_t index = 0; index < enemies_.size(); ++index) {
        if (!resolved_[index]) return false;
    }
    return baseHealth_ > 0;
}

bool Wave::lost() const { return !benchmarkLoop_ && baseHealth_ <= 0; }