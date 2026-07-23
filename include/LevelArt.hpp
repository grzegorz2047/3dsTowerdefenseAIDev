#pragma once

#include <cstddef>
#include <cstdint>

constexpr std::size_t kMaximumSceneProps = 48U;

enum class LevelTheme : std::uint8_t {
    Default,
    VhalPass,
};

enum class ScenePropType : std::uint8_t {
    Pine,
    Rock,
    Ruin,
    Banner,
    Barricade,
    Wagon,
    Cliff,
    Watchtower,
};

struct SceneProp {
    ScenePropType type = ScenePropType::Rock;
    float gridX = 0.0F;
    float gridZ = 0.0F;
    float scale = 1.0F;
    float rotationDegrees = 0.0F;
};

[[nodiscard]] constexpr const char* scenePropTypeName(ScenePropType type) {
    switch (type) {
        case ScenePropType::Pine: return "Pine";
        case ScenePropType::Rock: return "Rock";
        case ScenePropType::Ruin: return "Ruin";
        case ScenePropType::Banner: return "Banner";
        case ScenePropType::Barricade: return "Barricade";
        case ScenePropType::Wagon: return "Wagon";
        case ScenePropType::Cliff: return "Cliff";
        case ScenePropType::Watchtower: return "Watchtower";
    }
    return "Rock";
}
