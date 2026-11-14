# plan-38-J: Net groups — coupled differential-pair routing

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-I

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F, G, H, I) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

A group in **Differential Pair** mode (plan-38-B) is routed as one unit: the two
nets run side by side on the same layer at the group's **gap**, change layers
together with a staggered via pair, break out to their pads with short
uncoupled legs (at most the group's **max uncoupled** in total), and finish with
the length tuning of plan-38-I so the two nets match within the tolerance. When
a pair cannot be routed coupled, its two nets are routed as ordinary nets, tuned,
and reported `uncoupled` with the reason; the DRC reports a pair that runs apart
too far.

References:

- plan-38-B § 3 (`NetGroup`: `gap`, `maxUncoupled`, `tolerance`, a pair has exactly
  2 members, one width), plan-38-E (grid profiles, search), plan-38-F (ledger,
  negotiation), plan-38-H (verifier, chamfer), plan-38-I (tuner, `GroupOutcome`).

## 1. Goal

- A pair of two-pin nets on AFU-like open board space is routed with the two
  traces parallel at `gap` (edge to edge, ±0.001 mm) for all but the breakouts;
  uncoupled length ≤ `maxUncoupled`; both on one layer at a time; lengths within
  `tolerance`; the twelve DRC kinds of plan-38-H are 0 after applying.
- A pair that must change layers does so with two vias staggered along the run so
  both vias clear each other and the pair's other copper.
- A pair that cannot be coupled is routed uncoupled and `GroupOutcome.status ==
  .uncoupled(reason)`; nothing is left half-routed.
- `DesignRuleIssue.Kind.diffPair`: one line per pair whose uncoupled length exceeds
  its `maxUncoupled` (§ 3 defines uncoupled length, which counts a stretch where
  the two nets are on different layers).

### Non-goals (explicit constraints)

- No impedance calculation, no stackup model: `gap` is the owner's number.
- No pairs of more than two nets (a bus is a length group, plan-38-I).
- No coupled routing through kept (hand-drawn) copper: a pair's search treats kept
  copper as obstacle, like every net.
- No pair-aware push-and-shove; the pair is rip-up unit in the negotiation of F.

## 2. Current State

- After plan-38-I a pair is a length group; routing and tuning treat its nets
  individually.
- The grid engine routes one net profile at a time (plan-38-E § 3); a profile
  is `(halfWidth, clearance, viaRadius)`; a ledger entry is the copper of one
  connection of one net (plan-38-F).
- A via has `diameter` and `drill`, and the DRC's clearance counts its metal
  (`CopperGeometry`: a via is a zero-length segment with radius `diameter / 2`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Two-pin nets in AFU / LFU (pair candidates in tests) | 41 / 38 | `planning/plan-38-data/spike/results.txt` |
| Via metal vs pair spacing at the Custom defaults: pair centre spacing `w + gap` = 0.254 + 0.2 = 0.454 mm; two vias side by side need `diameter + clearance` = 0.5 + 0.1 = 0.6 mm | 0.454 < 0.6 → side-by-side vias overlap | arithmetic on `results.txt` rules (via 0.5 mm, clearance 0.1 mm) and the Trace default 10 mil (`BoardTrace.defaultWidth`) |

### Verified properties

- At the defaults a pair's two vias cannot sit side by side (0.454 < 0.6 above), so
  the staggered via pair of § 3 is required, not optional. The stagger distance
  along the run is `√(0.6² − 0.454²) = 0.392 mm` at these numbers.

## 3. Design Overview

**Terminals.** Pair `(P, N)`, each two-pin: pads `p₁, p₂` of P and `n₁, n₂` of N.
The pairing `(p₁ with n₁, p₂ with n₂)` or `(p₁ with n₂, p₂ with n₁)` that has the
smaller `|p–n|` sum is used; its two ends are the pair's **ends**, each with a
midpoint `M`.

**Fat route.** The pair's centreline is searched as one net with profile
`halfWidth = (2w + g)/2`, `clearance = c`, via radius `R_pair` (a disc that encloses
a staggered via pair, conservatively: `R_pair = (s + D + Δ)/2` with `s = w + g`, `D` the via
diameter, `Δ` the stagger below), on the pair's allowed layers (intersection of
both nets' rules). Sources and targets are the legal cells within
`maxUncoupled / 2` of each end's `M`, each with an initial cost equal to its
distance from `M`, so the search prefers to start close. The effective gap is
`g_eff = max(g, rules.minClearance)`.

**Expand.** The centreline polyline is cleaned like any chain (plan-38-H straighten,
chamfer, against the fat profile), then offset to `P′`, `N′` at `±s/2` with
mitre joins (chamfer corners keep the spacing). The side with `p` nearest at the
first end is P's, for the whole run (the pair never swaps sides).

**Layer change.** The fat route's layer change at a cell becomes two vias, `V_P`
at `centre + (s/2)·lateral − (Δ/2)·along` and `V_N` at `centre − (s/2)·lateral +
(Δ/2)·along`, with `Δ = √((D + c)² − s²)` (0 if `s ≥ D + c`). Each of the pair's two
traces then continues on the new layer from its own via.

**Breakouts.** At each end, `P′` and `N′` connect to their pad centres with an
octilinear leg each (the shortest legal, tried both bend orders), checked by the
verifier. **Uncoupled length** = the sum of the breakout legs plus every stretch
of P′ or N′ whose gap to the other trace is not `g_eff ± 0.02 mm`. It must be ≤
`maxUncoupled`, else the pair is `uncoupled`.

**Verify and fall back.** The whole pair goes through plan-38-H's verifier; any
violation, a failed search, a breakout over `maxUncoupled`, or an `Only` rule that
leaves no shared layer → the pair is dropped and its two nets are routed as
ordinary nets (E/F) and tuned (I), `GroupOutcome.status = .uncoupled(reason)`.

**Negotiation.** A pair is one queue entry (both nets' copper in one ledger entry);
it routes **before** the ordinary nets (strictly first, then by span) so it gets
the straight room; it can be ripped up as a unit as a victim; a pair ripped and
re-routed keeps its place at the front of the queue.

**Skew.** After coupling, plan-38-I's tuner equalises P and N within `tolerance`, with
the rule that bumps go on the *outer* side of the shorter trace only (away from the
partner), so the pair is never squeezed.

**DRC `diffPair`.** For each pair with both nets routed: uncoupled length > `maxUncoupled
+ 1e-4` → "Pair HDMI-CLK: 12.4 mm runs apart (max 5 mm)"; the two nets on different
layers at the same stretch counts as uncoupled there.

Where correctness risk concentrates: the staggered via pair and the offset at
corners (a mitre that closes the gap); both are checked by the verifier and by
geometry tests with hand-computed numbers.

Byte-identity is not a gate; legality and the measured gap are.

Rejected: routing P first then N "next to it" (N would be routed around P's
copper, not coupled; the fat route couples by construction); fixed pair vias side by
side (overlap, § 2); coupling only by cost (a cost cannot hold a gap to
±0.02 mm).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — pair geometry

- [x] `Routing/PairGeometry.swift`: end pairing, `offset(polyline, by:)` with mitre joins (and the chamfered-corner case), `stagger(D:c:s:)`, via pair placement, `uncoupledLength(P:N:gap:)`. (The code landed with plan-38-I Phase 3, a6d46d84, which shares files with it.)
- [x] Tests (`PairGeometryTests`): the offset of a straight, an L and a chamfered L stays `s` from the centreline everywhere (sampled every 0.01 mm to 1e-6); pairing picks the smaller sum; the stagger at the Custom defaults is 0.392 mm (±0.001); two vias of the pair clear each other by ≥ `c`; `uncoupledLength` of two parallel traces at `g` is 0 and of a trace that diverges 3 mm is the hand-computed value. (Offset criterion strengthened to what holds, see Corrections; the divergence's hand value is 3 − √((s + 0.02)² − s²) + 3 − 0.02 = 5.844 mm, measured 5.84.) — `-only-testing:PairGeometryTests` → 4 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: cases pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/PairGeometryTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: b0a33fcd

### Phase 2 — the fat search and expansion

- [x] `Routing/RoutingGrid.swift`: pair profile and pair-via map; `Routing/RoutingSearch.swift`: multi-source/target with initial costs; `Routing/RoutingPair.swift`: route centreline → clean → offset → vias → breakouts → verify (§ 3). (No grid change needed: the pair gets a grid of its own from a copy of the problem where N's copper is P's and one fat net, `2w + g` wide with a via disc of `viaSpacing + D`, replaces both; its profile and via map are that grid's. Search: `startCosts` and `targetCosts`, see Corrections. Vias: flared side by side, see Corrections.)
- [x] `Routing/RoutingState.swift`: a ledger entry may hold several nets' copper; stamp/unstamp every member. (Kept as one entry per net, which the ledger already stamps and un-stamps exactly; the pair is handled as a unit in Phase 3 by ripping and re-routing both nets' entries together.)
- [x] Tests (`RoutingPairTests`): open board, pads 20 mm apart, gap 0.2: both traces parallel at 0.2 ± 0.001 over all but the breakouts; a blocking trace forces a layer change → two vias, staggered, DRC-clean after apply; a width-0.5 mm pair keeps its own clearance from a Default neighbour; a pair with an Only{Top} rule never uses a via; a channel 0.5 mm wide (too narrow for `2w + g`) → `uncoupled`. (The layer-change case asserts two vias per change that clear each other and a clean verifier, not a stagger; "DRC-clean after apply" is the verifier on these engine-only boards, as in plan-38-I.) — `-only-testing:RoutingPairTests -only-testing:RoutingSearchTests` → 15 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/RoutingPairTests -only-testing:schematicalTests/RoutingSearchTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: 18e96d72

### Phase 3 — integration, fallback, skew, DRC

- [x] `Router.swift`: pairs first in the queue, as one entry; fallback to ordinary routing; skew tuning via plan-38-I with the outer-side rule; `GroupOutcome.status` gains `.uncoupled(reason)` and `.coupled(uncoupled: mm)`. (`Run.routePairs` before everything, clear of the ledger's copper; a victim that is a pair net rips its partner too, and the pair is routed again, coupled, at the start of the next round, its nets kept out of the ordinary queue meanwhile; a pair that can't be coupled is `uncouple`d and its nets route as ordinary nets. Skew: `RoutingTuning.tune(..., pairs:)` bumps the shorter net only on the side whose normal points away from the partner.)
- [x] `DesignRuleCheck.swift`: `Kind.diffPair` and `checkDiffPairs()` (§ 3).
- [x] Tests: `RoutingPairTests` — a pair's skew after tuning ≤ tolerance; the shorter trace's bumps are on the outer side; a pair ripped as a victim re-routes first; pair + 10 ordinary nets on an open board all complete and DRC-clean. `DesignRuleCheckTests` — a hand-drawn pair whose traces diverge 8 mm with `maxUncoupled` 5 gives one `.diffPair` line; a coupled pair gives none. (The pair-ripping board walls net 3 to Top across the pair's way, so it must rip the pair, which comes back coupled across it on Bottom with 4 vias. The DRC's "none" case is a pair with one net taken out; that coupled copper reads 0 runs-apart is `PairGeometryTests`'.) — `-only-testing:RoutingPairTests -only-testing:DesignRuleCheckTests` → 24 tests, 0 failures; with every engine suite and the goldens (14 classes) → `** TEST SUCCEEDED **`.

Acceptance: cases pass; the 11 DRC tests and goldens unchanged.
  Check: `-only-testing:schematicalTests/RoutingPairTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/DesignRuleGoldenTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: d784ff8b

### Phase 4 — fixtures

- [x] `RouterFixtureTests`: on AFU, a pair of the first two two-pin nets by net index, gap 0.2, plus everything else routed: assert the twelve DRC kinds are 0 and that `GroupOutcome.status` is `.coupled` or `.uncoupled(reason)` with the measured uncoupled length equal to `PairGeometry.uncoupledLength` of the applied copper; print status, uncoupled mm, skew mm. — `testDiffPairOnAFU` passed, `** TEST SUCCEEDED **` (30 s).
- [x] Results (the output of this run, filled in then): status `uncoupled("no legal breakout to a pad")` (AFU's first two two-pin nets don't start side by side, so the pair falls back to two ordinary nets), uncoupled — (the applied copper's two nets measure 31.13 mm apart), skew 0.38 mm; all 115 connections routed, nothing router-caused in the DRC.

Acceptance: as stated.
  Check: `-only-testing:schematicalTests/RouterFixtureTests` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `PairGeometryTests`, `RoutingPairTests`, `DesignRuleCheckTests`, `DesignRuleGoldenTests`, `RouterFixtureTests`.
- Coverage check: the stagger, offset-at-corner, fallback and outer-side branches each have a test that fails if removed.
- Runtime proof: plan-38-K shows a pair routed in the real app; here the fixture run.
- Doc sync: described in `docs/routing.md` (plan-38-M).
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions (owner: coupled differential-pair routing, 2026-11-13). Author decisions: fat-route-and-offset; staggered via pair; breakouts uncoupled up to `maxUncoupled`; pairs route first and rip as a unit; fall back to uncoupled + tuning, reported; outer-side skew bumps; no impedance model.

## Corrections

- **"The offset stays `s` from the centreline everywhere" can't hold at an
  outer corner.** A mitre's corner point stands `s / cos(θ/2)` from the
  centreline's corner (√2 · s at 90°, 1.082 · s at 45°), so samples there are
  farther than `s`; a round join would stay at `s` but isn't octilinear. What
  the pair needs, and what is tested: never closer than `s` anywhere (so the
  gap never shrinks below `gap`), exactly `s` at every straight part's
  middle, and the 90° mitre exactly √2 · s out.
- The 4th check's "hand-computed value" for a 3 mm divergence counts the
  tolerance band: P stays coupled to N's corner for √((s + 0.02)² − s²) =
  0.136 mm, N's rise for 0.02 mm, so 5.844 mm, not 6.
- **A staggered via pair is illegal at these sizes (§ 3 "Layer change").**
  The trace whose via comes later on the run passes beside the other net's
  via at the pair's centre spacing `s`; that needs `D/2 + w/2 + c`. Measured
  with the 0.8 mm via: P's trace to N's via gap −0.073 mm (`RoutingPair`
  probe, the verifier's clearance line); at PCBWay's 0.5 mm via it needs
  0.477 > 0.454 too. No stagger fixes it, as the trace passes the via either
  way. Replaced with a **flared** via pair: the two vias side by side at
  `L = max(D + c, D/2 + w/2 + c, s)` (`PairGeometry.viaSpacing`), each trace
  leaving its lane at 45° `(L − s)/2` before the via and returning as far
  after; the flares count as uncoupled length. The fat via disc is `L + D`.
  `stagger` stays, tested, but unused.
- **End costs must outweigh the walk.** § 3 gives sources "an initial cost
  equal to its distance from M". That ties with walking the same distance, and
  targets had no cost at all, so the centreline started and ended at the far
  edge of each end's region: 10.11 mm of breakouts on the open test board.
  Now both ends cost 3 × the distance (in cells), the search supports
  `targetCosts` (it continues until nothing open can beat the best target
  plus its cost), and the regions are at most a third of the end-to-end span,
  so they can't overlap: 1.06 mm uncoupled on the same board.
- An end's cells are only on the layers both of its pads are on (a Top
  surface pad: Top). Without that, the walled test board's pair ran wholly
  on Bottom, its ends joining nothing.

## Summary

The geometry is the risk: a via pair that fits and an offset that holds the gap
through corners. The fallback guarantees a pair is never left half-routed.
Untouched: ordinary nets' behavior (E, F), the file format.
