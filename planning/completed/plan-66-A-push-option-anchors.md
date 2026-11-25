# plan-66-A: Push and shove — the option, the stats, the regression anchors

Last updated: 2026-11-25
Overall Effort: x-large (1d–3d)
Effort: medium (1h–2h)
Depends on: nothing

Adds **push and shove** to the Autoroute sheet: when a connection has no legal
way, the router moves the unlocked copper of other nets aside — copper it put
down in this run and copper already on the board, hand-drawn or autorouted —
instead of ripping its own copper up and giving up on the rest. Locked copper
never moves. The whole feature is plan-66 A–F; this letter is its first
landable piece: the option itself (off by default, remembered, shown in the
sheet), the counters the later letters fill in, and **regression anchors**
that pin the router's exact output with the option off, so every later letter
can prove it changed nothing for anyone who leaves the option off.

Teardrops are **not** part of this plan (§ Decisions).

References:

- `docs/routing.md` (§ The Autoroute sheet's option table, § The engine, § Limits:
  "No push-and-shove: kept copper never moves (a deliberate scope decision)").
- `planning/completed/plan-38-F-router-negotiation.md` (the ledger, rip-up and
  history), `plan-38-H-router-cleanup.md` (cleanup, verify, repair).
- `schematical/schematical/Routing/Router.swift`, `RoutingState.swift`,
  `RoutingPolish.swift`, `RoutingSearch.swift`, `RoutingRepair.swift`,
  `RoutingApply.swift`, `RoutingProblemBuilder.swift`, `RoutingOptions.swift`.
- `AGENTS.md` § Git (commit on the current branch; itemized commits; a new
  plan is one commit).

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Letters B–F point here.

| Must be true | Command | Status |
|---|---|---|
| The cutouts plan's router letter has landed | `ls planning/completed \| grep -c '^plan-65-D'` → 1. It edits `RoutingProblem.swift`, `RoutingProblemBuilder.swift`, `RoutingGrid.swift` (`stampEdge`), `RoutingVerifier.swift`, `RoutingCleanup.swift` and `RoutingTestBoard.swift`, files this plan edits too; two plans may not braid, so this one starts after it | MET (2026-11-25, re-run on the second `/follow-plan 66`: `ls planning/completed \| grep -c '^plan-65-D'` → 1; plan-65-A…J are all in `planning/completed/`; router commit `7ade3de0` "plan-65-D: the autorouter reads the board edge"). The first invocation found it NOT MET (→ 0) and stopped |
| No other unfinished plan edits the router | `grep -ln 'Routing/' planning/plan-*.md \| grep -v 'plan-66\|plan-65'` → no output; `git log --oneline -3 -- schematical/schematical/Routing` shows only commits already on `main` | MET (2026-11-25 re-run: the grep → no output (`plan-63-F` is no longer listed); newest routing commit `7ade3de0`, on `main`) |
| The router's own tests pass on the tree this plan starts from | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/RoutingNegotiationTests -only-testing:schematicalTests/RoutingPolishTests -only-testing:schematicalTests/RoutingApplyTests -only-testing:schematicalTests/RoutingOptionsTests` → `** TEST SUCCEEDED **` | MET (2026-11-25, re-run in `worktree-P-66` on `main` at `2cc13337` (plan-65 landed): `Executed 34 tests, with 0 failures` in 58.1 s, `** TEST SUCCEEDED **`; the test files had the Phase 1 board move applied mid-build — a pure move, same 34 tests) |
| The option does not exist yet | `grep -rn 'pushAndShove' schematical docs planning --include='*.swift' --include='*.md' -l \| grep -v plan-66` → no output | MET (2026-11-25, re-run: no output) |

Everything below is written against the world where these hold. There is no
fallback for a world where they don't. Line numbers in this plan are from the
tree of 2026-11-25, before plan-65-D changed the files it names; the symbol
names are the reference, and a line number that has moved is not a correction.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `RoutingOptions.pushAndShove` exists (default `false`), is saved with the
  other options, decodes from a file that lacks it as `false`, and is a toggle
  "Push and shove" in the Autoroute sheet.
- `RoutingStats` carries the counters the engine letters fill in
  (`pushTried`, `pushKept`, `movedTraces`, `movedVias`), all zero for now.
- With the option off, the router's output on four fixed boards is pinned
  exactly (traces, vias, bends, wire), and the pin passes on the tree before and
  after this letter.

### Non-goals (explicit constraints)

- No engine behaviour changes: with the option on or off the router does
  exactly what it does today. (The option is read by nothing but the sheet and
  its tests until plan-66-C.)
- No change to any other option's meaning, default or saved name; no change to
  how a saved `autorouteOptions` from before this letter decodes.
- No automation (MCP) surface: `docs/agent-control.md` lists autorouting as not
  exposed, and it stays so.
- No teardrops, no Trace-tool push-and-shove (§ Decisions).

## 2. Current State

`RoutingOptions` (`Routing/RoutingOptions.swift`) is `Codable` with an explicit
`CodingKeys` list and `decodeIfPresent(…) ?? default` for every field, so adding
a field with a default never breaks an old saved value. The Autoroute sheet shows
the toggles `tuneGroups`, `routePairs` and `fanoutPlanes` at
`Routing/AutorouteSheet.swift:117-119`; the model remembers every option but
`scope` (`Routing/AutorouteModel.swift:67-90`, key `PreferenceKey.autorouteOptions`).
`RoutingStats` (`Routing/RoutingResult.swift:31`) holds the run's counters
(`rips`, `polishTried`, `polishKept`, …).

The word **shove** is already used in the router for something else:
`Router.Run.join(…, shove: Bool)` and `attempt(rip:shove:…)`
(`Router.swift:437`, `RoutingPolish.swift:47`) mean "an improvement move that
also looks through other nets' copper and routes the nets it crossed again"
(`grep -c 'shove' Routing/*.swift` → `Router.swift:3`, `RoutingPolish.swift:6`).
It stays as it is. Everything this plan adds is named **push**.

### Measured populations

| What | Count | Command |
|---|---|---|
| Routing source files, lines | 26 files, 5425 lines | `ls schematical/schematical/Routing/*.swift \| wc -l` → 26; `cat schematical/schematical/Routing/*.swift \| wc -l` → 5425 |
| Router test files | 25 | `ls schematical/schematicalTests \| grep -E '^(Rout\|Router\|Autoroute\|Assist)' \| wc -l` → 25 |
| Option toggles in the sheet | 3 | `grep -n 'Toggle(' schematical/schematical/Routing/AutorouteSheet.swift` → 5 lines: `tuneGroups` (117), `routePairs` (118), `fanoutPlanes` (119), and two that are not options (127 a layer row, 150 a field switch) |
| Consumers of `problem.obstacles` outside the builder | 13 uses in 5 files | `grep -n 'problem.obstacles' schematical/schematical/Routing/*.swift \| grep -v RoutingProblemBuilder \| wc -l` → 13 |

### Verified properties

- **Old saved options still decode.** `RoutingOptions.init(from:)` uses
  `decodeIfPresent` for all 15 keys (read, `RoutingOptions.swift:101-124`), so a
  new key with a default is invisible to an old value.
- **A test can pin the router's output.** `Router.route` is deterministic ("same
  board and options, same copper": `docs/routing.md` § Guarantees, and
  `RoutingNegotiationTests.testSameResultTwiceAndInAnyNetOrder`). Its result
  holds every trace point, so a hash of the rounded points pins the geometry.
- **The boards exist.** `twoGaps` (`RoutingNegotiationTests.swift:85`, where
  Fast gets there only by ripping up), `reversedChannel` (`:123`), `crossing`
  (`RoutingPolishTests.swift:56`) are `RoutingTestBoard`s the router's own tests
  already route (`twoGaps` to completion under every strategy,
  `RoutingNegotiationTests.swift:104-119`).

## 3. Design Overview (the whole of plan-66)

**What push and shove is here.** The router is a grid router: A* over layer ×
cell, every routed connection a ledger entry, the grid stamped from exact
distances, a result that is legal at every moment (`docs/routing.md` § The
engine). A continuous-geometry shove engine (move segments, insert jogs) would
be a second geometry engine beside that one. Push and shove here is built on
the first:

> When the hard search for a connection fails and the relaxed search finds a
> way through other nets' copper, the connections in the way are taken up and
> **re-laid at once, within a corridor around where they were**, priced to
> stay close to their old line, instead of being queued to be routed again
> anywhere. If every one of them finds its place, the new connection stands and
> they have moved; if any does not, the state goes back exactly as it was and
> the router does what it does today.

The user-visible meaning is the usual one: existing copper gives way a little;
it keeps its ends, its layer sequence and its number of vias; it moves within
`pushReach` of where it was. What differs from a continuous shove is that a
pushed trace is laid again on the routing grid and cleaned (straightened,
chamfered) like new copper, so a hand-drawn trace that is pushed comes out as a
tidy octilinear trace along roughly the old line, not as the old shape nudged.
Copper that is not in the way is never touched, byte for byte.

**The pieces, in landing order.**

| Letter | What | Risk |
|---|---|---|
| A | the option, the counters, the anchors | none: nothing reads the option |
| B | `MovableChain`: which existing copper may be pushed, extracted from the document | the chain rules (a wrong chain moves copper that should stay) |
| C | the engine holds existing chains as ledger entries of "bystander" nets that can be pushed but never ripped; cleanup freezes untouched ones; the result lists `moved`; apply splices them into the document | what the relaxed search can now cross; the splice into `BoardTrace` polylines |
| D | `push(…)`: the corridor search and the all-or-nothing transaction | the accept rule; the atomic restore |
| E | `push` in the negotiation, the improvement and every strategy; the fixtures | determinism, time, no regression on AFU/RegFile/LFU |
| F | the sheet's result line, the docs, a pass in the real app, the landing tier | none beyond the gates |

**Where correctness risk concentrates.** Not in legality: every push is a state
change inside the existing ledger (stamp and un-stamp are exact, restore is a
value copy as in `RoutingPolish.attempt`), and the finished copper still goes
through cleanup, `RoutingVerifier` and repair. It concentrates in (1) C, because
until C the relaxed search cannot cross existing copper (it crosses only cells
the *fixed* map leaves free, `RoutingSearch.swift:258-263`), so C is the letter
that makes foreign copper crossable and must keep it un-rippable; (2) B's chain
rules, since they decide which existing copper may move at all; (3) the apply
splice in C, which must leave the board's geometric connectivity unchanged.

**The gate class.** With `pushAndShove` off the plan is **provably neutral**:
the problem the builder emits is the same value as today, so the result is the
same, and byte-identity (the anchors below) is the gate. With it on, behaviour
changes legitimately and the gates are tests, not identity. The boards on which
the two are **expected to differ** are exactly those where a blocked connection's
relaxed way crosses unlocked copper of a net the run does not route (or this
run's own copper that is pushable); on every other board the on and off results
are expected to be equal and a test says so. Identity failing with the option
off is a bug to root-cause, not a reason to stop.

**What was probed.** The code was read (Router, RoutingState, RoutingGrid,
RoutingSearch, RoutingPolish, RoutingRepair, RoutingProblemBuilder,
RoutingApply, `BoardConnectivity`) and the baseline run above was made. Facts
read, each cited in the letter that uses it: relaxed search crosses only
non-fixed cells; `RoutingState` is a value type restored by assignment
(`RoutingPolish.swift:51,70`); cleanup has a `frozen` set (`RoutingRepair.swift:18`);
board connectivity is geometric (`Canvas/Ratsnest.swift`, `BoardConnectivity`).
No premise waits on a later measurement.

**Rejected alternatives.**

- *A continuous-geometry shove engine* (KiCad/Topor style). A second geometry
  engine, a second legality story, and none of the grid's exact stamp/un-stamp
  and snapshot/restore. The grid version gives the same user-visible behaviour
  at the cost of re-laying a pushed trace instead of nudging it.
- *Pushing by re-queueing victims* (what rip-up already does). Victims are
  routed again anywhere, in queue order; the result is not local and can lose
  the connection that had been routed. The corridor makes it local and the
  transaction makes it all-or-nothing.
- *Default on.* It can move hand-drawn copper. The option is off until the
  owner decides otherwise; turning the default on later is a one-line change
  and a golden update.
- *Pushing the run's own nets' existing copper.* A net the run routes keeps its
  existing copper where it is: its terminals and tie points are computed from
  it once (`RoutingTerminals.swift`), and moving it would invalidate them.
  Copper of every other unlocked net, and every connection this run makes, can
  be pushed.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial (say what remains). A moot task is
> `- [x] ~~text~~ — moot: <evidence>`, never deleted. Fill `Commit:` as each
> phase lands. An unticked box means NOT DONE.

### Phase 1 — the anchors (on the tree as it is, before any other edit)

Pin today's output first, so the pin is recorded against the code that exists.

- [x] (`RoutingTestBoard.swift` now has the `extension RoutingTestBoard` with the four boards; the call sites read `RoutingTestBoard.twoGaps()` etc.; the Check below passes) `schematicalTests/RoutingTestBoard.swift`: move the three boards `twoGaps`,
      `reversedChannel` (from `RoutingNegotiationTests.swift`) and `crossing`
      (from `RoutingPolishTests.swift`) out of those classes, unchanged, as
      `static func twoGaps() / reversedChannel() / crossing() -> RoutingTestBoard`
      in an `extension RoutingTestBoard`; the two test files call them there.
      Add a fourth, `static func parallelRows()`: `RoutingTestBoard(width: 20,
      height: 20)` with `options.grid = 0.127` and three straight nets on Top, each
      two 1 × 1 mm pads: net 1 at (3, 10) and (17, 10), net 2 at (3, 6) and
      (17, 6), net 3 at (3, 14) and (17, 14). Nothing blocks any of them, so the
      first pass routes all three and nothing is ever ripped up.
- [x] (eight board tests, literals filled from the unmodified tree; a second build and run reproduced every value: `Executed 9 tests, with 0 failures`) `schematicalTests/RoutingAnchorTests.swift` (new): for each of the four
      boards, `Router.route` at `.fast` and at `.balanced`, with the option
      untouched, then assert `routed`, `vias`, `bends`, `rips`, `wireLength` (to 1e-6) and
      `geometryHash`, a helper that rounds every point of every result trace and
      via to 1e-6 mm, orders them as returned, and folds the text into a 64-bit
      FNV-1a. The expected values are written into the test as literals, copied
      from the run on this unmodified tree (the test prints them with a
      `ANCHOR <board> <strategy> …` line when the literals are `nil`; the
      first run fills them in and the file is committed with the literals).
- [x] (`ANCHOR RegFile fast routed 24 vias 29 bends 87 rips 54 wire 301.450727907`, passes in Debug) A fifth test, `testRegFileFastIsPinned`: `Fixture.open("RegFile")`,
      `RoutingOptions.fixture` (Fast in Debug), the same assertions on the result,
      skipped in a Release build where `fixture` is Balanced.

Acceptance: the five tests pass on the tree before any later edit of this
letter, and their literals are in the file.
  Check: `XB -only-testing:schematicalTests/RoutingAnchorTests
  -only-testing:schematicalTests/RoutingNegotiationTests
  -only-testing:schematicalTests/RoutingPolishTests` → `** TEST SUCCEEDED **`
  (est. 3 min: the 1 m 52 s measured for four classes, plus RegFile at Fast in
  Debug, about 50 s, the figure plan-38's table gives).
  Measured: `Executed 30 tests, with 0 failures in 100.3 s`, `** TEST SUCCEEDED **`
  (anchors 9 in 47.6 s, Negotiation 10 in 27.1 s, Polish 11 in 25.6 s). The nine
  anchors are 8 board tests (four boards × Fast, Balanced) plus RegFile.
Commit: b827d66a

### Phase 2 — the option and the counters

- [x] (done; the Check below passes) `Routing/RoutingOptions.swift`: `var pushAndShove = false` with a doc
      comment ("move other nets' unlocked copper aside to make room;
      locked copper never moves"); add `pushAndShove` to `CodingKeys`, decode it
      with `decodeIfPresent(Bool.self, forKey: .pushAndShove) ?? false`, encode it.
- [x] (done) `Routing/RoutingResult.swift`: in `RoutingStats` add `var pushTried = 0`,
      `pushKept = 0`, `movedTraces = 0`, `movedVias = 0` (all `Int`, documented
      as "filled by plan-66-D/E").
- [x] (done; toggle after "Fan out to Planes") `Routing/AutorouteSheet.swift`: after the `Fan out to Planes` toggle
      (line 119) add `Toggle("Push and shove", isOn: $model.options.pushAndShove)`
      with `.help("Move other nets' unlocked traces and vias aside to make room.
      Locked copper never moves.")`.
- [x] (done: round-trip sets it; `testAnOldSavedValueHasPushAndShoveOff` added) `schematicalTests/RoutingOptionsTests.swift`: `testEveryFieldRoundTrips`
      sets `pushAndShove = true` and round-trips it; a new
      `testAnOldSavedValueHasPushAndShoveOff` decodes a JSON of the current
      `standard` options with the `pushAndShove` key removed and expects `false`.
- [x] (done) `schematicalTests/AutorouteModelTests.swift`:
      `testOptionsAreRememberedButNotTheScope` also sets and reads back
      `pushAndShove`.

Acceptance: the option round-trips, an old value decodes with it off, the sheet
shows the toggle, and the anchors still pass (nothing reads the option).
  Check: `XB -only-testing:schematicalTests/RoutingOptionsTests
  -only-testing:schematicalTests/AutorouteModelTests
  -only-testing:schematicalTests/RoutingAnchorTests` → `** TEST SUCCEEDED **`
  (est. 2 min).
  Measured: `Executed 23 tests, with 0 failures` (RoutingOptionsTests 5, AutorouteModelTests 9,
  RoutingAnchorTests 9 in 48.2 s — every anchor literal unchanged), `** TEST SUCCEEDED **`.
Commit: — (recorded in the next commit)

## Validation Plan

- Tests: `RoutingAnchorTests` (new, nine), `RoutingOptionsTests` (+1, 1 edited),
  `AutorouteModelTests` (1 edited).
- Coverage check: the anchors call `Router.route` at both strategies on four
  boards plus RegFile, so every stage of the engine (plan, negotiation,
  improvement, cleanup, repair) is in the pin that B–F must keep.
- Runtime proof: none beyond tests; the real-app pass is plan-66-F.
- Doc sync: none in this letter; `docs/routing.md`'s option table gets its row in
  plan-66-F, once the option does something.
- Final gate (run once, here): the two Check commands above (est. 4 min) — both run, one per phase: `Executed 30 tests, with 0 failures` and `Executed 23 tests, with 0 failures`, each `** TEST SUCCEEDED **`.

## Decisions

- **Push and shove covers all unlocked copper** — decided by the owner,
  2026-11-25. Existing traces and vias, hand-drawn or autorouted, may be moved,
  not just the run's own copper. Locked copper never moves. (§ 3)
- **Autorouter only** — decided by the owner, 2026-11-25. The Trace tool's Assist
  is unchanged; Route Selected Nets and the Route tool get push and shove because
  they use the same engine. (§ Non-goals)
- **No teardrops in this plan** — decided by the owner, 2026-11-25: "Don't add
  teardrops yet. This may be a board option, not an auto-router option." Nothing
  in plan-66 stores, generates or exports a teardrop. `planning/todo.md` keeps
  teardrops as a later item with that note (plan-66-F).
- **Default off** — decided by the author (2026-11-25): push and shove can move
  copper the user drew, so it is opt-in; changing the default later is a one-line
  change and an anchor update.
- **Grid push, not a continuous shove engine; own-net kept copper stays put;
  `push` is the name, `shove` stays as the router already uses it** — decided by
  the author, 2026-11-25 (§ 3, Rejected alternatives).

## Corrections

- **The anchor count is nine, not five** (2026-11-25). The plan says "five tests"
  and "(new, five)": four boards, each at Fast *and* Balanced, is eight board
  tests, plus the RegFile one. Measured: `Executed 9 tests` in `RoutingAnchorTests`.
  Validation Plan corrected below.
- **The Prerequisites gate was first NOT MET** (plan-65-D unlanded), the run
  stopped, and was repeated after the cutouts plan landed; the table above holds
  the later measurements.

## Summary

A is deliberately inert: it adds a switch that nothing reads, the counters, and
the pin that makes every later claim of "off changes nothing" checkable. The real
risk is in C and D; A makes them testable.
