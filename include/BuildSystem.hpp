#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "BuildFeedback.hpp"
#include "Economy.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Projectile.hpp"
#include "Tower.hpp"
#include "Wave.hpp"

enum class TowerActionResult : std::uint8_t {
    None,
    Selected,
    Upgraded,
    Sold,
    NoTower,
    InsufficientGold,
    MaximumLevel,
};

class BuildSystem {
public:
    explicit BuildSystem(const LevelData& level);

    void handleInput(const InputSnapshot& input);
    void update(float deltaSeconds, Wave& wave);
    void reset();
    void prepareBenchmarkLayout(std::size_t requestedTowers = 16U,
        std::uint8_t rocketSharePercent = 25U);
    void setProjectileLimit(std::size_t limit);
    void selectTowerType(TowerType type);
    void buildOrSelectCursor();
    void cancelAction();
    [[nodiscard]] TowerActionResult upgradeCursorTower();
    [[nodiscard]] TowerActionResult sellCursorTower();

    [[nodiscard]] std::size_t towerCount() const;
    [[nodiscard]] const Tower& towerAt(std::size_t index) const;
    [[nodiscard]] const ProjectilePool& projectiles() const;
    [[nodiscard]] int gold() const;
    [[nodiscard]] int towerCost() const;
    [[nodiscard]] TowerType selectedTowerType() const;
    [[nodiscard]] const char* selectedTowerName() const;
    [[nodiscard]] std::size_t cursorX() const;
    [[nodiscard]] std::size_t cursorZ() const;
    [[nodiscard]] std::size_t buildSpotCount() const;
    [[nodiscard]] std::size_t availableBuildSpotCount() const;
    [[nodiscard]] bool cursorOccupied() const;
    [[nodiscard]] const Tower* cursorTower() const;
    [[nodiscard]] bool hasEnoughGold() const;
    [[nodiscard]] bool cursorCanBuild() const;
    [[nodiscard]] BuildAttemptResult lastBuildResult() const;
    [[nodiscard]] TowerActionResult lastTowerAction() const;

private:
    static constexpr std::size_t kMaximumTowers = 16;

    [[nodiscard]] std::size_t towerIndexAt(std::size_t x, std::size_t z) const;
    [[nodiscard]] bool occupied(std::size_t x, std::size_t z) const;
    void moveCursor(int delta);
    void moveCursorToAvailable(int delta);
    void tryBuild();

    const LevelData* level_ = nullptr;
    std::array<GridPoint, kMaximumTowers> buildSpots_{};
    std::array<Tower, kMaximumTowers> towers_{};
    std::size_t buildSpotCount_ = 0;
    std::size_t towerCount_ = 0;
    std::size_t cursorIndex_ = 0;
    TowerType selectedTowerType_ = TowerType::Ballista;
    ProjectilePool projectiles_{};
    Economy economy_{};
    BuildAttemptResult lastBuildResult_ = BuildAttemptResult::None;
    TowerActionResult lastTowerAction_ = TowerActionResult::None;
};
