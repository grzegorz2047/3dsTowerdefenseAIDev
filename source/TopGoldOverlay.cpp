#include <3ds.h>

#include <algorithm>
#include <array>
#include <cstdio>

namespace {

struct Color {
    u8 r;
    u8 g;
    u8 b;
};

constexpr int kLogicalWidth = 400;
constexpr int kLogicalHeight = 240;
constexpr Color kPanel{24, 28, 38};
constexpr Color kPanelEdge{70, 76, 90};
constexpr Color kGold{244, 190, 48};
constexpr Color kGoldLight{255, 232, 112};
constexpr Color kText{250, 250, 246};

void putPixel(u8* framebuffer, int x, int y, Color color) {
    if (framebuffer == nullptr || x < 0 || x >= kLogicalWidth || y < 0 || y >= kLogicalHeight) return;
    const std::size_t offset = static_cast<std::size_t>(3 * (x * kLogicalHeight + (kLogicalHeight - 1 - y)));
    framebuffer[offset + 0] = color.b;
    framebuffer[offset + 1] = color.g;
    framebuffer[offset + 2] = color.r;
}

void fillRect(u8* framebuffer, int x, int y, int width, int height, Color color) {
    for (int px = x; px < x + width; ++px) {
        for (int py = y; py < y + height; ++py) putPixel(framebuffer, px, py, color);
    }
}

void drawCoin(u8* framebuffer, int centerX, int centerY) {
    for (int dx = -8; dx <= 8; ++dx) {
        for (int dy = -8; dy <= 8; ++dy) {
            const int distance = dx * dx + dy * dy;
            if (distance <= 64) putPixel(framebuffer, centerX + dx, centerY + dy, kGold);
            if (distance <= 25 && dx < 1 && dy < 1) putPixel(framebuffer, centerX + dx, centerY + dy, kGoldLight);
        }
    }
    fillRect(framebuffer, centerX - 1, centerY - 5, 2, 10, kPanelEdge);
}

void drawSegment(u8* framebuffer, int x, int y, int segment) {
    switch (segment) {
        case 0: fillRect(framebuffer, x + 2, y, 6, 2, kText); break;
        case 1: fillRect(framebuffer, x + 8, y + 2, 2, 6, kText); break;
        case 2: fillRect(framebuffer, x + 8, y + 10, 2, 6, kText); break;
        case 3: fillRect(framebuffer, x + 2, y + 16, 6, 2, kText); break;
        case 4: fillRect(framebuffer, x, y + 10, 2, 6, kText); break;
        case 5: fillRect(framebuffer, x, y + 2, 2, 6, kText); break;
        case 6: fillRect(framebuffer, x + 2, y + 8, 6, 2, kText); break;
        default: break;
    }
}

void drawDigit(u8* framebuffer, int x, int y, char digit) {
    static constexpr std::array<u8, 10> masks{
        0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
        0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111,
    };
    if (digit < '0' || digit > '9') return;
    const u8 mask = masks[static_cast<std::size_t>(digit - '0')];
    for (int segment = 0; segment < 7; ++segment) {
        if ((mask & (1U << segment)) != 0U) drawSegment(framebuffer, x, y, segment);
    }
}

void drawOverlay(u8* framebuffer, int gold) {
    const int clampedGold = std::clamp(gold, 0, 9999);
    char text[8]{};
    std::snprintf(text, sizeof(text), "%d", clampedGold);

    fillRect(framebuffer, 8, 8, 78, 28, kPanelEdge);
    fillRect(framebuffer, 10, 10, 74, 24, kPanel);
    drawCoin(framebuffer, 23, 22);

    int x = 38;
    for (const char* cursor = text; *cursor != '\0'; ++cursor) {
        drawDigit(framebuffer, x, 13, *cursor);
        x += 13;
    }
}

}  // namespace

void drawTopGoldOverlay(int gold) {
    u16 width = 0;
    u16 height = 0;
    u8* left = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
    drawOverlay(left, gold);
    u8* right = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, &width, &height);
    if (right != left) drawOverlay(right, gold);
}
