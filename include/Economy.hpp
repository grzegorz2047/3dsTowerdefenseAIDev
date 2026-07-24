#pragma once

#include <array>
#include <cstddef>

class Economy {
public:
    static constexpr int kInitialGold = 120;
    static constexpr int kTowerCost = 60;
    static constexpr int kKillReward = 15;
    static constexpr int kWaveCompletionReward = 25;
    static constexpr std::size_t kMaximumRewardedEnemies = 16;

    void reset();
    void beginWave();
    [[nodiscard]] bool trySpend(int amount);
    [[nodiscard]] bool credit(int amount);
    [[nodiscard]] bool rewardEnemy(std::size_t enemyIndex, int reward = kKillReward);
    [[nodiscard]] bool rewardWaveCompletion();

    [[nodiscard]] int gold() const;
    [[nodiscard]] bool canAfford(int amount) const;

private:
    int gold_ = kInitialGold;
    std::array<bool, kMaximumRewardedEnemies> rewarded_{};
};
