# plan-38-E: Routing engine — grid, obstacles, octilinear A*, layers, vias, plane fan-out (one pass)

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-D

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The first half of the engine: given a `RoutingProblem` (plan-38-D), route every
net's connections once, net by net, with exact obstacle handling, layer rules,
vias, 45° moves and plane fan-out, keeping every connection that succeeds.
There is no rip-up yet (F), no cleanup or exact verification (H): this letter's
output is "mostly complete, may have rough legs", used by F, G and H as their input.

References:

- plan-38-D (§ 3 scope, § 4 types); plan-38-B (§ 3 layer rules).
- `planning/plan-38-data/spike/README.md`, `results.txt`, `router-v1-no-ripup.swift`
  (the prototype this design generalizes; read its `search`, `stampPath`, `padNodes`).
- `DesignRuleCheck/DesignRuleCheck.swift:checkEdge` (finger exemption),
  `Ratsnest.swift` ("A Plane joins …").

## 1. Goal

- `Router.route(_ problem: RoutingProblem, progress:, isCancelled:) -> RoutingResult`
  exists, nonisolated, deterministic: the same problem gives the same result.
- On synthetic boards (see Phase tests) it finds the shortest legal path, goes
  around obstacles, changes layers by via only where allowed, never leaves an
  **Only** net's layers, prefers a **Prefer** layer, never enters foreign
  clearance, and ties a plane net to its plane with a via.
- On AFU (default options) a single pass completes at least 85 of the 115
  connections (the worst prototype run, `results.txt`: 85/115 at pitch 0.05),
  in under 30 s; the numbers for AFU, LFU and RegFile are recorded in this file.

### Non-goals (explicit constraints)

- No rip-up (F), no net ordering beyond "shortest span first, ties by net id",
  no cleanup of staircase artifacts (H), no length tuning (I), no pairs (J).
- No document edits, no UI. The engine reads only the `RoutingProblem`.
- Vias are through vias (the only kind the app has); no blind/buried vias.
- No push-and-shove (owner decision, plan-38-A § Decisions).

## 2. Current State

- The prototype (`router-v1-no-ripup.swift`) is a grid A* over per-layer owner
  arrays: `-1` free, a net id, or `-2` blocked; obstacles stamped by exact
  point-to-pad distance, routed cells stamped as disks of radius `clearance +
  width`; 8 moves, via cost 12, bend cost 0.3. Measured (`results.txt`): AFU at
  pitch 0.127 mm, trace 0.254 mm, 74–98 % complete in 0.3–1.4 s; finer pitch
  (0.1, 0.05) completed *fewer* (78 %, 74 %) because disk stamping is conservative
  and quantized — this design stamps routed copper by exact segment distance.
- No router exists in the app; `TraceRoute` (`BoardTrace.swift`) is the Trace
  tool's 1-bend geometry only.

### Measured populations

| What | Count | Command |
|---|---|---|
| Prototype single-pass AFU results, trace 0.254 mm | 98.3 %, 94.8 %, 89.6 %, 81.7 % (pitch 0.127); 78.3 % (0.1); 73.9 % (0.05) | `planning/plan-38-data/spike/results.txt` |
| Prototype single-pass LFU | 47.3 %, 76.6 %, 52.7 % | same |
| Grid size AFU / LFU at 0.1 mm | 534 × 445, same board outline | spike output `AFU: grid 534x445 x2 layers` |
| Two-layer fixtures | 3 of 3 (`layers` 2) | `results.txt` header table |

### Verified properties

- The prototype's A* (Swift `-O`, binary heap) expands a 534 × 445 × 2 grid for the
  whole of AFU in 0.83 s (pitch 0.1) and 2.11 s (pitch 0.05): a single pass at the
  default pitch is well inside the 30 s gate.
- A finger/header pad at the board edge sits inside the copper-to-edge keep-out:
  the prototype blocked it and routed RegFile 12/24 (`results.txt`); the DRC
  exempts them (`DesignRuleCheck.swift:checkEdge`). The grid here exempts them too.

## 3. Design Overview

**Grid.** Pitch `options.grid` (default 0.1 mm), origin the outline's bounding-box
corner, one plane of cells per routable layer. A **profile** is `(halfWidth,
clearance, viaRadius)` of a net's width/clearance/via; there is one blocked-map
per distinct profile (a board with a Default and a Power class has two), per
layer. A cell holds `Int32`: `-1` free, `n ≥ 0` covered only by net `n`'s copper,
`-2` blocked (covered by two nets, or by a hole, or outside). A cell is passable
for net `n` iff it is `-1` or `n`. A separate **via map** per profile uses the
via radius. The copper-to-edge keep-out and "outside the outline" are `-2`,
**except** cells within an exempt pad's bloat (fingers, header pads), which keep
only the pad's own net.

**Obstacle stamping.** For each `RoutingObstacle` and each kept trace/via, every
cell within `bloat = max(clearance_obstacle, clearance_profile) + r_obstacle +
halfWidth_profile` of its core (exact `CopperGeometry` distance) is stamped
with its net. Non-plated holes stamp `-2` over `drill/2 + clearance +
halfWidth`. A through-hole pad or a via stamps on every layer.

**Terminals.** A component's cells: every cell centre inside a pad (gap ≤ 0) on
the pad's layers (a through-hole pad: all routable layers); a pad too small to
hold a cell centre contributes the 4 nearest cells; a kept trace contributes the
cells its segments cross; a kept via contributes its cell on all layers. The
component's goal point (for the heuristic) is its first pad's centre.

**Search.** A* over `(x, y, layer)`; moves: 8 neighbours (4 with
`angle: .orthogonal`) on a layer, and a layer change at a cell where the via map is
free or own on **every routable layer**. A diagonal step also needs both
orthogonal neighbours passable (no corner squeezing). Cost per step: 1 or √2
(in cells), plus bend 0.3, plus via 12 (`strategy` scales it in F), plus
direction penalty ×1.5 against a layer's `.horizontal`/`.vertical` preference,
plus ×1.6 on a non-preferred layer of a **Prefer** net. Expansion never enters a
layer outside the net's `allowed` set; a layer change is not offered when
`vias == .none`, or the net's allowed set holds one layer, or `viaDrill` is below
`rules.minViaDrill`. Heuristic: octile distance to the goal point. Sources:
every cell of the growing tree; target: every cell of one other component.

**Net routing.** Nets run in order of ascending bounding-box half-perimeter, ties by
net id. A net's components are joined as a tree: from the tree, pick the other
component closest to the tree (pad-centre distance), search to it, stamp the new
copper by **exact segment distance** into every profile's maps (each segment
bloats by `max(c1,c2) + h1 + h2`, each via by `max(c1,c2) + r_via1 + h2`), add it
to the tree, repeat. A failed connection is recorded and does not discard the
net's others. The search aborts a connection after 3 000 000 expansions.

**Plane fan-out.** A net with `planeLayer` and `fanoutPlanes`: each component that
holds no through-hole pad or via gets one connection whose target is *any cell
where a via is legal* (Dijkstra, no goal point); the via is placed and the
component is then joined to the plane by the connectivity rule.

**Statuses.** `impossible` nets (plan-38-D) are not searched. Result per net:
`complete`, `partial(unrouted: n)`, `failed`, `impossible(reason)`.

Where correctness risk concentrates: obstacle stamping (an off-by-bloat is a DRC
clearance error) and the search's layer/via rules. Both are covered by synthetic
tests with hand-computed expectations; H adds the exact post-check.

Byte-identity is not a gate here; behavior is.

Rejected: a shape-based (non-grid) expansion-room router — more exact, but 3–5×
the code, and the grid's staircase is repaired in H; disk stamping of routed
cells (prototype; conservative, non-monotonic in pitch); one grid for all
profiles with the largest bloat (wastes room around Power-class neighbours).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — result types and the grid

- [x] `Routing/RoutingResult.swift`: `RoutingResult { traces: [RoutedTrace], vias: [RoutedVia], nets: [NetOutcome], stats }`, `RoutedTrace.net`, `NetOutcome { net, status, unroutedConnections }`, `RoutingProgress`. (`RoutedTrace.net` / `RoutedVia.net` are defaulted fields in `RoutingProblem.swift`; `NetOutcome` also lists each unrouted connection by its pads, which F's fixture test reads.)
- [x] `Routing/RoutingGrid.swift`: pitch/origin, `Profile`, per-profile per-layer maps, obstacle and keep-out stamping, the exempt-pad rule, `isPassable(cell, net, profile)`. (Exempt-pad rule corrected, see Corrections. The maps after fixed copper are kept as `fixedTraceMaps` / `fixedViaMaps` for F's un-stamping.)
- [x] `Routing/Router.swift`: `enum Router { static func route(...) }` returning an empty result for now.
- [x] Tests: `schematicalTests/RoutingGridTests.swift` with a helper `RoutingTestBoard` (rectangle outline, `pad(x,y,w,h,net,layers)`, `hole`, `net(...)`): a cell exactly `clearance + halfWidth` from a foreign pad is free, 0.01 mm closer is blocked; a cell next to a pad of net 1 is passable for net 1 and blocked for net 2; outside the outline and within `minCopperToEdge + halfWidth` of it are blocked; a cell inside an exempt finger on the edge is passable for its net (strengthened: its inner part is, its part inside the edge band is not — see Corrections); two nets' bloats overlapping makes the cell `-2`. (Plus: an L-shaped outline's outside; holes on every layer; via edge distance.) — `-only-testing:RoutingGridTests` → 6 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: the cases pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/RoutingGridTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: eac104da

### Phase 2 — the search for one connection

- [x] `Routing/RoutingSearch.swift`: A* of § 3 (binary heap, deterministic tie order by cell index), costs, layer/via rules, the 3 000 000 expansion cap, cancellation poll every 4096 expansions. (Heuristic: octile distance to the targets' cell bounding box, which is admissible, where a single goal point would not be for a many-cell target. Terminals of § 3 in `Routing/RoutingTerminals.swift`, each with its tie point; `SearchRules(net:grid:problem:)` derives a net's rules.)
- [x] Tests (`RoutingSearchTests`): straight run is straight; an obstacle forces a detour whose length equals the hand-computed 45° detour; two stacked blocking traces on Top force a via and a return on Bottom; with `vias: .none` the same board fails; a net `Only {Top}` never touches Bottom and fails rather than via; a `Prefer {Bottom}` net takes Bottom on an open board; `.horizontal` on Top makes a vertical run cost more than the same run on Bottom `.vertical`; `angle: .orthogonal` never produces a diagonal step; a via needing three free layers is refused when an inner layer cell under it is blocked (4-layer test board); the search is cancelled within 4096 expansions of `isCancelled` turning true. (The detour's "hand-computed" length is checked against a plain reference Dijkstra in the test over the same passability, with bend cost 0, so it measures optimality exactly.) — `-only-testing:RoutingSearchTests` → 10 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: the cases pass.
  Check: `-only-testing:schematicalTests/RoutingSearchTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: c3480ae1

### Phase 3 — nets, trees, stamping, plane fan-out

- [x] `Router.route`: ordering, tree growth, exact-segment stamping into every profile map, per-connection failure, statuses, progress callbacks (after each connection), conversion of a cell path to `RoutedTrace`s (merge collinear steps; one trace per layer run) and `RoutedVia`s. (Each path end ties to its terminal's exact point: a pad's centre, the nearest point on a kept trace's segment, a kept via's centre, since the DRC joins traces only on a segment, within 1e-4 mm.)
- [x] Plane fan-out of § 3. (Corrected target, see Corrections: a legal via site whose metal meets one of the component's pads, joined to the pad's centre by a trace.)
- [x] Tests (`RouterTests`): a 3-pad net becomes a tree (2 connections); a failed connection keeps the net's other connection (`partial(unrouted: 1)`); two crossing nets on a 2-layer board both complete (one uses a via); a Power-class net (0.5 mm) keeps its own wider clearance from a neighbour and the Default net next to it keeps the Default's; a GND net with a plane on Layer 2 of a 4-layer board gets one via per SMD-only component and no traces between them; an `impossible` net is reported and not searched; two runs on the same problem give equal results; reversing the order of `problem.nets` gives equal results. (Every result is also checked pair by pair for clearance with `RoutingCheck.violations`, which F reuses.) — `-only-testing:RouterTests -only-testing:RoutingSearchTests -only-testing:RoutingGridTests` → 23 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: the cases pass.
  Check: `-only-testing:schematicalTests/RouterTests -only-testing:schematicalTests/RoutingSearchTests -only-testing:schematicalTests/RoutingGridTests` → `** TEST SUCCEEDED **` (est. 2–3 min).
Commit: —

### Phase 4 — the fixtures, one pass

- [ ] `RouterFixtureTests`: `Fixture.open` + `routingProblem(options: .standard, rules: .customDefault)` + `Router.route` on AFU, LFU, RegFile; assert AFU completes ≥ 85 of 115 in < 30 s; assert no result trace has fewer than 2 points or zero-length segments; print `connections routed / total` and seconds for all three. Write the printed numbers into the Results line below, in the same commit.
- [ ] Results (the output of this run, filled in then): AFU __/115 in __ s; LFU __/188 in __ s; RegFile __/24 in __ s.

Acceptance: the AFU floor and the time bound hold; the three results are written above.
  Check: `-only-testing:schematicalTests/RouterFixtureTests` → `** TEST SUCCEEDED **` (est. 3 min; the spike's single pass took 0.3–2.1 s per board, so the bound is generous).
Commit: —

## Validation Plan

- Tests: `RoutingGridTests`, `RoutingSearchTests`, `RouterTests`, `RouterFixtureTests` (all new), `DesignRuleGoldenTests` (unchanged; the engine doesn't touch DRC).
- Coverage check: every file under `Routing/` named here is hit; the cancellation, via-refusal and exempt-pad tests would fail if those branches were removed.
- Runtime proof: Phase 4's fixture run.
- Doc sync: none yet (`docs/routing.md` is written in plan-38-M).
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: grid router with exact stamping; one blocked-map per net profile; vias pierce every routable layer; fan-out uses a legal-via-site target; AFU floor 85/115 is the worst prototype run, not a target (F raises the bar to 115/115).

## Corrections

- **Exempt pads (§ 3 Grid).** The plan had cells within an exempt pad's
  bloat keep only the pad's net, overriding the copper-to-edge keep-out. But
  `checkEdge` exempts only the finger/header pad itself (`if shape.node <
  copper.pads.count … chip < 0 … continue`); a trace entering the finger is
  still checked, so a trace centre inside the band is an `edge` issue, which
  plan-38-H's goal (0 edge issues) forbids. Corrected: the edge keep-out holds
  for every trace and via centre; the finger's own copper is stamped with its
  net as any pad is, so its net reaches it through the finger's inner part.
  The Phase 1 test checks both sides of that. The spike's RegFile 12/24 was
  not a finger problem: a probe of the RegFile problem (2026-11-14) found no
  exempt pad on any routed net (its 8 nets join LATCH, D, OUTA and OUTB pads).
- **Plane fan-out target (§ 3).** "Any cell where a via is legal" would leave a
  via the DRC calls "Goes nowhere": `checkConnections` counts the traces and
  pads that touch a via's metal (`touchingCount`), and a Plane is neither, so
  a via at the end of a trace touches 1 thing. Corrected: the target is a cell
  where a via fits *and* its metal meets one of the component's pads (not at
  the pad's exact centre), and a trace runs from the pad's centre to it, so
  the pad and the trace both touch the via (2). Tested in `RouterTests`.

## Summary

The risk is stamping and passability — a wrong bloat is a clearance error that
H must then catch. Untouched: rip-up (F), cleanup and verification (H).
