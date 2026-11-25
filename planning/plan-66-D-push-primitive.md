# plan-66-D: Push and shove — the corridor search and the push transaction

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-66-C

D builds the push itself, as a primitive the negotiation will call in E: given a
connection with no legal way and a relaxed way through other copper, take the
copper in the way up, lay the connection, and **re-lay each piece of that copper
at once, inside a corridor around where it was**, all or nothing. It is
exercised directly by tests on hand-built boards; `join` and the improvement do
not call it until E, so with D alone the router's results do not change.

References:

- `planning/completed/plan-66-A-push-option-anchors.md` (§ 3, § Decisions),
  `plan-66-B-movable-chains.md`, `plan-66-C-chains-in-the-engine.md` (entries
  with `chain`, `touched`, `replace`; `pushPrice`).
- `Routing/Router.swift:437-515` (`join`: the hard search, the relaxed detour,
  `victims`, `rip`, `Router.copper`), `RoutingSearch.swift` (`search`, `price`,
  `Maps`, `startCosts`), `RoutingState.swift` (`victims`, `remove`, `add`, `tie`,
  `groups`), `RoutingPolish.swift:47-80` (`attempt`: snapshot and restore).

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The gate for the whole feature is in plan-66-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-66-C is complete and archived | `ls planning/completed/plan-66-C-* \| wc -l` → 1 | MET (2026-11-25, in `worktree-P-66`: 1) |
| Chains are entries and a chain is never ripped | `grep -c 'var chain' schematical/schematical/Routing/RoutingState.swift` → at least 1 | MET (2026-11-25, in `worktree-P-66`: 1) |

If plan-66-C is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Router.Run.tryPush(net:sources:targets:plan:isCancelled:)` returns `true`
  and leaves the connection laid with every piece of copper it displaced
  re-laid inside `pushReach` of where it was, or returns `false` and leaves
  `state` exactly as it was.
- Each re-laid piece keeps its two ends, its sequence of layers and no more vias
  than it had; chains it moves are marked `touched`.
- On a board where the only way for a connection is through a gap an existing
  trace sits in, `tryPush` lays the connection and moves that trace; with the
  trace locked (a fixed obstacle), or farther from room than `pushReach`, it
  returns `false`.

### Non-goals (explicit constraints)

- `join`, `attempt` and `routeAll` do not call `tryPush` (E).
- No change to a result when the option is off, or on without a call to
  `tryPush`: the A anchors and C's identity tests pass unchanged.
- A connection that ends on another connection of its net is not pushed (it is
  ripped, as today); copper of a coupled pair and of a Plane fan-out is never
  pushed. Chains that end at anchors are always pushable.
- No continuous-geometry shove; no change to cleanup or verification (C).

## 2. Current State

`join` finds a hard path; failing that, with `present > 0`, a relaxed path
(`detour`) through ledger copper, takes `state.victims(of: detour, net:)` and
`rip`s them — within each net's rip budget, and never nets in `pinned` — then
searches hard again; the ripped nets are returned as `lost` and queued to be
routed again, anywhere (`Router.swift:471-503`). In the improvement, `attempt`
snapshots `state`, makes the move, and assigns the snapshot back unless the
score is better (`RoutingPolish.swift:51,69-72`): the precedent for an atomic
restore.

`RoutingSearch.search` takes sources, targets, a goal box for the heuristic,
optional `history`, `startCosts`/`targetCosts` and a `plan` of per-node prices
(`RoutingSearch.swift:156-157`); `price(layer, cell)` returns the extra cost of
entering a node or -1 when it cannot be entered (`:258-263`). A search with
`maxExpansions` (`SearchRules`, default 3 000 000) returns `.tooLong` when it
exhausts them.

### Measured populations

| What | Count | Command |
|---|---|---|
| Callers of `RoutingSearch.search` | 5 in 4 files, plus 6 in tests | `grep -rn '\.search(' schematical/schematical/Routing \| wc -l` → 5 (`AssistRouter` 1, `Router` 2, `RoutingPair` 1, `RoutingPlan` 1); `grep -rn '\.search(' schematical/schematicalTests \| wc -l` → 6; the new `corridor:` argument defaults to `nil`, so none changes |
| Stored properties of `Router.Strategy` | 19 | `awk '/struct Strategy/,/static func of/' schematical/schematical/Routing/Router.swift \| grep -c 'var '` → 19; D adds 5 with defaults |

### Verified properties

- **`state` can be restored by assignment.** `RoutingState` is a `struct` of
  value types (`grid`'s arrays, dictionaries, `history`); `RoutingPolish.attempt`
  already does `state = saved` after a failed multi-step move
  (`RoutingPolish.swift:51,70`, read).
- **`Router.copper` keeps ends exact.** It takes `startTie`/`endTie` and inserts
  them as the first and last point of the first and last run
  (`Router.swift:696-720`, read), which is how a re-laid victim keeps its anchor
  points exactly.
- **A ledger victim can be eligible.** An entry whose two end nodes are both
  terminals of its net's components and that no other entry of the net attaches
  to is a leaf-to-leaf connection: re-laying it between the same two end nodes
  keeps `groups(of:)` the same (`RoutingState.swift:215-239`: an entry joins what
  its first or last node lands on).

## 3. Design Overview

### The corridor

A corridor is a per-node price array the search reads: `corridor[node] < 0` means
the node may not be used; `≥ 0` is added to the price of entering it. Built from
the piece being re-laid (a ledger entry or a chain entry):

- for each of its traces, on that trace's layer, every cell whose centre is within
  `pushReach` of the trace's polyline (by `RoutingGrid.distance` to each segment)
  gets `pushDeviation × (distance ÷ grid pitch)`;
- for each of its vias, every cell within `pushReach` of the via's centre on every
  allowed layer gets the same price from that distance;
- everything else is -1.

`RoutingSearch.search` gains `corridor: UnsafeBufferPointer<Float>? = nil`
(through `Maps`); `price` returns -1 for a negative corridor value and otherwise
adds the value. With no corridor the search is as before, byte for byte. The
scratch array lives in `Router.Run` (`[Float]` of `cells × layers`, -1 filled,
only the cells set are reset after use), so a corridor costs the size of the
piece, not the board.

### The push transaction

```
tryPush(net, sources, targets, plan):
    saved = state; savedCounters = (rips, history…)
    inTransaction = []                       // entry ids added by this transaction
    ok = pushRoute(net, rules(net), sources, targets, corridor: nil, depth: 0,
                   commit: .connection)
    if !ok { state = saved; return false }
    pushTried/pushKept counters; return true

pushRoute(net, rules, sources, targets, corridor, depth, commit):
    if hard = find(rules with hard price, sources, targets, corridor)
        → commit(hard); return true
    guard depth < strategy.pushDepth
    detour = find(rules with present = strategy.present, pushPrice = 2 × present, corridor)   else return false
    victims = state.victims(of: detour, net: net.id)
    guard victims.count ≤ strategy.pushMaxVictims, every victim eligible, none in inTransaction
    describe each victim (its net, old copper, end nodes, end ties, old layers, old via count)
    state.remove(victims)                    // exact un-stamp
    guard hard2 = find(hard, sources, targets, corridor) else return false
    commit(hard2)                             // the connection is laid (its entry id → inTransaction)
    for v in victims (by id):
        c = corridor(of: v)
        guard pushRoute(v.net, rules(v.net), v.endNodes.a, v.endNodes.b, corridor: c, depth + 1,
                        commit: .victim(v)) else return false
        // `.victim` accepts a path only if it has the same layer sequence as v's copper
        // and no more vias; otherwise the search result is refused (treated as not found).
    return true
```

- **Commit.** `.connection` is `join`'s own addition of copper: ties from
  `state.tie(node, net:, components:)`, `Router.copper`, `state.add`. `.victim(v)`
  adds the re-laid copper as a new entry of `v.net` with `path` from the new path;
  for a chain victim with `chain: v.chain, touched: true`. The ends' ties are the
  terminals' own: for a chain `chainTerminals[chain]`'s `tie` at the found end node,
  for a ledger victim `state.tie(node, net:, components:)`. The commit factors out
  of `join` into `Run.addConnection(…)` which both use.
- **Eligible victim.** A chain entry; or a ledger entry whose net is in
  `routable`, not in `pairNets`, not `pinned`, whose two end nodes are terminals
  of the net's components, and to whose nodes no other entry of the net attaches.
- **A victim's search** goes from its end-`a` terminal nodes to its end-`b`
  terminal nodes: for a ledger victim the nodes `path.first`/`path.last`; for a
  chain the end points' nodes from the chain's `ends` (`RoutingState` keeps
  `chainTerminals[chain] = (a, b)` built from `ends[i].anchor` with
  `RoutingGrid.terminals(of:net:profile:allowed:)`, added in this phase).
- **The accept rule** is checked when a victim's path is found: the traces
  `Router.copper` would make have the same layer sequence as the victim's old
  traces, and `Router.viaCount(path) ≤` its old via count. A path that fails the
  rule is refused as if the search had found none.
- **Constants** (`Router.Strategy`, defaults for every strategy):
  `pushReach = 1.0` mm, `pushDeviation = 1.0` price per cell of distance per cell
  of travel, `pushDepth = 2` (the connection pushes victims; a victim may push
  once more), `pushMaxVictims = 12`, `pushMaxExpansions = 200 000` (applied to each
  search inside a push as `SearchRules.maxExpansions`).
- **Atomicity.** Every `state.remove`, `add` and counter change happens between
  the snapshot and the return; a `false` return is preceded by `state = saved`.
  A cancelled search (`stopped`) returns `false` the same way.
- **No ping-pong.** A victim cannot be an entry added by the same transaction, so
  two victims cannot push each other back and forth.

### Where correctness risk concentrates

The accept rule and the restore: a push that returns `true` must leave a legal
ledger (it is, by construction: every added entry is a hard-search path on the
grid after the removals) with every connection of every net still joined (the
ends are the same terminal nodes), and one that returns `false` must leave
nothing behind. Tests compare `state` before and after a refusal member by
member (`entries`, `grid.traceMaps`, `grid.viaMaps`, `history`).

**Gate class:** no existing behaviour changes (nothing calls `tryPush`): the A
anchors and C's identity tests are the **provably neutral** gate. The new
behaviour is tested, not diffed.

**Rejected alternatives.**

- *Re-queue the victims like rip-up does.* Not local, not all-or-nothing, the
  connection that was laid can be lost again (plan-66-A § 3).
- *Allow a victim to change layer sequence or gain vias.* A hand-drawn trace would
  grow a via; a pushed trace is meant to stay what it was.
- *A larger default depth.* Each level multiplies searches; depth 2 covers a
  victim that itself needs room. Raising it is one constant.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. An unticked box means NOT DONE.

### Phase 1 — the corridor in the search

- [x] (done; the parameter is `corridor: [Float]?`, not an unsafe buffer: see Corrections) `Routing/RoutingSearch.swift`: `search(…, corridor: UnsafeBufferPointer<Float>?
      = nil)`; `Maps.corridor`; `price` as in § 3 (a `hasCorridor` constant so the
      no-corridor path is unchanged).
- [x] (done) `Routing/RoutingCorridor.swift` (new): `struct RoutingCorridor` holding the
      scratch `[Float]` and the list of cells set; `mutating func make(for copper:
      (traces: [RoutedTrace], vias: [RoutedVia]), grid: RoutingGrid, reach:
      Double, deviation: Double)` and `mutating func clear()`.
- [x] (done) `Routing/Router.swift`: `Strategy.pushReach`, `pushDeviation`, `pushDepth`,
      `pushMaxVictims`, `pushMaxExpansions` with the defaults of § 3 (every strategy).
- [x] (done: 3 search tests and 1 grid test) `schematicalTests/RoutingSearchTests.swift`: `testNoCorridorIsTheSameSearch`
      (a corridor of all zeros gives the same path and cost as `nil`);
      `testACorridorKeepsAPathInsideIt` (a board with a straight run; a corridor
      one cell wide on its line returns that line, a corridor missing the targets
      returns `.noPath`); `testADeviationPriceKeepsAPathNearTheOldLine`.
      `schematicalTests/RoutingGridTests.swift`:
      `testACorridorAroundATraceIsWithinReach` (`make` sets exactly the cells
      within `reach` of the polyline and -1 elsewhere; `clear` restores all -1).

Acceptance: the four test groups pass and the A anchors still pass.
  Check: `XB -only-testing:schematicalTests/RoutingSearchTests
  -only-testing:schematicalTests/RoutingGridTests
  -only-testing:schematicalTests/RoutingAnchorTests` → `** TEST SUCCEEDED **`
  (est. 3 min).
  Measured: `Executed 29 tests, with 0 failures` (RoutingSearchTests 13, RoutingGridTests 7, RoutingAnchorTests 9), `** TEST SUCCEEDED **`.
Commit: — (recorded in the next commit)

### Phase 2 — the transaction

- [ ] `Routing/RoutingState.swift`: `chainTerminals: [Int: (a: [RoutingTerminal],
      b: [RoutingTerminal])]`, built in `init` from each chain's two `ends`
      (their `anchor` components through `grid.terminals`).
- [ ] `Routing/Router.swift`: factor `join`'s "add the found path as copper" into
      `Run.addConnection(_:found:net:main:target:rules:)`; `join` calls it,
      unchanged in effect (the A anchors and the whole router suite prove it).
- [ ] `Routing/RoutingPush.swift` (new, `extension Router.Run`): `tryPush`,
      `pushRoute`, the eligibility and accept rules, the describe-victim record,
      the `.connection`/`.victim` commit, per § 3. `Router.Run` gains
      `var corridor = RoutingCorridor()` and `var pushTried`, `pushKept` counters
      copied to `RoutingStats` in `result(…)`.
- [ ] `schematicalTests/RoutingTestBoard.swift`: `static func shoveGap()`: one
      layer `[.top]`, `options.grid = 0.1`, `options.vias = .none`, pads of net 2 at
      (3, 6) and (17, 14), pads of net 5 at (3, 10) and (17, 10) (all 1 × 1, net 5
      in `unrouted`), a wall of net 9 as `trace([(10, 0), (10, 9.4)], net: 9)` and
      `trace([(10, 10.6), (10, 20)], net: 9)`, and the chain of net 5 from pad 2 to
      pad 3 as `chain([(3, 10), (17, 10)], net: 5, from: 2, to: 3)`.
      Arithmetic (verified by hand against the rules: width 0.254, clearance 0.1):
      the wall's copper ends at 9.527 and 10.473, so a trace centre may sit in
      9.754 … 10.246; two traces need centres 0.354 apart; the chain at y = 10 leaves
      none, and moved to 9.8 it leaves 10.2.
      `static func ledgerShoveGap()`: the same with net 5 routed by the run (its two
      pads as net 5 components, not `unrouted`, no chain, wall unchanged).
- [ ] `schematicalTests/RoutingPushTests.swift` (new), each using `Router.Run`
      directly and `RoutingCheck`:
      `testAChainInAGapIsPushedAside` (`shoveGap()`; `tryPush` for net 2 → `true`;
      net 2 is joined; the chain entry has `touched` and its new points start and
      end at the pad centres, stay on `.top`, stay within `pushReach` of y = 10, and
      `finishedCopper()` has `moved.count == 1` and `RoutingCheck.violations == []`);
      `testALedgerConnectionInAGapIsPushedAside` (`ledgerShoveGap()`: `join` net 5
      first, then `tryPush` net 2; net 5 is re-laid, both joined, legal);
      `testALockedTraceIsNotPushed` (the chain as a fixed `trace(…)` obstacle:
      `tryPush` → `false`; `entries`, `grid.traceMaps`, `grid.viaMaps` and `history`
      equal the snapshot taken before);
      `testAPushBeyondReachFails` (the chain's gap moved so the room is more than
      `pushReach` from y = 10: `false`, state equal);
      `testAVictimThatWouldGainAViaIsRefused` (two layers, the chain pad-to-pad on
      Top, a victim re-lay that needs a via is refused: `false`, state equal);
      `testACascadeNeedsDepthTwo` (three chains side by side in a wider gap, the
      middle one's room only by pushing a neighbour: `pushDepth = 1` → `false`,
      `2` → `true`; coordinates chosen so the gap's centre range (wall copper ends
      grown by clearance and half a width) holds three traces 0.354 apart);
      `testABranchOfAMultiPinNetIsNotPushed` (a ledger entry whose end sits on
      another entry's node: `false`, state equal);
      `testACoupledPairIsNeverPushed`; `testACancelledPushIsRestored` (`isCancelled`
      true after the connection is laid: `false`, state equal);
      `testPushIsDeterministic` (the same board twice: equal entries).

Acceptance: all `RoutingPushTests` pass, the A anchors and C's identity tests too.
  Check: `XB -only-testing:schematicalTests/RoutingPushTests
  -only-testing:schematicalTests/RoutingAnchorTests
  -only-testing:schematicalTests/RoutingChainEngineTests
  -only-testing:schematicalTests/RoutingNegotiationTests` → `** TEST SUCCEEDED **`
  (est. 4 min: RoutingNegotiationTests alone took 26 s in the baseline run).
Commit: —

## Validation Plan

- Tests: `RoutingPushTests` (new, 10), `RoutingSearchTests` (+3),
  `RoutingGridTests` (+1). Negative cases: locked, reach, a via gained, a branch,
  a pair, a cancel.
- Coverage check: the push file is in the `schematical` target; every branch of
  `pushRoute` (hard hit, no detour, ineligible, over the victim cap, depth, victim
  refused) has a test above.
- Runtime proof: none beyond tests; the real-app pass is plan-66-F.
- Doc sync: none yet (plan-66-F).
- Final gate (once, here): both Check commands (est. 7 min).

## Decisions

plan-66-A § Decisions apply. Added by the author, 2026-11-25:

- **Push re-lays victims at once, inside a corridor, all or nothing** — the grid
  engine's own stamp/un-stamp and snapshot serve it. (§ 3)
- **A re-laid piece keeps its layers and gains no via** — a pushed trace stays
  what it was. (§ 3, accept rule)
- **Reach 1.0 mm, deviation price 1.0, depth 2, 12 victims, 200 000
  expansions** — starting constants in `Router.Strategy`; the tests above define
  what they must allow, and a change to one is recorded in Corrections. (§ 3)
- **A branch of a multi-pin net is ripped, not pushed** — its end sits on
  another entry's node, which a push would have to move with it. (§ 3)

## Corrections

- **The search takes the corridor as `[Float]?`** (2026-11-25), not an
  `UnsafeBufferPointer`: the array is not copied (a read-only argument), and the
  search turns it into a buffer inside its own pointer nest as it does for `history`
  and the plan's arrays. No corridor is an empty array there, so `hasCorridor` is
  false and the price is unchanged: the A anchors (9) pass.

## Summary

The risk is the atomic restore and the accept rule; D tests both by comparing
the whole state before and after a refusal. Nothing calls the primitive yet, so
D cannot change a result; E turns it on.
