#include "Projectile.hpp"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kDefaultProjectileSpeed = 6.0F;
constexpr float kImpactRadius = 0.18F;
constexpr float kImpactRadiusSquared = kImpactRadius * kImpactRadius;
constexpr float kTargetHeight = 0.48F;

float length3(float x, float y, float z) {
    return std::sqrt(x * x + y * y + z * z);
}

}  // namespace

void Projectile::launch(
    float startX,
    float startY,
    float startZ,
    std::size_t targetIndex,
    const ProjectilePayload& payload) {
    x_ = startX;
    y_ = startY;
    z_ = startZ;
    targetIndex_ = targetIndex;
    payload_ = payload;
    active_ = payload.damage > 0 || payload.slowDurationSeconds > 0.0F;
    velocityX_ = 0.0F;
    velocityY_ = 0.0F;
    velocityZ_ = 0.0F;

    if (payload_.effect == ProjectileEffect::GuidedRocket) {
        const float speed = payload_.speed > 0.0F ? payload_.speed : kDefaultProjectileSpeed;
        constexpr float kLaunchYaw = -0.62F;
        velocityX_ = std::sin(kLaunchYaw) * speed * 0.72F;
        velocityY_ = std::max(payload_.launchClimb, 0.0F) * speed;
        velocityZ_ = std::cos(kLaunchYaw) * speed * 0.72F;
        const float current = length3(velocityX_, velocityY_, velocityZ_);
        if (current > 0.0F) {
            const float scale = speed / current;
            velocityX_ *= scale;
            velocityY_ *= scale;
            velocityZ_ *= scale;
        }
    }
}

void Projectile::launch(float startX, float startY, float startZ, std::size_t targetIndex, int damage) {
    launch(startX, startY, startZ, targetIndex,
        {ProjectileEffect::Direct, damage, 0.0F, 0.0F, 1.0F});
}

void Projectile::updateGuidedVelocity(
    float deltaSeconds, float targetX, float targetY, float targetZ) {
    const float desiredX = targetX - x_;
    const float desiredY = targetY - y_;
    const float desiredZ = targetZ - z_;
    const float desiredLength = length3(desiredX, desiredY, desiredZ);
    const float currentLength = length3(velocityX_, velocityY_, velocityZ_);
    if (desiredLength <= 0.0001F || currentLength <= 0.0001F) return;

    const float currentX = velocityX_ / currentLength;
    const float currentY = velocityY_ / currentLength;
    const float currentZ = velocityZ_ / currentLength;
    const float goalX = desiredX / desiredLength;
    const float goalY = desiredY / desiredLength;
    const float goalZ = desiredZ / desiredLength;
    const float dot = std::clamp(currentX * goalX + currentY * goalY + currentZ * goalZ, -1.0F, 1.0F);
    const float angle = std::acos(dot);
    const float maximumTurn = std::max(payload_.turnRateRadiansPerSecond, 0.0F) * deltaSeconds;
    const float blend = angle <= 0.0001F ? 1.0F : std::min(maximumTurn / angle, 1.0F);

    float nextX = currentX + (goalX - currentX) * blend;
    float nextY = currentY + (goalY - currentY) * blend;
    float nextZ = currentZ + (goalZ - currentZ) * blend;
    const float nextLength = length3(nextX, nextY, nextZ);
    if (nextLength <= 0.0001F) return;

    const float speed = payload_.speed > 0.0F ? payload_.speed : kDefaultProjectileSpeed;
    velocityX_ = nextX / nextLength * speed;
    velocityY_ = nextY / nextLength * speed;
    velocityZ_ = nextZ / nextLength * speed;
}

ProjectileUpdateResult Projectile::update(float deltaSeconds, Wave& wave) {
    if (!active_) return ProjectileUpdateResult::None;
    if (deltaSeconds <= 0.0F) return ProjectileUpdateResult::None;
    if (targetIndex_ >= wave.spawnedCount()) {
        active_ = false;
        return ProjectileUpdateResult::Cancelled;
    }

    Enemy& target = wave.enemyAt(targetIndex_);
    if (target.dead() || target.reachedBase()) {
        active_ = false;
        return ProjectileUpdateResult::Cancelled;
    }

    const float dx = target.x() - x_;
    const float dy = kTargetHeight - y_;
    const float dz = target.z() - z_;
    const float distanceSquared = dx * dx + dy * dy + dz * dz;
    const float impactRadiusSquared = payload_.effect == ProjectileEffect::GuidedRocket
        ? 0.34F * 0.34F : kImpactRadiusSquared;
    if (distanceSquared <= impactRadiusSquared) {
        resolveImpact(wave, target);
        active_ = false;
        return ProjectileUpdateResult::Impact;
    }

    if (payload_.effect == ProjectileEffect::GuidedRocket) {
        updateGuidedVelocity(deltaSeconds, target.x(), kTargetHeight, target.z());
        x_ += velocityX_ * deltaSeconds;
        y_ += velocityY_ * deltaSeconds;
        z_ += velocityZ_ * deltaSeconds;
        if (y_ < -1.0F || y_ > 8.0F || std::abs(x_) > 20.0F || std::abs(z_) > 20.0F) {
            active_ = false;
            return ProjectileUpdateResult::Cancelled;
        }
        return ProjectileUpdateResult::None;
    }

    const float distance = std::sqrt(distanceSquared);
    const float speed = payload_.speed > 0.0F ? payload_.speed : kDefaultProjectileSpeed;
    const float travel = std::min(speed * deltaSeconds, distance);
    const float scale = distance > 0.0F ? travel / distance : 0.0F;
    x_ += dx * scale;
    y_ += dy * scale;
    z_ += dz * scale;

    if (travel >= distance) {
        resolveImpact(wave, target);
        active_ = false;
        return ProjectileUpdateResult::Impact;
    }
    return ProjectileUpdateResult::None;
}

void Projectile::resolveImpact(Wave& wave, Enemy& target) {
    if (payload_.effect == ProjectileEffect::Direct) {
        target.takeDamage(payload_.damage);
        return;
    }

    wave.applyAreaEffect(
        target.x(),
        target.z(),
        payload_.radius,
        payload_.damage,
        payload_.slowDurationSeconds,
        payload_.slowMovementMultiplier);
}

void Projectile::reset() {
    x_ = 0.0F;
    y_ = 0.0F;
    z_ = 0.0F;
    velocityX_ = 0.0F;
    velocityY_ = 0.0F;
    velocityZ_ = 0.0F;
    targetIndex_ = 0;
    payload_ = {};
    active_ = false;
}

bool Projectile::active() const { return active_; }
float Projectile::x() const { return x_; }
float Projectile::y() const { return y_; }
float Projectile::z() const { return z_; }
float Projectile::velocityX() const { return velocityX_; }
float Projectile::velocityY() const { return velocityY_; }
float Projectile::velocityZ() const { return velocityZ_; }
std::size_t Projectile::targetIndex() const { return targetIndex_; }
ProjectileEffect Projectile::effect() const { return payload_.effect; }

bool ProjectilePool::launch(
    float startX,
    float startY,
    float startZ,
    std::size_t targetIndex,
    const ProjectilePayload& payload) {
    for (std::size_t index = 0U; index < activeLimit_; ++index) {
        Projectile& projectile = projectiles_[index];
        if (!projectile.active()) {
            projectile.launch(startX, startY, startZ, targetIndex, payload);
            ++shotEventCount_;
            return true;
        }
    }
    return false;
}

bool ProjectilePool::launch(float startX, float startY, float startZ, std::size_t targetIndex, int damage) {
    return launch(startX, startY, startZ, targetIndex,
        {ProjectileEffect::Direct, damage, 0.0F, 0.0F, 1.0F});
}

void ProjectilePool::update(float deltaSeconds, Wave& wave) {
    for (Projectile& projectile : projectiles_) {
        if (projectile.update(deltaSeconds, wave) == ProjectileUpdateResult::Impact) {
            ++impactEventCount_;
        }
    }
}

void ProjectilePool::setActiveLimit(std::size_t limit) {
    activeLimit_ = std::max<std::size_t>(1U, std::min(limit, kCapacity));
    for (std::size_t index = activeLimit_; index < kCapacity; ++index) projectiles_[index].reset();
}

void ProjectilePool::reset() {
    for (Projectile& projectile : projectiles_) projectile.reset();
    shotEventCount_ = 0U;
    impactEventCount_ = 0U;
}

std::size_t ProjectilePool::activeCount() const {
    std::size_t count = 0;
    for (const Projectile& projectile : projectiles_) {
        if (projectile.active()) ++count;
    }
    return count;
}

std::size_t ProjectilePool::activeLimit() const { return activeLimit_; }

std::uint32_t ProjectilePool::shotEventCount() const { return shotEventCount_; }
std::uint32_t ProjectilePool::impactEventCount() const { return impactEventCount_; }

const Projectile& ProjectilePool::projectileAt(std::size_t index) const {
    return projectiles_.at(index);
}
