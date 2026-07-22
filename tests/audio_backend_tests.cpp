#include <cstdlib>
#include <iostream>
#include <string_view>

#include "AudioBackend.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    expect(selectAudioBackend(true, true) == AudioBackend::Ndsp, "NDSP must have priority when both backends are available");
    expect(selectAudioBackend(true, false) == AudioBackend::Ndsp, "NDSP must be selected when available");
    expect(selectAudioBackend(false, true) == AudioBackend::Csnd, "CSND must be selected when NDSP fails");
    expect(selectAudioBackend(false, false) == AudioBackend::None, "audio must report no backend when both services fail");

    expect(std::string_view(audioBackendName(AudioBackend::Ndsp)) == "NDSP", "NDSP name must be stable");
    expect(std::string_view(audioBackendName(AudioBackend::Csnd)) == "CSND", "CSND name must be stable");
    expect(std::string_view(audioBackendName(AudioBackend::None)) == "BRAK", "missing backend must be visible");

    std::cout << "Audio backend unit tests passed\n";
    return 0;
}
