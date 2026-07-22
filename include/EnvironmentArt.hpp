#pragma once

#include <vector>

#include "Level.hpp"

struct MeshVertex {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
};

void appendTutorialEnvironment(std::vector<MeshVertex>& vertices, const LevelData& level);
