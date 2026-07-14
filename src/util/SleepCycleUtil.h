#pragma once
#include <cstddef>

// Pure decision logic for the "short power-tap cycles the sleep wallpaper"
// feature. Kept free of Arduino/HAL includes so it is unit-testable on the host.
// The caller (main.cpp, at wake time) supplies the three inputs; this header
// owns only the rule that combines them.
namespace sleepcycle {

// Fewer than this many wallpapers means there is nothing to cycle to, so a tap
// should leave the current frame untouched.
inline constexpr size_t MIN_IMAGES_TO_CYCLE = 2;

// Should a short power-button tap during sleep repaint a new wallpaper?
// - enabled:                  the cycleWallpaperOnTap user setting is on
// - sleepModeSupportsCycling: the active sleep-screen mode draws from a
//                             wallpaper folder (Custom, or Cover-custom when not
//                             resolving to the book cover)
// - validImageCount:          number of usable BMPs in the sleep folder
inline bool shouldCycleOnTap(bool enabled, bool sleepModeSupportsCycling, size_t validImageCount) {
  return enabled && sleepModeSupportsCycling && validImageCount >= MIN_IMAGES_TO_CYCLE;
}

}  // namespace sleepcycle
