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
    const ExtendedMappedInput extended = input.extended();
    if (input.pressed(KEY_DLEFT) || input.pressed(KEY_DUP) || extended.cursorDelta < 0) {
        moveCursor(-1);
        cancelAction();
    }
    if (input.pressed(KEY_DRIGHT) || input.pressed(KEY_DDOWN) || extended.cursorDelta > 0) {
        moveCursor(1);
        cancelAction();
    }
    if (input.pressed(KEY_L) || extended.legacyShoulderDelta < 0) {
        selectTowerType(previousTowerType(selectedTowerType_));
    }
    if (input.pressed(KEY_R) || extended.legacyShoulderDelta > 0) {
        selectTowerType(nextTowerType(selectedTowerType_));
    }
    if (input.pressed(KEY_A)) {
        buildOrSelectCursor();
    }
    if (input.pressed(KEY_B) && !input.isHeld(KEY_SELECT)) {
        lastTowerAction_ = upgradeCursorTower();
    }
    if (input.pressed(KEY_Y) && !input.isHeld(KEY_SELECT)) {
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
std::size_t BuildSystem::cursorX() const { return static_cast<std::size_t>(buildSpots_[cursorIndex_].x); }
std::size_t BuildSystem::cursorZ() const { return static_cast<std::size_t>(buildSpots_[cursorIndex_].z); }
bool BuildSystem::cursorOccupied() const { return occupied(cursorX(), cursorZ()); }
const Tower* BuildSystem::cursorTower() const {
    const std::size_t index = towerIndexAt(cursorX(), cursorZ());
    return index < towerCount_ ? &towers_[index] : nullptr;
}
bool BuildSystem::hasEnoughGold() const { return economy_.canAfford(towerCost()); }
bool BuildSystem::cursorCanBuild() const { return !cursorOccupied() && hasEnoughGold(); }
BuildAttemptResult BuildSystem::lastBuildResult() const { return lastBuildResult_; }
TowerActionResult BuildSystem::lastTowerAction() const { return lastTowerAction_; }

std::size_t BuildSystem::towerIndexAt(std::size_t x, std::size_t z) const {
    for (std::size_t index = 0; index < towerCount_; ++index) {
        if (towers_[index].gridX() == x && towers_[index].gridZ() == z) return index;
    }
    return towerCount_;
}

bool BuildSystem::occupied(std::size_t x, std::size_t z) const {
    return towerIndexAt(x, z) < towerCount_;
}

void BuildSystem::moveCursor(int delta) {
    if (buildSpotCount_ == 0U || delta == 0) return;
    if (delta < 0) cursorIndex_ = cursorIndex_ == 0U ? buildSpotCount_ - 1U : cursorIndex_ - 1U;
    else cursorIndex_ = (cursorIndex_ + 1U) % buildSpotCount_;
}

void BuildSystem::tryBuild() {
    if (buildSpotCount_ == 0U) {
        lastBuildResult_ = BuildAttemptResult::InvalidTile;
        return;
    }
    if (occupied(cursorX(), cursorZ())) {
        lastBuildResult_ = BuildAttemptResult::Occupied;
        return;
    }
    const int cost = towerCost();
    if (!economy_.trySpend(cost)) {
        lastBuildResult_ = BuildAttemptResult::InsufficientGold;
        return;
    }
    towers_[towerCount_] = Tower(*level_, cursorX(), cursorZ(), selectedTowerType_);
    ++towerCount_;
    lastBuildResult_ = BuildAttemptResult::Built;
    lastTowerAction_ = TowerActionResult::None;
}
