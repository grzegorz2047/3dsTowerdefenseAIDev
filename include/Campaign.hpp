#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr std::size_t kCampaignMissionCount = 6;

struct CampaignMission {
    const char* id = "";
    const char* levelPath = "";
    const char* title = "";
    const char* objective = "";
    const char* availableTowers = "";
    const char* threats = "";
    const char* reward = "";
    std::uint8_t fullHealthThreshold = 3;
    std::uint8_t efficientTowerLimit = 4;
};

struct MissionResult {
    std::uint8_t stars = 0;
    bool newlyUnlockedNext = false;
};

class CampaignCatalog {
public:
    [[nodiscard]] static const std::array<CampaignMission, kCampaignMissionCount>& missions();
    [[nodiscard]] static const CampaignMission& mission(std::size_t index);
};

class CampaignProgress {
public:
    void reset();
    [[nodiscard]] bool unlocked(std::size_t index) const;
    [[nodiscard]] std::uint8_t bestStars(std::size_t index) const;
    [[nodiscard]] std::size_t unlockedCount() const;
    [[nodiscard]] MissionResult complete(std::size_t index, int baseHealth, std::size_t towersBuilt);

private:
    std::array<std::uint8_t, kCampaignMissionCount> bestStars_{};
    std::size_t unlockedCount_ = 1;
};
