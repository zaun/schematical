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

- [ ] `Routing/PairGeometry.swift`: end pairing, `offset(polyline, by:)` with mitre joins (and the chamfered-corner case), `stagger(D:c:s:)`, via pair placement, `uncoupledLength(P:N:gap:)`.
- [ ] Tests (`PairGeometryTests`): the offset of a straight, an L and a chamfered L stays `s` from the centreline everywhere (sampled every 0.01 mm to 1e-6); pairing picks the smaller sum; the stagger at the Custom defaults is 0.392 mm (±0.001); two vias of the pair clear each other by ≥ `c`; `uncoupledLength` of two parallel traces at `g` is 0 and of a trace that diverges 3 mm is the hand-computed value.

Acceptance: cases pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/PairGeometryTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: —

### Phase 2 — the fat search and expansion

- [ ] `Routing/RoutingGrid.swift`: pair profile and pair-via map; `Routing/RoutingSearch.swift`: multi-source/target with initial costs; `Routing/RoutingPair.swift`: route centreline → clean → offset → vias → breakouts → verify (§ 3).
- [ ] `Routing/RoutingState.swift`: a ledger entry may hold several nets' copper; stamp/unstamp every member.
- [ ] Tests (`RoutingPairTests`): open board, pads 20 mm apart, gap 0.2: both traces parallel at 0.2 ± 0.001 over all but the breakouts; a blocking trace forces a layer change → two vias, staggered, DRC-clean after apply; a width-0.5 mm pair keeps its own clearance from a Default neighbour; a pair with an Only{Top} rule never uses a via; a channel 0.5 mm wide (too narrow for `2w + g`) → `uncoupled`.

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/RoutingPairTests -only-testing:schematicalTests/RoutingSearchTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 3 — integration, fallback, skew, DRC

- [ ] `Router.swift`: pairs first in the queue, as one entry; fallback to ordinary routing; skew tuning via plan-38-I with the outer-side rule; `GroupOutcome.status` gains `.uncoupled(reason)` and `.coupled(uncoupled: mm)`.
- [ ] `DesignRuleCheck.swift`: `Kind.diffPair` and `checkDiffPairs()` (§ 3).
- [ ] Tests: `RoutingPairTests` — a pair's skew after tuning ≤ tolerance; the shorter trace's bumps are on the outer side; a pair ripped as a victim re-routes first; pair + 10 ordinary nets on an open board all complete and DRC-clean. `DesignRuleCheckTests` — a hand-drawn pair whose traces diverge 8 mm with `maxUncoupled` 5 gives one `.diffPair` line; a coupled pair gives none.

Acceptance: cases pass; the 11 DRC tests and goldens unchanged.
  Check: `-only-testing:schematicalTests/RoutingPairTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/DesignRuleGoldenTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 4 — fixtures

- [ ] `RouterFixtureTests`: on AFU, a pair of the first two two-pin nets by net index, gap 0.2, plus everything else routed: assert the twelve DRC kinds are 0 and that `GroupOutcome.status` is `.coupled` or `.uncoupled(reason)` with the measured uncoupled length equal to `PairGeometry.uncoupledLength` of the applied copper; print status, uncoupled mm, skew mm.
- [ ] Results (the output of this run, filled in then): status __, uncoupled __ mm, skew __ mm.

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

(Filled in during execution.)

## Summary

The geometry is the risk: a via pair that fits and an offset that holds the gap
through corners. The fallback guarantees a pair is never left half-routed.
Untouched: ordinary nets' behavior (E, F), the file format.
