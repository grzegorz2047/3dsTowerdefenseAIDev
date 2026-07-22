#include <cstdlib>
#include <iostream>

#include "Campaign.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testCatalogHasTutorialAndFiveRegionMissions() {
    const auto& missions = CampaignCatalog::missions();
    expect(missions.size() == 6, "campaign should contain tutorial and five region missions");
    expect(missions.front().id[0] != '\0', "tutorial should have an id");
    expect(missions.back().levelPath[0] != '\0', "final mission should have a level path");
    for (const CampaignMission& mission : missions) {
        expect(mission.title[0] != '\0', "mission card should have a title");
        expect(mission.objective[0] != '\0', "mission card should show its objective");
        expect(mission.availableTowers[0] != '\0', "mission card should show available towers");
        expect(mission.threats[0] != '\0', "mission card should show threats");
        expect(mission.reward[0] != '\0', "mission card should show a reward");
    }
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
    testCatalogHasTutorialAndFiveRegionMissions();
    testProgressUnlocksSequentially();
    testOptionalGoalsDoNotBlockProgress();
    testReplayKeepsBestScore();
    testLockedOrFailedMissionDoesNotAdvance();
    std::cout << "Campaign tests passed\n";
    return 0;
}
