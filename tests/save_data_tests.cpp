#include <cstdio>
#include <cstdlib>
#include <fstream>
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

void testVersionFourRoundTrip() {
    SaveData source{};
    source.campaign.unlockedCount = 3;
    source.campaign.bestStars = {3, 2, 1, 0, 0, 0};
    source.campaign.bestBaseHealth = {5, 4, 2, 0, 0, 0};
    source.campaign.fewestTowers = {2, 4, 7, 0, 0, 0};
    source.settings.soundEnabled = false;
    source.settings.musicEnabled = false;
    source.settings.preferredSpeed = 2;
    source.settings.stereoEnabled = false;
    source.settings.maximum3DDepthPercent = 45;

    const SaveLoadResult result = SaveDataCodec::deserialize(SaveDataCodec::serialize(source));
    expect(result.status == SaveLoadStatus::Loaded, "v4 save should deserialize");
    expect(!result.migrated, "v4 save should not report migration");
    expect(result.data.campaign.unlockedCount == 3, "unlocked count should round-trip");
    expect(result.data.campaign.bestStars[1] == 2, "stars should round-trip");
    expect(!result.data.settings.soundEnabled, "sound setting should round-trip");
    expect(!result.data.settings.musicEnabled, "music setting should round-trip independently");
    expect(result.data.settings.preferredSpeed == 2, "speed setting should round-trip");
    expect(!result.data.settings.stereoEnabled, "stereo setting should round-trip");
    expect(result.data.settings.maximum3DDepthPercent == 45, "3D depth should round-trip");
}

void testVersionThreeMigration() {
    const std::string payload =
        "version=3\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n"
        "base_health=5,2,0,0,0,0\n"
        "fewest_towers=2,8,0,0,0,0\n"
        "sound=0\n"
        "speed=2\n"
        "stereo=0\n"
        "stereo_depth=45\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Loaded, "v3 save should load");
    expect(result.migrated, "v3 save should report migration");
    expect(!result.data.settings.soundEnabled, "old sound setting should migrate");
    expect(result.data.settings.musicEnabled, "new music setting should default to enabled");
    expect(!result.data.settings.stereoEnabled, "old stereo setting should migrate");
}

void testVersionTwoMigration() {
    const std::string payload =
        "version=2\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n"
        "base_health=5,2,0,0,0,0\n"
        "fewest_towers=2,8,0,0,0,0\n"
        "sound=0\n"
        "speed=2\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Loaded, "v2 save should load");
    expect(result.migrated, "v2 save should report migration");
    expect(!result.data.settings.soundEnabled, "old sound setting should migrate");
    expect(result.data.settings.musicEnabled, "music should use safe default");
    expect(result.data.settings.preferredSpeed == 2, "old speed setting should migrate");
    expect(result.data.settings.stereoEnabled, "new stereo setting should use safe default");
    expect(result.data.settings.maximum3DDepthPercent == Stereo3D::kDefaultDepthPercent,
        "new depth limit should use safe default");
}

void testVersionOneMigration() {
    const std::string payload =
        "version=1\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Loaded, "v1 save should load");
    expect(result.migrated, "v1 save should report migration");
    expect(result.data.campaign.bestBaseHealth[0] == 0, "new records should use defaults");
    expect(result.data.settings.stereoEnabled, "new settings should use defaults");
    expect(result.data.settings.musicEnabled, "music should use enabled default");
}

void testCorruptionIsRejected() {
    SaveData data{};
    std::string serialized = SaveDataCodec::serialize(data);
    serialized[serialized.find("stars=") + 6] = '9';
    expect(SaveDataCodec::deserialize(serialized).status == SaveLoadStatus::Corrupt,
        "checksum mismatch should be corrupt");
}

void testInvalidMusicSettingIsRejected() {
    const std::string payload =
        "version=4\n"
        "unlocked=1\n"
        "stars=0,0,0,0,0,0\n"
        "base_health=0,0,0,0,0,0\n"
        "fewest_towers=0,0,0,0,0,0\n"
        "sound=1\n"
        "speed=1\n"
        "stereo=1\n"
        "stereo_depth=60\n"
        "music=2\n";
    expect(SaveDataCodec::deserialize(withChecksum(payload)).status == SaveLoadStatus::Corrupt,
        "out-of-range music setting should be rejected");
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
    testVersionFourRoundTrip();
    testVersionThreeMigration();
    testVersionTwoMigration();
    testVersionOneMigration();
    testCorruptionIsRejected();
    testInvalidMusicSettingIsRejected();
    testAtomicStoreAndReset(argv[1]);
    std::cout << "Save data tests passed\n";
    return 0;
}
