# plan-65-D: The Autorouter Reads the Board Edge

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-C (complete)

The autorouter stamps the board's outline as an edge keep-out and treats the cells outside it as blocked. With cutouts a route must also stay out of every cutout and keep the copper-to-edge distance from its edge,
and the verifier and the cleanup must say the same. This sub-plan makes the routing problem carry the cutouts and the four places that read the outline read `BoardEdge`.

The single outcome: an autoroute on a board with a cutout between two pads goes round it, no trace point or via is inside a cutout or within `minCopperToEdge` of its edge, and a routing problem with no cutouts is stamped, routed and verified exactly as today.

References:

- plan-65-A § 3 (the union rule; the router's parity test is risk 3), § 4.2 (`BoardEdge`)
- `schematical/schematical/Routing/RoutingProblem.swift` (`outline` at `:102`), `RoutingProblemBuilder.swift` (`:152`–`:163`), `RoutingGrid.swift` (`stampEdge` at `:228`, the call at `:102`, bounds at `:56`), `RoutingVerifier.swift` (`:77`–`:88`, `inside` at `:191`), `RoutingCleanup.swift` (`:88`–`:89`, `:148`)
- `schematicalTests/RoutingTestBoard.swift` (the hand-built problem), `RoutingGridTests.swift`, `RoutingVerifierTests.swift`, `RoutingCleanupTests.swift`, `RoutingProblemTests.swift`, `RoutingPlanTests.swift`
- `docs/routing.md`

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-C is complete | `ls planning/completed \| grep -c '^plan-65-C'` → 1 | MET (2026-11-25: C landed, 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `RoutingProblem` has `cutouts: [[CGPoint]]` (default empty) and `edge: BoardEdge`; `SchematicDocument.routingProblem` fills it from the document's cutouts.
- `RoutingGrid.stampEdge` blocks every cell inside a cutout and every cell within `minCopperToEdge` plus a profile's half width (via radius for vias) of any loop's edge.
- `RoutingVerifier` reports copper in or near a cutout as an `.edge` violation; `RoutingCleanup` never keeps a shortcut whose midpoint is in a cutout.
- The edge obstacles the builder adds (`kind: .outline`) include the cutouts' segments.

### Non-goals (explicit constraints)

- No change to a problem without cutouts: same stamp, same routes, same verifier output (`RoutingGridTests`, `RoutingPlanTests`, `RoutingVerifierTests` pass unedited).
- No router search, cost or negotiation change; no new option or setting.
- No change to the DRC (C) or any export (G).

## 2. Current State

- `RoutingProblem.outline: [CGPoint]` ("The board's outline, clockwise, mm", `:102`); it has two construction sites: `RoutingProblemBuilder.swift:163` and `RoutingTestBoard.swift` (`rg -n "RoutingProblem\(" schematical --glob '*.swift'` → 2).
- `RoutingProblemBuilder.swift:152`–`:156` adds one `RoutingObstacle(kind: .outline)` per outline segment.
- `RoutingGrid.init` takes bounds from the outline (`:56`) and calls `stampEdge(problem.outline)` (`:102`). `stampEdge` (`:228`): per grid row it sorts the outline's crossings and blocks the cells whose crossing count to the left is even (outside); then for each outline segment blocks cells within `edge + profile.halfWidth` (traces) and `edge + profile.viaRadius` (vias), per profile and layer.
- `RoutingVerifier` (`:77`–`:88`): `edges` = outline segments as `CopperShape`s; a shape is a violation when `!inside(centre, outline) || gap < rules.minCopperToEdge - tolerance`. `inside(_:_:)` (`:191`) is its own even-odd loop.
- `RoutingCleanup` (`:88`–`:89`) builds `edges` the same way and drops a shortcut when `!RoutingVerifier.inside(mid, problem.outline)` (`:148`).

### Measured populations

| What | Count | Command |
|---|---|---|
| lines in `Routing/` that mention the outline (comments included) | 28 | `rg -n "outline" schematical/schematical/Routing --glob '*.swift' \| wc -l` → 28 lines in 5 files (RoutingCleanup, RoutingGrid, RoutingVerifier, RoutingProblemBuilder, RoutingProblem) |
| construction sites of `RoutingProblem` | 2 | `rg -n "RoutingProblem\(" schematical --glob '*.swift' \| wc -l` → 2 |
| router test classes (the no-cutout check uses five of them) | 5 named | `ls schematical/schematicalTests \| grep -E "^(RoutingGridTests\|RoutingVerifierTests\|RoutingCleanupTests\|RoutingProblemTests\|RoutingPlanTests)"` → all five |

### Verified properties

- A cell is board material exactly when `BoardEdge.contains(point)` (plan-65-A's definition, tested in A Phase 1), so the stamp's "outside" is `!edge.contains(cell point)`; the per-row crossing sort is only a speed-up for the outline and stays for the outline.
- Fixture routing in a Debug build is slow (`RoutingTestBoard.swift` header: Balanced on AFU "takes many minutes"), so the tests of this sub-plan use the hand-built `RoutingTestBoard` (20 × 20 mm) and `RoutingOptions.fixture` (`.fast` in Debug), never `RouterFixtureTests`.

## 3. Design Overview

`RoutingProblem` gets `var cutouts: [[CGPoint]] = []` directly after `outline` (a default, so both construction sites keep compiling) and `var edge: BoardEdge { BoardEdge(outline: outline, cutouts: cutouts) }`. The four readers:

- **Builder**: `cutouts: document.cutouts.map { $0.shape.polygon }`; the `.outline` obstacle loop runs over `edge.segments`.
- **Grid**: `stampEdge(_ edge: BoardEdge)`. Outside the outline: the existing per-row crossing parity on `edge.outline`. Inside a cutout: for each cutout loop, the same per-row parity on that loop; a cell that is inside the loop is blocked on every map. The distance stamp runs over `edge.segments`. Risk: a cell inside two overlapping cutouts is stamped twice (idempotent: `blocked`).
- **Verifier**: `edges` from `edge.segments`; `inside` is `edge.contains`.
- **Cleanup**: `edges` from `edge.segments`; the shortcut test is `edge.contains(mid)` and the shortcut segment is also rejected when it crosses a cutout segment (`CopperGeometry.segmentsCross`), so a long diagonal cannot jump over a thin cutout whose midpoint is outside it.

Where the risk concentrates: the shortcut that skips a thin cutout (the last bullet) and the grid's inside test for a loop that is clockwise or counter-clockwise (parity is winding-independent; tested with both).

Byte-identity is the gate for no cutouts (the five classes), behaviour tests are the gate with them.

Rejected: converting each cutout into a set of pad-like `RoutingObstacle` shapes (a polygon is not a `CopperShape`, and the existing obstacle kinds are clearance-based, not "inside" tests).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Problem, grid, verifier and cleanup

- [x] `Routing/RoutingProblem.swift`: `cutouts` and `edge`. `Routing/RoutingProblemBuilder.swift`: fill `cutouts`, segments from `edge`.
- [x] `Routing/RoutingGrid.swift`: `stampEdge(_ edge: BoardEdge)` per § 3. `Routing/RoutingVerifier.swift`, `Routing/RoutingCleanup.swift`: `edge` per § 3 (`RoutingVerifier.inside(_:_:)` stays, as `BoardEdge.contains(_:in:)`'s caller, if tests use it; otherwise it is removed with its tests moved).
- [x] `schematicalTests/RoutingTestBoard.swift`: `var cutouts: [[CGPoint]] = []`, passed into the problem.
- [x] Tests `schematicalTests/CutoutRoutingTests.swift`: grid cells inside a 6 × 6 cutout are blocked on every layer and cells 0.2 mm outside its edge are blocked for a trace of `minCopperToEdge` 0.3 (and free at 0.6 mm); a clockwise and a counter-clockwise cutout stamp the same cells; two pads 10 mm apart with a 6 × 6 cutout between them route (`RoutingOptions.fixture`) and the result's trace points are all `edge.contains` and ≥ `minCopperToEdge` from every cutout segment; the verifier reports a hand-made trace through a cutout as `.edge`; cleanup does not replace a bend by a straight shortcut that crosses a 0.8 mm-wide cutout when the shortcut's midpoint is not inside the cutout; a builder test: a document with a cutout gives `problem.cutouts.count == 1` and `.outline` obstacles for its segments.

Acceptance: the router goes round a cutout, the verifier and cleanup agree, and the five existing classes pass unedited.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutRoutingTests -only-testing:schematicalTests/RoutingGridTests -only-testing:schematicalTests/RoutingVerifierTests -only-testing:schematicalTests/RoutingCleanupTests -only-testing:schematicalTests/RoutingProblemTests -only-testing:schematicalTests/RoutingPlanTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min: the six classes use small hand-built boards; the fixture classes are not run).
  Observed: CutoutRoutingTests 6 + RoutingGridTests + RoutingVerifierTests + RoutingCleanupTests + RoutingProblemTests + RoutingPlanTests (the six classes together), 0 failures, TEST SUCCEEDED.
Commit: 7ade3de0

## Validation Plan

- Tests: `CutoutRoutingTests` (positive and negative), plus the five existing classes as the no-cutout check.
- Coverage check: `RoutingGridTests` and `RoutingVerifierTests` call the changed `stampEdge` and verifier paths, so a break of the no-cutout path fails there.
- Runtime proof: J autoroutes a real board with a cutout in the app.
- Doc sync: `docs/routing.md` — one paragraph: cutouts are edges (copper-to-edge, blocked inside).
- Final gate (run ONCE, after Phase 1): the six classes of the check (est. 6 min). Observed: the same run as the Phase 1 check (`CutoutRoutingTests` 6 tests, the five existing classes), 0 failures; it took under a minute warm.

## Decisions

- Cutouts are edges for the router with the board's `minCopperToEdge` — decided by the owner (copper clears a cutout like the board edge), 2026-11-25.

## Corrections

- 2026-11-25: `RoutingVerifier.inside(_:_:)`, the verifier's own even-odd copy, was removed (nothing else called it: `rg -n "RoutingVerifier.inside" schematical` found none after the cleanup moved to `BoardEdge.contains`).
- 2026-11-25: `RoutingTestBoard.problem()` builds its `.outline` obstacles from `BoardEdge(...).segments` too, so a hand-built board with `cutouts` has the same obstacles the builder gives.

## Summary

Risk: the shortcut that jumps a thin cutout and the grid parity test. Left untouched: the search, costs, options, DRC and exports.
