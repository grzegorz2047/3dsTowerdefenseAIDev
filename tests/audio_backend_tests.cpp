#include <cstdlib>
#include <iostream>
#include <string_view>

#include "AudioBackend.hpp"
#include "AudioMusicPolicy.hpp"

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

    expect(shouldStartMissionMusic(AudioBackend::Ndsp, false, true), "NDSP should start available mission music");
    expect(shouldStartMissionMusic(AudioBackend::Csnd, false, true), "CSND fallback should start available mission music");
    expect(!shouldStartMissionMusic(AudioBackend::None, false, true), "missing backend must not start music");
    expect(!shouldStartMissionMusic(AudioBackend::Csnd, true, true), "active CSND music must not restart");
    expect(!shouldStartMissionMusic(AudioBackend::Ndsp, false, false), "missing samples must not start music");
    expect(usesCsndMusicLoop(AudioBackend::Csnd), "CSND must use the hardware loop path");
    expect(!usesCsndMusicLoop(AudioBackend::Ndsp), "NDSP must keep its wave-buffer loop path");

    std::cout << "Audio backend and music policy tests passed\n";
    return 0;
}
