#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "Enemy.hpp"
#include "Level.hpp"

class Wave {
public:
    explicit Wave(const LevelData& level);

    void update(float deltaSeconds);
    void reset();
    void applyAreaEffect(
        float centerX,
        float centerZ,
        float radius,
        int damage,
        float slowDurationSeconds = 0.0F,
        float slowMovementMultiplier = 1.0F);

    [[nodiscard]] std::size_t spawnedCount() const;
    [[nodiscard]] std::size_t enemyCount() const;
    [[nodiscard]] Enemy& enemyAt(std::size_t index);
    [[nodiscard]] const Enemy& enemyAt(std::size_t index) const;
    [[nodiscard]] int baseHealth() const;
    [[nodiscard]] bool completed() const;
    [[nodiscard]] bool lost() const;

private:
    static constexpr int kInitialBaseHealth = 5;

    const LevelData* level_ = nullptr;
    std::vector<Enemy> enemies_;
    std::array<float, kMaximumWaveEnemies> spawnIntervals_{};
    std::array<bool, kMaximumWaveEnemies> resolved_{};
    std::size_t spawnedCount_ = 0;
    float spawnTimer_ = 0.0F;
    int baseHealth_ = kInitialBaseHealth;
};