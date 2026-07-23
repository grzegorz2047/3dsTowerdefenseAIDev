#pragma once

#include <cstdint>
#include <string>

#include "Campaign.hpp"
#include "Stereo3D.hpp"

constexpr std::uint32_t kCurrentSaveVersion = 5;

struct GameSettings {
    bool soundEnabled = true;
    bool musicEnabled = true;
    std::uint8_t preferredSpeed = 1;
    bool stereoEnabled = true;
    std::uint8_t maximum3DDepthPercent = Stereo3D::kDefaultDepthPercent;
};

struct SaveData {
    CampaignProgressSnapshot campaign{};
    GameSettings settings{};
};

enum class SaveLoadStatus {
    Loaded,
    Missing,
    Corrupt,
    UnsupportedVersion,
};

struct SaveLoadResult {
    SaveLoadStatus status = SaveLoadStatus::Missing;
    SaveData data{};
    bool migrated = false;
    std::string error;
};

class SaveDataCodec {
public:
    [[nodiscard]] static std::string serialize(const SaveData& data);
    [[nodiscard]] static SaveLoadResult deserialize(const std::string& text);
};

class SaveDataStore {
public:
    [[nodiscard]] static SaveLoadResult load(const char* path);
    [[nodiscard]] static bool saveAtomically(const char* path, const SaveData& data, std::string& error);
    [[nodiscard]] static bool reset(const char* path);
};
