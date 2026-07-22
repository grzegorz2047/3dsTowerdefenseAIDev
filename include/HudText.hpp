#pragma once

#include "BuildFeedback.hpp"
#include "TutorialFlow.hpp"

[[nodiscard]] const char* tutorialInstruction(TutorialPhase phase);
[[nodiscard]] const char* buildAttemptMessage(BuildAttemptResult result);
