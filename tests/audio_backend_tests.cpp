#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include "AudioBackend.hpp"
#include "AudioMusicPolicy.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    expect(selectAudioBackend(true, true) == AudioBackend::Ndsp, "NDSP must have priority when both backends are available");
    expect(selectAudioBackend(true, false) == AudioBackend::Ndsp, "NDSP must be selected when available");
    expect(selectAudioBackend(false, true) == AudioBackend::Csnd, "CSND must be selected when NDSP fails");
    expect(selectAudioBackend(false, false) == AudioBackend::None, "audio must report no backend when both services fail");

    expect(std::string_view(audioBackendName(AudioBackend::Ndsp)) == "NDSP", "NDSP name must be stable");
    expect(std::string_view(audioBackendName(AudioBackend::Csnd)) == "CSND", "CSND name must be stable");
    expect(std::string_view(audioBackendName(AudioBackend::None)) == "BRAK", "missing backend must be visible");

    expect(musicPhaseFor(TutorialPhase::BuildFirstTower) == MusicPhase::Preparation,
        "building uses preparation music");
    expect(musicPhaseFor(TutorialPhase::ReadyToStart) == MusicPhase::Preparation,
        "ready state keeps preparation music");
    expect(musicPhaseFor(TutorialPhase::WaveRunning) == MusicPhase::Combat,
        "wave uses combat music");
    expect(musicPhaseFor(TutorialPhase::Victory) == MusicPhase::Silent,
        "victory fades music for the result cue");
    expect(musicPhaseFor(TutorialPhase::Defeat) == MusicPhase::Silent,
        "defeat fades music for the result cue");

    const MusicGainTargets prep = targetMusicGains(MusicPhase::Preparation, true);
    expect(prep.preparation == 1.0F && prep.combat == 0.0F && prep.ambient == 1.0F,
        "preparation enables calm music and ambient");
    const MusicGainTargets combat = targetMusicGains(MusicPhase::Combat, true);
    expect(combat.preparation == 0.0F && combat.combat == 1.0F && combat.ambient == 1.0F,
        "combat enables intense music and ambient");
    const MusicGainTargets muted = targetMusicGains(MusicPhase::Combat, false);
    expect(muted.preparation == 0.0F && muted.combat == 0.0F && muted.ambient == 0.0F,
        "music mute does not depend on SFX state");

    float gain = 0.0F;
    gain = approachMusicGain(gain, 1.0F, 0.225F);
    expect(std::fabs(gain - 0.5F) < 0.0001F, "half fade duration reaches half gain");
    gain = approachMusicGain(gain, 1.0F, 0.225F);
    expect(std::fabs(gain - 1.0F) < 0.0001F, "full fade duration reaches target gain");
    gain = approachMusicGain(gain, 0.0F, 0.45F);
    expect(std::fabs(gain) < 0.0001F, "fade out reaches silence without overshoot");

    expect(shouldStartMissionMusic(AudioBackend::Ndsp, false, true), "NDSP should start available mission music");
    expect(shouldStartMissionMusic(AudioBackend::Csnd, false, true), "CSND fallback should start available mission music");
    expect(!shouldStartMissionMusic(AudioBackend::None, false, true), "missing backend must not start music");
    expect(!shouldStartMissionMusic(AudioBackend::Csnd, true, true), "active CSND music must not restart");
    expect(!shouldStartMissionMusic(AudioBackend::Ndsp, false, false), "missing samples must not start music");
    expect(usesCsndMusicLoop(AudioBackend::Csnd), "CSND must use the hardware loop path");
    expect(!usesCsndMusicLoop(AudioBackend::Ndsp), "NDSP must keep its wave-buffer loop path");

    std::cout << "Audio backend and music policy tests passed\n";
    return 0;
}
