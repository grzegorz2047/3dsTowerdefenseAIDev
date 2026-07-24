#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "Enemy.hpp"
#include "Level.hpp"

class Wave {
public:
    explicit Wave(const LevelData& level);

    [[nodiscard]] bool startNextWave();
    void update(float deltaSeconds);
    void reset();
    void setBenchmarkLoop(bool enabled);
    void applyAreaEffect(
        float centerX,
        float centerZ,
        float radius,
        int damage,
        float slowDurationSeconds = 0.0F,
        float slowMovementMultiplier = 1.0F);

    [[nodiscard]] std::size_t spawnedCount() const;
    [[nodiscard]] std::size_t activeCount() const;
    [[nodiscard]] std::size_t enemyCount() const;
    [[nodiscard]] std::size_t missionEnemyCount() const;
    [[nodiscard]] std::size_t defeatedCount() const;
    [[nodiscard]] std::size_t waveNumber() const;
    [[nodiscard]] std::size_t waveCount() const;
    [[nodiscard]] std::size_t completedWaveCount() const;
    [[nodiscard]] std::uint32_t waveCompletedEventCount() const;
    [[nodiscard]] std::uint32_t deathEventCount() const;
    [[nodiscard]] std::uint32_t baseDamageEventCount() const;
    [[nodiscard]] Enemy& enemyAt(std::size_t index);
    [[nodiscard]] const Enemy& enemyAt(std::size_t index) const;
    [[nodiscard]] int baseHealth() const;
    [[nodiscard]] bool waveRunning() const;
    [[nodiscard]] bool awaitingNextWave() const;
    [[nodiscard]] bool completed() const;
    [[nodiscard]] bool lost() const;

private:
    static constexpr int kInitialBaseHealth = 5;

    void prepareWave(std::size_t waveIndex);
    [[nodiscard]] bool currentWaveResolved() const;

    const LevelData* level_ = nullptr;
    std::vector<Enemy> enemies_;
    std::array<float, kMaximumWaveEnemies> spawnIntervals_{};
    std::array<bool, kMaximumWaveEnemies> resolved_{};
    std::size_t spawnedCount_ = 0;
    std::size_t currentWaveIndex_ = 0;
    std::size_t completedWaveCount_ = 0;
    std::size_t missionDefeatedCount_ = 0;
    float spawnTimer_ = 0.0F;
    int baseHealth_ = kInitialBaseHealth;
    std::uint32_t waveCompletedEventCount_ = 0U;
    std::uint32_t deathEventCount_ = 0U;
    std::uint32_t baseDamageEventCount_ = 0U;
    bool waveRunning_ = false;
    bool benchmarkLoop_ = false;
};
