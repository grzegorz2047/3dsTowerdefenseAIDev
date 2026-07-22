#pragma once

#include <cstddef>

#include "Level.hpp"
#include "Projectile.hpp"
#include "Wave.hpp"

class Tower {
public:
    Tower() = default;
    Tower(const LevelData& level, std::size_t gridX, std::size_t gridZ);

    void update(float deltaSeconds, Wave& wave, ProjectilePool& projectiles);
    void resetCombat();

    [[nodiscard]] float x() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] std::size_t gridX() const;
    [[nodiscard]] std::size_t gridZ() const;
    [[nodiscard]] bool valid() const;
    [[nodiscard]] int shotsFired() const;

private:
    float x_ = 0.0F;
    float z_ = 0.0F;
    float cooldown_ = 0.0F;
    std::size_t gridX_ = 0;
    std::size_t gridZ_ = 0;
    int shotsFired_ = 0;
    bool valid_ = false;
};
