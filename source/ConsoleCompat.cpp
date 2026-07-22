#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {

constexpr int kMaximumRows = 30;
constexpr float kTextScale = 0.46F;
constexpr float kLineHeight = 9.2F;

enum class BottomUiMode {
    None,
    Campaign,
    Mission,
};

std::array<std::array<char, 64>, kMaximumRows> gRows{};
char gLastTitle[64]{};
bool gClearRequested = false;
bool gCitro2dReady = false;
BottomUiMode gMode = BottomUiMode::None;
C3D_RenderTarget* gBottomTarget = nullptr;
C2D_TextBuf gTextBuffer = nullptr;

void clearRows() {
    for (auto& row : gRows) row.fill('\0');
}

void storeRow(int row, const char* text) {
    if (row < 1 || row > kMaximumRows) return;
    std::snprintf(gRows[static_cast<std::size_t>(row - 1)].data(),
        gRows[static_cast<std::size_t>(row - 1)].size(), "%-.30s", text != nullptr ? text : "");
}

void drawTextLine(const char* text, float x, float y, float scale, u32 color) {
    if (text == nullptr || text[0] == '\0' || gTextBuffer == nullptr) return;
    C2D_Text parsed{};
    C2D_TextParse(&parsed, gTextBuffer, text);
    C2D_TextOptimize(&parsed);
    C2D_DrawText(&parsed, C2D_WithColor, x, y, 0.5F, scale, scale, color);
}

void drawPanels() {
    C2D_DrawRectSolid(0.0F, 0.0F, 0.1F, 320.0F, 28.0F, C2D_Color32(24, 43, 62, 255));
    if (gMode == BottomUiMode::Campaign) {
        C2D_DrawRectSolid(5.0F, 31.0F, 0.1F, 310.0F, 78.0F, C2D_Color32(20, 29, 40, 255));
        C2D_DrawRectSolid(5.0F, 112.0F, 0.1F, 310.0F, 78.0F, C2D_Color32(20, 29, 40, 255));
        C2D_DrawRectSolid(5.0F, 193.0F, 0.1F, 310.0F, 42.0F, C2D_Color32(20, 29, 40, 255));
    } else {
        C2D_DrawRectSolid(5.0F, 31.0F, 0.1F, 310.0F, 37.0F, C2D_Color32(20, 29, 40, 255));
        C2D_DrawRectSolid(5.0F, 72.0F, 0.1F, 310.0F, 58.0F, C2D_Color32(20, 29, 40, 255));
        C2D_DrawRectSolid(5.0F, 134.0F, 0.1F, 310.0F, 56.0F, C2D_Color32(20, 29, 40, 255));
        C2D_DrawRectSolid(5.0F, 194.0F, 0.1F, 310.0F, 41.0F, C2D_Color32(20, 29, 40, 255));
    }
}

void renderBottomFrame() {
    if (!gCitro2dReady || gBottomTarget == nullptr || gTextBuffer == nullptr || gMode == BottomUiMode::None) {
        return;
    }

    C2D_TextBufClear(gTextBuffer);
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(gBottomTarget, C2D_Color32(12, 18, 28, 255));
    C2D_SceneBegin(gBottomTarget);
    drawPanels();

    for (int row = 1; row <= 24; ++row) {
        const char* text = gRows[static_cast<std::size_t>(row - 1)].data();
        if (text[0] == '\0') continue;

        const float y = 4.0F + static_cast<float>(row - 1) * kLineHeight;
        const bool selected = text[0] == '>';
        if (selected) {
            C2D_DrawRectSolid(7.0F, y - 1.0F, 0.2F, 306.0F, kLineHeight,
                C2D_Color32(48, 91, 119, 255));
        }

        u32 color = C2D_Color32(236, 240, 244, 255);
        if (row == 1) color = C2D_Color32(112, 225, 255, 255);
        else if (selected) color = C2D_Color32(255, 238, 166, 255);
        else if (gMode == BottomUiMode::Mission && (row == 3 || row == 4)) {
            color = C2D_Color32(255, 222, 112, 255);
        }

        drawTextLine(text, 10.0F, y, row == 1 ? 0.50F : kTextScale, color);
    }

    C3D_FrameEnd(0);
}

}  // namespace

bool citadelGraphicsInit(std::size_t commandBufferSize) {
    if (!C3D_Init(commandBufferSize)) return false;
    if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)) {
        C3D_Fini();
        return false;
    }

    C2D_Prepare();
    gBottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    gTextBuffer = C2D_TextBufNew(4096);
    gCitro2dReady = gBottomTarget != nullptr && gTextBuffer != nullptr;
    if (!gCitro2dReady) {
        if (gTextBuffer != nullptr) {
            C2D_TextBufDelete(gTextBuffer);
            gTextBuffer = nullptr;
        }
        C2D_Fini();
        C3D_Fini();
        return false;
    }
    return true;
}

void citadelGraphicsFini() {
    if (gTextBuffer != nullptr) {
        C2D_TextBufDelete(gTextBuffer);
        gTextBuffer = nullptr;
    }
    if (gCitro2dReady) C2D_Fini();
    gCitro2dReady = false;
    gBottomTarget = nullptr;
    gMode = BottomUiMode::None;
    C3D_Fini();
}

void drawBottomUiOverlay() {
    if (gMode == BottomUiMode::Mission) renderBottomFrame();
}

void citadelSwapBuffers() {
    if (gMode == BottomUiMode::Campaign) renderBottomFrame();
    else gfxSwapBuffers();
}

namespace std {

int citadelConsolePrintf(const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);
    int result = 0;

    if (std::strcmp(format, "\x1b[2J\x1b[1;1H") == 0) {
        gClearRequested = true;
        va_end(arguments);
        return 0;
    }

    if (std::strcmp(format, "\x1b[%d;1H\x1b[36m%-39.39s\x1b[0m") == 0) {
        const int row = va_arg(arguments, int);
        const char* title = va_arg(arguments, const char*);
        const char* safeTitle = title != nullptr ? title : "";
        const BottomUiMode nextMode = std::strcmp(safeTitle, "CITADEL DEFENSE 3D") == 0
            ? BottomUiMode::Campaign
            : BottomUiMode::Mission;

        if (gMode != nextMode || gClearRequested || std::strncmp(gLastTitle, safeTitle, sizeof(gLastTitle) - 1U) != 0) {
            clearRows();
        }
        gMode = nextMode;
        storeRow(row, safeTitle);
        std::snprintf(gLastTitle, sizeof(gLastTitle), "%s", safeTitle);
        gClearRequested = false;
        va_end(arguments);
        return 0;
    }

    if (std::strcmp(format, "\x1b[%d;1H%-39.39s") == 0) {
        const int row = va_arg(arguments, int);
        const char* text = va_arg(arguments, const char*);
        if (gMode != BottomUiMode::None) {
            if (gClearRequested) {
                clearRows();
                gClearRequested = false;
            }
            storeRow(row, text);
            va_end(arguments);
            return 0;
        }
        result = ::printf("\x1b[%d;1H%-30.30s", row, text != nullptr ? text : "");
        va_end(arguments);
        return result;
    }

    result = ::vprintf(format, arguments);
    va_end(arguments);
    return result;
}

}  // namespace std
