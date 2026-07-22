#pragma once

#include <cstddef>
#include <cstdint>

#include <citro3d.h>

#include "BuildSystem.hpp"
#include "Camera.hpp"
#include "Level.hpp"
#include "Stereo3D.hpp"
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
    std::size_t towerVertexOffset_ = 0;
    std::size_t towerVertexCount_ = 0;
    std::size_t projectileVertexOffset_ = 0;
    std::size_t projectileVertexCount_ = 0;
    StereoFramePlan lastStereoPlan_{};
};
