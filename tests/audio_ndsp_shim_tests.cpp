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
        shouldAttemptNdspHleShim(kObservedNdspComponentNotFoundResult),
        "observed D880A7FA must enable the HLE shim retry");
    expect(
        resultSummary(kObservedNdspComponentNotFoundResult) == kResultSummaryNotFound,
        "observed result must decode to NOTFOUND summary");
    expect(
        resultModule(kObservedNdspComponentNotFoundResult) == kResultModuleDsp,
        "observed result must decode to DSP module");
    expect(
        resultDescription(kObservedNdspComponentNotFoundResult) == kResultDescriptionNotFound,
        "observed result must decode to NOT_FOUND description");

    expect(
        !shouldAttemptNdspHleShim(kAudioResultSuccess),
        "successful NDSP initialization must never attempt the shim");
    expect(
        !shouldAttemptNdspHleShim(0xD8B0A7FAU),
        "INVALIDSTATE summary must not be mistaken for a missing component");
    expect(
        !shouldAttemptNdspHleShim(0xD880A3FAU),
        "a non-DSP module must not enable the shim");
    expect(
        !shouldAttemptNdspHleShim(0xD880A7F9U),
        "a different description must not enable the shim");
    expect(
        !shouldAttemptNdspHleShim(1U),
        "generic failures must remain real failures");

    expect(
        ndspBackendReady(kAudioResultSuccess, false, kAudioResultNotAttempted),
        "normal successful NDSP initialization must select NDSP");
    expect(
        ndspBackendReady(kObservedNdspComponentNotFoundResult, true, kAudioResultSuccess),
        "D880A7FA followed by a successful shim must select NDSP");
    expect(
        ndspShimIsActive(true, kAudioResultSuccess),
        "successful attempted shim must be reported as active");
    expect(
        !ndspShimIsActive(false, kAudioResultSuccess),
        "an unattempted shim must never appear active even with a zero placeholder");
    expect(
        !ndspBackendReady(kObservedNdspComponentNotFoundResult, false, kAudioResultNotAttempted),
        "missing component without a retry must not select NDSP");
    expect(
        !ndspBackendReady(kObservedNdspComponentNotFoundResult, true, 0xD8E007F7U),
        "failed shim retry must not select NDSP");

    std::cout << "NDSP HLE shim policy tests passed\n";
    return 0;
}
