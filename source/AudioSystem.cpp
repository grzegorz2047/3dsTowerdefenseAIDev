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
        case AudioCue::BuildSuccess: return {0.16F, 520.0F, 880.0F, 0.42F, false};
        case AudioCue::BuildFailure: return {0.14F, 190.0F, 110.0F, 0.36F, true};
        case AudioCue::WaveStart: return {0.28F, 330.0F, 660.0F, 0.40F, false};
        case AudioCue::Shot: return {0.08F, 720.0F, 420.0F, 0.28F, true};
        case AudioCue::Hit: return {0.07F, 260.0F, 120.0F, 0.30F, true};
        case AudioCue::Victory: return {0.48F, 440.0F, 1040.0F, 0.44F, false};
        case AudioCue::Defeat: return {0.52F, 300.0F, 90.0F, 0.40F, false};
        case AudioCue::Count: break;
    }
    return {0.1F, 440.0F, 440.0F, 0.2F, false};
}

}  // namespace

AudioSystem::~AudioSystem() {
    shutdown();
}

bool AudioSystem::initialize() {
    if (initialized_) {
        return true;
    }

    if (R_FAILED(ndspInit())) {
        return false;
    }
    initialized_ = true;

    if (!generateSamples()) {
        shutdown();
        return false;
    }

    for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
        const int channel = kFirstSfxChannel + static_cast<int>(index);
        ndspChnReset(channel);
        ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
        ndspChnSetRate(channel, static_cast<float>(kSampleRate));
        ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
        float mix[12] = {0.72F, 0.72F};
        ndspChnSetMix(channel, mix);
    }
    return true;
}

void AudioSystem::play(AudioCue cue) {
    if (!initialized_ || cue == AudioCue::Count) {
        return;
    }

    const std::size_t cueIndex = static_cast<std::size_t>(cue);
    const Sample& sample = samples_[cueIndex];
    if (sample.data == nullptr || sample.count == 0U) {
        return;
    }

    const std::size_t slot = nextChannel_++ % kSfxChannelCount;
    const int channel = kFirstSfxChannel + static_cast<int>(slot);
    ndspChnWaveBufClear(channel);

    ndspWaveBuf& waveBuffer = waveBuffers_[slot];
    std::memset(&waveBuffer, 0, sizeof(waveBuffer));
    waveBuffer.data_vaddr = sample.data;
    waveBuffer.nsamples = static_cast<u32>(sample.count);
    waveBuffer.looping = false;
    ndspChnWaveBufAdd(channel, &waveBuffer);
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
    if (!initialized_) {
        return;
    }
    for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
        ndspChnWaveBufClear(kFirstSfxChannel + static_cast<int>(index));
        waveBuffers_[index] = {};
    }
}

void AudioSystem::shutdown() {
    if (!initialized_) {
        freeSamples();
        return;
    }
    stopAll();
    freeSamples();
    ndspExit();
    initialized_ = false;
    nextChannel_ = 0;
}

bool AudioSystem::available() const {
    return initialized_;
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
