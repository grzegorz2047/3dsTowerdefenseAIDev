#include "UiRenderer.hpp"

#include <cstdio>

#include "TouchUiLayout.hpp"

void UiRenderer::renderNarrative(const UiState& state) {
    if (topTarget_ == nullptr || bottomTarget_ == nullptr || textBuffer_ == nullptr) return;

    const u32 background = C2D_Color32(12, 18, 28, 255);
    const u32 panel = C2D_Color32(20, 29, 40, 255);
    const u32 panelStrong = C2D_Color32(24, 43, 62, 255);
    const u32 text = C2D_Color32(236, 240, 244, 255);
    const u32 accent = C2D_Color32(112, 225, 255, 255);
    const u32 gold = C2D_Color32(255, 222, 112, 255);

    gfxSet3D(false);
    beginFrame();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    C2D_Prepare();
    C2D_TargetClear(topTarget_, background);
    C2D_SceneBegin(topTarget_);
    C2D_DrawRectSolid(16.0F, 14.0F, 0.1F, 368.0F, 212.0F, panel);
    C2D_DrawRectSolid(16.0F, 14.0F, 0.2F, 368.0F, 42.0F, panelStrong);
    drawText("OSTATNIA CYTADELA", 76.0F, 25.0F, 0.72F, accent);
    drawWrappedText(state.title, 28.0F, 76.0F, 0.66F, gold, 28U, 2U);
    drawText(state.narrativeSpeaker, 28.0F, 126.0F, 0.48F, accent);
    drawWrappedText(state.narrativeText, 28.0F, 151.0F, 0.48F, text, 38U, 4U);
    C2D_Flush();

    C2D_Prepare();
    C2D_TargetClear(bottomTarget_, background);
    C2D_SceneBegin(bottomTarget_);
    C2D_DrawRectSolid(8.0F, 18.0F, 0.1F, 304.0F, 166.0F, panel);
    drawText("ODPRAWA", 106.0F, 31.0F, 0.66F, accent);
    drawWrappedText(state.narrativeText, 18.0F, 67.0F, 0.48F, text, 34U, 6U);
    char page[24]{};
    std::snprintf(page, sizeof(page), "%zu/%zu", state.narrativePage + 1U, state.narrativePageCount);
    drawText(page, 142.0F, 178.0F, 0.44F, gold);
    const TouchRect back = TouchUiLayout::rectFor(TouchUiAction::NarrativeBack);
    const TouchRect next = TouchUiLayout::rectFor(TouchUiAction::NarrativeNext);
    const TouchRect skip = TouchUiLayout::rectFor(TouchUiAction::NarrativeSkip);
    drawButton(back.x, back.y, back.width, back.height,
        state.narrativeCanGoBack ? "WSTECZ" : "ZAMKNIJ", false);
    drawButton(next.x, next.y, next.width, next.height, "DALEJ", true);
    drawButton(skip.x, skip.y, skip.width, skip.height, "POMIN", false);
    C2D_Flush();

    C3D_FrameEnd(0);
    composedFrameActive_ = false;
}
