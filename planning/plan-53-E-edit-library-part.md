# plan-53-E: Edit a User Library part: Save updates it, Save As… makes a new one

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-53-D

A User Library part gets an **Edit…** command that opens it in the Part Editor.
In that window **Save updates the existing library part** (same uuid, applied
at once everywhere, plan-53-D) and **Save As… makes a new library part** (a new
uuid, with the old one kept as `originId`). Together with plan-53-A this is the
owner's rule for editing: Save updates, Save As… makes new.

This sub-plan is the editor side and the drawer entry point; the library
window's Edit… button is plan-53-F.

References:

- plan-53-A (`freshened()`, `originId`), plan-53-D (`updateUserLibraryPart`),
  plan-53-C (the drawer's context-menu items, `UserLibraryPrompts`)
- `schematical/schematical/Editor/PartEditorTarget.swift` — `Source`
- `schematical/schematical/Editor/PartEditorModel.swift` — `init(target:…)`
  (`:358`), `save` / `saveAs` / `write(to:)` / `load(text:from:)` /
  `addToLibrary` (`:502–560`), `isNewPart` (`:154`)
- `schematical/schematical/Editor/PartEditorCommands.swift`,
  `Editor/PartEditorWindow.swift:31–34`, `Editor/Wizard/ReviewStepPage.swift:44–46`
- `schematical/schematical/Drawer/ChipDrawerView.swift` — `partRow` context menu (`:363`)
- `schematical/schematical/Help/KeyboardHelp.swift`

## Prerequisites

Stated in plan-53-A; this sub-plan additionally needs plan-53-D complete.

| Must be true | Command | Status |
|---|---|---|
| plan-53-D is complete and archived | `ls planning/completed/plan-53-D-*.md` → one file | MET (run 2026-11-21) |
| The catalog has `updateUserLibraryPart` | `grep -c 'func updateUserLibraryPart' schematical/schematical/Backend/PartCatalog.swift` → `1` | MET (run 2026-11-21: `1`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue, and report all of them if you stop.

## 1. Goal

- Right-click a User Library part in the drawer → **Edit…** opens the Part
  Editor on it, in All fields mode.
- In that window Save (⌘S) updates the library part: the picker, the library
  file and every open schematic that places it show the new version at once.
- Save As… (⇧⌘S) adds a new library part (new uuid, `originId` set; a taken
  name gets the usual `.2` suffix) and the window then edits that new part.
- Add to Library is not offered in that window (Save is the add).

### Non-goals (explicit constraints)

- **No kind change.** The window is not a new part (`isNewPart` false), so the
  kind's Change… is not offered; the catalog also refuses a kind change
  (plan-53-D).
- **A library window has no file.** Save As… does not show a file panel. Writing
  a `.schpart` file of a library part is not part of this plan (the library
  window's Show in Finder reveals its file).
- **File windows are unchanged** (plan-53-A): Open Part… keeps the uuid, Save
  rewrites the file, Save As… writes a new file under a new uuid.
- **A bundled part and a File Library part have no Edit…**: a bundled part has
  Copy to Editor…, and a File Library part is added to the library first.
- **No refresh across windows.** Two editors on one library part do not see each
  other's Saves; the later Save wins.

## 2. Current State

- `PartEditorTarget.Source` is `wizard`, `new(kind:)`, `copy(name:)`, `file(URL)`
  (`PartEditorTarget.swift:11–21`), `Hashable` and `Codable`.
- `PartEditorModel.init(target:catalog:validator:)` builds the draft per source
  (`PartEditorModel.swift:358–396`); `.copy` and `.file` set `mode = .allFields`;
  `isNewPart` is true only for `.wizard` and `.new` (`:367`, `:372`) and is what
  makes All fields show the kind's Change… (`:154`).
- `save()` writes to `fileURL` or calls `saveAs()` (`:525–540`); `saveAs()` shows
  an `NSSavePanel` then `write(to:)`; `write(to:)` validates, writes and sets
  `fileURL` and `savedText` (`:492–500`). `load(text:from:)` (`:502`) replaces the
  draft from a file. `addToLibrary()` (`:516`) registers the draft.
- `PartEditorCommands` shows **Save Part As…** and **Add to Library** for a
  focused editor (`PartEditorCommands.swift:22–26`); the toolbar has Open Part…,
  Save Part… and Add to Library (`PartEditorWindow.swift:31–34`); the wizard's
  review page has Save Part… and Add to Library (`ReviewStepPage.swift:44–46`).
- ⌘S in an editor window is `editor.save()`, disabled while `!editor.isValid`
  (`SchematicFileController.swift:678–682`).
- The drawer row's context menu has Copy to Editor…, gated by
  `PartKinds.name(of: part.kind)` and `PartDraft.enabledKinds`
  (`ChipDrawerView.swift:363–371`), and (after plan-53-C) Add to / Remove from
  User Library.

### Measured populations

| What | Count | Command |
|---|---|---|
| Places that show an Add to Library button or menu item | 3 | `grep -rn 'addToLibraryFromMenu' schematical/schematical --include='*.swift'` → `PartEditorCommands.swift:25`, `PartEditorWindow.swift:34`, `Wizard/ReviewStepPage.swift:46` (plus the definition) |
| Places in the app that build a `PartEditorTarget` | 2 | `grep -rn 'PartEditorTarget(' schematical/schematical --include='*.swift' \| wc -l` → 2 (`ChipDrawerView.swift:367`, `PartEditorCommands.swift:18`); the only `switch` on its `Source` is `PartEditorModel.init` (`PartEditorModel.swift:362`) |

### Verified properties

- **A new target case is Codable for free.** `Source` is a plain enum with
  associated values (`PartEditorTarget.swift:11–21`); adding
  `case libraryPart(uuid: String)` keeps the synthesized `Codable` and
  `Hashable`.
- **The editor can hold the library part's draft unchanged.** Read
  `PartEditorModel.copy(named:)` (`:409`): a registered custom part's draft is
  `PartDraft(file: JSONValue(text: registeredText))`; the same call without the
  new uuid gives the library part's own draft.

## 3. Design Overview

- **Target and model.** `PartEditorTarget.Source.libraryPart(uuid:)`;
  `PartEditorModel.libraryUUID: String?` (set only by that source). Its draft is
  built from `catalog.registeredText(uuid:)` as is (same uuid), mode All fields,
  `isNewPart` false, `fileURL` nil, `savedText` the draft's text. A uuid that is
  not in the catalog sets `openError` ("That part is not in your User Library.").
- **Save.** With `libraryUUID` set, `save()` calls `saveToLibrary()`:
  `validateNow()`, then `catalog.updateUserLibraryPart(uuid:fileText:)` with the
  draft's text; on success `savedText` = the draft's text and the status reads
  `Updated NAME in your User Library`. A failure shows the existing alert
  `Couldn't update the part`; `notInUserLibrary` reads `“NAME” is no longer in
  your User Library. Use Save As… to keep it.`
- **Save As….** With `libraryUUID` set, `saveAs()` calls `saveAsNewLibraryPart()`
  (no panel): `validateNow()`, `draft = draft.freshened()`,
  `catalog.addToUserLibrary(fileText:source:)`, then `libraryUUID` = the new
  part's uuid and `draft.name` = the part's registered name (suffixed when
  taken); status `Added NAME to your User Library`. The old part is unchanged.
- **Leaving the library.** `load(text:from:)` (Open Part… inside the window)
  sets `libraryUUID` = nil: the window is then a file window.
- **Menus.** Add to Library (`PartEditorCommands`, the toolbar, the wizard's
  review page) is hidden when `libraryUUID != nil`; the title of ⌘S and Save As…
  stay as they are.
- **Drawer.** Right-click a part listed `user` → **Edit…** (above Copy to
  Editor…): `openWindow(id: PartEditorScene.id, value: PartEditorTarget(source: .libraryPart(uuid:)))`,
  gated like Copy to Editor….

**Correctness risk** is low: the engine is plan-53-D's and tested there. The
care points are that Save As… must not touch the old part and that the window
rebinds to the new one.

**Gate class.** New behavior; model-level tests plus plan-53-F's real-app run.

Rejected alternatives:

- **A file panel for a library window's Save As…** — the owner's rule is
  "update existing / make new"; for a library part the new one is another
  library part.
- **Save As… leaves the window on the old part** — the next Save would then
  overwrite the part the user just chose not to change.

## 4. Detailed Design

### 4.1 `PartEditorTarget` and `PartEditorModel`

`case libraryPart(uuid: String)`; the init branch of § 3. New methods:
`saveToLibrary() throws -> CatalogPart` and
`saveAsNewLibraryPart() throws -> CatalogPart` (the throwing cores, called by
`save()` / `saveAs()` which show the alerts); a `libraryUUID` property
(`private(set)`).

### 4.2 Menus

`PartEditorCommands`, `PartEditorWindow` (toolbar) and `ReviewStepPage` wrap
their Add to Library item in `if model.libraryUUID == nil` (or
`editor.libraryUUID == nil`).

### 4.3 Drawer

`ChipDrawerView.partRow`: the Edit… item (§ 3).

### 4.4 Help and docs

`Help/KeyboardHelp.swift`: entry "Right-click a User Library part" gains
`Edit…: change it everywhere at once (Save updates it, Save As… adds a new one)`
(the plan-53-C entry for Remove becomes "Edit… / Remove from User Library…").
`docs/parts-format.md` § "The User Library": editing — Save updates in place and
applies at once, Save As… adds a new part whose file records `originId`.

## Compatibility / Format Impact

None to any file format. A new context-menu item and editor behavior for
library-bound windows only.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot tasks stay, struck through with evidence; fill
> each `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — The model: library-bound editing

Safe alone: nothing opens a `.libraryPart` window yet.

- [x] `PartEditorTarget.swift`: `case libraryPart(uuid: String)`.
- [x] `PartEditorModel.swift`: the init branch, `libraryUUID`, `saveToLibrary()`, `saveAsNewLibraryPart()`, `save()` / `saveAs()` routing, `load(text:from:)` clearing the binding (§ 3, § 4.1).
- [x] `schematicalTests/PartEditorLibraryTests.swift` (new, temp-folder library and a fresh catalog): the model for a library part has `libraryUUID`, `isNewPart == false`, a draft equal to the part's own (same uuid) and is not dirty; `saveToLibrary()` after changing a pin name returns the part with the same uuid, the catalog and file hold the new text, the picker has one part, and a placed chip in a document held by a controller shows the new pin name; `saveAsNewLibraryPart()` leaves the first part's text and file as they were, adds a second whose uuid differs, whose `originId` is the first's uuid, whose name is the suffixed one when the draft kept the old name, and rebinds: a following `saveToLibrary()` changes the second only; a bad draft (the backend rejects it) throws and changes nothing; `saveToLibrary()` after the part was removed throws `notInUserLibrary`; `load(text:from:)` clears `libraryUUID`; an unknown uuid sets `openError`.

Acceptance: Save updates in place and Save As… adds a new part, at the model level. **MET** (`xcodebuild test … -only-testing:schematicalTests/PartEditorLibraryTests` → `Executed 7 tests, with 0 failures`, `** TEST SUCCEEDED **`)
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartEditorLibraryTests` → `** TEST SUCCEEDED **` (est. 5–10 min).
Commit: b6ae9573

### Phase 2 — Entry point, menus, help

- [x] `ChipDrawerView.swift`: the Edit… item (§ 3).
- [x] `PartEditorCommands.swift`, `PartEditorWindow.swift`, `Wizard/ReviewStepPage.swift`: hide Add to Library when `libraryUUID != nil` (§ 4.2).
- [x] `Help/KeyboardHelp.swift`, `docs/parts-format.md`: § 4.4.
- [x] `schematicalTests/PartEditorLibraryTests.swift`: a `PartEditorTarget(source: .libraryPart(uuid:))` round-trips through `JSONEncoder`/`JSONDecoder` equal (windows are restored by value); `KeyboardHelpTests` still passes.

Acceptance: the drawer opens the editor on a library part; the menus match the model. **MET** (`xcodebuild test … PartEditorLibraryTests KeyboardHelpTests PartEditorWindowTests` → `Executed 21 tests, with 0 failures`, `** TEST SUCCEEDED **`; menus and sheets are exercised in plan-53-F's real-app run)
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartEditorLibraryTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 6–12 min). The menu and sheet behavior is exercised in plan-53-F's real-app run.
Commit: (this commit)

## Validation Plan

- Tests: `PartEditorLibraryTests` (Phases 1–2), plus plan-53-D's.
- Coverage check: every changed model method is called by
  `PartEditorLibraryTests`; the menu wiring is not unit-testable and is covered
  by the real-app run.
- Runtime proof: plan-53-F § 4.3.
- Doc sync: `docs/parts-format.md`, `Help/KeyboardHelp.swift` (Phase 2).
- Final gate (run ONCE, after Phase 2): the three test classes of Phase 2's check
  plus `-only-testing:schematicalTests/UserLibraryUpdateTests` and
  `-only-testing:schematicalTests/PartFileSaveOpenTests` → `** TEST SUCCEEDED **`
  (est. 10–15 min).

## Decisions

- Editing: Save updates the existing part, Save As… makes a new one — decided by the owner, 2026-11-21.
- This covers library parts as well as files, with a new Edit… on a User Library part — decided by the owner, 2026-11-21 ("Files and library parts").
- A library window's Save As… makes a new library part with no file panel — author's call: for a library part the "new one" is another library part.
- Add to Library is hidden in a library-bound window — author's call: Save is the add there.

## Corrections

(Filled in during execution.)

## Summary

The editor side of editing: two small routings over plan-53-D's engine and one
rebinding. Untouched: file windows, bundled parts, the file formats, the
library window (plan-53-F).
