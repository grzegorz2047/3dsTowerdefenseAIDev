#pragma once

#include <array>
#include <cstddef>

#include "Input.hpp"
#include "Level.hpp"
#include "Tower.hpp"
#include "Wave.hpp"

class BuildSystem {
public:
    explicit BuildSystem(const LevelData& level);

    void handleInput(const InputSnapshot& input);
    void update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] std::size_t towerCount() const;
    [[nodiscard]] const Tower& towerAt(std::size_t index) const;
    [[nodiscard]] int gold() const;
    [[nodiscard]] std::size_t cursorX() const;
    [[nodiscard]] std::size_t cursorZ() const;
    [[nodiscard]] bool cursorCanBuild() const;

private:
    static constexpr std::size_t kMaximumTowers = 16;
    static constexpr int kInitialGold = 120;
    static constexpr int kTowerCost = 60;
    static constexpr int kKillReward = 15;

    [[nodiscard]] bool occupied(std::size_t x, std::size_t z) const;
    void moveCursor(int delta);
    void tryBuild();

    const LevelData* level_ = nullptr;
    std::array<GridPoint, kMaximumTowers> buildSpots_{};
    std::array<Tower, kMaximumTowers> towers_{};
    std::array<bool, kMaximumTowers> rewarded_{};
    std::size_t buildSpotCount_ = 0;
    std::size_t towerCount_ = 0;
    std::size_t cursorIndex_ = 0;
    int gold_ = kInitialGold;
};
