#pragma once

#include <algorithm>

namespace WaveSpeed {

constexpr int kNormal = 1;
constexpr int kFast = 2;

[[nodiscard]] constexpr int initial() { return kNormal; }

[[nodiscard]] constexpr int normalize(int requested) {
    return requested == kFast ? kFast : kNormal;
}

[[nodiscard]] constexpr int toggled(int current) {
    return normalize(current) == kNormal ? kFast : kNormal;
}

[[nodiscard]] constexpr float simulationDelta(float realSeconds, int multiplier) {
    return std::max(realSeconds, 0.0F) * static_cast<float>(normalize(multiplier));
}

}  // namespace WaveSpeed
