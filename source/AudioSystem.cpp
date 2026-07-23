#include "AudioSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

#include "AudioChannelPool.hpp"

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
        case AudioCue::Shot: return {0.08F, 720.0F, 420.0F, 0.34F, true};
        case AudioCue::Hit: return {0.07F, 260.0F, 120.0F, 0.36F, true};
        case AudioCue::EnemyDeath: return {0.18F, 430.0F, 145.0F, 0.42F, false};
        case AudioCue::BaseDamage: return {0.32F, 155.0F, 68.0F, 0.58F, true};
        case AudioCue::Victory: return {0.48F, 440.0F, 1040.0F, 0.56F, false};
        case AudioCue::Defeat: return {0.52F, 300.0F, 90.0F, 0.52F, false};
        case AudioCue::Count: break;
    }
    return {0.1F, 440.0F, 440.0F, 0.3F, false};
}

float noteFrequency(int semitoneFromA4) {
    return 440.0F * std::pow(2.0F, static_cast<float>(semitoneFromA4) / 12.0F);
}

}  // namespace

AudioSystem::~AudioSystem() { shutdown(); }

bool AudioSystem::initialize() {
    if (backend_ != AudioBackend::None) return true;

    ndspInitialResult_ = ndspInit();
    ndspResult_ = ndspInitialResult_;
    if (R_SUCCEEDED(ndspInitialResult_)) {
        backend_ = AudioBackend::Ndsp;
        ndspSetOutputMode(NDSP_OUTPUT_STEREO);
        ndspSetMasterVol(1.0F);
        initializeNdspChannels();
    } else {
        // Never feed ndspUseComponent synthetic bytes. A failed NDSP init means
        // the application does not have a valid DSP component and must use the
        // hardware CSND service instead of reporting a silent NDSP backend.
        ndspExit();
        ndspShimAttempted_ = false;
        ndspShimActive_ = false;
        ndspShimResult_ = static_cast<Result>(kAudioResultNotAttempted);
        csndResult_ = csndInit();
        if (R_SUCCEEDED(csndResult_)) backend_ = AudioBackend::Csnd;
    }

    if (backend_ == AudioBackend::None) return false;
    if (!generateSamples() || !generateDiagnosticTone() || !generateMissionMusic()) {
        shutdown();
        return false;
    }
    return true;
}

void AudioSystem::initializeNdspChannels() {
    ndspChnReset(kDiagnosticChannel);
    ndspChnSetInterp(kDiagnosticChannel, NDSP_INTERP_LINEAR);
    ndspChnSetRate(kDiagnosticChannel, static_cast<float>(kSampleRate));
    ndspChnSetFormat(kDiagnosticChannel, NDSP_FORMAT_STEREO_PCM16);
    float diagnosticMix[12]{}; diagnosticMix[0] = 1.0F; diagnosticMix[1] = 1.0F;
    ndspChnSetMix(kDiagnosticChannel, diagnosticMix);
    for (std::size_t index = 0; index < kSfxChannelCount; ++index) {
        const int channel = kFirstSfxChannel + static_cast<int>(index);
        ndspChnReset(channel);
        ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
        ndspChnSetRate(channel, static_cast<float>(kSampleRate));
        ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
        float mix[12]{}; mix[0] = 0.82F; mix[1] = 0.82F;
        ndspChnSetMix(channel, mix);
    }
    ndspChnReset(kMusicChannel);
    ndspChnSetInterp(kMusicChannel, NDSP_INTERP_LINEAR);
    ndspChnSetRate(kMusicChannel, static_cast<float>(kSampleRate));
    ndspChnSetFormat(kMusicChannel, NDSP_FORMAT_MONO_PCM16);
    float musicMix[12]{}; musicMix[0] = 0.24F; musicMix[1] = 0.24F;
    ndspChnSetMix(kMusicChannel, musicMix);
}

void AudioSystem::play(AudioCue cue) {
    if (backend_ == AudioBackend::None || cue == AudioCue::Count) return;
    playSample(cue, samples_[static_cast<std::size_t>(cue)]);
}

void AudioSystem::playDiagnosticTone() {
    if (backend_ == AudioBackend::Ndsp) {
        if (diagnosticTone_.data == nullptr || diagnosticTone_.count == 0U) return;
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
    if (backend_ == AudioBackend::Csnd) play(AudioCue::BuildSuccess);
}

void AudioSystem::startMissionMusic() {
    if (backend_ == AudioBackend::Ndsp && missionMusic_.data != nullptr && missionMusic_.count != 0U) {
        ndspChnWaveBufClear(kMusicChannel);
        std::memset(&musicWaveBuffer_, 0, sizeof(musicWaveBuffer_));
        musicWaveBuffer_.data_vaddr = missionMusic_.data;
        musicWaveBuffer_.nsamples = static_cast<u32>(missionMusic_.count);
        musicWaveBuffer_.looping = true;
        ndspChnWaveBufAdd(kMusicChannel, &musicWaveBuffer_);
    }
}

void AudioSystem::stopMusic() {
    if (backend_ == AudioBackend::Ndsp) { ndspChnWaveBufClear(kMusicChannel); musicWaveBuffer_ = {}; }
}

std::size_t AudioSystem::channelSlotFor(AudioCue cue) {
    const std::size_t pool = audioPoolIndex(cue);
    const std::size_t slot = AudioChannelPool::slotFor(cue, poolCursors_[pool]);
    poolCursors_[pool] = AudioChannelPool::nextCursor(cue, poolCursors_[pool]);
    return slot;
}

void AudioSystem::playSample(AudioCue cue, const Sample& sample) {
    if (backend_ == AudioBackend::None || sample.data == nullptr || sample.count == 0U) return;
    const std::size_t slot = channelSlotFor(cue);
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
    if (R_FAILED(flushResult)) { lastPlayResult_ = flushResult; return; }
    lastPlayResult_ = csndPlaySound(channel,
        SOUND_ONE_SHOT | SOUND_FORMAT_16BIT | SOUND_LINEAR_INTERP,
        static_cast<u32>(kSampleRate), 0.82F, 0.0F, sample.data, nullptr, byteCount);
    if (R_SUCCEEDED(lastPlayResult_)) lastPlayResult_ = csndExecCmds(true);
}

void AudioSystem::playMask(std::uint32_t cueMask) {
    if (backend_ == AudioBackend::Ndsp && musicWaveBuffer_.data_vaddr == nullptr) startMissionMusic();
    constexpr std::array<AudioCue, 9U> order{AudioCue::Victory, AudioCue::Defeat, AudioCue::BaseDamage,
        AudioCue::WaveStart, AudioCue::BuildSuccess, AudioCue::BuildFailure, AudioCue::EnemyDeath,
        AudioCue::Hit, AudioCue::Shot};
    for (AudioCue cue : order) if ((cueMask & audioCueMask(cue)) != 0U) play(cue);
}

void AudioSystem::updateProbe() {
    if (backend_ == AudioBackend::Ndsp) {
        const AudioWaveStatus status = diagnosticWaveStatus();
        diagnosticSamplePosition_ = ndspChnGetSamplePos(kDiagnosticChannel);
        probeResult_ = 0;
        probeState_ = updateAudioProbeState(probeState_, true, audioWaveStatusActive(status));
        return;
    }
    if (backend_ != AudioBackend::Csnd || lastChannel_ < 0) return;
    CSND_ChnInfo info{};
    probeResult_ = csndGetState(static_cast<u32>(lastChannel_), &info);
    const bool ok = R_SUCCEEDED(probeResult_);
    probeState_ = updateAudioProbeState(probeState_, ok, ok && info.active != 0U);
}

void AudioSystem::stopAll() {
    if (backend_ == AudioBackend::Ndsp) {
        stopMusic(); ndspChnWaveBufClear(kDiagnosticChannel); diagnosticWaveBuffer_ = {};
        diagnosticSamplePosition_ = 0;
        for (std::size_t i = 0; i < kSfxChannelCount; ++i) {
            ndspChnWaveBufClear(kFirstSfxChannel + static_cast<int>(i)); waveBuffers_[i] = {};
        }
    } else if (backend_ == AudioBackend::Csnd) {
        for (std::size_t i = 0; i < kSfxChannelCount; ++i)
            CSND_SetPlayStateR(kFirstSfxChannel + static_cast<int>(i), 0U);
        lastPlayResult_ = csndExecCmds(true);
    }
}

void AudioSystem::shutdown() {
    stopAll(); freeSamples();
    if (backend_ == AudioBackend::Ndsp) ndspExit();
    else if (backend_ == AudioBackend::Csnd) csndExit();
    backend_ = AudioBackend::None; poolCursors_.fill(0U); lastChannel_ = -1;
    ndspResult_ = ndspInitialResult_ = ndspShimResult_ = static_cast<Result>(kAudioResultNotAttempted);
    ndspShimAttempted_ = ndspShimActive_ = false;
    csndResult_ = lastPlayResult_ = probeResult_ = static_cast<Result>(kAudioResultNotAttempted);
    probeState_ = {}; diagnosticWaveBuffer_ = {}; musicWaveBuffer_ = {}; diagnosticSamplePosition_ = 0;
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
AudioWaveStatus AudioSystem::diagnosticWaveStatus() const { return audioWaveStatusFromRaw(diagnosticWaveBuffer_.status); }
std::uint32_t AudioSystem::diagnosticSamplePosition() const { return diagnosticSamplePosition_; }
std::uint32_t AudioSystem::diagnosticSampleCount() const { return static_cast<std::uint32_t>(diagnosticTone_.count); }

bool AudioSystem::generateSamples() {
    for (std::uint8_t v = 0; v < static_cast<std::uint8_t>(AudioCue::Count); ++v)
        if (!generateSample(static_cast<AudioCue>(v))) return false;
    return true;
}

bool AudioSystem::generateSample(AudioCue cue) {
    const CueShape shape = shapeFor(cue);
    const std::size_t count = std::max<std::size_t>(1U, static_cast<std::size_t>(shape.durationSeconds * kSampleRate));
    auto* data = static_cast<std::int16_t*>(linearAlloc(count * sizeof(std::int16_t)));
    if (data == nullptr) return false;
    float phase = 0.0F;
    for (std::size_t i = 0; i < count; ++i) {
        const float p = static_cast<float>(i) / static_cast<float>(count);
        const float frequency = shape.startFrequency + (shape.endFrequency - shape.startFrequency) * p;
        phase += 2.0F * kPi * frequency / kSampleRate;
        const float envelope = std::min(1.0F, p * 18.0F) * std::max(0.0F, 1.0F - p) * std::max(0.0F, 1.0F - p);
        const float oscillator = shape.squareWave ? (std::sin(phase) >= 0.0F ? 1.0F : -1.0F) : std::sin(phase);
        data[i] = static_cast<std::int16_t>(std::clamp(oscillator * envelope * shape.amplitude, -1.0F, 1.0F) * 32767.0F);
    }
    DSP_FlushDataCache(data, count * sizeof(std::int16_t));
    samples_[static_cast<std::size_t>(cue)] = {data, count}; return true;
}

bool AudioSystem::generateDiagnosticTone() {
    constexpr float duration = 2.0F, frequency = 880.0F, amplitude = 0.75F;
    const std::size_t frames = static_cast<std::size_t>(duration * kSampleRate);
    auto* data = static_cast<std::int16_t*>(linearAlloc(frames * 2U * sizeof(std::int16_t)));
    if (data == nullptr) return false;
    for (std::size_t f = 0; f < frames; ++f) {
        const float p = static_cast<float>(f) / static_cast<float>(frames);
        const float env = std::min(1.0F, p * 100.0F) * std::min(1.0F, (1.0F - p) * 100.0F);
        const std::int16_t s = static_cast<std::int16_t>(std::sin(2.0F * kPi * frequency * f / kSampleRate) * env * amplitude * 32767.0F);
        data[f * 2U] = s; data[f * 2U + 1U] = s;
    }
    DSP_FlushDataCache(data, frames * 2U * sizeof(std::int16_t)); diagnosticTone_ = {data, frames}; return true;
}

bool AudioSystem::generateMissionMusic() {
    constexpr float beatSeconds = 0.40F; constexpr std::size_t beatCount = 16U;
    constexpr std::array<int, beatCount> melody{-12,-7,-5,-7,-12,-7,-3,-7,-10,-5,-3,-5,-12,-7,-5,-3};
    constexpr std::array<int,4U> bass{-24,-22,-20,-19};
    const std::size_t samplesPerBeat = static_cast<std::size_t>(beatSeconds * kSampleRate);
    const std::size_t count = samplesPerBeat * beatCount;
    auto* data = static_cast<std::int16_t*>(linearAlloc(count * sizeof(std::int16_t)));
    if (data == nullptr) return false;
    float mp = 0.0F, bp = 0.0F;
    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t beat = i / samplesPerBeat;
        const float p = static_cast<float>(i % samplesPerBeat) / samplesPerBeat;
        mp += 2.0F * kPi * noteFrequency(melody[beat]) / kSampleRate;
        bp += 2.0F * kPi * noteFrequency(bass[beat / 4U]) / kSampleRate;
        const float env = std::min(1.0F, p * 18.0F) * std::min(1.0F, (1.0F - p) * 5.0F);
        data[i] = static_cast<std::int16_t>(std::clamp(std::sin(mp)*0.23F*env + std::sin(bp)*0.15F + std::sin(mp*2.0F)*0.04F*env, -0.55F, 0.55F) * 32767.0F);
    }
    DSP_FlushDataCache(data, count * sizeof(std::int16_t)); missionMusic_ = {data, count}; return true;
}

void AudioSystem::freeSamples() {
    for (Sample& sample : samples_) if (sample.data != nullptr) { linearFree(sample.data); sample = {}; }
    if (diagnosticTone_.data != nullptr) { linearFree(diagnosticTone_.data); diagnosticTone_ = {}; }
    if (missionMusic_.data != nullptr) { linearFree(missionMusic_.data); missionMusic_ = {}; }
}
