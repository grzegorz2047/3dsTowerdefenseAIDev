#include <cstdlib>
#include <iostream>
#include <string>

#include "Campaign.hpp"
#include "Level.hpp"

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
    }

    std::cout << "Campaign level files tests passed\n";
    return 0;
}
