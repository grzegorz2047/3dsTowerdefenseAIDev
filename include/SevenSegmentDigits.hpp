#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace SevenSegmentDigits {

enum Segment : std::uint8_t {
    Top = 1U << 0U,
    UpperRight = 1U << 1U,
    LowerRight = 1U << 2U,
    Bottom = 1U << 3U,
    LowerLeft = 1U << 4U,
    UpperLeft = 1U << 5U,
    Middle = 1U << 6U,
};

constexpr std::array<std::uint8_t, 10U> kMasks{
    Top | UpperRight | LowerRight | Bottom | LowerLeft | UpperLeft,
    UpperRight | LowerRight,
    Top | UpperRight | Bottom | LowerLeft | Middle,
    Top | UpperRight | LowerRight | Bottom | Middle,
    UpperRight | LowerRight | UpperLeft | Middle,
    Top | LowerRight | Bottom | UpperLeft | Middle,
    Top | LowerRight | Bottom | LowerLeft | UpperLeft | Middle,
    Top | UpperRight | LowerRight,
    Top | UpperRight | LowerRight | Bottom | LowerLeft | UpperLeft | Middle,
    Top | UpperRight | LowerRight | Bottom | UpperLeft | Middle,
};

constexpr int clampValue(int value) {
    return value < 0 ? 0 : (value > 9999 ? 9999 : value);
}

constexpr std::uint8_t maskFor(unsigned int digit) {
    return digit < kMasks.size() ? kMasks[digit] : 0U;
}

constexpr std::size_t digitCount(int value) {
    const int safe = clampValue(value);
    return safe >= 1000 ? 4U : (safe >= 100 ? 3U : (safe >= 10 ? 2U : 1U));
}

}  // namespace SevenSegmentDigits
