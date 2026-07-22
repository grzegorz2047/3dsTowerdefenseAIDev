#include "BuildFeedback.hpp"

BuildAttemptResult evaluateBuildAttempt(
    bool hasBuildSpot,
    bool occupied,
    bool enoughGold,
    bool hasTowerCapacity,
    bool towerValid) {
    if (!hasBuildSpot) {
        return BuildAttemptResult::NoBuildSpot;
    }
    if (occupied) {
        return BuildAttemptResult::Occupied;
    }
    if (!enoughGold) {
        return BuildAttemptResult::InsufficientGold;
    }
    if (!hasTowerCapacity) {
        return BuildAttemptResult::TowerLimitReached;
    }
    if (!towerValid) {
        return BuildAttemptResult::InvalidTower;
    }
    return BuildAttemptResult::Built;
}
