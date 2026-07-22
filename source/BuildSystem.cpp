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
        cancelAction();
    }
    if (input.pressed(KEY_DRIGHT) || input.pressed(KEY_DDOWN)) {
        moveCursor(1);
        cancelAction();
    }
    if (input.pressed(KEY_L)) {
        selectTowerType(previousTowerType(selectedTowerType_));
    }
    if (input.pressed(KEY_R)) {
        selectTowerType(nextTowerType(selectedTowerType_));
    }
    if (input.pressed(KEY_A)) {
        buildOrSelectCursor();
    }
    if (input.pressed(KEY_B) && !input.isHeld(KEY_SELECT)) {
        lastTowerAction_ = upgradeCursorTower();
    }
    if (input.pressed(KEY_Y)) {
        lastTowerAction_ = sellCursorTower();
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
    selectedTowerType_ = TowerType::Ballista;
    projectiles_.reset();
    economy_.reset();
    cancelAction();
}

void BuildSystem::selectTowerType(TowerType type) {
    selectedTowerType_ = type;
    cancelAction();
}

void BuildSystem::buildOrSelectCursor() {
    tryBuild();
}

void BuildSystem::cancelAction() {
    lastBuildResult_ = BuildAttemptResult::None;
    lastTowerAction_ = TowerActionResult::None;
}

TowerActionResult BuildSystem::upgradeCursorTower() {
    const std::size_t index = towerIndexAt(cursorX(), cursorZ());
    if (index >= towerCount_) {
        return TowerActionResult::NoTower;
    }
    Tower& tower = towers_[index];
    if (!tower.canUpgrade()) {
        return TowerActionResult::MaximumLevel;
    }
    const int cost = tower.upgradeCost();
    if (!economy_.trySpend(cost)) {
        return TowerActionResult::InsufficientGold;
    }
    if (!tower.upgrade()) {
        economy_.credit(cost);
        return TowerActionResult::MaximumLevel;
    }
    return TowerActionResult::Upgraded;
}

TowerActionResult BuildSystem::sellCursorTower() {
    const std::size_t index = towerIndexAt(cursorX(), cursorZ());
    if (index >= towerCount_) {
        return TowerActionResult::NoTower;
    }
    const int refund = towers_[index].sellValue();
    if (!economy_.credit(refund)) {
        return TowerActionResult::None;
    }
    for (std::size_t move = index + 1; move < towerCount_; ++move) {
        towers_[move - 1] = towers_[move];
    }
    --towerCount_;
    towers_[towerCount_] = Tower{};
    return TowerActionResult::Sold;
}

std::size_t BuildSystem::towerCount() const { return towerCount_; }
const Tower& BuildSystem::towerAt(std::size_t index) const { return towers_.at(index); }
const ProjectilePool& BuildSystem::projectiles() const { return projectiles_; }
int BuildSystem::gold() const { return economy_.gold(); }
int BuildSystem::towerCost() const { return ::towerCost(selectedTowerType_); }
TowerType BuildSystem::selectedTowerType() const { return selectedTowerType_; }
const char* BuildSystem::selectedTowerName() const { return towerName(selectedTowerType_); }
BuildAttemptResult BuildSystem::lastBuildResult() const { return lastBuildResult_; }
TowerActionResult BuildSystem::lastTowerAction() const { return lastTowerAction_; }

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

const Tower* BuildSystem::cursorTower() const {
    const std::size_t index = towerIndexAt(cursorX(), cursorZ());
    return index < towerCount_ ? &towers_[index] : nullptr;
}

bool BuildSystem::hasEnoughGold() const {
    return economy_.canAfford(towerCost());
}

bool BuildSystem::cursorCanBuild() const {
    return level_ != nullptr && buildSpotCount_ > 0 && hasEnoughGold() && !cursorOccupied() && towerCount_ < kMaximumTowers;
}

std::size_t BuildSystem::towerIndexAt(std::size_t x, std::size_t z) const {
    for (std::size_t index = 0; index < towerCount_; ++index) {
        if (towers_[index].gridX() == x && towers_[index].gridZ() == z) {
            return index;
        }
    }
    return towerCount_;
}

bool BuildSystem::occupied(std::size_t x, std::size_t z) const {
    return towerIndexAt(x, z) < towerCount_;
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
    const bool hasBuildSpot = level_ != nullptr && buildSpotCount_ > 0;
    const bool spotOccupied = hasBuildSpot && cursorOccupied();
    if (spotOccupied) {
        lastBuildResult_ = BuildAttemptResult::None;
        lastTowerAction_ = TowerActionResult::Selected;
        return;
    }

    const bool enoughGold = hasEnoughGold();
    const bool hasCapacity = towerCount_ < kMaximumTowers;
    BuildAttemptResult result = evaluateBuildAttempt(
        hasBuildSpot,
        false,
        enoughGold,
        hasCapacity,
        true);
    if (result != BuildAttemptResult::Built) {
        lastBuildResult_ = result;
        lastTowerAction_ = TowerActionResult::None;
        return;
    }

    Tower tower(*level_, cursorX(), cursorZ(), selectedTowerType_);
    if (!tower.valid()) {
        lastBuildResult_ = BuildAttemptResult::InvalidTower;
        return;
    }
    if (!economy_.trySpend(towerCost())) {
        lastBuildResult_ = BuildAttemptResult::InsufficientGold;
        return;
    }

    towers_[towerCount_++] = tower;
    lastBuildResult_ = BuildAttemptResult::Built;
    lastTowerAction_ = TowerActionResult::None;
}
