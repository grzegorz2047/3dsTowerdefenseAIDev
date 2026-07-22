#include "Campaign.hpp"

#include <algorithm>
#include <limits>

namespace {

constexpr std::array<CampaignMission, kCampaignMissionCount> kMissions{{
    {"tutorial", "romfs:/levels/tutorial.lvl", "Straznica", "Zbuduj pierwsza wieze i obron brame.",
        "Kusza, Mozdzierz, Mroz", "Scout, Raider, Brute", "Dostep do Popielnej Bramy", 3, 4},
    {"ash_gate", "romfs:/levels/ash_gate.lvl", "Popielna Brama", "Utrzymaj waskie przejscie przez ruiny.",
        "Kusza, Mozdzierz, Mroz", "Szybkie grupy Scoutow", "Dostep do Zniszczonej Osady", 4, 5},
    {"ruined_village", "romfs:/levels/ruined_village.lvl", "Zniszczona Osada", "Obron cywilow przy rozwidlonej drodze.",
        "Kusza, Mozdzierz, Mroz", "Raiderzy w zwartych grupach", "Dostep do Kamiennego Mostu", 4, 6},
    {"stone_bridge", "romfs:/levels/stone_bridge.lvl", "Kamienny Most", "Zatrzymaj natarcie przed mostem.",
        "Kusza, Mozdzierz, Mroz", "Brute wspierani przez Scoutow", "Dostep do Doliny Echa", 5, 6},
    {"echo_valley", "romfs:/levels/echo_valley.lvl", "Dolina Echa", "Przetrwaj dluga trase i mieszane fale.",
        "Kusza, Mozdzierz, Mroz", "Mieszane, geste fale", "Dostep do Ostatniej Cytadeli", 5, 7},
    {"last_citadel", "romfs:/levels/last_citadel.lvl", "Ostatnia Cytadela", "Pokonaj finalne natarcie regionu.",
        "Kusza, Mozdzierz, Mroz", "Najsilniejsze fale i Brute", "Pieczec obroncy Asterii", 5, 8},
}};

}  // namespace

const std::array<CampaignMission, kCampaignMissionCount>& CampaignCatalog::missions() {
    return kMissions;
}

const CampaignMission& CampaignCatalog::mission(std::size_t index) {
    return kMissions.at(index);
}

void CampaignProgress::reset() {
    bestStars_.fill(0);
    bestBaseHealth_.fill(0);
    fewestTowers_.fill(0);
    unlockedCount_ = 1;
}

bool CampaignProgress::unlocked(std::size_t index) const {
    return index < unlockedCount_ && index < kCampaignMissionCount;
}

std::uint8_t CampaignProgress::bestStars(std::size_t index) const {
    return index < kCampaignMissionCount ? bestStars_[index] : 0;
}

std::uint8_t CampaignProgress::bestBaseHealth(std::size_t index) const {
    return index < kCampaignMissionCount ? bestBaseHealth_[index] : 0;
}

std::uint8_t CampaignProgress::fewestTowers(std::size_t index) const {
    return index < kCampaignMissionCount ? fewestTowers_[index] : 0;
}

std::size_t CampaignProgress::unlockedCount() const {
    return unlockedCount_;
}

CampaignProgressSnapshot CampaignProgress::snapshot() const {
    return {unlockedCount_, bestStars_, bestBaseHealth_, fewestTowers_};
}

bool CampaignProgress::restore(const CampaignProgressSnapshot& snapshotValue) {
    if (snapshotValue.unlockedCount == 0 || snapshotValue.unlockedCount > kCampaignMissionCount) {
        return false;
    }
    for (std::size_t index = 0; index < kCampaignMissionCount; ++index) {
        if (snapshotValue.bestStars[index] > 3 || snapshotValue.bestBaseHealth[index] > 5) {
            return false;
        }
        if (index >= snapshotValue.unlockedCount && snapshotValue.bestStars[index] != 0) {
            return false;
        }
    }
    unlockedCount_ = snapshotValue.unlockedCount;
    bestStars_ = snapshotValue.bestStars;
    bestBaseHealth_ = snapshotValue.bestBaseHealth;
    fewestTowers_ = snapshotValue.fewestTowers;
    return true;
}

MissionResult CampaignProgress::complete(std::size_t index, int baseHealth, std::size_t towersBuilt) {
    if (!unlocked(index) || baseHealth <= 0) {
        return {};
    }

    const CampaignMission& definition = CampaignCatalog::mission(index);
    std::uint8_t stars = 1;
    if (baseHealth >= definition.fullHealthThreshold) {
        ++stars;
    }
    if (towersBuilt <= definition.efficientTowerLimit) {
        ++stars;
    }

    bestStars_[index] = std::max(bestStars_[index], stars);
    bestBaseHealth_[index] = std::max(bestBaseHealth_[index], static_cast<std::uint8_t>(std::min(baseHealth, 5)));
    const std::uint8_t towerRecord = static_cast<std::uint8_t>(std::min<std::size_t>(towersBuilt, std::numeric_limits<std::uint8_t>::max()));
    if (fewestTowers_[index] == 0 || towerRecord < fewestTowers_[index]) {
        fewestTowers_[index] = towerRecord;
    }

    const std::size_t previousUnlockedCount = unlockedCount_;
    if (index + 1U < kCampaignMissionCount) {
        unlockedCount_ = std::max(unlockedCount_, index + 2U);
    }
    return {stars, unlockedCount_ > previousUnlockedCount};
}
