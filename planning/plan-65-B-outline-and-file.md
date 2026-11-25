# plan-65-B: Free-form Board Outline and the File Format

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-A (complete)

plan-65-A made cutouts a thing the document holds. This sub-plan makes them save and load, and gives the board an edge that is not a rectangle or a
card: `BoardSettings.customOutline`, a simple polygon. The DXF import (plan-65-I) is the only thing that makes one for a person; this sub-plan makes
the state, its undo steps, its file keys and their validation.

The single outcome: a document with cutouts and a free-form outline saves and opens with the same cutouts and the same outline (ids, shapes,
locks); a hand-edited file with a bad one is refused with a message that says which key and why; a document with neither saves exactly the bytes it does today.

References:

- plan-65-A § 3 and § 4 (the types and the terms)
- `schematical/schematical/Canvas/BoardSettings.swift` (`BoardSettings` at `:271`, `outline` at `:340`, `card` at `:393`), `Boards/BoardCard.swift:7`
  (`BoardType`), `Canvas/SchematicModel.swift` (`setBoardSize` at `:3267`, `setBoardType` at `:3318`, `replaceContents` at `:2092`)
- `Document/SchematicFile.swift`: `Board` (`:158`), the encode of `board` (`:836`–`:884`), `boardSettings()` (`:1381`), the keep-out reader (`:1275`, `:1667`)
- `docs/file-format.md` § Board (`:167`), § Keep-outs (`:311`), § Validation on open (`:644`), § Older versions (`:794`)
- `schematicalTests/PourFileTests.swift` (the test this one mirrors)

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-A is complete | `ls planning/completed \| grep -c '^plan-65-A'` → 1 | NOT MET (2026-11-25: A is written, not landed) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `BoardSettings.customOutline: [CGPoint]?` exists; `BoardSettings.outline` returns it while `type == .custom`; width and height are its bounds.
- `setBoardOutline(_:)` and `resetBoardOutline()` are each one named undo step; `setBoardSize` leaves a free-form board's size alone; `setBoardType` to anything but Custom drops the outline.
- `board.outline` and `board.cutouts` are written only when there is something to write, read back exactly, and validated (§ 4.3).
- `docs/file-format.md` documents both keys, the validation messages and what an older build does with them.

### Non-goals (explicit constraints)

- No reader of `boardEdge` changes (C, D, G); no drawing or sidebar (E, F); nothing makes a free-form outline for a person yet (I).
- No change to a file without the two keys: `SchematicFile` encodes and decodes it byte for byte as before.
- No `version` bump: the keys are optional and additive like `pours`; an older build ignores them (it shows the board as its width × height rectangle, with no cutouts, and the next save from that build drops them; § 5 records this in the doc).
- No rounding of the rectangle's own corners (the todo's other half); a rounded-corner board arrives as a DXF outline.

## 2. Current State

- `BoardSettings.outline` is `card?.outline ?? [rectangle from width, height]` (`BoardSettings.swift:340`); `card` is nil for `.custom` (`:393`).
- `setBoardSize` (`SchematicModel.swift:3267`) clamps width and height to `sizeRange` (1…1000) for a custom board; the PCB sidebar's Width and Height fields call it (`PCBSidebarView.swift:447`, `:453`).
- `setBoardType` (`:3318`) switches `board.type`, and for a card sets width and height from the standard.
- `SchematicFile.Board` has one optional array per item kind (`holes`, `pours`, `keepouts`, `:186`–`:194`) written only when non-empty (`:871`–`:884`); `boardSettings()` reads the scalars and `claim(id)` keeps ids unique across the file (`:1275`–`:1280`).
- Errors are `SchematicFileError.badPCB("<what> <detail>")` (`:1388`, `:1681`); the doc lists each message (`docs/file-format.md:725`–`:737`).

### Measured populations

| What | Count | Command |
|---|---|---|
| places that assign `board.width` or `board.height` in the model (each must leave a free-form board's bounds alone) | 14 lines | `rg -n "board\.(width\|height) =" schematical/schematical/Canvas/SchematicModel.swift \| wc -l` → 14 (all in `setBoardSize`, `setCardOptions`, `setCustomCard`, `setBoardType`) |
| existing test classes that save and open a board (the byte-identity check below uses three) | `PourFileTests`, `SchematicFileV5Tests`, `FileDesignRulesTests`, `HoleTests` | `ls schematical/schematicalTests \| grep -E "^(PourFileTests\|SchematicFileV5Tests\|FileDesignRulesTests\|HoleTests)"` → all four listed (2026-11-25) |

### Verified properties

- A file with neither key decodes and re-encodes unchanged by construction: `Board` is `Codable` with optional keys that encode only when non-nil (`SchematicFile.swift:158`–`:230`), the same mechanism `keepouts` uses.
- `replaceContents(…)` is what opening a file calls (`SchematicFile.swift:1303`–`:1320`); adding `cutouts:` there (A) is the only reader-side hook the model needs.
- The routine "do two segments cross" exists: `CopperGeometry.segmentsCross(_:_:_:_:)` (`Routing/CopperGeometry.swift:116`), a proper crossing (two segments that only touch at an end are not a crossing). plan-65-A's `BoardGeometry.crossesItself(_ points:)` calls it for every pair of non-adjacent edges, and `BoardCutout.problem` and `BoardSettings.problem(outline:)` both call that.

## 3. Design Overview

`customOutline` lives in `BoardSettings` (not beside the cutouts) because it *is* the board's shape: it is undone with the board (`UndoKey.board`),
read by every reader of `outline`, and replaces the width × height rectangle. Cutouts live in the document like holes.

Where the risk concentrates: width and height must stay equal to the outline's bounds, or `PrintExport.boardBounds`, the sidebar and the DRC disagree. Three writers can break
it (`setBoardSize`, `setBoardType`, the file reader); each is covered by a test that sets a free-form outline and then calls it.

Rejected: keeping the outline in a separate document array (every `BoardSettings` consumer would have to be told); keeping `customOutline` when the type changes to a card
(width and height would then disagree with it when the type returns to Custom, and a card's size is the standard's).

## 4. Detailed Design

### 4.1 `BoardSettings`

- `var customOutline: [CGPoint]? = nil`: clockwise, y down, at least 3 points, simple, bounds' top-left (0, 0), bounds within `sizeRange`.
- `var isFreeForm: Bool { type == .custom && customOutline != nil }`.
- `outline`: `card?.outline ?? (type == .custom ? customOutline : nil) ?? rectangle`.
- `static func problem(outline: [CGPoint]) -> String?`: `needs at least 3 points`, `crosses itself`, `must start at (0, 0): its top-left is (x, y)` (within 1e-6), `is W mm wide; sizes are 1 to 1000 mm` (likewise `high`). Returns the detail after "board outline ".

### 4.2 The model (`SchematicModel.swift`)

- `@discardableResult func setBoardOutline(_ points: [CGPoint]) -> Bool`, step `Change Board Outline`: refuses (false, no step) when `problem` is non-nil; else `board.type = .custom`, `board.customOutline = BoardGeometry.clockwise(points)`,
  `board.width/height` = the bounds' size, and `board.edgeBus` unchanged (a bus on a custom board is already dropped by `edgeBusID`). Cutouts, parts and everything else do not move.
- `func resetBoardOutline()`, step `Reset Board Outline`: `customOutline = nil`, width and height kept (the rectangle is the old bounds).
- `setBoardSize`: returns at once when `board.isFreeForm`. `setBoardType(_:)`: when the new type is not `.custom`, `customOutline = nil` (inside the same step).
- `setCardOptions`, `setCustomCard`: unchanged (they act only on a card type, where `customOutline` is already nil).

### 4.3 The file

`SchematicFile.Board` gains `outline: [Point]?` (written only when `settings.isFreeForm`) and `cutouts: [CutoutEntry]?` (written only when `document.cutouts` is non-empty), after `keepouts`:

```json
"outline": [ {"x": 0, "y": 0}, {"x": 60, "y": 0}, {"x": 60, "y": 40}, {"x": 0, "y": 40} ],
"cutouts": [ { "id": "…", "shape": "rounded", "x": 10, "y": 8, "width": 6, "height": 2, "radius": 1 },
             { "id": "…", "shape": "polygon", "points": [ {"x": 30, "y": 10}, {"x": 34, "y": 10}, {"x": 32, "y": 14} ], "locked": true } ]
```

`CutoutEntry`: `id: UUID`, `shape: String`, `x, y, width, height, radius: Double?` (rounded), `points: [Point]?` (polygon), `locked: Bool?` (only when true).

Reading (`boardSettings()` and a `boardCutout(_:)` next to `boardKeepout`; ids claimed with `claim(id)` like every item):

| Error (`SchematicFileError.badPCB`) | When |
|---|---|
| `board outline needs the board type custom` | `outline` present and `type` is not custom |
| `board outline needs at least 3 points` / `board outline crosses itself` / `board outline must start at (0, 0): its top-left is (x, y)` / `board outline is W mm wide; sizes are 1 to 1000 mm` | `BoardSettings.problem(outline:)` |
| `cutout <id> shape "x" must be rounded or polygon` | any other `shape` |
| `cutout <id> width 0.1 must be at least 0.5 mm` (same for `height`); `cutout <id> radius 5 must be 0 to 2 mm` | `BoardCutout.problem` (rounded; `radius` missing is 0; `x`, `y`, `width`, `height` missing is an error `cutout <id> needs x, y, width and height`) |
| `cutout <id> needs at least 3 points` / `cutout <id> crosses itself` / `cutout <id> needs points` | polygon |

A polygon cutout read from the file is stored clockwise (`BoardGeometry.clockwise`). The outline wins over `width` and `height` in the file: they are set to its bounds. A cutout outside the outline, or over another, is not a file error (the design rule check names it, C).

## 5. Compatibility / Format Impact

Two new optional keys under `board`. Written only when used. An older build ignores both: it draws the width × height rectangle and no cutouts, and its next save drops them. `docs/file-format.md` § Older versions says so.
Unchanged: every existing key, `version`, and the bytes of any document without the two keys.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — The free-form outline in the model

- [ ] `Canvas/BoardSettings.swift`: `customOutline`, `isFreeForm`, the new `outline`, `problem(outline:)`. `BoardSettings.problem(outline:)` calls A's `BoardGeometry.crossesItself`.
- [ ] `Canvas/SchematicModel.swift`: `setBoardOutline`, `resetBoardOutline`; the early return in `setBoardSize`; the drop in `setBoardType`.
- [ ] Tests `schematicalTests/FreeFormOutlineTests.swift`: `outline` is the polygon while Custom and the rectangle after `resetBoardOutline`; width and height equal the bounds after `setBoardOutline` with a counter-clockwise L-shape starting at (0, 0) (stored clockwise by `BoardGeometry.clockwise`, A); `setBoardOutline` refuses a bow-tie, a 2-point list, an outline whose top-left is (5, 0) and one 1200 mm wide, each leaving no undo step; `setBoardSize` on a free-form board changes nothing; `setBoardType(.customCard)` drops the outline and a later `.custom` is a rectangle; undo and redo of `setBoardOutline` restore type, outline, width and height exactly; cutouts and part positions are unchanged by `setBoardOutline`.

Acceptance: a free-form outline is set, reset and undone exactly, and every reader of `BoardSettings.outline` sees it (asserted through `BoardEdge`).
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/FreeFormOutlineTests -only-testing:schematicalTests/BoardCutoutTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 2 — The file

- [ ] `Document/SchematicFile.swift`: `Board.outline`, `Board.cutouts`, `Board.CutoutEntry`; the encode of both (only when used); `boardSettings()` reads and validates `outline`; a `boardCutout(_:)` reader beside `boardKeepout`; `claim(entry.id)` for cutouts; `replaceContents(… cutouts:)`.
- [ ] `docs/file-format.md`: rows for `outline` and `cutouts` in the Board table (`:167`–`:200`); a `### Cutouts` section after `### Keep-outs` (`:311`) with the JSON above, the shapes, ranges and "bounds start at (0, 0)"; the error list (`:725`–`:737`) gains every message in § 4.3; § Older versions (`:794`) gets the paragraph in § 5.
- [ ] Tests `schematicalTests/CutoutFileTests.swift`: a document with a rounded and a locked polygon cutout and a free-form outline saves and opens to an equal `cutouts` array and `board.outline`; a document with neither writes JSON containing neither key (`XCTAssertFalse(json.contains("\"cutouts\""))` and `"outline"`); each row of the error table fails with exactly that message (ids filled in); `width`/`height` in the file are overridden by the outline's bounds; two cutouts with one id are refused (the existing duplicate-id message).

Acceptance: round trip is exact; every error message in the table is produced; documents without the keys are unchanged.
  Check: `XB -only-testing:schematicalTests/CutoutFileTests -only-testing:schematicalTests/PourFileTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/FileDesignRulesTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 3 min; the three existing classes are the check that files without the keys are unchanged).
Commit: —

## Validation Plan

- Tests: `FreeFormOutlineTests`, `CutoutFileTests`, with the negative cases listed.
- Coverage check: both new classes are in the test target; the Phase 2 check also runs `PourFileTests`, `SchematicFileV5Tests` and `FileDesignRulesTests`, which exercise the same `Board` encode and `boardSettings()`.
- Runtime proof: none beyond the tests (no UI before E).
- Doc sync: `docs/file-format.md` (Board table, Cutouts, validation list, Older versions).
- Final gate (run ONCE, after Phase 2): `-only-testing:schematicalTests/FreeFormOutlineTests -only-testing:schematicalTests/CutoutFileTests -only-testing:schematicalTests/CutoutModelTests -only-testing:schematicalTests/PourFileTests -only-testing:schematicalTests/BoardCatalogTests -only-testing:schematicalTests/BoardCardTests` → `** TEST SUCCEEDED **` (est. 4 min; the last two cover `setBoardType`'s card paths).

## Decisions

- DXF import has an "outline plus cutouts" mode, so the board needs a free-form outline — decided by the owner, 2026-11-25. Settles § 3 and § 4.1.
- A free-form board is the Custom type with an outline; leaving Custom drops it; its width and height are read-only — decided by the author (no user-visible fork beyond the above), 2026-11-25. Settles § 4.2.

## Corrections

<Filled in during execution.>

## Summary

Risk: width and height drifting from the outline's bounds; three writers and a test each. Left untouched: every export, the pour fill, the DRC, the router (they already read `outline`, so a free-form outline reaches them; C, D and G make them read the cutouts).
