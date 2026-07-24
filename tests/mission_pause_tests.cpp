#include <cassert>
#include "MissionPause.hpp"
int main() {
    assert(MissionPause::toggled(false, false, false));
    assert(!MissionPause::toggled(true, false, false));
    assert(!MissionPause::toggled(false, true, false));
    assert(!MissionPause::toggled(false, false, true));
    assert(MissionPause::gameplayInputAllowed(false));
    assert(!MissionPause::gameplayInputAllowed(true));
    assert(MissionPause::simulationSeconds(1.0F / 60.0F, 1, false) == 1.0F / 60.0F);
    assert(MissionPause::simulationSeconds(1.0F / 60.0F, 2, false) == 2.0F / 60.0F);
    assert(MissionPause::simulationSeconds(10.0F, 2, true) == 0.0F);
    assert(MissionPause::simulationSeconds(-1.0F, 2, false) == 0.0F);
    assert(MissionPause::simulationSeconds(1.0F, 0, false) == 0.0F);
    return 0;
}
