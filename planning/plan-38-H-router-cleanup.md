# plan-38-H: Routing engine — cleanup (45° shortcuts, chamfers, via minimization, stubs) and exact verification with repair

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-G

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F, G) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The grid engine's raw output is a staircase through cell centres, with ties to
pads that are only approximately legal. This letter turns it into the copper a
person would draw — straight 45° runs, chamfered corners, no pointless via
hops, no stubs — and then **proves** every piece against the design rules with
the DRC's own geometry. A piece that fails is reverted, then re-routed with
extra margin, and only reported unrouted if it still cannot be made legal. The
guarantee that follows is the one the whole plan promises: **routing never adds
a clearance, edge, via, trace-width, hole or connectivity issue to the DRC.**

References:

- plan-38-D (`CopperGeometry`, `RoutingProblem`), E (`RoutingGrid`, tie cells),
  F (`RoutingState` ledger, negotiation), G (`applyRouting`).
- `DesignRuleCheck/DesignRuleCheck.swift`: `checkClearance` (`:460`), `checkEdge`
  (`:516`), `checkHoles` (`:552`), `checkVias` (`:611`), `checkTraceWidths` (`:631`),
  `checkConnections` (`:151`, the "unconnected" rules a trace end and a via must meet).

## 1. Goal

- `Router.route` returns cleaned geometry: after it, applying the AFU result and
  running the DRC gives **0** issues of kinds `short, unrouted, unconnected, layer,
  layerRule, clearance, edge, drill, annularRing, holeToHole, via, traceWidth` and
  the same silkscreen issues as before routing (AFU: 16).
- On LFU and RegFile, the same kinds other than `unrouted` are 0 and `unrouted`
  equals `total − routed` from the result.
- Cleaning never makes a net longer (per net: cleaned length ≤ raw length) and
  never adds a via (cleaned vias ≤ raw vias); both are asserted and the ratios
  recorded.
- `RoutingVerifier` agrees with the DRC: on every fixture result and on injected
  violations it flags the same pieces of copper the DRC flags.

### Non-goals (explicit constraints)

- No new design rules and no change to the DRC. The verifier reads
  `DesignRules` and the same `CopperGeometry`.
- Kept (hand-drawn or locked) copper is never modified, only touched at its
  existing vertices or along its segments.
- No teardrops, no pad-entry shaping beyond a straight tie leg.
- No length tuning (I) or pair coupling (J).

## 2. Current State

- After plan-38-E/F a connection is a list of grid cells; `RoutedTrace`s are those
  cells with collinear steps merged (`Router.route`); ties from the first and last
  cell to a pad centre are the straight leg the converter emits, unchecked.
- The DRC's rules the router can break and where they live: clearance between
  copper of different nets (`checkClearance`: gap < `minClearance − 1e-4`), copper
  to board edge except fingers/holes (`checkEdge`), via drill/diameter minimums
  (`checkVias`), trace width (`checkTraceWidths`), hole-to-hole and annular ring
  (`checkHoles`); connection rules: a trace end must land on a pad on its layer, a
  via or another trace; a via touched by one thing "Goes nowhere"
  (`checkConnections`).
- Traces that touch on a layer join if a point of one is within 1e-4 mm of a
  segment of the other (`Ratsnest.swift:meet`); `join` (G) makes the shared vertex.
- Measured (`results.txt`): raw prototype AFU wire 1102–1153 mm and 110–156 vias
  at 0.127 mm pitch (quantized paths).

### Measured populations

| What | Count | Command |
|---|---|---|
| AFU DRC issues before routing | 131 = 115 `unrouted` + 16 silkscreen | `results.txt` fixture table (`drc 131`, `unrouted 115`); `planning/todo.md` "16 silkscreen" |
| LFU DRC issues before routing | 188 = 188 `unrouted` | `results.txt` (`drc 188`, `unrouted 188`) |
| RegFile DRC issues before routing | 32 = 24 `unrouted` + 8 other | `results.txt` (`drc 32`, `unrouted 24`) |
| DRC rule minimums in the Custom defaults | clearance 0.1, trace 0.1, via 0.5 / 0.2, edge 0.25 mm | `results.txt` header |

### Verified properties

- The DRC treats two traces of one net as one conductor and never flags them
  (`checkClearance`: `ra != rb` and equal nets skipped), so the verifier only
  compares copper of **different** connectivity groups/nets; same-net copper may
  touch.

## 3. Design Overview

**Pipeline** (after the negotiation of F, inside `Router.route`): net graph →
tie legs → chain optimization → verification → repair → result.

1. **Net graph.** Per net, every ledger polyline plus kept copper becomes vertices
   and edges. Where one polyline's end lies on another's segment (≤ 1e-6 mm) a
   vertex is inserted there. **Anchors** are: pad terminal points, via centres,
   attachment points on kept copper, and every vertex of degree ≠ 2. A **chain**
   is the run of degree-2 vertices between two anchors; only chains are rewritten,
   so a junction never moves.
2. **Tie legs.** A chain end at a pad is replaced by the exact pad centre; a chain
   end on a kept trace is the exact point on its segment (never a new vertex of the
   kept trace — `join` in G inserts that).
3. **Straighten.** From the chain's first vertex, take the farthest later vertex
   reachable by an **octilinear 1- or 2-segment route** (the straight line if it is
   0°, 45° or 90°, else a straight run plus one 45° leg, both orders tried) that is
   legal and not longer than the chain portion it replaces; repeat from there. With
   `angle: .orthogonal` the allowed routes are straight or L only.
4. **Chamfer** (`.octilinear` only): each 90° corner becomes two 45° bends when the
   cut is legal; cut = `min(0.5 × the shorter adjacent segment, 4 × width)`.
5. **Via hops.** A via pair with only a short chain between them, entering and
   leaving on the same layer, is replaced by one run on that layer when legal; a
   via whose one side has no copper is removed (and the stub with it).
6. **Stubs.** A chain that ends on an unanchored vertex, zero-length segments and
   collinear vertices are removed.
7. **Verify** (`RoutingVerifier`): against everything not of the net's group:
   `gap ≥ max(net clearance, rules.minClearance) − 1e-4` between all copper pairs
   (`CopperGeometry.nearPairs` / `gap`); `gap ≥ rules.minCopperToEdge − 1e-4` to
   the outline except exempt pads; `width ≥ rules.minTraceWidth`; via `drill ≥
   minViaDrill`, `diameter ≥ minViaDiameter`, ring `(diameter − drill)/2 ≥
   minAnnularRing`; hole-to-hole `≥ minHoleToHole` between via holes and drilled
   holes/pads; every trace end on a pad on its layer, a via or another trace of
   the net; every via touched by ≥ 2 copper items; every plane-fan-out via on its
   plane's net. A violation names the ledger connection it belongs to.
8. **Repair.** A violating chain is reverted to its pre-optimization polyline (the
   tie legs stay) and re-verified; if its raw form fails too, its connection is
   ripped and re-routed with a **margin** of `0.5 × pitch` added to its search
   profile (stored per connection in the ledger), using F's negotiation for the
   affected nets only; at most 3 attempts per connection; a connection still
   failing is left unrouted with the reason "could not be made legal under the
   design rules" in `NetOutcome`.

**Verifier ≡ DRC.** The verifier is written against the same functions as the DRC's
checks, and a parity test (below) runs both on the same copper. The DRC remains
the authority: the landing gate (plan-38-M) runs the real DRC on the applied
result.

Where correctness risk concentrates: step 3's legality test (a shortcut that cuts a
corner too close), and the repair loop terminating. Gate class: behavior; not
byte-identity.

Rejected: any-angle shortcuts (the Trace tool's angles are 20–90° but the board
look and the verifier's cost both favor octilinear); smoothing before the
graph (it would move junctions); a post-route DRC run on the document (the
engine must not need the main actor; the verifier is the same geometry, in
value types).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — net graph, ties, merge, stubs, and the verifier

- [x] `Routing/RoutingGraph.swift`: build the graph per net (vertex insertion at ends-on-segments, anchors, chains), tie legs, collinear merge, stub and zero-length removal; `Router.route` runs it on the F ledger. (In `Routing/RoutingCleanup.swift`, with Phases 2–3: `anchors(of:)`, chains between anchors, `tidy()`; tie legs are already exact from plan-38-E's `Router.copper`. `Router.route` → `Run.finishedCopper()` in `Routing/RoutingRepair.swift`.)
- [x] `Routing/RoutingVerifier.swift`: the checks of § 3 step 7 over a `RoutingProblem` + result geometry, returning `[Violation { kind, connection, detail }]`. (A violation names the result's trace and via indices; `finishedCopper` maps them to ledger entries. To tell pads, kept traces, vias, holes and the outline apart exactly, `RoutingObstacle` gained `kind` and the `drill` of pads and vias, set by `routingProblem`.)
- [x] Tests: `schematicalTests/RoutingGraphTests.swift` — a T junction inserts one vertex and anchors it; chains stop at anchors; a stub is removed; ties land on the exact pad centre; kept traces are unchanged. `schematicalTests/RoutingVerifierTests.swift` — a clean synthetic result has no violations; each rule (clearance 0.01 mm short of the minimum, edge, trace width 0.09 mm, via drill 0.15 mm, ring 0.04 mm, hole-to-hole, a trace end in free space, a one-sided via) yields exactly one violation of its kind. **Parity**: for each of those injected cases, apply the copper to a document and assert the DRC reports the matching kind on the same items. (Trace width: the app stores mil, 5 at least, so a 0.09 mm trace can't exist on a board; the case uses 5 mil under a 0.15 mm rule instead, the same check. Parity asserts the DRC's line is on the applied copper.) — `-only-testing:RoutingGraphTests -only-testing:RoutingVerifierTests` → 14 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: graph cases and every verifier rule pass; parity holds for all injected cases.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/RoutingGraphTests -only-testing:schematicalTests/RoutingVerifierTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: 24b641af (with Phases 2–3)

### Phase 2 — straighten and chamfer

- [x] `Routing/RoutingCleanup.swift`: `straighten`, `chamfer` per § 3 (steps 3–4), with the never-longer rule. (Legality is exact through a 1 mm tile index of every other net's copper, `CopperIndex`: gap ≥ the larger clearance, copper-to-edge, inside the outline.)
- [x] Tests (`RoutingCleanupTests`): a staircase of 10 diagonal cell steps becomes one 45° segment; an L around an obstacle keeps the corner clear of the obstacle's clearance and gets a chamfer only where legal; with `.orthogonal` no 45° segment appears; a shortcut that would pass 0.01 mm inside clearance is refused; cleaned length ≤ raw length on every case. (The refused shortcut is checked on both sides: `isLegal` false at 0.09 mm, true at 0.11 mm, and the cleanup takes it then.) — `-only-testing:RoutingCleanupTests` → 6 tests (Phases 2–3), 0 failures, `** TEST SUCCEEDED **`.

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/RoutingCleanupTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: 24b641af

### Phase 3 — via hops

- [x] `RoutingCleanup.swift`: `removeViaHops` per § 3 step 5.
- [x] Tests (`RoutingCleanupTests`): a Top→Bottom→Top hop over empty board is replaced by a single Top run and both vias vanish; the same hop under a blocking foreign trace on Top stays; cleaned vias ≤ raw vias. — in the 6-test run above. Every `RoutingCleanupTests` case also asserts never longer, never more vias, and no violation from the verifier.

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/RoutingCleanupTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: 24b641af

### Phase 4 — repair loop and the DRC guarantee

- [x] `Router.swift`: verify → revert chain → margin re-route (≤ 3 attempts, per-connection margin in the ledger) → report; `NetOutcome` gains `reason`. (In `Routing/RoutingRepair.swift`, `Run.finishedCopper()`: a faulted connection goes back to its raw copper as a whole; if the raw copper faults too it is ripped and routed again on a grid where its net is `0.5 × pitch × attempt` wider each side, stored at its true width. A net the margin route can't rejoin is marked, and its outcome carries `Router.illegalReason`. Margins are per attempt, not stored per connection: a re-routed connection is a new ledger entry.)
- [x] Tests (`RouterFixtureTests`, extended): for AFU, LFU and RegFile with `.balanced`: `applyRouting` then `designRuleCheck(rules: .customDefault)`: count of issues by kind — AFU: 0 of the twelve kinds of § 1 and 16 silkscreen; LFU and RegFile: 0 of the eleven kinds other than `unrouted`, `unrouted == total − routed`; per-net cleaned length ≤ raw length and cleaned vias ≤ raw vias; print wire mm, vias, vertex count before and after, and seconds. A synthetic board whose only legal route needs margin (a 0.12 mm channel at 0.127 mm pitch, 0.1 mm clearance) is completed by repair or reported with the reason, never DRC-dirty. (The fixtures' raw copper is already legal, so the repair branches are covered by `RoutingRepairTests`: a connection injected straight through another net's pad is re-routed with a margin, legal; one that can't be is left unrouted with the reason; `reverting` swaps an entry's pieces; the 0.12 mm channel is never dirty.) — `-only-testing:RouterFixtureTests/testNegotiationOnTheFixtures -only-testing:RoutingVerifierTests -only-testing:RoutingCleanupTests` → 16 tests, 0 failures, `** TEST SUCCEEDED **` (549 s); `-only-testing:RoutingRepairTests` → 4 tests, 0 failures.
- [x] Results (the output of this run, filled in then): AFU wire 1614 mm → 1582 mm, vias 115 → 115, vertices 928 → 834 (115/115 routed, 27.6 s; DRC after apply: 16 silkscreen, nothing else); LFU wire 2578 → 2528 mm, vias 252 → 246, vertices 1868 → 1592 (147/188 routed, 484 s; DRC: 41 unrouted = 188 − 147, nothing router-caused); RegFile wire 315 → 314 mm, vias 31 → 31, vertices 216 → 204 (24/24, 32.5 s; DRC: 8 silkscreen, nothing else).

Acceptance: the DRC-clean guarantee holds on all three fixtures; the never-longer and never-more-vias asserts hold. If a DRC issue of a router-caused kind appears, find which verifier rule disagreed with the DRC (parity case first) and fix the verifier or the repair; the gate is not relaxed.
  Check: `-only-testing:schematicalTests/RouterFixtureTests -only-testing:schematicalTests/RoutingVerifierTests -only-testing:schematicalTests/RoutingCleanupTests` → `** TEST SUCCEEDED **` (est. 6 min: three fixture routes, Balanced LFU ≈ 10 s each).
Commit: 8ea81c80

## Validation Plan

- Tests: `RoutingGraphTests`, `RoutingVerifierTests`, `RoutingCleanupTests`, `RouterFixtureTests`, `DesignRuleGoldenTests` and `DesignRuleCheckTests` (unchanged).
- Coverage check: every verifier rule, and the revert and margin branches of the repair loop, each have a test that fails if the branch is removed.
- Runtime proof: Phase 4's DRC-after-apply on three real boards.
- Doc sync: none yet.
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: octilinear only; anchors never move; never longer / never more vias; the DRC is the authority and the verifier is held to it by parity tests; margin re-route up to 3 attempts, then report.

## Corrections

- **Hole to hole was not kept by the grid.** PCBWay's minimum is 0.4064 mm:
  two vias of one net closer than about 0.81 mm between centres, or a via
  beside its own net's through-hole pad, break it, and copper clearance only
  keeps *other* nets apart. Fixed at the source (plan-38-E's grid): every
  hole of any net, a pad's drill or a via's, blocks the via maps of every net
  within `drill/2 + minHoleToHole + the profile's via drill/2`
  (`RoutingGrid.stampHole`, called from the obstacles and `stampVia`), and
  `RoutingState.reach` covers that distance so un-stamping stays exact.
  `RoutingProfile` gained `viaDrillRadius`.
- Measured with the whole pipeline (2026-11-14, probe deleted): AFU and
  RegFile raw copper already pass the verifier (0 violations), and after
  apply the DRC lists only the silkscreen issues they had before routing
  (AFU 16, RegFile 8). Cleanup: AFU wire 1613 → 1581 mm, vias 115 → 115;
  RegFile 315 → 313 mm, 31 → 31.
- `RouterTests.testEachClassKeepsItsOwnClearance` had an extra assertion of
  mine, that two Default nets *happen* to run closer than the Power
  clearance; once cleaned they didn't (0.446 mm apart, legal). Replaced with
  what it meant: the grid keeps a Default net 0.1 + half widths from a
  Default net's copper and 0.3 + half widths from the Power net's.

## Summary

This letter turns "mostly routed" into "provably legal": the risk is a verifier
that disagrees with the DRC, which the parity tests exist to catch. Untouched:
the document, the UI, groups.
