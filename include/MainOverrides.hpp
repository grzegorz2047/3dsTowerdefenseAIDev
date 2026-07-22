#pragma once

#include <cstdarg>
#include <cstdio>

#include "Renderer.hpp"

namespace std {
int citadelConsolePrintf(const char* format, ...);
}

void drawTopGoldOverlay(int gold);

#define printf citadelConsolePrintf
#define render(camera, wave, buildSystem, tutorialFlow, stereoEnabled, maximum3DDepthPercent) \
    render(camera, wave, buildSystem, tutorialFlow, stereoEnabled, maximum3DDepthPercent), \
    drawTopGoldOverlay(buildSystem.gold())
