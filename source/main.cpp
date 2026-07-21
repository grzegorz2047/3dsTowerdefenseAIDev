#include <3ds.h>
#include <citro3d.h>

#include <cstring>

#include "vshader_shbin.h"

namespace {

constexpr u32 kClearColor = 0x182030FF;

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
    {-0.65F, -0.45F, 0.0F, 0.2F, 0.8F, 0.3F, 1.0F},
    { 0.65F, -0.45F, 0.0F, 0.9F, 0.6F, 0.2F, 1.0F},
    { 0.00F,  0.65F, 0.0F, 0.3F, 0.5F, 1.0F, 1.0F},
};

DVLB_s* shaderBinary = nullptr;
shaderProgram_s shaderProgram{};
int projectionUniform = -1;
C3D_Mtx projection{};
void* vertexBuffer = nullptr;
C3D_RenderTarget* topTarget = nullptr;

bool initializeScene() {
    shaderBinary = DVLB_ParseFile(
        reinterpret_cast<u32*>(vshader_shbin),
        vshader_shbin_size);
    if (shaderBinary == nullptr) {
        return false;
    }

    shaderProgramInit(&shaderProgram);
    shaderProgramSetVsh(&shaderProgram, &shaderBinary->DVLE[0]);
    C3D_BindProgram(&shaderProgram);

    projectionUniform = shaderInstanceGetUniformLocation(
        shaderProgram.vertexShader,
        "projection");
    if (projectionUniform < 0) {
        return false;
    }

    C3D_AttrInfo* attributes = C3D_GetAttrInfo();
    AttrInfo_Init(attributes);
    AttrInfo_AddLoader(attributes, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attributes, 1, GPU_FLOAT, 4);

    Mtx_OrthoTilt(&projection, -1.0F, 1.0F, -1.0F, 1.0F, 0.0F, 1.0F, true);

    vertexBuffer = linearAlloc(sizeof(kVertices));
    if (vertexBuffer == nullptr) {
        return false;
    }
    std::memcpy(vertexBuffer, kVertices, sizeof(kVertices));

    C3D_BufInfo* buffers = C3D_GetBufInfo();
    BufInfo_Init(buffers);
    BufInfo_Add(buffers, vertexBuffer, sizeof(Vertex), 2, 0x10);

    C3D_TexEnv* environment = C3D_GetTexEnv(0);
    C3D_TexEnvInit(environment);
    C3D_TexEnvSrc(environment, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
    C3D_TexEnvFunc(environment, C3D_Both, GPU_REPLACE);

    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    return true;
}

void renderScene() {
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projectionUniform, &projection);
    C3D_DrawArrays(GPU_TRIANGLES, 0, 3);
}

void shutdownScene() {
    if (vertexBuffer != nullptr) {
        linearFree(vertexBuffer);
        vertexBuffer = nullptr;
    }

    shaderProgramFree(&shaderProgram);
    if (shaderBinary != nullptr) {
        DVLB_Free(shaderBinary);
        shaderBinary = nullptr;
    }
}

}  // namespace

int main() {
    gfxInitDefault();

    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        gfxExit();
        return 1;
    }

    topTarget = C3D_RenderTargetCreate(
        240,
        400,
        GPU_RB_RGBA8,
        GPU_RB_DEPTH24_STENCIL8);
    if (topTarget == nullptr) {
        C3D_Fini();
        gfxExit();
        return 1;
    }

    C3D_RenderTargetSetOutput(
        topTarget,
        GFX_TOP,
        GFX_LEFT,
        kDisplayTransferFlags);

    if (!initializeScene()) {
        shutdownScene();
        C3D_Fini();
        gfxExit();
        return 1;
    }

    while (aptMainLoop()) {
        hidScanInput();
        if ((hidKeysDown() & KEY_START) != 0U) {
            break;
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(topTarget, C3D_CLEAR_ALL, kClearColor, 0);
        C3D_FrameDrawOn(topTarget);
        renderScene();
        C3D_FrameEnd(0);
    }

    shutdownScene();
    C3D_Fini();
    gfxExit();
    return 0;
}
