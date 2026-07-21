#include <3ds.h>
#include <citro3d.h>

#include "Camera.hpp"
#include "Input.hpp"
#include "Renderer.hpp"

namespace {

constexpr float kMaximumDeltaSeconds = 1.0F / 15.0F;

float calculateDeltaSeconds(u64 nowMilliseconds, u64 previousMilliseconds) {
    if (previousMilliseconds == 0U || nowMilliseconds <= previousMilliseconds) {
        return 1.0F / 60.0F;
    }

    const float elapsed = static_cast<float>(nowMilliseconds - previousMilliseconds) / 1000.0F;
    return elapsed < kMaximumDeltaSeconds ? elapsed : kMaximumDeltaSeconds;
}

}  // namespace

int main() {
    gfxInitDefault();
    romfsInit();

    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        romfsExit();
        gfxExit();
        return 1;
    }

    Renderer renderer;
    if (!renderer.initialize()) {
        renderer.shutdown();
        C3D_Fini();
        romfsExit();
        gfxExit();
        return 1;
    }

    InputSystem inputSystem;
    Camera camera;
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
        renderer.render(camera);
    }

    renderer.shutdown();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
