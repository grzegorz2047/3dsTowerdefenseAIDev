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

    [[nodiscard]] std::size_t spawnedCount() const;
    [[nodiscard]] const Enemy& enemyAt(std::size_t index) const;
    [[nodiscard]] int baseHealth() const;
    [[nodiscard]] bool completed() const;
    [[nodiscard]] bool lost() const;

private:
    static constexpr std::size_t kEnemyCount = 5;
    static constexpr int kInitialBaseHealth = 5;

    const LevelData* level_ = nullptr;
    std::vector<Enemy> enemies_;
    std::array<bool, kEnemyCount> resolved_{};
    std::size_t spawnedCount_ = 0;
    float spawnTimer_ = 0.0F;
    int baseHealth_ = kInitialBaseHealth;
};
