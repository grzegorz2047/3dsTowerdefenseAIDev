#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <3ds.h>

#include "AudioBackend.hpp"
#include "AudioEvents.hpp"
#include "AudioNdspShim.hpp"
#include "AudioProbe.hpp"
#include "AudioWaveStatus.hpp"

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
    void startMissionMusic();
    void stopMusic();
    void updateProbe();
    void stopAll();
    void shutdown();

    [[nodiscard]] bool available() const;
    [[nodiscard]] AudioBackend backend() const;
    [[nodiscard]] Result ndspResult() const;
    [[nodiscard]] Result ndspInitialResult() const;
    [[nodiscard]] Result ndspShimResult() const;
    [[nodiscard]] bool ndspShimAttempted() const;
    [[nodiscard]] bool ndspShimActive() const;
    [[nodiscard]] Result csndResult() const;
    [[nodiscard]] Result lastPlayResult() const;
    [[nodiscard]] Result probeResult() const;
    [[nodiscard]] int lastChannel() const;
    [[nodiscard]] bool channelActive() const;
    [[nodiscard]] bool channelEverActive() const;
    [[nodiscard]] AudioWaveStatus diagnosticWaveStatus() const;
    [[nodiscard]] std::uint32_t diagnosticSamplePosition() const;
    [[nodiscard]] std::uint32_t diagnosticSampleCount() const;

private:
    static constexpr int kSampleRate = 22050;
    static constexpr int kDiagnosticChannel = 0;
    static constexpr std::size_t kSfxChannelCount = 6;
    static constexpr int kFirstSfxChannel = 1;
    static constexpr int kMusicChannel = kFirstSfxChannel + static_cast<int>(kSfxChannelCount);

    struct Sample {
        std::int16_t* data = nullptr;
        std::size_t count = 0;
    };

    [[nodiscard]] bool generateSamples();
    [[nodiscard]] bool generateSample(AudioCue cue);
    [[nodiscard]] bool generateDiagnosticTone();
    [[nodiscard]] bool generateMissionMusic();
    void playSample(const Sample& sample);
    void initializeNdspChannels();
    void freeSamples();

    AudioBackend backend_ = AudioBackend::None;
    Result ndspResult_ = static_cast<Result>(kAudioResultNotAttempted);
    Result ndspInitialResult_ = static_cast<Result>(kAudioResultNotAttempted);
    Result ndspShimResult_ = static_cast<Result>(kAudioResultNotAttempted);
    bool ndspShimAttempted_ = false;
    bool ndspShimActive_ = false;
    Result csndResult_ = static_cast<Result>(kAudioResultNotAttempted);
    Result lastPlayResult_ = static_cast<Result>(kAudioResultNotAttempted);
    Result probeResult_ = static_cast<Result>(kAudioResultNotAttempted);
    int lastChannel_ = -1;
    AudioProbeState probeState_{};
    std::size_t nextChannel_ = 0;
    std::array<Sample, static_cast<std::size_t>(AudioCue::Count)> samples_{};
    Sample diagnosticTone_{};
    Sample missionMusic_{};
    ndspWaveBuf diagnosticWaveBuffer_{};
    ndspWaveBuf musicWaveBuffer_{};
    std::uint32_t diagnosticSamplePosition_ = 0;
    std::array<ndspWaveBuf, kSfxChannelCount> waveBuffers_{};
};
