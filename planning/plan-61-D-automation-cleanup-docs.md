# plan-61-D: Automation, the last of the controller, docs

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-61-C (prerequisites are stated in plan-61-A § Prerequisites)

Closes plan 61: the agent-control tools get tests on the document
architecture, what is left of `SchematicFileController` becomes an exporter
with an honest name, and the docs describe the app as it now is. The single
outcome: no source or doc names `SchematicFileController`, and the window
tools pass a test that drives them through `NSDocumentController`.

References:

- `planning/plan-59-data/findings.md` row A13 and § A16.
- `schematical/schematical/Automation/AutomationSession.swift`,
  `Automation/AutomationTools.swift`.
- `docs/agent-control.md`, `docs/export.md`, `docs/file-format.md`.

## Prerequisites

Stated in plan-61-A. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-61-C is complete | `rg -c '^- \[ \]' planning/plan-61-C-document-app-behavior.md` → no output | MET (no output, 2026-11-24, after 5f2e130a) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run before you continue and before you stop; report all prerequisites if
> you stop.

## 1. Goal

- `list_windows`, `new_window`, `open`, `save` and `focus_window` are tested
  against open `SchematicNSDocument`s.
- The exporter type is `SchematicExporter` in `Document/SchematicExporter.swift`;
  `Document/SchematicFileController.swift` is gone.
- `docs/agent-control.md`, `docs/export.md` and `docs/file-format.md` name the
  document class and the system behaviors, not the controller.

### Non-goals (explicit constraints)

- The tools' names, arguments and result text formats do not change
  (plan-59 A13: the formats are the unchanged tool code's), with one stated
  exception: `save` with a path is a Save As, so the window is then named
  after the new file, as it was before.
- `planning/todo.md` is not edited by this plan's commits when the owner has
  uncommitted changes in it (`git status --short planning/todo.md`).

## 2. Current State

After 61-C. `AutomationTools.sessions()` (`Automation/AutomationTools.swift:228`)
maps the open documents to contexts; `new_window`, `open` and `save`
(`AutomationSession.swift:56–96`) call `NSDocumentController` and
`NSDocument.save(to:ofType:for:)` (put there by 61-B from the spike's patch).
`SchematicFileController` holds only exports, print, the custom-parts offer,
`updateOpenDocuments` and the panel helpers. `WindowReader` and
`WindowCloseGuard` live in its file; the Part Editor uses the guard
(`Editor/PartEditorModel.swift:356`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Docs naming the controller | 3 | `rg -l 'SchematicFileController' docs` → `docs/export.md`, `docs/agent-control.md`, `docs/file-format.md` |
| Window tools | 5 | `rg -n 'name: "(list_windows\|new_window\|open\|save\|focus_window)"' schematical/schematical/Automation/AutomationSession.swift` → lines 20, 47, 56, 72, 88 |
| Test files constructing the controller | 8 (9 sites) | `rg -c 'SchematicFileController\(document:' schematical/schematicalTests` → 8 files, sum 9 |

### Verified properties

| Claim | How verified |
|---|---|
| The five tools work through `NSDocumentController` with the model taken from the document | plan-59 A13 (H), `planning/plan-59-data/evidence/A-H-session2.txt`: `opened window 90792DB8`, `opened in window 611651DC`, `saved to exp/copy.schematical`, `window 611651DC is frontmost` |
| `AutomationTools.call(_:_:)` is the dispatcher the MCP server calls, so a test can call it without the server | read `Automation/AutomationTools.swift` (`static func call`); plan-59's probe ran the tools through it |

## 3. Design Overview

Three steps: tests for the tools, the rename, the docs. The rename is
mechanical (type and file name; `WindowReader` and `WindowCloseGuard` move to
`Document/WindowHelpers.swift`). Risk is low; the tests come first so the
rename is checked by them.

Byte-identity is not a gate.

Rejected: deleting the exporter type and moving its methods onto the
document (the 8 test files build it without a document today, and the A16
seam keeps that).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot tasks keep their line with the
> evidence. Fill `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — Tool tests

- [x] New `schematicalTests/AutomationWindowToolsTests.swift`: with a temporary folder set as the shared folder, `AutomationTools.call("new_window")` adds a document; `call("open", ["path": …])` opens an example copied there and a second call does not add a document; `call("save")` on an untitled document fails with `This window has no file yet; pass a path.`; `call("save", ["path": …])` writes the file; `call("list_windows")` lists every open document with `unsaved` only on an edited one. Done: 5 tests, one per tool (`focus_window` too), also the refusals (a missing file, a path outside the folder, an unknown window id). `AutomationFolder.shareTemporarily(_:)` shares the temporary folder with no bookmark saved and puts the user's back.

Acceptance: the tests pass.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/AutomationWindowToolsTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min). Result, with `AutomationTests`: `Executed 38 tests, with 1 test skipped and 0 failures`, `** TEST SUCCEEDED **`.
Commit: c3cc4c6e

### Phase 2 — The exporter's name

- [x] `git mv schematical/schematical/Document/SchematicFileController.swift schematical/schematical/Document/SchematicExporter.swift`; rename the type in the app and the 8 test files; move `WindowReader` and `WindowCloseGuard` to `Document/WindowHelpers.swift`; `FocusedValues.schematicFile` becomes `schematicExporter`. Done: 10 Swift files changed by the rename (7 in the app, 3 in the tests: Corrections 3).

Acceptance: the name is gone and the A16 classes pass.
  Check: `rg -c 'SchematicFileController' schematical` → no output; `xcodebuild test … -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/SchematicExportActionTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/FileDesignRulesTests -only-testing:schematicalTests/PartEditorLibraryTests -only-testing:schematicalTests/UserLibraryEndToEndTests -only-testing:schematicalTests/UserLibraryUpdateTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 7 min). Result: `grep -rc 'SchematicFileController' schematical` → no file above 0; the 8 classes with `FabCheckFixesTests` and `PartEditorWindowTests`: `Executed 112 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: —

### Phase 3 — Docs and close-out

- [ ] `docs/file-format.md`: who reads and writes the file (`SchematicNSDocument`), autosave in place, load-file paths relative to the destination of each write and absolute for a document with no place yet.
- [ ] `docs/agent-control.md`: the window tools act on documents; `save` with a path is Save As; `undo` and `redo` act on the document.
- [ ] `docs/export.md`: the exporter's name; printing goes through the document.
- [ ] `git mv` the four `planning/plan-61-*.md` files to `planning/completed/`.

Acceptance: no doc names the controller.
  Check: `rg -c 'SchematicFileController' docs schematical` → no output (est. 1 min).
Commit: —

## Validation Plan

- Tests: `AutomationWindowToolsTests` (new); the 8 A16 classes.
- Coverage check: the two `rg -c` commands returning nothing.
- Runtime proof: plan-59 A13's evidence file.
- Doc sync: the three docs above.
- Final gate for the whole of plan 61 (once, after this phase): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/DocumentUndoTests -only-testing:schematicalTests/SchematicNSDocumentTests -only-testing:schematicalTests/AutomationWindowToolsTests -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/PCBUndoTests` plus the 8 A16 classes → `** TEST SUCCEEDED **` (est. 10 min; the whole Debug suite is not run because it stalls in `AutorouteModelTests testTheSummaryOnAFU`).

## Decisions

- The exporter keeps its own type, renamed `SchematicExporter` — decided by the author, 2026-11-24.

## Corrections

1. § 2 said `new_window` and `open` worked as 61-B left them. They needed a window to run in (`needsWindow`, from when a new tab was opened from an existing window), so with no document open both failed with `no schematic window is open (use new_window)`, `new_window` among them. Both are `needsWindow: false` now and take no `window` argument (one still passed is ignored). `open` of a file that is already open answered `opened (in window <the caller's>)`; it now names the window that holds the file.
2. `save` right after an edit left the document marked Edited (`testSaveNeedsAPathForAnUntitledDocumentAndWritesTheFile`, first run: `XCTAssertFalse failed` on `isDocumentEdited`): the document takes in an edit one run loop turn after it (plan-61-A Corrections 8), so the mark arrived after the save. The tool waits 50 ms before saving.
3. § 2 counted 8 test files constructing the controller. After 61-B phase 3 none constructs it (they build a `SchematicNSDocument` and use its `model.file`, plan-61-B Corrections 2), so the rename touched 3 test files that name the type (`FabCheckFixesTests`, `SchematicExportActionTests`, `SchematicNSDocumentTests`), not 8. § 2 also expected `updateOpenDocuments` and the custom-parts offer's trigger in the controller; `updateOpenDocuments` is `SchematicNSDocument`'s (same Corrections 2) and the offer is made by the document when its window is made.

## Summary

Low risk: tests for five tools, one rename, three docs.
