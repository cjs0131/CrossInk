#pragma once
#include <string>
#include <utility>

#include "activities/Activity.h"

class Bitmap;

class SleepActivity final : public Activity {
 public:
  explicit SleepActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, bool canSnapshotOverlayBackground,
                         std::string currentBookPath = {}, bool fromTimeout = false)
      : Activity("Sleep", renderer, mappedInput),
        canSnapshotOverlayBackground(canSnapshotOverlayBackground),
        currentBookPath(std::move(currentBookPath)),
        fromTimeout(fromTimeout) {}
  void onEnter() override;

  // Repaint the next sleep wallpaper (random, not recently shown) when eligible:
  // the cycleWallpaperOnTap setting is on, the active sleep mode draws from a
  // wallpaper folder, and there are at least two images. Returns true if a new
  // wallpaper was painted; false leaves the current frame untouched. Used by the
  // wake path when the power button is short-tapped during sleep.
  bool cycleWallpaper() const;

  // True if the given sleep-screen mode rotates through a wallpaper folder (and
  // so can be cycled). Static so the wake path can check before touching the
  // display. COVER_CUSTOM only cycles when the last sleep was not from the reader
  // (otherwise it shows the book cover, mirroring onEnter()'s routing).
  static bool modeSupportsCycling(uint8_t sleepScreenMode, bool lastSleepFromReader);

  // Count the wallpapers the custom sleep screen would choose from. A pinned
  // favorite counts as 1 (fixed, not cyclable). Mirrors the file filter used by
  // selectRandomSleepImage() so eligibility matches what actually renders.
  static size_t countValidSleepImages();

 private:
  void renderDefaultSleepScreen() const;
  void renderCustomSleepScreen() const;
  void renderCoverSleepScreen() const;
  void renderReadingStatsSleepScreen() const;
  void renderMinimalSleepScreen() const;
  void renderMinimalStatsSleepScreen() const;
  void renderDashboardSleepScreen() const;
  void renderBitmapSleepScreen(const Bitmap& bitmap) const;
  void renderLastScreenSleepScreen() const;
  void renderBlankSleepScreen() const;
  void renderOverlaySleepScreen() const;
  bool canSnapshotOverlayBackground = false;
  bool overlayBackgroundBufferStored = false;
  std::string currentBookPath;
  bool fromTimeout = false;
};
