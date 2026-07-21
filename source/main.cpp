#include <3ds.h>
#include <citro3d.h>

#include "Camera.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Renderer.hpp"
#include "Wave.hpp"

namespace {

constexpr float kMaximumDeltaSeconds = 1.0F / 15.0F;
constexpr float kRestartDelaySeconds = 1.5F;

float calculateDeltaSeconds(u64 nowMilliseconds, u64 previousMilliseconds) {
    if (previousMilliseconds == 0U || nowMilliseconds <= previousMilliseconds) {
        return 1.0F / 60.0F;
    }

    const float elapsed = static_cast<float>(nowMilliseconds - previousMilliseconds) / 1000.0F;
    return elapsed < kMaximumDeltaSeconds ? elapsed : kMaximumDeltaSeconds;
}

int shutdownWithError() {
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 1;
}

}  // namespace

int main() {
    gfxInitDefault();
    if (romfsInit() != 0) {
        gfxExit();
        return 1;
    }

    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        romfsExit();
        gfxExit();
        return 1;
    }

    const LevelLoadResult levelResult = LevelLoader::loadFromRomFs("romfs:/levels/tutorial.lvl");
    if (!levelResult.success) {
        return shutdownWithError();
    }

    Renderer renderer;
    if (!renderer.initialize(levelResult.level)) {
        renderer.shutdown();
        return shutdownWithError();
    }

    InputSystem inputSystem;
    Camera camera;
    Wave wave(levelResult.level);
    float restartTimer = 0.0F;
    u64 previousMilliseconds = osGetTime();

    while (aptMainLoop()) {
        const InputSnapshot input = inputSystem.poll();
        if (input.pressed(KEY_START)) {
            break;
        }

        const u64 nowMilliseconds = osGetTime();
        const float deltaSeconds = calculateDeltaSeconds(nowMilliseconds, previousMilliseconds);
        previousMilliseconds = nowMilliseconds;

        camera.update(input, deltaSeconds);
        if (wave.completed() || wave.lost()) {
            restartTimer += deltaSeconds;
            if (restartTimer >= kRestartDelaySeconds) {
                wave.reset();
                restartTimer = 0.0F;
            }
        } else {
            wave.update(deltaSeconds);
        }
        renderer.render(camera, wave);
    }

    renderer.shutdown();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
