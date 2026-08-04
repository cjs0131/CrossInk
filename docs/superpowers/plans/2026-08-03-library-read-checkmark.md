# Read-book Checkmark in Library — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Show a checkmark instead of the book icon in the file browser for books that have been marked finished.

**Architecture:** Add a checkmark bitmap + `UIIcon::BookCompleted` value. Branch the shared `rowIcon` hook in `FileBrowserActivity` so a finished book returns the checkmark. Read finished-status lazily (once per book, on first draw) into a small per-entry byte cache to avoid repeated SD reads while scrolling; the cache clears on every folder (re)load, which the "mark finished" flow already triggers. Feature applies to the in-memory `files` path only; folders large enough to use index mode (>200 entries) keep the plain book icon.

**Tech Stack:** C++ (ESP-IDF / Arduino-ESP32), PlatformIO, 1-bit icon bitmaps drawn via `GfxRenderer::drawIcon`.

## Global Constraints

- ESP32-C3, ~380 KB RAM, no PSRAM. Stability over features. (CLAUDE.md)
- Icon bitmap format: MSB-first, `ceil(w/8)` bytes per row, **0 bit = black pixel, 1 bit = white/transparent** (confirmed in `GfxRenderer::drawIcon`, `lib/GfxRenderer/GfxRenderer.cpp:1513`, `blackPixels = ~bitmap[...]`).
- New `UIIcon` enum values are appended at the end of the enum to keep existing values stable.
- Do not edit generated files. Run `clang-format -i` on touched C++ files before committing.
- User-facing strings use `tr(STR_*)`; this feature adds no new strings.
- Add a `CHANGELOG.md` entry (Added section).
- Do not commit `.pio/`, `*.generated.h`, `compile_commands.json`, or `platformio.local.ini`.

---

### Task 1: Add the checkmark icon (bitmap + enum + theme mapping)

**Files:**
- Create: `src/components/icons/check24.h`
- Create: `src/components/icons/check.h`
- Modify: `src/components/themes/BaseTheme.h:118-132` (append enum value)
- Modify: `src/components/themes/lyra/LyraTheme.cpp:19-28` (includes) and `:72-113` (`iconForName`)

**Interfaces:**
- Produces: `UIIcon::BookCompleted` (enum value in `BaseTheme.h`), rendered as `Check24Icon` at size 24 and `CheckIcon` at size 32 via `LyraTheme::iconForName` (inherited by `MinimalTheme`).

- [ ] **Step 1: Create the 24×24 checkmark bitmap**

Create `src/components/icons/check24.h` with exactly:

```cpp
#pragma once
#include <cstdint>

// size: 24x24
static const uint8_t Check24Icon[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF,
    0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0x0F, 0xFF, 0xFE, 0x1F, 0xFF, 0xFC, 0x1F, 0xFF, 0xFC, 0x3F,
    0xF9, 0xF8, 0x7F, 0xF0, 0xF0, 0xFF, 0xF0, 0x60, 0xFF, 0xF8, 0x21, 0xFF, 0xFC, 0x03, 0xFF, 0xFE, 0x07, 0xFF,
    0xFF, 0x07, 0xFF, 0xFF, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

- [ ] **Step 2: Create the 32×32 checkmark bitmap**

Create `src/components/icons/check.h` with exactly:

```cpp
#pragma once
#include <cstdint>

// size: 32x32
static const uint8_t CheckIcon[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFE, 0x0F, 0xFF, 0xFF, 0xFC, 0x0F, 0xFF, 0xFF, 0xFC, 0x0F, 0xFF, 0xFF, 0xF8, 0x1F, 0xFF, 0xFF, 0xF0, 0x7F,
    0xFF, 0xFF, 0xE0, 0x7F, 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xFF, 0xC1, 0xFF, 0xFC, 0x7F, 0x83, 0xFF, 0xF8, 0x3F, 0x03, 0xFF, 0xF8, 0x1F, 0x07, 0xFF,
    0xF8, 0x0E, 0x0F, 0xFF, 0xFC, 0x04, 0x1F, 0xFF, 0xFF, 0x00, 0x1F, 0xFF, 0xFF, 0x80, 0x3F, 0xFF, 0xFF, 0x80, 0x7F, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF,
    0xFF, 0xE0, 0xFF, 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

- [ ] **Step 3: Append the enum value**

In `src/components/themes/BaseTheme.h`, add `BookCompleted` at the end of the `UIIcon` enum (line ~131, after `Chart`):

```cpp
enum UIIcon {
  Folder,
  Text,
  Image,
  Book,
  BookmarkIcon,
  File,
  Recent,
  Settings,
  Transfer,
  Library,
  Wifi,
  Hotspot,
  Chart,
  BookCompleted
};
```

- [ ] **Step 4: Include the new headers in LyraTheme.cpp**

In `src/components/themes/lyra/LyraTheme.cpp`, add these two includes alongside the existing icon includes (after line 20, `components/icons/book24.h`):

```cpp
#include "components/icons/check.h"
#include "components/icons/check24.h"
```

- [ ] **Step 5: Map the enum value to bitmaps in `iconForName`**

In `src/components/themes/lyra/LyraTheme.cpp`, add a case in each size branch of `iconForName`.

In the `size == 24` switch (after the `UIIcon::Book` case, ~line 82):

```cpp
      case UIIcon::BookCompleted:
        return Check24Icon;
```

In the `size == 32` switch (after the `UIIcon::Book` case, ~line 93):

```cpp
      case UIIcon::BookCompleted:
        return CheckIcon;
```

- [ ] **Step 6: Format and build**

Run:
```bash
find src lib include test -name "*.cpp" -o -name "*.h" | xargs clang-format -i
pio run -e simulator
```
Expected: build succeeds. The new icon is not yet used anywhere, so no visible change — this task just makes it available.

- [ ] **Step 7: Commit**

```bash
git add src/components/icons/check.h src/components/icons/check24.h \
  src/components/themes/BaseTheme.h src/components/themes/lyra/LyraTheme.cpp
git commit -m "feat: add checkmark icon for completed books"
```

---

### Task 2: Show the checkmark for finished books in the file browser

**Files:**
- Modify: `src/activities/home/FileBrowserActivity.h:54` (add cache member)
- Modify: `src/activities/home/FileBrowserActivity.cpp:227` (`loadFiles` — clear cache), `:349` (`onExit` — clear cache), `:929-932` (`rowIcon` lambda)
- Modify: `CHANGELOG.md`

**Interfaces:**
- Consumes: `UIIcon::BookCompleted` from Task 1; `BookActions::isBookCompleted(const std::string&)` (`src/activities/home/BookActions.h:23`, returns `bool`, already reads the book's stats file and returns `false` for books with no stats); `UITheme::getFileIcon(const std::string&)` → `UIIcon`.
- Produces: no new public interface.

- [ ] **Step 1: Add the completed-status cache member**

In `src/activities/home/FileBrowserActivity.h`, in the `// Files state` block (after line 54, `std::vector<std::string> files;`), add:

```cpp
  // Lazy per-entry finished-status cache, parallel to `files`.
  // 0 = unknown (not yet read), 1 = not completed, 2 = completed.
  // Cleared on every loadFiles(); index mode (usingIndex) does not use it.
  std::vector<uint8_t> fileCompleted;
```

- [ ] **Step 2: Clear the cache whenever the list reloads**

In `src/activities/home/FileBrowserActivity.cpp`, at the top of `loadFiles()` (line 227, alongside the existing reset lines), add:

```cpp
void FileBrowserActivity::loadFiles() {
  usingIndex = false;
  clearIndexNameCache();
  fileCompleted.clear();
  fileListMemoryLimited = false;
```

Because `FileBrowserAction::ToggleCompleted` already calls `loadFiles()` after toggling (`FileBrowserActivity.cpp:634`), marking a book finished/unfinished clears the cache, so the checkmark reflects the new status on the next draw. No separate per-slot invalidation is needed.

- [ ] **Step 3: Clear the cache on exit**

In `src/activities/home/FileBrowserActivity.cpp`, in `onExit()` (line 349, next to `files.clear();`), add:

```cpp
  files.clear();
  fileCompleted.clear();
```

- [ ] **Step 4: Branch the `rowIcon` hook to return the checkmark**

In `src/activities/home/FileBrowserActivity.cpp`, replace the `rowIcon` lambda (lines 929-932):

```cpp
    const auto rowIcon = [this](int index) {
      const std::string entry = entryNameAt(index);
      return UITheme::getFileIcon(entry);
    };
```

with:

```cpp
    const auto rowIcon = [this](int index) {
      const std::string entry = entryNameAt(index);
      UIIcon icon = UITheme::getFileIcon(entry);
      // Finished books show a checkmark instead of the book icon. Read status
      // lazily (once per book) into fileCompleted to avoid an SD read per row
      // on every redraw. Index mode has no in-memory files vector, so it is
      // skipped and keeps the plain book icon.
      if (icon == UIIcon::Book && !usingIndex && index >= 0 &&
          static_cast<size_t>(index) < files.size()) {
        if (fileCompleted.size() != files.size()) {
          fileCompleted.assign(files.size(), 0);
        }
        uint8_t& slot = fileCompleted[static_cast<size_t>(index)];
        if (slot == 0) {
          const std::string fullPath = buildFullPath(basepath, entry);
          slot = BookActions::isBookCompleted(fullPath) ? 2 : 1;
        }
        if (slot == 2) {
          icon = UIIcon::BookCompleted;
        }
      }
      return icon;
    };
```

- [ ] **Step 5: Add the changelog entry**

In `CHANGELOG.md`, under the current unreleased/top version's `### Added` section (create the section if absent), add:

```markdown
- Finished books now show a checkmark instead of the book icon in the library browser.
```

- [ ] **Step 6: Format and build**

Run:
```bash
find src lib include test -name "*.cpp" -o -name "*.h" | xargs clang-format -i
pio run -e simulator
```
Expected: build succeeds.

- [ ] **Step 7: Verify in the simulator**

Run the simulator (`pio run -e simulator` then launch the built simulator binary, or the repo's usual simulator run command). In a folder containing at least one book:

1. Mark a book **Finished** via its context menu. Its left icon becomes a checkmark without leaving the folder.
2. Mark the same book **Unfinished**. The book icon returns.
3. Scroll the list up and down repeatedly — no new lag (cached reads).
4. Folders and non-book files are unchanged.

Expected: checkmark appears/disappears with finished status; no visual regressions. If the simulator cannot exercise the context menu, verify on hardware instead (see Step 8).

- [ ] **Step 8: Commit**

```bash
git add src/activities/home/FileBrowserActivity.h src/activities/home/FileBrowserActivity.cpp CHANGELOG.md
git commit -m "feat: show checkmark for finished books in library"
```

**Hardware verification (real device):** Copy the firmware to an Xteink X4. Open the book browser, mark a book Finished (context menu or the configured Mark Finished button) — its icon becomes a checkmark immediately. Mark it Unfinished — the book icon returns. Scroll a folder of books; confirm no added lag. A folder with 200+ entries shows plain book icons (index mode, expected). No cache reset needed for this feature.

---

## Notes on testing approach

The core logic is a three-state lazy-read cache (`unknown → read once → reuse`) welded to `BookActions::isBookCompleted`, which itself reads the SD card. A host unit test would require mocking `Storage` and `BookReadingStats`, which the current `test/` suite does not do for activities. The logic is small and the risk is visual, so verification is the simulator plus on-device check above rather than a new host test. If a host test is wanted later, the state machine could be extracted into a free helper taking a `bool isCompleted(index)` callback and tested in isolation — but that indirection is not worth adding to a 10-line hot path now.
