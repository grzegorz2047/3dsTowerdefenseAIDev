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
    require(audioPriority(AudioCue::Victory) == AudioPriority::Critical, "victory uses critical priority");
    require(audioPriority(AudioCue::BaseDamage) == AudioPriority::Critical, "base damage uses protected priority");
    require(audioPriority(AudioCue::EnemyDeath) == AudioPriority::Combat, "enemy death uses combat priority");

    AudioEventRouter router;
    require(router.update({TutorialPhase::BuildFirstTower, 0, 0, 5}) == 0U, "first frame initializes without sound");
    require(router.update({TutorialPhase::BuildFirstTower, 0, 0, 5}) == 0U, "unchanged frame stays silent");

    std::uint32_t cues = router.update({TutorialPhase::WaveRunning, 0, 0, 5});
    require(hasCue(cues, AudioCue::WaveStart), "wave transition emits start cue");

    cues = router.update({TutorialPhase::WaveRunning, 2, 0, 5});
    require(hasCue(cues, AudioCue::Shot), "projectile increase emits shot cue");
    cues = router.update({TutorialPhase::WaveRunning, 3, 0, 5}, 0.01F);
    require(!hasCue(cues, AudioCue::Shot), "rapid duplicate shot is suppressed");

    cues = router.update({TutorialPhase::WaveRunning, 1, 1, 5}, 0.10F);
    require(hasCue(cues, AudioCue::Hit), "projectile decrease emits hit cue");
    require(hasCue(cues, AudioCue::EnemyDeath), "defeated count increase emits death cue");

    cues = router.update({TutorialPhase::WaveRunning, 1, 2, 5}, 0.01F);
    require(!hasCue(cues, AudioCue::EnemyDeath), "rapid duplicate death is suppressed");
    cues = router.update({TutorialPhase::WaveRunning, 1, 3, 5}, 0.12F);
    require(hasCue(cues, AudioCue::EnemyDeath), "death cue returns after cooldown");

    cues = router.update({TutorialPhase::WaveRunning, 1, 3, 4}, 0.10F);
    require(hasCue(cues, AudioCue::BaseDamage), "base health loss emits protected cue");
    cues = router.update({TutorialPhase::WaveRunning, 1, 3, 3}, 0.01F);
    require(!hasCue(cues, AudioCue::BaseDamage), "rapid duplicate base hit is suppressed");

    cues = router.update({TutorialPhase::Victory, 0, 3, 3}, 0.20F);
    require(hasCue(cues, AudioCue::Victory), "victory transition emits cue");
    require(router.update({TutorialPhase::Victory, 0, 3, 3}) == 0U, "victory cue is not repeated");

    router.reset({TutorialPhase::BuildFirstTower, 0, 0, 5});
    cues = router.update({TutorialPhase::Defeat, 0, 0, 0});
    require(hasCue(cues, AudioCue::Defeat), "defeat transition emits cue");
    require(hasCue(cues, AudioCue::BaseDamage), "final base loss coexists with defeat");

    std::cout << "Audio event tests passed.\n";
    return 0;
}
