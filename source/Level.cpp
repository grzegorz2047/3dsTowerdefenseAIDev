#include "Level.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace {

TileType parseTile(char value, bool& valid) {
    valid = true;
    switch (value) {
        case '.': return TileType::Ground;
        case 'R': return TileType::Road;
        case 'B': return TileType::BuildSpot;
        case '#': return TileType::Blocked;
        case 'S': return TileType::Spawn;
        case 'E': return TileType::Base;
        default:
            valid = false;
            return TileType::Ground;
    }
}

bool parseDimensions(const std::string& value, std::uint8_t& width, std::uint8_t& height) {
    const std::size_t separator = value.find(',');
    if (separator == std::string::npos) {
        return false;
    }
    const long parsedWidth = std::strtol(value.substr(0, separator).c_str(), nullptr, 10);
    const long parsedHeight = std::strtol(value.substr(separator + 1).c_str(), nullptr, 10);
    if (parsedWidth <= 0 || parsedHeight <= 0 ||
        parsedWidth > static_cast<long>(kMaximumMapWidth) ||
        parsedHeight > static_cast<long>(kMaximumMapHeight)) {
        return false;
    }
    width = static_cast<std::uint8_t>(parsedWidth);
    height = static_cast<std::uint8_t>(parsedHeight);
    return true;
}

bool parsePoint(const std::string& value, GridPoint& point) {
    const std::size_t separator = value.find(',');
    if (separator == std::string::npos) {
        return false;
    }
    point.x = static_cast<std::int16_t>(std::strtol(value.substr(0, separator).c_str(), nullptr, 10));
    point.z = static_cast<std::int16_t>(std::strtol(value.substr(separator + 1).c_str(), nullptr, 10));
    return true;
}

}  // namespace

TileType LevelData::tileAt(std::size_t x, std::size_t z) const {
    if (x >= width || z >= height) {
        return TileType::Blocked;
    }
    return tiles[z * kMaximumMapWidth + x];
}

LevelLoadResult LevelLoader::loadFromRomFs(const char* path) {
    LevelLoadResult result{};
    std::ifstream input(path);
    if (!input.is_open()) {
        result.error = "Nie mozna otworzyc pliku poziomu";
        return result;
    }

    std::string line;
    bool readingGrid = false;
    std::size_t gridRow = 0;
    bool hasSpawn = false;
    bool hasBase = false;

    while (std::getline(input, line)) {
        if (line.empty() || line[0] == ';') {
            continue;
        }

        if (readingGrid && gridRow < result.level.height) {
            if (line.size() != result.level.width) {
                result.error = "Wiersz siatki ma nieprawidlowa szerokosc";
                return result;
            }
            for (std::size_t x = 0; x < result.level.width; ++x) {
                bool valid = false;
                const TileType tile = parseTile(line[x], valid);
                if (!valid) {
                    result.error = "Nieznany znak pola mapy";
                    return result;
                }
                result.level.tiles[gridRow * kMaximumMapWidth + x] = tile;
                hasSpawn = hasSpawn || tile == TileType::Spawn;
                hasBase = hasBase || tile == TileType::Base;
            }
            ++gridRow;
            if (gridRow == result.level.height) {
                readingGrid = false;
            }
            continue;
        }

        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            result.error = "Nieprawidlowy wpis poziomu";
            return result;
        }
        const std::string key = line.substr(0, equals);
        const std::string value = line.substr(equals + 1);

        if (key == "id") {
            result.level.id = value;
        } else if (key == "name") {
            result.level.name = value;
        } else if (key == "size") {
            if (!parseDimensions(value, result.level.width, result.level.height)) {
                result.error = "Nieprawidlowy rozmiar poziomu";
                return result;
            }
        } else if (key == "grid") {
            if (value != "begin" || result.level.width == 0 || result.level.height == 0) {
                result.error = "Siatka musi wystapic po rozmiarze";
                return result;
            }
            readingGrid = true;
            gridRow = 0;
        } else if (key == "path") {
            std::stringstream points(value);
            std::string pointValue;
            while (std::getline(points, pointValue, ';')) {
                if (result.level.pathLength >= kMaximumPathPoints ||
                    !parsePoint(pointValue, result.level.path[result.level.pathLength])) {
                    result.error = "Nieprawidlowa trasa";
                    return result;
                }
                ++result.level.pathLength;
            }
        }
    }

    if (readingGrid || gridRow != result.level.height) {
        result.error = "Niekompletna siatka poziomu";
        return result;
    }
    if (result.level.id.empty() || result.level.name.empty() || !hasSpawn || !hasBase) {
        result.error = "Brak wymaganych danych poziomu";
        return result;
    }
    if (result.level.pathLength < 2) {
        result.error = "Trasa musi miec co najmniej dwa punkty";
        return result;
    }

    const GridPoint first = result.level.path[0];
    const GridPoint last = result.level.path[result.level.pathLength - 1];
    if (result.level.tileAt(first.x, first.z) != TileType::Spawn ||
        result.level.tileAt(last.x, last.z) != TileType::Base) {
        result.error = "Trasa musi zaczynac sie w S i konczyc w E";
        return result;
    }

    result.success = true;
    return result;
}
