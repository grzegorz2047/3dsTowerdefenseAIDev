#include "BuildSystem.hpp"

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
        towers_[index].update(deltaSeconds, wave, projectiles_);
    }
    projectiles_.update(deltaSeconds, wave);

    for (std::size_t index = 0; index < wave.spawnedCount(); ++index) {
        const Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead()) {
            economy_.rewardEnemy(index);
        }
    }
}

void BuildSystem::reset() {
    towers_.fill(Tower{});
    towerCount_ = 0;
    cursorIndex_ = 0;
    projectiles_.reset();
    economy_.reset();
}

std::size_t BuildSystem::towerCount() const { return towerCount_; }
const Tower& BuildSystem::towerAt(std::size_t index) const { return towers_.at(index); }
const ProjectilePool& BuildSystem::projectiles() const { return projectiles_; }
int BuildSystem::gold() const { return economy_.gold(); }
int BuildSystem::towerCost() const { return Economy::kTowerCost; }

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
    return economy_.canAfford(Economy::kTowerCost);
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
    if (!economy_.trySpend(Economy::kTowerCost)) {
        return;
    }

    towers_[towerCount_++] = tower;
}
