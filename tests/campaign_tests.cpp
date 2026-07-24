#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

#include "Campaign.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testCatalogHasTenDistinctMissions() {
    const auto& missions = CampaignCatalog::missions();
    expect(missions.size() == 10U, "campaign should contain ten missions");
    std::set<std::string> ids;
    std::set<std::string> paths;
    std::uint8_t previousDifficulty = 0U;
    for (const CampaignMission& mission : missions) {
        expect(mission.id[0] != '\0', "mission should have an id");
        expect(mission.levelPath[0] != '\0', "mission should have a level path");
        expect(mission.title[0] != '\0', "mission card should have a title");
        expect(mission.objective[0] != '\0', "mission card should show its objective");
        expect(mission.availableTowers[0] != '\0', "mission card should show available defenses");
        expect(mission.threats[0] != '\0', "mission card should show threats");
        expect(mission.reward[0] != '\0', "mission card should show a reward");
        expect(ids.insert(mission.id).second, "mission ids should be unique");
        expect(paths.insert(mission.levelPath).second, "mission level paths should be unique");
        expect(mission.difficulty > previousDifficulty, "difficulty should increase every mission");
        previousDifficulty = mission.difficulty;
    }
    expect(std::string(missions[5].id) == "flooded_road",
        "sixth mission should introduce the larger map arc");
    expect(std::string(missions[7].id) == "storm_ring",
        "eighth mission should be the storm ring");
    expect(std::string(missions[9].id) == "portal_nexus",
        "tenth mission should be the multi-portal finale");
}

void testProgressUnlocksSequentially() {
    CampaignProgress progress;
    expect(progress.unlocked(0), "tutorial should start unlocked");
    expect(!progress.unlocked(1), "second mission should start locked");
    const MissionResult result = progress.complete(0, 3, 4);
    expect(result.stars == 3, "meeting both optional criteria should award three stars");
    expect(result.newlyUnlockedNext, "first completion should unlock next mission");
    expect(progress.unlocked(1), "second mission should be unlocked");
    expect(!progress.unlocked(2), "missions should unlock sequentially");
}

void testAllTenMissionsCanUnlock() {
    CampaignProgress progress;
    for (std::size_t index = 0U; index + 1U < kCampaignMissionCount; ++index) {
        expect(progress.unlocked(index), "current mission should be unlocked");
        const MissionResult result = progress.complete(index, 5, 4);
        expect(result.newlyUnlockedNext, "first victory should unlock the following mission");
    }
    expect(progress.unlocked(kCampaignMissionCount - 1U), "portal nexus should become unlocked");
    const MissionResult finalResult = progress.complete(kCampaignMissionCount - 1U, 5, 8);
    expect(!finalResult.newlyUnlockedNext, "final mission should not unlock beyond the catalog");
}

void testOptionalGoalsDoNotBlockProgress() {
    CampaignProgress progress;
    const MissionResult result = progress.complete(0, 1, 12);
    expect(result.stars == 1, "victory should always award at least one star");
    expect(progress.unlocked(1), "one-star victory should still unlock next mission");
}

void testReplayKeepsBestScore() {
    CampaignProgress progress;
    (void)progress.complete(0, 3, 4);
    const MissionResult replay = progress.complete(0, 1, 12);
    expect(replay.stars == 1, "replay should report stars from the current run");
    expect(progress.bestStars(0) == 3, "replay should not lower the best score");
    expect(!replay.newlyUnlockedNext, "replay should not repeatedly report the same unlock");
}

void testLockedOrFailedMissionDoesNotAdvance() {
    CampaignProgress progress;
    expect(progress.complete(1, 5, 1).stars == 0, "locked mission should not complete");
    expect(progress.complete(0, 0, 1).stars == 0, "defeat should not award stars");
    expect(!progress.unlocked(1), "defeat should not unlock next mission");
}

}  // namespace

int main() {
    testCatalogHasTenDistinctMissions();
    testProgressUnlocksSequentially();
    testAllTenMissionsCanUnlock();
    testOptionalGoalsDoNotBlockProgress();
    testReplayKeepsBestScore();
    testLockedOrFailedMissionDoesNotAdvance();
    std::cout << "Campaign tests passed\n";
    return 0;
}
