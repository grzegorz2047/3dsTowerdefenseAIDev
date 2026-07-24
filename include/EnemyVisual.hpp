#pragma once

#include <algorithm>
#include <cstddef>

#include "Level.hpp"

struct EnemyVisualProfile {
    float halfWidth;
    float halfDepth;
    float bodyHeight;
    float headHeight;
    float red;
    float green;
    float blue;
    bool alwaysShowHealthBar;
};

namespace EnemyVisual {

[[nodiscard]] constexpr EnemyVisualProfile profile(EnemyType type) {
    switch (type) {
        case EnemyType::Scout:
            return {0.16F, 0.12F, 0.48F, 0.16F, 0.83F, 0.69F, 0.24F, false};
        case EnemyType::Brute:
            return {0.34F, 0.25F, 0.88F, 0.24F, 0.42F, 0.18F, 0.12F, true};
        case EnemyType::Raider:
        default:
            return {0.24F, 0.17F, 0.66F, 0.20F, 0.62F, 0.20F, 0.24F, true};
    }
}

[[nodiscard]] constexpr std::size_t typeIndex(EnemyType type) {
    switch (type) {
        case EnemyType::Scout: return 0U;
        case EnemyType::Raider: return 1U;
        case EnemyType::Brute: return 2U;
        default: return 1U;
    }
}

[[nodiscard]] inline std::size_t healthBucket(int health, int maximumHealth) {
    if (maximumHealth <= 0 || health <= 0) return 0U;
    const int clamped = std::min(health, maximumHealth);
    return static_cast<std::size_t>((clamped * 10 + maximumHealth - 1) / maximumHealth);
}

[[nodiscard]] constexpr bool shouldShowHealthBar(EnemyType type, int health, int maximumHealth) {
    return maximumHealth > 0 && health > 0 &&
        (profile(type).alwaysShowHealthBar || health < maximumHealth);
}

}  // namespace EnemyVisual
