#include "UiRenderer.hpp"

#include <algorithm>
#include <cstdio>

#include "TouchUiLayout.hpp"

namespace {

const u32 kBackground = C2D_Color32(12, 18, 28, 255);
const u32 kPanel = C2D_Color32(20, 29, 40, 255);
const u32 kPanelStrong = C2D_Color32(24, 43, 62, 255);
const u32 kText = C2D_Color32(236, 240, 244, 255);
const u32 kMuted = C2D_Color32(164, 178, 191, 255);
const u32 kAccent = C2D_Color32(112, 225, 255, 255);
const u32 kGold = C2D_Color32(255, 222, 112, 255);
const u32 kDanger = C2D_Color32(255, 132, 132, 255);
const u32 kSelected = C2D_Color32(48, 91, 119, 255);

const char* towerLabel(TowerType type) {
    switch (type) {
        case TowerType::Ballista: return "KUSZA";
        case TowerType::Mortar: return "MOZDZIERZ";
        case TowerType::Frost: return "MROZ";
        default: return "WIEZA";
    }
}

}  // namespace

UiRenderer::~UiRenderer() { shutdown(); }

bool UiRenderer::initialize() {
    shutdown();
    bottomTarget_ = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    textBuffer_ = C2D_TextBufNew(4096);
    if (bottomTarget_ == nullptr || textBuffer_ == nullptr) {
        shutdown();
        return false;
    }
    return true;
}

void UiRenderer::shutdown() {
    if (textBuffer_ != nullptr) {
        C2D_TextBufDelete(textBuffer_);
        textBuffer_ = nullptr;
    }
    bottomTarget_ = nullptr;
    composedFrameActive_ = false;
}

void UiRenderer::beginFrame() {
    if (textBuffer_ != nullptr) C2D_TextBufClear(textBuffer_);
    composedFrameActive_ = true;
}

void UiRenderer::drawText(const char* text, float x, float y, float scale, u32 color) {
    if (text == nullptr || text[0] == '\0' || textBuffer_ == nullptr) return;
    C2D_Text parsed{};
    C2D_TextParse(&parsed, textBuffer_, text);
    C2D_TextOptimize(&parsed);
    C2D_DrawText(&parsed, C2D_WithColor, x, y, 0.5F, scale, scale, color);
}

void UiRenderer::drawButton(float x, float y, float width, float height, const char* label, bool selected) {
    C2D_DrawRectSolid(x, y, 0.1F, width, height, selected ? kSelected : kPanelStrong);
    drawText(label, x + 7.0F, y + height * 0.36F, 0.44F, selected ? kGold : kText);
}

void UiRenderer::renderStandalone(const UiState& state) {
    if (bottomTarget_ == nullptr || textBuffer_ == nullptr) return;
    beginFrame();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    renderBottom(state);
    C3D_FrameEnd(0);
}

void UiRenderer::renderBottom(const UiState& state) {
    if (bottomTarget_ == nullptr || textBuffer_ == nullptr) return;
    C2D_Prepare();
    C2D_TargetClear(bottomTarget_, kBackground);
    C2D_SceneBegin(bottomTarget_);
    if (state.screen == UiScreen::Campaign) drawCampaign(state);
    else if (state.screen == UiScreen::Loading) {
        C2D_DrawRectSolid(8.0F, 70.0F, 0.1F, 304.0F, 100.0F, kPanel);
        drawText("CITADEL DEFENSE 3D", 35.0F, 91.0F, 0.72F, kAccent);
        drawText("Ladowanie poziomu...", 62.0F, 126.0F, 0.58F, kText);
        drawText(state.title, 45.0F, 151.0F, 0.48F, kMuted);
    } else {
        drawMission(state);
    }
    C2D_Flush();
    composedFrameActive_ = false;
}

void UiRenderer::drawCampaign(const UiState& state) {
    C2D_DrawRectSolid(0.0F, 0.0F, 0.1F, 320.0F, 30.0F, kPanelStrong);
    drawText("CITADEL DEFENSE 3D", 12.0F, 7.0F, 0.58F, kAccent);
    drawText("KAMPANIA", 238.0F, 8.0F, 0.45F, kMuted);

    C2D_DrawRectSolid(6.0F, 36.0F, 0.1F, 144.0F, 158.0F, kPanel);
    for (std::size_t index = 0; index < kCampaignMissionCount; ++index) {
        const float y = 42.0F + static_cast<float>(index) * 24.0F;
        if (index == state.selectedMission) C2D_DrawRectSolid(10.0F, y - 3.0F, 0.2F, 136.0F, 21.0F, kSelected);
        char line[48]{};
        std::snprintf(line, sizeof(line), "%zu. %.16s  %u/3", index + 1U,
            state.missionUnlocked[index] ? state.campaignTitles[index] : "ZABLOKOWANE",
            static_cast<unsigned int>(state.missionStars[index]));
        drawText(line, 14.0F, y, 0.40F, index == state.selectedMission ? kGold : kText);
    }

    C2D_DrawRectSolid(156.0F, 36.0F, 0.1F, 158.0F, 158.0F, kPanel);
    drawText(state.title, 164.0F, 45.0F, 0.50F, kAccent);
    drawText(state.objective, 164.0F, 70.0F, 0.39F, kText);
    drawText(state.availableTowers, 164.0F, 97.0F, 0.37F, kMuted);
    drawText(state.threats, 164.0F, 121.0F, 0.37F, kMuted);
    char stars[48]{};
    std::snprintf(stars, sizeof(stars), "3*: HP >= %u, wieze <= %u",
        static_cast<unsigned int>(state.fullHealthThreshold),
        static_cast<unsigned int>(state.efficientTowerLimit));
    drawText(stars, 164.0F, 150.0F, 0.36F, kGold);
    if (state.statusMessage != nullptr && state.statusMessage[0] != '\0') {
        drawText(state.statusMessage, 164.0F, 174.0F, 0.34F, state.saveProblem ? kDanger : kMuted);
    }

    drawButton(6.0F, 201.0F, 74.0F, 33.0F, "A GRAJ", false);
    drawButton(84.0F, 201.0F, 74.0F, 33.0F, state.soundEnabled ? "X DZW ON" : "X DZW OFF", false);
    drawButton(162.0F, 201.0F, 74.0F, 33.0F, state.stereoEnabled ? "L 3D ON" : "L 3D OFF", false);
    drawButton(240.0F, 201.0F, 74.0F, 33.0F, "START WYJ", false);
}

void UiRenderer::drawMission(const UiState& state) {
    const TouchRect ballista = TouchUiLayout::rectFor(TouchUiAction::SelectBallista);
    const TouchRect mortar = TouchUiLayout::rectFor(TouchUiAction::SelectMortar);
    const TouchRect frost = TouchUiLayout::rectFor(TouchUiAction::SelectFrost);
    drawButton(ballista.x, ballista.y, ballista.width, ballista.height, "KUSZA",
        state.selectedTower == TowerType::Ballista);
    drawButton(mortar.x, mortar.y, mortar.width, mortar.height, "MOZDZIERZ",
        state.selectedTower == TowerType::Mortar);
    drawButton(frost.x, frost.y, frost.width, frost.height, "MROZ",
        state.selectedTower == TowerType::Frost);

    C2D_DrawRectSolid(8.0F, 58.0F, 0.1F, 304.0F, 62.0F, kPanel);
    char resources[64]{};
    std::snprintf(resources, sizeof(resources), "ZLOTO %d   BAZA %d   WROG %zu/%zu",
        state.gold, state.baseHealth, state.spawnedEnemies, state.totalEnemies);
    drawText(resources, 14.0F, 65.0F, 0.42F, kGold);
    char selection[64]{};
    std::snprintf(selection, sizeof(selection), "%s koszt %d  pole %zu,%zu  %s",
        towerLabel(state.selectedTower), state.towerCost, state.cursorX, state.cursorZ,
        state.cursorOccupied ? "WIEZA" : "BUDOWA");
    drawText(selection, 14.0F, 85.0F, 0.38F, kText);
    drawText(state.instruction, 14.0F, 104.0F, 0.34F, kMuted);

    if (state.diagnosticsVisible) {
        drawDiagnostics(state);
        return;
    }

    const TouchRect pause = TouchUiLayout::rectFor(TouchUiAction::TogglePause);
    const TouchRect speed = TouchUiLayout::rectFor(TouchUiAction::ToggleSpeed);
    const TouchRect cancel = TouchUiLayout::rectFor(TouchUiAction::Cancel);
    drawButton(pause.x, pause.y, pause.width, pause.height, state.paused ? "WZNOW" : "PAUZA", state.paused);
    char speedLabel[16]{};
    std::snprintf(speedLabel, sizeof(speedLabel), "TEMPO %dx", state.speedMultiplier);
    drawButton(speed.x, speed.y, speed.width, speed.height, speedLabel, state.speedMultiplier == 2);
    drawButton(cancel.x, cancel.y, cancel.width, cancel.height, "ANULUJ", false);
    drawText(state.statusMessage, 168.0F, 141.0F, 0.34F, kGold);

    const TouchRect build = TouchUiLayout::rectFor(TouchUiAction::BuildOrSelect);
    const TouchRect upgrade = TouchUiLayout::rectFor(TouchUiAction::Upgrade);
    const TouchRect sell = TouchUiLayout::rectFor(TouchUiAction::Sell);
    const TouchRect start = TouchUiLayout::rectFor(TouchUiAction::StartWave);
    drawButton(build.x, build.y, build.width, build.height, "BUDUJ", false);
    drawButton(upgrade.x, upgrade.y, upgrade.width, upgrade.height, "ULEPSZ", false);
    drawButton(sell.x, sell.y, sell.width, sell.height, "SPRZEDAJ", false);
    drawButton(start.x, start.y, start.width, start.height,
        state.missionFinished ? "KONIEC" : (state.tutorialPhase == TutorialPhase::WaveRunning ? "FALA" : "START"), false);
}

void UiRenderer::drawDiagnostics(const UiState& state) {
    C2D_DrawRectSolid(8.0F, 128.0F, 0.2F, 304.0F, 106.0F, kPanelStrong);
    const float fps = state.performance.averageFrameMilliseconds > 0.0F
        ? 1000.0F / state.performance.averageFrameMilliseconds : 0.0F;
    char line[96]{};
    std::snprintf(line, sizeof(line), "FPS %.1f AVG %.1fms MAX %.1fms", fps,
        state.performance.averageFrameMilliseconds, state.performance.worstFrameMilliseconds);
    drawText(line, 13.0F, 137.0F, 0.36F, state.performance.frameBudgetExceeded() ? kDanger : kText);
    std::snprintf(line, sizeof(line), "RENDER %.1fms  MEM %luKB", state.performance.lastRenderMilliseconds,
        static_cast<unsigned long>(state.performance.freeLinearMemoryBytes / 1024U));
    drawText(line, 13.0F, 158.0F, 0.36F, state.performance.memoryReserveLow() ? kDanger : kText);
    std::snprintf(line, sizeof(line), "3D %s %u%% oczy %u sep %.4f", state.stereoEnabled ? "ON" : "OFF",
        static_cast<unsigned int>(state.maximum3DDepthPercent), static_cast<unsigned int>(state.stereoEyeCount),
        state.stereoSeparation);
    drawText(line, 13.0F, 179.0F, 0.34F, kText);
    std::snprintf(line, sizeof(line), "AUDIO %s HLE %s CH %d", state.audioBackend,
        state.ndspShimActive ? "ON" : (state.ndspShimAttempted ? "BLAD" : "NIE"), state.audioChannel);
    drawText(line, 13.0F, 200.0F, 0.34F, kMuted);
    std::snprintf(line, sizeof(line), "BUF %s %lu/%lu", state.audioWaveStatus,
        static_cast<unsigned long>(state.audioSamplePosition), static_cast<unsigned long>(state.audioSampleCount));
    drawText(line, 13.0F, 219.0F, 0.32F, kMuted);
}

void UiRenderer::renderTopOverlay(C3D_RenderTarget* target, const UiState& state) {
    if (target == nullptr || textBuffer_ == nullptr || state.screen != UiScreen::Mission) return;
    if (!composedFrameActive_) beginFrame();
    C2D_Prepare();
    C2D_SceneBegin(target);
    C2D_DrawRectSolid(8.0F, 8.0F, 0.8F, 130.0F, 28.0F, C2D_Color32(24, 28, 38, 225));
    C2D_DrawCircleSolid(23.0F, 22.0F, 0.9F, 9.0F, C2D_Color32(244, 190, 48, 255));
    char text[64]{};
    std::snprintf(text, sizeof(text), "%d   HP %d   %zu/%zu", std::clamp(state.gold, 0, 9999),
        state.baseHealth, state.spawnedEnemies, state.totalEnemies);
    drawText(text, 39.0F, 14.0F, 0.46F, kText);
    C2D_Flush();
}
