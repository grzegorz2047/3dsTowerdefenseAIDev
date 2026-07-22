#pragma once

#include <cstddef>

#include <citro2d.h>
#include <citro3d.h>

#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "Level.hpp"
#include "TutorialFlow.hpp"
#include "Wave.hpp"

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] bool initialize(const LevelData& level);
    void render(
        const Camera& camera,
        const Wave& wave,
        const BuildSystem& buildSystem,
        const TutorialFlow& tutorialFlow);
    void shutdown();

private:
    [[nodiscard]] bool buildLevelMesh(const LevelData& level);
    [[nodiscard]] bool initializeHud();
    void drawScene(const Camera& camera, const Wave& wave, const BuildSystem& buildSystem);
    void drawBottomPanel(
        const Camera& camera,
        const Wave& wave,
        const BuildSystem& buildSystem,
        const TutorialFlow& tutorialFlow);

    const LevelData* level_ = nullptr;
    C3D_RenderTarget* topTarget_ = nullptr;
    C3D_RenderTarget* bottomTarget_ = nullptr;
    DVLB_s* shaderBinary_ = nullptr;
    shaderProgram_s shaderProgram_{};
    bool shaderProgramInitialized_ = false;
    bool citro2dInitialized_ = false;
    C2D_TextBuf staticTextBuffer_ = nullptr;
    C2D_TextBuf dynamicTextBuffer_ = nullptr;
    C2D_Text titleText_{};
    C2D_Text controlsText_{};
    int projectionUniform_ = -1;
    int modelViewUniform_ = -1;
    void* vertexBuffer_ = nullptr;
    std::size_t levelVertexCount_ = 0;
    std::size_t enemyVertexOffset_ = 0;
    std::size_t enemyVertexCount_ = 0;
    std::size_t towerVertexOffset_ = 0;
    std::size_t towerVertexCount_ = 0;
    std::size_t projectileVertexOffset_ = 0;
    std::size_t projectileVertexCount_ = 0;
};

// Renderer.cpp includes this header before its implementation. Disable only its
// legacy bottom-screen Citro2D draw calls so they cannot overwrite the system
// PrintConsole fallback. The 3D top-screen pass remains unchanged.
#define C2D_TargetClear(...) ((void)0)
#define C2D_SceneBegin(...) ((void)0)
#define C2D_DrawRectSolid(...) ((void)0)
#define C2D_DrawText(...) ((void)0)
