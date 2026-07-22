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
        !shouldAttemptNdspHleShim(kAudioResultSuccess),
        "successful NDSP initialization must never attempt the shim");
    expect(
        !shouldAttemptNdspHleShim(0xD8E007F7U),
        "unrelated DSP failures must not be hidden by the shim");
    expect(
        !shouldAttemptNdspHleShim(1U),
        "generic failures must remain real failures");

    expect(
        ndspBackendReady(kAudioResultSuccess, false, kAudioResultNotAttempted),
        "normal successful NDSP initialization must select NDSP");
    expect(
        ndspBackendReady(kNdspComponentNotFoundResult, true, kAudioResultSuccess),
        "D8B0A7FA followed by a successful shim must select NDSP");
    expect(
        ndspShimIsActive(true, kAudioResultSuccess),
        "successful attempted shim must be reported as active");
    expect(
        !ndspShimIsActive(false, kAudioResultSuccess),
        "an unattempted shim must never appear active even with a zero placeholder");
    expect(
        !ndspBackendReady(kNdspComponentNotFoundResult, false, kAudioResultNotAttempted),
        "missing component without a retry must not select NDSP");
    expect(
        !ndspBackendReady(kNdspComponentNotFoundResult, true, 0xD8E007F7U),
        "failed shim retry must not select NDSP");

    std::cout << "NDSP HLE shim policy tests passed\n";
    return 0;
}
