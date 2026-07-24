#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "SaveData.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

std::uint32_t fnv1a(const std::string& value) {
    std::uint32_t hash = 2166136261U;
    for (const unsigned char byte : value) {
        hash ^= byte;
        hash *= 16777619U;
    }
    return hash;
}

std::string withChecksum(const std::string& payload) {
    std::ostringstream stream;
    stream << payload << "checksum=" << fnv1a(payload) << '\n';
    return stream.str();
}

void testVersionSixRoundTrip() {
    SaveData source{};
    source.campaign.unlockedCount = 10;
    source.campaign.bestStars = {3, 2, 1, 3, 2, 1, 2, 3, 2, 1};
    source.campaign.bestBaseHealth = {5, 4, 2, 5, 4, 2, 4, 5, 4, 2};
    source.campaign.fewestTowers = {2, 4, 7, 5, 6, 7, 6, 8, 8, 9};
    source.settings.soundEnabled = false;
    source.settings.musicEnabled = false;
    source.settings.preferredSpeed = 2;
    source.settings.stereoEnabled = false;
    source.settings.maximum3DDepthPercent = 45;

    const std::string serialized = SaveDataCodec::serialize(source);
    expect(serialized.find("version=6\n") != std::string::npos,
        "current save should use version six");
    expect(serialized.find("speed=1\n") != std::string::npos,
        "temporary 2x speed must never be persisted");
    const SaveLoadResult result = SaveDataCodec::deserialize(serialized);
    expect(result.status == SaveLoadStatus::Loaded, "v6 save should deserialize");
    expect(!result.migrated, "v6 save should not report migration");
    expect(result.data.campaign.unlockedCount == 10U, "unlocked count should round-trip");
    expect(result.data.campaign.bestStars[9] == 1U, "portal mission score should round-trip");
    expect(!result.data.settings.soundEnabled, "sound setting should round-trip");
    expect(!result.data.settings.musicEnabled, "music setting should round-trip independently");
    expect(result.data.settings.preferredSpeed == 1U,
        "every loaded session should return to safe 1x speed");
    expect(!result.data.settings.stereoEnabled, "stereo setting should round-trip");
    expect(result.data.settings.maximum3DDepthPercent == 45U, "3D depth should round-trip");
}

void testVersionFiveCampaignExpansionMigration() {
    const std::string payload =
        "version=5\n"
        "unlocked=9\n"
        "stars=3,2,1,3,2,1,2,3,2\n"
        "base_health=5,4,2,5,4,2,4,5,4\n"
        "fewest_towers=2,4,7,5,6,7,6,8,8\n"
        "sound=0\n"
        "speed=2\n"
        "stereo=0\n"
        "stereo_depth=45\n"
        "music=0\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Loaded, "v5 save should migrate");
    expect(result.migrated, "v5 save should report migration");
    expect(result.data.campaign.unlockedCount == 9U, "v5 unlock state should remain unchanged");
    expect(result.data.campaign.bestStars[8] == 2U, "old finale score should be preserved");
    expect(result.data.campaign.bestStars[9] == 0U, "portal mission should start without a score");
    expect(!result.data.settings.musicEnabled, "v5 music choice should be preserved");
    expect(result.data.settings.preferredSpeed == 1U, "v5 speed should normalize to 1x");
}

void testVersionFourCampaignExpansionMigration() {
    const std::string payload =
        "version=4\n"
        "unlocked=6\n"
        "stars=3,2,1,3,2,1\n"
        "base_health=5,4,2,5,4,2\n"
        "fewest_towers=2,4,7,5,6,7\n"
        "sound=0\n"
        "speed=2\n"
        "stereo=0\n"
        "stereo_depth=45\n"
        "music=0\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Loaded, "v4 save should migrate");
    expect(result.migrated, "v4 save should report migration");
    expect(result.data.campaign.unlockedCount == 6U, "legacy unlocked progress should remain unchanged");
    expect(result.data.campaign.bestStars[5] == 1U, "legacy final score should be preserved");
    expect(result.data.campaign.bestStars[6] == 0U && result.data.campaign.bestStars[9] == 0U,
        "new missions should start without scores");
}

void testOlderMigrations() {
    const std::string versionThree =
        "version=3\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n"
        "base_health=5,2,0,0,0,0\n"
        "fewest_towers=2,8,0,0,0,0\n"
        "sound=0\n"
        "speed=2\n"
        "stereo=0\n"
        "stereo_depth=45\n";
    const SaveLoadResult v3 = SaveDataCodec::deserialize(withChecksum(versionThree));
    expect(v3.status == SaveLoadStatus::Loaded && v3.migrated, "v3 save should migrate");
    expect(v3.data.campaign.bestStars[9] == 0U, "v3 portal slot should be empty");

    const std::string versionTwo =
        "version=2\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n"
        "base_health=5,2,0,0,0,0\n"
        "fewest_towers=2,8,0,0,0,0\n"
        "sound=0\n"
        "speed=2\n";
    const SaveLoadResult v2 = SaveDataCodec::deserialize(withChecksum(versionTwo));
    expect(v2.status == SaveLoadStatus::Loaded && v2.migrated, "v2 save should migrate");

    const std::string versionOne =
        "version=1\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n";
    const SaveLoadResult v1 = SaveDataCodec::deserialize(withChecksum(versionOne));
    expect(v1.status == SaveLoadStatus::Loaded && v1.migrated, "v1 save should migrate");
}

void testCorruptionAndArrayLengthAreRejected() {
    SaveData data{};
    std::string serialized = SaveDataCodec::serialize(data);
    serialized[serialized.find("stars=") + 6U] = '9';
    expect(SaveDataCodec::deserialize(serialized).status == SaveLoadStatus::Corrupt,
        "checksum mismatch should be corrupt");

    const std::string wrongV6 =
        "version=6\n"
        "unlocked=1\n"
        "stars=0,0,0,0,0,0,0,0,0\n"
        "base_health=0,0,0,0,0,0,0,0,0\n"
        "fewest_towers=0,0,0,0,0,0,0,0,0\n"
        "sound=1\n"
        "speed=1\n"
        "stereo=1\n"
        "stereo_depth=60\n"
        "music=1\n";
    expect(SaveDataCodec::deserialize(withChecksum(wrongV6)).status == SaveLoadStatus::Corrupt,
        "v6 should require ten campaign entries");
}

void testAtomicStoreAndReset(const char* root) {
    const std::string path = std::string(root) + "/build/host-tests/save-data-test.sav";
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());
    SaveData source{};
    std::string error;
    expect(SaveDataStore::saveAtomically(path.c_str(), source, error), "atomic save should succeed");
    expect(SaveDataStore::load(path.c_str()).status == SaveLoadStatus::Loaded, "stored save should load");
    expect(SaveDataStore::reset(path.c_str()), "reset should remove save");
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    testVersionSixRoundTrip();
    testVersionFiveCampaignExpansionMigration();
    testVersionFourCampaignExpansionMigration();
    testOlderMigrations();
    testCorruptionAndArrayLengthAreRejected();
    testAtomicStoreAndReset(argv[1]);
    std::cout << "Save data tests passed\n";
    return 0;
}
