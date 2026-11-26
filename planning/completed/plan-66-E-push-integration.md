# plan-66-E: Push and shove — in the negotiation, every strategy, and the fixtures

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-66-D

E turns the primitive on: when `pushAndShove` is set and a connection has no
legal way, `join` tries `tryPush` before it falls back to ripping up. Because
the improvement passes and the retry of unrouted nets both go through `join`,
they push too. The three strategies use it alike, a run keeps the better of its
runs with a tie going to the one that moved less existing copper, and the
fixtures prove that nothing router-caused breaks and nothing routes worse.

References:

- `planning/completed/plan-66-A-push-option-anchors.md` (§ 3, § Decisions),
  `plan-66-D-push-primitive.md` (`tryPush`, the constants).
- `Routing/Router.swift:295-362` (`routeAll`), `437-515` (`join`),
  `168-175` (`better`), `RoutingPolish.swift:47-153` (`attempt`, `polish`),
  `RouterFixtureTests.swift:37-75`, `RouterQualityFixtureTests.swift`.

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The gate for the whole feature is in plan-66-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-66-D is complete and archived | `ls planning/completed/plan-66-D-* \| wc -l` → 1 | MET (2026-11-25, in `worktree-P-66`: 1) |
| `tryPush` exists | `grep -c 'func tryPush' schematical/schematical/Routing/RoutingPush.swift` → 1 | MET (2026-11-25, in `worktree-P-66`: 1) |

If plan-66-D is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- With `pushAndShove` on, a connection whose only way is through a gap an
  existing unlocked trace sits in is routed and that trace is moved; with it
  off the same board leaves the connection unrouted and the trace untouched.
- With it on, no fixture result has a router-caused DRC issue, and AFU, RegFile
  (Debug, negotiation alone) and AFU, RegFile, LFU (Release, Balanced) route at
  least as many connections as with it off.
- With it on and nothing blocked, the result equals the result with it off.

### Non-goals (explicit constraints)

- No change with the option off: the A anchors pass unchanged.
- No push in `fanOut` (Plane fan-out), in a coupled pair's routing, or in
  `Router.tuned`; a net in `pinned` or `pairNets` is never a victim (D).
- No change to the strategies' other numbers (rounds, via costs, plan, polish).
- No sheet text, no docs (plan-66-F).

## 2. Current State

`join` (`Router.swift:437-515`): hard search; failing that and with `present > 0`,
the relaxed `detour`, the victims, the rip within budget, the hard search again;
a still-failed target is `skipped` for the round. `routeAll` runs round 0 hard
only (`present: 0`), then rounds with `present` growing; `polish` retries nets
still unrouted with `join(net, present: 0, …)` (`RoutingPolish.swift:131-136`)
and runs `attempt` moves, which `join` serves. `Router.better` orders two runs by
routed count, then by score (`:168-175`).

`RouterFixtureTests.testNegotiationOnTheFixtures` routes each fixture with
negotiation alone in a Debug build (floors AFU 115, LFU 127, RegFile 12) and with
Balanced in Release; `RouterQualityFixtureTests` prints `QUALITY` lines for
Balanced and Thorough in Release and bounds them a little over the measured
numbers (`:77-110`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Places `join` is called | 6 | `cd schematical/schematical/Routing && grep -n 'join(' Router.swift RoutingPolish.swift RoutingRepair.swift RoutingPair.swift \| grep -v 'func join' \| wc -l` → 6: `routeAll` ×2 (`Router.swift:327,356`), `attempt` ×2 (`RoutingPolish.swift:58,65`), the unrouted-net retry (`:134`), and the margin re-route in `RoutingRepair.swift:112`, whose temporary run C sets to `pushAndShove = false`; the other five reach the one hook |
| Fixture floors in Debug | AFU 115, LFU 127, RegFile 12 | `grep -n 'floors' schematical/schematicalTests/RouterFixtureTests.swift` → line 38 |
| Release bounds | AFU ≤ 80 vias, RegFile ≤ 18, LFU ≥ 186 routed ≤ 320 vias | `sed -n 77,100p schematical/schematicalTests/RouterQualityFixtureTests.swift` |

### Verified properties

- **`join` is the single funnel.** Negotiation rounds, the improvement's
  `attempt` and the retry of unrouted nets all call it (read at the lines above),
  and `attempt` restores `state` by assignment when a move is not kept
  (`RoutingPolish.swift:69-72`), so a push made inside a move that is dropped is
  dropped with it.
- **A push only runs where there is no hard path.** It is attempted only after
  `find(hard, …)` returned nil, so on a board where every first search succeeds
  it never runs and the result cannot change.

## 3. Design Overview

### The hook

In `join`, right after the hard search:

```swift
var path = find(hard, sources: sources, targets: targets, plan: prices, isCancelled: isCancelled)
if path == nil, problem.options.pushAndShove, !stopped,
   tryPush(net: net, sources: sources, targets: targets, plan: prices, isCancelled: isCancelled) {
    report(progress, round: round, net: net.name)
    continue          // the connection is laid; groups(of:) is read again
}
```

On `false` (state restored) the existing relaxed-rip branch runs as before; it
already refuses a rip whose victims include a chain entry (C). `tryPush` is not
called when the net is in `pairNets` (a pair is routed by `routePairs`) or in
`pinned`.

### Every strategy; the choice between runs

The hook is not conditioned on the strategy: Fast, Balanced and Thorough all
push. `Router.better` keeps its order (more routed, then lower score) and gains a
last step: when the two scores are equal within `1e-9`, the run with fewer
`movedTraces + movedVias` wins. `RoutingStats.pushTried`, `pushKept` are summed
into the kept run's stats; `movedTraces`/`movedVias` are set from `moved` (C).

### Cancellation and time

`isCancelled` and the run's `deadline` already reach every search through `find`;
a cancelled push returns `false` with the state restored (D), so Stop and a time
limit leave a legal result with whatever pushes had completed.

### Where correctness risk concentrates

Not in legality (the ledger, cleanup, verifier and repair are unchanged). In
**regression**: a push replaces a rip-up, which changes the order in which
connections are routed on every board where negotiation ever failed a hard
search, so results with the option on can differ from results with it off on
such boards, and for the worse. The gates are routed counts and DRC on the
fixtures (below), not identity; the A anchors guard the off case.

**Gate class:** option off — **provably neutral** (the A anchors, byte for byte).
Option on — behaviour changes by design; the **expected** differences are boards
where a hard search fails: the boards with chains, and fixtures where negotiation
ripped up. A diff on a board where no hard search fails is a bug.

**Rejected alternatives.**

- *Push only after the rounds, as a last resort.* The push is local and cheap,
  and doing it first keeps the copper that rip-up would have lost.
- *Push in the improvement's moves as well as in `join`.* The moves rip nets to
  make a connection cheaper; they already go through `join`, which pushes when a
  hard search fails. A separate "push to save a via" move would be a new move
  type with its own score; not part of this plan.
- *A strategy-specific switch.* The option is the switch; per-strategy behaviour
  would make Fast unpredictable beside the other two.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. An unticked box means NOT DONE.

### Phase 1 — the hook, the tie-break, the counters

- [x] (done; `tryPush` gains `blocked:` so the failed hard search is not repeated: see Corrections) `Routing/Router.swift`: the hook of § 3 in `join`; `better` gains the
      moved-copper tie-break; `route` sums `pushTried`/`pushKept` into the kept
      result's stats.
- [x] (done: 6 tests) `schematicalTests/RoutingPushIntegrationTests.swift` (new):
      `testPushRoutesWhatOffCannot` (`shoveGap()`; for `.fast`, `.balanced`,
      `.thorough`: option off → `routed == 0` of 1, `moved` empty, `pushTried == 0`;
      option on → `routed == 1`, `moved.count == 1`, `pushKept ≥ 1`,
      `RoutingCheck.violations == []`, the moved chain's first and last points equal
      the pad centres);
      `testALockedTraceIsNeverMoved` (the chain as a `trace(…)` obstacle, option on:
      unrouted, `moved` empty);
      `testNothingBlockedGivesTheOffResult` (`parallelRows()` plus a net-5 chain in
      the corner, option on and off at `.fast` and `.balanced`: equal traces, vias,
      `routed`; `rips == 0` in both, `pushTried == 0`);
      `testPushOnIsDeterministic` (`shoveGap()` at `.balanced`, three times: equal
      results; Balanced runs four searches on four threads);
      `testAStoppedRunWithPushIsLegal` (`isCancelled` true after the first
      connection: `RoutingCheck.violations == []`);
      `testBetterPrefersLessMovedCopperOnATie` (two results with equal routed and
      score, one with `movedTraces: 3`: the other is better).
- [x] (done: unchanged; it passes, `RoutingPolishTests` 10) `schematicalTests/RoutingPolishTests.swift`: `testBetterPrefersMoreRoutedThenFewerViasAndBends`
      still passes (the new step is last); no edit unless the helper changes.

Acceptance: the six tests pass and the A anchors, C's identity tests and D's push
tests still pass.
  Check: `XB -only-testing:schematicalTests/RoutingPushIntegrationTests
  -only-testing:schematicalTests/RoutingAnchorTests
  -only-testing:schematicalTests/RoutingPushTests
  -only-testing:schematicalTests/RoutingChainEngineTests
  -only-testing:schematicalTests/RoutingPolishTests` → `** TEST SUCCEEDED **`
  (est. 4 min).
  Measured: `Executed 45 tests, with 0 failures` (RoutingPushIntegrationTests 6, RoutingAnchorTests 9, RoutingPushTests 10, RoutingChainEngineTests 10, RoutingPolishTests 10), `** TEST SUCCEEDED **`.
Commit: 1d1af049

### Phase 2 — the fixtures

- [x] (done: `PUSH AFU routed 115/115, pushTried 0`; `PUSH RegFile routed 24/24, pushTried 28, pushKept 0, rips 33, vias 26, wire 311.7 mm`, `** TEST SUCCEEDED **`) `schematicalTests/RouterFixtureTests.swift`: `testPushOnTheFixtures`: for AFU
      and RegFile, the loop of `testNegotiationOnTheFixtures` (negotiation alone in
      Debug, Balanced in Release) with `options.pushAndShove = true`; asserts
      `result.stats.routed ≥ floors[name]` (the same floors), the
      cleanup-never-longer and never-more-vias checks of that test, no
      router-caused DRC issue after `applyRouting`, `unrouted` equal to
      `connections − routed`, and prints `PUSH <name> routed …, pushTried …,
      pushKept …`.
- [x] (done, at 12–18 mil: see Corrections; `PUSHCASES 8 of 32`) A document-level test, `RoutingPushIntegrationTests.testPushOnARoutedBoardKeepsItsConnections`:
      open RegFile, route it (Fast), apply, and record each net's group count from
      `connectivity().netGroups()`; then for each net in key order and each `width`
      in `[20, 30, 40]` (mil): on a fresh copy of that routed document build the
      problem with `scope: .nets([that key])`, `existing: .clearUnlocked`,
      `widthOverride: width`, `pushAndShove: true`, route (Fast), apply. For every
      case assert: `designRuleCheck` has no router-caused kind; `unrouted` equals
      `connections − routed`; every other net's group count is unchanged; one
      `undo()` restores every trace and via bit for bit. Over all cases assert at
      least one has `routed == connections` and `summary.movedTraces +
      summary.movedVias > 0`, and print each such case as `PUSHCASE net=<name>
      width=<mil> moved=<traces>+<vias>` (plan-66-F's real-app pass reproduces the
      first one).
- [x] (written, compiled in Debug (they skip there); run in plan-66-F's landing tier, whose Release numbers go into the bounds) `schematicalTests/RouterQualityFixtureTests.swift`: `testPushAFU`,
      `testPushRegFile`, `testPushLFU` (Release only, the file's own `#if DEBUG`
      skip): `run(name)` with `options.pushAndShove = true`; the same assertions as
      the off versions for `routed`, and bounds a little over the measured `vias`,
      `oddBends`, `wire`, written from the first run (the numbers go into
      `docs/routing.md` § Results in plan-66-F).

Acceptance: the Debug tests pass; the three Release tests are written and run in
plan-66-F's landing tier.
  Check: `XB -only-testing:schematicalTests/RouterFixtureTests/testPushOnTheFixtures
  -only-testing:schematicalTests/RoutingPushIntegrationTests/testPushOnARoutedBoardKeepsItsConnections`
  → `** TEST SUCCEEDED **` (est. 5 min: AFU 35 s and RegFile about 50 s in Debug,
  the figures plan-38's table gives for negotiation alone, and up to 3 × the nets of
  RegFile Fast runs on a 32-pad board for the document test; the Release tests are
  not run here because their build is 8 min and they are the landing tier).
  Measured: `testPushOnTheFixtures` and `testNegotiationOnTheFixtures` `Executed 2 tests, with 0 failures` (403.7 s: the
  off test routes LFU too, 330 s of it); `RoutingPushIntegrationTests` `Executed 7 tests, with 0 failures` in 180.6 s
  (the document test 32 cases, each routed with push on and off); `** TEST SUCCEEDED **`.
Commit: 6357ddf6

## Validation Plan

- Tests: `RoutingPushIntegrationTests` (new, 7), `RouterFixtureTests` (+1),
  `RouterQualityFixtureTests` (+3, Release). Negative cases: locked, cancelled,
  off.
- Coverage check: the hook line in `join` is executed by `testPushRoutesWhatOffCannot`
  (the push branch) and by every other router test (the not-taken branch).
- Runtime proof: the document-level test runs the real `routingProblem` →
  `Router.route` → `applyRouting` → DRC → undo path.
- Doc sync: none here (plan-66-F).
- Final gate (once, here): the two Check commands (est. 7 min).

## Decisions

plan-66-A § Decisions apply. Added by the author, 2026-11-25:

- **One hook, in `join`, before the rip-up; all three strategies push** — the
  option is the switch. (§ 3)
- **A tie between runs goes to the one that moved less existing copper** —
  least disturbance. (§ 3)
- **The fixtures gate on routed count, router-caused DRC and bounds measured
  once, not on identity with the off result** — a push changes the order of
  routing wherever a hard search fails. (§ 3)

## Corrections

- **`tryPush` takes `blocked:`** (2026-11-25): `join` has just searched for a legal way and
  found none, and a failed search is the expensive kind (it visits everything reachable), so
  the hook passes `blocked: true` and the push's own first hard search is skipped.
- **Measured-count mislabels in the archived A and D** were corrected: RoutingNegotiationTests
  has 11 tests and RoutingPolishTests 10 (the plans had them the other way about).
- **The document test's widths are 12, 14, 16 and 18 mil, not 20, 30 and 40** (2026-11-25).
  The first run with the plan's widths had 24 cases and no case that routed completely by
  moving copper: on this card a net re-routed at 20 mil or more routes 1 of 3 connections with
  or without push (24 mil: 0 of 3 for LATCH1.18). A probe of 6 widths × 8 nets × push off and on
  (48 pairs) showed pushing never routes less and, between 12 and 18 mil, gets 8 of 32 cases to
  3 of 3 where off left 1 or 2: `PUSHCASE net=LATCH1.13 width=14 moved=2+1`, `LATCH1.14 14 1+0`,
  `LATCH1.17 12/14/16 3+1`, `LATCH1.18 16/18 3+1`, `LATCH1.19 18 6+3` (the first is the case
  plan-66-F's real-app pass reproduces). The test asserts, for each case, that push routed at
  least as many connections as off.
- **The fixtures never push anything that matters** (2026-11-25): with the option on, AFU
  routes 115 of 115 with `pushTried 0` (every first search found a way), and RegFile 24 of 24
  with `pushTried 28, pushKept 0` and the same 26 vias, 312 mm and 33 rips as off; the fixtures
  hold no copper, so only the run's own connections could be pushed. The Release tests'
  bounds are therefore the off tests' (`testPushAFU`/`RegFile`/`LFU`) until the landing tier
  gives the numbers.

## Summary

E is a hook of eight lines and a set of gates. The risk is regression on boards
where negotiation used to succeed by ripping; the fixtures' routed floors and
Release bounds are the check, and the option stays off by default until the
owner has seen the numbers.
