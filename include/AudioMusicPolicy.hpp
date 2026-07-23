#pragma once

#include "AudioBackend.hpp"

[[nodiscard]] constexpr bool shouldStartMissionMusic(
    AudioBackend backend,
    bool alreadyPlaying,
    bool hasSamples) {
    return backend != AudioBackend::None && !alreadyPlaying && hasSamples;
}

[[nodiscard]] constexpr bool usesCsndMusicLoop(AudioBackend backend) {
    return backend == AudioBackend::Csnd;
}
