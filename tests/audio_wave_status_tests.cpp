#include <cstdlib>
#include <iostream>

#include "AudioWaveStatus.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    expect(audioWaveStatusFromRaw(0) == AudioWaveStatus::Free, "0 must map to FREE");
    expect(audioWaveStatusFromRaw(1) == AudioWaveStatus::Queued, "1 must map to QUEUED");
    expect(audioWaveStatusFromRaw(2) == AudioWaveStatus::Playing, "2 must map to PLAYING");
    expect(audioWaveStatusFromRaw(3) == AudioWaveStatus::Done, "3 must map to DONE");
    expect(audioWaveStatusFromRaw(4) == AudioWaveStatus::Unknown, "other values must map to UNKNOWN");

    expect(!audioWaveStatusActive(AudioWaveStatus::Free), "FREE must not be active");
    expect(audioWaveStatusActive(AudioWaveStatus::Queued), "QUEUED must be active");
    expect(audioWaveStatusActive(AudioWaveStatus::Playing), "PLAYING must be active");
    expect(!audioWaveStatusActive(AudioWaveStatus::Done), "DONE must not be active");
    expect(!audioWaveStatusActive(AudioWaveStatus::Unknown), "UNKNOWN must not be active");

    expect(std::string(audioWaveStatusName(AudioWaveStatus::Playing)) == "PLAYING", "PLAYING label must be stable");

    std::cout << "NDSP wave status tests passed\n";
    return 0;
}
