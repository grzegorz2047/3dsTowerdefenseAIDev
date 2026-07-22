#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Wave.hpp"

enum class ProjectileEffect : std::uint8_t {
    Direct,
    Splash,
    Frost,
};

struct ProjectilePayload {
    ProjectileEffect effect = ProjectileEffect::Direct;
    int damage = 1;
    float radius = 0.0F;
    float slowDurationSeconds = 0.0F;
    float slowMovementMultiplier = 1.0F;
};

class Projectile {
public:
    void launch(
        float startX,
        float startY,
        float startZ,
        std::size_t targetIndex,
        const ProjectilePayload& payload);
    void launch(float startX, float startY, float startZ, std::size_t targetIndex, int damage);
    void update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] bool active() const;
    [[nodiscard]] float x() const;
    [[nodiscard]] float y() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] std::size_t targetIndex() const;
    [[nodiscard]] ProjectileEffect effect() const;

private:
    void resolveImpact(Wave& wave, Enemy& target);

    float x_ = 0.0F;
    float y_ = 0.0F;
    float z_ = 0.0F;
    std::size_t targetIndex_ = 0;
    ProjectilePayload payload_{};
    bool active_ = false;
};

class ProjectilePool {
public:
    static constexpr std::size_t kCapacity = 32;

    [[nodiscard]] bool launch(
        float startX,
        float startY,
        float startZ,
        std::size_t targetIndex,
        const ProjectilePayload& payload);
    [[nodiscard]] bool launch(float startX, float startY, float startZ, std::size_t targetIndex, int damage);
    void update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] std::size_t activeCount() const;
    [[nodiscard]] const Projectile& projectileAt(std::size_t index) const;

private:
    std::array<Projectile, kCapacity> projectiles_{};
};