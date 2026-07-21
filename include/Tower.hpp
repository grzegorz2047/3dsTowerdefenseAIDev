#pragma once

#include "Level.hpp"
#include "Wave.hpp"

class Tower {
public:
    explicit Tower(const LevelData& level);

    void update(float deltaSeconds, Wave& wave);
    void reset();

    [[nodiscard]] float x() const;
    [[nodiscard]] float z() const;
    [[nodiscard]] bool valid() const;
    [[nodiscard]] int shotsFired() const;

private:
    float x_ = 0.0F;
    float z_ = 0.0F;
    float cooldown_ = 0.0F;
    int shotsFired_ = 0;
    bool valid_ = false;
};
