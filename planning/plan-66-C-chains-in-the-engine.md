# plan-66-C: Push and shove — existing copper in the engine, the result and the document

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-66-B

C makes the engine **hold** the chains B extracts: each becomes a ledger entry
of a "bystander" net that a push can move but a rip-up never touches, untouched
ones are frozen through cleanup, and the result lists the chains that moved.
The document side lands too: `applyRouting` splices a moved chain's new points
into the `BoardTrace` polylines it came from, as part of the same single undo
step. Nothing in the engine *moves* a chain yet (D does); C proves, with chains
held and never moved, that the router behaves exactly as it does with the same
copper as fixed obstacles, and proves the apply path with hand-made results.

References:

- `planning/completed/plan-66-A-push-option-anchors.md` (§ 3, § Prerequisites, § Decisions),
  `plan-66-B-movable-chains.md` (the types).
- `Routing/RoutingState.swift` (`add`, `remove`, `rip`, `victims`, `groups`),
  `RoutingGrid.swift` (`init`, `stamp`, `restoreFixed`), `RoutingSearch.swift:244-266`
  (`price`), `Router.swift:437-515` (`join`), `RoutingPolish.swift:16,47`,
  `RoutingRepair.swift:6-58` (`cleanupInput`, `finishedCopper`),
  `RoutingApply.swift:33-71`.

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The gate for the whole feature is in plan-66-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-66-B is complete and archived | `ls planning/completed/plan-66-B-* \| wc -l` → 1 | MET (2026-11-25, in `worktree-P-66`: 1) |
| `MovableChain` exists | `grep -c 'struct MovableChain' schematical/schematical/Routing/RoutingProblem.swift` → 1 | MET (2026-11-25, in `worktree-P-66`: 1) |

If plan-66-B is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- With `pushAndShove` on and chains in the problem, `Router.route` on any board
  gives the same result as the same board with those chains as fixed obstacles
  (traces, vias, stats but the new counters), because nothing yet moves a chain.
- `RoutingResult.moved` lists the chains whose copper changed, in chain order;
  new copper (`traces`, `vias`) never contains a chain's copper.
- `applyRouting` replaces a moved chain's points in the document inside the one
  "Autoroute" undo step and leaves the board's connectivity as it was.

### Non-goals (explicit constraints)

- Nothing moves a chain: no `push`, no corridor, no change to `join` or
  `attempt` beyond refusing to rip a chain entry (D and E).
- Copper of the run's own nets is never a chain (A § 3).
- No change to a problem with no chains: every existing test passes unchanged and
  the A anchors hold (that is this letter's expected-identity gate).
- No file-format change; no automation.

## 2. Current State

**What the relaxed search crosses.** `price(layer, cell)` returns 0 for a free or
own cell and, in a relaxed search, `present × (1 + history)` for a cell the
*fixed* map leaves free or own while the live map has another net's copper — that
is, only ledger copper; fixed copper returns -1 (`RoutingSearch.swift:258-263`,
read). A chain held as a ledger entry is therefore crossable, which is what D
needs and what C must keep switched off until D asks for it.

**The ledger.** `RoutingState.add` stamps an entry and indexes it by tile;
`remove` un-stamps exactly (`restoreFixed` then re-stamp the entries near); `rip`
removes entries and the entries of their nets left hanging
(`RoutingState.swift:113-125,148-191`). `groups(of:)` and `dangling(of:)` return
nothing for a net with no terminals (`guard let terminals = terminals[net]`,
`:216,244`), so an entry of a net with no components is invisible to them — the
property that lets a bystander's entries sit in the ledger beside routed nets.
`RoutingState` is a value type; restoring is assignment
(`RoutingPolish.swift:51,70`).

**Cleanup.** `cleanupInput()` builds one `RoutingCleanup` from every ledger entry
and sets `frozen`/`frozenVias` for a coupled pair's copper so cleanup does not
touch it (`RoutingRepair.swift:15-19`); `finishedCopper()` verifies the cleaned
copper with `RoutingVerifier` and sends violating entries back to raw, then
rips or re-routes them (`:28-58`). Both treat every entry as new copper.

**Apply.** `applyRouting` removes the cleared copper, appends vias and traces as
`auto`, joins each new trace, selects what was added, all in one
`recordEdit(.pcb, "Autoroute")` (`RoutingApply.swift:33-71`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Call sites of `state.rip(` | 6 | `grep -rn 'state.rip(' schematical/schematical/Routing \| wc -l` → 6 |
| Files that call `finishedCopper()` | 6 | `grep -rln 'finishedCopper()' schematical \| wc -l` → 6: `Router.swift`, its definition in `RoutingRepair.swift`, and four tests (`RouterFixtureTests`, `RoutingPlanTests`, `RoutingPolishTests` ×4 calls, `RoutingRepairTests` ×2); every test use is `copper.traces` / `copper.vias` (read), so a struct with those two properties is source-compatible |
| Test files using `RoutingCheck.violations` | 7 | `grep -rln 'RoutingCheck.violations' schematical/schematicalTests \| wc -l` → 7; the helper is updated once, in `RouterTests.swift` |

### Verified properties

- **A bystander's entries do not disturb the routed nets' bookkeeping.** `groups`
  and `dangling` return early for a net absent from `terminals`
  (`RoutingState.swift:216,244`, read); `terminals` is built only from
  `problem.nets` (`:53-60`).
- **Cleanup can leave pieces alone.** `cleanup.frozen`/`frozenVias` are index sets
  of traces and vias cleanup skips (`RoutingRepair.swift:18-19`, read).
- **A grid with copper as entries equals one built from scratch with it as
  obstacles.** That is the invariant `RoutingState`'s own doc states and tests
  (`RoutingStateTests`); Phase 1 extends it to chains and tests it.

## 3. Design Overview

### The engine holds chains as entries

- `RoutingGrid.init` builds profiles and clearances over `problem.nets +
  problem.bystanders` and records `grid.isBystander: [Bool]` indexed by net id.
- `LedgerEntry` gains `var chain: Int?` (the `MovableChain.id` it is) and
  `var touched = false`. `RoutingState.init` ends with one `add` per chain, before
  the run does anything: `path = [nodeA, nodeB]` (the end points' nodes on the
  end layers), `traces` the runs as `RoutedTrace` in chain order, `vias` as
  `RoutedVia`, `allowed` every layer, `chain: id`. `netsByID` includes the
  bystanders. A `RoutingState.replace(_ id: Int, traces:, vias:, touched:)` swaps
  an entry's copper for other copper exactly (un-stamp, stamp), keeping its id,
  net, path, nodes and chain.
- **A chain cell is crossable only on request.** `SearchRules` gains
  `var pushPrice: Double = 0`. In `RoutingSearch.price`, a relaxed search that
  finds a cell whose live owner is a bystander net returns -1 when `pushPrice` is
  0 and `pushPrice × (1 + history)` otherwise; a hard search never crosses it.
  With `pushPrice` 0 — C, and every search until D — a chain behaves as fixed
  copper.
- **A chain entry is never ripped.** In `Router.Run.join` (rip branch, both the
  negotiation and the improvement form) the victims `all` must contain no entry
  with `chain != nil`, else no rip happens; `polishable(net)` is false for a net
  in `bystanders`; the improvement's loops over `state.ledger` skip chain entries.

### Cleanup, verification, repair

- `cleanupInput()` freezes every chain entry whose `touched` is false (their
  traces and vias, by index). A touched one is cleaned like new copper: its anchors
  are its ends, which cleanup already treats as fixed.
- `finishedCopper()` verifies all entries (so a pushed chain is checked against
  the new copper). A violation is **attributed only to entries that are new or
  touched**: a violation whose every owner is an untouched chain is dropped (the
  user's existing board may already break a rule; the router does not repair it).
  When a *touched* chain entry is still illegal after reverting to its raw
  copper, `state.replace` puts the chain's original copper back (`touched = false`)
  and the loop re-verifies: the violation is now blamed on the new copper, which
  the existing path (revert, `reroute`, rip) handles.
- `finishedCopper()` returns a `FinishedCopper` with `traces`, `vias` (new copper
  only: entries with `chain == nil`) and `moved: [MovedChain]` (touched chain
  entries, cleaned).

### The result

```swift
nonisolated struct MovedChain: Hashable, Sendable {
    var chain: Int                 // MovableChain.id
    var runs: [[CGPoint]]          // new points of each run, in chain order
    var vias: [CGPoint]            // new centre of each via, in chain order
}
```
`RoutingResult.moved: [MovedChain] = []`; `stats.movedTraces` = runs moved,
`stats.movedVias` = vias moved (set from `moved`). `Router.Run.result` (the raw
ledger form) and the live preview snapshot exclude chain entries.

### Apply

`applyRouting`, inside the same `recordEdit`, after the cleared copper is removed
and before new copper is added, for each `MovedChain` in order: for each run `k`,
find the `BoardTrace` by `runs[k].trace`; **skip the whole chain** and count it in
`summary.skipped` when the trace is gone, locked, or its points at
`first...last` no longer equal the run's recorded points; otherwise replace the
vertices `first...last` by the new points (reversed back when `run.reversed`),
keeping the id, layer, width, `locked` and `auto` of the trace; set each via's
`centre`. No `join` (a chain keeps its anchors, so it keeps its contacts).
`RoutingSummary` gains `movedTraces` and `movedVias`. Moved items are not added
to the selection.

### Where correctness risk concentrates

(1) That crossing is really off: a test builds a relaxed search over a chain and
over the same copper as an obstacle and the two must agree. (2) The splice: the
resulting polyline must have no zero-length segment (`TraceRoute.isLegal`) and
the document's connectivity must be unchanged (`netGroups()` before = after).
(3) `finishedCopper`'s attribution: an untouched chain that the user's board
already makes illegal must not be "repaired".

**Gate class:** problems with no chains are **provably neutral** (the A anchors
and every existing router test pass unchanged); with chains and no push, the
result must equal the fixed-obstacle result — an identity test. Expected
differences: none until D.

**Rejected alternatives.**

- *Keep chains as fixed copper and un-fix them when pushed.* Fixed maps are
  restored by `restoreFixed` from a snapshot taken at init; making them mutable
  adds a second restoration path beside the ledger's. Entries already have exact
  stamp/un-stamp and tile lookup.
- *A "pushable" set per grid cell.* Per-profile maps make that an array the size
  of the maps; the owner check (`value` is a bystander's id) needs one `[Bool]`.
- *Rip a chain when the push fails.* A rip loses the user's copper if the
  connection that wanted it then fails; a chain is never removed.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. An unticked box means NOT DONE.

### Phase 1 — chains as entries; crossing off

- [x] (done) `Routing/RoutingState.swift`: `LedgerEntry.chain: Int?`, `touched: Bool`;
      `add(…, chain: Int? = nil)`; `replace(_:traces:vias:touched:)`;
      `init` seeds one entry per `problem.movable` as in § 3 and puts
      `problem.bystanders` in `netsByID`.
- [x] (done; a per-cell mask instead of `isBystander` — see Corrections) `Routing/RoutingGrid.swift`: profiles and `clearanceOfNet` over `nets +
      bystanders`; `let isBystander: [Bool]`.
- [x] (done) `Routing/RoutingState.swift`, `reach` (`:134`): the `largest` term runs over
      `problem.nets + problem.bystanders`, so un-stamping a wide chain restores the
      whole area it covered.
- [x] (done; `Maps` carries the two masks, not a Bool buffer) `Routing/RoutingSearch.swift`: `SearchRules.pushPrice`; `Maps` carries an
      `UnsafeBufferPointer<Bool>` for `grid.isBystander`; `price` as in § 3.
- [x] (done: `rip` skips chain entries, `join` refuses a way through one, `polishable`, the polish loop, the snapshot and `Run.result`) `Routing/Router.swift`, `RoutingPolish.swift`: the three "never ripped"
      rules of § 3.
- [x] (done; plus `tracesAsChains()`) `schematicalTests/RoutingTestBoard.swift`: `mutating func chain(_ points:
      [CGPoint], width: Double = 0.254, layer: CopperLayer = .top, net: Int,
      from: Int? = nil, to: Int? = nil)` adding a `MovableChain` of one run whose
      ends are the pads `from` and `to` (indices into `pads`), or a free end at
      the first / last point when nil (a wall that no pad holds), and a bystander
      record for its net; `problem()` fills `movable` and `bystanders`, and leaves the chain out of
      `obstacles`. `static func chainAsObstacle(_ board:)` returns the same board
      with `trace(points, net:)` instead, for the identity tests.
- [x] (done: 6 tests) `schematicalTests/RoutingChainEngineTests.swift` (new):
      `testTheGridWithChainsEqualsTheGridWithObstacles` (`RoutingState(problem:).grid
      .traceMaps` and `viaMaps` equal `RoutingGrid(problem:)` of the obstacle form,
      for the `twoGaps` board with its wall as a chain);
      `testARelaxedSearchDoesNotCrossAChainUnlessPriced` (search over a cell a
      chain covers: -1 / no path at `pushPrice 0`; a path at `pushPrice 5`);
      `testRoutingWithChainsEqualsRoutingWithObstacles` (`Router.route` at `.fast`
      and `.balanced` on `twoGaps()` and `reversedChannel()` with their walls as
      chains and as obstacles, and on `parallelRows()` with a net-5 chain
      `chain([(2, 2), (5, 2)], net: 5, from: 6, to: 7)` between two extra net-5 pads
      (indices 6 and 7: `parallelRows` has six) in the corner: equal traces, vias and stats but the new counters); `testAChainEntryIsNeverRipped` (a board where the only
      relaxed way crosses a chain: the connection stays unrouted and the chain's
      entry is in the ledger with `touched == false`).

Acceptance: the four tests pass and the A anchors still pass.
  Check: `XB -only-testing:schematicalTests/RoutingChainEngineTests
  -only-testing:schematicalTests/RoutingAnchorTests
  -only-testing:schematicalTests/RoutingStateTests
  -only-testing:schematicalTests/RoutingGridTests` → `** TEST SUCCEEDED **`
  (est. 3 min).
  Measured: `Executed 25 tests, with 0 failures` (RoutingChainEngineTests 6, RoutingAnchorTests 9, RoutingStateTests 4, RoutingGridTests 6), `** TEST SUCCEEDED **`.
Commit: e20e9922

### Phase 2 — cleanup, the result

- [x] (done; `FinishedCopper` also carries `chainTraces`/`chainVias`) `RoutingRepair.swift`: `cleanupInput()` freezes untouched chain entries;
      `finishedCopper()` returns `FinishedCopper` (`traces`, `vias`, `moved`),
      attributes violations as in § 3, restores a chain that stays illegal.
- [x] (done) `RoutingRepair.swift`, `reroute(_:margin:)` (`:87-130`): its temporary run is
      built from `wide = problem` and then fed the real ledger; with chains that
      would seed every chain twice and lose a touched chain's new geometry. Set
      `wide.movable = []` and `wide.options.pushAndShove = false`, and copy every
      ledger entry with `chain: entry.chain, touched: entry.touched` (`add` takes
      both); entries it adds back to the real state are the new ones as before.
- [x] (done) `RoutingResult.swift`: `MovedChain`, `RoutingResult.moved`.
- [x] (done) `Router.swift`: `Router.route` builds the result with `moved` and the two
      stats; `Run.result` and the progress snapshot skip chain entries.
- [x] (done) `schematicalTests/RouterTests.swift` (`RoutingCheck`): `violations(result,
      problem)` also treats every chain of `problem.movable` as an obstacle at its
      final geometry (moved copper if listed in `result.moved`, else original) and
      checks `moved` against the other copper.
- [x] (done: 4 in `RoutingChainEngineTests`, 1 in `RoutingRepairTests`) Tests in `RoutingChainEngineTests`: `testAnUntouchedChainIsFrozenAndUnlisted`;
      `testATouchedChainIsCleanedAndListed` (the test calls `state.replace` with a
      staircase path between the chain's ends and `touched: true`, runs
      `finishedCopper`, and expects one `MovedChain` whose run has fewer vertices);
      `testAChainThatStaysIllegalGoesBack` (replace with copper that breaks the
      clearance to a pad; expect `moved` empty and the original geometry
      restored); `testAnIllegalUntouchedChainIsNotRepaired` (an existing chain that
      already breaks clearance to another net's pad: no entry is ripped, `moved`
      empty);
      `testRepairRerouteKeepsChainEntries` (in `RoutingRepairTests`, the board of its
      test at line 45 with a chain added far from the routes: after `reroute`, the
      ledger holds each chain entry once, with its `touched` flag).

Acceptance: the five tests pass; the A anchors and every router test still pass.
  Check: `XB -only-testing:schematicalTests/RoutingChainEngineTests
  -only-testing:schematicalTests/RoutingAnchorTests
  -only-testing:schematicalTests/RouterTests
  -only-testing:schematicalTests/RoutingRepairTests
  -only-testing:schematicalTests/RoutingCleanupTests` → `** TEST SUCCEEDED **`
  (est. 4 min).
  Measured: `Executed 41 tests, with 0 failures` (RoutingChainEngineTests 10, RoutingAnchorTests 9, RouterTests 7 + RoutingRepairTests 5 + RoutingCleanupTests 10), `** TEST SUCCEEDED **`.
Commit: — (recorded in the next commit)

### Phase 3 — apply (the splice)

- [ ] `Routing/RoutingApply.swift`: the splice of § 3 inside `applyRouting`;
      `RoutingSummary.movedTraces`, `movedVias`.
- [ ] `schematicalTests/RoutingApplyTests.swift`: on the RegFile fixture with its
      copper routed and applied, `routingProblem` with the option on for a scope of
      one net; take one chain, build a `RoutingResult` whose `moved` has that chain
      with its middle vertices shifted 0.2 mm sideways (ends unchanged) and call
      `applyRouting`:
      `testAMovedChainReplacesItsPointsAndKeepsTheRest` (the trace keeps its id,
      width, layer, `locked`, `auto`; points outside `first...last` unchanged;
      ends equal);
      `testAMovedChainWithAViaMovesTheVia` (a via chain: the via keeps its id,
      drill, diameter, tenting; its centre moves);
      `testTheBoardsConnectivityIsUnchangedByAMove` (`connectivity().netGroups()`
      before equals after, as sets of pad groups per net);
      `testAMoveIsOneUndoStepAndUndoRestoresThePoints` (one `undo()` returns every
      trace's points bit for bit);
      `testAStaleChainIsSkipped` (the trace edited between the problem and the
      apply: `summary.skipped` 1, document unchanged);
      `testNoSegmentIsZeroLengthAfterASplice`;
      `testAChainEndMovedWithinItsPadStaysConnected` (the first point of a moved run
      moved from the hand-drawn vertex to the pad's centre, both inside the pad:
      `netGroups()` unchanged).

Acceptance: the seven tests pass, plus the existing `RoutingApplyTests`.
  Check: `XB -only-testing:schematicalTests/RoutingApplyTests`
  → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `RoutingChainEngineTests` (new, 8), `RoutingRepairTests` (+1), `RoutingApplyTests` (+7),
  `RouterTests` (helper), plus the existing router suites unchanged.
- Coverage check: the three phases' tests call `RoutingState.init`,
  `finishedCopper`, `applyRouting` — the code C changes — and the A anchors run
  the unchanged path through all of them.
- Runtime proof: none beyond tests; the real-app pass is plan-66-F.
- Doc sync: none yet (plan-66-F).
- Final gate (once, here): the three Check commands (est. 10 min).

## Decisions

plan-66-A § Decisions apply. Added by the author, 2026-11-25:

- **Chains are ledger entries of bystander nets, crossable only at a push
  price** — so the grid's exact stamp/un-stamp and restore serve pushes, and
  nothing else can disturb a chain. (§ 3)
- **An untouched chain is frozen and never blamed** — the router changes only
  what a push touched. (§ 3)
- **A chain that cannot be made legal goes back to its original** — never
  removed, never left illegal by the router. (§ 3)
- **No `join` after a splice and no selection of moved copper** — anchors keep
  their contacts; selection stays on what the run added. (§ 3)

## Corrections

- **A per-cell mask replaces `grid.isBystander`** (2026-11-25). § 3 keys "is this
  cell a chain's" on the live owner being a bystander's net id. A cell with *two*
  owners holds `blocked` (-2) and has no owner id, so a chain overlapped by a
  routed net's copper would read as "another net's routed copper" and be crossable
  at `present` while the same copper as an obstacle is not: the identity this
  letter is gated on would break exactly there. `RoutingGrid.traceMask`/`viaMask`
  (a byte per map cell, set by the same stamps — `stamp`, `stampHole` — when the
  copper is a chain's, and cleared by `restoreFixed`) mark every cell a chain
  touched; the search prices such a cell at `pushPrice` (plus `present` when
  another net is also there) and at -1 when `pushPrice` is 0. Memory: a byte per
  profile × layer × cell on each of two maps, and none without bystanders.
- **`RoutingState.rip` ignores chain entries** (2026-11-25), besides `join`
  refusing a way whose victims include one: a ripped chain is the user's copper
  lost, so the guarantee is in the one function every rip goes through.
- **Phase 1 needed part of Phase 2's `finishedCopper`** (2026-11-25): `Router.route`
  runs it, and with chains seeded as entries the cleanup input and the verifier
  would have chain copper in them. Untouched chain entries are frozen, a violation
  is blamed only on new or touched entries (else a stub or a clearance the user's
  own board already breaks would loop the repair forever, since `rip` now refuses
  chains), and only non-chain copper is returned. `FinishedCopper`, `moved` and the
  restore of a chain that stays illegal are Phase 2.
- **`RoutingCleanup` and `RoutingVerifier` read the bystanders' clearance** too
  (2026-11-25): `clearanceOf` over `nets + bystanders`, as the grid's.
- **Six tests in Phase 1, not four**: the grid-equality test is joined by the
  un-stamp, `replace` and (in the tests) mask checks.
- **A chain's free end is a stub, and cleanup removes stubs** (2026-11-25). The
  first Phase 2 tests gave their chains free ends; cleanup took them out as stubs
  and the verifier flagged "joins nothing", so a touched chain went back to its
  original and `moved` stayed empty. A real chain ends on a pad, a junction with
  other copper, or a via; the Phase 2 test boards give theirs pads, and
  `tracesAsChains()` (free-ended walls) is used only where nothing is touched.
  Related, found while writing it: cleanup can change a touched chain's *shape*
  (a stub or a via hop removed), which the document's chain cannot follow, so
  `finishedCopper` sends such a chain back to the copper the push left
  (`sameShape`), and `finished` skips a chain whose run or via count differs.
- **`FinishedCopper` also returns the chains' copper as it stands** (`chainTraces`,
  `chainVias`) (2026-11-25): length-match tuning verifies its result against
  `problem.obstacles`, which no longer holds the chains; `Router.route` hands
  `tuned` a problem with the chains as obstacles (`withChainsAsObstacles`).
- **A chain cleanup repairs on its own is listed as moved** (2026-11-25): a push
  that left a chain illegal can be made legal by cleanup (straightened off a pad);
  that result is the kept one. A chain goes back to its original only when it is
  illegal even raw (`testAChainThatStaysIllegalGoesBack`, over a big pad that no
  straightening clears).

## Summary

The risk is the first phase: making foreign copper crossable in principle while
keeping it un-crossable in fact. The identity test (chains vs obstacles) is the
guard; without it D could not tell its own effect from C's leaks. The document
splice is the second risk; its tests assert connectivity and undo, not just
points.
