# plan-50-A: Board Stackup Model, File and Defaults

Last updated: 2026-11-16
Overall Effort: x-large (1d–3d)
Effort: medium (1h–2h)
Depends on: nothing

The PCB has no stackup: no copper weights, dielectric heights or surface finish, so nothing can be said about
impedance or current, and the KiCad and Gerber exports carry no thickness. plan-50 (A–F) adds a stackup to the
board, a vias-can-be-open option, richer Gerber X2 output and impedance / current guidance. This sub-plan lays
the data underneath all of it: a `BoardStackup` on `BoardSettings`, with a per-layer-count default taken from a
real fab's published stackup, saved in the file, undoable, and kept in step with the layer count.

The single outcome: a new board has a standard stackup for its layer count; the stackup survives save, open
and undo; changing the layer count gives the new count's standard stackup; and a file with no stackup opens with
the standard one. No screen shows it yet (plan-50-B).

The whole plan-50, in letter order (each is one sitting):

| Plan | Delivers |
|---|---|
| A (this) | `BoardStackup` model, file `board.stackup`, defaults, undoable setters |
| B | Stackup section in the PCB sidebar; the finish becomes the board's (Board Preview and Image export follow it) |
| C | Per-via tenting (Top / Bottom): file, sidebar, preview, Gerber mask, KiCad via |
| D | Gerber X2 aperture and net attributes on copper, project attributes, shared net names |
| E | Drill-tool functions, the `.gbrjob` job file, the KiCad stackup |
| F | Impedance and current estimates from the stackup, readouts, and the Calculate Width sheet |

References:

- `docs/file-format.md` § `board` (the table rows `layerStyles` at line 161 to `vias` at line 167, and § Design rules), `docs/boards-format.md`
- `planning/completed/plan-49-per-file-design-rules.md` — the precedent this plan mirrors (a per-file board setting, copied up front, undoable, byte-golden tests stripped of the new key)
- `planning/plan-50-data/README.md` — the measurements below come from it
- `schematical/schematical/Canvas/BoardSettings.swift`, `Document/SchematicFile.swift`, `Canvas/SchematicModel.swift`

## Prerequisites

These are a precondition on the whole of plan-50, stated once here.

| Must be true | Command | Status |
|---|---|---|
| plan number 50 is this plan's alone | `ls planning planning/completed \| grep '^plan-50' \| grep -v -E '^plan-50-([A-F]-\|data)'` → no output | MET (2026-11-16) |
| Xcode builds and tests the app | `xcodebuild -version` → `Xcode 26.3` or later | MET (2026-11-16: `Xcode 26.3`) |
| KiCad 10's CLI and python are installed (plans C, D and E check against them) | `ls ~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli` → the path | MET (2026-11-16) |
| a Python venv with `gerbonara==1.6.3` and `scikit-rf==2.2.0` (plans D, E and F check against them) | `python3 -m venv $VENV && $VENV/bin/pip install gerbonara==1.6.3 scikit-rf==2.2.0 && $VENV/bin/python -c "import gerbonara, skrf"` → no error. Creating the venv is part of meeting this row; it needs the network once. | MET (2026-11-16, in the session scratchpad; recreate in a new `$VENV`) |

Everything below is written against the world where these hold. `$DD` in the checks is a scratch derived-data
folder (any empty directory); `$VENV` the venv above.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to stop.
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `BoardSettings.stackup: BoardStackup` exists. `BoardStackup.standard(layers:)` is the default for 1, 2, 4 and
  6 layers (§ 3). `BoardSettings.layers` and `stackup` always agree: one copper thickness per layer, one
  dielectric between each two copper layers (one for a 1-layer board).
- The file saves `board.stackup` always and reads it back; a file without it opens with the standard stackup.
- Changing the layer count (`setLayerCount`, or a board type that needs 2 layers) replaces the copper and
  dielectrics with the new count's standard ones and keeps the finish and temperature rise, as one undo step.
- Five edits are undoable one-step actions: finish, temperature rise, one copper thickness, one dielectric
  (kind, thickness or Er), and reset to the standard stackup.

### Non-goals (explicit constraints)

- The file version does not change. `stackup` is an optional key; older builds ignore it (synthesized
  `Codable` drops unknown keys, as for `designRules`, plan-49 § 2).
- No screen changes (plan-50-B), no export changes (C, D), no calculators (E).
- `board.color` (solder mask colour) is unchanged and stays outside the stackup.
- The 100 mil trace-width limit (`BoardTrace.widthRange`) is unchanged.
- Settings ▸ DRC Defaults and `DesignRules` are untouched.

## 2. Current State

- `BoardSettings` (`Canvas/BoardSettings.swift:271`) holds `layers` (1, 2, 4 or 6 — `layerCounts`), `color`,
  `rules` (plan-49) and the rest of the board; it is `nonisolated`, `Hashable`, `Sendable`.
- Undo: `UndoHistory` diffs the whole `board` (`old.board != new.board`, `Canvas/UndoHistory.swift:134`), so
  anything inside `BoardSettings` is undoable with no other bookkeeping. Edits go through
  `SchematicDocument.recordEdit(.pcb, "<name>") { … }` (`Canvas/SchematicModel.swift:1741`).
- The layer count is assigned in 3 places: `SchematicFile.swift:1254` (reader), `SchematicModel.swift:2733`
  (`setBoardType` bumps a card to 2 layers) and `:2790` (`setLayerCount`).
- File: `SchematicFile.Board` is a `Codable` struct (`Document/SchematicFile.swift:150`); `Board(…)` is built at
  `:760`–`:813` and read by `boardSettings()` (`:1237`), which starts from `BoardSettings()` and throws
  `SchematicFileError.badPCB(<detail>)` for a bad value. `designRules` (plan-49) is the precedent for a board
  setting written always and defaulted when missing.
- Three byte-golden tests compare a saved file with a golden after `TestSupport.withoutDesignRules` strips the
  `designRules` object: `SchematicFileV5Tests.swift:158`, `NetClassTests.swift:265`, `NetRulesTests.swift:194`.

### Measured populations

| What | Count | Command |
|---|---|---|
| assignments of the layer count | 3 | `grep -rn --include='*.swift' "\.layers = " schematical/schematical` → `SchematicFile.swift:1254`, `SchematicModel.swift:2733`, `:2790` |
| constructions of `BoardSettings()` | 27 | `grep -rn --include='*.swift' "BoardSettings()" schematical/schematical schematical/schematicalTests \| wc -l` → 27 (all get `stackup` by default) |
| byte-golden saved-file tests | 3 | `grep -rn "withoutDesignRules" schematical/schematicalTests \| grep -v "static func"` → 3 lines |

### Verified properties

- **KiCad's total thickness includes both solder masks.** KiCad 10.0.8's `.gbrjob` for a 4-layer board with
  Cu 0.035 / 0.0152 / 0.0152 / 0.035, dielectrics 0.2104 / 1.065 / 0.2104 and two 0.01 masks gives
  `BoardThickness 1.6062` (`planning/plan-50-data/README.md`). The model's `thickness` uses the same sum.
- **A published fab stackup exists for 4 and 6 layers** (JLCPCB, read 2026-11-16, in the README); 1- and 2-layer
  use the 2-layer εr 4.5 JLCPCB states and KiCad's own default 1.51 mm core.
- **Older builds open a file with `stackup`** by the same synthesized-`Codable` argument as `designRules`
  (`SchematicFile.Board` has no custom `init(from:)`: `grep -n "init(from" schematical/schematical/Document/SchematicFile.swift` → 2
  lines, `:414` in `End` and `:536` in `JSONValue`, neither `Board`).

## 3. Design Overview

```swift
nonisolated enum SurfaceFinish: String, CaseIterable, Hashable, Sendable {
    case hasl, leadFreeHasl, enig, osp          // titles "HASL (leaded)", "HASL (lead-free)", "ENIG", "OSP"
    var kiCadName: String                        // "HAL SnPb", "HAL lead-free", "ENIG", "OSP"
}
nonisolated enum DielectricKind: String, CaseIterable, Hashable, Sendable { case core, prepreg }
nonisolated struct Dielectric: Hashable, Sendable { var kind: DielectricKind; var thickness: Double; var er: Double }  // mm
nonisolated struct BoardStackup: Hashable, Sendable {
    var copper: [Double]            // mm, one per copper layer, Top first, Bottom last
    var dielectrics: [Dielectric]   // between copper[i] and copper[i+1]; one for a 1-layer board
    var finish: SurfaceFinish
    var temperatureRise: Double     // °C, for current guidance (plan-50-E)
    static let maskThickness = 0.01
    var thickness: Double { copper.reduce(0, +) + dielectrics.reduce(0) { $0 + $1.thickness }
                            + (copper.count == 1 ? 1 : 2) * Self.maskThickness }
    static func standard(layers: Int, finish: SurfaceFinish? = nil, temperatureRise: Double = 10) -> BoardStackup
    static func problem(…) -> String?   // the file reader's refusals
}
```

Ranges: copper 0.005…0.2 mm, dielectric thickness 0.02…5 mm, Er 1…20, temperature rise 1…100 °C.

**The standard stackups** (mm; the `standard(layers:)` table):

| Layers | Copper | Dielectrics (kind, thickness, εr) | Finish | `thickness` |
|---|---|---|---|---|
| 1 | 0.035 | core 1.555 / 4.5 | lead-free HASL | 1.6 |
| 2 | 0.035, 0.035 | core 1.51 / 4.5 | lead-free HASL | 1.6 |
| 4 | 0.035, 0.0152, 0.0152, 0.035 | prepreg 0.2104 / 4.4, core 1.065 / 4.6, prepreg 0.2104 / 4.4 | lead-free HASL | 1.6062 |
| 6 | 0.035, 0.0152 ×4, 0.035 | prepreg 0.0994 / 4.1, core 0.55 / 4.6, prepreg 0.1164 / 4.16, core 0.55 / 4.6, prepreg 0.0994 / 4.1 | ENIG | 1.566 |

The 4- and 6-layer rows are JLCPCB's published JLC04161H-7628 and JLC06161H-3313 stackups; the 6-layer finish is
ENIG because JLCPCB offers no HASL at 6 layers or more.

**One invariant point.** `BoardSettings.layers` gets a `didSet`: when `stackup.copper.count != layers` the
stackup becomes `.standard(layers: layers, finish: stackup.finish, temperatureRise: stackup.temperatureRise)`.
So all three assignment sites keep the invariant with no other change, and `setLayerCount` stays one undo step.
The file reader assigns `layers` first and `stackup` after, so a stored stackup wins.

**File.** `board.stackup` is written always:

```json
"stackup": { "copper": [0.035, 0.035],
             "dielectrics": [ { "type": "core", "thickness": 1.51, "er": 4.5 } ],
             "finish": "leadFreeHasl", "temperatureRise": 10 }
```

A missing `stackup`, or a missing key inside it, takes the standard value for the layer count. A wrong number of
`copper` or `dielectrics` entries, an unknown `type` or `finish`, or a value outside its range is
`badPCB("board stackup <detail>")` with the detail naming the key and the allowed range, like the via errors.

**Risk.** Low overall; it concentrates in the three byte-golden tests, which now see a new key. They are fixed
the way plan-49 fixed them: a `TestSupport.withoutStackup` helper strips the object, so the goldens keep their
meaning. Gate: tests, not byte-identity (saved files legitimately gain a key).

**Rejected.** *Writing the stackup only when it differs from the standard*: a later change to the defaults
would silently change old files (plan-49 made the same call for rules). *A `didSet` per setter*: one invariant
point is smaller. *Per-manufacturer stackup presets*: PCBWay's page is 404 and a menu of one fab is not useful.
*Mask thickness as a setting*: a constant 0.01 mm, as KiCad's default.

## Compatibility / Format Impact

- New optional key `board.stackup` (above), documented in `docs/file-format.md`. An older build opens the file
  and drops the key on its next save.
- Every file saved by this build gains the key, so saved-file goldens change by exactly that object.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. `- [~]` for
> partial, with one line on what remains. A moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's
> `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — The model

- [x] New `schematical/schematical/Canvas/BoardStackup.swift`: `SurfaceFinish`, `DielectricKind`, `Dielectric`,
      `BoardStackup` (§ 3), `standard(layers:)` with the table above, the ranges, and
      `problem(copper:dielectrics:finish:temperatureRise:layers:) -> String?` returning the refusal text or nil.
- [x] `Canvas/BoardSettings.swift`: `var stackup = BoardStackup.standard(layers: 2)` on `BoardSettings`, and the
      `didSet` on `layers` (§ 3).
- [x] Tests: new `schematicalTests/BoardStackupTests.swift`: the four standard stackups' `thickness` equal
      1.6, 1.6, 1.6062 and 1.566 (to 1e-9); `standard(layers:)` keeps a given finish and temperature rise;
      `BoardSettings.layers = 4` on a fresh board makes `stackup.copper.count == 4` and
      `dielectrics.count == 3`; `layers = 1` gives one dielectric; `problem` refuses a wrong count, a
      thickness of 0.001, an Er of 0.5 and a temperature rise of 0.

Acceptance: the model and its invariant behave as above.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/BoardStackupTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min, mostly the build). Observed: `Executed 5 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: —

### Phase 2 — The file

- [ ] `Document/SchematicFile.swift`: `Board.StackupEntry` (`copper: [Double]?`, `dielectrics: [DielectricEntry]?`,
      `finish: String?`, `temperatureRise: Double?`), written in `Board(…)` (always) and read in
      `boardSettings()` after `layers`, with the refusals in § 3.
- [ ] `docs/file-format.md`: the `stackup` row in the `board` table and a § Stackup (keys, ranges, defaults,
      the four standard stackups' numbers, errors).
- [ ] `TestSupport.swift`: `withoutStackup(_:)`, a regex `(?s)\n    "stackup" : \{.*?\n    \},?` removal that fails
      when the object is absent (as `withoutDesignRules` does); apply it in the 3 golden tests (the sites in
      § 2).
- [ ] Tests, in `BoardStackupTests`: a saved file has `board.stackup` and opens to an equal `BoardStackup`; a file
      with no `stackup` opens to `standard(layers:)` for its count; a 4-layer file whose `copper` has 2 entries
      throws `badPCB` naming `copper`; an unknown `finish` throws; an edited stackup (copper 0.07, Er 4.0, finish
      ENIG, rise 20) round-trips.

Acceptance: round trip and refusals as above; the 3 goldens still pass with the strip.
  Check: `xcodebuild … test -only-testing:schematicalTests/BoardStackupTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/NetRulesTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 3 — Edits and undo

- [ ] `Canvas/SchematicModel.swift`, in the Board section next to `setLayerCount`: `setStackupFinish(_:)`,
      `setStackupTemperatureRise(_:)` (clamped to its range), `setCopperThickness(_ mm: Double, at index: Int)`,
      `setDielectric(at index: Int, kind:thickness:er:)` (each argument optional; clamped), `resetStackup()` (the
      standard for the board's layers, keeping nothing but the layer count). Each is one
      `recordEdit(.pcb, "Change Stackup")` (or "Reset Stackup") that changes only when the value differs, and
      ignores an out-of-range index.
- [ ] Tests, in `BoardStackupTests`: each setter changes the value and `doc.undo(on: .pcb)` restores it, one step
      each, named as in the setters (`doc.undoName(on: .pcb)`); a setter with the same value records no step
      (`undoName(on: .pcb)` unchanged); `setLayerCount(4)` then `undo(on: .pcb)` returns the 2-layer stackup; clamping at both ends of each range; `setBoardType` of a card on a 1-layer
      board leaves a 2-layer stackup.

Acceptance: every edit is one undo step; the layer-count change round-trips through undo.
  Check: `xcodebuild … test -only-testing:schematicalTests/BoardStackupTests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/BoardLayersTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

## Validation Plan

- Tests: `BoardStackupTests` (new), plus the existing file, layer and undo suites named in the phases.
- Coverage check: the three goldens fail without `withoutStackup` (run once with the helper's call removed
  from `SchematicFileV5Tests`; the assertion shows the extra `stackup` object), proving the key really is saved.
- Doc sync: `docs/file-format.md` (Phase 2).
- Final gate (once, after Phase 3): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/BoardStackupTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/NetRulesTests -only-testing:schematicalTests/FileDesignRulesTests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/UndoHistoryTests -only-testing:schematicalTests/BoardLayersTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/KiCadExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).

## Decisions

- The stackup is a full table (copper per layer, a dielectric between each pair, finish) — decided by the owner, 2026-11-16. Impedance needs the dielectric height under a trace, so a total thickness alone cannot do it. (§ 3)
- The finish is one board setting that the preview and renders follow — decided by the owner, 2026-11-16. Delivered in plan-50-B.
- Defaults are JLCPCB's published 4- and 6-layer stackups and a 1.6 mm FR4 core for 1 and 2 layers — decided by the author (an internal choice). They are only starting values, editable in plan-50-B.
- Always written, never "only when changed" — decided by the author, following plan-49. (§ 3)

## Corrections

(Filled in during execution.)

## Summary

The risk is small and sits in the three saved-file goldens and the one `didSet` that keeps `layers` and
`stackup` in step. Untouched: the mask colour, design rules, trace-width limit, every export and every screen.
