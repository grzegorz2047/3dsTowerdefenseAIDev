#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <citro3d.h>

#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "Level.hpp"
#include "Stereo3D.hpp"
#include "TutorialFlow.hpp"
#include "UiState.hpp"
#include "Wave.hpp"

class UiRenderer;

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] bool initialize(const LevelData& level, std::size_t benchmarkDecorations = 0U);
    void render(
        const Camera& camera,
        const Wave& wave,
        const BuildSystem& buildSystem,
        const TutorialFlow& tutorialFlow,
        std::uint8_t maximum3DDepthPercent,
        UiRenderer& uiRenderer,
        const UiState& uiState);
    void shutdown();

    [[nodiscard]] std::uint8_t lastEyeCount() const;
    [[nodiscard]] float lastStereoSlider() const;
    [[nodiscard]] float lastStereoSeparation() const;

private:
    [[nodiscard]] bool buildLevelMesh(const LevelData& level);
    void drawScene(C3D_RenderTarget* target, const Camera& camera,
        const Wave& wave, const BuildSystem& buildSystem, float eyeIod);

    const LevelData* level_ = nullptr;
    C3D_RenderTarget* topLeftTarget_ = nullptr;
    C3D_RenderTarget* topRightTarget_ = nullptr;
    DVLB_s* shaderBinary_ = nullptr;
    shaderProgram_s shaderProgram_{};
    bool shaderProgramInitialized_ = false;
    int projectionUniform_ = -1;
    int modelViewUniform_ = -1;
    void* vertexBuffer_ = nullptr;
    std::size_t levelVertexCount_ = 0;
    std::size_t enemyVertexOffset_ = 0;
    std::size_t enemyVertexCount_ = 0;
    std::array<std::size_t, 4U> towerVertexOffsets_{};
    std::array<std::size_t, 4U> towerVertexCounts_{};
    std::size_t projectileVertexOffset_ = 0U;
    std::size_t projectileVertexCount_ = 0U;
    std::size_t rocketVertexOffset_ = 0U;
    std::size_t rocketVertexCount_ = 0U;
    StereoFramePlan lastStereoPlan_{};
    std::size_t benchmarkDecorations_ = 0U;
};
