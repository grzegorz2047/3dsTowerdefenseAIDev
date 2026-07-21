#pragma once

#include <cstddef>

#include <citro3d.h>

#include "Camera.hpp"
#include "Level.hpp"
#include "Tower.hpp"
#include "Wave.hpp"

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] bool initialize(const LevelData& level);
    void render(const Camera& camera, const Wave& wave, const Tower& tower);
    void shutdown();

private:
    [[nodiscard]] bool buildLevelMesh(const LevelData& level);
    void drawScene(const Camera& camera, const Wave& wave, const Tower& tower);
    void drawBottomPanel(const Camera& camera, const Wave& wave);

    C3D_RenderTarget* topTarget_ = nullptr;
    C3D_RenderTarget* bottomTarget_ = nullptr;
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
};
