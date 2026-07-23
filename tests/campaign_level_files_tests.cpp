#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "Campaign.hpp"
#include "Level.hpp"
#include "Narrative.hpp"
#include "PerformanceBudget.hpp"
#include "SceneArt.hpp"

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

std::size_t countTiles(const LevelData& level, TileType type) {
    std::size_t count = 0U;
    for (std::size_t z = 0U; z < level.height; ++z) {
        for (std::size_t x = 0U; x < level.width; ++x) {
            if (level.tileAt(x, z) == type) ++count;
        }
    }
    return count;
}

void validateTutorialScene(const std::string& root, const LevelData& level) {
    const std::string scenePath = root + "/romfs/scenes/tutorial.art";
    std::ifstream sceneInput(scenePath);
    expect(sceneInput.is_open(), "tutorial scene file should exist");
    const SceneArtLoadResult scene = SceneArtLoader::parse(sceneInput);
    expect(scene.success, std::string("tutorial scene should parse: ") + scene.error);
    expect(scene.art.levelId == level.id, "scene id should match tutorial level");
    expect(scene.art.theme == LevelTheme::VhalPass, "tutorial should use Vhal Pass art direction");
    expect(scene.art.propCount >= 18U, "tutorial should contain a composed environment");

    std::set<ScenePropType> landmarkTypes;
    std::size_t propVertices = 0U;
    for (std::size_t index = 0U; index < scene.art.propCount; ++index) {
        const SceneProp& prop = scene.art.props[index];
        landmarkTypes.insert(prop.type);
        propVertices += scenePropVertexBudget(prop.type);
    }
    expect(landmarkTypes.count(ScenePropType::Ruin) > 0U, "scene should contain village ruins");
    expect(landmarkTypes.count(ScenePropType::Watchtower) > 0U, "scene should contain citadel towers");
    expect(landmarkTypes.count(ScenePropType::Pine) > 0U, "scene should contain valley vegetation");
    expect(landmarkTypes.size() >= 6U, "scene should use varied modular props");

    const std::size_t tileVertices = static_cast<std::size_t>(level.width) * level.height * 6U;
    const std::size_t roadVertices = countTiles(level, TileType::Road) * kVerticesPerBox;
    const std::size_t buildPlatformVertices = countTiles(level, TileType::BuildSpot) * 2U * kVerticesPerBox;
    const std::size_t landmarksAndTerrain = 13U * kVerticesPerBox;
    const std::size_t conservativeTotal = tileVertices + roadVertices + buildPlatformVertices +
        landmarksAndTerrain + propVertices;
    expect(conservativeTotal <= PerformanceBudget::kMaximumLevelVertices,
        "tutorial scene should fit the static geometry budget");
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
        if (result.level.id == "tutorial") validateTutorialScene(root, result.level);

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

    std::cout << "Campaign level, narrative and scene art tests passed\n";
    return 0;
}
