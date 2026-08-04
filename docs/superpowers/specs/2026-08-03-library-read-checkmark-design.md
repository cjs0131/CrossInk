# Read-book checkmark in the library — design

## Goal

In the file browser, a book that has been marked finished shows a checkmark
where its book icon normally sits, so finished books are recognizable at a
glance without opening the context menu.

## Background

The read/finished state already exists. "Mark Finished" (context menu, or the
power-button `MARK_FINISHED` action) sets `BookReadingStats::isCompleted` and
saves it to the book's `stats_v5.bin`. `BookActions::isBookCompleted(fullPath)`
reads that flag back (`src/activities/home/BookActions.cpp:141`). It returns
`false` gracefully for books with no stats file.

Every row's left icon is chosen in one shared place: the `rowIcon` lambda in
`FileBrowserActivity::render` (`src/activities/home/FileBrowserActivity.cpp:929`),
which today always returns `UITheme::getFileIcon(entry)`. That lambda feeds both
list renderers (compact `MinimalTheme` list and default `GUI.drawList`), so a
single change covers both display styles.

Icons are 1-bit bitmaps drawn via `renderer.drawIcon`, selected by a `UIIcon`
enum value (`src/components/themes/BaseTheme.h:118`) mapped to a bitmap in
`LyraTheme::iconForName` (`src/components/themes/lyra/LyraTheme.cpp:72`). The
book icon is `Book24Icon` (24×24, compact list) / `BookIcon` (32×32, default
list). There is no checkmark icon anywhere in the codebase today.

## Entry model and the two paths

File entries live in one of two forms:

- **Vector path** (`usingIndex == false`): names in `std::vector<std::string>
  files`. Used for folders with ≤ `INDEX_THRESHOLD` (200) entries.
- **Index path** (`usingIndex == true`): folders over 200 entries stream row
  names from a `FileIndex` on demand to save RAM; there is no in-memory `files`
  vector to attach state to.

The checkmark applies to the vector path only. In index mode a book keeps the
plain book icon. This avoids doing an SD stats-read per visible row on every
scroll in a very large folder, which is exactly the case index mode exists to
keep cheap. Revisitable later if wanted.

## Design

### 1. Checkmark bitmap + icon enum value

- Add `src/components/icons/check24.h` (`Check24Icon`, 24×24) and
  `src/components/icons/check.h` (`CheckIcon`, 32×32), in the same 1-bit format
  as `book24.h` / `book.h`. A plain checkmark glyph, no book outline.
- Add `UIIcon::BookCompleted` to the enum in `BaseTheme.h`.
- Add cases in `LyraTheme::iconForName` mapping `BookCompleted` to `Check24Icon`
  at size 24 and `CheckIcon` at size 32. `MinimalTheme` inherits this mapping.

### 2. The branch in the icon hook

In the `rowIcon` lambda, after computing the normal icon:

- If the normal icon is `UIIcon::Book` (i.e. the entry is a book) and the
  vector path is active, consult the completed cache (below). If the book is
  completed, return `UIIcon::BookCompleted`; otherwise return `UIIcon::Book`.
- Non-book rows and index-mode rows are unaffected.

### 3. Lazy completed cache

Repeated SD reads in the redraw path are the cost to avoid: `rowIcon` runs for
every visible book, and the list redraws on every highlight move. So the
completed status is read at most once per book and remembered.

- Add `std::vector<uint8_t> fileCompleted` parallel to `files` (0 = unknown,
  1 = not completed, 2 = completed). One byte per entry; a 200-book folder
  costs 200 bytes.
- Sized/reset to `files.size()` filled with 0 wherever `files` is (re)built
  (`loadFiles` and the vector-load path).
- In `rowIcon`, for a book row whose cache slot is 0 (unknown), call
  `BookActions::isBookCompleted(fullPath)` once, store 1 or 2, and use it. Slots
  1/2 are reused without touching the SD card.

Net SD cost: each visible book is read once; scrolling reuses cached values.
Books scrolled past are read the first time they become visible, not up front.

### 4. Cache invalidation on toggle

Marking a book finished/unfinished must update its checkmark immediately.

- `FileBrowserAction::ToggleCompleted`
  (`src/activities/home/FileBrowserActivity.cpp:629`) and the power-button
  `MARK_FINISHED` path both already trigger a redraw. On toggle, reset the
  affected entry's `fileCompleted` slot to 0 (unknown) so the next draw
  re-reads the now-current status. Resetting a single slot is enough; resetting
  the whole vector is an acceptable simpler fallback.

## Error handling

- `isBookCompleted` already returns `false` for a missing/short stats file, so
  a book that was never opened simply shows the book icon. No new failure paths.
- If the checkmark bitmap were somehow missing, `iconForName` falls through to
  its existing default; no crash.

## Testing / verification

Host test suite (`test/`, CMake/GoogleTest) can cover the cache logic if it is
factored to be host-testable (e.g. the unknown → read-once → reuse transition),
without hardware.

On device:

1. Mark a book finished from the context menu. Its left icon becomes a
   checkmark immediately, without leaving the folder.
2. Mark it unfinished. The book icon returns.
3. Scroll a book folder up and down; confirm no visible lag from per-row reads
   (cache should make repeat draws free).
4. Confirm folders and non-book files are unchanged.
5. A folder with 200+ entries (index mode) shows plain book icons, no checkmark
   — expected.

## Out of scope

- Checkmark in index mode (200+ entry folders).
- Any change to how finished status is stored or set.
- Cover-thumbnail or default-list overlay treatments beyond the icon swap.
