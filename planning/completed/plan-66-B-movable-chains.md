# plan-66-B: Push and shove — which existing copper may move (movable chains)

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-66-A

B teaches `SchematicDocument.routingProblem` to cut the existing copper of
nets the run does not route into **movable chains** — stretches of trace and
via from one anchor to the next — when `pushAndShove` is on. Nothing pushes
them yet (C and D do); B delivers the data type, the chain rules, and the
guarantee that every piece of existing copper is either in exactly one chain or
still an obstacle, never both and never neither.

References:

- `planning/completed/plan-66-A-push-option-anchors.md` (§ 3 design, § Prerequisites,
  § Decisions — read first).
- `schematical/schematical/Routing/RoutingProblemBuilder.swift` (the builder),
  `RoutingProblem.swift`, `CopperGeometry.swift` (`CopperGeometry.shape(of:)`,
  `CopperShape.gap(to:)`).
- `schematical/schematical/Canvas/BoardTrace.swift` (`BoardTrace`, `BoardVia`,
  `TraceRoute.same`), `Canvas/Ratsnest.swift` (`BoardConnectivity`: "a trace
  joins the pads it touches on its layer and the traces it meets there; a via
  joins whatever touches its metal"), `Canvas/SchematicModel.swift:3818`
  (`join`: traces that touch share a vertex).

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The gate for the whole feature is in plan-66-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-66-A is complete and archived | `ls planning/completed/plan-66-A-* \| wc -l` → 1 | MET (2026-11-25, in `worktree-P-66`: 1) |
| The option and the counters exist | `grep -c 'pushAndShove' schematical/schematical/Routing/RoutingOptions.swift` → at least 3 | MET (2026-11-25: 4) |

If plan-66-A is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- With `pushAndShove` on, `routingProblem` returns `problem.movable` (the
  chains), `problem.bystanders` (one `RoutingNet` of rules per net that has a
  chain), and `problem.obstacles` without the chains' traces and vias.
- With it off, `movable` and `bystanders` are empty and `obstacles` is exactly
  what it is today.
- For every net the run does not route: each unlocked trace segment and via of
  it is in exactly one chain or exactly one obstacle.

### Non-goals (explicit constraints)

- No engine change: nothing in `Router`, `RoutingState`, `RoutingGrid` reads
  `movable` or `bystanders` until plan-66-C. A problem with chains routed by the
  engine as it stands would see free space where the chains are — C owns that, so
  B's problem is not routed by any test.
- No change to the document, the file format or the DRC.
- Copper of a net the run routes is never a chain (§ 3 of plan-66-A).

## 2. Current State

`routingProblem(options:rules:)` (`Routing/RoutingProblemBuilder.swift:19-166`)
works from `connectivity()` (pads, traces, vias, a root per node) after removing
what the options clear. Its `nets` are the scope's nets with two or more
components (`:84-119`); then **every** trace segment and via of every net is
appended to `obstacles` tagged with its net (`:136-151`), so a net may run over
its own copper and never another's.

Board connectivity is geometric: `join` makes every contact between two traces
on a layer a shared vertex (`SchematicModel.swift:3818-3835`); a pad is joined
by a trace that touches it; a via by whatever touches its metal
(`Canvas/Ratsnest.swift`, `BoardConnectivity` doc). A trace is one polyline
(`BoardTrace.points`, mm, ≥ 2 points, consecutive points distinct), on one
layer, one width (`Canvas/BoardTrace.swift:9`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Places the builder appends traces and vias to `obstacles` | 2 loops | `sed -n '136,151p' schematical/schematical/Routing/RoutingProblemBuilder.swift` → the `for (t, trace)` and `for (v, via)` loops |
| `RoutingProblem` memberwise constructions | 2 | `grep -rn 'RoutingProblem(' schematical --include='*.swift' \| wc -l` → 2 (the builder, `RoutingTestBoard.problem()`) |

### Verified properties

- **A new field with a default on `RoutingProblem` breaks no constructor.**
  `RoutingProblem` is a `struct` with stored properties and the two constructions
  above call the memberwise initialiser by label; a stored property with a default
  value is optional in the memberwise initialiser (Swift rule, and the file's own
  `removedTraces` is passed by every caller, so the two callers are the full list).
- **A chain's copper is the copper that today is an obstacle.** The partition
  test in Phase 2 checks that against `routingProblem` with the option off.

## 3. Design Overview

### The types (`Routing/RoutingProblem.swift`)

```swift
nonisolated struct MovableChain: Hashable, Sendable, Identifiable {
    let id: Int                  // index into RoutingProblem.movable
    var net: Int                 // the net, as BoardConnectivity numbers it
    var width: Double            // mm: every run's width
    var viaDrill: Double         // mm; 0 and 0 when the chain has no via
    var viaDiameter: Double
    var runs: [Run]              // end A to end B
    var vias: [Via]              // runs.count - 1 of them; via i joins runs i and i + 1
    var ends: [End]              // exactly 2: A (start of runs[0]) and B (end of runs.last)

    struct Run: Hashable, Sendable {
        var trace: UUID          // the BoardTrace the run lies in
        var layer: CopperLayer
        var first: Int, last: Int  // vertex indices into that trace's points, first < last
        var reversed: Bool       // the chain runs from `last` down to `first`
        var points: [CGPoint]    // in chain order, mm
    }
    struct Via: Hashable, Sendable { var id: UUID; var centre: CGPoint }
    struct End: Hashable, Sendable {
        var point: CGPoint       // the anchor's exact point
        var layer: CopperLayer   // the layer of the run that ends here
        var anchor: RoutingComponent   // what a search starts from (below)
    }
}
```

`RoutingProblem` gains `var movable: [MovableChain] = []` and
`var bystanders: [RoutingNet] = []`.

### What is a chain

For net *N* (not in `problem.nets`, on a net, not cleared by the options), all
of *N*'s unlocked copper. At a point *P* on a layer *L*, count the trace-segment
ends at *P* on *L* (points equal within `TraceRoute.epsilon`), the pads of *N*
whose copper covers *P* on *L* (`CopperShape.gap(to:) ≤ 1e-6`; a through-hole pad
is on every layer), and the vias of *N* whose centre is at *P*:

- *P* is an **interior vertex** when it has exactly two segment ends, no pad and
  no via (a bend, or a plain point on a run).
- *P* is a **via vertex** when it has one segment end on *L*, no pad, one via at
  *P*, and exactly one segment end at *P* on exactly one other layer, and nothing
  else touches the via's metal.
- Every other point where copper of *N* has a vertex is an **anchor**: a pad
  touches it, or three or more segment ends meet (a junction), or it is a free end
  (one segment end, no pad, no via), or a via with more than two ends or with a
  single end.

A **chain** is a maximal sequence of segments and vias from one anchor to
another that passes only through interior and via vertices. Its two anchors are
its `ends`; an end's `anchor` component is what a search starts from:

- a **pad**: the pad's shape (`CopperGeometry.shape(of:layers:node:)`) and label;
  the terminal's tie is the pad's centre, so a chain re-laid by a push ends at the
  pad's centre (as every new connection does), which may differ from the vertex a
  hand-drawn trace ended at inside the pad; the copper still touches the pad;
- a **via anchor**: a `RoutedVia` at its centre (`RoutingGrid.terminals` takes the
  nearest cell on every allowed layer);
- a **junction** or a **free end**: a pad-like `CopperShape(core: .segment(p, p),
  radius: options.grid, layers: [layer], node: -2)` at the point *p*. A
  zero-length trace would not do: `terminals` accepts a cell within half a pitch
  of a segment (`RoutingTerminals.swift:68`), and a point off the lattice can have
  none that near, while a shape of radius one pitch always contains a cell centre.
  Its tie is *p* itself (the middle of the shape's box).

A chain is **fixed** — left in `obstacles`, as today — when any of these holds:

1. any trace or via in it is locked;
2. its runs do not all have one width, or its vias do not all have one drill and
   one diameter;
3. a run lies on a layer the run's options leave out (`options.enabled(board.traceLayers)`);
4. another piece of copper of the same net touches it other than at its two
   anchors (a pad or via of *N* touching a segment other than the first or the
   last segment of the chain, a segment of another trace of *N* crossing it away
   from a vertex), or any copper of another net touches it;
5. it has fewer than two anchors (a closed loop);
6. its net already has a movable chain of a different width or via size, in
   `id` order — the first chain's sizes are the net's (a net has one routing
   profile; `RoutingGrid` keys profiles by net).

### The bystander net

For each net with at least one chain, one `RoutingNet` of rules in
`problem.bystanders`: `id` the net, `name`, `key`, `width` and via size from its
chains, `clearance` the net's class clearance (never under the DRC's minimum, as
`routingProblem` computes it for routed nets, `:93-96`), `allowed`/`preferred`
from `effectiveLayers(ofNet:)` ∩ the run's layers, `components: []`,
`planeLayer: nil`, `group: nil`, `impossible: nil`, `pins` from the connectivity.
It is never routed (no components, so `connections` is 0) and never reported.

### Where correctness risk concentrates

In the chain rules: a chain wrongly declared movable can move copper that
carried a connection the geometry no longer keeps. The rules are the
conservative side of every doubt (a fixed chain costs only a missed push). The
document's connectivity after a push is checked in plan-66-C (a test compares
`netGroups()` before and after); B's tests check the chain structure.

**Gate class:** the option off is **provably neutral** (the problem is equal to
today's: an equality test); the option on changes the problem by design, and the
**expected** difference is exactly the partition property above.

**Rejected alternatives.**

- *One chain per `BoardTrace`.* A trace polyline can pass through a junction
  that another trace is attached to; moving it whole would pull the junction.
- *Chains that end only at pads.* A trunk with branches would be fixed whole;
  junction anchors let each branch move on its own.
- *A chain per net.* One width, one via size, and one profile per net would be
  forced on whatever the user drew; rule 6 keeps that to the first chain only.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. An unticked box means NOT DONE.

### Phase 1 — the types, and the problem unchanged

- [x] (done) `Routing/RoutingProblem.swift`: add `MovableChain` as in § 3, and the two
      fields `movable`, `bystanders` (defaults `[]`) on `RoutingProblem`.
- [x] (done; see Corrections: the fixtures carry no copper, so a bent trace and a via are added to a two-LED board as well) `schematicalTests/RoutingProblemTests.swift`: `testAProblemWithoutChainsIsAsBefore`:
      the RegFile fixture's `routingProblem(options: .standard, rules: .customDefault)`
      has `movable.isEmpty`, `bystanders.isEmpty`, and `obstacles.count` equal to
      pads + trace segments + vias + holes + outline edges counted from
      `doc.connectivity()` in the test (the count the builder makes today).

Acceptance: the project builds, and the test passes with the option off.
  Check: `XB -only-testing:schematicalTests/RoutingProblemTests`
  → `** TEST SUCCEEDED **` (est. 2 min: build 50 s, tests seconds).
  Measured: `Executed 11 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: 0bb9e5e0

### Phase 2 — the extraction

- [x] (done; the signature takes `grid:` and `excluding:` instead of `rules:` and `scope` — see Corrections) `Routing/MovableChains.swift` (new): `extension SchematicDocument`
      `func movableChains(in copper: BoardConnectivity, netOfRoot: [Int: Int],
      excluding scope: Set<Int>, routable: [CopperLayer], rules: DesignRules) ->
      [MovableChain]` implementing § 3: a per-layer table of vertices keyed by the
      point rounded to 1e-4 mm (equal within `TraceRoute.epsilon` after rounding
      is the identity), the classification, the walk from each anchor along
      unvisited segments, the six fixed-chain rules, ids in order of the first
      segment's trace index.
- [x] (done; the loops skip covered segments and vias instead of filtering afterwards — same set) `RoutingProblemBuilder.swift`: when `options.pushAndShove`, call it after
      `obstacles` is built; remove from `obstacles` exactly the segments and vias
      the chains cover (match by trace id and vertex range, via id), fill
      `problem.movable`, and build `problem.bystanders` as in § 3. With the
      option off nothing is called and nothing changes.
- [x] (done: 12 tests on a two-LED board; see Corrections) `schematicalTests/RoutingChainTests.swift` (new), on documents built from
      the RegFile fixture with `unrouteAll(onlyAuto: false)` and traces added
      with `addTrace(points:layer:width:)` between pad centres read from
      `doc.connectivity().pads` (the test picks two pads of one net by label):
      a pad-to-pad trace is one chain, `ends` both pads, `runs.count == 1`;
      a trace with a bend keeps the bend inside one run; a trunk with a branch
      attached at a vertex gives three chains meeting at a junction anchor; a
      top trace, a via and a bottom trace pad to pad give one chain with
      `vias.count == 1`, `runs.count == 2`, `runs[0].layer != runs[1].layer`; a
      locked trace, a net in the run's scope, two widths in one chain, a pad
      touching a middle segment, and a run on a layer the options disable each
      leave the copper in `obstacles` and `movable` empty.
- [x] (done) The same file, `testEveryExistingSegmentIsInExactlyOneChainOrOneObstacle`:
      route RegFile with `Router.route` at `RoutingOptions.fixture`, apply it
      (`doc.applyRouting`), build the problem for the scope `.nets([one key])` with
      the option on, and for every net other than that one count its trace
      segments and vias in `obstacles` + `movable` and compare with the same count
      from `connectivity()` — equal per net, and no segment counted twice.
- [x] (done) `testTheOptionOffGivesTheSameObstaclesAsBefore`: the problem with the option
      off equals the problem the same document gives with every `pushAndShove`
      line of the builder bypassed — expressed as: `obstacles` (as a `Set`) with
      the option on, plus the chains' own segments and vias, equals `obstacles`
      with the option off.

Acceptance: all `RoutingChainTests` and `RoutingProblemTests` pass.
  Check: `XB -only-testing:schematicalTests/RoutingChainTests
  -only-testing:schematicalTests/RoutingProblemTests` → `** TEST SUCCEEDED **`
  (est. 3 min; the RegFile route at Fast in Debug is the long part, about the
  52 s the fixtures took in plan-38).
  Measured: `RoutingChainTests` 14 tests, 0 failures (78.4 s), `RoutingProblemTests` 11, 0 failures; with the anchors:
  `Executed 20 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: 9a2ef872

## Validation Plan

- Tests: `RoutingChainTests` (new, 14), `RoutingProblemTests` (+1). Negative
  cases: every fixed-chain rule has a test.
- Coverage check: the new file is in the `schematical` target, so the tests
  exercise it; the partition test runs it over a routed fixture.
- Runtime proof: none beyond tests (nothing routes with chains until C).
- Doc sync: none yet; plan-66-F writes `docs/routing.md`.
- Final gate (once, here): the Phase 2 Check (est. 3 min).

## Decisions

plan-66-A § Decisions apply. Added by the author, 2026-11-25:

- **Chains end at anchors (pads, junctions, free ends, multi-layer vias); vias
  with exactly two ends are inside a chain** — so a branch moves on its own and a
  plain layer change moves with the trace. (§ 3)
- **One routing profile per net** — a net's first chain (by id) sets the width
  and via size; a chain that differs stays fixed. (§ 3, rule 6)
- **Conservative fixed rules** — a chain touched by other copper away from its
  anchors, or on a disabled layer, is never moved. (§ 3)

## Corrections

- **The example fixtures carry no copper** (2026-11-25). Phase 1 says the RegFile
  fixture's obstacle count is pads + trace segments + vias + …; measured, none of
  `AFU`, `LFU`, `RegFile` has a trace or a via as committed (the first run of the
  test failed on `segments > 0`). `testAProblemWithoutChainsIsAsBefore` therefore
  checks all three fixtures at zero copper *and* a two-LED board with a bent trace
  (2 segments) and a via, so the count means something.
- **`movableChains` takes `grid:` and `excluding:`, not `rules:` and `scope`**
  (2026-11-25). The extraction needs the grid (a junction's anchor shape has
  radius one pitch) and no rules; `excluding` is the set of nets never chained: the
  routed nets plus nets with no stable key (`planeNet(ofNet:) == nil`), which could
  not host a bystander `RoutingNet`.
- **Two traces meeting end to end are a junction, not a bend** (2026-11-25). § 3
  calls a point with "exactly two segment ends" interior. Run/via structure
  (`vias.count == runs.count - 1`) has no place for two runs joined on one layer
  without a via, so an interior vertex here is one with a *single trace occurrence*
  (a bend of one trace); the meeting of two traces is an anchor and the chains on
  either side move on their own.
- **A bystander's `allowed` also holds its chains' layers** (2026-11-25). § 3 says
  `effectiveLayers ∩ the run's layers`; a net's own drawn copper may lie on a layer
  its rule does not list, and pushing must not force it off that layer, so the run
  layers of its chains are added (they are all enabled, by rule 3).
- **Rule 4's "touch" is measured on the segment, not only at vertices**
  (2026-11-25): a pad touching a segment where neither end is in the pad, a via's
  metal touching a segment with no end at its centre, and a vertex of any trace
  on the middle of a segment (as `BoardConnectivity` joins them) each mark that
  segment, so its chain stays. Found while testing: `addTrace` joins a second trace
  into the first with a shared vertex, so the "pad touching a middle segment" test
  needs the second trace to start at the first's *end*.
- **A scoped net that is already whole is not routed, so its copper can be chained**
  (2026-11-25): `testEveryExistingSegmentIsInExactlyOneChainOrOneObstacle` first
  asserted that no chain is of the scope net; on the routed RegFile the scope net
  has one component, so the run does not route it. The assertion is removed; the
  per-net partition equality covers every net.
- **The chain tests are 14, not 8** (2026-11-25): one per rule (locked trace, locked
  via, net in the run, two widths, pad over a middle segment, disabled layer,
  closed loop, a net's first chain sets the width) beside the three positive shapes,
  the reversed direction and the two partition tests; the Validation Plan's "8" was
  a floor.

## Summary

B is data only: the engine does not read it. The risk is the rules; each one has
a negative test and the partition test makes a copper segment counted in two
places or in none fail.
