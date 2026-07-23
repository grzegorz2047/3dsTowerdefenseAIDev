#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <3ds.h>

#include "AudioBackend.hpp"
#include "AudioEvents.hpp"
#include "AudioMusicPolicy.hpp"
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
    void updateMusic(TutorialPhase phase, bool enabled, float deltaSeconds);
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
    [[nodiscard]] MusicPhase musicPhase() const;

private:
    static constexpr int kSampleRate = 22050;
    static constexpr int kDiagnosticChannel = 0;
    static constexpr std::size_t kSfxChannelCount = 6;
    static constexpr int kFirstSfxChannel = 1;
    static constexpr int kPreparationMusicChannel = 7;
    static constexpr int kCombatMusicChannel = 8;
    static constexpr int kAmbientChannel = 9;

    struct Sample {
        std::int16_t* data = nullptr;
        std::size_t count = 0;
    };

    [[nodiscard]] bool generateSamples();
    [[nodiscard]] bool generateSample(AudioCue cue);
    [[nodiscard]] bool generateDiagnosticTone();
    [[nodiscard]] bool generateMusicLayers();
    [[nodiscard]] bool generateMusicLayer(Sample& destination, bool combat);
    [[nodiscard]] bool generateAmbientLayer();
    void playSample(AudioCue cue, const Sample& sample);
    [[nodiscard]] std::size_t channelSlotFor(AudioCue cue);
    void initializeNdspChannels();
    void startMusicLayers();
    void applyMusicMix();
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
    std::array<std::size_t, 3U> poolCursors_{};
    std::array<Sample, static_cast<std::size_t>(AudioCue::Count)> samples_{};
    Sample diagnosticTone_{};
    Sample preparationMusic_{};
    Sample combatMusic_{};
    Sample ambient_{};
    ndspWaveBuf diagnosticWaveBuffer_{};
    std::array<ndspWaveBuf, 3U> musicWaveBuffers_{};
    std::array<ndspWaveBuf, kSfxChannelCount> waveBuffers_{};
    std::uint32_t diagnosticSamplePosition_ = 0;
    bool musicLayersStarted_ = false;
    MusicPhase musicPhase_ = MusicPhase::Silent;
    float preparationGain_ = 0.0F;
    float combatGain_ = 0.0F;
    float ambientGain_ = 0.0F;
};
