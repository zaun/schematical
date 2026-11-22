# plan-53-F: The User Library window, and the end-to-end run

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-53-E

A window, **File › User Library…**, lists every part in the User Library and
removes them. It also opens a part in the editor (**Edit…**, plan-53-E), adds a
part from a file (without the Load Part… question, since adding is the point),
shows a part's file in Finder, and lists library files that could not be loaded
so they can be removed. This sub-plan ends with
a run of the real app that proves the whole of plan-53: a part added, listed,
edited in place and seen to change at once in an open schematic, saved as a new
part, kept across a relaunch, offered on opening a schematic, and removed.

References:

- plan-53-A, plan-53-B, plan-53-C, plan-53-D, plan-53-E
- `schematical/schematical/schematicalApp.swift` — the scenes (`WindowGroup`s
  at `:64–165`)
- `schematical/schematical/BillOfMaterials/BillOfMaterialsWindow.swift` — the
  `Table` idiom to mirror (`:218`)
- `schematical/schematical/Document/SchematicFileController.swift` —
  `PartFileLoading`, `PartFileMenuItems`
- `schematical/schematical/Document/UserLibraryPrompts.swift` (plan-53-C)
- `schematical/schematical/Help/KeyboardHelp.swift:110`

## Prerequisites

Stated in plan-53-A; this sub-plan additionally needs plan-53-E complete.

| Must be true | Command | Status |
|---|---|---|
| plan-53-E is complete and archived | `ls planning/completed/plan-53-E-*.md` → one file | MET (run 2026-11-21) |
| The catalog lists library items and problems | `grep -c 'userLibraryItems\|userLibraryProblems' schematical/schematical/Backend/PartCatalog.swift` → at least `2` | MET (run 2026-11-21: `4`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue, and report all of them if you stop.

## 1. Goal

- File › User Library… opens one window (a second use brings the same window
  to the front) listing the User Library: name, manufacturer, kind, package,
  date added; empty-library text when there are none.
- Selected parts can be removed after a confirmation; **Edit…** opens the
  selected part in the Part Editor (double-clicking a row does the same);
  **Add Part…** adds a part file; **Show in Finder** reveals the selected
  parts' files.
- Library files that could not be loaded are listed in the same window with
  the reason, and can be removed.
- The end-to-end run in § 4.3 passes on the real app.

### Non-goals (explicit constraints)

- **No editing in this window itself**: Edit… opens the Part Editor
  (plan-53-E), where Save updates the part and Save As… makes a new one.
- **No export/import** of the whole library, no sync.
- **The window has no per-schematic state**; it reads the one shared catalog.
- **The owner's real library is left as it was found** by the end-to-end run:
  it removes what it added (§ 4.3).

## 2. Current State

- All the app's secondary windows are `WindowGroup`s keyed by a target value
  (`schematicalApp.swift:64–165`); none is a single-instance `Window`. The
  deployment target is macOS 15.8 (`MACOSX_DEPLOYMENT_TARGET = 15.8` in
  `project.pbxproj`), which has `Window`.
- The Bill of Materials window shows a `Table(lines, selection:)` with a
  `ContentUnavailableView` for no lines (`BillOfMaterialsWindow.swift:205–235`).
- `PartFileMenuItems` (`SchematicFileController.swift:606`) is a `View`, so it can
  take `@Environment(\.openWindow)` as `NewProjectMenuItem` does (`:626`).
- No code reveals a file in Finder
  (`grep -rn 'activateFileViewerSelecting' schematical/schematical --include='*.swift' | wc -l` → 0).
- plan-53-B gives the catalog `userLibraryItems` (part, added, url),
  `userLibraryProblems`, `removeFromUserLibrary(uuid:)`, `addToUserLibrary(fileText:source:)`
  and the library's `removeFile(at:)`.

### Verified properties

- **`PartCatalogStore` is `@Observable`** (`PartCatalog.swift:1340–1342`), so a
  view that reads `userLibraryItems` and `userLibraryProblems` refreshes when they
  change; both derive from stored properties of the store.
- **The part's kind name is available.** `PartKinds.name(of: part.kind)`
  (`ChipDrawerView.swift:365`) and `PartDraft.displayName(of:)`
  (`PartEditorModel.swift:442`) give the kind's display name.

## 3. Design Overview

One new view with a small pure row builder, one scene, one menu item:

- `UserLibraryRow` (`id` = uuid, `name`, `manufacturer`, `kind`, `package`, `added`,
  `url`) built by `UserLibraryRow.rows(from items: [UserLibraryItem]) -> [UserLibraryRow]`,
  in name order (natural order, as the drawer's `nameRank`).
- `UserLibraryWindowView(catalog:)`: if there are problems, a top section
  `Couldn't load` (file name, reason, **Remove**, **Show in Finder**); then a
  `Table` of rows with multi-selection, or `ContentUnavailableView("No Parts in Your Library", …)`;
  a bottom bar: **Add Part…**, **Edit…** (one part selected), **Show in Finder**,
  **Remove** (the last two and Edit… disabled with nothing selected).
- Scene: `Window("User Library", id: "user-library") { UserLibraryWindowView(catalog: .shared) }`,
  `.defaultSize(width: 760, height: 420)`, `.restorationBehavior(.disabled)`.
- Menu: `File › User Library…` after Load Part… in `PartFileMenuItems`
  (`openWindow(id: "user-library")`).

**Correctness risk** is low: the data and the actions are plan-53-B's and C's
and are tested there; this sub-plan is presentation. The real run (§ 4.3) is
where an integration fault would show.

**Gate class.** New behavior: tests for the row builder and the new catalog
call, plus the real-app run.

Rejected alternatives:

- **A sheet in Settings** — the owner asked for a window to list and remove.
- **A `WindowGroup` with a dummy target** — a `Window` is one window by
  construction, which is what a library list needs.

## 4. Detailed Design

### 4.1 Catalog addition

`PartCatalogStore.removeUserLibraryProblem(_ problem: UserPartLibraryProblem) throws`:
`userLibrary.removeFile(at:)`, then drops the entry from `userLibraryProblems`.

### 4.2 Actions

- **Add Part…**: `PartFileLoading.addToLibrary(catalog:)` — the open panel as
  Load Part…'s, then `catalog.addToUserLibrary(fileText:source:)` with no
  question; a failure shows plan-53-C's failures alert (`Couldn't add to your
  User Library`). The panel code is shared with `loadPart`.
- **Remove**: plan-53-C's `removeAlert(name:)` for one part; for several,
  `removeAlert(count:)` (new): message `Remove N parts from your User Library?`,
  the same text and buttons; then `removeFromUserLibrary(uuid:)` for each,
  failures through the failures alert.
- **Edit…** (and a double-click on a row): `openWindow(id: PartEditorScene.id, value: PartEditorTarget(source: .libraryPart(uuid:)))` for the selected part, gated like the drawer's Edit… (`PartKinds.name(of:)` and `PartDraft.enabledKinds`).
- **Show in Finder**: `NSWorkspace.shared.activateFileViewerSelecting(urls)`.

### 4.3 The end-to-end run

Run on the real app (Debug build, `run` skill), with the library folder
`~/Library/Containers/jgz.schematical/Data/Library/Application Support/UserLibrary`
(`jgz.schematical` is `PRODUCT_BUNDLE_IDENTIFIER`). Before: record
`ls` of the folder. Steps, each a checked line in Phase 2:

1. Right-click `SN74HC00N` in the drawer → **Copy to Editor…**; set the part
   number to `PROOF-NAND`; **Save Part…** to `/tmp/plan53/proof-nand.schpart`;
   close the editor.
2. **File › Load Part…** → choose the file → the sheet offers Add to Library /
   Use Once / Cancel → **Use Once**. The drawer shows a **File Library**
   category with `PROOF-NAND`. The library folder is unchanged.
3. Right-click `PROOF-NAND` → **Add to User Library**. The part now lists under
   **User Library**; the folder holds one `.schpart`.
4. **File › User Library…** shows `PROOF-NAND` with its manufacturer, kind,
   package and date.
5. Place `PROOF-NAND` on the schematic, save it as `/tmp/plan53/proof.schematical`.
6. In the library window select `PROOF-NAND` → **Edit…**: the Part Editor opens
   in All fields mode with no Add to Library button. Rename pin 1 from `A` to
   `AA`, press ⌘S. The status reads "Updated PROOF-NAND in your User Library",
   the placed part on the schematic shows pin `AA` at once (no relaunch), and
   the folder still holds one `.schpart`, whose text has `AA`.
7. In the editor choose **Save As…**: no file panel; `PROOF-NAND.2` appears in
   the library window and the drawer (two files in the folder) and the editor's
   title is `PROOF-NAND.2`; `PROOF-NAND` still has pin `AA`.
8. Quit and relaunch the app: the drawer lists `PROOF-NAND` and `PROOF-NAND.2`
   under **User Library** with no action taken.
9. Open `/tmp/plan53/proof.schematical` (it embeds the version with pin `A`,
   under the uuid the library now holds with other content): the sheet offers
   to add its part; answer **Not Now**: the schematic opens with the part placed
   and pin `A`, and the part is listed under **File Library**. Open the file
   again: the sheet appears again; answer **Add to Library**: the part lists
   under User Library (three parts).
10. In the window select all three parts → **Remove** (confirm "Remove 3
    parts"). The drawer lists none and the folder is back to its recorded `ls`.
    Quit the app and delete `/tmp/plan53`.

Driving follows `AGENTS.md` § Testing in the app: when the run is done, move
the pointer to the top-left corner (0, 0). Before driving, check which app is
frontmost (`lsappinfo front`), as other sessions also drive the screen.

## Compatibility / Format Impact

None to any file format. A new window and one File menu item.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot tasks stay, struck through with evidence; fill
> each `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — The window

- [x] `Backend/PartCatalog.swift`: `removeUserLibraryProblem` (§ 4.1).
- [x] `Document/SchematicFileController.swift`: `PartFileLoading.addToLibrary(catalog:)` sharing `loadPart`'s open panel (§ 4.2); `PartFileMenuItems` gets **User Library…** (`@Environment(\.openWindow)`, `openWindow(id: "user-library")`).
- [x] `Document/UserLibraryPrompts.swift`: `removeAlert(count:)` (§ 4.2).
- [x] `Drawer/UserLibraryWindow.swift` (new): `UserLibraryRow`, `UserLibraryWindowView` (§ 3, § 4.2), including Edit… and the row double-click. Mirror `BillOfMaterialsWindow.swift`'s `Table` and `ContentUnavailableView`.
- [x] `schematicalApp.swift`: the `Window("User Library", id: "user-library")` scene (§ 3), next to the Part Editor's scene.
- [x] `schematicalTests/UserLibraryWindowTests.swift` (new, temp-folder library): `UserLibraryRow.rows` for three parts gives three rows in name order with the part's manufacturer, kind name, package and the file's added date; after `removeFromUserLibrary` it gives two; `removeUserLibraryProblem` deletes a corrupt file and empties `userLibraryProblems`; `removeAlert(count: 3)` message is `Remove 3 parts from your User Library?` with buttons Remove, Cancel.

Acceptance: the row builder and the new catalog call behave as listed, and the app builds with the new scene. **MET** (`xcodebuild test … UserLibraryWindowTests UserLibraryPromptsTests` → `Executed 13 tests, with 0 failures`, `** TEST SUCCEEDED **`; the app builds with the new scene)
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryWindowTests -only-testing:schematicalTests/UserLibraryPromptsTests` → `** TEST SUCCEEDED **` (est. 6–12 min).
Commit: —

### Phase 2 — Help, docs and the end-to-end run

- [ ] `Help/KeyboardHelp.swift`: an entry "File › User Library…" → "List the parts in your User Library; remove them, add a part file, or show a part's file in Finder".
- [ ] `docs/parts-format.md` § Single part files, "The User Library" paragraph: the window and where the folder is (`Application Support/UserLibrary` in the app's container).
- [ ] The run of § 4.3, steps 1–10, each noted in the Corrections section with what was seen (or "as expected").
- [ ] After the run: the library folder's `ls` equals the one recorded before step 1; `/tmp/plan53` is gone; the pointer is at (0, 0).

Acceptance: all ten steps of § 4.3 behave as written, and the library folder ends as it began.
  Check: § 4.3 in the real app, then `ls "$HOME/Library/Containers/jgz.schematical/Data/Library/Application Support/UserLibrary"` → the same listing as before the run (est. 30–40 min; this is the only check that exercises the sheets, the drawer menu and the relaunch, which cannot run headless).
Commit: —

## Validation Plan

- Tests: `UserLibraryWindowTests` (Phase 1), plus plan-53-A–C's.
- Coverage check: the window view itself is not unit-tested (SwiftUI); its data
  and actions are, and the run covers the view.
- Runtime proof: § 4.3.
- Doc sync: `docs/parts-format.md`, `docs/file-format.md` (plan-53-C), `Help/KeyboardHelp.swift`.
- Final gate (run ONCE, after Phase 2):
  `ctest --test-dir /tmp/logic-build -R 'part_file_test|library_check|listing_check'` and
  `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartRegistrationTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PartFileSaveOpenTests -only-testing:schematicalTests/UserLibraryUpdateTests -only-testing:schematicalTests/PartEditorLibraryTests -only-testing:schematicalTests/UserPartLibraryTests -only-testing:schematicalTests/UserLibraryCatalogTests -only-testing:schematicalTests/UserLibraryPromptsTests -only-testing:schematicalTests/UserLibraryWindowTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/AutomationTests`
  → all pass (est. 15–25 min). It covers every area plan-53 touches (backend key,
  catalog, file open, Part Editor, drawer categories, help, automation text).

## Decisions

- A new window lists the user library parts and removes them — decided by the owner, 2026-11-21.
- The window has Edit…, which opens the part in the Part Editor (Save updates, Save As… makes new) — follows from the owner's edit rule, 2026-11-21.
- Parts from the library are saved with a schematic as custom parts are now — decided by the owner, 2026-11-21 (no change to the file).
- Add Part…, Show in Finder and the failed-files list in the window — author's call, offered in the design discussion and not objected to by the owner.
- A single-instance `Window` scene — author's call.

## Corrections

(Filled in during execution, including what each step of § 4.3 showed.)

## Summary

Presentation over a tested model; the real run is the check that matters. After
plan-53 a custom part has one identity per version, lives in the User Library
or the File Library, survives a relaunch when the user keeps it, and every
schematic still carries the parts it uses. Untouched: the `.schematical`
format, bundled parts, the backend's one-level unload.
