#include "Renderer.hpp"

#include <3ds.h>

#include <cmath>
#include <cstring>
#include <vector>

#include "PerformanceBudget.hpp"
#include "SceneArt.hpp"
#include "UiRenderer.hpp"
#include "vshader_shbin.h"

namespace {

constexpr u32 kTopClearColor = 0x52677BFF;
constexpr u32 kDisplayTransferFlags =
    GX_TRANSFER_FLIP_VERT(0) |
    GX_TRANSFER_OUT_TILED(0) |
    GX_TRANSFER_RAW_COPY(0) |
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
    GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);
constexpr float kPi = 3.14159265358979323846F;

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

TileVisual visualFor(TileType tile, LevelTheme theme, std::size_t x, std::size_t z) {
    const float variation = ((x + z * 3U) % 3U == 0U) ? 0.025F : 0.0F;
    if (theme == LevelTheme::VhalPass) {
        switch (tile) {
            case TileType::Road: return {0.43F + variation, 0.34F, 0.25F, 0.035F, 0.055F};
            case TileType::BuildSpot: return {0.30F, 0.43F, 0.52F, 0.09F, 0.13F};
            case TileType::Blocked: return {0.27F, 0.29F, 0.28F, 0.24F + variation, 0.04F};
            case TileType::Spawn: return {0.20F, 0.27F, 0.24F, 0.10F, 0.05F};
            case TileType::Base: return {0.37F, 0.34F, 0.30F, 0.18F, 0.04F};
            case TileType::Ground:
            default: return {0.25F + variation, 0.39F + variation, 0.24F, 0.0F, 0.025F};
        }
    }
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

Vertex rotatedVertex(float centerX, float centerZ, float localX, float y, float localZ,
    float radians, float r, float g, float b) {
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    return {centerX + localX * cosine - localZ * sine, y,
        centerZ + localX * sine + localZ * cosine, r, g, b, 1.0F};
}

void appendRotatedBox(std::vector<Vertex>& vertices, float centerX, float centerZ,
    float halfX, float halfZ, float y0, float y1, float radians, float r, float g, float b) {
    const Vertex a0 = rotatedVertex(centerX, centerZ, -halfX, y0, halfZ, radians, r, g, b);
    const Vertex b0 = rotatedVertex(centerX, centerZ, halfX, y0, halfZ, radians, r, g, b);
    const Vertex c0 = rotatedVertex(centerX, centerZ, halfX, y0, -halfZ, radians, r, g, b);
    const Vertex d0 = rotatedVertex(centerX, centerZ, -halfX, y0, -halfZ, radians, r, g, b);
    const Vertex a1 = rotatedVertex(centerX, centerZ, -halfX, y1, halfZ, radians, r, g, b);
    const Vertex b1 = rotatedVertex(centerX, centerZ, halfX, y1, halfZ, radians, r, g, b);
    const Vertex c1 = rotatedVertex(centerX, centerZ, halfX, y1, -halfZ, radians, r, g, b);
    const Vertex d1 = rotatedVertex(centerX, centerZ, -halfX, y1, -halfZ, radians, r, g, b);
    appendQuad(vertices, a0, b0, b1, a1);
    appendQuad(vertices, c0, d0, d1, c1);
    appendQuad(vertices, b0, c0, c1, b1);
    appendQuad(vertices, d0, a0, a1, d1);
    appendQuad(vertices, a1, b1, c1, d1);
}

void appendTile(std::vector<Vertex>& vertices, float x, float z, const TileVisual& visual) {
    const float half = 0.5F - visual.inset;
    appendQuad(vertices,
        {x-half, visual.height, z-half, visual.r, visual.g, visual.b, 1.0F},
        {x-half, visual.height, z+half, visual.r, visual.g, visual.b, 1.0F},
        {x+half, visual.height, z+half, visual.r, visual.g, visual.b, 1.0F},
        {x+half, visual.height, z-half, visual.r, visual.g, visual.b, 1.0F});
}

void appendRoadDetail(std::vector<Vertex>& vertices, float x, float z, std::size_t index) {
    const float offset = (index % 2U == 0U) ? -0.16F : 0.16F;
    appendBox(vertices, x + offset, z, 0.13F, 0.35F, 0.038F, 0.052F, 0.31F, 0.27F, 0.23F);
}

void appendBuildPlatform(std::vector<Vertex>& vertices, float x, float z) {
    appendBox(vertices, x, z, 0.37F, 0.37F, 0.08F, 0.16F, 0.38F, 0.40F, 0.40F);
    appendBox(vertices, x, z, 0.27F, 0.27F, 0.16F, 0.20F, 0.24F, 0.54F, 0.66F);
}

void appendCitadel(std::vector<Vertex>& vertices, float x, float z) {
    appendBox(vertices, x, z, 0.58F, 0.52F, 0.15F, 0.31F, 0.24F, 0.26F, 0.28F);
    appendBox(vertices, x, z, 0.34F, 0.34F, 0.30F, 1.08F, 0.49F, 0.50F, 0.49F);
    appendBox(vertices, x, z, 0.43F, 0.43F, 1.05F, 1.18F, 0.25F, 0.27F, 0.29F);
    appendBox(vertices, x - 0.43F, z, 0.13F, 0.18F, 0.28F, 0.92F, 0.43F, 0.44F, 0.43F);
    appendBox(vertices, x + 0.43F, z, 0.13F, 0.18F, 0.28F, 0.92F, 0.43F, 0.44F, 0.43F);
    appendBox(vertices, x, z + 0.37F, 0.06F, 0.04F, 0.70F, 1.40F, 0.78F, 0.58F, 0.16F);
    appendBox(vertices, x, z + 0.40F, 0.20F, 0.04F, 1.24F, 1.38F, 0.63F, 0.12F, 0.13F);
}

void appendGate(std::vector<Vertex>& vertices, float x, float z) {
    appendBox(vertices, x, z - 0.38F, 0.18F, 0.16F, 0.08F, 0.93F, 0.28F, 0.30F, 0.31F);
    appendBox(vertices, x, z + 0.38F, 0.18F, 0.16F, 0.08F, 0.74F, 0.28F, 0.30F, 0.31F);
    appendBox(vertices, x, z, 0.16F, 0.52F, 0.68F, 0.88F, 0.18F, 0.19F, 0.20F);
    appendBox(vertices, x + 0.02F, z, 0.06F, 0.24F, 0.16F, 0.75F, 0.12F, 0.76F, 0.57F);
    appendBox(vertices, x - 0.32F, z + 0.49F, 0.24F, 0.13F, 0.03F, 0.28F, 0.34F, 0.31F, 0.27F);
}

void appendProp(std::vector<Vertex>& vertices, const SceneProp& prop,
    float offsetX, float offsetZ) {
    const float x = offsetX + prop.gridX;
    const float z = offsetZ + prop.gridZ;
    const float s = prop.scale;
    const float angle = prop.rotationDegrees * kPi / 180.0F;
    switch (prop.type) {
        case ScenePropType::Pine:
            appendBox(vertices, x, z, 0.07F*s, 0.07F*s, 0.0F, 0.50F*s, 0.28F, 0.18F, 0.10F);
            appendBox(vertices, x, z, 0.30F*s, 0.30F*s, 0.40F*s, 0.72F*s, 0.18F, 0.32F, 0.19F);
            appendBox(vertices, x, z, 0.22F*s, 0.22F*s, 0.68F*s, 0.92F*s, 0.20F, 0.37F, 0.21F);
            break;
        case ScenePropType::Rock:
            appendRotatedBox(vertices, x, z, 0.30F*s, 0.22F*s, 0.0F, 0.31F*s, angle, 0.34F, 0.35F, 0.34F);
            break;
        case ScenePropType::Ruin:
            appendRotatedBox(vertices, x, z, 0.42F*s, 0.10F*s, 0.0F, 0.70F*s, angle, 0.40F, 0.39F, 0.36F);
            appendRotatedBox(vertices, x + 0.23F*s, z + 0.18F*s, 0.12F*s, 0.31F*s, 0.0F, 0.42F*s, angle, 0.34F, 0.33F, 0.31F);
            break;
        case ScenePropType::Banner:
            appendRotatedBox(vertices, x, z, 0.035F*s, 0.035F*s, 0.0F, 0.92F*s, angle, 0.28F, 0.18F, 0.09F);
            appendRotatedBox(vertices, x + 0.12F*s, z, 0.15F*s, 0.025F*s, 0.58F*s, 0.82F*s, angle, 0.66F, 0.13F, 0.15F);
            break;
        case ScenePropType::Barricade:
            appendRotatedBox(vertices, x, z, 0.45F*s, 0.07F*s, 0.10F*s, 0.27F*s, angle, 0.42F, 0.25F, 0.10F);
            appendRotatedBox(vertices, x - 0.23F*s, z, 0.06F*s, 0.08F*s, 0.0F, 0.39F*s, angle, 0.28F, 0.18F, 0.09F);
            appendRotatedBox(vertices, x + 0.23F*s, z, 0.06F*s, 0.08F*s, 0.0F, 0.39F*s, angle, 0.28F, 0.18F, 0.09F);
            break;
        case ScenePropType::Wagon:
            appendRotatedBox(vertices, x, z, 0.38F*s, 0.25F*s, 0.18F*s, 0.42F*s, angle, 0.39F, 0.22F, 0.09F);
            appendRotatedBox(vertices, x - 0.32F*s, z, 0.09F*s, 0.30F*s, 0.04F*s, 0.20F*s, angle, 0.18F, 0.16F, 0.13F);
            appendRotatedBox(vertices, x + 0.32F*s, z, 0.09F*s, 0.30F*s, 0.04F*s, 0.20F*s, angle, 0.18F, 0.16F, 0.13F);
            break;
        case ScenePropType::Cliff:
            appendRotatedBox(vertices, x, z, 0.65F*s, 0.48F*s, -0.16F*s, 0.38F*s, angle, 0.30F, 0.31F, 0.30F);
            break;
        case ScenePropType::Watchtower:
            appendRotatedBox(vertices, x, z, 0.24F*s, 0.24F*s, 0.0F, 0.70F*s, angle, 0.40F, 0.39F, 0.36F);
            appendRotatedBox(vertices, x, z, 0.34F*s, 0.34F*s, 0.68F*s, 0.82F*s, angle, 0.24F, 0.28F, 0.31F);
            break;
    }
}

void appendEnemy(std::vector<Vertex>& vertices) {
    appendBox(vertices, 0.0F, 0.0F, 0.23F, 0.15F, 0.12F, 0.66F, 0.58F, 0.13F, 0.34F);
    appendBox(vertices, 0.0F, 0.0F, 0.15F, 0.14F, 0.66F, 0.91F, 0.72F, 0.48F, 0.30F);
    appendBox(vertices, -0.28F, 0.0F, 0.07F, 0.22F, 0.30F, 0.74F, 0.45F, 0.48F, 0.52F);
}

void appendBallistaTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.42F,.42F,.05F,.22F,.25F,.31F,.38F);
    appendBox(v,0,0,.12F,.12F,.20F,.78F,.47F,.31F,.14F);
    appendBox(v,0,-.18F,.48F,.07F,.70F,.82F,.67F,.45F,.20F);
    appendBox(v,0,-.52F,.04F,.42F,.73F,.79F,.20F,.16F,.12F);
}
void appendMortarTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.45F,.45F,.05F,.25F,.29F,.33F,.38F);
    appendBox(v,0,0,.32F,.32F,.24F,.52F,.38F,.40F,.42F);
    appendRotatedBox(v,0,-.18F,.17F,.40F,.48F,.76F,-.18F,.18F,.20F,.22F);
    appendBox(v,0,-.57F,.13F,.18F,.58F,.82F,.10F,.11F,.12F);
}
void appendFrostTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.43F,.43F,.04F,.20F,.18F,.34F,.51F);
    appendBox(v,0,0,.25F,.25F,.18F,.85F,.23F,.67F,.83F);
    appendRotatedBox(v,0,0,.08F,.50F,.50F,.94F,.78F,.65F,.91F,1.0F);
    appendRotatedBox(v,0,0,.50F,.08F,.50F,.94F,.78F,.65F,.91F,1.0F);
}
void appendRocketTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.46F,.46F,.04F,.22F,.29F,.31F,.35F);
    appendBox(v,0,0,.29F,.29F,.20F,.66F,.43F,.18F,.13F);
    appendBox(v,-.17F,-.18F,.11F,.36F,.58F,.82F,.18F,.20F,.22F);
    appendBox(v,.17F,-.18F,.11F,.36F,.58F,.82F,.18F,.20F,.22F);
    appendBox(v,-.17F,-.58F,.08F,.12F,.61F,.79F,.80F,.24F,.12F);
    appendBox(v,.17F,-.58F,.08F,.12F,.61F,.79F,.80F,.24F,.12F);
}
void appendProjectile(std::vector<Vertex>& v) {
    appendBox(v,0,0,.06F,.06F,-.06F,.06F,.94F,.76F,.24F);
}
void appendRocketProjectile(std::vector<Vertex>& v) {
    appendBox(v,0,0,.07F,.25F,-.07F,.07F,.72F,.72F,.75F);
    appendBox(v,0,-.29F,.065F,.08F,-.065F,.065F,.91F,.22F,.13F);
    appendBox(v,0,.31F,.05F,.12F,-.05F,.05F,1.0F,.62F,.10F);
    appendBox(v,0,.52F,.035F,.18F,-.035F,.035F,1.0F,.26F,.05F);
}
void appendBenchmarkDecoration(std::vector<Vertex>& v, std::size_t i, float x, float z) {
    switch (i % 6U) {
        case 0: appendBox(v,x,z,.07F,.07F,0,.48F,.30F,.18F,.08F); appendBox(v,x,z,.30F,.30F,.42F,.86F,.16F,.38F,.17F); break;
        case 1: appendRotatedBox(v,x,z,.34F,.25F,0,.33F,.35F,.36F,.35F,.34F); break;
        case 2: appendBox(v,x,z,.38F,.10F,0,.70F,.43F,.41F,.36F); appendBox(v,x+.25F,z,.10F,.28F,0,.38F,.34F,.33F,.30F); break;
        case 3: appendBox(v,x,z,.035F,.035F,0,.92F,.28F,.18F,.09F); appendBox(v,x+.15F,z,.16F,.025F,.56F,.82F,.70F,.12F,.15F); break;
        case 4: appendRotatedBox(v,x,z,.45F,.07F,.10F,.27F,.55F,.42F,.25F,.10F); break;
        default: appendBox(v,x,z,.22F,.22F,0,.72F,.40F,.38F,.33F); appendBox(v,x,z,.35F,.35F,.70F,.84F,.24F,.28F,.31F); break;
    }
}

float worldX(const LevelData& level, std::size_t gridX) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(gridX);
}

float worldZ(const LevelData& level, std::size_t gridZ) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(gridZ);
}

}  // namespace

Renderer::~Renderer() { shutdown(); }

bool Renderer::initialize(const LevelData& level, std::size_t benchmarkDecorations) {
    shutdown();
    level_ = &level;
    benchmarkDecorations_ = benchmarkDecorations;
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
    vertices.reserve(PerformanceBudget::kMaximumLevelVertices + 512U);
    const float offsetX = -static_cast<float>(level.width) * 0.5F + 0.5F;
    const float offsetZ = -static_cast<float>(level.height) * 0.5F + 0.5F;
    const SceneArtLoadResult artResult = SceneArtLoader::load(SceneArtLoader::pathFor(level.id.c_str()).c_str());
    const LevelTheme theme = artResult.success && artResult.art.levelId == level.id
        ? artResult.art.theme : LevelTheme::Default;

    if (theme == LevelTheme::VhalPass) {
        appendBox(vertices, 0.0F, 0.0F, static_cast<float>(level.width) * 0.62F,
            static_cast<float>(level.height) * 0.62F, -0.22F, -0.03F, 0.20F, 0.27F, 0.21F);
    }

    std::size_t roadIndex = 0U;
    for (std::size_t z = 0; z < level.height; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            const TileType tile = level.tileAt(x, z);
            const float tileX = offsetX + static_cast<float>(x);
            const float tileZ = offsetZ + static_cast<float>(z);
            appendTile(vertices, tileX, tileZ, visualFor(tile, theme, x, z));
            if (theme == LevelTheme::VhalPass && tile == TileType::Road) appendRoadDetail(vertices, tileX, tileZ, roadIndex++);
            if (theme == LevelTheme::VhalPass && tile == TileType::BuildSpot) appendBuildPlatform(vertices, tileX, tileZ);
            if (tile == TileType::Base) appendCitadel(vertices, tileX, tileZ);
            else if (tile == TileType::Spawn) appendGate(vertices, tileX, tileZ);
        }
    }
    if (artResult.success && artResult.art.levelId == level.id) {
        for (std::size_t index = 0U; index < artResult.art.propCount; ++index) appendProp(vertices, artResult.art.props[index], offsetX, offsetZ);
    }
    if (level.id == "performance_stress") {
        for (std::size_t i=0; i<benchmarkDecorations_; ++i) {
            const std::size_t column=i % static_cast<std::size_t>(level.width);
            const std::size_t band=i / static_cast<std::size_t>(level.width);
            const float x=offsetX+static_cast<float>(column);
            const float z=band%2U==0U ? offsetZ+.45F : offsetZ+static_cast<float>(level.height-1U)-.45F;
            appendBenchmarkDecoration(vertices,i,x,z);
        }
    }
    if (vertices.size() > PerformanceBudget::kMaximumLevelVertices) return false;

    levelVertexCount_ = vertices.size();
    enemyVertexOffset_=vertices.size(); appendEnemy(vertices); enemyVertexCount_=vertices.size()-enemyVertexOffset_;
    towerVertexOffsets_[0]=vertices.size(); appendBallistaTower(vertices); towerVertexCounts_[0]=vertices.size()-towerVertexOffsets_[0];
    towerVertexOffsets_[1]=vertices.size(); appendMortarTower(vertices); towerVertexCounts_[1]=vertices.size()-towerVertexOffsets_[1];
    towerVertexOffsets_[2]=vertices.size(); appendFrostTower(vertices); towerVertexCounts_[2]=vertices.size()-towerVertexOffsets_[2];
    towerVertexOffsets_[3]=vertices.size(); appendRocketTower(vertices); towerVertexCounts_[3]=vertices.size()-towerVertexOffsets_[3];
    projectileVertexOffset_=vertices.size(); appendProjectile(vertices); projectileVertexCount_=vertices.size()-projectileVertexOffset_;
    rocketVertexOffset_=vertices.size(); appendRocketProjectile(vertices); rocketVertexCount_=vertices.size()-rocketVertexOffset_;

    const std::size_t bytes = vertices.size() * sizeof(Vertex);
    vertexBuffer_ = linearAlloc(bytes);
    if (vertexBuffer_ == nullptr) return false;
    std::memcpy(vertexBuffer_, vertices.data(), bytes);
    return true;
}

void Renderer::render(const Camera& camera, const Wave& wave,
    const BuildSystem& buildSystem, const TutorialFlow&,
    std::uint8_t maximum3DDepthPercent,
    UiRenderer& uiRenderer, const UiState& uiState) {
    lastStereoPlan_ = Stereo3D::plan(osGet3DSliderState(), maximum3DDepthPercent);
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
        const std::size_t type=static_cast<std::size_t>(tower.type());
        C3D_DrawArrays(GPU_TRIANGLES,static_cast<int>(towerVertexOffsets_[type]),static_cast<int>(towerVertexCounts_[type]));
    }
    if (level_ != nullptr) {
        C3D_Mtx cursor{}; camera.writeView(cursor);
        Mtx_Translate(&cursor, worldX(*level_, buildSystem.cursorX()),
            buildSystem.cursorCanBuild() ? -0.02F : -0.65F, worldZ(*level_, buildSystem.cursorZ()), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &cursor);
        const std::size_t selected=static_cast<std::size_t>(buildSystem.selectedTowerType());
        C3D_DrawArrays(GPU_TRIANGLES,static_cast<int>(towerVertexOffsets_[selected]),static_cast<int>(towerVertexCounts_[selected]));
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
        Mtx_Translate(&model,projectile.x(),projectile.y(),projectile.z(),true);
        const bool rocket=projectile.effect()==ProjectileEffect::GuidedRocket;
        if (rocket) Mtx_RotateY(&model,std::atan2(projectile.velocityX(),projectile.velocityZ()),true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,modelViewUniform_,&model);
        C3D_DrawArrays(GPU_TRIANGLES,static_cast<int>(rocket?rocketVertexOffset_:projectileVertexOffset_),static_cast<int>(rocket?rocketVertexCount_:projectileVertexCount_));
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
    levelVertexCount_=enemyVertexOffset_=enemyVertexCount_=0U;
    towerVertexOffsets_.fill(0U); towerVertexCounts_.fill(0U);
    projectileVertexOffset_=projectileVertexCount_=0U;
    rocketVertexOffset_=rocketVertexCount_=0U;
    lastStereoPlan_ = {};
    benchmarkDecorations_ = 0U;
}
