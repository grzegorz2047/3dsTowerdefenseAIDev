#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <3ds.h>

#include "AudioBackend.hpp"
#include "AudioEvents.hpp"
#include "AudioProbe.hpp"

class AudioSystem {
public:
    AudioSystem() = default;
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] bool initialize();
    void play(AudioCue cue);
    void playMask(std::uint32_t cueMask);
    void playDiagnosticTone();
    void updateProbe();
    void stopAll();
    void shutdown();

    [[nodiscard]] bool available() const;
    [[nodiscard]] AudioBackend backend() const;
    [[nodiscard]] Result ndspResult() const;
    [[nodiscard]] Result csndResult() const;
    [[nodiscard]] Result lastPlayResult() const;
    [[nodiscard]] Result probeResult() const;
    [[nodiscard]] int lastChannel() const;
    [[nodiscard]] bool channelActive() const;
    [[nodiscard]] bool channelEverActive() const;

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
    [[nodiscard]] bool generateDiagnosticTone();
    void playSample(const Sample& sample);
    void initializeNdspChannels();
    void freeSamples();

    AudioBackend backend_ = AudioBackend::None;
    Result ndspResult_ = 0;
    Result csndResult_ = 0;
    Result lastPlayResult_ = 0;
    Result probeResult_ = 0;
    int lastChannel_ = -1;
    AudioProbeState probeState_{};
    std::size_t nextChannel_ = 0;
    std::array<Sample, static_cast<std::size_t>(AudioCue::Count)> samples_{};
    Sample diagnosticTone_{};
    std::array<ndspWaveBuf, kSfxChannelCount> waveBuffers_{};
};
