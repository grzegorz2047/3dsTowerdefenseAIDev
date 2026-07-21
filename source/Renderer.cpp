#include "Renderer.hpp"

#include <3ds.h>

#include <cstring>

#include "vshader_shbin.h"

namespace {

constexpr u32 kTopClearColor = 0x182030FF;
constexpr u32 kBottomClearColor = 0x101820FF;
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

constexpr Vertex kVertices[] = {
    {-6.0F, 0.0F, -6.0F, 0.18F, 0.42F, 0.20F, 1.0F},
    { 6.0F, 0.0F, -6.0F, 0.18F, 0.42F, 0.20F, 1.0F},
    { 6.0F, 0.0F,  6.0F, 0.24F, 0.50F, 0.25F, 1.0F},
    { 6.0F, 0.0F,  6.0F, 0.24F, 0.50F, 0.25F, 1.0F},
    {-6.0F, 0.0F,  6.0F, 0.24F, 0.50F, 0.25F, 1.0F},
    {-6.0F, 0.0F, -6.0F, 0.18F, 0.42F, 0.20F, 1.0F},

    {-0.5F, 0.0F,  0.5F, 0.65F, 0.65F, 0.72F, 1.0F},
    { 0.5F, 0.0F,  0.5F, 0.65F, 0.65F, 0.72F, 1.0F},
    { 0.5F, 1.0F,  0.5F, 0.82F, 0.82F, 0.90F, 1.0F},
    { 0.5F, 1.0F,  0.5F, 0.82F, 0.82F, 0.90F, 1.0F},
    {-0.5F, 1.0F,  0.5F, 0.82F, 0.82F, 0.90F, 1.0F},
    {-0.5F, 0.0F,  0.5F, 0.65F, 0.65F, 0.72F, 1.0F},

    { 0.5F, 0.0F, -0.5F, 0.52F, 0.52F, 0.60F, 1.0F},
    {-0.5F, 0.0F, -0.5F, 0.52F, 0.52F, 0.60F, 1.0F},
    {-0.5F, 1.0F, -0.5F, 0.70F, 0.70F, 0.78F, 1.0F},
    {-0.5F, 1.0F, -0.5F, 0.70F, 0.70F, 0.78F, 1.0F},
    { 0.5F, 1.0F, -0.5F, 0.70F, 0.70F, 0.78F, 1.0F},
    { 0.5F, 0.0F, -0.5F, 0.52F, 0.52F, 0.60F, 1.0F},

    { 0.5F, 0.0F,  0.5F, 0.70F, 0.58F, 0.24F, 1.0F},
    { 0.5F, 0.0F, -0.5F, 0.70F, 0.58F, 0.24F, 1.0F},
    { 0.5F, 1.0F, -0.5F, 0.88F, 0.72F, 0.30F, 1.0F},
    { 0.5F, 1.0F, -0.5F, 0.88F, 0.72F, 0.30F, 1.0F},
    { 0.5F, 1.0F,  0.5F, 0.88F, 0.72F, 0.30F, 1.0F},
    { 0.5F, 0.0F,  0.5F, 0.70F, 0.58F, 0.24F, 1.0F},

    {-0.5F, 0.0F, -0.5F, 0.55F, 0.43F, 0.18F, 1.0F},
    {-0.5F, 0.0F,  0.5F, 0.55F, 0.43F, 0.18F, 1.0F},
    {-0.5F, 1.0F,  0.5F, 0.75F, 0.58F, 0.22F, 1.0F},
    {-0.5F, 1.0F,  0.5F, 0.75F, 0.58F, 0.22F, 1.0F},
    {-0.5F, 1.0F, -0.5F, 0.75F, 0.58F, 0.22F, 1.0F},
    {-0.5F, 0.0F, -0.5F, 0.55F, 0.43F, 0.18F, 1.0F},

    {-0.5F, 1.0F,  0.5F, 0.84F, 0.84F, 0.92F, 1.0F},
    { 0.5F, 1.0F,  0.5F, 0.84F, 0.84F, 0.92F, 1.0F},
    { 0.5F, 1.0F, -0.5F, 0.84F, 0.84F, 0.92F, 1.0F},
    { 0.5F, 1.0F, -0.5F, 0.84F, 0.84F, 0.92F, 1.0F},
    {-0.5F, 1.0F, -0.5F, 0.84F, 0.84F, 0.92F, 1.0F},
    {-0.5F, 1.0F,  0.5F, 0.84F, 0.84F, 0.92F, 1.0F},

    {-0.5F, 0.0F, -0.5F, 0.35F, 0.35F, 0.40F, 1.0F},
    { 0.5F, 0.0F, -0.5F, 0.35F, 0.35F, 0.40F, 1.0F},
    { 0.5F, 0.0F,  0.5F, 0.35F, 0.35F, 0.40F, 1.0F},
    { 0.5F, 0.0F,  0.5F, 0.35F, 0.35F, 0.40F, 1.0F},
    {-0.5F, 0.0F,  0.5F, 0.35F, 0.35F, 0.40F, 1.0F},
    {-0.5F, 0.0F, -0.5F, 0.35F, 0.35F, 0.40F, 1.0F},
};

constexpr int kGroundVertexCount = 6;
constexpr int kTowerVertexCount = 36;

}  // namespace

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize() {
    topTarget_ = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    bottomTarget_ = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (topTarget_ == nullptr || bottomTarget_ == nullptr) {
        return false;
    }

    C3D_RenderTargetSetOutput(topTarget_, GFX_TOP, GFX_LEFT, kDisplayTransferFlags);
    C3D_RenderTargetSetOutput(bottomTarget_, GFX_BOTTOM, GFX_LEFT, kDisplayTransferFlags);

    auto* shaderData = reinterpret_cast<u32*>(const_cast<u8*>(vshader_shbin));
    shaderBinary_ = DVLB_ParseFile(shaderData, vshader_shbin_size);
    if (shaderBinary_ == nullptr) {
        return false;
    }

    shaderProgramInit(&shaderProgram_);
    shaderProgramInitialized_ = true;
    shaderProgramSetVsh(&shaderProgram_, &shaderBinary_->DVLE[0]);
    C3D_BindProgram(&shaderProgram_);

    projectionUniform_ = shaderInstanceGetUniformLocation(shaderProgram_.vertexShader, "projection");
    modelViewUniform_ = shaderInstanceGetUniformLocation(shaderProgram_.vertexShader, "modelView");
    if (projectionUniform_ < 0 || modelViewUniform_ < 0) {
        return false;
    }

    C3D_AttrInfo* attributes = C3D_GetAttrInfo();
    AttrInfo_Init(attributes);
    AttrInfo_AddLoader(attributes, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attributes, 1, GPU_FLOAT, 4);

    vertexBuffer_ = linearAlloc(sizeof(kVertices));
    if (vertexBuffer_ == nullptr) {
        return false;
    }
    std::memcpy(vertexBuffer_, kVertices, sizeof(kVertices));

    C3D_BufInfo* buffers = C3D_GetBufInfo();
    BufInfo_Init(buffers);
    BufInfo_Add(buffers, vertexBuffer_, sizeof(Vertex), 2, 0x10);

    C3D_TexEnv* environment = C3D_GetTexEnv(0);
    C3D_TexEnvInit(environment);
    C3D_TexEnvSrc(environment, C3D_Both, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(environment, C3D_Both, GPU_REPLACE);

    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_CullFace(GPU_CULL_BACK_CCW);
    return true;
}

void Renderer::render(const Camera& camera) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    drawScene(camera);
    drawBottomPanel(camera);
    C3D_FrameEnd(0);
}

void Renderer::drawScene(const Camera& camera) {
    C3D_RenderTargetClear(topTarget_, C3D_CLEAR_ALL, kTopClearColor, 0);
    C3D_FrameDrawOn(topTarget_);

    C3D_Mtx projection{};
    C3D_Mtx groundView{};
    camera.writeProjection(projection);
    camera.writeView(groundView);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projectionUniform_, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &groundView);
    C3D_DrawArrays(GPU_TRIANGLES, 0, kGroundVertexCount);

    C3D_Mtx towerView{};
    camera.writeView(towerView);
    Mtx_Translate(&towerView, 0.0F, 0.0F, 0.0F, true);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &towerView);
    C3D_DrawArrays(GPU_TRIANGLES, kGroundVertexCount, kTowerVertexCount);
}

void Renderer::drawBottomPanel(const Camera& camera) {
    const u32 rotationShade = static_cast<u32>(camera.rotationIndex()) * 0x08080000U;
    C3D_RenderTargetClear(bottomTarget_, C3D_CLEAR_ALL, kBottomClearColor + rotationShade, 0);
    C3D_FrameDrawOn(bottomTarget_);
}

void Renderer::shutdown() {
    if (vertexBuffer_ != nullptr) {
        linearFree(vertexBuffer_);
        vertexBuffer_ = nullptr;
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
