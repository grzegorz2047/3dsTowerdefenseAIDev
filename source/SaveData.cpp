#include "SaveData.hpp"

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace {

std::uint32_t fnv1a(const std::string& value) {
    std::uint32_t hash = 2166136261U;
    for (const unsigned char byte : value) {
        hash ^= byte;
        hash *= 16777619U;
    }
    return hash;
}

std::string joinArray(const std::array<std::uint8_t, kCampaignMissionCount>& values) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) stream << ',';
        stream << static_cast<unsigned int>(values[index]);
    }
    return stream.str();
}

bool parseUnsigned(const std::string& text, unsigned long& value) {
    if (text.empty()) return false;
    errno = 0;
    char* end = nullptr;
    value = std::strtoul(text.c_str(), &end, 10);
    return errno != ERANGE && end != text.c_str() && *end == '\0';
}

bool parseArray(const std::string& text, std::array<std::uint8_t, kCampaignMissionCount>& output) {
    std::stringstream stream(text);
    std::string token;
    std::size_t index = 0;
    while (std::getline(stream, token, ',')) {
        if (index >= output.size()) return false;
        unsigned long value = 0;
        if (!parseUnsigned(token, value) || value > 255UL) return false;
        output[index++] = static_cast<std::uint8_t>(value);
    }
    return index == output.size();
}

std::string payloadFor(const SaveData& data, std::uint32_t version) {
    std::ostringstream stream;
    stream << "version=" << version << '\n';
    stream << "unlocked=" << data.campaign.unlockedCount << '\n';
    stream << "stars=" << joinArray(data.campaign.bestStars) << '\n';
    if (version >= 2) {
        stream << "base_health=" << joinArray(data.campaign.bestBaseHealth) << '\n';
        stream << "fewest_towers=" << joinArray(data.campaign.fewestTowers) << '\n';
        stream << "sound=" << (data.settings.soundEnabled ? 1 : 0) << '\n';
        stream << "speed=" << static_cast<unsigned int>(data.settings.preferredSpeed) << '\n';
    }
    return stream.str();
}

bool parsePayload(const std::string& payload, std::map<std::string, std::string>& values) {
    std::stringstream stream(payload);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos || separator == 0 || separator + 1 >= line.size()) return false;
        values[line.substr(0, separator)] = line.substr(separator + 1);
    }
    return true;
}

SaveLoadResult corrupt(const char* message) {
    SaveLoadResult result{};
    result.status = SaveLoadStatus::Corrupt;
    result.error = message;
    return result;
}

}  // namespace

std::string SaveDataCodec::serialize(const SaveData& data) {
    const std::string payload = payloadFor(data, kCurrentSaveVersion);
    std::ostringstream stream;
    stream << payload << "checksum=" << fnv1a(payload) << '\n';
    return stream.str();
}

SaveLoadResult SaveDataCodec::deserialize(const std::string& text) {
    const std::size_t checksumPosition = text.rfind("checksum=");
    if (checksumPosition == std::string::npos) return corrupt("Brak sumy kontrolnej");

    const std::string payload = text.substr(0, checksumPosition);
    const std::size_t checksumEnd = text.find('\n', checksumPosition);
    const std::string checksumText = text.substr(checksumPosition + 9, checksumEnd - (checksumPosition + 9));
    unsigned long storedChecksum = 0;
    if (!parseUnsigned(checksumText, storedChecksum) || storedChecksum != fnv1a(payload)) {
        return corrupt("Nieprawidlowa suma kontrolna");
    }

    std::map<std::string, std::string> values;
    if (!parsePayload(payload, values)) return corrupt("Nieprawidlowy format zapisu");

    unsigned long version = 0;
    unsigned long unlocked = 0;
    if (!parseUnsigned(values["version"], version) || !parseUnsigned(values["unlocked"], unlocked)) {
        return corrupt("Brak wersji lub postepu");
    }
    if (version == 0 || version > kCurrentSaveVersion) {
        SaveLoadResult result{};
        result.status = SaveLoadStatus::UnsupportedVersion;
        result.error = "Nieobslugiwana wersja zapisu";
        return result;
    }

    SaveData data{};
    data.campaign.unlockedCount = static_cast<std::size_t>(unlocked);
    if (!parseArray(values["stars"], data.campaign.bestStars)) return corrupt("Nieprawidlowe gwiazdki");

    bool migrated = false;
    if (version == 1) {
        migrated = true;
    } else {
        if (!parseArray(values["base_health"], data.campaign.bestBaseHealth) ||
            !parseArray(values["fewest_towers"], data.campaign.fewestTowers)) {
            return corrupt("Nieprawidlowe rekordy misji");
        }
        unsigned long sound = 0;
        unsigned long speed = 0;
        if (!parseUnsigned(values["sound"], sound) || sound > 1 ||
            !parseUnsigned(values["speed"], speed) || (speed != 1 && speed != 2)) {
            return corrupt("Nieprawidlowe ustawienia");
        }
        data.settings.soundEnabled = sound == 1;
        data.settings.preferredSpeed = static_cast<std::uint8_t>(speed);
    }

    CampaignProgress validator;
    if (!validator.restore(data.campaign)) return corrupt("Niespojny postep kampanii");

    SaveLoadResult result{};
    result.status = SaveLoadStatus::Loaded;
    result.data = data;
    result.migrated = migrated;
    return result;
}

SaveLoadResult SaveDataStore::load(const char* path) {
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        SaveLoadResult result{};
        result.status = SaveLoadStatus::Missing;
        return result;
    }
    std::ostringstream content;
    content << input.rdbuf();
    return SaveDataCodec::deserialize(content.str());
}

bool SaveDataStore::saveAtomically(const char* path, const SaveData& data, std::string& error) {
    const std::string temporaryPath = std::string(path) + ".tmp";
    {
        std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
        if (!output.is_open()) {
            error = "Nie mozna otworzyc pliku tymczasowego";
            return false;
        }
        const std::string serialized = SaveDataCodec::serialize(data);
        output.write(serialized.data(), static_cast<std::streamsize>(serialized.size()));
        output.flush();
        if (!output.good()) {
            error = "Nie mozna zapisac pliku tymczasowego";
            output.close();
            std::remove(temporaryPath.c_str());
            return false;
        }
    }

    std::remove(path);
    if (std::rename(temporaryPath.c_str(), path) != 0) {
        error = "Nie mozna podmienic pliku zapisu";
        std::remove(temporaryPath.c_str());
        return false;
    }
    return true;
}

bool SaveDataStore::reset(const char* path) {
    const std::string temporaryPath = std::string(path) + ".tmp";
    std::remove(temporaryPath.c_str());
    return std::remove(path) == 0 || errno == ENOENT;
}
