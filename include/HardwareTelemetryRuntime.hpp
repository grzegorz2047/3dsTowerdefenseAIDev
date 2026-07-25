#pragma once

#include "HardwareTelemetry.hpp"

void hardwareTelemetryRecordGameCpu(float milliseconds);
const HardwareTelemetrySnapshot& hardwareTelemetrySnapshot();
