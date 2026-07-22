#include "Economy.hpp"

#include <limits>

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

bool Economy::credit(int amount) {
    if (amount <= 0 || gold_ > std::numeric_limits<int>::max() - amount) {
        return false;
    }
    gold_ += amount;
    return true;
}

bool Economy::rewardEnemy(std::size_t enemyIndex) {
    if (enemyIndex >= rewarded_.size() || rewarded_[enemyIndex]) {
        return false;
    }
    rewarded_[enemyIndex] = true;
    return credit(kKillReward);
}

int Economy::gold() const {
    return gold_;
}

bool Economy::canAfford(int amount) const {
    return amount >= 0 && gold_ >= amount;
}