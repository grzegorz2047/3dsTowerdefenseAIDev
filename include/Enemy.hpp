#pragma once

#include <cstddef>

#include "Level.hpp"

class Enemy {
public:
    explicit Enemy(const LevelData& level);

    void update(float deltaSeconds);
    void reset();

    [[nodiscard]] float x() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] bool reachedBase() const;
    [[nodiscard]] float pathProgress() const;

private:
    const LevelData* level_ = nullptr;
    std::size_t segmentIndex_ = 0;
    float segmentProgress_ = 0.0F;
    float x_ = 0.0F;
    float z_ = 0.0F;
    bool reachedBase_ = false;
};
