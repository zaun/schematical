# plan-30-C: Loose ends — gray on screen, black in export; landing

Last updated: 2026-11-08
Effort: medium (1h–2h)
Depends on: plan-30-B, plan-30-D (landing tier only)

C makes loose ends visible and lands plan-30. On the canvas, a wire with a
loose end (either end on a junction with `isLoose`, plan-30-A) is drawn
**gray, solid**, instead of the normal white/black line color. In an export
(SVG, PDF, PNG), it is drawn black, like every other wire. C then runs
plan-30's landing tier once.

References:

- `schematical/schematical/Simulation/Simulation.swift`: `WireStyle` :190,
  `wireStyle(_:)` :510.
- `schematical/schematical/Canvas/SchematicCanvasView.swift`: `WireLayer`
  inputs :259–271, `JunctionDot` :985, `WireLayer.paint` :1015–1054.
- `schematical/schematical/Canvas/SchematicExportScene.swift`: :20–21
  (routes and dots).
- `schematical/schematical/Canvas/SchematicPainter.swift`: `ExportPalette`
  :89, `ScreenPalette.screenColor` :124.
- `docs/export.md` (the export draws black line art).

## Prerequisites

plan-30-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-30-B complete | `grep -c "^Commit: —" planning/plan-30-B-loose-ends-auto-connect.md` → `0` (or the file is in `planning/completed/`) | MET (2026-11-08: `grep -c "^Commit: —"` → `0`; B landed in ce6357b) |

## 1. Goal

- In edit mode, a wire with a loose end is stroked with `Ink.secondary` (the
  system's secondary label color, gray) at the normal width, solid. Every
  other wire is unchanged.
- The export scene draws every wire black, as today. A loose end changes
  nothing in an export.
- A loose junction draws a dot only when 3 or more wire ends are on it (a
  real branch point). On screen the dot is gray; in an export it is black.
  A selected loose junction always shows the accent selection dot. A
  1- or 2-wire loose end has no dot, so it doesn't look like a connection.

### Non-goals (explicit constraints)

- No dashes. The owner asked for gray only (§ Decisions).
- Play-mode colors win. A shorted, toggling, floating or valued net draws
  as today, loose end or not. Gray replaces only `.normal`.
- Plain junctions keep their dot, as today.

## 2. Current State

- `WireStyle` (:190) has `.normal`, `.shorted`, `.toggling`, `.floating`,
  `.value`. The canvas sets each route's style from
  `simulation.wireStyle(wire.id)` (:264). `WireLayer.paint` switches on it
  (:1031). That switch is the only one in the app, and it is exhaustive
  (`grep -rn "case .floating" schematical/schematical` → 1, :1034).
- The export scene (`SchematicExportScene.swift:20`) builds routes with the
  default `.normal` style. With `ScreenPalette.export` or `SVGPainter`,
  `.primary` is black at alpha 1 (`ExportPalette` :92), and `.secondary`
  would be black at 0.6. So gray must never reach the export scene's
  routes, which this design ensures by setting it only in the canvas.
- Junction dots are built in two places: the canvas (:267) and the export
  scene (:21). `WireLayer.paint` fills every dot with `.primary`, or
  `.accent` when selected (:1044–1049).

### Measured populations

| What | Count | Command |
|---|---|---|
| `WireStyle` switches | 1 | `grep -rn "case .floating" schematical/schematical` → SchematicCanvasView :1034 |
| `JunctionDot(` builders | 2 | `grep -rn "JunctionDot(" schematical/schematical` → canvas :268, export :21 |

## 3. Design

- `WireStyle.loose`, documented "Edit mode: a wire with a loose end."
  `WireLayer.paint`: `case .loose: painter.stroke(path, with: .secondary,
  style: solid)`.
- `SchematicDocument.hasLooseEnd(_ wire: Wire) -> Bool`: either end is
  `.junction(id)` of a junction with `isLoose`.
- `SchematicDocument.showsDot(_ junction: Junction) -> Bool`:
  `!junction.isLoose ||` (wire ends on it) `>= 3`.
- `static func WireLayer.style(simulated: WireStyle, loose: Bool) -> WireStyle`:
  `.loose` when `simulated == .normal && loose`, else `simulated`. Canvas
  (:264): `style: WireLayer.style(simulated: simulation.wireStyle(wire.id),
  loose: document.hasLooseEnd(wire))`.
- `JunctionDot` gets `var isLoose = false`. Canvas (:267): include a
  junction when `showsDot(junction) || isSelected`, with `isLoose` set from
  the junction. Export (:21): include a junction when `showsDot`, and leave
  `isLoose` false.
- `WireLayer.paint` dots: `.accent` if selected, else `.secondary` if
  `isLoose`, else `.primary`.

Correctness risk is low. The one trap is putting gray into the export scene,
where `.secondary` exports as 60 % black. The design never does that, and a
test pins it.

Rejected alternative: map `.loose` to black in `ExportPalette`. Roles there
are per ink, not per wire, and `.secondary` is shared with other drawing.
Keeping the export scene `.normal` needs no palette change.

## Phases

> **NOTE: keep the checkboxes current.** Tick in the same commit as the work.
> An unticked box means NOT DONE.

### Phase 1 — Gray loose wires

- [x] `Simulation/Simulation.swift`: `WireStyle.loose`.
- [x] `Canvas/SchematicModel.swift`: `hasLooseEnd`, `showsDot` (§ 3).
- [x] `Canvas/SchematicCanvasView.swift`: route style and dots (:259–271),
      `JunctionDot.isLoose`, and the `.loose` case and dot ink in `paint`.
      Update the `WireLayer` doc comment (:989) to say loose wires are gray.
- [x] `Canvas/SchematicExportScene.swift:21`: filter dots by `showsDot`.
- [x] `docs/export.md`: one line under what the export draws: "Wires with a
      loose end are gray on screen and black in an export, like every
      wire."
- [x] Tests:
  - `schematicalTests/LooseEndTests.swift` `testLooseStyleOnlyReplacesNormal`:
    `WireLayer.style(simulated:loose:)` gives `.loose` for
    (`.normal`, true), and the simulated style otherwise (`.shorted`,
    `.floating`, `.value(.green)` with loose true; `.normal` with loose
    false). It also paints one `.loose` route through `WireLayer.paint` with the
    tests' `RecordingPainter` (`SchematicSceneTests.swift:15`) and expects
    exactly one stroke, with `Ink.secondary` and line width 1.5.
  - `schematicalTests/SchematicSVGTests.swift`
    `testLooseWireExportsBlack`: two `SN74HC00N` wired 1Y→1A, then the
    second one erased (one loose wire left). The SVG's `paintValues` is
    `["none", "#000"]`, as in `testOneChipAndAWire` (:70), and the text has
    no `stroke-opacity="0.6"` (SVGPainter :30 writes the alpha there).
  - `schematicalTests/LooseEndTests.swift` `testLooseDotOnlyAtBranches`:
    `showsDot` is false for 1- and 2-wire loose junctions, true for 3, and
    true for any plain junction.

- [x] Observed: the Check → `LooseEndTests` `Executed 18 tests, with 0
      failures`, `SchematicSVGTests` `Executed 6 tests, with 0 failures`,
      `** TEST SUCCEEDED **`.

Acceptance: loose wires paint gray on screen, export black, and loose ends show no dot unless they branch.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/LooseEndTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 4 min).
Commit: f39627b

### Phase 2 — Landing tier (run once)

| Gate | Command | Expected |
|---|---|---|
| App builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p30-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| All app tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` (A, B and C changed shared model code that every canvas test goes through, so the whole app test target runs here once; est. 8 min) |
| Harness | `scripts/harness/build.sh scripts/harness/<f>.swift && /tmp/schematical-harness/H.app/Contents/MacOS/harness` for `bus-erase`, `generic-header`, `bus-lines`, `bus-ports` | no `FAIL` lines (`bus-lines` needs plan-30-D) |
| Runtime proof | In the real app: place two parts, wire them, erase one with the eraser (a click), screenshot; place the same part at the same spot, screenshot. Then move the pointer to (0, 0) (AGENTS.md). | First screenshot: the wire is still there, gray. Second: the wire is back on the pin, normal color. |

The backend is not touched by plan-30, so the backend tests are not a gate
(`git diff --stat <plan-30-A first commit>^.. -- backend` → empty).

- [x] Run every gate and record the observed values under the table.
      Run after merging main (e35daab, 5 tactile-switch commits; clean
      merge 2156942), on the merged tree:
  - App builds: `** BUILD SUCCEEDED **`.
  - All app tests: 213 test cases passed, 0 failed, `** TEST SUCCEEDED **`.
  - Harness: `bus-erase` 29 ok, 0 FAIL, `ALL OK`; `generic-header` 15 ok,
    0 FAIL; `bus-ports` 19 ok, 0 FAIL, `ALL OK`; `bus-lines` first run 7
    FAIL and a crash, the same as on 48645a4 before plan-30 (see
    Corrections) → fixed in plan-30-D → 35 ok, 0 FAIL, `ALL OK`.
  - Runtime proof, in a separate instance of the `/tmp/p30-dd` build (the
    owner's own running build was not touched), driven with real CGEvents
    (`/tmp/p25drive/drive`), screenshots with `screencapture -l`: opened a
    file with two SN74HC00N wired 1Y→1A; Eraser click on the right part →
    the wire stays, drawn gray, no dot at its end; dragged SN74HC00N from
    the drawer to the same spot → the wire is on 1A again, normal white.
    (A first drop landed 30 pt low, so the new part's VCC tip sat on the
    loose end and the wire joined VCC, which is the designed behavior; it
    was erased and dropped again.) Pointer moved to (0, 0) afterwards.
- [ ] Move `plan-30-A`, `plan-30-B` and `plan-30-C` to `planning/completed/`.

Acceptance: every gate's observed value matches.
  Check: the table (est. 20 min; this is the plan's one landing run).
Commit: —

## Validation Plan

- Tests: Phase 1's three tests. Phase 2 is the final gate.
- Doc sync: `docs/export.md`, plus `docs/file-format.md` from A.

## Decisions

- Disconnected wires are gray, not dashed — decided by Justin Zaun,
  2026-11-08.
- Exports draw them black, with no colors — decided by Justin Zaun,
  2026-11-08.
- "Disconnected" means a wire with a loose end — author's call: that is the
  state A creates. Wires that are disconnected some other way, such as a net
  with no driver, already have play-mode styling.
- Loose ends get no dot below 3 wires — author's call: a dot reads as a
  connection.

## Corrections

- `docs/export.md` also says a loose end gets a dot only where 3 or more
  wires meet, since § 1 makes that true of exports too.
- `RoutedWire.style`'s doc comment ("Play mode: …") now also names
  `.loose`; it was the one other comment the new case made wrong.
- The landing tier's `bus-lines` harness was already failing before
  plan-30 (7 FAIL lines and a crash on 48645a4, run from a detached
  checkout), because it still expected bus refusals that later plans turned
  into T and bus joins. A new letter, plan-30-D, updates it; C's landing
  tier depends on D.
- `LooseEndTests.swift` imports SwiftUI for `.value(.green)` (§ Phase 1
  test list); the first build failed without it.

## Summary

Small and contained: one new style case, two dot builders, and a test that
keeps gray out of exports.
