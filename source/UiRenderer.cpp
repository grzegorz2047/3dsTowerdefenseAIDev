#include "UiRenderer.hpp"

#include <algorithm>
#include <cstdio>
#include <string>

#include "SevenSegmentDigits.hpp"
#include "TouchUiLayout.hpp"

namespace {

const u32 kBackground = C2D_Color32(12, 18, 28, 255);
const u32 kPanel = C2D_Color32(20, 29, 40, 255);
const u32 kPanelStrong = C2D_Color32(24, 43, 62, 255);
const u32 kText = C2D_Color32(236, 240, 244, 255);
const u32 kMuted = C2D_Color32(190, 202, 213, 255);
const u32 kAccent = C2D_Color32(112, 225, 255, 255);
const u32 kGold = C2D_Color32(255, 222, 112, 255);
const u32 kDanger = C2D_Color32(255, 132, 132, 255);
const u32 kSelected = C2D_Color32(48, 91, 119, 255);
constexpr float kMinimumReadableScale = 0.40F;

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
    topTarget_ = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottomTarget_ = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    textBuffer_ = C2D_TextBufNew(4096);
    if (topTarget_ == nullptr || bottomTarget_ == nullptr || textBuffer_ == nullptr) {
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
    topTarget_ = nullptr;
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
    const float readableScale = std::max(scale, kMinimumReadableScale);
    C2D_DrawText(&parsed, C2D_WithColor, x, y, 0.5F, readableScale, readableScale, color);
}

void UiRenderer::drawWrappedText(const char* text, float x, float y, float scale, u32 color,
    std::size_t charactersPerLine, std::size_t maximumLines) {
    if (text == nullptr || text[0] == '\0' || charactersPerLine == 0U || maximumLines == 0U) return;

    const float readableScale = std::max(scale, kMinimumReadableScale);
    const std::string source(text);
    std::size_t cursor = 0U;
    std::size_t lineIndex = 0U;
    std::string line;
    const float lineHeight = 34.0F * readableScale;

    while (cursor < source.size() && lineIndex < maximumLines) {
        while (cursor < source.size() && source[cursor] == ' ') ++cursor;
        if (cursor >= source.size()) break;

        const std::size_t separator = source.find_first_of(" \n", cursor);
        const std::size_t wordEnd = separator == std::string::npos ? source.size() : separator;
        const std::string word = source.substr(cursor, wordEnd - cursor);
        const bool explicitBreak = separator != std::string::npos && source[separator] == '\n';

        if (!line.empty() && line.size() + 1U + word.size() > charactersPerLine) {
            drawText(line.c_str(), x, y + static_cast<float>(lineIndex) * lineHeight, readableScale, color);
            ++lineIndex;
            line.clear();
            if (lineIndex >= maximumLines) break;
        }

        if (!line.empty()) line += ' ';
        line += word;
        cursor = separator == std::string::npos ? source.size() : separator + 1U;

        if (explicitBreak || line.size() >= charactersPerLine) {
            drawText(line.c_str(), x, y + static_cast<float>(lineIndex) * lineHeight, readableScale, color);
            ++lineIndex;
            line.clear();
        }
    }

    if (!line.empty() && lineIndex < maximumLines) {
        drawText(line.c_str(), x, y + static_cast<float>(lineIndex) * lineHeight, readableScale, color);
    }
}

void UiRenderer::drawButton(float x, float y, float width, float height, const char* label, bool selected) {
    C2D_DrawRectSolid(x, y, 0.1F, width, height, selected ? kSelected : kPanelStrong);
    drawText(label, x + 7.0F, y + height * 0.34F, 0.46F, selected ? kGold : kText);
}

void UiRenderer::drawSegmentDigit(unsigned int digit, float x, float y, float scale, u32 color) {
    const std::uint8_t mask = SevenSegmentDigits::maskFor(digit);
    const float thickness = 2.0F * scale;
    const float horizontal = 8.0F * scale;
    const float vertical = 6.0F * scale;
    const float right = x + horizontal - thickness;
    const float middleY = y + vertical;
    const float bottomY = y + vertical * 2.0F;

    if ((mask & SevenSegmentDigits::Top) != 0U) C2D_DrawRectSolid(x, y, 0.95F, horizontal, thickness, color);
    if ((mask & SevenSegmentDigits::UpperRight) != 0U) C2D_DrawRectSolid(right, y, 0.95F, thickness, vertical, color);
    if ((mask & SevenSegmentDigits::LowerRight) != 0U) C2D_DrawRectSolid(right, middleY, 0.95F, thickness, vertical, color);
    if ((mask & SevenSegmentDigits::Bottom) != 0U) C2D_DrawRectSolid(x, bottomY, 0.95F, horizontal, thickness, color);
    if ((mask & SevenSegmentDigits::LowerLeft) != 0U) C2D_DrawRectSolid(x, middleY, 0.95F, thickness, vertical, color);
    if ((mask & SevenSegmentDigits::UpperLeft) != 0U) C2D_DrawRectSolid(x, y, 0.95F, thickness, vertical, color);
    if ((mask & SevenSegmentDigits::Middle) != 0U) C2D_DrawRectSolid(x, middleY, 0.95F, horizontal, thickness, color);
}

void UiRenderer::drawSegmentNumber(int value, float x, float y, float scale, u32 color) {
    const int safe = SevenSegmentDigits::clampValue(value);
    char digits[5]{};
    std::snprintf(digits, sizeof(digits), "%d", safe);
    const float advance = 11.0F * scale;
    for (std::size_t index = 0U; digits[index] != '\0'; ++index) {
        drawSegmentDigit(static_cast<unsigned int>(digits[index] - '0'),
            x + static_cast<float>(index) * advance, y, scale, color);
    }
}

void UiRenderer::renderStandalone(const UiState& state) {
    if (topTarget_ == nullptr || bottomTarget_ == nullptr || textBuffer_ == nullptr) return;
    gfxSet3D(false);
    beginFrame();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    drawStandaloneTop(state);
    renderBottom(state);
    C3D_FrameEnd(0);
}

void UiRenderer::drawStandaloneTop(const UiState& state) {
    C2D_Prepare();
    C2D_TargetClear(topTarget_, kBackground);
    C2D_SceneBegin(topTarget_);
    C2D_DrawRectSolid(16.0F, 14.0F, 0.1F, 368.0F, 212.0F, kPanel);
    C2D_DrawRectSolid(16.0F, 14.0F, 0.2F, 368.0F, 42.0F, kPanelStrong);
    drawText("CITADEL DEFENSE 3D", 42.0F, 25.0F, 0.78F, kAccent);

    if (state.screen == UiScreen::Loading) {
        drawText("LADOWANIE MISJI", 112.0F, 88.0F, 0.62F, kGold);
        drawWrappedText(state.title, 65.0F, 124.0F, 0.58F, kText, 24U, 2U);
        drawText("Przygotowywanie pola bitwy...", 82.0F, 178.0F, 0.44F, kMuted);
    } else {
        drawText("WYBRANA MISJA", 28.0F, 72.0F, 0.44F, kMuted);
        drawWrappedText(state.title, 28.0F, 94.0F, 0.68F, kGold, 25U, 2U);
        drawWrappedText(state.objective, 28.0F, 144.0F, 0.46F, kText, 36U, 3U);
        drawText("A: rozpocznij   START: wyjdz", 28.0F, 202.0F, 0.42F, kMuted);
    }
    C2D_Flush();
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
        drawWrappedText(state.title, 45.0F, 151.0F, 0.46F, kMuted, 25U, 2U);
    } else {
        drawMission(state);
    }
    C2D_Flush();
    composedFrameActive_ = false;
}

void UiRenderer::drawCampaign(const UiState& state) {
    C2D_DrawRectSolid(0.0F, 0.0F, 0.1F, 320.0F, 30.0F, kPanelStrong);
    drawText("CITADEL DEFENSE 3D", 10.0F, 7.0F, 0.56F, kAccent);
    drawText("KAMPANIA", 238.0F, 8.0F, 0.45F, kMuted);

    C2D_DrawRectSolid(6.0F, 36.0F, 0.1F, 144.0F, 158.0F, kPanel);
    for (std::size_t index = 0; index < kCampaignMissionCount; ++index) {
        const float itemY = 42.0F + static_cast<float>(index) * 24.0F;
        if (index == state.selectedMission) {
            C2D_DrawRectSolid(10.0F, itemY - 3.0F, 0.2F, 136.0F, 21.0F, kSelected);
        }
        char line[48]{};
        std::snprintf(line, sizeof(line), "%zu. %.13s %u/3", index + 1U,
            state.missionUnlocked[index] ? state.campaignTitles[index] : "ZABLOKOWANE",
            static_cast<unsigned int>(state.missionStars[index]));
        drawText(line, 13.0F, itemY, 0.42F, index == state.selectedMission ? kGold : kText);
    }

    C2D_DrawRectSolid(156.0F, 36.0F, 0.1F, 158.0F, 158.0F, kPanel);
    drawWrappedText(state.title, 164.0F, 43.0F, 0.47F, kAccent, 17U, 2U);
    drawText("CEL:", 164.0F, 76.0F, 0.40F, kMuted);
    drawWrappedText(state.objective, 164.0F, 92.0F, 0.42F, kText, 18U, 2U);
    drawWrappedText(state.availableTowers, 164.0F, 123.0F, 0.40F, kMuted, 19U, 1U);
    drawWrappedText(state.threats, 164.0F, 142.0F, 0.40F, kMuted, 19U, 1U);
    char stars[48]{};
    std::snprintf(stars, sizeof(stars), "3*: HP>=%u WIEZE<=%u",
        static_cast<unsigned int>(state.fullHealthThreshold),
        static_cast<unsigned int>(state.efficientTowerLimit));
    drawText(stars, 164.0F, 163.0F, 0.40F, kGold);
    if (state.saveProblem && state.statusMessage != nullptr && state.statusMessage[0] != '\0') {
        drawWrappedText(state.statusMessage, 164.0F, 180.0F, 0.40F, kDanger, 19U, 1U);
    }

    drawButton(6.0F, 201.0F, 98.0F, 33.0F, "A GRAJ", false);
    drawButton(111.0F, 201.0F, 98.0F, 33.0F, state.soundEnabled ? "X DZW ON" : "X DZW OFF", false);
    drawButton(216.0F, 201.0F, 98.0F, 33.0F, "START WYJ", false);
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
    std::snprintf(resources, sizeof(resources), "ZLOTO %d  BAZA %d  WROG %zu/%zu",
        state.gold, state.baseHealth, state.spawnedEnemies, state.totalEnemies);
    drawText(resources, 14.0F, 64.0F, 0.47F, kGold);
    char selection[64]{};
    std::snprintf(selection, sizeof(selection), "%s %dG  POLE %zu,%zu  %s",
        towerLabel(state.selectedTower), state.towerCost, state.cursorX, state.cursorZ,
        state.cursorOccupied ? "ZAJETE" : "WOLNE");
    drawText(selection, 14.0F, 85.0F, 0.43F, kText);
    drawWrappedText(state.instruction, 14.0F, 104.0F, 0.40F, kMuted, 38U, 1U);

    if (state.diagnosticsVisible) {
        drawDiagnostics(state);
        return;
    }

    if (state.missionFinished) {
        drawButton(8.0F, 128.0F, 144.0F, 40.0F, "X KAMPANIA", true);
        drawButton(160.0F, 128.0F, 152.0F, 40.0F, "Y POWTORZ", false);
    } else {
        const TouchRect pause = TouchUiLayout::rectFor(TouchUiAction::TogglePause);
        const TouchRect speed = TouchUiLayout::rectFor(TouchUiAction::ToggleSpeed);
        const TouchRect cancel = TouchUiLayout::rectFor(TouchUiAction::Cancel);
        drawButton(pause.x, pause.y, pause.width, pause.height, state.paused ? "WZNOW" : "PAUZA", state.paused);
        char speedLabel[16]{};
        std::snprintf(speedLabel, sizeof(speedLabel), "TEMPO %dx", state.speedMultiplier);
        drawButton(speed.x, speed.y, speed.width, speed.height, speedLabel, state.speedMultiplier == 2);
        drawButton(cancel.x, cancel.y, cancel.width, cancel.height, "ANULUJ", false);
        drawWrappedText(state.statusMessage, 168.0F, 133.0F, 0.40F, kGold, 12U, 2U);
    }

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
    drawText(line, 13.0F, 137.0F, 0.40F, state.performance.frameBudgetExceeded() ? kDanger : kText);
    std::snprintf(line, sizeof(line), "RENDER %.1fms MEM %luKB", state.performance.lastRenderMilliseconds,
        static_cast<unsigned long>(state.performance.freeLinearMemoryBytes / 1024U));
    drawText(line, 13.0F, 158.0F, 0.40F, state.performance.memoryReserveLow() ? kDanger : kText);
    std::snprintf(line, sizeof(line), "3D %s %u%% OCZY %u", state.stereoEnabled ? "ON" : "OFF",
        static_cast<unsigned int>(state.maximum3DDepthPercent), static_cast<unsigned int>(state.stereoEyeCount));
    drawText(line, 13.0F, 179.0F, 0.40F, kText);
    std::snprintf(line, sizeof(line), "AUDIO %s CH %d", state.audioBackend, state.audioChannel);
    drawText(line, 13.0F, 200.0F, 0.40F, kMuted);
    std::snprintf(line, sizeof(line), "BUF %s %lu/%lu", state.audioWaveStatus,
        static_cast<unsigned long>(state.audioSamplePosition), static_cast<unsigned long>(state.audioSampleCount));
    drawText(line, 13.0F, 219.0F, 0.40F, kMuted);
}

void UiRenderer::renderTopOverlay(C3D_RenderTarget* target, const UiState& state) {
    if (target == nullptr || textBuffer_ == nullptr || state.screen != UiScreen::Mission) return;
    if (!composedFrameActive_) beginFrame();
    C2D_Prepare();
    C2D_SceneBegin(target);
    C2D_DrawRectSolid(8.0F, 8.0F, 0.8F, 224.0F, 32.0F, C2D_Color32(24, 28, 38, 235));
    C2D_DrawCircleSolid(21.0F, 24.0F, 0.9F, 10.0F, C2D_Color32(244, 190, 48, 255));
    C2D_DrawCircleSolid(18.5F, 21.0F, 0.92F, 3.0F, C2D_Color32(255, 232, 136, 255));
    drawSegmentNumber(state.gold, 36.0F, 13.0F, 1.05F, kGold);
    char status[48]{};
    std::snprintf(status, sizeof(status), "HP %d   WROG %zu/%zu", state.baseHealth,
        state.spawnedEnemies, state.totalEnemies);
    drawText(status, 94.0F, 16.0F, 0.48F, kText);
    C2D_Flush();
}
