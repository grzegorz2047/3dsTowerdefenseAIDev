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
    if (value.empty()) return false;
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (errno == ERANGE || end == value.c_str() || *end != '\0') return false;
    output = parsed;
    return true;
}

bool parseFloat(const std::string& value, float& output) {
    if (value.empty()) return false;
    errno = 0;
    char* end = nullptr;
    const float parsed = std::strtof(value.c_str(), &end);
    if (errno == ERANGE || end == value.c_str() || *end != '\0') return false;
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
        !parseLong(value.substr(separator + 1), parsedHeight)) return false;
    if (parsedWidth <= 0 || parsedHeight <= 0 ||
        parsedWidth > static_cast<long>(kMaximumMapWidth) ||
        parsedHeight > static_cast<long>(kMaximumMapHeight)) return false;
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
        !parseLong(value.substr(separator + 1), z)) return false;
    if (x < std::numeric_limits<std::int16_t>::min() ||
        x > std::numeric_limits<std::int16_t>::max() ||
        z < std::numeric_limits<std::int16_t>::min() ||
        z > std::numeric_limits<std::int16_t>::max()) return false;
    point.x = static_cast<std::int16_t>(x);
    point.z = static_cast<std::int16_t>(z);
    return true;
}

bool parseEnemyType(const std::string& value, EnemyType& type) {
    if (value == "Scout") { type = EnemyType::Scout; return true; }
    if (value == "Raider") { type = EnemyType::Raider; return true; }
    if (value == "Brute") { type = EnemyType::Brute; return true; }
    return false;
}

bool parseWaveEntry(const std::string& value, WaveEntry& entry) {
    const std::size_t first = value.find(',');
    const std::size_t second = first == std::string::npos ? std::string::npos : value.find(',', first + 1);
    if (first == std::string::npos || second == std::string::npos ||
        value.find(',', second + 1) != std::string::npos) return false;
    long count = 0;
    float interval = 0.0F;
    if (!parseEnemyType(value.substr(0, first), entry.type) ||
        !parseLong(value.substr(first + 1, second - first - 1), count) ||
        !parseFloat(value.substr(second + 1), interval)) return false;
    if (count <= 0 || count > static_cast<long>(kMaximumWaveEnemies) ||
        interval < 0.1F || interval > 10.0F) return false;
    entry.count = static_cast<std::uint8_t>(count);
    entry.spawnIntervalSeconds = interval;
    return true;
}

bool isPathTile(TileType tile) {
    return tile == TileType::Road || tile == TileType::Spawn || tile == TileType::Base;
}

bool parsePathIndex(const std::string& key, std::size_t& index) {
    if (key == "path") {
        index = 0U;
        return true;
    }
    if (key.size() != 5U || key.rfind("path", 0U) != 0U || key[4] < '1' || key[4] > '3') {
        return false;
    }
    index = static_cast<std::size_t>(key[4] - '0');
    return index < kMaximumPaths;
}

bool parsePathValue(LevelData& level, std::size_t pathIndex, const std::string& value) {
    GridPoint* destination = pathIndex == 0U
        ? level.path.data()
        : level.additionalPaths[pathIndex - 1U].data();
    std::size_t& length = pathIndex == 0U
        ? level.pathLength
        : level.additionalPathLengths[pathIndex - 1U];
    if (length != 0U) return false;
    std::stringstream points(value);
    std::string pointValue;
    while (std::getline(points, pointValue, ';')) {
        if (pointValue.empty() || length >= kMaximumPathPoints ||
            !parsePoint(pointValue, destination[length])) {
            return false;
        }
        ++length;
    }
    return length > 0U;
}

bool validateSinglePath(const LevelData& level, std::size_t pathIndex,
    const GridPoint& expectedBase, std::string& error) {
    const GridPoint* points = level.pathData(pathIndex);
    const std::size_t length = level.pathLengthAt(pathIndex);
    if (points == nullptr || length < 2U) {
        error = "Kazda trasa musi miec co najmniej dwa punkty";
        return false;
    }
    for (std::size_t index = 0U; index < length; ++index) {
        const GridPoint point = points[index];
        if (point.x < 0 || point.z < 0 ||
            point.x >= static_cast<std::int16_t>(level.width) ||
            point.z >= static_cast<std::int16_t>(level.height)) {
            error = "Punkt trasy znajduje sie poza mapa";
            return false;
        }
        const TileType tile = level.tileAt(static_cast<std::size_t>(point.x),
            static_cast<std::size_t>(point.z));
        if (!isPathTile(tile)) {
            error = "Trasa moze przebiegac tylko przez S, R i E";
            return false;
        }
        if (index > 0U) {
            const GridPoint previous = points[index - 1U];
            const int distance = std::abs(static_cast<int>(point.x - previous.x)) +
                std::abs(static_cast<int>(point.z - previous.z));
            if (distance != 1) {
                error = "Kolejne punkty trasy musza byc sasiednie";
                return false;
            }
        }
    }
    const GridPoint first = points[0];
    const GridPoint last = points[length - 1U];
    if (level.tileAt(static_cast<std::size_t>(first.x), static_cast<std::size_t>(first.z)) != TileType::Spawn ||
        level.tileAt(static_cast<std::size_t>(last.x), static_cast<std::size_t>(last.z)) != TileType::Base) {
        error = "Kazda trasa musi zaczynac sie w S i konczyc w E";
        return false;
    }
    if (last.x != expectedBase.x || last.z != expectedBase.z) {
        error = "Wszystkie trasy musza prowadzic do wspolnej bazy";
        return false;
    }
    return true;
}

bool validatePaths(LevelData& level, std::size_t spawnCount, std::string& error) {
    level.pathCount = level.pathLength > 0U ? 1U : 0U;
    for (std::size_t index = 0U; index < level.additionalPathLengths.size(); ++index) {
        if (level.additionalPathLengths[index] == 0U) continue;
        if (index + 1U != level.pathCount) {
            error = "Dodatkowe trasy musza byc numerowane kolejno";
            return false;
        }
        ++level.pathCount;
    }
    if (level.pathCount == 0U || spawnCount != level.pathCount) {
        error = "Liczba pol S musi odpowiadac liczbie tras";
        return false;
    }
    const GridPoint base = level.path[level.pathLength - 1U];
    for (std::size_t index = 0U; index < level.pathCount; ++index) {
        if (!validateSinglePath(level, index, base, error)) return false;
    }
    return true;
}

}  // namespace

TileType LevelData::tileAt(std::size_t x, std::size_t z) const {
    if (x >= width || z >= height) return TileType::Blocked;
    return tiles[z * kMaximumMapWidth + x];
}

const GridPoint* LevelData::pathData(std::size_t index) const {
    if (index >= pathCount) return nullptr;
    return index == 0U ? path.data() : additionalPaths[index - 1U].data();
}

std::size_t LevelData::pathLengthAt(std::size_t index) const {
    if (index >= pathCount) return 0U;
    return index == 0U ? pathLength : additionalPathLengths[index - 1U];
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
    std::size_t gridRow = 0U;
    std::size_t spawnCount = 0U;
    std::size_t baseCount = 0U;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == ';') continue;

        if (readingGrid && gridRow < result.level.height) {
            if (line.size() != result.level.width) {
                result.error = "Wiersz siatki ma nieprawidlowa szerokosc";
                return result;
            }
            for (std::size_t x = 0U; x < result.level.width; ++x) {
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
            if (gridRow == result.level.height) readingGrid = false;
            continue;
        }

        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            result.error = "Nieprawidlowy wpis poziomu";
            return result;
        }
        const std::string key = line.substr(0U, equals);
        const std::string value = line.substr(equals + 1U);

        if (key == "id") result.level.id = value;
        else if (key == "name") result.level.name = value;
        else if (key == "size") {
            if (!parseDimensions(value, result.level.width, result.level.height)) {
                result.error = "Nieprawidlowy rozmiar poziomu";
                return result;
            }
        } else if (key == "grid") {
            if (value != "begin" || result.level.width == 0U || result.level.height == 0U) {
                result.error = "Siatka musi wystapic po rozmiarze";
                return result;
            }
            readingGrid = true;
            gridRow = 0U;
        } else {
            std::size_t pathIndex = 0U;
            if (parsePathIndex(key, pathIndex)) {
                if (readingGrid || gridRow != result.level.height ||
                    !parsePathValue(result.level, pathIndex, value)) {
                    result.error = "Nieprawidlowa trasa";
                    return result;
                }
            } else if (key == "waves") {
                if (result.level.pathLength == 0U) {
                    result.error = "Fale musza wystapic po trasie";
                    return result;
                }
                std::stringstream entries(value);
                std::string entryValue;
                while (std::getline(entries, entryValue, ';')) {
                    if (entryValue.empty() || result.level.waveEntryCount >= kMaximumWaveEntries) {
                        result.error = "Nieprawidlowa definicja fal";
                        return result;
                    }
                    WaveEntry entry{};
                    if (!parseWaveEntry(entryValue, entry) ||
                        result.level.totalEnemyCount + entry.count > kMaximumMissionEnemies) {
                        result.error = "Nieprawidlowa definicja fal";
                        return result;
                    }
                    result.level.waveEntries[result.level.waveEntryCount++] = entry;
                    result.level.totalEnemyCount += entry.count;
                }
            } else {
                result.error = "Nieznany klucz poziomu";
                return result;
            }
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
    if (baseCount != 1U) {
        result.error = "Poziom musi zawierac dokladnie jedna baze";
        return result;
    }
    if (!validatePaths(result.level, spawnCount, result.error)) return result;
    if (result.level.waveEntryCount == 0U || result.level.totalEnemyCount == 0U) {
        result.error = "Poziom musi definiowac co najmniej jedna fale";
        return result;
    }

    result.success = true;
    return result;
}
