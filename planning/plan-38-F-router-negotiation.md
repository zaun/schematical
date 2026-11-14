# plan-38-F: Routing engine — negotiated rip-up and reroute, net ordering, strategies, cancellation

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-E

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

A single pass leaves connections that a better arrangement of the others would
have allowed. This letter makes the engine **negotiate**: a failed connection
is searched again through other nets' routed copper at a price, the connections
it displaces are ripped up and re-queued, history makes contested cells
dearer each round, and the loop runs until every connection is routed, a round
makes no progress, the strategy's limit is reached, the time limit passes or
the run is cancelled. The state after any round, or after a cancel, is always a
legal non-overlapping set of copper.

References:

- plan-38-E (§ 3 grid, stamping, search; `Router.route`); `planning/plan-38-data/spike/`
  (`router-v2-ripup.swift`: whole-net rip-up; its results are the baseline).

## 1. Goal

- With strategy **Balanced**, AFU (fixture, default options) completes all 115
  connections, in under 60 s (the prototype did it in 1.2–1.5 s with whole-net
  rip-up).
- LFU completes at least 127 of 188 (the prototype's best run, a floor), with
  every unrouted connection named in the result; RegFile at least 12 of 24
  (the prototype's run, before it exempted fingers), results recorded here.
- A synthetic board where the single pass of plan-38-E leaves a connection
  unrouted is completed by Fast, Balanced and Thorough.
- A cancel or time limit returns the routed set at that moment, which has no
  overlaps (tested by re-checking every pair).

### Non-goals (explicit constraints)

- No push-and-shove of existing locked/hand copper: kept copper is immovable
  obstacle. (Owner decision, plan-38-A § Decisions.)
- No change to the single-connection search (E) except the relaxed mode.
- No cleanup / exact verification (H); the cap on ripped connections per round
  is a quality control, not a correctness rule.

## 2. Current State

- plan-38-E routes each connection once and stamps its copper (exact segment
  distance) into per-profile maps; nothing can be removed.
- The prototype's v2 rip-up: failed connection → relaxed search (foreign cells at
  cost 25) → victims = nets with copper near that path → rip up whole nets →
  re-route → re-queue victims; limit 12 rip-ups per net; rebuild every map from the
  base each time (`router-v2-ripup.swift`, `rebuild()`).
- Measured (`results.txt`, v2): AFU 115/115 in 1.03–1.52 s (125–156 vias); LFU
  10 runs 26.1–67.6 % (49–127 of 188), 9–11 s each, 2–4 nets dropped whole per
  run; the best LFU runs lose only the two rails (41 + 27 = 68 connections of 188).

### Measured populations

| What | Count | Command |
|---|---|---|
| LFU connections on the two big nets / all | 68 / 188 | `results.txt` § Connections per net (41 + 27) |
| AFU connections on the two big nets / all | 46 / 115 | same (26 + 20) |
| Prototype LFU runs | 10, range 26.1–67.6 % | `results.txt` |

### Verified properties

- Prototype run-to-run variation came from iterating Swift dictionaries (hash
  order is per-process random): the same input gave 26.1 %–67.6 % on LFU. The real
  engine therefore iterates only sorted arrays, and a test pins determinism.

## 3. Design Overview

**State.** `RoutingState` holds, per connection ever routed, a ledger entry `{ id,
net, path cells, trace/via geometry, stamped bounding box }`, the per-net
union-find of joined components (recomputed from the ledger and the kept copper,
never stored incrementally), a `fixed` map (obstacles, kept copper — immutable),
and the routed maps of plan-38-E. **Unstamp(connection)** clears its bounding box
(grown by the largest bloat) in the routed maps back to the fixed base and
re-stamps every other ledger entry whose box meets it — exact, no reference
counts. A property test compares the maps after rip-up with a map rebuilt from
scratch from the remaining entries: equal cell for cell.

**Work queue.** Entries are *nets with at least one unjoined component pair*, not
specific edges: a rip-up can split a net's tree anywhere, and the next join
simply connects the two nearest unjoined groups. Order: nets with failures in the
last round first (descending failure count), then **width descending** (a Power
class claims room first), then bounding-box half-perimeter ascending, then net id.

**Relaxed search.** The search of E with one change: a cell covered only by other
nets' *routed* copper (not `fixed`) is passable at `+ present × (1 + history[cell])`;
`fixed` and `-2` cells never are. A relaxed path yields the set of ledger entries
whose stamped copper the path crosses (found by a spatial index: ledger entries
bucketed by 64-cell tiles).

**Round.** For each queued net, in order: hard search; if it joins, stamp it; if
not, relaxed search; no relaxed path → the connection stays failed (it is blocked by
fixed copper or the outline; counted); else rip up the victims (connection
level), stamp the relaxed path hard, `history[cell] += historyStep` for every
cell of the path, and queue the victims' nets. `present` multiplies by 1.5 each
round. A connection ripped more than `maxRips` times per round-set is left failed
to stop thrash.

**Strategies** (`options.strategy`): `fast` — 6 rounds, via cost 12, historyStep
0.5, maxRips 8; `balanced` — 30 rounds, via cost 12, historyStep 1.0, maxRips 12;
`thorough` — up to 120 rounds, and up to 3 independent runs (tie-break order of
the queue permuted by `seed`, `seed + 1`, `seed + 2`; stop at the first run that
completes; else keep the run with the most routed connections, then the least
wire). A round with no change in the failed set for 3 rounds ends the run.

**Cancel and time.** `isCancelled` and `timeLimit` are polled every 4096 search
expansions and between connections; on a stop the current ledger is the result
(always legal: only hard-stamped copper is in it).

Determinism: no dictionary iteration in any decision; `seed` only permutes ties
in Thorough. Equal inputs and options give equal results (tested).

Where correctness risk concentrates: un-stamping (a stale cell is a phantom
obstacle or a missed one); the property test above is the control.

Byte-identity is not a gate; completion is, with floors from the prototype.

Rejected: whole-net rip-up (the prototype's; it threw away a 41-connection rail
for one failed connection); reference-counted stamps (3+ nets overlap in a cell,
so counts need per-net tables); simulated annealing over orderings (non-deterministic
cost for no measured need).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — ledger, joins and exact un-stamping

- [x] `Routing/RoutingState.swift`: ledger entries, per-net joined groups from ledger + kept copper, `stamp(entry)`, `unstamp(entry)` of § 3, a spatial tile index. (Un-stamping is `remove(_:)`; groups come from what each entry's two ends land on, a component's terminal or another entry's node, since kept copper is already inside its component (plan-38-D). Added `dangling(of:)`: entries left hanging by one end after a rip-up, which Phase 3 rips with it so no stub ends on nothing.)
- [x] `Router.route` (plan-38-E) uses `RoutingState` for its single pass; its results must not change. — `RouterFixtureTests` after: AFU 99/115 (5 677 419 expansions), LFU 70/188 (6 933 784), RegFile 15/24 (5 180 621), identical to plan-38-E Phase 4.
- [x] Tests (`RoutingStateTests`): stamp three connections, unstamp the middle one, compare every map cell with a state built from scratch from the other two (equal); after unstamping a tree's middle edge the net reports two unjoined groups; the tile index returns exactly the entries whose box meets a query box. (Plus: a branch left hanging is found by `dangling`.) — `-only-testing:RoutingStateTests -only-testing:RouterTests -only-testing:RouterFixtureTests` → 12 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: cell-for-cell equality; plan-38-E's tests and `RouterFixtureTests` results unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/RoutingStateTests -only-testing:schematicalTests/RouterTests -only-testing:schematicalTests/RouterFixtureTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: 43b2317f

### Phase 2 — relaxed search, victims, history

- [x] `Routing/RoutingSearch.swift`: relaxed mode with `present`/`history`, returning the path's victim entries. (`SearchRules.present` > 0 makes a search relaxed: a cell only other nets' routed copper covers, where the fixed map is free or own, costs `present × (1 + history)`, and a via the same summed over its layers; the victims are found from the path by `RoutingState.victims(of:net:)`, by exact distance to each entry's copper. `RoutingSearch.lastCost` gives the path's cost.)
- [x] `Routing/RoutingState.swift`: history map, `rip(entries)`. (`rip` also rips the entries left hanging by one end, and returns the nets that lost copper.)
- [x] Tests (`RoutingNegotiationTests`): a board where connection A takes the middle channel and leaves B no route — the relaxed search for B returns A as its only victim; relaxed search never crosses a `fixed` obstacle or a locked kept trace; history raises the cost of a contested cell (the cost of the same path rises after `history += 1`). (Plus: ripping A restores the grid to the empty board's.) — `-only-testing:RoutingNegotiationTests -only-testing:RoutingSearchTests` → 14 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: the cases pass.
  Check: `-only-testing:schematicalTests/RoutingNegotiationTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 3 — the negotiation loop and strategies

- [ ] `Routing/Router.swift`: queue order, rounds, `maxRips`, the stall rule, the three strategies, Thorough's runs and keep-best, cancel / time limit returning the current legal ledger, `RoutingStats` (rounds, rips, seconds, vias, wire mm).
- [ ] Tests (`RoutingNegotiationTests`): the two-connection board of Phase 2 completes under Fast, Balanced and Thorough where the single pass fails; a 6-net "channel" board (hand-built: 6 pads on the left, 6 on the right in reversed order, 2 layers) completes under Balanced; a cancel after the first round returns a result with no overlapping pair of copper (check every pair with `CopperGeometry.gap` against the rule); `timeLimit: 0.001` returns promptly with that property; same input twice → equal results; reversing `problem.nets` → equal results; Thorough with seeds 1 and 2 may differ but each is legal.

Acceptance: the cases pass.
  Check: `-only-testing:schematicalTests/RoutingNegotiationTests -only-testing:schematicalTests/RouterTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 4 — the fixtures

- [ ] `RouterFixtureTests`: with `.balanced`, AFU = 115/115 in < 60 s; LFU ≥ 127/188; RegFile ≥ 12/24; print connections routed, vias, wire mm and seconds for each; assert every unrouted LFU connection appears in `NetOutcome` with its net's name.
- [ ] Results (the output of this run, filled in then): AFU __/115, __ vias, __ mm, __ s; LFU __/188, __ vias, __ mm, __ s; RegFile __/24 … Also the same three with `.fast` and `.thorough` (counts and seconds only).

Acceptance: the AFU, LFU and RegFile floors hold and the results are written above. If a floor fails, diagnose (dump the failed connections' blockers for ONE net) and fix the engine; the floors are not lowered.
  Check: `-only-testing:schematicalTests/RouterFixtureTests` → `** TEST SUCCEEDED **` (est. 5 min: Balanced LFU took 9–11 s in the prototype and Thorough runs up to 3 times).
Commit: —

## Validation Plan

- Tests: `RoutingStateTests`, `RoutingNegotiationTests`, `RouterTests`, `RouterFixtureTests`.
- Coverage check: the rip-up, history, stall and cancel branches are each hit by a test that fails if removed.
- Runtime proof: Phase 4's fixture run.
- Doc sync: none yet (plan-38-M writes `docs/routing.md`).
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: connection-level (not net-level) rip-up; queue entries are nets with unjoined groups; width-descending then shortest-first ordering; the strategy table of § 3; floors for LFU and RegFile are the prototype's results and are never lowered to pass.

## Corrections

(Filled in during execution.)

## Summary

The risk is state consistency under rip-up — the equality property test guards
it. The measured question this letter answers is how far the real engine
gets past the prototype on LFU, whose two rails are 36 % of its connections.
