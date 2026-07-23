#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
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

std::string layoutSignature(const LevelData& level) {
    std::ostringstream stream;
    stream << static_cast<unsigned int>(level.width) << 'x'
           << static_cast<unsigned int>(level.height) << ':';
    for (std::size_t index = 0U; index < level.pathLength; ++index) {
        stream << level.path[index].x << ',' << level.path[index].z << ';';
    }
    stream << '|';
    for (std::size_t z = 0U; z < level.height; ++z) {
        for (std::size_t x = 0U; x < level.width; ++x) {
            if (level.tileAt(x, z) == TileType::BuildSpot) stream << x << ',' << z << ';';
        }
    }
    return stream.str();
}

std::size_t validateScene(const std::string& root, const LevelData& level,
    std::size_t minimumProps) {
    const std::string scenePath = root + "/romfs/scenes/" + level.id + ".art";
    std::ifstream sceneInput(scenePath);
    expect(sceneInput.is_open(), "scene file should exist: " + level.id);
    const SceneArtLoadResult scene = SceneArtLoader::parse(sceneInput);
    expect(scene.success, "scene should parse: " + level.id + " (" + scene.error + ")");
    expect(scene.art.levelId == level.id, "scene id should match level: " + level.id);
    expect(scene.art.theme == LevelTheme::VhalPass, "handcrafted scene should use Vhal Pass theme: " + level.id);
    expect(scene.art.propCount >= minimumProps, "scene should contain enough landmarks: " + level.id);

    std::set<ScenePropType> landmarkTypes;
    std::size_t propVertices = 0U;
    for (std::size_t index = 0U; index < scene.art.propCount; ++index) {
        const SceneProp& prop = scene.art.props[index];
        landmarkTypes.insert(prop.type);
        propVertices += scenePropVertexBudget(prop.type);
    }
    expect(landmarkTypes.count(ScenePropType::Ruin) > 0U, "scene should contain ruins: " + level.id);
    expect(landmarkTypes.count(ScenePropType::Watchtower) > 0U, "scene should contain watchtowers: " + level.id);
    expect(landmarkTypes.size() >= 5U, "scene should use varied modular props: " + level.id);

    const std::size_t tileVertices = static_cast<std::size_t>(level.width) * level.height * 6U;
    const std::size_t roadVertices = countTiles(level, TileType::Road) * kVerticesPerBox;
    const std::size_t buildPlatformVertices = countTiles(level, TileType::BuildSpot) * 2U * kVerticesPerBox;
    const std::size_t landmarksAndTerrain = 13U * kVerticesPerBox;
    const std::size_t conservativeTotal = tileVertices + roadVertices + buildPlatformVertices +
        landmarksAndTerrain + propVertices;
    expect(conservativeTotal <= PerformanceBudget::kMaximumLevelVertices,
        "scene should fit static geometry budget: " + level.id);
    return scene.art.propCount;
}

bool isLargeCampaignMap(const std::string& id) {
    return id == "flooded_road" || id == "iron_ravine" || id == "storm_ring";
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    const std::string root = argv[1];
    std::set<std::string> narrativeCardIds;
    std::set<std::string> layouts;
    std::size_t largeMapCount = 0U;

    for (const CampaignMission& mission : CampaignCatalog::missions()) {
        const std::string romfsPrefix = "romfs:/";
        const std::string configuredPath = mission.levelPath;
        expect(configuredPath.rfind(romfsPrefix, 0) == 0, "mission path should use romfs:/ prefix");
        const std::string hostPath = root + "/romfs/" + configuredPath.substr(romfsPrefix.size());
        const LevelLoadResult result = LevelLoader::loadFromRomFs(hostPath.c_str());
        expect(result.success, "level should load: " + std::string(mission.id) + " (" + result.error + ")");
        expect(result.level.id == mission.id, "level id should match catalog: " + std::string(mission.id));
        expect(!result.level.name.empty(), "level name should be present: " + std::string(mission.id));
        expect(result.level.pathLength >= 2, "path should be usable: " + std::string(mission.id));
        expect(result.level.waveEntryCount > 0, "waves should be present: " + std::string(mission.id));
        expect(result.level.totalEnemyCount > 0 && result.level.totalEnemyCount <= kMaximumWaveEnemies,
            "enemy count should fit budget: " + std::string(mission.id));
        expect(countTiles(result.level, TileType::BuildSpot) >= 3U,
            "mission should offer meaningful build choices: " + std::string(mission.id));
        expect(layouts.insert(layoutSignature(result.level)).second,
            "each campaign map should have a unique route and build layout: " + std::string(mission.id));

        if (result.level.id == "tutorial") validateScene(root, result.level, 18U);
        if (isLargeCampaignMap(result.level.id)) {
            ++largeMapCount;
            expect(result.level.width == 16U && result.level.height == 16U,
                "new campaign map should use full 16x16 area: " + result.level.id);
            expect(result.level.pathLength >= 25U,
                "large campaign map should have a substantial route: " + result.level.id);
            expect(countTiles(result.level, TileType::BuildSpot) >= 10U,
                "large map should provide distributed build positions: " + result.level.id);
            validateScene(root, result.level, 18U);
        }

        const std::string narrativePath = root + "/romfs/narrative/pl/" + mission.id + ".txt";
        std::ifstream narrativeInput(narrativePath);
        expect(narrativeInput.is_open(), "narrative file should exist: " + std::string(mission.id));
        const NarrativeLoadResult narrative = NarrativeLoader::parse(narrativeInput);
        expect(narrative.success, "narrative should parse: " + std::string(mission.id) + " (" + narrative.error + ")");
        expect(narrative.narrative.missionId == mission.id,
            "narrative id should match catalog: " + std::string(mission.id));
        expect(narrative.narrative.complete(), "narrative should be complete: " + std::string(mission.id));

        const NarrativeCard* cards[] = {
            &narrative.narrative.briefing[0], &narrative.narrative.briefing[1],
            &narrative.narrative.mechanic, &narrative.narrative.victory,
            &narrative.narrative.defeat};
        for (const NarrativeCard* card : cards) {
            expect(narrativeCardIds.insert(card->id).second,
                "narrative card id should be globally unique: " + card->id);
            expect(card->text.size() <= 180U,
                "narrative card should fit a short 3DS screen: " + card->id);
        }
    }

    expect(largeMapCount == 3U, "campaign should contain three new 16x16 maps");
    std::cout << "Campaign level, narrative and scene art tests passed\n";
    return 0;
}
