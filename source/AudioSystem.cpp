#include "AudioSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {

constexpr float kPi = 3.14159265358979323846F;

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

    ndspResult_ = ndspInit();
    if (R_SUCCEEDED(ndspResult_)) {
        backend_ = AudioBackend::Ndsp;
        ndspSetOutputMode(NDSP_OUTPUT_STEREO);
        initializeNdspChannels();
    } else {
        csndResult_ = csndInit();
        if (R_SUCCEEDED(csndResult_)) {
            backend_ = AudioBackend::Csnd;
        }
    }

    if (backend_ == AudioBackend::None) {
        return false;
    }

    if (!generateSamples()) {
        shutdown();
        return false;
    }

    // An audible startup cue makes backend verification independent from any
    // gameplay action. It also confirms that initialization and playback both
    // succeeded rather than merely reporting that a service handle opened.
    play(AudioCue::BuildSuccess);
    return true;
}

void AudioSystem::initializeNdspChannels() {
    for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
        const int channel = kFirstSfxChannel + static_cast<int>(index);
        ndspChnReset(channel);
        ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
        ndspChnSetRate(channel, static_cast<float>(kSampleRate));
        ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
        float mix[12] = {1.0F, 1.0F};
        ndspChnSetMix(channel, mix);
    }
}

void AudioSystem::play(AudioCue cue) {
    if (backend_ == AudioBackend::None || cue == AudioCue::Count) {
        return;
    }

    const std::size_t cueIndex = static_cast<std::size_t>(cue);
    const Sample& sample = samples_[cueIndex];
    if (sample.data == nullptr || sample.count == 0U) {
        return;
    }

    const std::size_t slot = nextChannel_++ % kSfxChannelCount;
    const int channel = kFirstSfxChannel + static_cast<int>(slot);

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

void AudioSystem::stopAll() {
    if (backend_ == AudioBackend::Ndsp) {
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
}

bool AudioSystem::available() const {
    return backend_ != AudioBackend::None;
}

AudioBackend AudioSystem::backend() const {
    return backend_;
}

Result AudioSystem::ndspResult() const {
    return ndspResult_;
}

Result AudioSystem::csndResult() const {
    return csndResult_;
}

Result AudioSystem::lastPlayResult() const {
    return lastPlayResult_;
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

void AudioSystem::freeSamples() {
    for (Sample& sample : samples_) {
        if (sample.data != nullptr) {
            linearFree(sample.data);
            sample.data = nullptr;
            sample.count = 0;
        }
    }
}
