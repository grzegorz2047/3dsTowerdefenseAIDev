#pragma once

#include <cstdarg>
#include <cstdio>

#include "Renderer.hpp"

namespace std {
int citadelConsolePrintf(const char* format, ...);
}

bool citadelGraphicsInit(std::size_t commandBufferSize);
void citadelGraphicsFini();
void citadelSwapBuffers();
void drawTopGoldOverlay(int gold);

#define printf citadelConsolePrintf
#define C3D_Init(commandBufferSize) citadelGraphicsInit(commandBufferSize)
#define C3D_Fini() citadelGraphicsFini()
#define gfxSwapBuffers() citadelSwapBuffers()
#define render(camera, wave, buildSystem, tutorialFlow, stereoEnabled, maximum3DDepthPercent) \
    render(camera, wave, buildSystem, tutorialFlow, stereoEnabled, maximum3DDepthPercent), \
    drawTopGoldOverlay(buildSystem.gold())
