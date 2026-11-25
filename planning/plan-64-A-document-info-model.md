# plan-64-A: Document Info — model, undo, file

Last updated: 2026-11-25
Overall Effort: large (3h–1d)
Effort: medium (1h–2h)
Depends on: nothing

Plan-64 adds **Document Info**: a title, author, revision, created date, last-updated date and description kept
with each schematic, edited in its own window (⌘I) and the Schematic sidebar, and optionally drawn on the
schematic as a **title block** that moves like a text block. Turning the block off (the toggle, or Delete on it)
keeps the information.

This sub-plan builds the data and nothing visible: a `DocumentInfo` value on the document, its undo, the file
format, and the date stamping on save. B adds the window and menu, C the title block on the canvas, D the
exports, the agent tools and the landing checks.

| Sub-plan | Delivers |
|---|---|
| A (this) | `DocumentInfo` on the document, undo, file format, dates stamped on save |
| B | Document Info window, ⌘I, Window menu item, keyboard help |
| C | The title block: layout, drawing, selecting, moving, Delete = hide, sidebar panel |
| D | Title block in image/PDF/print/KiCad/EasyEDA exports, agent-control tools, docs, landing |

References:

- `docs/file-format.md` (§ top-level table, § Notes — the precedent for an optional, snapped, grid-placed item).
- `schematical/schematical/Canvas/UndoHistory.swift` (`DocumentState`, `UndoKey`, `UndoValue`, `UndoDiff`).
- `schematical/schematical/Document/SchematicFile.swift` (`Note`, `SchematicFile.init(document:fileURL:)`, `apply`).
- `schematical/schematical/Document/SchematicNSDocument.swift` (`snapshot()`, `data(ofType:)`).

## Prerequisites

These are a precondition on the whole feature (B, C and D point here).

| Must be true | Command | Status |
|---|---|---|
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p64-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-25 at fb27347c: `** BUILD SUCCEEDED **`) |
| No Document Info exists yet | `grep -rln "DocumentInfo" schematical/schematical` → no output | MET (2026-11-25: no output) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- A document carries a `DocumentInfo` (title, author, revision, description, created, updated, automatic-dates
  flag, and the title block's position when it is shown). Edits to it are undoable steps. It round-trips through
  the `.schematical` file. While **automatic dates** is on (the default), every save sets *Updated* to the time of
  the write and fills *Created* the first time it is empty; with it off, neither date is touched.

### Non-goals (explicit constraints)

- No window, menu item or sidebar (B, C). No drawing (C). No export (D).
- The file `version` stays 4 (5 when custom parts are embedded): the new key is optional, as `notes` was; an
  older build ignores it.
- A file written from a document whose info is still the default has **no** `documentInfo` key, so every existing
  fixture and golden file stays byte for byte what it is.
- `SchematicFile(document:fileURL:)` stays a pure function of the document: the date stamp happens in
  `SchematicNSDocument.snapshot()`, before the file is built.
- Opening a file never changes its dates.

## 2. Current State

- `SchematicDocument` (`schematical/schematical/Canvas/SchematicModel.swift`) keeps the saved model as properties
  (`notes` at :1682, `board`, …). `state` (:2160) copies them into `DocumentState`; `setState` (:2171) writes
  one back for undo/redo; `replaceContents(…)` (:2092) loads a file.
- Undo is a per-key diff (`UndoDiff.changes(from:to:)`, `UndoHistory.swift:100-130`); `board` is the precedent
  for a single-value key: `UndoKey.board`, `UndoValue.board(BoardSettings)`, a diff when `old.board != new.board`,
  and `case .board:` in `apply` (:196-197). Edits go through `recordEdit(_ page:, _ name:, coalescing:)`
  (:2198); typing merges with `coalescing: "note-text:\(id)"` (:3071).
- The file: optional arrays/objects on `SchematicFile` (`notes` at :593, `board`), encoded in
  `init(document:fileURL:)` (:826) and applied in `apply(to:catalog:fileURL:)` (:965, `replaceContents` at :1296);
  a bad value throws a `SchematicFileError` case (`badNote` at :674, message at :718). Positions snap on open
  (`Self.note(entry)`, :1337).
- A save runs `SchematicNSDocument.data(ofType:)` → `snapshot()` (`SchematicNSDocument.swift:136-148`), which calls
  `model.document.willSave()` and builds the `SchematicFile`. The class autosaves in place
  (`autosavesInPlace` → true, :34), so every write of the file, autosaves included, goes through `snapshot()`.
- The file has no title or author today; KiCad's `(title_block (title …))` is the file's base name
  (`KiCadSchematicWriter.swift:117`).

### Measured populations

| What | Count | Command |
|---|---|---|
| `DocumentState(` constructions in the app | 1 | `grep -rn "DocumentState(" schematical/schematical \| wc -l` → 1 |
| `DocumentState(` constructions in tests (all use labelled arguments, so a new defaulted field breaks none) | 15 | `grep -rn "DocumentState(" schematical/schematicalTests \| wc -l` → 15 |
| `replaceContents(` callers: app / tests (a new defaulted parameter breaks none) | 1 / 11 | `grep -rn "replaceContents(" schematical/schematical \| wc -l` → 2 (1 call + the definition); tests → 11 |

### Verified properties

- A new key on `UndoKey`/`UndoValue` needs edits in four places only (the enums, `UndoDiff.changes`, `apply`'s
  `switch key`): read in full, `UndoHistory.swift:28-230`; `switch key` has no `default`, so the compiler lists any
  site missed.
- `recordEdit` records nothing when the body changes nothing (doc comment, `SchematicModel.swift:2205-2209`), so
  re-typing the same value is not a step.
- Stamping the dates outside `recordEdit` does not make the document "edited": only `recordEdit` (through the
  undo manager) bumps the change count; `snapshot()` already mutates `openStep` through `willSave()` the same way.

## 3. Design Overview

One value, `DocumentInfo`, held by `SchematicDocument` like `board`. Everything about the feature (the six
fields, the dates' mode, whether and where the title block is shown) is in it, so the file, the undo step and the
copy in `DocumentState` are each one field.

```swift
nonisolated struct DocumentInfo: Equatable, Sendable {
    var title = "", author = "", revision = "", description = ""
    var created: Date?          // nil = not set yet
    var updated: Date?
    var automaticDates = true   // on: save stamps the dates and the fields are read-only in the UI
    var titleBlock: CGPoint?    // nil = not on the schematic; else the block's top-left, on the grid
}
```

- **Dates (decided by the owner):** one checkbox, *Set dates automatically*, default on. On, `stampDates(now:)`
  (run by `snapshot()`) sets `created = now` when `created` is nil and always sets `updated = now`; the fields
  show the stored values, read-only. Off, `stampDates` does nothing and the fields are editable. The stored dates
  are kept in the file in both modes.
- **Old files:** no key → `DocumentInfo()`; Created stays blank until the first save in this version stamps it.
- **A new, never-saved document** has blank dates until its first write (autosave or save).
- **Undo:** `UndoKey.documentInfo` / `UndoValue.documentInfo(DocumentInfo)`, one value diffed whole. The stamp
  is not an undo step, and an undo of an info edit restores that step's whole value including the dates; the next
  save stamps again. The cost is a harmless, transient older *Updated*.
- **Risk:** low. The correctness risk is the diff/apply symmetry for the new key and the byte-identity of files
  with default info; both have tests below.
- **Gate class:** behavior-changing for the file only when info is non-default; byte-identity of every default
  file is the neutral-path check (Phase 2).

Rejected alternatives:

- Separate keys per field in `UndoKey`: ten keys for one panel; the value is small and edited by one panel.
- Stamping inside `SchematicFile.init`: makes encoding impure, so "same schematic twice → same bytes" and every
  round-trip test would need a clock.
- A new file `version`: nothing else in the file changes shape; `notes` set the precedent for an optional key.

## 4. Detailed Design

**File shape** (`docs/file-format.md` gets a `## Document info` section and a top-level table row):

```json
"documentInfo": {
  "title": "Clock board", "author": "J. Smith", "revision": "B", "description": "…",
  "created": "2026-11-25T14:07:00Z", "updated": "2026-11-25T16:30:12Z",
  "automaticDates": true,
  "titleBlock": { "x": 400, "y": 520 }
}
```

- Every key optional on read; written: `title`, `author`, `revision`, `description` only when not empty;
  `created`/`updated` only when set (ISO 8601, UTC, whole seconds); `automaticDates` only as `false`;
  `titleBlock` only when shown. The whole key is left out when the info equals `DocumentInfo()`.
- On open: an unparsable date is `SchematicFileError.badDocumentInfo("created is not a date")`; `titleBlock`
  `x`/`y` snap to the grid (`SchematicGrid.snap`), as a note's do. Text values are kept as stored (no trimming).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. Use `- [~]` for
> partly done. Mark a moot task `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line when the phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The value, the model API, undo

Lands the data with nothing reading it, so it is safe alone.

- [ ] Add `schematical/schematical/Canvas/DocumentInfo.swift`: the struct above; `static let empty = DocumentInfo()`
      is not needed (use `DocumentInfo()`); `static func dateText(_ date: Date?, in zone: TimeZone = .current) -> String`
      (`"yyyy-MM-dd HH:mm"`, `en_US_POSIX`, "" for nil); `mutating func stamp(now: Date)` (the rule in § 3).
- [ ] `SchematicModel.swift`: `private(set) var documentInfo = DocumentInfo()` beside `notes`; add
      `documentInfo` to `state` (:2160) and `setState` (:2171); add `documentInfo: DocumentInfo = DocumentInfo()` to
      `replaceContents` (:2092) and assign it there.
- [ ] `SchematicModel.swift`, a `// MARK: Document info` section: `func editDocumentInfo(_ name: String,
      coalescing: String? = nil, _ change: (inout DocumentInfo) -> Void)` (one `recordEdit(.schematic, name,
      coalescing:)`, assigns only when the value changed); named setters the window will bind to: `setInfoTitle`,
      `setInfoAuthor`, `setInfoRevision`, `setInfoDescription` (coalescing key `"info-<field>"`, names "Edit Title",
      "Edit Author", "Edit Revision", "Edit Description"), `setInfoCreated(_ Date?)`, `setInfoUpdated(_ Date?)`
      (names "Edit Created Date", "Edit Updated Date"), `setAutomaticDates(_ Bool)` ("Change Automatic Dates");
      and `func stampDates(now: Date = Date())` — assigns `documentInfo.stamp(now:)` directly, outside `recordEdit`.
- [ ] `UndoHistory.swift`: `DocumentState.documentInfo = DocumentInfo()`; `UndoKey.documentInfo`;
      `UndoValue.documentInfo(DocumentInfo)`; `UndoDiff.changes` adds the key when `old.documentInfo !=
      new.documentInfo`; `apply` has `case .documentInfo: if case let .documentInfo(i)? = value { state.documentInfo = i }`.
- [ ] Tests: new `schematicalTests/DocumentInfoTests.swift` (`@MainActor final class DocumentInfoTests: XCTestCase`,
      `TestSupport.newDocument()`): defaults; each setter records one step with its name; typing in one field
      coalesces into one step, two fields do not; undo then redo restores the whole value; setting the same value
      records no step; `stampDates` with automatic on sets `created` once and `updated` every call; with automatic
      off changes nothing; `stampDates` adds no undo step (`undoName` unchanged); `dateText` for a fixed `Date` and a
      fixed `TimeZone(identifier: "UTC")`. Extend `UndoHistoryTests.swift` with a diff/apply round-trip for the key.

Acceptance: `DocumentInfoTests` and `UndoHistoryTests` pass; the app builds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p64-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/DocumentInfoTests -only-testing:schematicalTests/UndoHistoryTests` → `** TEST SUCCEEDED **` (est. 8 min: incremental app build 5, two test classes 3).
Commit: —

### Phase 2 — The file, and the stamp on save

- [ ] `SchematicFile.swift`: `struct DocumentInfoEntry: Codable, Equatable, Sendable` (shape in § 4, all fields
      optional, `titleBlock: Point?` using the existing `SchematicFile.Point` (`x`, `y` doubles, :65)) and `var documentInfo: DocumentInfoEntry? = nil` on `SchematicFile`, with its doc comment.
- [ ] `SchematicFile.init(document:fileURL:)` (:826): `documentInfo = document.documentInfo == DocumentInfo() ? nil :
      DocumentInfoEntry(document.documentInfo)`. Dates written with `ISO8601DateFormatter` (`.withInternetDateTime`,
      UTC, no fractional seconds).
- [ ] `SchematicFile.apply` (:1296): decode into a `DocumentInfo` (`Self.documentInfo(entry)` beside `Self.note`),
      snapping `titleBlock`; pass `documentInfo:` to `replaceContents`. Add `SchematicFileError.badDocumentInfo(String)`
      with its message beside `badNote` (:674, :718).
- [ ] `SchematicNSDocument.swift` `snapshot()` (:146): call `model.document.stampDates()` before `SchematicFile(…)`.
- [ ] `docs/file-format.md`: a `documentInfo` row in the top-level table (after `labels`), a `## Document info`
      section (shape, field table, "optional; an older build ignores it; the version stays 4", the automatic-dates
      rule, "written only when not the default"), and the bad-value rule in the validation list (near :653).
- [ ] Tests (`DocumentInfoTests`): a default document encodes with no `documentInfo` key (`String(data:)` does
      not contain it); a full info round-trips through `SchematicFile.encoded()` / `decode` / `apply` (all six
      fields, `automaticDates` false, `titleBlock`); `titleBlock` (104, 57) opens as (100, 60); a bad `created` throws
      `badDocumentInfo`; a file with no key opens as `DocumentInfo()`. In `SchematicNSDocumentTests.swift`: writing a
      document with `data(ofType:)` stamps `created` and `updated` (both non-nil) and a second write moves `updated`
      only; with automatic dates off neither moves.

Acceptance: a default document's saved bytes are unchanged; a stamped document round-trips.
  Check: `xcodebuild … test -only-testing:schematicalTests/DocumentInfoTests -only-testing:schematicalTests/SchematicNSDocumentTests -only-testing:schematicalTests/SchematicFileV5Tests` → `** TEST SUCCEEDED **` (est. 10 min; `SchematicFileV5Tests` (j) is the existing save → open → save same-bytes check, so it catches a default file that changed).
Commit: —

## Validation Plan

- Tests: `DocumentInfoTests` (new), `UndoHistoryTests`, `SchematicNSDocumentTests`, `SchematicFileV5Tests` as above.
- Coverage check: both folders are synchronized root groups of the Xcode project
  (`grep -c PBXFileSystemSynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj` → 5, 3 groups
  between their Begin/End markers), so `DocumentInfo.swift` and `DocumentInfoTests.swift` join the app and test
  targets without a project edit; a test that is not in the run's output did not run.
- Runtime proof: none beyond the tests; the first visible behavior is B.
- Doc sync: `docs/file-format.md` (Phase 2).
- Final gate: runs once in plan-64-D.

## Decisions

- Dates are controlled by one **Set dates automatically** checkbox, default on; on = stamped on save and the
  fields disabled, off = the fields are editable; the dates are stored in the file either way — decided by the
  owner, 2026-11-25.
- A file saved before this feature shows a blank Created until its first save in this version — decided by the
  owner, 2026-11-25.
- Revision is free text — decided by the owner, 2026-11-25.
- `Updated` is stamped on every write of the file, autosave included; `Created` on the first write after it is
  empty — author's call (autosave in place goes through the same `snapshot()`).
- Dates are written as ISO 8601 UTC and shown as `yyyy-MM-dd HH:mm` in the viewer's time zone — author's call
  (a fixed format, so exports and tests do not depend on the locale).

## Corrections

(Filled in during execution.)

## Summary

The risk is small: one value, one undo key, one optional file key. The file format, existing fixtures and the
version number are untouched for any document that has no info.
