# plan-52-D: A Pour Joins Its Net

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-52-C (complete)

plan-52-C computes a pour's fill and which pads, vias and traces of its net each piece touches. This sub-plan makes the board
believe it: `connectivity()` joins every node a kept piece touches, so the ratsnest draws no unrouted line between pour-joined
pins, the design rule check and the net list count them connected, and the autorouter sees them as one component. It also keeps
the fills (computed once per change) so the canvas, Board Preview and the exports read them, and keeps a drag smooth by not
refilling while a mouse gesture moves something the fill does not depend on.

The single outcome: with a ground pour on a layer over two ground pads, `ratsnest()` has no line between them, undo of the pour
brings it back, and a document with no pour builds exactly the connectivity it built before.

References:

- plan-52-A § 3, plan-52-C § 4 (`PourFill`, `PourCopper`, `PourNode`)
- `schematical/schematical/Canvas/Ratsnest.swift`: `connectivity(drawing:excludingTraces:excludingVias:)` (`:56`), `buildConnectivity`
  (`:68`, the plane join at `:303`), `BoardConnectivity` (`:543`), `planeClearances` (`:615`), `viaNode`/`traceNode`
- `Canvas/SchematicModel.swift`: `copperRevision` (`:1345`), `isGestureOpen` (`:1841`), `beginGesture`/`endGesture` (`:1924`/`:1933`)
- `Routing/RoutingProblemBuilder.swift` (`componentRoots` at `:50`, `planes` at `:60`)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-C is complete | `ls planning/completed \| grep -c '^plan-52-C'` → 1 | MET (2026-11-21: plan-52-C landed as 9d09816b) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `BoardConnectivity.pourFills: [UUID: PourFill]` holds the fill of every pour with a net; `copper.pourNet(of:)` names a pour's net.
- In `connectivity()`, the nodes of one kept piece (`PourFill.joined[i]`) are one group: `root` is equal across them.
- `connectivity()` is still built once per `copperRevision`; the fills are computed in that build, not on every redraw.
- While a gesture is open and no pour or keep-out changed, the build reuses the last fills; `endGesture()` makes the next build refill.
- The builds the router and the trace tool make (`excludingTraces`, `excludingVias`, `drawing`) join with the kept fills.
- A document with no pours produces exactly the `BoardConnectivity` it did (same `root`, `netOfPad`, `planeNets`).

### Non-goals (explicit constraints)

- The fill itself (plan-52-C) and any drawing (E, F) or export (G) are not touched.
- Planes are unchanged: `planeNets`, the plane join at `Ratsnest.swift:303` and `planeClearances` stay; pours are on trace layers only.
- No change to the router's search or to `RoutingProblem`'s fields; it sees the pours only through `root`.
- No per-pour fill cache (all pours refill together; measured 24–275 ms for the whole feature's worst board).

## 2. Current State

- `buildConnectivity` makes nodes (pads, then traces, then vias), joins by union-find (`parent`, `find`, `union`, `:132`–`:141`),
  names nets, joins Plane nets (`:303`–`:323`) and returns `BoardConnectivity(… root: parent.indices.map(find) …)` (`:326`).
  `root` is a `let` (`:563`).
- The plain `connectivity()` is kept by `copperRevision` in `keptConnectivity` (`:61`–`:64`); calls with `drawing`, `excludingTraces` or
  `excludingVias` build fresh (`:58`).
- Callers of `connectivity(`: 22 sites in 12 files (`grep -rn "connectivity(" schematical/schematical | grep -v "func connectivity" | wc -l` → 22,
  run 2026-11-21); the router builds `connectivity(excludingTraces:excludingVias:)` once per run
  (`RoutingProblemBuilder.swift:46`), `RoutingApply.unroute` and `RoutingScope.fromSelection` call the plain one.
- `endGesture()` (`SchematicModel.swift:1933`) closes the gesture and records the step; it bumps no revision.

### Measured populations

| What | Count | Command |
|---|---|---|
| `connectivity(` call sites | 22 in 12 files | `grep -rn "connectivity(" schematical/schematical \| grep -v "func connectivity" \| wc -l` → 22; `grep -rln … \| wc -l` → 12 |
| `BoardConnectivity(` initializer calls in `Ratsnest.swift` | 2 (the `names` stub and the real return) | `grep -n "BoardConnectivity(" schematical/schematical/Canvas/Ratsnest.swift` → `:271`, `:326` |

### Verified properties

- A through-hole pad is on every layer and a surface-mount pad on its side: `padThrough`/`padLayer` (`Ratsnest.swift:166`–`:167`), the rule the
  adapter below repeats for `PourCopper.layers`.
- `BoardCopperCacheTests.assertFresh` compares the kept connectivity with `connectivity(excludingTraces: [UUID()])` (a fresh build
  that leaves nothing out): with pours the two must still agree, so a non-plain build must use the same fills the plain one stored.

## 3. Design Overview

Layering: `buildConnectivity` builds nodes and unions as today, joins Planes, then (only if `!pours.isEmpty`) makes a provisional
`BoardConnectivity`, asks `pourFills(for:)` for the fills, unions each kept piece's `joined` nodes, and returns the final
`BoardConnectivity` with `root` recomputed and `pourFills` set. Same-net determination uses the *pre-pour* roots (a trace's net is its
root's pad net), and a pour never joins two different nets, so one pass is enough: a join cannot change a net.

`pourFills(for:)`:

| Situation | Fills used |
|---|---|
| plain build, no gesture open | computed now; stored in `keptPourFills` |
| plain build, gesture open, kept fills exist, `pours` and `keepouts` equal the kept ones | the kept fills (stale copper, no cost) |
| plain build, gesture open, a pour or keep-out changed (a resize drag) | computed now; stored |
| `drawing` / `excludingTraces` / `excludingVias` build | the kept fills if any, else computed (not stored) |

`endGesture()` bumps `copperRevision` when `pours` is non-empty, so the first build after a release refills.
A fill names nodes by `PourNode` (`pad(index)`, `trace(id)`, `via(id)`); a node that no longer exists (a trace the router left out)
is skipped.

**Where correctness risk concentrates:** this is the one change to `root`, which the DRC, ratsnest, net list and router all read.
Mitigations: a document with no pour skips the code entirely (same bytes, tested); the join is by same net only (tested: a pour never
joins pads of another net); the stale-fill rule is tested.
**Byte-identity is a gate only for the no-pour case** (`BoardCopperCacheTests`, `PlaneNetTests`, `RoutingProblemTests` unchanged and
passing); with pours, behavior changes by design (the ratsnest shrinks) and the tests below name the expected differences.
**What was probed:** nothing new beyond plan-52-C's measurements; the router's use of `root` was read (`RoutingProblemBuilder.swift:46`–`:53`:
components are the pads, traces and vias that share a root).

Rejected alternatives: *joining by a cheap geometric test (pad inside the rectangle)*: wrong for enclosed pads and keep-outs and would
disagree with the drawn fill; *computing fills lazily in the renderer*: connectivity needs them, and the renderer must show what
connectivity used; *refilling on every gesture frame*: 24–275 ms per frame on a dense board.

## 4. Detailed Design

`Canvas/PourConnectivity.swift` (new), an extension of `SchematicDocument`:

- `func pourCopper(for copper: BoardConnectivity) -> [PourCopper]`: per pad index `p`: `net = copper.netOfPad[p]`, `layers` = `nil` for a
  plated through-hole pad, `[side.copperLayer]` for a plated surface-mount pad, `isHole = true` for a non-plated pad with `drill > 0`
  (shape = its `holeRect` rounded rect), `padRect` = the pad's rect when plated, `shape` = `PCBCanvasView.outline(of:).cgPath`, samples =
  centre, four corners, four edge midpoints; per via: net from the root, `layers` nil, circle, centre plus four rim points; per trace:
  net from the root, `layers` `[trace.layer]`, stroked polyline, samples = vertices and segment midpoints. Net of a node by root:
  `copper.netOfRoot()[copper.root[node]]`.
- `func pourFills(for copper: BoardConnectivity, plain: Bool) -> [UUID: PourFill]`: the table of § 3; resolves each pour's net with
  `copper.net(of:)`, calls `PourFiller.fill(pours:nets:keepouts:copper:outline: board.outline, edge: board.rules.minCopperToEdge, rules: board.rules)`.
- In `Ratsnest.swift`: `BoardConnectivity.root` becomes `var` (or the struct is rebuilt), `var pourFills: [UUID: PourFill] = [:]`,
  `func pourNet(of pour: BoardPour) -> Int?`; `keptPourFills` beside `keptConnectivity` (`@ObservationIgnored`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — The adapter and the join in a plain build

- [x] `Canvas/PourConnectivity.swift`: `pourCopper(for:)` and `pourFills(for:plain:)` as § 4 (plain rows of the table only).
- [x] `Ratsnest.swift`: `pourFills`, `pourNet(of:)`, `root` rebuilt after the join; after the plane join (`:323`), if `!pours.isEmpty`: provisional
      `BoardConnectivity`, fills, `union` of each `joined` list (map `PourNode` → node index; skip unknown), final struct with `pourFills`.
- [x] Tests: new `schematicalTests/PourConnectivityTests.swift`, using `PlaneNetTests`' two-LED board (copy its private `board()` helper): (a) a
      ground pour over the whole 4-layer board on Top: `ratsnest().count` drops from 2 to 1 (the anodes stay) and the two cathode pads share a
      `root`; (b) the same pour on net `5V` joins no pad; (c) a pour with no net joins nothing; (d) a keep-out over one LED's cathode pad
      (on Top) leaves that pad unjoined: `ratsnest().count` is 2; (e) a pour on Bottom joins through-hole pads (they are on every layer);
      (f) undo of `addPour` restores `ratsnest().count == 2`; (g) a pour never changes `netOfPad`.

Acceptance: (a)–(g) hold.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourConnectivityTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min). Observed (all three phases' suites, one run): `Executed 54 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: COMMIT_D

### Phase 2 — Kept fills, gestures and the other builds

- [x] `Ratsnest.swift`/`SchematicModel.swift`: `keptPourFills: (revision: Int, pours: [BoardPour], keepouts: [BoardKeepout], fills: [UUID: PourFill])?`;
      the full table of § 3 in `pourFills(for:plain:)`; `endGesture()` bumps `copperRevision` when `!pours.isEmpty`.
- [x] Tests (`PourConnectivityTests`): (h) inside `beginGesture`, moving a footprint (a frame) does not call the filler (count the calls with a
      static counter on `PourFiller`, reset per test) and `endGesture()` then the next `connectivity()` calls it once; (i) inside a gesture,
      `setPourRect` makes the next `connectivity()` call it; (j) `connectivity(excludingTraces: [UUID()])` equals `connectivity()` field by field
      with a pour present (extend `BoardCopperCacheTests.assertFresh` to compare `pourFills.mapValues(\.pieces.count)` too); (k) a no-pour document:
      the filler is never called.
- [x] Extend `BoardCopperCacheTests.testKeptResultsFollowEveryEdit` with a pour added before the footprint move and checked with `assertFresh`.

Acceptance: (h)–(k) hold and the extended cache test passes.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourConnectivityTests -only-testing:schematicalTests/BoardCopperCacheTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min). Observed: as Phase 1.
Commit: COMMIT_D

### Phase 3 — The router and the rest see it

- [x] Test `PourConnectivityTests` (l): on the two-LED board with a Top ground pour, `RoutingProblemBuilder`'s problem for the ground net has
      one component (build it as `RoutingProblemTests.testNetsScopeGivesThatNet` does) and no problem net for ground is routable; without the pour it has two.
- [x] Test (m): `designRuleCheck` of the same board reports no `.unrouted` issue for ground with the pour, one without it (`DesignRuleCheckTests` style).
- [x] If either fails because a reader of `root` ignores `pourFills`, fix the reader; record it under Corrections.

Acceptance: (l) and (m) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourConnectivityTests -only-testing:schematicalTests/RoutingProblemTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/PlaneNetTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 9 min). Observed: as Phase 1 (`PourConnectivityTests` 11 cases, `BoardCopperCacheTests`, `PlaneNetTests`, `RoutingProblemTests`, `DesignRuleCheckTests`, `NetListTests`).
Commit: COMMIT_D

## Validation Plan

- Tests: `PourConnectivityTests` (new, 11 cases), `BoardCopperCacheTests` (extended), `PlaneNetTests`, `RoutingProblemTests`, `DesignRuleCheckTests` (unchanged, must pass).
- Coverage check: `grep -c "pourFills" schematical/schematical/Canvas/Ratsnest.swift` → at least 4, all exercised by `PourConnectivityTests`.
- Runtime proof: plan-52-E (the first screen).
- Doc sync: `docs/routing.md` § (the fan-out to Planes row, line 70) gets one sentence: a net a pour joins needs no route there.
- Final gate (once, after Phase 3): `xcodebuild … test -only-testing:schematicalTests/PourConnectivityTests -only-testing:schematicalTests/BoardCopperCacheTests -only-testing:schematicalTests/PlaneNetTests -only-testing:schematicalTests/RoutingProblemTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/NetListTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).

## Decisions

- A pour joins the nodes it touches, so a pin the pour reaches needs no route — decided by the author, from the owner's "flood fill … keep the buffer from nets and pins" (2026-11-21): the flood is copper, so it connects.
- The fills are refilled on every change, but not while a gesture moves what they do not depend on — decided by the author: measured fill time is up to 275 ms; a frame must not pay it.
- No-pour documents skip the whole path — decided by the author: the byte-identity gate for old documents.

## Corrections

- The suite has 11 cases, not 13: (a)–(g) of Phase 1 became five tests (`testGroundPourOnTopJoinsTheGroundPads` holds (a) and (g); `testAPourOnAnotherNetJoinsNoPad` holds (b) and (c)), and (h)–(k) four. `grep -c "func test" schematical/schematicalTests/PourConnectivityTests.swift` → 11.
- The two-LED board of `PlaneNetTests` leaves the footprints where the app puts them, which is off a 100 × 80 mm board; the pour tests move them to (30, 40) and (60, 40) first, as `RoutingProblemTests.wiredPair` does. Without that the pour's fill touched no pad and every case failed.
- Test (m)'s "no `.unrouted` line" is asserted as one fewer `.unrouted` issue (the anodes stay unrouted), and (l) as the ground net being absent from `routingProblem(…).nets` (a net with one component has nothing to route).
- Phase 3's "fix the reader if it ignores `pourFills`" did not trigger: the router, the DRC and `netGroups` read `root`, which carries the join.

## Summary

The risk is `root`: every reader of connectivity now sees pour joins. The no-pour path is untouched, the join is same-net only, and
the router, DRC and net list are re-run before anything draws a pour. Untouched: the fill, Planes, the router's search.
