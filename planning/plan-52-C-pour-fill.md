# plan-52-C: The Pour Fill Geometry

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-52-B (complete)

A pour is a rectangle, a layer and a net (plan-52-A). This sub-plan computes what copper it holds: the pure geometry that takes
the pour, the keep-outs and the copper on the board and returns the filled shape and, for each piece of it, which pads, vias and
traces it touches. It is a function of values (no document, no UI), so it is tested with synthetic copper; plan-52-D wires it to
the document.

The single outcome: `PourFiller.fill(…)` returns, for every pour with a net, the pieces of its rectangle that keep the pour's
clearance from other nets' copper and from keep-outs, the board edge and the non-plated holes, join the net's pads through thermal
reliefs (or solid), and are not islands; and it does so in the times the probes measured.

References:

- plan-52-A § 3 (the fill in one paragraph) and § 4 (`BoardPour`, `BoardKeepout`); `planning/plan-52-data/README.md`,
  `probe_boolean_correctness.swift`, `probe_fill_pipeline.swift` (the probes this is built from; read them first)
- `schematical/schematical/Canvas/PCBCanvasView.swift`: `planeCuts` (`:424`, how a Plane cuts what it leaves out), `outline(of: PlacedPad)`,
  `path(_:)` and `traceStyle(width:)` (a trace's stroke)
- `Canvas/BoardTrace.swift` (`BoardTrace.points`, `widthMM`, `layer`; `BoardVia.rect`, `centre`, `diameter`), `Canvas/BoardModel.swift`
  (`PlacedPad`: `rect`, `drill`, `plated`, `holeRect`), `Canvas/BoardSettings.swift` (`DesignRules.minClearance`, `minCopperToEdge`)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-B is complete | `ls planning/completed \| grep -c '^plan-52-B'` → 1 | NOT MET (2026-11-21: 0; A and B are not started) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `Canvas/PourFill.swift` (new) has `PourNode`, `PourCopper`, `PourFill` and `PourFiller.fill(pours:nets:keepouts:copper:outline:edge:rules:)`.
- For a pour with a resolved net, the result's pieces cover its rectangle (within the board edge pull-back) except: every item on
  its layer not on its net grown by the clearance; each non-plated hole grown by the copper-to-edge; each keep-out that applies to
  the layer; the rectangle (grown by the clearance) of an earlier pour with another resolved net on the layer; and, in `thermal`
  mode, a ring round each same-net plated pad (the pad grown by `thermalGap`) less four axis-aligned spokes `spokeWidth` wide.
- A piece with no same-net item inside it (an island), and a piece whose bounding box is narrower than `minWidth`, are dropped.
- Each kept piece lists the same-net nodes it touches.
- The time for the fill of a real routed board is under 2 s (the probes measured 24 / 110 / 275 ms for 300 / 700 / 1500 obstacles).

### Non-goals (explicit constraints)

- No document, connectivity or drawing code is touched (plan-52-D, E). Planes, `planeCuts` and `planeClearances` are unchanged.
- No width clean-up by erosion (measured at 4 s, `planning/plan-52-data/README.md`); no teardrops, no hatched pours, no spoke angles
  other than 0°/90°, no per-pad connection overrides, no net-class clearances (a Plane has none either).
- Vias of the pour's net join it solid, with no relief.

## 2. Current State

- A Plane's cuts are built from `BoardConnectivity.planeClearances(on:)` (`Ratsnest.swift:619`) and drawn in
  `PCBCanvasView.planeCuts` (`:424`) and `drawPlane` (`:459`): clearance by filling a path and stroking it `2 × distance` wide with
  round joins. That is the same growth this plan uses, now as a `CGPath` (`copy(strokingWithWidth:lineCap:lineJoin:miterLimit:)`).
- A pad's outline is `PCBCanvasView.outline(of:)` (`PlacedPad` → `Path`), used by the Gerber export too
  (`GerberExport.shape(of:)`), so Gerber and canvas agree on a pad's shape.
- `GerberOutline.contours(of: CGPath)` (`GerberWriter.swift:308`) flattens a `CGPath` (curves included) to point loops.

### Measured populations

| What | Count | Command |
|---|---|---|
| obstacles in the 1500-obstacle probe | 1500 (+500 same-net pads, 2000 spokes) | `planning/plan-52-data/probe_fill_pipeline.swift`, `run(other: 1500, same: 500)` → `TOTAL 275 ms` |
| traces, vias, parts on the biggest routed example | 237, 40, 61 | `python3 -c` over `examples/slow PCB.schematical` `board.traces`, `board.vias`, `parts` (run 2026-11-21) |

### Verified properties

- `CGPath.subtracting(_:using: .winding)` equals the exact "not inside any grown obstacle" test: `mismatch 0 of 200000` (`./verify`,
  recorded in `planning/plan-52-data/README.md`).
- Two items 1 mm apart, each grown by 0.2 mm, leave a 0.6 mm strip between them; none within 0.15 mm of an item (same probe).
- A thermal ring with two crossing spokes cut out of it leaves copper in the spokes and none in the four gaps (`probe_thermal.swift`, recorded in the README).
- The result of `subtracting` holds curve elements (1757 in the probe), so consumers must flatten (`GerberOutline.contours` does).

## 3. Design Overview

One pure function, in four steps per pour, in drawing order:

1. **Region.** `rect` ∩ (the board outline less a stroke `2 × edge` wide).
2. **Cuts**, one combined path: for each `PourCopper` on the layer (`layers == nil` or contains it): not on the pour's net → its
   `shape` grown by the clearance (a `hole` item: by `edge`); on the net and a plated pad, `thermal` mode → ring less spokes;
   keep-outs; earlier other-net pours' rects grown. Same-net traces and vias are not cut (they merge into the fill).
3. **Fill** = region − cuts, `componentsSeparated(using: .winding)`.
4. **Keep**: drop a piece with `min(bounding box width, height) < minWidth`; keep a piece only if a sample point of some same-net
   item lies inside it, and record those items.

**Where correctness risk concentrates:** (a) the sample-point rule for "touches" (a pad is touched if its centre, four corners
or four edge midpoints are inside a piece; a via its centre and four rim points; a trace each vertex and segment midpoint). The
spokes start at the pad centre so a thermal pad's centre is always in the piece its spokes reach; (b) the spokes must be cut out of
the ring **before** the one big subtraction, so other-net clearances still cut them (the probe's pipeline; adding the spokes back after the subtraction was measured at 67 / 412 / 1053 ms for that step against 2 / 10 / 21 ms).
**What was probed:** everything above, `planning/plan-52-data/README.md`. No premise is unproven.

**Byte-identity is not a gate** (new behavior). The gate is the geometric tests below plus the time budget.

Rejected alternatives: *erosion/dilation width clean-up* (4 s measured); *a boolean per piece and pad for the touch test* (1542 ms against 8 ms by sample points at 1500 + 500); *Clipper port* (the system booleans pass the correctness probe and need no code).

## 4. Detailed Design

```swift
nonisolated enum PourNode: Hashable, Sendable { case pad(Int), trace(UUID), via(UUID) }   // pad = BoardConnectivity pad index

nonisolated struct PourCopper: Sendable {
    var node: PourNode
    var net: Int?                    // nil: on no net
    var layers: Set<CopperLayer>?    // nil: every layer (a through-hole pad, a via)
    var shape: CGPath                // the metal, in board mm
    var samples: [CGPoint]           // the "touches" test points (§ 3.4)
    var padRect: CGRect?             // a plated pad's rect: a same-net one gets a ring and spokes
    var isHole = false               // a non-plated hole: cut by `edge`, never joins
}

nonisolated struct PourFill: Sendable {
    let pour: UUID
    let pieces: [CGPath]             // kept pieces, each its outer loop and holes
    let joined: [[PourNode]]         // joined[i] = same-net nodes touching pieces[i], sorted
    var path: CGPath                 // the union of the pieces (for drawing)
    var isEmpty: Bool { pieces.isEmpty }
}
```

`fill(pours:nets:keepouts:copper:outline:edge:rules:)` returns `[UUID: PourFill]`; `nets[pour.id]` is the pour's resolved net
(absent: the pour fills nothing, and is not an earlier pour for another). `grown(_ p: CGPath, by c: Double)` is `p ∪ stroke(p, 2c,
round cap, round join)`. Spoke rectangles for a pad with rect `r`, gap `g`, width `w`: horizontal `(r.minX−g−0.05, r.midY−w/2,
r.width+2g+0.1, w)`, vertical `(r.midX−w/2, r.minY−g−0.05, w, r.height+2g+0.1)`. Trace metal: the polyline stroked `widthMM` wide,
round caps and joins. Pad metal: `PCBCanvasView.outline(of:)`. Via metal: its circle.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Region, clearances, keep-outs, edge

- [ ] `Canvas/PourFill.swift`: the three types of § 4 and `PourFiller.fill` steps 1–3 for `solid` pours and for other-net items:
      region, cuts (other-net items grown, holes by `edge`, keep-outs, earlier other-net pour rects), the subtraction,
      `componentsSeparated`; step 4 keeps every piece (no island rule yet).
- [ ] `PourFiller` also has `nonisolated(unsafe) static var fillCount = 0`, incremented once per `fill(…)` call; plan-52-D's tests read it to prove when a refill happens.
- [ ] Tests: new `schematicalTests/PourFillTests.swift` with a helper that builds `PourCopper` for a rect pad, a round via and a trace;
      a 100 × 80 board, a pour `(10,10,40,30)`: (a) with no copper the fill's bounding box is the rect; (b) another-net pad at the
      middle: no copper within the clearance, copper at clearance + 0.05 (use `CGPath.contains`); (c) a trace across the pour: two
      pieces; (d) a keep-out: no copper inside it, copper just outside; (e) a pour touching the board edge: no copper within `edge` of
      the outline; (f) a non-plated hole: none within `edge` of it; (g) two pours of different nets overlapping: the later has none
      inside the earlier's rect grown by the clearance.

Acceptance: each of (a)–(g) asserts the copper present and absent at the points named.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourFillTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — Thermal reliefs, same-net merge, islands, minimum width, joins

- [ ] Same-net pads: `thermal` → ring (pad grown by `thermalGap`) less the two spoke rectangles, added to the cuts before the
      subtraction; `solid` → nothing. Same-net traces and vias: never cut.
- [ ] Step 4: drop a piece whose bounding box is narrower than `minWidth`; keep a piece only if some same-net `PourCopper` has a sample
      point inside it (bounding-box prefilter, then `piece.contains(point, using: .winding)`); fill `joined`.
- [ ] Tests (`PourFillTests`): (a) a same-net pad in `thermal`: copper in each of the four spokes, none in the four corner gaps, `joined`
      lists the pad; (b) the same in `solid`: copper at the pad's edge, no ring; (c) a same-net pad enclosed by other-net traces: its
      piece is dropped (island) and `joined` is empty; (d) a sliver narrower than `minWidth`, not enclosed: dropped; (e) a same-net
      trace inside the pour: not cut, and `joined` lists it; (f) a pour with no resolved net: no entry with pieces; (g) two same-net
      pads whose spokes meet one piece: one `joined` entry holding both.

Acceptance: (a)–(g) hold, in particular the spoke and gap points of (a).
  Check: `xcodebuild … test -only-testing:schematicalTests/PourFillTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 3 — Real board, time budget

- [ ] Test `PourFillTests.testFillOfARealRoutedBoard`: open `examples/slow PCB.schematical` (4 layers; via `URL(fileURLWithPath: #filePath)`
      up to the repo root, `SchematicFile.decode(…).apply(to:…)` as `Fixture.open` does), build `PourCopper` for every pad, via and trace
      of `document.connectivity()` with the net from its root (the adapter plan-52-D adds does this properly; here a local helper),
      fill one ground pour over the whole board on Top and one on Bottom, assert the elapsed time is under 2 s and that no piece
      contains a sample point of an other-net pad (`PourFill.pieces` vs those samples).
- [ ] Record the measured time in this phase's `Commit:` line.

Acceptance: elapsed < 2 s and no other-net sample point inside any piece.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourFillTests/testFillOfARealRoutedBoard CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `PourFillTests` (new), 15 cases above.
- Coverage check: the file is in the app target, under the tests' `@testable import schematical`; `grep -c "PourFiller" schematical/schematicalTests/PourFillTests.swift` → at least 10.
- Runtime proof: none yet; the fill is first drawn in plan-52-E.
- Doc sync: none (internal); behavior is documented in `docs/export.md` by plan-52-G.
- Final gate (once, after Phase 3): `xcodebuild … test -only-testing:schematicalTests/PourFillTests -only-testing:schematicalTests/PourModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).

## Decisions

- Pours keep a clearance from other nets' pads, pins and traces and from keep-outs — decided by the owner, 2026-11-21. (§ 1, § 3)
- Islands are always removed — decided by the author: KiCad's default; a floating flood joins nothing and the DRC would flag it.
- Thermal relief is the default; spokes are axis-aligned and `spokeWidth` wide; vias join solid — decided by the author: the todo asks for thermal reliefs; KiCad's defaults (gap and spoke 0.5 mm) are the ones `KiCadBoardWriter.zones` already writes for a Plane.
- "Touches" is a sample-point rule, minimum width a bounding-box rule — decided by the author: the exact booleans cost seconds (measured); the rule is stated, tested and documented.
- A later pour gives way to the rectangle of an earlier pour of another net — decided by the author: predictable, cheap, no fill-of-fill dependency.

## Corrections

(Filled in during execution.)

## Summary

The risk is the geometry's edge cases (spokes cut by neighbours, enclosed pads) and the time on a dense board, both under test
before anything calls it. Untouched: the document, connectivity, drawing, Planes.
