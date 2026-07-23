#pragma once

#include <cstdint>

enum class TouchUiAction : std::uint8_t {
    None,
    SelectBallista, SelectMortar, SelectFrost, SelectRocket,
    BuildOrSelect, Upgrade, Sell, StartWave, TogglePause, ToggleSpeed, Cancel,
    CampaignRow0, CampaignRow1, CampaignRow2, CampaignRow3, CampaignRow4, CampaignRow5,
    CampaignPlay, CampaignBenchmark, ToggleSound, ToggleMusic, Exit,
    NarrativeBack, NarrativeNext, NarrativeSkip,
    BenchmarkMapDown, BenchmarkMapUp,
    BenchmarkEnemiesDown, BenchmarkEnemiesUp,
    BenchmarkTowersDown, BenchmarkTowersUp,
    BenchmarkProjectilesDown, BenchmarkProjectilesUp,
    BenchmarkDecorationsDown, BenchmarkDecorationsUp,
    BenchmarkRocketsDown, BenchmarkRocketsUp,
    BenchmarkAutomatic, BenchmarkStart, BenchmarkBack,
    ResultCampaign, ResultReplay,
};

struct TouchRect {
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t width = 0;
    std::int16_t height = 0;

    [[nodiscard]] constexpr bool contains(std::int16_t pointX, std::int16_t pointY) const {
        return pointX >= x && pointY >= y &&
            pointX < static_cast<std::int16_t>(x + width) &&
            pointY < static_cast<std::int16_t>(y + height);
    }
};

class TouchUiLayout {
public:
    static constexpr std::int16_t kScreenWidth = 320;
    static constexpr std::int16_t kScreenHeight = 240;
    static constexpr std::int16_t kMinimumButtonHeight = 40;

    [[nodiscard]] static TouchUiAction actionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction campaignActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction narrativeActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction benchmarkActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchUiAction resultActionAt(std::int16_t x, std::int16_t y);
    [[nodiscard]] static TouchRect rectFor(TouchUiAction action);
};
