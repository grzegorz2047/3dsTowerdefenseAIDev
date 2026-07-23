#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Wave.hpp"

enum class ProjectileEffect : std::uint8_t {
    Direct,
    Splash,
    Frost,
    GuidedRocket,
};

enum class ProjectileUpdateResult : std::uint8_t {
    None,
    Impact,
    Cancelled,
};

struct ProjectilePayload {
    ProjectileEffect effect = ProjectileEffect::Direct;
    int damage = 1;
    float radius = 0.0F;
    float slowDurationSeconds = 0.0F;
    float slowMovementMultiplier = 1.0F;
    float speed = 6.0F;
    float turnRateRadiansPerSecond = 0.0F;
    float launchClimb = 0.0F;
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
    [[nodiscard]] ProjectileUpdateResult update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] bool active() const;
    [[nodiscard]] float x() const;
    [[nodiscard]] float y() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] float velocityX() const;
    [[nodiscard]] float velocityY() const;
    [[nodiscard]] float velocityZ() const;
    [[nodiscard]] std::size_t targetIndex() const;
    [[nodiscard]] ProjectileEffect effect() const;

private:
    void resolveImpact(Wave& wave, Enemy& target);
    void updateGuidedVelocity(float deltaSeconds, float targetX, float targetY, float targetZ);

    float x_ = 0.0F;
    float y_ = 0.0F;
    float z_ = 0.0F;
    float velocityX_ = 0.0F;
    float velocityY_ = 0.0F;
    float velocityZ_ = 0.0F;
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
    void setActiveLimit(std::size_t limit);

    [[nodiscard]] std::size_t activeCount() const;
    [[nodiscard]] std::size_t activeLimit() const;
    [[nodiscard]] std::uint32_t shotEventCount() const;
    [[nodiscard]] std::uint32_t impactEventCount() const;
    [[nodiscard]] const Projectile& projectileAt(std::size_t index) const;

private:
    std::array<Projectile, kCapacity> projectiles_{};
    std::size_t activeLimit_ = kCapacity;
    std::uint32_t shotEventCount_ = 0U;
    std::uint32_t impactEventCount_ = 0U;
};
