# plan-53-C: Offers to add a part to the User Library

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-53-B

The library exists (plan-53-B); this sub-plan is how parts get into it and out
of it from the places the user already works:

- **File › Load Part…** asks whether to add the part to the User Library or use
  it once.
- **Opening a schematic** that embeds custom parts not in the User Library
  offers to add them.
- The Part Editor's **Add to Library** adds to the User Library.
- A drawer part's context menu: **Add to User Library** on a File Library part,
  **Remove from User Library…** on a User Library part.

The window that lists the library is plan-53-D.

References:

- plan-53-A, plan-53-B
- `schematical/schematical/Document/SchematicFileController.swift` —
  `PartFileLoading` (`:571`), `PartFileMenuItems` (`:606`), `load(_:from:)`,
  `run(_ alert:)`, `present(_:title:)`
- `schematical/schematical/Document/SchematicFile.swift` — `apply`
- `schematical/schematical/Editor/PartEditorModel.swift` — `addToLibrary`
- `schematical/schematical/Drawer/ChipDrawerView.swift` — `partRow` context menu (`:363`)
- `schematical/schematical/Help/KeyboardHelp.swift:110`
- `docs/file-format.md` § Custom parts

## Prerequisites

Stated in plan-53-A; this sub-plan additionally needs plan-53-B complete.

| Must be true | Command | Status |
|---|---|---|
| plan-53-B is complete and archived | `ls planning/completed/plan-53-B-*.md` → one file | NOT MET at writing (unbuilt) |
| The catalog has the library API | `grep -c 'func addToUserLibrary' schematical/schematical/Backend/PartCatalog.swift` → `2` | NOT MET at writing |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue, and report all of them if you stop.

## 1. Goal

- Load Part… on a file whose part is not in the User Library shows a choice
  (Add to Library, Use Once, Cancel) and does what it says; a part already in
  the library is reported as such and nothing else happens.
- Opening a schematic lists its embedded parts that are not in the User
  Library and offers Add to Library or Not Now. **Not Now is asked again the
  next time the file is opened.**
- The Part Editor's Add to Library puts the part in the User Library.
- A File Library part can be added from its drawer row; a User Library part can
  be removed from it (after a confirmation).

### Non-goals (explicit constraints)

- **Opening a file never adds a part without the user's say-so**, and a file
  open that is answered "Not Now" still opens and works.
- **No remembered "declined" state.** Nothing about a file is stored.
- **The `.schematical` file does not change**: it embeds its custom parts as
  before, whatever the answer.
- **No new window.** The library window is plan-53-D.
- **Load Part… keeps its menu title and place.**

## 2. Current State

- `PartFileLoading.loadPart` (`SchematicFileController.swift:575`): an open
  panel, then `catalog.register(fileText:source:)`; a failure shows an alert
  ("Couldn't load “…”"); success shows nothing.
- `SchematicFileController.load(_:from:)` (`SchematicFileController.swift:267`)
  calls `file.apply(to:catalog:fileURL:)` and shows an error alert on failure.
  `apply` returns nothing (`SchematicFile.swift:954`). Both the
  `open(_ url:)` in-place path and `loadClaimedOpen()` reach `load`.
- `PartEditorModel.addToLibrary()` (`PartEditorModel.swift:516`) registers the
  draft for the session and sets the status "Added … to the parts drawer". It
  is called by `addToLibraryFromMenu()` from `PartEditorCommands.swift:25`,
  `PartEditorWindow.swift:34` and `Wizard/ReviewStepPage.swift:46`.
- The drawer row's context menu has one item, Copy to Editor…
  (`ChipDrawerView.swift:363–371`).
- The controller already has `run(_ alert: NSAlert) async -> NSApplication.ModalResponse`
  (used at `SchematicFileController.swift:434`).
- `KeyboardHelp.swift:110`: "File › Load Part… — Add a .schpart file's part to
  the drawer for this session". `KeyboardHelpTests` checks that entries are
  non-empty and that listed keys exist, not their wording.

### Measured populations

| What | Count | Command |
|---|---|---|
| Callers of `addToLibraryFromMenu` | 3 | `grep -rn 'addToLibraryFromMenu' schematical/schematical --include='*.swift' \| wc -l` → 4 (the definition and 3 callers) |
| Callers of `SchematicFile.apply` in the app | 1 | `grep -rn '\.apply(to: document' schematical/schematical --include='*.swift' \| wc -l` → 1 (`SchematicFileController.swift:269`) |
| Test files that call `apply` | 20 | `grep -rln 'apply(to:.*catalog:' schematical/schematicalTests \| wc -l` → 20 (every one is `SchematicFile.apply`; `@discardableResult` keeps them unchanged) |

### Verified properties

- **`load(_:from:)` is synchronous and main-actor.** Read
  `SchematicFileController.swift:264–275`; it is called from `open(_ url:)`
  (async) and `loadClaimedOpen()` (sync). The offer therefore starts a `Task`
  and does not block the open.
- **The alerts in this file are run through `run(_:)`.** Read `:427–434`,
  `:510–520`, `:528–534`.

## 3. Design Overview

Each prompt is split in two: a **pure core** (decides, adds, collects errors)
that tests call with a stub answer, and a **thin alert** that asks.

```swift
// New file: Document/UserLibraryPrompts.swift
enum LibraryChoice { case add, useOnce, cancel }

enum LoadPartOutcome { case added(CatalogPart), usedOnce(CatalogPart), alreadyInLibrary(CatalogPart), cancelled }
```

- **Load Part… core** — `PartFileLoading.load(text:source:catalog:ask:) async throws -> LoadPartOutcome`:
  parse the file's uuid and name (`PartFileObject`); if that uuid is registered
  with the same content and listed `user`, return `.alreadyInLibrary` without
  asking; otherwise `ask(name)`; `.add` → `catalog.addToUserLibrary(fileText:source:)`,
  `.useOnce` → `catalog.register(fileText:source:)`, `.cancel` → `.cancelled`.
  A file with no readable name is registered as before (the backend's message
  reports it).
- **File open core** — `apply` returns the custom parts the file embedded
  (`@discardableResult func apply(...) throws -> [CatalogPart]`: after the
  remap of plan-53-A, in embed order, each once).
  `UserLibraryPrompts.offerable(_ parts:, catalog:) -> [CatalogPart]` keeps those
  not listed `user`. `UserLibraryPrompts.add(_ parts:, catalog:) -> [(String, Error)]`
  adds each and returns the failures (part name, error).
- **Alerts** — `NSAlert`s run through the window's sheet as the controller's do:
  - Load Part…: message `Add “NAME” to your User Library?`; text `Adding keeps it for every schematic. Using it once keeps it until you quit.`; buttons **Add to Library** (default), **Use Once**, **Cancel**.
  - Already in the library: message `“NAME” is already in your User Library.`; button OK.
  - File open: message `This schematic has custom parts that are not in your User Library`; text lists up to 8 names, one per line, then `and N more`, followed by `They are saved in the schematic either way.`; buttons **Add to Library** (default), **Not Now**.
  - Remove: message `Remove “NAME” from your User Library?`; text `Schematics that use it keep their own copy.`; buttons **Remove**, **Cancel**.
  - Failures of an add or remove: one alert, message `Couldn't add to your User Library` / `Couldn't remove from your User Library`, text one `NAME: reason` line each.

**Correctness risk** is small: the pure cores are tested; the alerts are thin.
The one subtle point is the file-open path, which must not run the offer when
the open itself failed, and must not block the open.

**Gate class.** New behavior; tests plus a runtime run in plan-53-D.

Rejected alternatives:

- **A remembered per-file "don't ask again"** — the owner chose to ask every
  time (2026-11-21); no state to go stale.
- **Adding automatically on open** — the owner wants an offer.
- **A third Load Part… choice for "this file only"** — a loaded part lands in a
  schematic only when placed, and is then saved in it; "Use Once" is that.

## 4. Detailed Design

### 4.1 `apply`

`SchematicFile.apply` collects the registered parts of `customParts` (it already
has them while registering, plan-53-A § 4.4) into an array and returns it.
Marked `@discardableResult`.

### 4.2 `SchematicFileController.load`

After a successful `apply`:

```swift
let offered = UserLibraryPrompts.offerable(parts, catalog: catalog)
if !offered.isEmpty { Task { await offerToAdd(offered, fileName: url.lastPathComponent) } }
```

`offerToAdd` runs the alert via `run(_:)`; on **Add to Library** it calls
`UserLibraryPrompts.add` and shows the failures alert if any.

### 4.3 Load Part…

`PartFileLoading.loadPart` keeps its panel and error alert; between reading the
text and registering it calls `load(text:source:catalog:ask:)` with the alert as
`ask`, and shows the already-in-library alert for that outcome.

### 4.4 Part Editor

`PartEditorModel.addToLibrary()` calls `catalog.addToUserLibrary(fileText:source:)`
(after plan-53-A's freshening of a colliding draft) and sets the status
`Added NAME to your User Library`. A failed library write
(`PartRegistrationError.libraryWrite`) is shown by the existing alert in
`addToLibraryFromMenu`.

### 4.5 Drawer rows

In `ChipDrawerView.partRow`'s `contextMenu`, before Copy to Editor…, when
`part.uuid != nil`: if `catalog.listing(of: part) == .file`, a button **Add to
User Library** (`catalog.addToUserLibrary(part)`); if `.user`, **Remove from
User Library…** (the confirmation, then `catalog.removeFromUserLibrary(uuid:)`).
A failure shows the failures alert.

### 4.6 Help

`KeyboardHelp.swift`: the Load Part… entry reads "Add a .schpart file's part to
your User Library, or use it until you quit"; new entries: "Right-click a File
Library part" → "Add to User Library: keep a part that came with a schematic",
"Right-click a User Library part" → "Remove from User Library…".

## Compatibility / Format Impact

None to any file format. Behavior: Load Part… now asks; opening a schematic with
unlisted custom parts now shows an alert; Part Editor Add to Library persists.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot tasks stay, struck through with evidence; fill
> each `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — The cores: `apply` result, offerable, add, Load Part…

Safe alone: pure functions and one new return value, no alert shown yet.

- [ ] `SchematicFile.swift`: `apply` returns `[CatalogPart]`, `@discardableResult` (§ 4.1).
- [ ] `Document/UserLibraryPrompts.swift` (new): `LibraryChoice`, `LoadPartOutcome`, `UserLibraryPrompts.offerable`, `UserLibraryPrompts.add` (§ 3).
- [ ] `SchematicFileController.swift`: `PartFileLoading.load(text:source:catalog:ask:)` (§ 3), not yet called by `loadPart` (Phase 2 wires it; until then Load Part… behaves as before).
- [ ] `schematicalTests/UserLibraryPromptsTests.swift` (new, temp-folder library as in plan-53-B's tests): `load` with `.add` lists the part `user` and writes the file; `.useOnce` lists `file` and writes nothing; `.cancel` registers nothing; a part already `user` with the same content returns `.alreadyInLibrary` and `ask` is not called; the same uuid with other content asks, and the result has a fresh uuid; a bad file throws the backend's message; `apply` of a v5 file with two embedded parts returns both, once each, and `offerable` drops the one already `user`; `add` returns a failure for a part when the library folder is unwritable and still adds the others.

Acceptance: the cores do what § 3 says on stub answers.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryPromptsTests -only-testing:schematicalTests/SchematicFileV5Tests` → `** TEST SUCCEEDED **` (est. 6–12 min).
Commit: —

### Phase 2 — The alerts and the drawer menu

- [ ] `UserLibraryPrompts.swift`: the alert builders of § 3 as pure functions returning an `NSAlert` (message, text, buttons) so a test can read them: `loadPartAlert(name:)`, `alreadyInLibraryAlert(name:)`, `fileOpenAlert(names:)`, `removeAlert(name:)`, `failuresAlert(title:failures:)`.
- [ ] `SchematicFileController.swift`: `loadPart` asks with `loadPartAlert` (sheet on the key window, as the open panel is) and shows `alreadyInLibraryAlert`; `load(_:from:)` runs § 4.2.
- [ ] `ChipDrawerView.swift`: the two context-menu items (§ 4.5).
- [ ] `Help/KeyboardHelp.swift`: the three entries (§ 4.6).
- [ ] `schematicalTests/UserLibraryPromptsTests.swift`: each alert builder's message, button titles in order and default button (first); `fileOpenAlert` with 10 names lists 8 and says `and 2 more`.

Acceptance: the alert texts and buttons are as in § 3, and the drawer menu items call the catalog (covered by Phase 3's runtime check).
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryPromptsTests -only-testing:schematicalTests/KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 6–12 min).
Commit: —

### Phase 3 — Part Editor and docs

- [ ] `PartEditorModel.swift`: `addToLibrary()` per § 4.4. `Editor/PartEditorSections.swift:17` and `Editor/Wizard/IdentityStepPage.swift:15` keep their text ("Add to Library adds this one as …").
- [ ] `schematicalTests/PartFileSaveOpenTests.swift`: `testAddToLibrary` also asserts the part's listing is `user` and that its file is in the library folder; add: Add to Library of a draft the library already holds (same content) is a no-op that returns the same part.
- [ ] `docs/file-format.md` § Custom parts: a bullet "On open, the app offers to add the embedded parts that are not in the User Library; the answer does not change the file." `docs/parts-format.md` § Single part files, "The User Library" paragraph (plan-53-B): how parts get in: Load Part…, the open offer, the Part Editor's Add to Library, the drawer's Add to User Library.

Acceptance: the editor's Add to Library persists.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartFileSaveOpenTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 6–12 min).
Commit: —

## Validation Plan

- Tests: per phase, with the negative cases (bad file, same uuid other content,
  unwritable folder, cancel).
- Coverage check: every changed function is called by `UserLibraryPromptsTests`
  or `PartFileSaveOpenTests`; the sheet presentation and the drawer menu are
  covered by the runtime proof in plan-53-D (they cannot run headless).
- Runtime proof: plan-53-D runs the app and exercises Load Part…, the open
  offer, Add/Remove from the drawer menu, and a relaunch.
- Doc sync: `docs/file-format.md`, `docs/parts-format.md`, `Help/KeyboardHelp.swift`.
- Final gate (run ONCE, after Phase 3): the five test classes named in Phases
  1–3 → `** TEST SUCCEEDED **` (est. 10–15 min).

## Decisions

- Opening a file offers to add its embedded custom parts to the User Library — decided by the owner, 2026-11-21.
- "Not Now" is asked again at every open of that file — decided by the owner, 2026-11-21 (no remembered state).
- Load Part… offers a choice when loading a part — decided by the owner, 2026-11-21.
- Load Part… choices are Add to Library / Use Once / Cancel — author's call: a three-button sheet avoids a second dialog for the "this session only" case, which then lists under File Library.
- The remove confirmation and the context-menu wording — author's call.

## Corrections

(Filled in during execution.)

## Summary

The prompts are thin over tested cores. The one care point is the file-open
offer, which must follow a successful open and never block it. Untouched: file
formats, the library window, the backend.
