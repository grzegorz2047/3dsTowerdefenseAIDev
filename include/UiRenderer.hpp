#pragma once

#include <citro2d.h>
#include <citro3d.h>

#include "UiState.hpp"

class UiRenderer {
public:
    UiRenderer() = default;
    ~UiRenderer();

    UiRenderer(const UiRenderer&) = delete;
    UiRenderer& operator=(const UiRenderer&) = delete;

    [[nodiscard]] bool initialize();
    void shutdown();
    void beginFrame();

    void renderStandalone(const UiState& state);
    void renderBottom(const UiState& state);
    void renderTopOverlay(C3D_RenderTarget* target, const UiState& state);

private:
    void drawCampaign(const UiState& state);
    void drawMission(const UiState& state);
    void drawDiagnostics(const UiState& state);
    void drawText(const char* text, float x, float y, float scale, u32 color);
    void drawButton(float x, float y, float width, float height, const char* label, bool selected);

    C3D_RenderTarget* bottomTarget_ = nullptr;
    C2D_TextBuf textBuffer_ = nullptr;
};
