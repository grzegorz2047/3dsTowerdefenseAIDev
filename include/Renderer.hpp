#pragma once

#include <cstddef>

#include <citro3d.h>

#include "Camera.hpp"
#include "Level.hpp"

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] bool initialize(const LevelData& level);
    void render(const Camera& camera);
    void shutdown();

private:
    [[nodiscard]] bool buildLevelMesh(const LevelData& level);
    void drawScene(const Camera& camera);
    void drawBottomPanel(const Camera& camera);

    C3D_RenderTarget* topTarget_ = nullptr;
    C3D_RenderTarget* bottomTarget_ = nullptr;
    DVLB_s* shaderBinary_ = nullptr;
    shaderProgram_s shaderProgram_{};
    bool shaderProgramInitialized_ = false;
    int projectionUniform_ = -1;
    int modelViewUniform_ = -1;
    void* vertexBuffer_ = nullptr;
    std::size_t vertexCount_ = 0;
};
