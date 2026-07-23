#include "TouchUiLayout.hpp"

#include <array>

namespace {

constexpr TouchRect kBallista{8, 8, 72, 44};
constexpr TouchRect kMortar{85, 8, 72, 44};
constexpr TouchRect kFrost{162, 8, 72, 44};
constexpr TouchRect kRocket{239, 8, 72, 44};
constexpr TouchRect kCancel{240, 128, 72, 40};
constexpr TouchRect kPause{8, 128, 72, 40};
constexpr TouchRect kSpeed{88, 128, 72, 40};
constexpr TouchRect kBuild{8, 184, 72, 48};
constexpr TouchRect kUpgrade{88, 184, 72, 48};
constexpr TouchRect kSell{168, 184, 64, 48};
constexpr TouchRect kStart{240, 184, 72, 48};
constexpr TouchRect kResultCampaign{8, 190, 144, 42};
constexpr TouchRect kResultReplay{160, 190, 152, 42};

constexpr std::array<TouchRect, 6U> kCampaignRows{{
    {6, 36, 144, 24}, {6, 60, 144, 24}, {6, 84, 144, 24},
    {6, 108, 144, 24}, {6, 132, 144, 24}, {6, 156, 144, 24},
}};
constexpr TouchRect kCampaignPlay{6, 201, 58, 33};
constexpr TouchRect kCampaignBenchmark{68, 201, 58, 33};
constexpr TouchRect kCampaignSound{130, 201, 58, 33};
constexpr TouchRect kCampaignMusic{192, 201, 58, 33};
constexpr TouchRect kCampaignExit{254, 201, 60, 33};

constexpr TouchRect kNarrativeBack{8, 194, 96, 40};
constexpr TouchRect kNarrativeNext{112, 194, 96, 40};
constexpr TouchRect kNarrativeSkip{216, 194, 96, 40};

constexpr std::array<TouchRect, 6U> kBenchmarkDown{{
    {202, 34, 44, 24}, {202, 61, 44, 24}, {202, 88, 44, 24},
    {202, 115, 44, 24}, {202, 142, 44, 24}, {202, 169, 44, 24},
}};
constexpr std::array<TouchRect, 6U> kBenchmarkUp{{
    {252, 34, 60, 24}, {252, 61, 60, 24}, {252, 88, 60, 24},
    {252, 115, 60, 24}, {252, 142, 60, 24}, {252, 169, 60, 24},
}};
constexpr TouchRect kBenchmarkAutomatic{8, 199, 144, 34};
constexpr TouchRect kBenchmarkStart{160, 199, 72, 34};
constexpr TouchRect kBenchmarkBack{240, 199, 72, 34};

bool outside(std::int16_t x, std::int16_t y) {
    return x < 0 || y < 0 || x >= TouchUiLayout::kScreenWidth || y >= TouchUiLayout::kScreenHeight;
}

}  // namespace

TouchUiAction TouchUiLayout::actionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    const std::array<std::pair<TouchRect, TouchUiAction>, 11U> actions{{
        {kBallista, TouchUiAction::SelectBallista}, {kMortar, TouchUiAction::SelectMortar},
        {kFrost, TouchUiAction::SelectFrost}, {kRocket, TouchUiAction::SelectRocket},
        {kPause, TouchUiAction::TogglePause}, {kSpeed, TouchUiAction::ToggleSpeed},
        {kCancel, TouchUiAction::Cancel}, {kBuild, TouchUiAction::BuildOrSelect},
        {kUpgrade, TouchUiAction::Upgrade}, {kSell, TouchUiAction::Sell},
        {kStart, TouchUiAction::StartWave},
    }};
    for (const auto& entry : actions) if (entry.first.contains(x, y)) return entry.second;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::campaignActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    constexpr std::array<TouchUiAction, 6U> rows{
        TouchUiAction::CampaignRow0, TouchUiAction::CampaignRow1, TouchUiAction::CampaignRow2,
        TouchUiAction::CampaignRow3, TouchUiAction::CampaignRow4, TouchUiAction::CampaignRow5};
    for (std::size_t index = 0U; index < rows.size(); ++index) {
        if (kCampaignRows[index].contains(x, y)) return rows[index];
    }
    if (kCampaignPlay.contains(x, y)) return TouchUiAction::CampaignPlay;
    if (kCampaignBenchmark.contains(x, y)) return TouchUiAction::CampaignBenchmark;
    if (kCampaignSound.contains(x, y)) return TouchUiAction::ToggleSound;
    if (kCampaignMusic.contains(x, y)) return TouchUiAction::ToggleMusic;
    if (kCampaignExit.contains(x, y)) return TouchUiAction::Exit;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::narrativeActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    if (kNarrativeBack.contains(x, y)) return TouchUiAction::NarrativeBack;
    if (kNarrativeNext.contains(x, y)) return TouchUiAction::NarrativeNext;
    if (kNarrativeSkip.contains(x, y)) return TouchUiAction::NarrativeSkip;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::benchmarkActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    constexpr std::array<TouchUiAction, 6U> down{
        TouchUiAction::BenchmarkMapDown, TouchUiAction::BenchmarkEnemiesDown,
        TouchUiAction::BenchmarkTowersDown, TouchUiAction::BenchmarkProjectilesDown,
        TouchUiAction::BenchmarkDecorationsDown, TouchUiAction::BenchmarkRocketsDown};
    constexpr std::array<TouchUiAction, 6U> up{
        TouchUiAction::BenchmarkMapUp, TouchUiAction::BenchmarkEnemiesUp,
        TouchUiAction::BenchmarkTowersUp, TouchUiAction::BenchmarkProjectilesUp,
        TouchUiAction::BenchmarkDecorationsUp, TouchUiAction::BenchmarkRocketsUp};
    for (std::size_t index = 0U; index < down.size(); ++index) {
        if (kBenchmarkDown[index].contains(x, y)) return down[index];
        if (kBenchmarkUp[index].contains(x, y)) return up[index];
    }
    if (kBenchmarkAutomatic.contains(x, y)) return TouchUiAction::BenchmarkAutomatic;
    if (kBenchmarkStart.contains(x, y)) return TouchUiAction::BenchmarkStart;
    if (kBenchmarkBack.contains(x, y)) return TouchUiAction::BenchmarkBack;
    return TouchUiAction::None;
}

TouchUiAction TouchUiLayout::resultActionAt(std::int16_t x, std::int16_t y) {
    if (outside(x, y)) return TouchUiAction::None;
    if (kResultCampaign.contains(x, y)) return TouchUiAction::ResultCampaign;
    if (kResultReplay.contains(x, y)) return TouchUiAction::ResultReplay;
    return TouchUiAction::None;
}

TouchRect TouchUiLayout::rectFor(TouchUiAction action) {
    switch (action) {
        case TouchUiAction::SelectBallista: return kBallista;
        case TouchUiAction::SelectMortar: return kMortar;
        case TouchUiAction::SelectFrost: return kFrost;
        case TouchUiAction::SelectRocket: return kRocket;
        case TouchUiAction::BuildOrSelect: return kBuild;
        case TouchUiAction::Upgrade: return kUpgrade;
        case TouchUiAction::Sell: return kSell;
        case TouchUiAction::StartWave: return kStart;
        case TouchUiAction::TogglePause: return kPause;
        case TouchUiAction::ToggleSpeed: return kSpeed;
        case TouchUiAction::Cancel: return kCancel;
        case TouchUiAction::CampaignPlay: return kCampaignPlay;
        case TouchUiAction::CampaignBenchmark: return kCampaignBenchmark;
        case TouchUiAction::ToggleSound: return kCampaignSound;
        case TouchUiAction::ToggleMusic: return kCampaignMusic;
        case TouchUiAction::Exit: return kCampaignExit;
        case TouchUiAction::NarrativeBack: return kNarrativeBack;
        case TouchUiAction::NarrativeNext: return kNarrativeNext;
        case TouchUiAction::NarrativeSkip: return kNarrativeSkip;
        case TouchUiAction::BenchmarkAutomatic: return kBenchmarkAutomatic;
        case TouchUiAction::BenchmarkStart: return kBenchmarkStart;
        case TouchUiAction::BenchmarkBack: return kBenchmarkBack;
        case TouchUiAction::ResultCampaign: return kResultCampaign;
        case TouchUiAction::ResultReplay: return kResultReplay;
        default: return {};
    }
}
