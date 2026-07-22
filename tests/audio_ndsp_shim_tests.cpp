#include <cstdlib>
#include <iostream>

#include "AudioNdspShim.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    expect(
        shouldAttemptNdspHleShim(kNdspComponentNotFoundResult),
        "missing DSP component must enable the HLE shim retry");
    expect(
        !shouldAttemptNdspHleShim(0U),
        "successful NDSP initialization must never attempt the shim");
    expect(
        !shouldAttemptNdspHleShim(0xD8E007F7U),
        "unrelated DSP failures must not be hidden by the shim");
    expect(
        !shouldAttemptNdspHleShim(1U),
        "generic failures must remain real failures");

    std::cout << "NDSP HLE shim policy tests passed\n";
    return 0;
}
