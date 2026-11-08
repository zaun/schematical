# plan-25-A: Locked corners and L legs (route model)

Last updated: 2026-11-07
Overall Effort: large (3h–1d)
Effort: medium (1h–2h)
Depends on: nothing

plan-25 changes how the Line and Bus tools draw. Both get one drawing
model. A route is a list of **locked legs** plus one **live leg** to the
cursor. Space locks the live leg and drawing carries on from its end. Esc
unlocks the last locked leg, or cancels when nothing is locked. `/` flips the
live leg's L. Each leg is an L, not Line's midpoint Z. A pin forces the
direction a wire leaves or enters it. The Z is used only when both ends of
one leg are forced along the same axis.

This sub-plan builds the model only: `RouteDraft`, the leg rule, a Line
`connect` that takes a drawn path, and a Bus join that takes a path of any
length. Nothing calls the new pieces from the canvas yet (that is
plan-25-B), with one exception. `connect(_:_:)` without a path now routes
with the leg rule, so a pin-to-free-point wire becomes an L where it used
to be a Z.

Outcome: `RouteDraft(start: (0,0), startAxis: .horizontal)` with a locked leg to
(50,100) and a live leg to a right-side pin tip at (0,100) gives the path
`(0,0) (50,0) (50,100) (0,100)`: a U. `BusJoin.join` given the path
`(100,100) (300,100) (300,200) (100,200)` makes one bus of three lines with
two L corners.

References:

- `schematical/schematical/Canvas/SchematicModel.swift`: `OrthogonalRoute`
  (line 691), `connect(_:_:)` (line 1982), `anchorTip` (1926), `pinTip`
  (1860), `ChipLayout.Side` (127), `busDraft`/`addBus` (2199–2227)
- `schematical/schematical/Canvas/BusModel.swift`: `BusJoin.join` (673),
  `BusJoin.build` (756), `BusDraft` (610), `Bus.pinTip` (423),
  `BusLine.isHorizontal` (54)
- `schematical/schematicalTests/BusJoinTests.swift`, `WireSettleTests.swift`

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| Escape-cancel is on main (plan-25-B extends its monitor) | `git log --oneline -1 b7c95b5` → `b7c95b5 Line and Bus tools: Escape during a drag cancels it, placing nothing` | MET (2026-11-07) |
| No other plan changes `BusJoin.build` or `connect` | `grep -ln 'BusJoin\|func connect' planning/plan-*.md` → only plan-25 files | MET (2026-11-07) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `RouteDraft` (new, pure value type) holds a start point, the start's
  forced axis, the locked legs and the live leg's flip flag. It answers
  `path(to:endAxis:)`, `lock(to:endAxis:)`, `unlock()` and `flip()`.
- `OrthogonalRoute.leg(from:to:first:last:preferred:)` returns the leg
  (§4.1).
- `SchematicDocument.connect(_:_:path:)` places a wire along a given path.
  `connect(_:_:)` keeps its signature and routes one leg with the pins'
  axes.
- `BusJoin.join(_:path:)` and `SchematicDocument.busDraft(path:)` accept a
  path of 2 or more points with every Bus rule unchanged.
  `join(_:from:to:)`, `busDraft(from:to:)` and `addBus(from:to:)` keep their
  current behavior: they try H-first, then V-first.

### Non-goals (explicit constraints)

- No change to the file format, `Wire`, `Bus`, `BusJoint` or the saved
  JSON. A wire's corners are still `interior` points. A bus is still lines.
- No change to Bus rules: T/L joins, leaving a corner straight makes a T,
  end-to-end merge, joining two buses, wire-name matching and the
  too-many-wires refusal all stay as they are. The new rules apply on top.
- No change to `OrthogonalRoute.settled`, `moveNode`, `moveSegment`, or to
  how wires re-fit when parts move.
- No change to the PCB Trace tool.

## 2. Current State

- **Line route.** `OrthogonalRoute.route(from:to:)` (`SchematicModel.swift:694`)
  is straight when the ends are aligned. Otherwise it is a Z: out
  horizontally, a vertical run at the snapped midpoint x, then horizontally
  in. `connect` (`:2005`) stores `route` minus its ends as `interior`. The
  preview (`SchematicCanvasView.swift:325`) draws the same `route`. It
  ignores which side a pin faces, so a top pin's wire leaves sideways.
- **Pin sides.** A chip pin has `ChipLayout.Side`: `.left`, `.right`, `.top`
  or `.bottom` (`SchematicModel.swift:127`). `PlacedChip.tip(of:)` (`:591`)
  puts the tip one stub outside that side. `flipped()` swaps sides
  (`:369`), so `chip.layout.slots` (as `pinTip` reads it) is already
  flip-aware. A bus port pin sticks out across its line
  (`BusModel.swift:401`, `point(on:along:out:)`). Its stub is vertical on a
  horizontal line, and horizontal on a vertical one.
- **Bus route.** `BusJoin.join(_:from:to:)` (`BusModel.swift:673`) tries
  `[p, (q.x,p.y), q]` (H-first), then `[p, (p.x,q.y), q]`. The first
  candidate `build` accepts wins. `build` (`:756`) turns the path into
  `drawn` lines. It handles corner/free-end touches at `path[0]` and
  `path.last`, and expects one corner joint at `path[1]` only when
  `drawn.count == 2`. A path of more than 3 points would get no joints
  expected at its later corners, so `jointShapes` would not match and
  `build` would refuse it. That is the one line to generalize.
- **Settling keeps L's.** `settled` (`SchematicModel.swift:738`) only moves
  `points[1]` onto the start row when `points[1].x != start.x`, and the
  last bend onto the end row when it is off the end's column. On an
  orthogonal path either condition already implies the point is on that
  row (read the function: the segment is horizontal, so the y's match), so
  `settled` leaves any orthogonal drawn path as it is. That includes
  V-first L's and V–H–V Z's. So `path(forWire:)` shows exactly what was
  drawn.

### Measured populations

| What | Count | Command |
|---|---|---|
| Callers of `OrthogonalRoute.route` | 2 (connect, canvas preview) | `grep -rn 'OrthogonalRoute.route(' schematical/schematical` → 2 non-definition lines |
| Callers of `busDraft`/`addBus`/`BusJoin.join` in the app | 3 (`SchematicModel.swift:2200`, `:2209`, `SchematicCanvasView.swift:334/610`) | `grep -rn 'busDraft\|addBus(\|BusJoin.join' schematical/schematical \| grep -v 'func \|///'` → 4 lines |
| `connect(` calls in tests | 9 | `grep -c '\.connect(' schematical/schematicalTests/*.swift` → 2+1+4+2 |
| Bus join test calls | 9 in BusJoinTests, 4 elsewhere | `grep -c 'BusJoin.join\|addBus(\|busDraft' …` |
| App tests | 88 | `grep -c 'func test' schematical/schematicalTests/*.swift` summed |

### Verified properties

- **Tests don't pin wire shapes.** Every test that reads a wire's path
  after `connect` asserts `isOrthogonal` and the end tips
  (`SelectionGestureTests.swift:116-121`, `SelectionModelTests.swift:166-171,190-193`),
  or compares interior to the captured base (`SelectionModelTests.swift:156`).
  None hard-codes a Z. The `circuit()` helpers find `middle` from the first
  wire, which is pin to pin (LED K right side → LED A left side, both
  horizontal), so it stays a Z under the new rule. Read in all four files.
- **New Swift files need no project edit.** The project uses synchronized
  groups (`grep -c PBXFileSystemSynchronizedRootGroup schematical.xcodeproj/project.pbxproj` → 5).

## 3. Design Overview

Three layers, lowest risk first:

1. `OrthogonalRoute.leg` and `simplified`: pure functions.
2. `RouteDraft`: a pure value type over `leg`, shared by both tools.
3. Document entry points: `connect(_:_:path:)`, `anchorAxis(_:)`,
   `BusJoin.join(_:path:)`, `busDraft(path:)`.

**Risk** sits in `BusJoin.build`'s generalization. It is the code that
decides valid joins, and existing behavior for 2- and 3-point paths must
not move. The change only adds the expected-corner joints at every
interior point. It keeps `drawn[0]` and `drawn.last` handling as is, so
for `drawn.count <= 2` the expected set is the same set as today. The
existing BusJoinTests (7 tests) and the bus harness scripts guard it.

The gate is behavioral (unit tests). Wire shapes are expected to change
for a pin to free point `connect`, where the Z becomes an L.

Rejected alternatives:

- *Posture follows the first mouse motion* (Altium-style). It is hidden
  state that changes with a twitch of the mouse. `/` is explicit, and the
  owner chose it.
- *Store locks in `Wire`/`Bus`.* Locks only exist while drawing. Once
  placed they are plain corners, and the file format stays the same.
- *Separate draft types per tool.* The owner wants the two tools to work
  the same way. One type gives them one rule and one set of tests.

## 4. Detailed Design

### 4.1 The leg rule (`OrthogonalRoute`)

```swift
enum RouteAxis: Hashable { case horizontal, vertical
    var other: RouteAxis { … } }

/// Points after `p` up to and including `q`.
static func leg(from p: CGPoint, to q: CGPoint,
                first: RouteAxis?, last: RouteAxis?,
                preferred: RouteAxis) -> [CGPoint]
```

- `p == q` → `[]`.
- Aligned (`p.x == q.x || p.y == q.y`) → `[q]` (straight, as today, even
  when a forced axis disagrees).
- `first` and `last` both set:
  - equal, `.horizontal` → Z with its vertical run at `snap((p.x+q.x)/2)`
    (today's route);
  - equal, `.vertical` → Z with its horizontal run at `snap((p.y+q.y)/2)`;
  - different → the L whose first segment is `first`.
- Only `first` set → the L starting along `first`.
- Only `last` set → the L starting along `last.other`.
- Neither → the L starting along `preferred`.

The L starting horizontally is `[(q.x, p.y), q]`. The one starting
vertically is `[(p.x, q.y), q]`.

`route(from:to:)` stays as `leg(from:to:first:.horizontal,last:.horizontal,…)`
prefixed with `a`. It keeps its one remaining meaning (pin-to-pin default)
for any harness that calls it.

`static func simplified(_ path: [CGPoint]) -> [CGPoint]` runs
`mergeCoincident`, then `removeDoubleBacks`, then drops an interior point
when its neighbours are on one row or column with it and it lies between
them (a lock in the middle of a straight run). It never drops the first or
last point.

### 4.2 `RouteDraft` (new file `Canvas/RouteDraft.swift`)

```swift
struct RouteDraft: Equatable {
    let start: CGPoint
    let startAxis: RouteAxis?        // the start pin's axis, nil for a free point
    private(set) var legs: [[CGPoint]] = []   // each locked leg: points after its start
    private(set) var flipped = false          // `/` on the live leg

    var hasLocks: Bool
    var legStart: CGPoint            // legs.last?.last ?? start
    /// The live leg's default first axis: across the last locked segment
    /// (so a new leg turns), else horizontal.
    var preferredAxis: RouteAxis     // flipped ? default.other : default
    func liveLeg(to: CGPoint, endAxis: RouteAxis?) -> [CGPoint]
        // leg(from: legStart, to:, first: legs.isEmpty ? startAxis : nil,
        //     last: endAxis, preferred: preferredAxis)
    func path(to: CGPoint, endAxis: RouteAxis?) -> [CGPoint]
        // simplified([start] + legs.flatMap{$0} + liveLeg)
    /// Whether `/` changes the live leg: true only when neither end is forced.
    func canFlip(endAxis: RouteAxis?) -> Bool
    mutating func flip()             // toggles `flipped`
    /// Locks the live leg as `path` shows it. False (no change) when the
    /// leg is empty (cursor on the leg start). Resets `flipped`.
    mutating func lock(to: CGPoint, endAxis: RouteAxis?) -> Bool
    /// Drops the last locked leg. False when there was none. Resets `flipped`.
    mutating func unlock() -> Bool
}
```

The default "across the last locked segment" axis is the author's call.
After locking a vertical run, the natural next move is sideways, and
continuing straight would merge into the locked run anyway. `/` overrides
it for that leg.

### 4.3 Line entry points (`SchematicModel.swift`)

- `func anchorAxis(_ anchor: LineAnchor) -> RouteAxis?`: `.pin` →
  `.horizontal` for a `.left`/`.right` slot, `.vertical` for
  `.top`/`.bottom` (slot looked up as `pinTip` does). `.busPin` →
  `.vertical` when its port's line `isHorizontal`, else `.horizontal`.
  `.point` → nil.
- `func connect(_ start: LineAnchor, _ end: LineAnchor, path: [CGPoint]? = nil) -> String?`:
  the voltage check and endpoint resolution are unchanged. The route is
  `OrthogonalRoute.simplified([fromTip] + bends + [toTip])`, where `bends`
  is `path` minus its first and last point. With `path == nil` it is
  `[fromTip] + leg(fromTip→toTip, first: anchorAxis(start), last: anchorAxis(end), preferred: .horizontal)`.
  `interior` is the route minus its ends. A path whose first or last point
  is not that anchor's tip is a programming error, so `connect` uses the
  tips, not the path's ends, and drops only the path's ends.

### 4.4 Bus entry points (`BusModel.swift`, `SchematicModel.swift`)

- `BusJoin.join(_ buses: [Bus], candidates: [[CGPoint]])`: today's body,
  with the candidate list passed in and `p`/`q` taken from
  `candidates[0].first/last`. Each candidate first goes through
  `OrthogonalRoute.simplified`. A candidate that simplifies to under 2
  points is skipped.
  `join(_:from:to:)` builds today's two candidates and calls it, so its
  results do not change.
- `build`: replace the `drawn.count == 2` corner expectation with a loop
  over `0..<drawn.count-1` that expects
  `makeCorner(resolve(drawn[i].b-end), resolve(drawn[i+1].a-end), at: path[i+1])`.
  Nothing else in `build` changes.
- `BusDraft.path` doc: "the route as drawn, 2 or more points".
- `SchematicDocument.busDraft(candidates:)` and `addBus(candidates:)`:
  the same as the `from:to:` versions, over `BusJoin.join(_:candidates:)`.
  The Bus-only fallback (plan-25-B passes it) is: the path as drawn, then
  the same path with the live leg's L flipped, and the flipped one only
  when the live leg is a free L. `RouteDraft.busCandidates(to:)` returns
  that list: `[path(to:nil)]`, plus the flipped-live-leg path when
  `canFlip(endAxis: nil)` and the live leg has a corner.

## Phases

> Tick each box in the same commit as its work. `- [~]` for partial.

### Phase 1 — Leg rule and simplify

Pure functions with no callers yet. Safe alone.

- [ ] `SchematicModel.swift`: add `RouteAxis`, `OrthogonalRoute.leg`,
      `OrthogonalRoute.simplified` per §4.1. Re-express `route(from:to:)`
      over `leg`.
- [ ] Tests: new `schematicalTests/RouteLegTests.swift`. Aligned → straight.
      H/H → Z at mid-x. V/V → Z at mid-y. H/V and V/H → L. Only-first,
      only-last and neither × preferred. `simplified` drops a straight-through
      lock and a double-back, and keeps a U.

Acceptance: every case in §4.1 returns the listed points, and `route` returns
the same points as before for the cases in `WireSettleTests`.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/RouteLegTests -only-testing:schematicalTests/WireSettleTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 2 — RouteDraft

- [ ] New `Canvas/RouteDraft.swift` per §4.2, including `busCandidates(to:)`.
- [ ] Tests: new `schematicalTests/RouteDraftTests.swift` covers:
      - the U from the Outcome;
      - lock → unlock → lock gives the same path;
      - `unlock()` with no locks returns false;
      - `lock` at the leg start returns false;
      - after a vertical locked run the live leg starts horizontally;
      - `flip()` changes a free leg and `canFlip` is false when an end is
        forced;
      - `flipped` resets on lock and on unlock;
      - the first leg honors `startAxis` (a vertical start leaves vertically).

Acceptance: the cases above pass.
  Check: `xcodebuild … test -only-testing:schematicalTests/RouteDraftTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 3 — Document entry points (Line and Bus)

Highest risk: `BusJoin.build` and `connect`'s default route.

- [ ] `SchematicModel.swift`: `anchorAxis(_:)` and `connect(_:_:path:)` per
      §4.3.
- [ ] `BusModel.swift`: `BusJoin.join(_:candidates:)`, the `build` corner
      loop, and the `BusDraft.path` doc, per §4.4. `join(_:from:to:)` calls
      through.
- [ ] `SchematicModel.swift`: `busDraft(candidates:)` and
      `addBus(candidates:)` over the new join.
- [ ] Tests, `BusJoinTests.swift`:
      - a 4-point U makes 3 lines and 2 corners;
      - a U that starts on a bus's free end extends that line and still
        makes its corners;
      - a path that crosses itself is refused with `badJoint`;
      - a path of collinear points is simplified to one line.
- [ ] Tests, new `schematicalTests/ConnectPathTests.swift`:
      - pin to pin through a U stores the U's bends;
      - pin to free point with no path is an L leaving the pin along its
        side (an LED pin, horizontal);
      - a supply pin's wire leaves vertically: `5V` (pin on the bottom,
        `ChipLayout` `.vcc` → `side: .bottom`, `SchematicModel.swift:197`)
        and `GND` (pin on top, `:199`), both placed with
        `TestSupport.place`;
      - pin to pin on different rows with no path is still the mid-x Z.

Acceptance: the new tests pass, and every existing test passes unchanged.
That covers the 7 BusJoinTests and the 9 `connect` callers.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → "0 failures" (est. 5 min; the whole app suite, because `connect` and `BusJoin` are used across 5 test files, and listing them all costs as much as running it).
Commit: —

## Validation Plan

- Tests: RouteLegTests, RouteDraftTests, ConnectPathTests (new). Additions
  to BusJoinTests, including the refusals.
- Coverage: each new function is called by a new test (named above).
- Runtime proof: plan-25-B (the canvas wires this in).
- Doc sync: doc comments on `OrthogonalRoute.route/leg`, `connect`,
  `BusJoin.join`, `BusDraft.path`. No `docs/` file describes drawing
  (`grep -il 'line tool\|bus tool' docs/*.md` → none).
- Final gate: run once at the end of plan-25-B.

## Decisions

- Space locks the live leg and drawing continues from its end, repeatable.
  Owner, 2026-11-07.
- Esc unlocks the last lock, and cancels the drawing when nothing is
  locked. There is no Backspace binding. Owner, 2026-11-07 ("backspace
  seems odd").
- `/` flips the live leg's L. Owner, 2026-11-07.
- Line and Bus work the same way, and the Bus-specific rules apply to Bus
  only. Owner, 2026-11-07. Here that means one `RouteDraft`, with Bus join
  rules only in `BusJoin`.
- Each leg is an L. A pin forces the leaving/entering axis by its side, and
  the Z is used only when both forced axes on one leg agree. Owner agreed
  to the L-leg proposal, 2026-11-07. Using the pin's side, not "always
  horizontal", is the author's reading of it: pins face all four sides
  (§2).
- The live leg turns across the last locked run by default. Author's call
  (§4.2).

## Corrections

## Summary

The risk is in `BusJoin.build`'s corner loop, which is guarded by the
existing join tests. Everything else is new pure code. File format,
settling and the Trace tool stay as they are.
