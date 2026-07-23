#include "AudioChannelPool.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    require(AudioChannelPool::slotFor(AudioCue::Shot, 0U) == 0U, "combat pool starts at slot 0");
    require(AudioChannelPool::slotFor(AudioCue::Hit, 2U) == 2U, "combat pool owns slots 0-2");
    require(AudioChannelPool::slotFor(AudioCue::Shot, 3U) == 0U, "combat pool wraps after three slots");

    require(AudioChannelPool::slotFor(AudioCue::BuildSuccess, 0U) == 3U, "action pool starts at slot 3");
    require(AudioChannelPool::slotFor(AudioCue::WaveStart, 1U) == 4U, "action pool owns slots 3-4");
    require(AudioChannelPool::slotFor(AudioCue::BuildFailure, 2U) == 3U, "action pool wraps after two slots");

    require(AudioChannelPool::slotFor(AudioCue::Victory, 0U) == 5U, "critical pool owns protected slot 5");
    require(AudioChannelPool::slotFor(AudioCue::Defeat, 99U) == 5U, "critical pool never leaves protected slot");

    require(AudioChannelPool::nextCursor(AudioCue::Shot, 2U) == 0U, "combat cursor wraps");
    require(AudioChannelPool::nextCursor(AudioCue::BuildSuccess, 1U) == 0U, "action cursor wraps");
    require(AudioChannelPool::nextCursor(AudioCue::Victory, 0U) == 0U, "critical cursor remains fixed");

    std::cout << "Audio channel pool tests passed.\n";
    return 0;
}
