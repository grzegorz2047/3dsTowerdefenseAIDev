#include "AudioSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

#include "AudioNdspShim.hpp"

namespace {

constexpr float kPi = 3.14159265358979323846F;

// Azahar's DSP HLE does not execute the DSP program and only requires a
// component buffer to pass through libctru's normal NDSP initialization path.
// This project-owned all-zero buffer contains no Nintendo firmware data.
alignas(0x80) constexpr std::array<std::uint8_t, 0x400> kSyntheticHleComponent{};

struct CueShape {
    float durationSeconds;
    float startFrequency;
    float endFrequency;
    float amplitude;
    bool squareWave;
};

CueShape shapeFor(AudioCue cue) {
    switch (cue) {
        case AudioCue::BuildSuccess: return {0.16F, 520.0F, 880.0F, 0.55F, false};
        case AudioCue::BuildFailure: return {0.14F, 190.0F, 110.0F, 0.48F, true};
        case AudioCue::WaveStart: return {0.28F, 330.0F, 660.0F, 0.52F, false};
        case AudioCue::Shot: return {0.08F, 720.0F, 420.0F, 0.38F, true};
        case AudioCue::Hit: return {0.07F, 260.0F, 120.0F, 0.42F, true};
        case AudioCue::Victory: return {0.48F, 440.0F, 1040.0F, 0.56F, false};
        case AudioCue::Defeat: return {0.52F, 300.0F, 90.0F, 0.52F, false};
        case AudioCue::Count: break;
    }
    return {0.1F, 440.0F, 440.0F, 0.3F, false};
}

}  // namespace

AudioSystem::~AudioSystem() {
    shutdown();
}

bool AudioSystem::initialize() {
    if (backend_ != AudioBackend::None) {
        return true;
    }

    ndspInitialResult_ = ndspInit();
    ndspResult_ = ndspInitialResult_;
    const std::uint32_t initialRaw = static_cast<std::uint32_t>(ndspInitialResult_);

    if (shouldAttemptNdspHleShim(initialRaw)) {
        ndspShimAttempted_ = true;
        ndspUseComponent(
            kSyntheticHleComponent.data(),
            static_cast<u32>(kSyntheticHleComponent.size()),
            0x00FFU,
            0x00FFU);
        ndspShimResult_ = ndspInit();
        ndspResult_ = ndspShimResult_;
        ndspShimActive_ = ndspShimIsActive(
            ndspShimAttempted_,
            static_cast<std::uint32_t>(ndspShimResult_));
    }

    if (ndspBackendReady(
            initialRaw,
            ndspShimAttempted_,
            static_cast<std::uint32_t>(ndspShimResult_))) {
        backend_ = AudioBackend::Ndsp;
        ndspSetOutputMode(NDSP_OUTPUT_STEREO);
        ndspSetMasterVol(1.0F);
        initializeNdspChannels();
    } else {
        csndResult_ = csndInit();
        if (audioResultSucceeded(static_cast<std::uint32_t>(csndResult_))) {
            backend_ = AudioBackend::Csnd;
        }
    }

    if (backend_ == AudioBackend::None) {
        return false;
    }

    if (!generateSamples() || !generateDiagnosticTone()) {
        shutdown();
        return false;
    }

    if (backend_ == AudioBackend::Ndsp) {
        playDiagnosticTone();
    } else {
        play(AudioCue::BuildSuccess);
    }
    return true;
}

void AudioSystem::initializeNdspChannels() {
    ndspChnReset(kDiagnosticChannel);
    ndspChnSetInterp(kDiagnosticChannel, NDSP_INTERP_LINEAR);
    ndspChnSetRate(kDiagnosticChannel, static_cast<float>(kSampleRate));
    ndspChnSetFormat(kDiagnosticChannel, NDSP_FORMAT_STEREO_PCM16);
    float diagnosticMix[12]{};
    diagnosticMix[0] = 1.0F;
    diagnosticMix[1] = 1.0F;
    ndspChnSetMix(kDiagnosticChannel, diagnosticMix);

    for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
        const int channel = kFirstSfxChannel + static_cast<int>(index);
        ndspChnReset(channel);
        ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
        ndspChnSetRate(channel, static_cast<float>(kSampleRate));
        ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
        float mix[12]{};
        mix[0] = 1.0F;
        mix[1] = 1.0F;
        ndspChnSetMix(channel, mix);
    }
}

void AudioSystem::play(AudioCue cue) {
    if (backend_ == AudioBackend::None || cue == AudioCue::Count) {
        return;
    }

    const Sample& sample = samples_[static_cast<std::size_t>(cue)];
    playSample(sample);
}

void AudioSystem::playDiagnosticTone() {
    if (backend_ == AudioBackend::Ndsp) {
        if (diagnosticTone_.data == nullptr || diagnosticTone_.count == 0U) {
            return;
        }
        ndspChnWaveBufClear(kDiagnosticChannel);
        std::memset(&diagnosticWaveBuffer_, 0, sizeof(diagnosticWaveBuffer_));
        diagnosticWaveBuffer_.data_vaddr = diagnosticTone_.data;
        diagnosticWaveBuffer_.nsamples = static_cast<u32>(diagnosticTone_.count);
        diagnosticWaveBuffer_.looping = false;
        diagnosticSamplePosition_ = 0;
        lastChannel_ = kDiagnosticChannel;
        probeResult_ = 0;
        probeState_ = {false, false, probeState_.everActive};
        ndspChnWaveBufAdd(kDiagnosticChannel, &diagnosticWaveBuffer_);
        lastPlayResult_ = 0;
        return;
    }

    if (backend_ == AudioBackend::Csnd) {
        play(AudioCue::BuildSuccess);
    }
}

void AudioSystem::playSample(const Sample& sample) {
    if (backend_ == AudioBackend::None || sample.data == nullptr || sample.count == 0U) {
        return;
    }

    const std::size_t slot = nextChannel_++ % kSfxChannelCount;
    const int channel = kFirstSfxChannel + static_cast<int>(slot);
    lastChannel_ = channel;

    if (backend_ == AudioBackend::Ndsp) {
        ndspChnWaveBufClear(channel);
        ndspWaveBuf& waveBuffer = waveBuffers_[slot];
        std::memset(&waveBuffer, 0, sizeof(waveBuffer));
        waveBuffer.data_vaddr = sample.data;
        waveBuffer.nsamples = static_cast<u32>(sample.count);
        waveBuffer.looping = false;
        ndspChnWaveBufAdd(channel, &waveBuffer);
        lastPlayResult_ = 0;
        return;
    }

    const u32 byteCount = static_cast<u32>(sample.count * sizeof(std::int16_t));
    const Result flushResult = CSND_FlushDataCache(sample.data, byteCount);
    if (R_FAILED(flushResult)) {
        lastPlayResult_ = flushResult;
        return;
    }

    lastPlayResult_ = csndPlaySound(
        channel,
        SOUND_ONE_SHOT | SOUND_FORMAT_16BIT | SOUND_LINEAR_INTERP,
        static_cast<u32>(kSampleRate),
        1.0F,
        0.0F,
        sample.data,
        nullptr,
        byteCount);
}

void AudioSystem::playMask(std::uint32_t cueMask) {
    for (std::uint8_t value = 0; value < static_cast<std::uint8_t>(AudioCue::Count); ++value) {
        const AudioCue cue = static_cast<AudioCue>(value);
        if ((cueMask & audioCueMask(cue)) != 0U) {
            play(cue);
        }
    }
}

void AudioSystem::updateProbe() {
    if (backend_ == AudioBackend::Ndsp) {
        const AudioWaveStatus status = diagnosticWaveStatus();
        diagnosticSamplePosition_ = ndspChnGetSamplePos(kDiagnosticChannel);
        probeResult_ = 0;
        probeState_ = updateAudioProbeState(
            probeState_,
            true,
            audioWaveStatusActive(status));
        return;
    }

    if (backend_ != AudioBackend::Csnd || lastChannel_ < 0) {
        return;
    }

    CSND_ChnInfo info{};
    probeResult_ = csndGetState(static_cast<u32>(lastChannel_), &info);
    const bool querySucceeded = R_SUCCEEDED(probeResult_);
    probeState_ = updateAudioProbeState(probeState_, querySucceeded, querySucceeded && info.active != 0U);
}

void AudioSystem::stopAll() {
    if (backend_ == AudioBackend::Ndsp) {
        ndspChnWaveBufClear(kDiagnosticChannel);
        diagnosticWaveBuffer_ = {};
        diagnosticSamplePosition_ = 0;
        for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
            ndspChnWaveBufClear(kFirstSfxChannel + static_cast<int>(index));
            waveBuffers_[index] = {};
        }
        return;
    }

    if (backend_ == AudioBackend::Csnd) {
        for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
            CSND_SetPlayStateR(kFirstSfxChannel + static_cast<int>(index), 0U);
        }
        lastPlayResult_ = csndExecCmds(true);
    }
}

void AudioSystem::shutdown() {
    stopAll();
    freeSamples();

    if (backend_ == AudioBackend::Ndsp) {
        ndspExit();
    } else if (backend_ == AudioBackend::Csnd) {
        csndExit();
    }

    backend_ = AudioBackend::None;
    nextChannel_ = 0;
    lastChannel_ = -1;
    ndspResult_ = static_cast<Result>(kAudioResultNotAttempted);
    ndspInitialResult_ = static_cast<Result>(kAudioResultNotAttempted);
    ndspShimResult_ = static_cast<Result>(kAudioResultNotAttempted);
    ndspShimAttempted_ = false;
    ndspShimActive_ = false;
    csndResult_ = static_cast<Result>(kAudioResultNotAttempted);
    lastPlayResult_ = static_cast<Result>(kAudioResultNotAttempted);
    probeResult_ = static_cast<Result>(kAudioResultNotAttempted);
    probeState_ = {};
    diagnosticWaveBuffer_ = {};
    diagnosticSamplePosition_ = 0;
}

bool AudioSystem::available() const { return backend_ != AudioBackend::None; }
AudioBackend AudioSystem::backend() const { return backend_; }
Result AudioSystem::ndspResult() const { return ndspResult_; }
Result AudioSystem::ndspInitialResult() const { return ndspInitialResult_; }
Result AudioSystem::ndspShimResult() const { return ndspShimResult_; }
bool AudioSystem::ndspShimAttempted() const { return ndspShimAttempted_; }
bool AudioSystem::ndspShimActive() const { return ndspShimActive_; }
Result AudioSystem::csndResult() const { return csndResult_; }
Result AudioSystem::lastPlayResult() const { return lastPlayResult_; }
Result AudioSystem::probeResult() const { return probeResult_; }
int AudioSystem::lastChannel() const { return lastChannel_; }
bool AudioSystem::channelActive() const { return probeState_.active; }
bool AudioSystem::channelEverActive() const { return probeState_.everActive; }
AudioWaveStatus AudioSystem::diagnosticWaveStatus() const {
    return audioWaveStatusFromRaw(diagnosticWaveBuffer_.status);
}
std::uint32_t AudioSystem::diagnosticSamplePosition() const { return diagnosticSamplePosition_; }
std::uint32_t AudioSystem::diagnosticSampleCount() const {
    return static_cast<std::uint32_t>(diagnosticTone_.count);
}

bool AudioSystem::generateSamples() {
    for (std::uint8_t value = 0; value < static_cast<std::uint8_t>(AudioCue::Count); ++value) {
        if (!generateSample(static_cast<AudioCue>(value))) {
            return false;
        }
    }
    return true;
}

bool AudioSystem::generateSample(AudioCue cue) {
    const CueShape shape = shapeFor(cue);
    const std::size_t sampleCount = std::max<std::size_t>(
        1U,
        static_cast<std::size_t>(shape.durationSeconds * static_cast<float>(kSampleRate)));
    auto* data = static_cast<std::int16_t*>(linearAlloc(sampleCount * sizeof(std::int16_t)));
    if (data == nullptr) {
        return false;
    }

    float phase = 0.0F;
    for (std::size_t index = 0; index < sampleCount; ++index) {
        const float progress = static_cast<float>(index) / static_cast<float>(sampleCount);
        const float frequency = shape.startFrequency + (shape.endFrequency - shape.startFrequency) * progress;
        phase += 2.0F * kPi * frequency / static_cast<float>(kSampleRate);
        const float attack = std::min(1.0F, progress * 18.0F);
        const float release = std::max(0.0F, 1.0F - progress);
        const float envelope = attack * release * release;
        const float oscillator = shape.squareWave
            ? (std::sin(phase) >= 0.0F ? 1.0F : -1.0F)
            : std::sin(phase);
        const float sample = std::clamp(oscillator * envelope * shape.amplitude, -1.0F, 1.0F);
        data[index] = static_cast<std::int16_t>(sample * 32767.0F);
    }

    DSP_FlushDataCache(data, sampleCount * sizeof(std::int16_t));
    samples_[static_cast<std::size_t>(cue)] = {data, sampleCount};
    return true;
}

bool AudioSystem::generateDiagnosticTone() {
    constexpr float kDurationSeconds = 2.0F;
    constexpr float kFrequency = 880.0F;
    constexpr float kAmplitude = 0.75F;
    const std::size_t frameCount = static_cast<std::size_t>(kDurationSeconds * static_cast<float>(kSampleRate));
    auto* data = static_cast<std::int16_t*>(linearAlloc(frameCount * 2U * sizeof(std::int16_t)));
    if (data == nullptr) {
        return false;
    }

    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        const float progress = static_cast<float>(frame) / static_cast<float>(frameCount);
        const float attack = std::min(1.0F, progress * 100.0F);
        const float release = std::min(1.0F, (1.0F - progress) * 100.0F);
        const float envelope = attack * release;
        const float phase = 2.0F * kPi * kFrequency * static_cast<float>(frame) / static_cast<float>(kSampleRate);
        const std::int16_t sample = static_cast<std::int16_t>(
            std::sin(phase) * envelope * kAmplitude * 32767.0F);
        data[frame * 2U] = sample;
        data[frame * 2U + 1U] = sample;
    }

    DSP_FlushDataCache(data, frameCount * 2U * sizeof(std::int16_t));
    diagnosticTone_ = {data, frameCount};
    return true;
}

void AudioSystem::freeSamples() {
    for (Sample& sample : samples_) {
        if (sample.data != nullptr) {
            linearFree(sample.data);
            sample = {};
        }
    }
    if (diagnosticTone_.data != nullptr) {
        linearFree(diagnosticTone_.data);
        diagnosticTone_ = {};
    }
}
