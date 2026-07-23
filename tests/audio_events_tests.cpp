#include "AudioEvents.hpp"
#include "AudioPolicy.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

bool hasCue(std::uint32_t mask, AudioCue cue) {
    return (mask & audioCueMask(cue)) != 0U;
}

}  // namespace

int main() {
    require(cueForBuildResult(BuildAttemptResult::Built) == AudioCue::BuildSuccess, "successful build has success cue");
    require(cueForBuildResult(BuildAttemptResult::Occupied) == AudioCue::BuildFailure, "failed build has failure cue");
    require(audioPriority(AudioCue::Victory) == AudioPriority::Critical, "victory uses protected priority");
    require(audioPriority(AudioCue::Defeat) == AudioPriority::Critical, "defeat uses protected priority");
    require(audioPriority(AudioCue::BaseDamage) == AudioPriority::Action, "base damage cannot replace mission result");
    require(audioPriority(AudioCue::EnemyDeath) == AudioPriority::Action, "enemy death has a dedicated action channel");
    require(audioCooldownSeconds(AudioCue::EnemyDeath) == 0.0F, "death events are never dropped by cooldown");
    require(audioCooldownSeconds(AudioCue::BaseDamage) == 0.0F, "base events are never dropped by cooldown");

    AudioEventRouter router;
    require(router.update({TutorialPhase::BuildFirstTower, 0, 0, 0, 0}) == 0U,
        "first frame initializes without sound");
    require(router.update({TutorialPhase::BuildFirstTower, 0, 0, 0, 0}) == 0U,
        "unchanged counters stay silent");

    std::uint32_t cues = router.update({TutorialPhase::WaveRunning, 0, 0, 0, 0});
    require(hasCue(cues, AudioCue::WaveStart), "wave transition emits start cue");

    cues = router.update({TutorialPhase::WaveRunning, 1, 0, 0, 0});
    require(hasCue(cues, AudioCue::Shot), "shot counter increase emits shot cue");
    cues = router.update({TutorialPhase::WaveRunning, 2, 0, 0, 0}, 0.01F);
    require(!hasCue(cues, AudioCue::Shot), "rapid shot series is rate limited");

    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 0, 0}, 0.10F);
    require(hasCue(cues, AudioCue::Hit), "impact counter increase emits hit cue");
    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 0, 0}, 0.10F);
    require(!hasCue(cues, AudioCue::Hit), "unchanged impact counter does not fake a hit");

    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 1, 0}, 0.01F);
    require(hasCue(cues, AudioCue::EnemyDeath), "death counter increase emits death cue");
    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 2, 0}, 0.01F);
    require(hasCue(cues, AudioCue::EnemyDeath), "consecutive death event is not suppressed");

    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 2, 1}, 0.01F);
    require(hasCue(cues, AudioCue::BaseDamage), "base event counter emits a dedicated cue");
    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 2, 2}, 0.01F);
    require(hasCue(cues, AudioCue::BaseDamage), "consecutive base event is not suppressed");

    cues = router.update({TutorialPhase::Victory, 2, 1, 2, 2}, 0.20F);
    require(hasCue(cues, AudioCue::Victory), "victory transition emits cue");
    require(router.update({TutorialPhase::Victory, 2, 1, 2, 2}) == 0U,
        "victory cue is not repeated");

    router.reset({TutorialPhase::BuildFirstTower, 0, 0, 0, 0});
    cues = router.update({TutorialPhase::Defeat, 0, 0, 0, 1});
    require(hasCue(cues, AudioCue::Defeat), "defeat transition emits cue");
    require(hasCue(cues, AudioCue::BaseDamage), "final base event coexists with defeat");

    std::cout << "Audio event tests passed.\n";
    return 0;
}
