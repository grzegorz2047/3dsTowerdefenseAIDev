#pragma once

#include <cstddef>

#include "HardwareTelemetry.hpp"

void hardwareTelemetryInitializeDeviceProfile();
void hardwareTelemetryBeginFrame(float gameCpuMilliseconds);
void hardwareTelemetryFinishFrame(std::size_t freeLinearMemoryBytes, std::uint8_t eyeCount);
const HardwareTelemetrySnapshot& hardwareTelemetrySnapshot();
