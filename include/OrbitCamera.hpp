#pragma once

class OrbitCamera {
public:
    static constexpr int kCircleDeadzone = 24;
    static constexpr int kCircleMaximum = 156;
    static constexpr float kDefaultYaw = 0.0F;
    static constexpr float kDefaultPitch = 0.50F;
    static constexpr float kDefaultDistance = 13.25F;
    static constexpr float kMinimumDistance = 9.0F;
    static constexpr float kMaximumDistance = 21.0F;
    static constexpr float kYawRadiansPerSecond = 1.8F;
    static constexpr float kZoomUnitsPerSecond = 7.0F;

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
