#pragma once

#include <array>
#include <istream>
#include <string>

struct NarrativeCard {
    std::string id;
    std::string speaker;
    std::string text;

    [[nodiscard]] bool complete() const {
        return !id.empty() && !speaker.empty() && !text.empty();
    }
};

struct MissionNarrative {
    std::string missionId;
    std::array<NarrativeCard, 2U> briefing{};
    NarrativeCard mechanic{};
    NarrativeCard victory{};
    NarrativeCard defeat{};

    [[nodiscard]] bool complete() const {
        return !missionId.empty() && briefing[0].complete() && briefing[1].complete() &&
            mechanic.complete() && victory.complete() && defeat.complete();
    }
};

struct NarrativeLoadResult {
    bool success = false;
    MissionNarrative narrative{};
    std::string error;
};

class NarrativeLoader {
public:
    [[nodiscard]] static NarrativeLoadResult parse(std::istream& input);
    [[nodiscard]] static NarrativeLoadResult load(const char* path);
    [[nodiscard]] static std::string pathFor(const char* language, const char* missionId);
};
