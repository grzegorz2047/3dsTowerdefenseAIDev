#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "Campaign.hpp"
#include "Level.hpp"
#include "Narrative.hpp"

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    const std::string root = argv[1];
    std::set<std::string> narrativeCardIds;

    for (const CampaignMission& mission : CampaignCatalog::missions()) {
        const std::string romfsPrefix = "romfs:/";
        const std::string configuredPath = mission.levelPath;
        expect(configuredPath.rfind(romfsPrefix, 0) == 0, "mission path should use romfs:/ prefix");
        const std::string hostPath = root + "/romfs/" + configuredPath.substr(romfsPrefix.size());
        const LevelLoadResult result = LevelLoader::loadFromRomFs(hostPath.c_str());
        expect(result.success, std::string("level should load: ") + mission.id + " (" + result.error + ")");
        expect(result.level.id == mission.id, std::string("level id should match catalog: ") + mission.id);
        expect(!result.level.name.empty(), std::string("level name should be present: ") + mission.id);
        expect(result.level.pathLength >= 2, std::string("path should be usable: ") + mission.id);
        expect(result.level.waveEntryCount > 0, std::string("waves should be present: ") + mission.id);
        expect(result.level.totalEnemyCount > 0, std::string("enemies should be present: ") + mission.id);

        const std::string narrativePath = root + "/romfs/narrative/pl/" + mission.id + ".txt";
        std::ifstream narrativeInput(narrativePath);
        expect(narrativeInput.is_open(), std::string("narrative file should exist: ") + mission.id);
        const NarrativeLoadResult narrative = NarrativeLoader::parse(narrativeInput);
        expect(narrative.success, std::string("narrative should parse: ") + mission.id + " (" + narrative.error + ")");
        expect(narrative.narrative.missionId == mission.id,
            std::string("narrative id should match catalog: ") + mission.id);
        expect(narrative.narrative.complete(), std::string("narrative should be complete: ") + mission.id);

        const NarrativeCard* cards[] = {
            &narrative.narrative.briefing[0], &narrative.narrative.briefing[1],
            &narrative.narrative.mechanic, &narrative.narrative.victory,
            &narrative.narrative.defeat};
        for (const NarrativeCard* card : cards) {
            expect(narrativeCardIds.insert(card->id).second,
                std::string("narrative card id should be globally unique: ") + card->id);
            expect(card->text.size() <= 180U,
                std::string("narrative card should fit a short 3DS screen: ") + card->id);
        }
    }

    std::cout << "Campaign level and narrative files tests passed\n";
    return 0;
}
