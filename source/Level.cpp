#include "Level.hpp"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <limits>
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

bool parseLong(const std::string& value, long& output) {
    if (value.empty()) {
        return false;
    }

    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (errno == ERANGE || end == value.c_str() || *end != '\0') {
        return false;
    }

    output = parsed;
    return true;
}

bool parseDimensions(const std::string& value, std::uint8_t& width, std::uint8_t& height) {
    const std::size_t separator = value.find(',');
    if (separator == std::string::npos || value.find(',', separator + 1) != std::string::npos) {
        return false;
    }

    long parsedWidth = 0;
    long parsedHeight = 0;
    if (!parseLong(value.substr(0, separator), parsedWidth) ||
        !parseLong(value.substr(separator + 1), parsedHeight)) {
        return false;
    }

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
    if (separator == std::string::npos || value.find(',', separator + 1) != std::string::npos) {
        return false;
    }

    long x = 0;
    long z = 0;
    if (!parseLong(value.substr(0, separator), x) ||
        !parseLong(value.substr(separator + 1), z)) {
        return false;
    }

    if (x < std::numeric_limits<std::int16_t>::min() ||
        x > std::numeric_limits<std::int16_t>::max() ||
        z < std::numeric_limits<std::int16_t>::min() ||
        z > std::numeric_limits<std::int16_t>::max()) {
        return false;
    }

    point.x = static_cast<std::int16_t>(x);
    point.z = static_cast<std::int16_t>(z);
    return true;
}

bool isPathTile(TileType tile) {
    return tile == TileType::Road || tile == TileType::Spawn || tile == TileType::Base;
}

bool validatePath(const LevelData& level, std::string& error) {
    if (level.pathLength < 2) {
        error = "Trasa musi miec co najmniej dwa punkty";
        return false;
    }

    for (std::size_t index = 0; index < level.pathLength; ++index) {
        const GridPoint point = level.path[index];
        if (point.x < 0 || point.z < 0 ||
            point.x >= static_cast<std::int16_t>(level.width) ||
            point.z >= static_cast<std::int16_t>(level.height)) {
            error = "Punkt trasy znajduje sie poza mapa";
            return false;
        }

        const TileType tile = level.tileAt(
            static_cast<std::size_t>(point.x),
            static_cast<std::size_t>(point.z));
        if (!isPathTile(tile)) {
            error = "Trasa moze przebiegac tylko przez S, R i E";
            return false;
        }

        if (index > 0) {
            const GridPoint previous = level.path[index - 1];
            const int distance = std::abs(static_cast<int>(point.x - previous.x)) +
                std::abs(static_cast<int>(point.z - previous.z));
            if (distance != 1) {
                error = "Kolejne punkty trasy musza byc sasiednie";
                return false;
            }
        }
    }

    const GridPoint first = level.path[0];
    const GridPoint last = level.path[level.pathLength - 1];
    if (level.tileAt(static_cast<std::size_t>(first.x), static_cast<std::size_t>(first.z)) != TileType::Spawn ||
        level.tileAt(static_cast<std::size_t>(last.x), static_cast<std::size_t>(last.z)) != TileType::Base) {
        error = "Trasa musi zaczynac sie w S i konczyc w E";
        return false;
    }

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
    std::size_t spawnCount = 0;
    std::size_t baseCount = 0;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
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
                spawnCount += tile == TileType::Spawn ? 1U : 0U;
                baseCount += tile == TileType::Base ? 1U : 0U;
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
            if (readingGrid || gridRow != result.level.height) {
                result.error = "Trasa musi wystapic po kompletnej siatce";
                return result;
            }
            std::stringstream points(value);
            std::string pointValue;
            while (std::getline(points, pointValue, ';')) {
                if (pointValue.empty() || result.level.pathLength >= kMaximumPathPoints ||
                    !parsePoint(pointValue, result.level.path[result.level.pathLength])) {
                    result.error = "Nieprawidlowa trasa";
                    return result;
                }
                ++result.level.pathLength;
            }
        } else {
            result.error = "Nieznany klucz poziomu";
            return result;
        }
    }

    if (readingGrid || gridRow != result.level.height) {
        result.error = "Niekompletna siatka poziomu";
        return result;
    }
    if (result.level.id.empty() || result.level.name.empty()) {
        result.error = "Brak identyfikatora lub nazwy poziomu";
        return result;
    }
    if (spawnCount != 1 || baseCount != 1) {
        result.error = "Poziom musi zawierac dokladnie jeden spawn i jedna baze";
        return result;
    }
    if (!validatePath(result.level, result.error)) {
        return result;
    }

    result.success = true;
    return result;
}
