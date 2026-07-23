#include "Narrative.hpp"

#include <fstream>
#include <map>
#include <sstream>

namespace {

std::string trim(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
}

bool parseCard(const std::map<std::string, std::string>& values, const char* prefix, NarrativeCard& card) {
    const std::string root(prefix);
    const auto id = values.find(root + ".id");
    const auto speaker = values.find(root + ".speaker");
    const auto text = values.find(root + ".text");
    if (id == values.end() || speaker == values.end() || text == values.end()) return false;
    card = {id->second, speaker->second, text->second};
    return card.complete();
}

bool stableId(const std::string& value) {
    if (value.empty()) return false;
    for (const unsigned char ch : value) {
        const bool valid = (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '.';
        if (!valid) return false;
    }
    return true;
}

}  // namespace

NarrativeLoadResult NarrativeLoader::parse(std::istream& input) {
    std::map<std::string, std::string> values;
    std::string line;
    std::size_t lineNumber = 0U;
    while (std::getline(input, line)) {
        ++lineNumber;
        const std::string clean = trim(line);
        if (clean.empty() || clean[0] == '#') continue;
        const std::size_t separator = clean.find('=');
        if (separator == std::string::npos || separator == 0U || separator + 1U >= clean.size()) {
            return {false, {}, "Nieprawidlowy wiersz narracji: " + std::to_string(lineNumber)};
        }
        const std::string key = trim(clean.substr(0U, separator));
        const std::string value = trim(clean.substr(separator + 1U));
        if (key.empty() || value.empty() || values.count(key) != 0U) {
            return {false, {}, "Powtorzony lub pusty klucz narracji"};
        }
        values.emplace(key, value);
    }

    MissionNarrative narrative{};
    const auto mission = values.find("mission.id");
    if (mission == values.end() || !stableId(mission->second)) {
        return {false, {}, "Brak stabilnego mission.id"};
    }
    narrative.missionId = mission->second;
    if (!parseCard(values, "briefing.1", narrative.briefing[0]) ||
        !parseCard(values, "briefing.2", narrative.briefing[1]) ||
        !parseCard(values, "mechanic", narrative.mechanic) ||
        !parseCard(values, "victory", narrative.victory) ||
        !parseCard(values, "defeat", narrative.defeat)) {
        return {false, {}, "Niekompletny zestaw kart narracji"};
    }

    const std::array<const NarrativeCard*, 5U> cards{
        &narrative.briefing[0], &narrative.briefing[1], &narrative.mechanic,
        &narrative.victory, &narrative.defeat};
    for (const NarrativeCard* card : cards) {
        if (!stableId(card->id)) return {false, {}, "Niestabilny identyfikator karty narracji"};
    }
    return {true, narrative, {}};
}

NarrativeLoadResult NarrativeLoader::load(const char* path) {
    if (path == nullptr || path[0] == '\0') return {false, {}, "Brak sciezki narracji"};
    std::ifstream input(path);
    if (!input.is_open()) return {false, {}, "Nie mozna otworzyc narracji"};
    return parse(input);
}

std::string NarrativeLoader::pathFor(const char* language, const char* missionId) {
    const std::string safeLanguage = language != nullptr && language[0] != '\0' ? language : "pl";
    const std::string safeMission = missionId != nullptr ? missionId : "";
    return "romfs:/narrative/" + safeLanguage + "/" + safeMission + ".txt";
}
