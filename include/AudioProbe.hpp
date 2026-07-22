#pragma once

struct AudioProbeState {
    bool querySucceeded = false;
    bool active = false;
    bool everActive = false;
};

[[nodiscard]] constexpr AudioProbeState updateAudioProbeState(
    AudioProbeState previous,
    bool querySucceeded,
    bool active) {
    return {
        querySucceeded,
        querySucceeded && active,
        previous.everActive || (querySucceeded && active),
    };
}
