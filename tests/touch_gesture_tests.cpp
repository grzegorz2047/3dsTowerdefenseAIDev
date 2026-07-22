#include <cstdlib>
#include <iostream>

#include "TouchGesture.hpp"

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testCleanTap() {
    TouchGesture gesture;
    expect(!gesture.update(true, 40, 60).tapped, "press should not trigger before release");
    expect(gesture.tracking(), "press should start tracking");
    const TouchGestureResult result = gesture.update(false, 40, 60);
    expect(result.tapped, "stationary press and release should be a tap");
    expect(result.startX == 40 && result.startY == 60, "tap should preserve press coordinates");
    expect(result.endX == 40 && result.endY == 60, "tap should preserve release coordinates");
    expect(!gesture.tracking(), "release should finish tracking");
}

void testSmallJitterRemainsTap() {
    TouchGesture gesture;
    (void)gesture.update(true, 100, 100);
    (void)gesture.update(true, 105, 104);
    const TouchGestureResult result = gesture.update(false, 105, 104);
    expect(result.tapped, "small stylus jitter should remain a tap");
}

void testDragCancelsTap() {
    TouchGesture gesture;
    (void)gesture.update(true, 100, 100);
    (void)gesture.update(true, 109, 100);
    expect(gesture.cancelledByMovement(), "movement beyond threshold should cancel gesture");
    const TouchGestureResult result = gesture.update(false, 109, 100);
    expect(!result.tapped, "cancelled drag must not activate a button");
    expect(!gesture.cancelledByMovement(), "release should reset cancellation state");
}

void testDistanceUsesBothAxes() {
    TouchGesture gesture;
    (void)gesture.update(true, 10, 10);
    (void)gesture.update(true, 16, 16);
    const TouchGestureResult result = gesture.update(false, 16, 16);
    expect(!result.tapped, "diagonal movement beyond circular threshold should cancel tap");
}

void testReleaseWithoutPressDoesNothing() {
    TouchGesture gesture;
    const TouchGestureResult result = gesture.update(false, 12, 14);
    expect(!result.tapped, "release without tracked press must do nothing");
}

}  // namespace

int main() {
    testCleanTap();
    testSmallJitterRemainsTap();
    testDragCancelsTap();
    testDistanceUsesBothAxes();
    testReleaseWithoutPressDoesNothing();
    std::cout << "Touch gesture tests passed\n";
    return 0;
}