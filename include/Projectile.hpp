#pragma once

#include <array>
#include <cstddef>

#include "Wave.hpp"

class Projectile {
public:
    void launch(float startX, float startY, float startZ, std::size_t targetIndex, int damage);
    void update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] bool active() const;
    [[nodiscard]] float x() const;
    [[nodiscard]] float y() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] std::size_t targetIndex() const;

private:
    float x_ = 0.0F;
    float y_ = 0.0F;
    float z_ = 0.0F;
    std::size_t targetIndex_ = 0;
    int damage_ = 0;
    bool active_ = false;
};

class ProjectilePool {
public:
    static constexpr std::size_t kCapacity = 32;

    [[nodiscard]] bool launch(float startX, float startY, float startZ, std::size_t targetIndex, int damage);
    void update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] std::size_t activeCount() const;
    [[nodiscard]] const Projectile& projectileAt(std::size_t index) const;

private:
    std::array<Projectile, kCapacity> projectiles_{};
};
