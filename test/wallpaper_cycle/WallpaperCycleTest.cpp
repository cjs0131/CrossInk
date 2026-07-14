#include <gtest/gtest.h>

#include "util/SleepCycleUtil.h"

// The pure decision behind "short power-tap cycles the sleep wallpaper":
// cycle only when the feature is enabled, the sleep-screen mode actually draws
// from a wallpaper folder, and there are at least two images to move between.

TEST(WallpaperCycle, CyclesWhenEnabledSupportedAndEnoughImages) {
  EXPECT_TRUE(sleepcycle::shouldCycleOnTap(/*enabled=*/true, /*sleepModeSupportsCycling=*/true,
                                           /*validImageCount=*/3));
}

TEST(WallpaperCycle, DoesNotCycleWhenDisabled) { EXPECT_FALSE(sleepcycle::shouldCycleOnTap(false, true, 5)); }

TEST(WallpaperCycle, DoesNotCycleWhenModeUnsupported) {
  // e.g. Dark/Light/Blank/Cover sleep screens have no folder to rotate through.
  EXPECT_FALSE(sleepcycle::shouldCycleOnTap(true, false, 5));
}

TEST(WallpaperCycle, DoesNotCycleWithZeroImages) { EXPECT_FALSE(sleepcycle::shouldCycleOnTap(true, true, 0)); }

TEST(WallpaperCycle, DoesNotCycleWithASingleImage) {
  // One wallpaper: nothing to cycle to, keep the current frame.
  EXPECT_FALSE(sleepcycle::shouldCycleOnTap(true, true, 1));
}

TEST(WallpaperCycle, CyclesAtTheTwoImageBoundary) { EXPECT_TRUE(sleepcycle::shouldCycleOnTap(true, true, 2)); }
