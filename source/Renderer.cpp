#include "Renderer.hpp"

#include <3ds.h>

#include <cstring>
#include <vector>

#include "UiRenderer.hpp"
#include "vshader_shbin.h"

namespace {

constexpr u32 kTopClearColor = 0x182030FF;
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
        case TileType::Spawn: return {0.16F, 0.24F, 0.20F, 0.08F, 0.05F};
        case TileType::Base: return {0.34F, 0.29F, 0.25F, 0.16F, 0.05F};
        case TileType::Ground:
        default: return {0.18F, 0.46F, 0.22F, 0.0F, 0.04F};
    }
}

void appendQuad(std::vector<Vertex>& vertices, const Vertex& a, const Vertex& b,
    const Vertex& c, const Vertex& d) {
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
    vertices.push_back(c);
    vertices.push_back(d);
    vertices.push_back(a);
}

void appendBox(std::vector<Vertex>& vertices, float centerX, float centerZ,
    float halfX, float halfZ, float y0, float y1, float r, float g, float b) {
    const float x0 = centerX - halfX;
    const float x1 = centerX + halfX;
    const float z0 = centerZ - halfZ;
    const float z1 = centerZ + halfZ;
    appendQuad(vertices, {x0,y0,z1,r,g,b,1}, {x1,y0,z1,r,g,b,1}, {x1,y1,z1,r,g,b,1}, {x0,y1,z1,r,g,b,1});
    appendQuad(vertices, {x1,y0,z0,r,g,b,1}, {x0,y0,z0,r,g,b,1}, {x0,y1,z0,r,g,b,1}, {x1,y1,z0,r,g,b,1});
    appendQuad(vertices, {x1,y0,z1,r,g,b,1}, {x1,y0,z0,r,g,b,1}, {x1,y1,z0,r,g,b,1}, {x1,y1,z1,r,g,b,1});
    appendQuad(vertices, {x0,y0,z0,r,g,b,1}, {x0,y0,z1,r,g,b,1}, {x0,y1,z1,r,g,b,1}, {x0,y1,z0,r,g,b,1});
    appendQuad(vertices, {x0,y1,z1,r,g,b,1}, {x1,y1,z1,r,g,b,1}, {x1,y1,z0,r,g,b,1}, {x0,y1,z0,r,g,b,1});
}

void appendTile(std::vector<Vertex>& vertices, float x, float z, const TileVisual& visual) {
    const float half = 0.5F - visual.inset;
    appendQuad(vertices,
        {x-half, visual.height, z-half, visual.r, visual.g, visual.b, 1.0F},
        {x-half, visual.height, z+half, visual.r, visual.g, visual.b, 1.0F},
        {x+half, visual.height, z+half, visual.r, visual.g, visual.b, 1.0F},
        {x+half, visual.height, z-half, visual.r, visual.g, visual.b, 1.0F});
}

void appendCitadel(std::vector<Vertex>& vertices, float x, float z) {
    appendBox(vertices, x, z, 0.46F, 0.46F, 0.15F, 0.27F, 0.25F, 0.28F, 0.32F);
    appendBox(vertices, x, z, 0.28F, 0.28F, 0.25F, 0.82F, 0.47F, 0.50F, 0.54F);
    appendBox(vertices, x, z, 0.34F, 0.34F, 0.80F, 0.92F, 0.25F, 0.28F, 0.32F);
    appendBox(vertices, x, z + 0.31F, 0.03F, 0.03F, 0.68F, 1.13F, 0.82F, 0.62F, 0.18F);
    appendBox(vertices, x, z + 0.34F, 0.13F, 0.03F, 1.02F, 1.10F, 0.64F, 0.12F, 0.16F);
}

void appendGate(std::vector<Vertex>& vertices, float x, float z) {
    appendBox(vertices, x, z - 0.30F, 0.13F, 0.13F, 0.08F, 0.78F, 0.30F, 0.32F, 0.34F);
    appendBox(vertices, x, z + 0.30F, 0.13F, 0.13F, 0.08F, 0.78F, 0.30F, 0.32F, 0.34F);
    appendBox(vertices, x, z, 0.14F, 0.43F, 0.66F, 0.84F, 0.16F, 0.18F, 0.20F);
    appendBox(vertices, x + 0.03F, z, 0.04F, 0.18F, 0.16F, 0.66F, 0.14F, 0.78F, 0.62F);
}

void appendEnemy(std::vector<Vertex>& vertices) {
    appendBox(vertices, 0.0F, 0.0F, 0.23F, 0.15F, 0.12F, 0.66F, 0.58F, 0.13F, 0.34F);
    appendBox(vertices, 0.0F, 0.0F, 0.15F, 0.14F, 0.66F, 0.91F, 0.72F, 0.48F, 0.30F);
    appendBox(vertices, -0.28F, 0.0F, 0.07F, 0.22F, 0.30F, 0.74F, 0.45F, 0.48F, 0.52F);
}

void appendTower(std::vector<Vertex>& vertices) {
    appendBox(vertices, 0.0F, 0.0F, 0.42F, 0.42F, 0.06F, 0.22F, 0.27F, 0.32F, 0.39F);
    appendBox(vertices, 0.0F, 0.0F, 0.31F, 0.31F, 0.20F, 0.88F, 0.43F, 0.49F, 0.57F);
    appendBox(vertices, 0.0F, 0.0F, 0.39F, 0.39F, 0.86F, 1.00F, 0.27F, 0.32F, 0.39F);
    appendBox(vertices, 0.0F, -0.33F, 0.08F, 0.30F, 0.92F, 1.04F, 0.42F, 0.24F, 0.10F);
}

void appendProjectile(std::vector<Vertex>& vertices) {
    appendBox(vertices, 0.0F, 0.0F, 0.06F, 0.06F, -0.06F, 0.06F, 0.94F, 0.76F, 0.24F);
}

float worldX(const LevelData& level, std::size_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::size_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

}  // namespace

Renderer::~Renderer() { shutdown(); }

bool Renderer::initialize(const LevelData& level) {
    shutdown();
    level_ = &level;
    gfxSet3D(true);

    topLeftTarget_ = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    topRightTarget_ = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (topLeftTarget_ == nullptr || topRightTarget_ == nullptr) {
        shutdown();
        return false;
    }
    C3D_RenderTargetSetOutput(topLeftTarget_, GFX_TOP, GFX_LEFT, kDisplayTransferFlags);
    C3D_RenderTargetSetOutput(topRightTarget_, GFX_TOP, GFX_RIGHT, kDisplayTransferFlags);

    auto* shaderData = reinterpret_cast<u32*>(const_cast<u8*>(vshader_shbin));
    shaderBinary_ = DVLB_ParseFile(shaderData, vshader_shbin_size);
    if (shaderBinary_ == nullptr) { shutdown(); return false; }
    shaderProgramInit(&shaderProgram_);
    shaderProgramInitialized_ = true;
    shaderProgramSetVsh(&shaderProgram_, &shaderBinary_->DVLE[0]);
    projectionUniform_ = shaderInstanceGetUniformLocation(shaderProgram_.vertexShader, "projection");
    modelViewUniform_ = shaderInstanceGetUniformLocation(shaderProgram_.vertexShader, "modelView");
    if (projectionUniform_ < 0 || modelViewUniform_ < 0 || !buildLevelMesh(level)) {
        shutdown();
        return false;
    }
    return true;
}

bool Renderer::buildLevelMesh(const LevelData& level) {
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<std::size_t>(level.width) * level.height * 6U + 1000U);
    const float offsetX = -static_cast<float>(level.width) * 0.5F + 0.5F;
    const float offsetZ = -static_cast<float>(level.height) * 0.5F + 0.5F;
    for (std::size_t z = 0; z < level.height; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            const TileType tile = level.tileAt(x, z);
            const float tileX = offsetX + static_cast<float>(x);
            const float tileZ = offsetZ + static_cast<float>(z);
            appendTile(vertices, tileX, tileZ, visualFor(tile));
            if (tile == TileType::Base) appendCitadel(vertices, tileX, tileZ);
            else if (tile == TileType::Spawn) appendGate(vertices, tileX, tileZ);
        }
    }
    levelVertexCount_ = vertices.size();
    enemyVertexOffset_ = vertices.size(); appendEnemy(vertices); enemyVertexCount_ = vertices.size() - enemyVertexOffset_;
    towerVertexOffset_ = vertices.size(); appendTower(vertices); towerVertexCount_ = vertices.size() - towerVertexOffset_;
    projectileVertexOffset_ = vertices.size(); appendProjectile(vertices); projectileVertexCount_ = vertices.size() - projectileVertexOffset_;

    const std::size_t bytes = vertices.size() * sizeof(Vertex);
    vertexBuffer_ = linearAlloc(bytes);
    if (vertexBuffer_ == nullptr) return false;
    std::memcpy(vertexBuffer_, vertices.data(), bytes);
    return true;
}

void Renderer::render(const Camera& camera, const Wave& wave,
    const BuildSystem& buildSystem, const TutorialFlow&,
    bool stereoEnabled, std::uint8_t maximum3DDepthPercent,
    UiRenderer& uiRenderer, const UiState& uiState) {
    lastStereoPlan_ = Stereo3D::plan(osGet3DSliderState(), stereoEnabled, maximum3DDepthPercent);
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    drawScene(topLeftTarget_, camera, wave, buildSystem, lastStereoPlan_.leftEyeIod);
    uiRenderer.renderTopOverlay(topLeftTarget_, uiState);
    if (lastStereoPlan_.stereo) {
        drawScene(topRightTarget_, camera, wave, buildSystem, lastStereoPlan_.rightEyeIod);
        uiRenderer.renderTopOverlay(topRightTarget_, uiState);
    }
    uiRenderer.renderBottom(uiState);
    C3D_FrameEnd(0);
}

void Renderer::drawScene(C3D_RenderTarget* target, const Camera& camera,
    const Wave& wave, const BuildSystem& buildSystem, float eyeIod) {
    C3D_RenderTargetClear(target, C3D_CLEAR_ALL, kTopClearColor, 0);
    C3D_FrameDrawOn(target);
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
    if (eyeIod == 0.0F) camera.writeProjection(projection);
    else camera.writeStereoProjection(projection, eyeIod);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projectionUniform_, &projection);

    C3D_Mtx view{};
    camera.writeView(view);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &view);
    C3D_DrawArrays(GPU_TRIANGLES, 0, static_cast<int>(levelVertexCount_));

    for (std::size_t index = 0; index < buildSystem.towerCount(); ++index) {
        const Tower& tower = buildSystem.towerAt(index);
        C3D_Mtx model{}; camera.writeView(model);
        Mtx_Translate(&model, tower.x(), 0.0F, tower.z(), true);
        Mtx_RotateY(&model, tower.aimAngleRadians(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &model);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(towerVertexOffset_), static_cast<int>(towerVertexCount_));
    }
    if (level_ != nullptr) {
        C3D_Mtx cursor{}; camera.writeView(cursor);
        Mtx_Translate(&cursor, worldX(*level_, buildSystem.cursorX()),
            buildSystem.cursorCanBuild() ? -0.02F : -0.65F, worldZ(*level_, buildSystem.cursorZ()), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &cursor);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(towerVertexOffset_), static_cast<int>(towerVertexCount_));
    }
    for (std::size_t index = 0; index < wave.spawnedCount(); ++index) {
        const Enemy& enemy = wave.enemyAt(index);
        if (enemy.dead() || enemy.reachedBase()) continue;
        C3D_Mtx model{}; camera.writeView(model);
        Mtx_Translate(&model, enemy.x(), 0.0F, enemy.z(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &model);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(enemyVertexOffset_), static_cast<int>(enemyVertexCount_));
    }
    const ProjectilePool& projectiles = buildSystem.projectiles();
    for (std::size_t index = 0; index < ProjectilePool::kCapacity; ++index) {
        const Projectile& projectile = projectiles.projectileAt(index);
        if (!projectile.active()) continue;
        C3D_Mtx model{}; camera.writeView(model);
        Mtx_Translate(&model, projectile.x(), projectile.y(), projectile.z(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &model);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(projectileVertexOffset_), static_cast<int>(projectileVertexCount_));
    }
}

std::uint8_t Renderer::lastEyeCount() const { return lastStereoPlan_.eyeCount; }
float Renderer::lastStereoSlider() const { return lastStereoPlan_.slider; }
float Renderer::lastStereoSeparation() const { return lastStereoPlan_.separation; }

void Renderer::shutdown() {
    if (vertexBuffer_ != nullptr) { linearFree(vertexBuffer_); vertexBuffer_ = nullptr; }
    if (shaderProgramInitialized_) { shaderProgramFree(&shaderProgram_); shaderProgramInitialized_ = false; }
    if (shaderBinary_ != nullptr) { DVLB_Free(shaderBinary_); shaderBinary_ = nullptr; }
    if (topRightTarget_ != nullptr) { C3D_RenderTargetDelete(topRightTarget_); topRightTarget_ = nullptr; }
    if (topLeftTarget_ != nullptr) { C3D_RenderTargetDelete(topLeftTarget_); topLeftTarget_ = nullptr; }
    level_ = nullptr;
    projectionUniform_ = -1;
    modelViewUniform_ = -1;
    levelVertexCount_ = enemyVertexOffset_ = enemyVertexCount_ = 0U;
    towerVertexOffset_ = towerVertexCount_ = projectileVertexOffset_ = projectileVertexCount_ = 0U;
    lastStereoPlan_ = {};
}
