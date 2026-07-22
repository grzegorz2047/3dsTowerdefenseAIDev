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

void testVersionTwoRoundTrip() {
    SaveData source{};
    source.campaign.unlockedCount = 3;
    source.campaign.bestStars = {3, 2, 1, 0, 0, 0};
    source.campaign.bestBaseHealth = {5, 4, 2, 0, 0, 0};
    source.campaign.fewestTowers = {2, 4, 7, 0, 0, 0};
    source.settings.soundEnabled = false;
    source.settings.preferredSpeed = 2;

    const SaveLoadResult result = SaveDataCodec::deserialize(SaveDataCodec::serialize(source));
    expect(result.status == SaveLoadStatus::Loaded, "v2 save should deserialize");
    expect(!result.migrated, "v2 save should not report migration");
    expect(result.data.campaign.unlockedCount == 3, "unlocked count should round-trip");
    expect(result.data.campaign.bestStars[1] == 2, "stars should round-trip");
    expect(result.data.campaign.bestBaseHealth[0] == 5, "base health should round-trip");
    expect(result.data.campaign.fewestTowers[1] == 4, "tower record should round-trip");
    expect(!result.data.settings.soundEnabled, "sound setting should round-trip");
    expect(result.data.settings.preferredSpeed == 2, "speed setting should round-trip");
}

void testVersionOneMigration() {
    const std::string payload =
        "version=1\n"
        "unlocked=2\n"
        "stars=3,1,0,0,0,0\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Loaded, "v1 save should load");
    expect(result.migrated, "v1 save should report migration");
    expect(result.data.campaign.unlockedCount == 2, "v1 progress should migrate");
    expect(result.data.campaign.bestStars[0] == 3, "v1 stars should migrate");
    expect(result.data.campaign.bestBaseHealth[0] == 0, "new records should use defaults");
    expect(result.data.settings.soundEnabled, "new settings should use defaults");
    expect(result.data.settings.preferredSpeed == 1, "new speed should use default");
}

void testCorruptionIsRejected() {
    SaveData data{};
    std::string serialized = SaveDataCodec::serialize(data);
    serialized[serialized.find("stars=") + 6] = '9';
    const SaveLoadResult result = SaveDataCodec::deserialize(serialized);
    expect(result.status == SaveLoadStatus::Corrupt, "checksum mismatch should be corrupt");
}

void testInconsistentProgressIsRejected() {
    const std::string payload =
        "version=2\n"
        "unlocked=1\n"
        "stars=3,1,0,0,0,0\n"
        "base_health=5,4,0,0,0,0\n"
        "fewest_towers=2,3,0,0,0,0\n"
        "sound=1\n"
        "speed=1\n";
    const SaveLoadResult result = SaveDataCodec::deserialize(withChecksum(payload));
    expect(result.status == SaveLoadStatus::Corrupt, "locked mission progress should be rejected");
}

void testAtomicStoreAndReset(const char* root) {
    const std::string path = std::string(root) + "/build/host-tests/save-data-test.sav";
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());

    SaveData source{};
    source.campaign.unlockedCount = 2;
    source.campaign.bestStars = {3, 1, 0, 0, 0, 0};
    source.campaign.bestBaseHealth = {5, 2, 0, 0, 0, 0};
    source.campaign.fewestTowers = {2, 8, 0, 0, 0, 0};

    std::string error;
    expect(SaveDataStore::saveAtomically(path.c_str(), source, error), "atomic save should succeed");
    expect(SaveDataStore::load(path.c_str()).status == SaveLoadStatus::Loaded, "stored save should load");
    std::ifstream temporary(path + ".tmp");
    expect(!temporary.good(), "temporary file should not remain after commit");
    expect(SaveDataStore::reset(path.c_str()), "reset should remove save");
    expect(SaveDataStore::load(path.c_str()).status == SaveLoadStatus::Missing, "reset save should be missing");
}

}  // namespace

int main(int argc, char** argv) {
    expect(argc == 2, "repository root argument is required");
    testVersionTwoRoundTrip();
    testVersionOneMigration();
    testCorruptionIsRejected();
    testInconsistentProgressIsRejected();
    testAtomicStoreAndReset(argv[1]);
    std::cout << "Save data tests passed\n";
    return 0;
}
