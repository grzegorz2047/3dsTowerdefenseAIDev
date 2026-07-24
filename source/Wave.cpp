#include "Wave.hpp"

#include <algorithm>

namespace {

constexpr float kSpawnTimeEpsilon = 0.00001F;

}  // namespace

Wave::Wave(const LevelData& level) : level_(&level) {
    enemies_.reserve(kMaximumWaveEnemies);
    reset();
    if (level.id == "performance_stress") setBenchmarkLoop(true);
}

void Wave::prepareWave(std::size_t waveIndex) {
    enemies_.clear();
    resolved_.fill(false);
    spawnIntervals_.fill(0.0F);
    spawnedCount_ = 0U;
    spawnTimer_ = 0.0F;
    if (level_ == nullptr || waveIndex >= level_->waveEntryCount) return;

    const WaveEntry& entry = level_->waveEntries[waveIndex];
    const std::size_t count = std::min<std::size_t>(entry.count, kMaximumWaveEnemies);
    for (std::size_t index = 0U; index < count; ++index) {
        enemies_.emplace_back(*level_, entry.type);
        spawnIntervals_[index] = entry.spawnIntervalSeconds;
    }
}

bool Wave::startNextWave() {
    if (waveRunning_ || lost() || completed() || level_ == nullptr ||
        currentWaveIndex_ >= level_->waveEntryCount) {
        return false;
    }
    prepareWave(currentWaveIndex_);
    if (enemies_.empty()) return false;
    waveRunning_ = true;
    return true;
}

void Wave::update(float deltaSeconds) {
    if (!waveRunning_ || (!benchmarkLoop_ && lost())) return;

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
            ++missionDefeatedCount_;
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

    if (!benchmarkLoop_ && !lost() && currentWaveResolved()) {
        waveRunning_ = false;
        ++completedWaveCount_;
        ++waveCompletedEventCount_;
        currentWaveIndex_ = completedWaveCount_;
    }
}

bool Wave::currentWaveResolved() const {
    if (enemies_.empty() || spawnedCount_ < enemies_.size()) return false;
    for (std::size_t index = 0U; index < enemies_.size(); ++index) {
        if (!resolved_[index]) return false;
    }
    return true;
}

void Wave::reset() {
    enemies_.clear();
    resolved_.fill(false);
    spawnIntervals_.fill(0.0F);
    spawnedCount_ = 0U;
    currentWaveIndex_ = 0U;
    completedWaveCount_ = 0U;
    missionDefeatedCount_ = 0U;
    spawnTimer_ = 0.0F;
    baseHealth_ = kInitialBaseHealth;
    waveCompletedEventCount_ = 0U;
    deathEventCount_ = 0U;
    baseDamageEventCount_ = 0U;
    waveRunning_ = false;
    if (benchmarkLoop_) (void)startNextWave();
}

void Wave::setBenchmarkLoop(bool enabled) {
    benchmarkLoop_ = enabled;
    if (enabled) {
        baseHealth_ = kInitialBaseHealth;
        if (!waveRunning_) (void)startNextWave();
    }
}

void Wave::applyAreaEffect(
    float centerX,
    float centerZ,
    float radius,
    int damage,
    float slowDurationSeconds,
    float slowMovementMultiplier,
    DamageType damageType) {
    const float radiusSquared = std::max(radius, 0.0F) * std::max(radius, 0.0F);
    for (std::size_t index = 0; index < spawnedCount_; ++index) {
        Enemy& enemy = enemies_[index];
        if (resolved_[index] || enemy.dead() || enemy.reachedBase()) continue;
        const float dx = enemy.x() - centerX;
        const float dz = enemy.z() - centerZ;
        if (dx * dx + dz * dz > radiusSquared) continue;
        enemy.takeDamage(damage, damageType);
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
std::size_t Wave::missionEnemyCount() const { return level_ == nullptr ? 0U : level_->totalEnemyCount; }
std::size_t Wave::defeatedCount() const { return missionDefeatedCount_; }
std::size_t Wave::waveNumber() const {
    if (waveCount() == 0U) return 0U;
    if (completed()) return waveCount();
    return std::min(currentWaveIndex_ + 1U, waveCount());
}
std::size_t Wave::waveCount() const { return level_ == nullptr ? 0U : level_->waveEntryCount; }
std::size_t Wave::completedWaveCount() const { return completedWaveCount_; }
std::uint32_t Wave::waveCompletedEventCount() const { return waveCompletedEventCount_; }
std::uint32_t Wave::deathEventCount() const { return deathEventCount_; }
std::uint32_t Wave::baseDamageEventCount() const { return baseDamageEventCount_; }
Enemy& Wave::enemyAt(std::size_t index) { return enemies_.at(index); }
const Enemy& Wave::enemyAt(std::size_t index) const { return enemies_.at(index); }
int Wave::baseHealth() const { return baseHealth_; }
bool Wave::waveRunning() const { return waveRunning_; }
bool Wave::awaitingNextWave() const { return !waveRunning_ && !completed() && !lost(); }

bool Wave::completed() const {
    return !benchmarkLoop_ && level_ != nullptr && level_->waveEntryCount > 0U &&
        completedWaveCount_ >= level_->waveEntryCount && baseHealth_ > 0;
}

bool Wave::lost() const { return !benchmarkLoop_ && baseHealth_ <= 0; }
