#include <cstdlib>
#include <iostream>

#include "PerformanceBudget.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testAveragesAndWorstFrame() {
    PerformanceSampler sampler;
    sampler.record(16.0F, 8.0F, 1024U * 1024U);
    sampler.record(34.0F, 20.0F, 900U * 1024U);

    const PerformanceSnapshot& snapshot = sampler.snapshot();
    expect(snapshot.sampleCount == 2U, "sampler should count samples");
    expect(snapshot.averageFrameMilliseconds == 25.0F, "sampler should average frame time");
    expect(snapshot.worstFrameMilliseconds == 34.0F, "sampler should retain worst frame");
    expect(snapshot.lastRenderMilliseconds == 20.0F, "sampler should expose last render time");
    expect(!snapshot.frameBudgetExceeded(), "34ms should stay below warning threshold");
}

void testWarnings() {
    PerformanceSampler sampler;
    sampler.record(40.0F, 30.0F, 256U * 1024U);

    const PerformanceSnapshot& snapshot = sampler.snapshot();
    expect(snapshot.frameBudgetExceeded(), "slow frame should exceed warning budget");
    expect(snapshot.memoryReserveLow(), "low linear memory should be reported");
}

void testNegativeDurationsAreClamped() {
    PerformanceSampler sampler;
    sampler.record(-1.0F, -2.0F, 1024U * 1024U);

    const PerformanceSnapshot& snapshot = sampler.snapshot();
    expect(snapshot.lastFrameMilliseconds == 0.0F, "negative frame time should clamp to zero");
    expect(snapshot.lastRenderMilliseconds == 0.0F, "negative render time should clamp to zero");
}

}  // namespace

int main() {
    testAveragesAndWorstFrame();
    testWarnings();
    testNegativeDurationsAreClamped();
    std::cout << "Performance budget tests passed\n";
    return 0;
}
