#pragma once

#include <cstdint>

enum class AudioBackend : std::uint8_t {
    None,
    Ndsp,
    Csnd,
};

[[nodiscard]] constexpr AudioBackend selectAudioBackend(bool ndspAvailable, bool csndAvailable) {
    if (ndspAvailable) {
        return AudioBackend::Ndsp;
    }
    if (csndAvailable) {
        return AudioBackend::Csnd;
    }
    return AudioBackend::None;
}

[[nodiscard]] constexpr const char* audioBackendName(AudioBackend backend) {
    switch (backend) {
        case AudioBackend::Ndsp: return "NDSP";
        case AudioBackend::Csnd: return "CSND";
        case AudioBackend::None:
        default: return "BRAK";
    }
}
