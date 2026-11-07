# plan-16-D: Trace model

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-16-C

This sub-plan adds the document side of PCB traces. A trace is a copper
polyline on one copper layer, with a width in mil. Every segment is at a
multiple of 45° (0°, 45°, 90°, 135°…). Copper that touches is connected: when
a trace is added, every point where it touches or crosses a trace on the same
layer becomes a shared vertex of both, and pads it touches count as
connected. Traces are saved in the file. No UI here; E adds the tool and the
sidebar.

References:

- `schematical/schematical/Canvas/SchematicModel.swift:OrthogonalRoute` (the schematic routing this mirrors, at 45°)
- `schematical/schematical/Canvas/SchematicModel.swift:connect(_:_:)`, `segmentHit(at:tolerance:)`
- `schematical/schematical/Canvas/BoardModel.swift:PlacedPad`, `boardPads`
- `schematical/schematical/Document/SchematicFile.swift` (board decode, ID uniqueness check)
- plan-16-A (`CopperLayer`, `LayerType`, `canSetLayerCount`, `setLayerType`), plan-16-B (`BoardSide`), plan-16-C (`BoardVisibility`)

## Prerequisites

See plan-16-A § Prerequisites. plan-16-C must be complete:
`rg -n 'struct BoardVisibility' schematical/schematical/Canvas/BoardSettings.swift` → 1 match.

## 1. Goal

- `SchematicDocument.addTrace(from:to:layer:diagonalFirst:)` adds a 45°
  trace. Afterwards every same-layer contact point is a vertex of both traces.
  `connectedRun(of:)` returns the traces joined to it; `pads(touching:)`
  returns the pads the run touches. Traces survive Save and Open, and a bad
  trace in a file gives a specific error.

### Non-goals

- No vias. A trace stays on its layer; it reaches other layers only through
  through-hole pads.
- No nets, ratsnest, design-rule checks or clearance. A crossing joins two
  traces whether or not the schematic agrees.
- No node or segment dragging after a trace is drawn.
- No traces on Plane layers.
- Trace width does not count for contact: only centrelines are tested.

## 2. Current State

- Nothing PCB-side has connectivity today. `SchematicDocument` holds `chips`,
  `wires`, `junctions`, `buses`, `board` and `texts`
  (`SchematicModel.swift:780-825`). `replaceContents(…, board:, texts:)` sets
  them all on open.
- Schematic precedent: `OrthogonalRoute.route(from:to:)` makes a default
  two-bend route; `normalize` and `mergeCoincident` clean paths
  (`SchematicModel.swift:651-700`). `connect` splits a wire at a grid point
  so the new wire can branch there; that is the schematic's "join".
- Pads on the board are `PlacedPad` (`rect`, `drill`, `shape`, …) from
  `PlacedChip.boardPads` (`BoardModel.swift:102`). After B, `drill == 0` pads
  sit on the part's side; `drill > 0` pads go through every layer.
- The file checks that IDs are unique across parts, junctions, wires, buses
  and texts (`docs/file-format.md` errors: "an ID is used more than once
  across parts, junctions, wires and board texts").

### Measured populations

| What | Count | Command |
|---|---|---|
| ID kinds in the uniqueness check today | listed in the doc line | `rg -n 'used more than once' docs/file-format.md` → 1 line |
| Existing PCB document collections | 2 (`texts`, chip `board`) | `rg -n 'private\(set\) var texts\|var board = BoardSettings' schematical/schematical/Canvas/SchematicModel.swift \| wc -l` → 2 |

### Verified properties

- Two segments of the same layer, each at a multiple of 45°, meet in a single
  point, an overlap, or not at all.
  - Their intersection is the standard parametric segment intersection.
  - When they are parallel and collinear, they overlap on an interval. Both
    ends of that interval are shared vertices.
  - The point may be off the 0.635 mm grid. Two diagonals cross at a half
    step; a pad centre can be off-grid. So trace vertices are not required to
    be on the grid, and the file doesn't snap them.
- A route between any two points can be made of at most two segments at
  multiples of 45°. With d = min(|dx|, |dy|), the route is one straight run
  of max − min along the major axis plus one diagonal of d.

## 3. Design

### Types (`Canvas/BoardTrace.swift`, new file)

```swift
nonisolated struct BoardTrace: Identifiable, Hashable, Sendable {
    let id: UUID
    var layer: CopperLayer
    /// mil, within `widthRange`.
    var width: Int
    /// mm; at least 2, consecutive points distinct, every segment at a multiple of 45°.
    var points: [CGPoint]

    static let widthRange = 5...100
    static let defaultWidth = 10
    var widthMM: Double { Double(width) * 0.0254 }
}
```

### 45° routing (`TraceRoute`)

- `route(from a, to b, diagonalFirst: Bool) -> [CGPoint]`. If a and b are
  already at a multiple of 45° (dx = 0, dy = 0, or |dx| = |dy|, within
  `TraceRoute.epsilon = 1e-6` mm), the route is `[a, b]`. Otherwise it is
  `[a, m, b]`:
  - **straight first:** m = a + the straight run along the major axis
    (sign × (major − minor));
  - **diagonal first:** m = a + (sign(dx)·d, sign(dy)·d).
- `isLegal(_ points:)`: at least 2 points, no zero-length segment, every
  segment at a multiple of 45° (within `epsilon`).
- `clean(_ points:)`: removes consecutive duplicates (within `epsilon`).

### Anchors (where a trace end lands)

`traceAnchor(at p, layer, tolerance t) -> CGPoint` tries, in order:

1. **A pad on that layer whose rect contains p** → the pad's centre. A
   surface-mount pad is on its part's side layer. A through-hole pad is on
   every layer.
2. **A vertex of a same-layer trace within t** → that vertex.
3. **A point on a same-layer trace segment within t** → the nearest point on
   that segment.
4. **Otherwise** → `BoardGrid.snap(p)`.

E passes t = 6 screen points ÷ scale, as the schematic line tool does.

### Adding and joining

`addTrace(from a, to b, layer, diagonalFirst) -> UUID?`:

1. Refuse (return nil, nothing changed) when `a == b` or when `layer`'s type
   is Plane.
2. `points = route(from: a, to: b, …)`. The new trace has width
   `defaultWidth`.
3. **`join(id)`.** For every other trace T on the same layer and every
   segment pair (new, T), compute the contact points (single crossing,
   touching endpoint, or both ends of a collinear overlap). Insert each point
   into both polylines as a vertex, in order along the segment, skipping
   points already within `epsilon` of a vertex.
4. Return the new ID.

Inserting a vertex on a straight segment keeps every segment at its angle,
so `isLegal` still holds.

`join` is also run:

- after `setTraceLayer` (E), against the new layer;
- after `moveTraces(_:by:)` (the plan-15 group drag, E).

Old vertices left in other traces by an earlier join are harmless collinear
points and are kept.

### Connectivity queries

- `connectedRun(of id) -> [UUID]`: the traces on the same layer reachable
  through shared vertices (within `epsilon`), found breadth-first. It
  includes `id`.
- `pads(touching run) -> [String]`: for each pad on that layer (as in
  Anchors 1) whose rect any run segment's centreline intersects or
  touches, the label `"<reference>.<designator>"` (e.g. `U1.3`, or
  `U1.EP` for an unnumbered centre pad). Sorted by reference, then
  designator.

### Editing helpers (used by E)

- `setTraceWidth(id, mil)`, clamped to `widthRange`.
- `setTraceLayer(id, layer)`: ignored for a Plane layer or a layer not on
  the board; otherwise it sets the layer, then runs `join`.
- `deleteTrace(id)`, which also drops it from the selection.
- `moveTraces(ids, by: CGSize)`: translates every vertex, then runs `join`
  for each moved trace.
- `trace(at p, tolerance t, visible: BoardVisibility) -> BoardTrace?`: the
  last trace drawn whose layer is visible and whose segment lies within
  `widthMM / 2 + t` of p.

### Layer rules (the A and C hooks)

- `canSetLayerCount(n)` also refuses when any trace is on a layer that `n`
  would drop: "Delete or move the traces on Layer 4 first." (Bottom is
  dropped only by `n = 1`.)
- `setLayerType(layer, .plane)` is refused while that layer holds a trace.
  A's sidebar picker shows the Plane choice disabled, with the help text
  "Move or delete this layer's traces first."

### Rejected alternatives

- **One trace object per connected run** (merge on join). Every branch would
  share a single width, so a thin branch off a 50 mil power trace would be
  impossible. Per-trace width with shared vertices keeps both.
- **Joins as a separate junction table**, like schematic junctions. Copper
  contact is geometric, so shared vertices express it with no extra state,
  and a deleted trace leaves nothing dangling.
- **Snapping every vertex to the grid.** Crossings of diagonals and pad
  centres are off-grid. Forcing the grid would bend traces off 45° or off
  their pads.

## Compatibility / Format Impact

New optional `board.traces` (version stays 3; see plan-16-A Corrections):

```json
"traces": [ { "id": "…", "layer": "top", "width": 10,
              "points": [ { "x": 2.54, "y": 5.08 }, { "x": 7.62, "y": 5.08 }, { "x": 10.16, "y": 7.62 } ] } ]
```

- The array is written in drawing order, as `[]` when empty; missing means
  none.
- Points are kept as saved, not snapped.
- Trace IDs join the file-wide ID uniqueness check.

New `badPCB` details:

- `trace <id> layer "inner4" is not on a 4-layer board`
- `trace <id> layer inner2 is a plane`
- `trace <id> width 3 must be 5 to 100 mil`
- `trace <id> has fewer than 2 points`
- `trace <id> segment 2 has no length` (segments are numbered from 1)
- `trace <id> segment 2 is not at a multiple of 45°`

## Phases

### Phase 1 — Trace value, routing and file format

- [ ] New `Canvas/BoardTrace.swift`: `BoardTrace`, `TraceRoute` (`route`,
      `isLegal`, `clean`, `epsilon`). No project edit: the app target is a
      synced folder (`grep -c PBXFileSystemSynchronizedRootGroup
      schematical/schematical.xcodeproj/project.pbxproj` → 4, and existing
      files such as `BoardModel.swift` aren't listed in it: `grep -c` → 0).
- [ ] `Canvas/SchematicModel.swift`: `private(set) var traces:
      [BoardTrace]`. Add `traces:` to `replaceContents`, which clears it with
      the rest.
- [ ] `Document/SchematicFile.swift`: `Board.traces` entries (`id`, `layer`,
      `width`, `points`), encoded and decoded with the errors above, and
      trace IDs in the uniqueness check.
- [ ] Tests (`<tests>/TraceRouteTests.swift`):
  - route for each octant, straight-first and diagonal-first;
  - an already-45° pair gives 2 points;
  - `isLegal` rejects a 30° segment and a zero-length one;
  - round trip of two traces;
  - each error detail;
  - a trace ID equal to a text ID is refused.

Acceptance: routes are legal in all octants, and traces survive a round trip.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:<tests>/TraceRouteTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 3 min).
Commit: —

### Phase 2 — Anchors, joining and connectivity

- [ ] `Canvas/SchematicModel.swift`: `traceAnchor`, `addTrace`, `join`,
      `connectedRun`, `pads(touching:)`, `setTraceWidth`, `setTraceLayer`,
      `deleteTrace`, `moveTraces` and `trace(at:tolerance:visible:)`, as in
      §3. Extend `canSetLayerCount` and `setLayerType` with the trace rules.
- [ ] Tests (`<tests>/TraceJoinTests.swift`):
  - a horizontal trace crossing a vertical one on Top: both gain the
    crossing vertex, and `connectedRun` has both;
  - the same crossing on Top vs Bottom: no vertex, separate runs;
  - two diagonals crossing at an off-grid point join;
  - a collinear overlap: both interval ends are shared;
  - an end released on another trace's segment lands on it and joins;
  - an end on an SMD pad of a Top part lands on the pad centre, and that
    pad is in `pads(touching:)`;
  - the same on Bottom does not join that pad;
  - a through-hole pad joins from an inner Wire layer;
  - `addTrace` on a Plane layer returns nil;
  - `setTraceLayer` to Bottom re-joins against Bottom traces;
  - `canSetLayerCount(2)` is refused with a trace on Layer 3;
  - `setLayerType(.inner(3), .plane)` is refused while it holds a trace.

Acceptance: every same-layer contact is a shared vertex, different layers
never join, and pad contact follows the side and through-hole rules.
  Check: `xcodebuild … test -only-testing:<tests>/TraceJoinTests` → all pass (est. 3 min).
Commit: —

## Validation Plan

- Tests: `TraceRouteTests` and `TraceJoinTests`, including the negative cases
  (Plane refusal, cross-layer non-join, bad file entries).
- Runtime proof: none at this step (no UI); E's runtime check covers it.
- Doc sync: `docs/file-format.md`. Add a Traces table under Board, the error
  details, trace IDs in the uniqueness sentence, and a trace in the example.
- Final gate: run once after E (see plan-16-E).

## Decisions

- Segments are limited to multiples of 45° — owner, 2026-11-07 ("limited to
  90, 45, 135"). A drag makes at most one bend, straight first; E's Option
  key puts the diagonal first.
- Touching or crossing copper on the same layer joins, and different layers
  never do — owner, 2026-11-07 ("any crossing/touching is an auto-join").
- Width is per trace. Joins are shared vertices, not merged objects — author's
  call, delegated by the owner. A thin branch can come off a wide trace.
- No vias in plan-16 — author's call. Through-hole pads are the only layer
  change; vias are a separate plan.
- Trace ends may be free (on a grid point), so a route can be drawn in
  pieces; each piece joins the last one at the shared end.
- New traces are 10 mil — owner, 2026-11-07.

## Corrections

None yet.

## Summary

The risk sits in contact detection: diagonal crossings, collinear overlaps,
and off-grid points compared within `epsilon`. Phase 2's tests pin each case.
UI comes after the model is proven (E). There are no vias or nets, and the
schematic is untouched.
