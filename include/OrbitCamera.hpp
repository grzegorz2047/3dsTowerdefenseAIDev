#pragma once

class OrbitCamera {
public:
    static constexpr int kCircleDeadzone = 20;
    static constexpr float kDefaultYaw = 0.0F;
    static constexpr float kDefaultPitch = 0.72F;
    static constexpr float kDefaultDistance = 17.0F;
    static constexpr float kMinimumDistance = 10.0F;
    static constexpr float kMaximumDistance = 24.0F;

    void update(int circleX, int circleY, float deltaSeconds);
    void rotateQuarterTurn(int direction);

    [[nodiscard]] float yaw() const;
    [[nodiscard]] float pitch() const;
    [[nodiscard]] float distance() const;

private:
    [[nodiscard]] static float normalizedAxis(int value);
    static float wrapRadians(float value);

    float yaw_ = kDefaultYaw;
    float pitch_ = kDefaultPitch;
    float distance_ = kDefaultDistance;
};
