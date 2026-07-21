#include "Economy.hpp"

void Economy::reset() {
    gold_ = kInitialGold;
    rewarded_.fill(false);
}

bool Economy::trySpend(int amount) {
    if (amount < 0 || !canAfford(amount)) {
        return false;
    }
    gold_ -= amount;
    return true;
}

bool Economy::rewardEnemy(std::size_t enemyIndex) {
    if (enemyIndex >= rewarded_.size() || rewarded_[enemyIndex]) {
        return false;
    }
    rewarded_[enemyIndex] = true;
    gold_ += kKillReward;
    return true;
}

int Economy::gold() const {
    return gold_;
}

bool Economy::canAfford(int amount) const {
    return amount >= 0 && gold_ >= amount;
}
