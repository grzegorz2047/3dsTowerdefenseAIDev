#include <cmath>
#include <cstdlib>
#include <iostream>

#include "WaveSpeed.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    expect(WaveSpeed::initial() == 1, "every mission should start at 1x");
    expect(WaveSpeed::normalize(0) == 1 && WaveSpeed::normalize(3) == 1,
        "invalid or stale speed values should normalize to 1x");
    expect(WaveSpeed::toggled(1) == 2 && WaveSpeed::toggled(2) == 1,
        "explicit speed toggle should alternate predictably");
    expect(std::fabs(WaveSpeed::simulationDelta(1.0F, 1) - 1.0F) < 0.0001F,
        "one real second should equal one simulation second at 1x");
    expect(std::fabs(WaveSpeed::simulationDelta(1.0F, 2) - 2.0F) < 0.0001F,
        "one real second should equal two simulation seconds at explicit 2x");
    expect(WaveSpeed::simulationDelta(-1.0F, 2) == 0.0F,
        "negative frame time should never advance simulation");
    std::cout << "Wave speed tests passed\n";
    return 0;
}
