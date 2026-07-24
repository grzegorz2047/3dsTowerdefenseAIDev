#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr std::size_t kOriginalCampaignMissionCount = 6;
constexpr std::size_t kVersionFiveCampaignMissionCount = 9;
constexpr std::size_t kCampaignMissionCount = 10;

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
    std::uint8_t difficulty = 1;
};

struct MissionResult {
    std::uint8_t stars = 0;
    bool newlyUnlockedNext = false;
};

struct CampaignProgressSnapshot {
    std::size_t unlockedCount = 1;
    std::array<std::uint8_t, kCampaignMissionCount> bestStars{};
    std::array<std::uint8_t, kCampaignMissionCount> bestBaseHealth{};
    std::array<std::uint8_t, kCampaignMissionCount> fewestTowers{};
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
    [[nodiscard]] std::uint8_t bestBaseHealth(std::size_t index) const;
    [[nodiscard]] std::uint8_t fewestTowers(std::size_t index) const;
    [[nodiscard]] std::size_t unlockedCount() const;
    [[nodiscard]] CampaignProgressSnapshot snapshot() const;
    [[nodiscard]] bool restore(const CampaignProgressSnapshot& snapshot);
    [[nodiscard]] MissionResult complete(std::size_t index, int baseHealth, std::size_t towersBuilt);

private:
    std::array<std::uint8_t, kCampaignMissionCount> bestStars_{};
    std::array<std::uint8_t, kCampaignMissionCount> bestBaseHealth_{};
    std::array<std::uint8_t, kCampaignMissionCount> fewestTowers_{};
    std::size_t unlockedCount_ = 1;
};
