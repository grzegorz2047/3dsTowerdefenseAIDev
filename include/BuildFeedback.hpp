#pragma once

enum class BuildAttemptResult {
    None,
    Built,
    NoBuildSpot,
    Occupied,
    InsufficientGold,
    TowerLimitReached,
    InvalidTower,
};

[[nodiscard]] BuildAttemptResult evaluateBuildAttempt(
    bool hasBuildSpot,
    bool occupied,
    bool enoughGold,
    bool hasTowerCapacity,
    bool towerValid = true);
