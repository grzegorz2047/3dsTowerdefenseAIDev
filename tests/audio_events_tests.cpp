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
    require(audioPriority(AudioCue::Victory) == AudioPriority::Critical, "victory uses critical channel pool");
    require(audioPriority(AudioCue::BuildSuccess) == AudioPriority::Action, "build uses action channel pool");
    require(audioPriority(AudioCue::Shot) == AudioPriority::Combat, "shot uses combat channel pool");

    AudioEventRouter router;
    require(router.update({TutorialPhase::BuildFirstTower, 0, 0, 5}) == 0U, "first frame initializes without sound");
    require(router.update({TutorialPhase::BuildFirstTower, 0, 0, 5}) == 0U, "unchanged frame stays silent");

    std::uint32_t cues = router.update({TutorialPhase::ReadyToStart, 0, 0, 5});
    require(cues == 0U, "ready state does not start combat sound");

    cues = router.update({TutorialPhase::WaveRunning, 0, 0, 5});
    require(hasCue(cues, AudioCue::WaveStart), "wave transition emits start cue");
    require(router.update({TutorialPhase::WaveRunning, 0, 0, 5}) == 0U, "wave cue is not repeated");

    cues = router.update({TutorialPhase::WaveRunning, 2, 0, 5});
    require(hasCue(cues, AudioCue::Shot), "projectile increase emits shot cue");
    require(!hasCue(cues, AudioCue::Hit), "projectile increase is not a hit");

    cues = router.update({TutorialPhase::WaveRunning, 3, 0, 5}, 0.01F);
    require(!hasCue(cues, AudioCue::Shot), "rapid duplicate shot is suppressed");
    cues = router.update({TutorialPhase::WaveRunning, 4, 0, 5}, 0.10F);
    require(hasCue(cues, AudioCue::Shot), "shot returns after cooldown");

    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 5}, 0.10F);
    require(hasCue(cues, AudioCue::Hit), "projectile decrease emits hit cue");
    require(hasCue(cues, AudioCue::EnemyDeath), "defeated enemy emits death cue");

    cues = router.update({TutorialPhase::WaveRunning, 2, 1, 4}, 0.10F);
    require(hasCue(cues, AudioCue::BaseDamage), "base health loss emits protected cue");

    cues = router.update({TutorialPhase::Victory, 0, 1, 4}, 0.10F);
    require(hasCue(cues, AudioCue::Victory), "victory transition emits cue");
    require(router.update({TutorialPhase::Victory, 0, 1, 4}) == 0U, "victory cue is not repeated");

    router.reset({TutorialPhase::BuildFirstTower, 0, 0, 5});
    cues = router.update({TutorialPhase::Defeat, 0, 0, 0});
    require(hasCue(cues, AudioCue::Defeat), "defeat transition emits cue");
    require(hasCue(cues, AudioCue::BaseDamage), "final base damage can coexist with defeat");

    std::cout << "Audio event tests passed.\n";
    return 0;
}
