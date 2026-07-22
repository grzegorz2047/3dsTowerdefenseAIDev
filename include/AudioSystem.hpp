#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <3ds.h>

#include "AudioEvents.hpp"

class AudioSystem {
public:
    AudioSystem() = default;
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] bool initialize();
    void play(AudioCue cue);
    void playMask(std::uint32_t cueMask);
    void stopAll();
    void shutdown();

    [[nodiscard]] bool available() const;

private:
    static constexpr int kSampleRate = 22050;
    static constexpr std::size_t kSfxChannelCount = 6;
    static constexpr int kFirstSfxChannel = 1;

    struct Sample {
        std::int16_t* data = nullptr;
        std::size_t count = 0;
    };

    [[nodiscard]] bool generateSamples();
    [[nodiscard]] bool generateSample(AudioCue cue);
    void freeSamples();

    bool initialized_ = false;
    std::size_t nextChannel_ = 0;
    std::array<Sample, static_cast<std::size_t>(AudioCue::Count)> samples_{};
    std::array<ndspWaveBuf, kSfxChannelCount> waveBuffers_{};
};
