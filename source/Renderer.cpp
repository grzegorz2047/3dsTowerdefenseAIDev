#include "Renderer.hpp"

#include <3ds.h>

#include <cstdio>
#include <cstring>
#include <vector>

#include "vshader_shbin.h"

namespace {

constexpr u32 kTopClearColor = 0x182030FF;
constexpr u32 kHudBackground = C2D_Color32(14, 20, 32, 255);
constexpr u32 kHudPanel = C2D_Color32(28, 38, 56, 255);
constexpr u32 kHudAccent = C2D_Color32(45, 73, 105, 255);
constexpr u32 kHudText = C2D_Color32(236, 241, 248, 255);
constexpr u32 kHudMuted = C2D_Color32(166, 181, 202, 255);
constexpr u32 kHudGood = C2D_Color32(82, 212, 126, 255);
constexpr u32 kHudWarning = C2D_Color32(245, 188, 66, 255);
constexpr u32 kHudDanger = C2D_Color32(235, 82, 82, 255);
constexpr u32 kDisplayTransferFlags =
    GX_TRANSFER_FLIP_VERT(0) |
    GX_TRANSFER_OUT_TILED(0) |
    GX_TRANSFER_RAW_COPY(0) |
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
    GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);

struct Vertex {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
};

struct TileVisual {
    float r;
    float g;
    float b;
    float height;
    float inset;
};

TileVisual visualFor(TileType tile) {
    switch (tile) {
        case TileType::Road: return {0.48F, 0.34F, 0.20F, 0.03F, 0.03F};
        case TileType::BuildSpot: return {0.20F, 0.42F, 0.68F, 0.07F, 0.10F};
        case TileType::Blocked: return {0.24F, 0.25F, 0.28F, 0.38F, 0.05F};
        case TileType::Spawn: return {0.18F, 0.72F, 0.28F, 0.08F, 0.05F};
        case TileType::Base: return {0.78F, 0.22F, 0.18F, 0.16F, 0.05F};
        case TileType::Ground:
        default: return {0.18F, 0.46F, 0.22F, 0.0F, 0.04F};
    }
}

void appendQuad(std::vector<Vertex>& vertices, const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& d) {
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
    vertices.push_back(c);
    vertices.push_back(d);
    vertices.push_back(a);
}

void appendTile(std::vector<Vertex>& vertices, float centerX, float centerZ, const TileVisual& visual) {
    const float half = 0.5F - visual.inset;
    const float y = visual.height;
    appendQuad(
        vertices,
        {centerX - half, y, centerZ - half, visual.r, visual.g, visual.b, 1.0F},
        {centerX - half, y, centerZ + half, visual.r, visual.g, visual.b, 1.0F},
        {centerX + half, y, centerZ + half, visual.r, visual.g, visual.b, 1.0F},
        {centerX + half, y, centerZ - half, visual.r, visual.g, visual.b, 1.0F});
}

void appendBoxAt(
    std::vector<Vertex>& vertices,
    float centerX,
    float centerZ,
    float halfX,
    float halfZ,
    float y0,
    float y1,
    float r,
    float g,
    float b) {
    const float x0 = centerX - halfX;
    const float x1 = centerX + halfX;
    const float z0 = centerZ - halfZ;
    const float z1 = centerZ + halfZ;
    appendQuad(vertices,
        {x0, y0, z1, r, g, b, 1.0F}, {x1, y0, z1, r, g, b, 1.0F},
        {x1, y1, z1, r, g, b, 1.0F}, {x0, y1, z1, r, g, b, 1.0F});
    appendQuad(vertices,
        {x1, y0, z0, r, g, b, 1.0F}, {x0, y0, z0, r, g, b, 1.0F},
        {x0, y1, z0, r, g, b, 1.0F}, {x1, y1, z0, r, g, b, 1.0F});
    appendQuad(vertices,
        {x1, y0, z1, r, g, b, 1.0F}, {x1, y0, z0, r, g, b, 1.0F},
        {x1, y1, z0, r, g, b, 1.0F}, {x1, y1, z1, r, g, b, 1.0F});
    appendQuad(vertices,
        {x0, y0, z0, r, g, b, 1.0F}, {x0, y0, z1, r, g, b, 1.0F},
        {x0, y1, z1, r, g, b, 1.0F}, {x0, y1, z0, r, g, b, 1.0F});
    appendQuad(vertices,
        {x0, y1, z1, r, g, b, 1.0F}, {x1, y1, z1, r, g, b, 1.0F},
        {x1, y1, z0, r, g, b, 1.0F}, {x0, y1, z0, r, g, b, 1.0F});
}

void appendEnemy(std::vector<Vertex>& vertices) {
    constexpr float leatherR = 0.30F;
    constexpr float leatherG = 0.16F;
    constexpr float leatherB = 0.10F;
    constexpr float clothR = 0.58F;
    constexpr float clothG = 0.13F;
    constexpr float clothB = 0.34F;
    constexpr float skinR = 0.72F;
    constexpr float skinG = 0.48F;
    constexpr float skinB = 0.30F;
    constexpr float shieldR = 0.45F;
    constexpr float shieldG = 0.48F;
    constexpr float shieldB = 0.52F;

    appendBoxAt(vertices, -0.13F, 0.0F, 0.09F, 0.11F, 0.06F, 0.28F, leatherR, leatherG, leatherB);
    appendBoxAt(vertices,  0.13F, 0.0F, 0.09F, 0.11F, 0.06F, 0.28F, leatherR, leatherG, leatherB);
    appendBoxAt(vertices, 0.0F, 0.0F, 0.23F, 0.15F, 0.26F, 0.66F, clothR, clothG, clothB);
    appendBoxAt(vertices, 0.0F, 0.0F, 0.15F, 0.14F, 0.66F, 0.91F, skinR, skinG, skinB);
    appendBoxAt(vertices, -0.29F, 0.0F, 0.06F, 0.08F, 0.38F, 0.66F, skinR, skinG, skinB);
    appendBoxAt(vertices,  0.29F, 0.0F, 0.06F, 0.08F, 0.38F, 0.66F, skinR, skinG, skinB);
    appendBoxAt(vertices, -0.35F, 0.02F, 0.07F, 0.23F, 0.30F, 0.74F, shieldR, shieldG, shieldB);
    appendBoxAt(vertices, 0.0F, -0.12F, 0.18F, 0.04F, 0.86F, 0.97F, leatherR, leatherG, leatherB);
}

void appendTower(std::vector<Vertex>& vertices) {
    constexpr float stoneR = 0.43F;
    constexpr float stoneG = 0.49F;
    constexpr float stoneB = 0.57F;
    constexpr float darkStoneR = 0.27F;
    constexpr float darkStoneG = 0.32F;
    constexpr float darkStoneB = 0.39F;
    constexpr float woodR = 0.42F;
    constexpr float woodG = 0.24F;
    constexpr float woodB = 0.10F;
    constexpr float metalR = 0.72F;
    constexpr float metalG = 0.76F;
    constexpr float metalB = 0.80F;

    appendBoxAt(vertices, 0.0F, 0.0F, 0.42F, 0.42F, 0.06F, 0.22F, darkStoneR, darkStoneG, darkStoneB);
    appendBoxAt(vertices, 0.0F, 0.0F, 0.31F, 0.31F, 0.20F, 0.88F, stoneR, stoneG, stoneB);
    appendBoxAt(vertices, 0.0F, 0.0F, 0.39F, 0.39F, 0.86F, 1.00F, darkStoneR, darkStoneG, darkStoneB);
    appendBoxAt(vertices, -0.30F, -0.30F, 0.09F, 0.09F, 0.98F, 1.19F, stoneR, stoneG, stoneB);
    appendBoxAt(vertices,  0.30F, -0.30F, 0.09F, 0.09F, 0.98F, 1.19F, stoneR, stoneG, stoneB);
    appendBoxAt(vertices, -0.30F,  0.30F, 0.09F, 0.09F, 0.98F, 1.19F, stoneR, stoneG, stoneB);
    appendBoxAt(vertices,  0.30F,  0.30F, 0.09F, 0.09F, 0.98F, 1.19F, stoneR, stoneG, stoneB);
    appendBoxAt(vertices, 0.0F, 0.0F, 0.08F, 0.27F, 1.02F, 1.11F, woodR, woodG, woodB);
    appendBoxAt(vertices, -0.22F, 0.0F, 0.22F, 0.04F, 1.05F, 1.13F, metalR, metalG, metalB);
    appendBoxAt(vertices,  0.22F, 0.0F, 0.22F, 0.04F, 1.05F, 1.13F, metalR, metalG, metalB);
    appendBoxAt(vertices, 0.0F, -0.28F, 0.04F, 0.12F, 1.07F, 1.15F, metalR, metalG, metalB);
}

void appendProjectile(std::vector<Vertex>& vertices) {
    appendBoxAt(vertices, 0.0F, 0.0F, 0.035F, 0.13F, -0.025F, 0.025F, 0.78F, 0.48F, 0.13F);
    appendBoxAt(vertices, 0.0F, -0.15F, 0.07F, 0.06F, -0.05F, 0.05F, 0.82F, 0.86F, 0.90F);
}

float worldX(const LevelData& level, std::size_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::size_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

void drawDynamicText(C2D_TextBuf buffer, const char* value, float x, float y, float scale, u32 color) {
    C2D_Text text{};
    C2D_TextParse(&text, buffer, value);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor, x, y, 0.5F, scale, scale, color);
}

const char* instructionFor(const TutorialFlow& flow) {
    switch (flow.phase()) {
        case TutorialPhase::BuildFirstTower: return "1. D-PAD: WYBIERZ NIEBIESKIE POLE\n2. A: ZBUDUJ PIERWSZA WIEZE";
        case TutorialPhase::ReadyToStart: return "WIEZA GOTOWA\nNACISNIJ X, ABY URUCHOMIC FALE";
        case TutorialPhase::WaveRunning: return "BRON CYTADELI\nWIEZE STRZELAJA AUTOMATYCZNIE";
        case TutorialPhase::Victory: return "ZWYCIESTWO!\nNACISNIJ Y, ABY ZAGRAC PONOWNIE";
        case TutorialPhase::Defeat: return "PORAZKA - BAZA UPADLA\nNACISNIJ Y, ABY SPROBOWAC PONOWNIE";
    }
    return "";
}

const char* buildFeedbackText(BuildAttemptResult result) {
    switch (result) {
        case BuildAttemptResult::Built: return "WIEZA ZBUDOWANA";
        case BuildAttemptResult::NoBuildSpot: return "BRAK POLA BUDOWY";
        case BuildAttemptResult::Occupied: return "TO POLE JEST JUZ ZAJETE";
        case BuildAttemptResult::InsufficientGold: return "ZA MALO ZLOTA";
        case BuildAttemptResult::TowerLimitReached: return "OSIAGNIETO LIMIT WIEZ";
        case BuildAttemptResult::InvalidTower: return "NIE MOZNA TU ZBUDOWAC";
        case BuildAttemptResult::None:
        default: return "NIEBIESKIE = BUDOWA | BRAZOWE = DROGA";
    }
}

u32 buildFeedbackColor(BuildAttemptResult result) {
    if (result == BuildAttemptResult::Built) {
        return kHudGood;
    }
    if (result == BuildAttemptResult::None) {
        return kHudMuted;
    }
    return kHudWarning;
}

}  // namespace

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(const LevelData& level) {
    level_ = &level;
    topTarget_ = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (topTarget_ == nullptr) {
        return false;
    }
    C3D_RenderTargetSetOutput(topTarget_, GFX_TOP, GFX_LEFT, kDisplayTransferFlags);

    if (!initializeHud()) {
        return false;
    }

    auto* shaderData = reinterpret_cast<u32*>(const_cast<u8*>(vshader_shbin));
    shaderBinary_ = DVLB_ParseFile(shaderData, vshader_shbin_size);
    if (shaderBinary_ == nullptr) {
        return false;
    }

    shaderProgramInit(&shaderProgram_);
    shaderProgramInitialized_ = true;
    shaderProgramSetVsh(&shaderProgram_, &shaderBinary_->DVLE[0]);

    projectionUniform_ = shaderInstanceGetUniformLocation(shaderProgram_.vertexShader, "projection");
    modelViewUniform_ = shaderInstanceGetUniformLocation(shaderProgram_.vertexShader, "modelView");
    if (projectionUniform_ < 0 || modelViewUniform_ < 0) {
        return false;
    }

    return buildLevelMesh(level);
}

bool Renderer::initializeHud() {
    if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)) {
        return false;
    }
    citro2dInitialized_ = true;
    C2D_Prepare();

    bottomTarget_ = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    if (bottomTarget_ == nullptr) {
        return false;
    }

    staticTextBuffer_ = C2D_TextBufNew(384);
    dynamicTextBuffer_ = C2D_TextBufNew(1024);
    if (staticTextBuffer_ == nullptr || dynamicTextBuffer_ == nullptr) {
        return false;
    }

    C2D_TextParse(&titleText_, staticTextBuffer_, "CITADEL DEFENSE 3D");
    C2D_TextOptimize(&titleText_);
    C2D_TextParse(
        &controlsText_,
        staticTextBuffer_,
        "D-PAD: POLE   A: BUDUJ   X: START FALI\nL/R: OBROT   CIRCLE PAD: KAMERA   START: WYJSCIE");
    C2D_TextOptimize(&controlsText_);
    return true;
}

bool Renderer::buildLevelMesh(const LevelData& level) {
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<std::size_t>(level.width) * level.height * 6U + 900U);

    const float offsetX = -static_cast<float>(level.width) * 0.5F + 0.5F;
    const float offsetZ = -static_cast<float>(level.height) * 0.5F + 0.5F;
    for (std::size_t z = 0; z < level.height; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            appendTile(vertices, offsetX + static_cast<float>(x), offsetZ + static_cast<float>(z), visualFor(level.tileAt(x, z)));
        }
    }

    levelVertexCount_ = vertices.size();
    enemyVertexOffset_ = vertices.size();
    appendEnemy(vertices);
    enemyVertexCount_ = vertices.size() - enemyVertexOffset_;
    towerVertexOffset_ = vertices.size();
    appendTower(vertices);
    towerVertexCount_ = vertices.size() - towerVertexOffset_;
    projectileVertexOffset_ = vertices.size();
    appendProjectile(vertices);
    projectileVertexCount_ = vertices.size() - projectileVertexOffset_;

    const std::size_t bytes = vertices.size() * sizeof(Vertex);
    vertexBuffer_ = linearAlloc(bytes);
    if (vertexBuffer_ == nullptr) {
        return false;
    }
    std::memcpy(vertexBuffer_, vertices.data(), bytes);
    return true;
}

void Renderer::render(
    const Camera& camera,
    const Wave& wave,
    const BuildSystem& buildSystem,
    const TutorialFlow& tutorialFlow) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    drawBottomPanel(camera, wave, buildSystem, tutorialFlow);
    drawScene(camera, wave, buildSystem);
    C3D_FrameEnd(0);
}

void Renderer::drawScene(const Camera& camera, const Wave& wave, const BuildSystem& buildSystem) {
    C3D_RenderTargetClear(topTarget_, C3D_CLEAR_ALL, kTopClearColor, 0);
    C3D_FrameDrawOn(topTarget_);

    C3D_BindProgram(&shaderProgram_);
    C3D_AttrInfo* attributes = C3D_GetAttrInfo();
    AttrInfo_Init(attributes);
    AttrInfo_AddLoader(attributes, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attributes, 1, GPU_FLOAT, 4);

    C3D_BufInfo* buffers = C3D_GetBufInfo();
    BufInfo_Init(buffers);
    BufInfo_Add(buffers, vertexBuffer_, sizeof(Vertex), 2, 0x10);

    C3D_TexEnv* environment = C3D_GetTexEnv(0);
    C3D_TexEnvInit(environment);
    C3D_TexEnvSrc(environment, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(environment, C3D_Both, GPU_REPLACE);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_CullFace(GPU_CULL_BACK_CCW);

    C3D_Mtx projection{};
    C3D_Mtx modelView{};
    camera.writeProjection(projection);
    camera.writeView(modelView);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projectionUniform_, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &modelView);
    C3D_DrawArrays(GPU_TRIANGLES, 0, static_cast<int>(levelVertexCount_));

    for (std::size_t index = 0; index < buildSystem.towerCount(); ++index) {
        const Tower& tower = buildSystem.towerAt(index);
        C3D_Mtx towerView{};
        camera.writeView(towerView);
        Mtx_Translate(&towerView, tower.x(), 0.0F, tower.z(), true);
        Mtx_RotateY(&towerView, tower.aimAngleRadians(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &towerView);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(towerVertexOffset_), static_cast<int>(towerVertexCount_));
    }

    if (level_ != nullptr) {
        C3D_Mtx cursorView{};
        camera.writeView(cursorView);
        Mtx_Translate(
            &cursorView,
            worldX(*level_, buildSystem.cursorX()),
            buildSystem.cursorCanBuild() ? -0.02F : -0.65F,
            worldZ(*level_, buildSystem.cursorZ()),
            true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &cursorView);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(towerVertexOffset_), static_cast<int>(towerVertexCount_));
    }

    for (std::size_t index = 0; index < wave.spawnedCount(); ++index) {
        const Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead() || enemy.reachedBase()) {
            continue;
        }
        C3D_Mtx enemyView{};
        camera.writeView(enemyView);
        Mtx_Translate(&enemyView, enemy.x(), 0.0F, enemy.z(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &enemyView);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(enemyVertexOffset_), static_cast<int>(enemyVertexCount_));
    }

    const ProjectilePool& projectiles = buildSystem.projectiles();
    for (std::size_t index = 0; index < ProjectilePool::kCapacity; ++index) {
        const Projectile& projectile = projectiles.projectileAt(index);
        if (!projectile.active()) {
            continue;
        }
        C3D_Mtx projectileView{};
        camera.writeView(projectileView);
        Mtx_Translate(&projectileView, projectile.x(), projectile.y(), projectile.z(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &projectileView);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(projectileVertexOffset_), static_cast<int>(projectileVertexCount_));
    }
}

void Renderer::drawBottomPanel(
    const Camera&,
    const Wave& wave,
    const BuildSystem& buildSystem,
    const TutorialFlow& tutorialFlow) {
    C2D_TargetClear(bottomTarget_, kHudBackground);
    C2D_SceneBegin(bottomTarget_);

    C2D_DrawRectSolid(8.0F, 8.0F, 0.1F, 304.0F, 30.0F, kHudPanel);
    C2D_DrawRectSolid(8.0F, 44.0F, 0.1F, 304.0F, 58.0F, kHudAccent);
    C2D_DrawRectSolid(8.0F, 108.0F, 0.1F, 304.0F, 54.0F, kHudPanel);
    C2D_DrawRectSolid(8.0F, 168.0F, 0.1F, 304.0F, 28.0F, kHudPanel);
    C2D_DrawRectSolid(8.0F, 202.0F, 0.1F, 304.0F, 32.0F, kHudPanel);

    C2D_DrawText(&titleText_, C2D_WithColor | C2D_AlignCenter, 160.0F, 12.0F, 0.5F, 0.54F, 0.54F, kHudText);
    C2D_DrawText(&controlsText_, C2D_WithColor | C2D_AlignCenter, 160.0F, 205.0F, 0.5F, 0.32F, 0.32F, kHudMuted);

    C2D_TextBufClear(dynamicTextBuffer_);
    char text[128];

    drawDynamicText(dynamicTextBuffer_, "CO TERAZ:", 18.0F, 49.0F, 0.38F, kHudMuted);
    drawDynamicText(dynamicTextBuffer_, instructionFor(tutorialFlow), 18.0F, 64.0F, 0.52F,
        tutorialFlow.phase() == TutorialPhase::Defeat ? kHudDanger : kHudText);

    std::snprintf(text, sizeof(text), "BAZA %d", wave.baseHealth());
    drawDynamicText(dynamicTextBuffer_, text, 18.0F, 116.0F, 0.48F, wave.baseHealth() <= 2 ? kHudDanger : kHudText);
    std::snprintf(text, sizeof(text), "ZLOTO %d", buildSystem.gold());
    drawDynamicText(dynamicTextBuffer_, text, 112.0F, 116.0F, 0.48F, kHudText);
    std::snprintf(text, sizeof(text), "KOSZT %d", buildSystem.towerCost());
    drawDynamicText(dynamicTextBuffer_, text, 218.0F, 116.0F, 0.42F, kHudMuted);

    std::snprintf(text, sizeof(text), "WIEZE %zu", buildSystem.towerCount());
    drawDynamicText(dynamicTextBuffer_, text, 18.0F, 140.0F, 0.42F, kHudText);
    std::snprintf(text, sizeof(text), "FALA %zu/%zu", wave.spawnedCount(), wave.enemyCount());
    drawDynamicText(dynamicTextBuffer_, text, 112.0F, 140.0F, 0.42F, kHudText);
    std::snprintf(text, sizeof(text), "POLE %zu,%zu", buildSystem.cursorX(), buildSystem.cursorZ());
    drawDynamicText(dynamicTextBuffer_, text, 218.0F, 140.0F, 0.36F, kHudMuted);

    drawDynamicText(
        dynamicTextBuffer_,
        buildFeedbackText(buildSystem.lastBuildResult()),
        18.0F,
        175.0F,
        0.40F,
        buildFeedbackColor(buildSystem.lastBuildResult()));
}

void Renderer::shutdown() {
    if (vertexBuffer_ != nullptr) {
        linearFree(vertexBuffer_);
        vertexBuffer_ = nullptr;
    }
    level_ = nullptr;
    levelVertexCount_ = 0;
    enemyVertexOffset_ = 0;
    enemyVertexCount_ = 0;
    towerVertexOffset_ = 0;
    towerVertexCount_ = 0;
    projectileVertexOffset_ = 0;
    projectileVertexCount_ = 0;

    if (staticTextBuffer_ != nullptr) {
        C2D_TextBufDelete(staticTextBuffer_);
        staticTextBuffer_ = nullptr;
    }
    if (dynamicTextBuffer_ != nullptr) {
        C2D_TextBufDelete(dynamicTextBuffer_);
        dynamicTextBuffer_ = nullptr;
    }
    if (citro2dInitialized_) {
        C2D_Fini();
        citro2dInitialized_ = false;
    }

    if (shaderProgramInitialized_) {
        shaderProgramFree(&shaderProgram_);
        shaderProgramInitialized_ = false;
    }
    if (shaderBinary_ != nullptr) {
        DVLB_Free(shaderBinary_);
        shaderBinary_ = nullptr;
    }
    topTarget_ = nullptr;
    bottomTarget_ = nullptr;
}
