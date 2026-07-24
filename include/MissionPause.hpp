#pragma once

namespace MissionPause {
constexpr bool toggled(bool paused, bool sessionFinished, bool benchmarkMode) {
    return (!sessionFinished && !benchmarkMode) ? !paused : paused;
}
constexpr bool gameplayInputAllowed(bool paused) { return !paused; }
constexpr float simulationSeconds(float frameSeconds, int speedMultiplier, bool paused) {
    if (paused || frameSeconds <= 0.0F || speedMultiplier <= 0) return 0.0F;
    return frameSeconds * static_cast<float>(speedMultiplier);
}
}  // namespace MissionPause
