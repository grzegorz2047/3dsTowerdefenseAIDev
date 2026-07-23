#pragma once

#include <array>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "LevelArt.hpp"

struct SceneArt {
    std::string levelId;
    LevelTheme theme = LevelTheme::Default;
    std::array<SceneProp, kMaximumSceneProps> props{};
    std::size_t propCount = 0U;
};

struct SceneArtLoadResult {
    bool success = false;
    SceneArt art{};
    std::string error;
};

class SceneArtLoader {
public:
    [[nodiscard]] static SceneArtLoadResult parse(std::istream& input) {
        SceneArtLoadResult result{};
        std::string line;
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == ';') continue;
            const std::size_t equals = line.find('=');
            if (equals == std::string::npos) return failure("Nieprawidlowy wpis sceny");
            const std::string key = line.substr(0U, equals);
            const std::string value = line.substr(equals + 1U);
            if (key == "level") result.art.levelId = value;
            else if (key == "theme") {
                if (value == "VhalPass") result.art.theme = LevelTheme::VhalPass;
                else if (value == "Default") result.art.theme = LevelTheme::Default;
                else return failure("Nieznany motyw sceny");
            } else if (key == "prop") {
                if (result.art.propCount >= kMaximumSceneProps) return failure("Za duzo dekoracji");
                SceneProp prop{};
                if (!parseProp(value, prop)) return failure("Nieprawidlowa dekoracja");
                result.art.props[result.art.propCount++] = prop;
            } else return failure("Nieznany klucz sceny");
        }
        if (result.art.levelId.empty()) return failure("Brak identyfikatora sceny");
        result.success = true;
        return result;
    }

    [[nodiscard]] static SceneArtLoadResult load(const char* path) {
        std::ifstream input(path);
        if (!input.is_open()) return failure("Nie mozna otworzyc sceny");
        return parse(input);
    }

    [[nodiscard]] static std::string pathFor(const char* levelId) {
        return std::string("romfs:/scenes/") + (levelId != nullptr ? levelId : "") + ".art";
    }

private:
    [[nodiscard]] static SceneArtLoadResult failure(const char* message) {
        SceneArtLoadResult result{};
        result.error = message;
        return result;
    }

    [[nodiscard]] static bool parseFloat(const std::string& text, float& value) {
        if (text.empty()) return false;
        errno = 0;
        char* end = nullptr;
        value = std::strtof(text.c_str(), &end);
        return errno != ERANGE && end != text.c_str() && *end == '\0';
    }

    [[nodiscard]] static bool parseType(const std::string& text, ScenePropType& type) {
        for (int raw = 0; raw <= static_cast<int>(ScenePropType::Watchtower); ++raw) {
            const auto candidate = static_cast<ScenePropType>(raw);
            if (text == scenePropTypeName(candidate)) { type = candidate; return true; }
        }
        return false;
    }

    [[nodiscard]] static bool parseProp(const std::string& value, SceneProp& prop) {
        std::array<std::string, 5U> parts{};
        std::stringstream stream(value);
        for (std::size_t index = 0U; index < parts.size(); ++index) {
            if (!std::getline(stream, parts[index], ',')) return false;
        }
        std::string extra;
        if (std::getline(stream, extra, ',')) return false;
        if (!parseType(parts[0], prop.type) || !parseFloat(parts[1], prop.gridX) ||
            !parseFloat(parts[2], prop.gridZ) || !parseFloat(parts[3], prop.scale) ||
            !parseFloat(parts[4], prop.rotationDegrees)) return false;
        return prop.gridX >= -4.0F && prop.gridX <= 20.0F && prop.gridZ >= -4.0F &&
            prop.gridZ <= 20.0F && prop.scale >= 0.25F && prop.scale <= 3.0F &&
            prop.rotationDegrees >= -360.0F && prop.rotationDegrees <= 360.0F;
    }
};
