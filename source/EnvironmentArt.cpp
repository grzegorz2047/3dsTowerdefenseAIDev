#include "EnvironmentArt.hpp"

#include <cstddef>

namespace {

struct Color {
    float r;
    float g;
    float b;
};

void appendQuad(
    std::vector<MeshVertex>& vertices,
    const MeshVertex& a,
    const MeshVertex& b,
    const MeshVertex& c,
    const MeshVertex& d) {
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
    vertices.push_back(c);
    vertices.push_back(d);
    vertices.push_back(a);
}

void appendTop(
    std::vector<MeshVertex>& vertices,
    float centerX,
    float centerZ,
    float halfX,
    float halfZ,
    float y,
    Color color) {
    appendQuad(
        vertices,
        {centerX - halfX, y, centerZ - halfZ, color.r, color.g, color.b, 1.0F},
        {centerX - halfX, y, centerZ + halfZ, color.r, color.g, color.b, 1.0F},
        {centerX + halfX, y, centerZ + halfZ, color.r, color.g, color.b, 1.0F},
        {centerX + halfX, y, centerZ - halfZ, color.r, color.g, color.b, 1.0F});
}

void appendBox(
    std::vector<MeshVertex>& vertices,
    float centerX,
    float centerZ,
    float halfX,
    float halfZ,
    float y0,
    float y1,
    Color color) {
    const float x0 = centerX - halfX;
    const float x1 = centerX + halfX;
    const float z0 = centerZ - halfZ;
    const float z1 = centerZ + halfZ;
    appendQuad(vertices,
        {x0, y0, z1, color.r, color.g, color.b, 1.0F}, {x1, y0, z1, color.r, color.g, color.b, 1.0F},
        {x1, y1, z1, color.r, color.g, color.b, 1.0F}, {x0, y1, z1, color.r, color.g, color.b, 1.0F});
    appendQuad(vertices,
        {x1, y0, z0, color.r, color.g, color.b, 1.0F}, {x0, y0, z0, color.r, color.g, color.b, 1.0F},
        {x0, y1, z0, color.r, color.g, color.b, 1.0F}, {x1, y1, z0, color.r, color.g, color.b, 1.0F});
    appendQuad(vertices,
        {x1, y0, z1, color.r, color.g, color.b, 1.0F}, {x1, y0, z0, color.r, color.g, color.b, 1.0F},
        {x1, y1, z0, color.r, color.g, color.b, 1.0F}, {x1, y1, z1, color.r, color.g, color.b, 1.0F});
    appendQuad(vertices,
        {x0, y0, z0, color.r, color.g, color.b, 1.0F}, {x0, y0, z1, color.r, color.g, color.b, 1.0F},
        {x0, y1, z1, color.r, color.g, color.b, 1.0F}, {x0, y1, z0, color.r, color.g, color.b, 1.0F});
    appendTop(vertices, centerX, centerZ, halfX, halfZ, y1, color);
}

float worldX(const LevelData& level, std::size_t x) {
    return -static_cast<float>(level.width) * 0.5F + 0.5F + static_cast<float>(x);
}

float worldZ(const LevelData& level, std::size_t z) {
    return -static_cast<float>(level.height) * 0.5F + 0.5F + static_cast<float>(z);
}

void appendGround(std::vector<MeshVertex>& vertices, float x, float z) {
    appendTop(vertices, x, z, 0.46F, 0.46F, 0.0F, {0.18F, 0.44F, 0.20F});
}

void appendRoad(std::vector<MeshVertex>& vertices, float x, float z) {
    appendTop(vertices, x, z, 0.47F, 0.47F, 0.035F, {0.45F, 0.30F, 0.16F});
    appendBox(vertices, x - 0.39F, z, 0.045F, 0.43F, 0.04F, 0.09F, {0.60F, 0.48F, 0.31F});
    appendBox(vertices, x + 0.39F, z, 0.045F, 0.43F, 0.04F, 0.09F, {0.60F, 0.48F, 0.31F});
}

void appendBuildSpot(std::vector<MeshVertex>& vertices, float x, float z) {
    appendGround(vertices, x, z);
    appendBox(vertices, x, z, 0.39F, 0.39F, 0.02F, 0.11F, {0.25F, 0.39F, 0.54F});
    appendBox(vertices, x, z, 0.25F, 0.25F, 0.11F, 0.145F, {0.40F, 0.62F, 0.82F});
}

void appendBlocked(std::vector<MeshVertex>& vertices, float x, float z, std::size_t seed) {
    appendGround(vertices, x, z);
    const float offset = (seed % 2U == 0U) ? -0.12F : 0.12F;
    appendBox(vertices, x + offset, z - 0.08F, 0.25F, 0.28F, 0.01F, 0.37F, {0.27F, 0.29F, 0.31F});
    appendBox(vertices, x - offset, z + 0.18F, 0.18F, 0.16F, 0.02F, 0.53F, {0.36F, 0.37F, 0.39F});
}

void appendSpawn(std::vector<MeshVertex>& vertices, float x, float z) {
    appendTop(vertices, x, z, 0.47F, 0.47F, 0.04F, {0.19F, 0.33F, 0.22F});
    appendBox(vertices, x - 0.34F, z, 0.11F, 0.36F, 0.04F, 0.85F, {0.31F, 0.27F, 0.34F});
    appendBox(vertices, x + 0.34F, z, 0.11F, 0.36F, 0.04F, 0.85F, {0.31F, 0.27F, 0.34F});
    appendBox(vertices, x, z + 0.28F, 0.28F, 0.09F, 0.62F, 0.88F, {0.38F, 0.30F, 0.46F});
    appendBox(vertices, x, z, 0.18F, 0.04F, 0.12F, 0.66F, {0.58F, 0.19F, 0.73F});
}

void appendBase(std::vector<MeshVertex>& vertices, float x, float z) {
    appendTop(vertices, x, z, 0.48F, 0.48F, 0.04F, {0.34F, 0.31F, 0.28F});
    appendBox(vertices, x, z, 0.40F, 0.40F, 0.04F, 0.25F, {0.36F, 0.40F, 0.46F});
    appendBox(vertices, x, z, 0.29F, 0.29F, 0.25F, 0.88F, {0.49F, 0.54F, 0.61F});
    appendBox(vertices, x - 0.30F, z - 0.30F, 0.10F, 0.10F, 0.82F, 1.14F, {0.32F, 0.37F, 0.44F});
    appendBox(vertices, x + 0.30F, z - 0.30F, 0.10F, 0.10F, 0.82F, 1.14F, {0.32F, 0.37F, 0.44F});
    appendBox(vertices, x - 0.30F, z + 0.30F, 0.10F, 0.10F, 0.82F, 1.14F, {0.32F, 0.37F, 0.44F});
    appendBox(vertices, x + 0.30F, z + 0.30F, 0.10F, 0.10F, 0.82F, 1.14F, {0.32F, 0.37F, 0.44F});
    appendBox(vertices, x, z - 0.30F, 0.11F, 0.04F, 0.25F, 0.63F, {0.20F, 0.12F, 0.07F});
}

}  // namespace

void appendTutorialEnvironment(std::vector<MeshVertex>& vertices, const LevelData& level) {
    for (std::size_t z = 0; z < level.height; ++z) {
        for (std::size_t x = 0; x < level.width; ++x) {
            const float wx = worldX(level, x);
            const float wz = worldZ(level, z);
            switch (level.tileAt(x, z)) {
                case TileType::Road: appendRoad(vertices, wx, wz); break;
                case TileType::BuildSpot: appendBuildSpot(vertices, wx, wz); break;
                case TileType::Blocked: appendBlocked(vertices, wx, wz, x + z * level.width); break;
                case TileType::Spawn: appendSpawn(vertices, wx, wz); break;
                case TileType::Base: appendBase(vertices, wx, wz); break;
                case TileType::Ground:
                default: appendGround(vertices, wx, wz); break;
            }
        }
    }
}
