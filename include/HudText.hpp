#pragma once

#include "BuildFeedback.hpp"
#include "TutorialFlow.hpp"

[[nodiscard]] const char* tutorialInstruction(TutorialPhase phase);
[[nodiscard]] const char* buildAttemptMessage(BuildAttemptResult result);
[[nodiscard]] const char* audioStatusMessage(bool available);

#ifdef __3DS__
#include <cstdarg>

namespace std {
int citadelConsolePrintf(const char* format, ...);
}

// main.cpp uses std::printf for the libctru text console. On an original
// Nintendo 3DS XL the useful safe area is narrower than the nominal 40
// columns, so route those writes through a bounded physical-screen adapter.
#define printf citadelConsolePrintf
#endif
