#include <cstdlib>
#include <iostream>

#include "SevenSegmentDigits.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testValueClamping() {
    expect(SevenSegmentDigits::clampValue(-1) == 0, "negative gold must clamp to zero");
    expect(SevenSegmentDigits::clampValue(42) == 42, "normal gold must remain unchanged");
    expect(SevenSegmentDigits::clampValue(12000) == 9999, "gold must fit four digits");
}

void testDigitCounts() {
    expect(SevenSegmentDigits::digitCount(0) == 1U, "zero must render one digit");
    expect(SevenSegmentDigits::digitCount(9) == 1U, "single digit count");
    expect(SevenSegmentDigits::digitCount(10) == 2U, "two digit count");
    expect(SevenSegmentDigits::digitCount(999) == 3U, "three digit count");
    expect(SevenSegmentDigits::digitCount(1000) == 4U, "four digit count");
}

void testRequiredSegments() {
    using namespace SevenSegmentDigits;
    expect(maskFor(0U) == (Top | UpperRight | LowerRight | Bottom | LowerLeft | UpperLeft),
        "zero segment mask");
    expect(maskFor(1U) == (UpperRight | LowerRight), "one segment mask");
    expect((maskFor(8U) & Middle) != 0U, "eight must contain middle segment");
    expect(maskFor(10U) == 0U, "invalid digit must be blank");
}

}  // namespace

int main() {
    testValueClamping();
    testDigitCounts();
    testRequiredSegments();
    std::cout << "Seven-segment digit tests passed\n";
    return 0;
}
