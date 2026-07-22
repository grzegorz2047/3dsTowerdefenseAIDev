#include "AudioEvents.hpp"

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

    AudioEventRouter router;
    require(router.update({TutorialPhase::BuildFirstTower, 0}) == 0U, "first frame initializes without sound");
    require(router.update({TutorialPhase::BuildFirstTower, 0}) == 0U, "unchanged frame stays silent");

    std::uint32_t cues = router.update({TutorialPhase::ReadyToStart, 0});
    require(cues == 0U, "ready state does not start combat sound");

    cues = router.update({TutorialPhase::WaveRunning, 0});
    require(hasCue(cues, AudioCue::WaveStart), "wave transition emits start cue");
    require(router.update({TutorialPhase::WaveRunning, 0}) == 0U, "wave cue is not repeated");

    cues = router.update({TutorialPhase::WaveRunning, 2});
    require(hasCue(cues, AudioCue::Shot), "projectile increase emits shot cue");
    require(!hasCue(cues, AudioCue::Hit), "projectile increase is not a hit");

    cues = router.update({TutorialPhase::WaveRunning, 1});
    require(hasCue(cues, AudioCue::Hit), "projectile decrease emits hit cue");

    cues = router.update({TutorialPhase::Victory, 0});
    require(hasCue(cues, AudioCue::Victory), "victory transition emits cue");
    require(hasCue(cues, AudioCue::Hit), "remaining projectile resolution can coexist with victory");
    require(router.update({TutorialPhase::Victory, 0}) == 0U, "victory cue is not repeated");

    router.reset({TutorialPhase::BuildFirstTower, 0});
    cues = router.update({TutorialPhase::Defeat, 0});
    require(hasCue(cues, AudioCue::Defeat), "defeat transition emits cue");

    std::cout << "Audio event tests passed.\n";
    return 0;
}
