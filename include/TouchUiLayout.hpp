#pragma once

#include <cstdint>

enum class TouchUiAction : std::uint8_t {
    None,
    SelectBallista,
    SelectMortar,
    SelectFrost,
    BuildOrSelect,
    Upgrade,
    Sell,
    StartWave,
    Cancel,
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
    [[nodiscard]] static TouchRect rectFor(TouchUiAction action);
};
