#pragma once

#include <cstddef>

#include "Level.hpp"

class Enemy {
public:
    explicit Enemy(const LevelData& level);

    void update(float deltaSeconds);
    void reset();
    void takeDamage(int amount);

    [[nodiscard]] float x() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] bool reachedBase() const;
    [[nodiscard]] bool dead() const;
    [[nodiscard]] int health() const;
    [[nodiscard]] float pathProgress() const;

private:
    static constexpr int kMaximumHealth = 3;

    const LevelData* level_ = nullptr;
    std::size_t segmentIndex_ = 0;
    float segmentProgress_ = 0.0F;
    float x_ = 0.0F;
    float z_ = 0.0F;
    int health_ = kMaximumHealth;
    bool reachedBase_ = false;
};
