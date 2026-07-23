#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "Campaign.hpp"
#include "Narrative.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testValidNarrativeParses() {
    std::istringstream input(
        "mission.id=test_mission\n"
        "briefing.1.id=test.arrival\nbriefing.1.speaker=Mira\nbriefing.1.text=Pierwsza karta.\n"
        "briefing.2.id=test.plan\nbriefing.2.speaker=Arven\nbriefing.2.text=Druga karta.\n"
        "mechanic.id=test.mechanic\nmechanic.speaker=Brom\nmechanic.text=Mechanika.\n"
        "victory.id=test.victory\nvictory.speaker=Arven\nvictory.text=Wygrana.\n"
        "defeat.id=test.defeat\ndefeat.speaker=Mira\ndefeat.text=Porazka.\n");
    const NarrativeLoadResult result = NarrativeLoader::parse(input);
    expect(result.success, "complete narrative should parse");
    expect(result.narrative.complete(), "parsed narrative should be complete");
    expect(result.narrative.briefing[1].speaker == "Arven", "speaker should be preserved");
}

void testInvalidNarrativeIsRejected() {
    std::istringstream missing("mission.id=test\nbriefing.1.id=test.one\n");
    expect(!NarrativeLoader::parse(missing).success, "missing cards must be rejected");

    std::istringstream unstable(
        "mission.id=Bad Id\n"
        "briefing.1.id=a\nbriefing.1.speaker=A\nbriefing.1.text=A\n"
        "briefing.2.id=b\nbriefing.2.speaker=B\nbriefing.2.text=B\n"
        "mechanic.id=c\nmechanic.speaker=C\nmechanic.text=C\n"
        "victory.id=d\nvictory.speaker=D\nvictory.text=D\n"
        "defeat.id=e\ndefeat.speaker=E\ndefeat.text=E\n");
    expect(!NarrativeLoader::parse(unstable).success, "unstable mission id must be rejected");
}

void testEveryCampaignMissionHasNarrative(const char* root) {
    std::set<std::string> cardIds;
    for (const CampaignMission& mission : CampaignCatalog::missions()) {
        const std::string path = std::string(root) + "/romfs/narrative/pl/" + mission.id + ".txt";
        std::ifstream input(path);
        expect(input.is_open(), "campaign narrative file must exist");
        const NarrativeLoadResult result = NarrativeLoader::parse(input);
        expect(result.success, "campaign narrative file must parse");
        expect(result.narrative.missionId == mission.id, "narrative mission id must match catalog");
        expect(result.narrative.complete(), "campaign narrative must contain all cards");

        const NarrativeCard* cards[] = {
            &result.narrative.briefing[0], &result.narrative.briefing[1],
            &result.narrative.mechanic, &result.narrative.victory, &result.narrative.defeat};
        for (const NarrativeCard* card : cards) {
            expect(cardIds.insert(card->id).second, "narrative card ids must be globally unique");
            expect(card->text.size() <= 180U, "narrative card must stay short for 3DS screen");
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    testValidNarrativeParses();
    testInvalidNarrativeIsRejected();
    testEveryCampaignMissionHasNarrative(argv[1]);
    std::cout << "Narrative tests passed\n";
    return 0;
}
