#pragma once

#include <cstddef>

#include "Level.hpp"

class Enemy {
public:
    Enemy(const LevelData& level, EnemyType type = EnemyType::Raider);

    void update(float deltaSeconds);
    void reset();
    void takeDamage(int amount);
    void applySlow(float durationSeconds, float movementMultiplier);

    [[nodiscard]] float x() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] bool reachedBase() const;
    [[nodiscard]] bool dead() const;
    [[nodiscard]] int health() const;
    [[nodiscard]] int maxHealth() const;
    [[nodiscard]] int baseDamage() const;
    [[nodiscard]] float movementSpeed() const;
    [[nodiscard]] float effectiveMovementSpeed() const;
    [[nodiscard]] bool slowed() const;
    [[nodiscard]] bool hitFlashActive() const;
    [[nodiscard]] float pathProgress() const;
    [[nodiscard]] EnemyType type() const;

private:
    static constexpr float kHitFlashSeconds = 0.12F;

    const LevelData* level_ = nullptr;
    EnemyType type_ = EnemyType::Raider;
    std::size_t segmentIndex_ = 0;
    float segmentProgress_ = 0.0F;
    float x_ = 0.0F;
    float z_ = 0.0F;
    float slowRemainingSeconds_ = 0.0F;
    float slowMovementMultiplier_ = 1.0F;
    float hitFlashRemainingSeconds_ = 0.0F;
    int health_ = 0;
    bool reachedBase_ = false;
};
