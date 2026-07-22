#include "Projectile.hpp"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kProjectileSpeed = 6.0F;
constexpr float kImpactRadius = 0.18F;
constexpr float kImpactRadiusSquared = kImpactRadius * kImpactRadius;
constexpr float kTargetHeight = 0.48F;

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
}

void Projectile::update(float deltaSeconds, Wave& wave) {
    if (!active_ || deltaSeconds <= 0.0F || targetIndex_ >= wave.spawnedCount()) {
        if (targetIndex_ >= wave.spawnedCount()) {
            active_ = false;
        }
        return;
    }

    Enemy& target = wave.enemyAt(targetIndex_);
    if (target.dead() || target.reachedBase()) {
        active_ = false;
        return;
    }

    const float dx = target.x() - x_;
    const float dy = kTargetHeight - y_;
    const float dz = target.z() - z_;
    const float distanceSquared = dx * dx + dy * dy + dz * dz;
    if (distanceSquared <= kImpactRadiusSquared) {
        resolveImpact(wave, target);
        active_ = false;
        return;
    }

    const float distance = std::sqrt(distanceSquared);
    const float travel = std::min(kProjectileSpeed * deltaSeconds, distance);
    const float scale = distance > 0.0F ? travel / distance : 0.0F;
    x_ += dx * scale;
    y_ += dy * scale;
    z_ += dz * scale;

    if (travel >= distance) {
        resolveImpact(wave, target);
        active_ = false;
    }
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
    targetIndex_ = 0;
    payload_ = {};
    active_ = false;
}

bool Projectile::active() const { return active_; }
float Projectile::x() const { return x_; }
float Projectile::y() const { return y_; }
float Projectile::z() const { return z_; }
std::size_t Projectile::targetIndex() const { return targetIndex_; }
ProjectileEffect Projectile::effect() const { return payload_.effect; }

bool ProjectilePool::launch(
    float startX,
    float startY,
    float startZ,
    std::size_t targetIndex,
    const ProjectilePayload& payload) {
    for (Projectile& projectile : projectiles_) {
        if (!projectile.active()) {
            projectile.launch(startX, startY, startZ, targetIndex, payload);
            return true;
        }
    }
    return false;
}

void ProjectilePool::update(float deltaSeconds, Wave& wave) {
    for (Projectile& projectile : projectiles_) {
        projectile.update(deltaSeconds, wave);
    }
}

void ProjectilePool::reset() {
    for (Projectile& projectile : projectiles_) {
        projectile.reset();
    }
}

std::size_t ProjectilePool::activeCount() const {
    std::size_t count = 0;
    for (const Projectile& projectile : projectiles_) {
        if (projectile.active()) {
            ++count;
        }
    }
    return count;
}

const Projectile& ProjectilePool::projectileAt(std::size_t index) const {
    return projectiles_.at(index);
}