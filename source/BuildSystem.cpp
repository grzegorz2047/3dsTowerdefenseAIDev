#include "BuildSystem.hpp"

#include <algorithm>

BuildSystem::BuildSystem(const LevelData& level) : level_(&level) {
    for (std::size_t z = 0; z < level.height && buildSpotCount_ < kMaximumTowers; ++z) {
        for (std::size_t x = 0; x < level.width && buildSpotCount_ < kMaximumTowers; ++x) {
            if (level.tileAt(x, z) == TileType::BuildSpot) {
                buildSpots_[buildSpotCount_++] = {
                    static_cast<std::int16_t>(x),
                    static_cast<std::int16_t>(z)};
            }
        }
    }
    reset();
}

void BuildSystem::handleInput(const InputSnapshot& input) {
    if (input.pressed(KEY_DLEFT) || input.pressed(KEY_DUP)) {
        moveCursor(-1);
    }
    if (input.pressed(KEY_DRIGHT) || input.pressed(KEY_DDOWN)) {
        moveCursor(1);
    }
    if (input.pressed(KEY_A)) {
        tryBuild();
    }
}

void BuildSystem::update(float deltaSeconds, Wave& wave) {
    for (std::size_t index = 0; index < towerCount_; ++index) {
        towers_[index].update(deltaSeconds, wave);
    }

    for (std::size_t index = 0; index < wave.spawnedCount() && index < rewarded_.size(); ++index) {
        const Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead() && !rewarded_[index]) {
            rewarded_[index] = true;
            gold_ += kKillReward;
        }
    }
}

void BuildSystem::reset() {
    towers_.fill(Tower{});
    rewarded_.fill(false);
    towerCount_ = 0;
    cursorIndex_ = 0;
    gold_ = kInitialGold;
}

std::size_t BuildSystem::towerCount() const {
    return towerCount_;
}

const Tower& BuildSystem::towerAt(std::size_t index) const {
    return towers_.at(index);
}

int BuildSystem::gold() const {
    return gold_;
}

int BuildSystem::towerCost() const {
    return kTowerCost;
}

std::size_t BuildSystem::cursorX() const {
    if (buildSpotCount_ == 0) {
        return 0;
    }
    return static_cast<std::size_t>(buildSpots_[cursorIndex_].x);
}

std::size_t BuildSystem::cursorZ() const {
    if (buildSpotCount_ == 0) {
        return 0;
    }
    return static_cast<std::size_t>(buildSpots_[cursorIndex_].z);
}

bool BuildSystem::cursorOccupied() const {
    return buildSpotCount_ > 0 && occupied(cursorX(), cursorZ());
}

bool BuildSystem::hasEnoughGold() const {
    return gold_ >= kTowerCost;
}

bool BuildSystem::cursorCanBuild() const {
    return level_ != nullptr && buildSpotCount_ > 0 && hasEnoughGold() && !cursorOccupied();
}

bool BuildSystem::occupied(std::size_t x, std::size_t z) const {
    for (std::size_t index = 0; index < towerCount_; ++index) {
        if (towers_[index].gridX() == x && towers_[index].gridZ() == z) {
            return true;
        }
    }
    return false;
}

void BuildSystem::moveCursor(int delta) {
    if (buildSpotCount_ == 0) {
        return;
    }
    const int count = static_cast<int>(buildSpotCount_);
    const int current = static_cast<int>(cursorIndex_);
    cursorIndex_ = static_cast<std::size_t>((current + delta + count) % count);
}

void BuildSystem::tryBuild() {
    if (!cursorCanBuild() || towerCount_ >= kMaximumTowers || level_ == nullptr) {
        return;
    }

    Tower tower(*level_, cursorX(), cursorZ());
    if (!tower.valid()) {
        return;
    }

    towers_[towerCount_++] = tower;
    gold_ -= kTowerCost;
}
