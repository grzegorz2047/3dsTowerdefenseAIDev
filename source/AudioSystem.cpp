#include "AudioSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

#include "AudioChannelPool.hpp"

namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kPreparationVolume = 0.20F;
constexpr float kCombatVolume = 0.24F;
constexpr float kAmbientVolume = 0.055F;

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

void setNdspMonoMix(int channel, float volume) {
    float mix[12]{};
    mix[0] = volume;
    mix[1] = volume;
    ndspChnSetMix(channel, mix);
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
        ndspShimAttempted_ = false;
        ndspShimActive_ = false;
        ndspShimResult_ = static_cast<Result>(kAudioResultNotAttempted);
        csndResult_ = csndInit();
        if (R_SUCCEEDED(csndResult_)) backend_ = AudioBackend::Csnd;
    }

    if (backend_ == AudioBackend::None) return false;
    if (!generateSamples() || !generateDiagnosticTone() || !generateMusicLayers()) {
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
        setNdspMonoMix(channel, 0.82F);
    }

    for (int channel : {kPreparationMusicChannel, kCombatMusicChannel, kAmbientChannel}) {
        ndspChnReset(channel);
        ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
        ndspChnSetRate(channel, static_cast<float>(kSampleRate));
        ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
        setNdspMonoMix(channel, 0.0F);
    }
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

void AudioSystem::startMusicLayers() {
    if (musicLayersStarted_ || backend_ == AudioBackend::None) return;
    const std::array<Sample*, 3U> samples{&preparationMusic_, &combatMusic_, &ambient_};
    const std::array<int, 3U> channels{kPreparationMusicChannel, kCombatMusicChannel, kAmbientChannel};
    for (std::size_t index = 0; index < samples.size(); ++index) {
        const Sample& sample = *samples[index];
        if (sample.data == nullptr || sample.count == 0U) return;
        if (backend_ == AudioBackend::Ndsp) {
            ndspWaveBuf& buffer = musicWaveBuffers_[index];
            std::memset(&buffer, 0, sizeof(buffer));
            buffer.data_vaddr = sample.data;
            buffer.nsamples = static_cast<u32>(sample.count);
            buffer.looping = true;
            ndspChnWaveBufAdd(channels[index], &buffer);
        } else {
            const u32 byteCount = static_cast<u32>(sample.count * sizeof(std::int16_t));
            const Result flushResult = CSND_FlushDataCache(sample.data, byteCount);
            if (R_FAILED(flushResult)) {
                lastPlayResult_ = flushResult;
                return;
            }
            lastPlayResult_ = csndPlaySound(
                channels[index],
                SOUND_ENABLE | SOUND_REPEAT | SOUND_FORMAT_16BIT | SOUND_LINEAR_INTERP,
                static_cast<u32>(kSampleRate),
                0.0F,
                0.0F,
                sample.data,
                sample.data,
                byteCount);
            if (R_FAILED(lastPlayResult_)) return;
        }
    }
    if (backend_ == AudioBackend::Csnd) lastPlayResult_ = csndExecCmds(true);
    musicLayersStarted_ = R_SUCCEEDED(lastPlayResult_) || backend_ == AudioBackend::Ndsp;
}

void AudioSystem::applyMusicMix() {
    const float prep = preparationGain_ * kPreparationVolume;
    const float combat = combatGain_ * kCombatVolume;
    const float ambient = ambientGain_ * kAmbientVolume;
    if (backend_ == AudioBackend::Ndsp) {
        setNdspMonoMix(kPreparationMusicChannel, prep);
        setNdspMonoMix(kCombatMusicChannel, combat);
        setNdspMonoMix(kAmbientChannel, ambient);
    } else if (backend_ == AudioBackend::Csnd && musicLayersStarted_) {
        CSND_SetVol(kPreparationMusicChannel, CSND_VOL(prep, 0.0F), 0U);
        CSND_SetVol(kCombatMusicChannel, CSND_VOL(combat, 0.0F), 0U);
        CSND_SetVol(kAmbientChannel, CSND_VOL(ambient, 0.0F), 0U);
        lastPlayResult_ = csndExecCmds(true);
    }
}

void AudioSystem::updateMusic(TutorialPhase phase, bool enabled, float deltaSeconds) {
    musicPhase_ = musicPhaseFor(phase);
    if (enabled) startMusicLayers();
    const MusicGainTargets targets = targetMusicGains(musicPhase_, enabled && musicLayersStarted_);
    preparationGain_ = approachMusicGain(preparationGain_, targets.preparation, deltaSeconds);
    combatGain_ = approachMusicGain(combatGain_, targets.combat, deltaSeconds);
    ambientGain_ = approachMusicGain(ambientGain_, targets.ambient, deltaSeconds);
    applyMusicMix();
}

void AudioSystem::stopMusic() {
    if (!musicLayersStarted_) return;
    if (backend_ == AudioBackend::Ndsp) {
        for (std::size_t index = 0; index < musicWaveBuffers_.size(); ++index) {
            ndspChnWaveBufClear(kPreparationMusicChannel + static_cast<int>(index));
            musicWaveBuffers_[index] = {};
        }
    } else if (backend_ == AudioBackend::Csnd) {
        for (int channel : {kPreparationMusicChannel, kCombatMusicChannel, kAmbientChannel}) {
            CSND_SetPlayStateR(channel, 0U);
        }
        lastPlayResult_ = csndExecCmds(true);
    }
    musicLayersStarted_ = false;
    musicPhase_ = MusicPhase::Silent;
    preparationGain_ = 0.0F;
    combatGain_ = 0.0F;
    ambientGain_ = 0.0F;
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
    if (R_FAILED(flushResult)) {
        lastPlayResult_ = flushResult;
        return;
    }

    lastPlayResult_ = csndPlaySound(
        channel,
        SOUND_ENABLE | SOUND_ONE_SHOT | SOUND_FORMAT_16BIT | SOUND_LINEAR_INTERP,
        static_cast<u32>(kSampleRate),
        0.82F,
        0.0F,
        sample.data,
        nullptr,
        byteCount);
    if (R_SUCCEEDED(lastPlayResult_)) lastPlayResult_ = csndExecCmds(true);
}

void AudioSystem::playMask(std::uint32_t cueMask) {
    constexpr std::array<AudioCue, 9U> order{
        AudioCue::BaseDamage,
        AudioCue::WaveStart,
        AudioCue::BuildSuccess,
        AudioCue::BuildFailure,
        AudioCue::EnemyDeath,
        AudioCue::Hit,
        AudioCue::Shot,
        AudioCue::Victory,
        AudioCue::Defeat,
    };
    for (AudioCue cue : order) {
        if ((cueMask & audioCueMask(cue)) != 0U) play(cue);
    }
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
    stopMusic();
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
    if (backend_ == AudioBackend::Ndsp) ndspExit();
    else if (backend_ == AudioBackend::Csnd) csndExit();

    backend_ = AudioBackend::None;
    poolCursors_.fill(0U);
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
    musicWaveBuffers_ = {};
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
MusicPhase AudioSystem::musicPhase() const { return musicPhase_; }

bool AudioSystem::generateSamples() {
    for (std::uint8_t value = 0; value < static_cast<std::uint8_t>(AudioCue::Count); ++value) {
        if (!generateSample(static_cast<AudioCue>(value))) return false;
    }
    return true;
}

bool AudioSystem::generateSample(AudioCue cue) {
    const CueShape shape = shapeFor(cue);
    const std::size_t count = std::max<std::size_t>(
        1U,
        static_cast<std::size_t>(shape.durationSeconds * static_cast<float>(kSampleRate)));
    auto* data = static_cast<std::int16_t*>(linearAlloc(count * sizeof(std::int16_t)));
    if (data == nullptr) return false;

    float phase = 0.0F;
    for (std::size_t index = 0; index < count; ++index) {
        const float progress = static_cast<float>(index) / static_cast<float>(count);
        const float frequency = shape.startFrequency +
            (shape.endFrequency - shape.startFrequency) * progress;
        phase += 2.0F * kPi * frequency / static_cast<float>(kSampleRate);
        const float release = std::max(0.0F, 1.0F - progress);
        const float envelope = std::min(1.0F, progress * 18.0F) * release * release;
        const float oscillator = shape.squareWave
            ? (std::sin(phase) >= 0.0F ? 1.0F : -1.0F)
            : std::sin(phase);
        data[index] = static_cast<std::int16_t>(
            std::clamp(oscillator * envelope * shape.amplitude, -1.0F, 1.0F) * 32767.0F);
    }

    DSP_FlushDataCache(data, count * sizeof(std::int16_t));
    samples_[static_cast<std::size_t>(cue)] = {data, count};
    return true;
}

bool AudioSystem::generateDiagnosticTone() {
    constexpr float kDurationSeconds = 2.0F;
    constexpr float kFrequency = 880.0F;
    constexpr float kAmplitude = 0.75F;
    const std::size_t frameCount = static_cast<std::size_t>(
        kDurationSeconds * static_cast<float>(kSampleRate));
    auto* data = static_cast<std::int16_t*>(linearAlloc(
        frameCount * 2U * sizeof(std::int16_t)));
    if (data == nullptr) return false;

    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        const float progress = static_cast<float>(frame) / static_cast<float>(frameCount);
        const float envelope = std::min(1.0F, progress * 100.0F) *
            std::min(1.0F, (1.0F - progress) * 100.0F);
        const float phase = 2.0F * kPi * kFrequency * static_cast<float>(frame) /
            static_cast<float>(kSampleRate);
        const std::int16_t sample = static_cast<std::int16_t>(
            std::sin(phase) * envelope * kAmplitude * 32767.0F);
        data[frame * 2U] = sample;
        data[frame * 2U + 1U] = sample;
    }

    DSP_FlushDataCache(data, frameCount * 2U * sizeof(std::int16_t));
    diagnosticTone_ = {data, frameCount};
    return true;
}

bool AudioSystem::generateMusicLayer(Sample& destination, bool combat) {
    constexpr std::size_t kBeatCount = 16U;
    constexpr std::array<int, kBeatCount> preparationMelody{
        -12, -7, -5, -7, -12, -7, -3, -7,
        -10, -5, -3, -5, -12, -7, -5, -3,
    };
    constexpr std::array<int, kBeatCount> combatMelody{
        -12, -5, 0, -3, -10, -3, 2, -1,
        -8, -1, 3, 0, -10, -3, 2, 5,
    };
    constexpr std::array<int, 4U> bass{-24, -22, -20, -19};
    const float secondsPerBeat = combat ? 0.28F : 0.40F;
    const std::size_t samplesPerBeat = static_cast<std::size_t>(
        secondsPerBeat * static_cast<float>(kSampleRate));
    const std::size_t count = samplesPerBeat * kBeatCount;
    auto* data = static_cast<std::int16_t*>(linearAlloc(count * sizeof(std::int16_t)));
    if (data == nullptr) return false;

    float melodyPhase = 0.0F;
    float bassPhase = 0.0F;
    for (std::size_t index = 0; index < count; ++index) {
        const std::size_t beat = index / samplesPerBeat;
        const float beatProgress = static_cast<float>(index % samplesPerBeat) /
            static_cast<float>(samplesPerBeat);
        const float loopProgress = static_cast<float>(index) / static_cast<float>(count - 1U);
        const int note = combat ? combatMelody[beat] : preparationMelody[beat];
        melodyPhase += 2.0F * kPi * noteFrequency(note) / static_cast<float>(kSampleRate);
        bassPhase += 2.0F * kPi * noteFrequency(bass[beat / 4U]) / static_cast<float>(kSampleRate);
        const float beatEnvelope = std::min(1.0F, beatProgress * 16.0F) *
            std::min(1.0F, (1.0F - beatProgress) * 5.0F);
        const float loopEnvelope = std::sin(kPi * loopProgress);
        const float percussion = combat && beatProgress < 0.08F
            ? (1.0F - beatProgress / 0.08F) * std::sin(melodyPhase * 0.35F) * 0.09F
            : 0.0F;
        const float sample = std::clamp((
            std::sin(melodyPhase) * (combat ? 0.25F : 0.20F) * beatEnvelope +
            std::sin(bassPhase) * (combat ? 0.18F : 0.13F) +
            percussion) * loopEnvelope * loopEnvelope,
            -0.58F,
            0.58F);
        data[index] = static_cast<std::int16_t>(sample * 32767.0F);
    }
    data[0] = 0;
    data[count - 1U] = 0;
    DSP_FlushDataCache(data, count * sizeof(std::int16_t));
    destination = {data, count};
    return true;
}

bool AudioSystem::generateAmbientLayer() {
    constexpr float kDurationSeconds = 5.0F;
    const std::size_t count = static_cast<std::size_t>(kDurationSeconds * kSampleRate);
    auto* data = static_cast<std::int16_t*>(linearAlloc(count * sizeof(std::int16_t)));
    if (data == nullptr) return false;
    for (std::size_t index = 0; index < count; ++index) {
        const float progress = static_cast<float>(index) / static_cast<float>(count - 1U);
        const float loopEnvelope = std::sin(kPi * progress);
        const float wind = std::sin(2.0F * kPi * 43.0F * progress) * 0.10F +
            std::sin(2.0F * kPi * 71.0F * progress) * 0.05F;
        const float distantBird = std::sin(2.0F * kPi * 7.0F * progress) > 0.985F
            ? std::sin(2.0F * kPi * 820.0F * static_cast<float>(index) / kSampleRate) * 0.08F
            : 0.0F;
        const float sample = (wind + distantBird) * loopEnvelope * loopEnvelope;
        data[index] = static_cast<std::int16_t>(std::clamp(sample, -0.25F, 0.25F) * 32767.0F);
    }
    data[0] = 0;
    data[count - 1U] = 0;
    DSP_FlushDataCache(data, count * sizeof(std::int16_t));
    ambient_ = {data, count};
    return true;
}

bool AudioSystem::generateMusicLayers() {
    return generateMusicLayer(preparationMusic_, false) &&
        generateMusicLayer(combatMusic_, true) &&
        generateAmbientLayer();
}

void AudioSystem::freeSamples() {
    for (Sample& sample : samples_) {
        if (sample.data != nullptr) {
            linearFree(sample.data);
            sample = {};
        }
    }
    for (Sample* sample : {&diagnosticTone_, &preparationMusic_, &combatMusic_, &ambient_}) {
        if (sample->data != nullptr) {
            linearFree(sample->data);
            *sample = {};
        }
    }
}
