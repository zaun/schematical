# plan-38-I: Net groups — length measurement, meander tuning, DRC length-match check

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-H

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F, G, H) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

Makes a **net group** (plan-38-B) mean something: after routing, every member of
a group ends within the group's tolerance of the longest member, by adding
meanders (accordion bumps) to the shorter nets, each placement proven legal by
plan-38-H's verifier. The DRC gains a **Length Match** check so a hand-edited
group is reported too, and the sidebar's group editor shows each member's
current length and its difference from the longest. Differential pairs run
side by side in plan-38-J; this letter treats a pair's two nets as a length
group and tunes them the same way.

References:

- plan-38-B § 3 (`NetGroup`: `tolerance`, `maxMeander`), plan-38-H (§ 3 pipeline,
  `RoutingVerifier`, chamfer), plan-38-F (ledger), plan-38-C (Net Groups editor).
- `Canvas/Ratsnest.swift` (`padsOfPin` is a local of `connectivity()`; this letter
  stores it), `DesignRuleCheck/DesignRuleCheck.swift` (`DesignRuleIssue.Kind`).

## 1. Goal

- `SchematicDocument.netLength(ofNet:in:) -> Double?` (mm; nil unless the net has
  exactly two pins and is routed) and `BoardConnectivity.pinCount(ofNet:)` exist.
- `Router.route` with `tuneGroups` returns, per group, a `GroupOutcome { target,
  lengths: [net: Double], spread, status: matched | residual(spread) | ineligible(net,
  reason) }`; on boards with room, `status == .matched`: `max − min ≤ tolerance`.
- The DRC reports `DesignRuleIssue.Kind.lengthMatch` for a group whose routed
  members differ by more than its tolerance (one line per group, naming the
  shortest and longest net and the difference), and an `ineligible` line for a
  member with more than two pins.
- Tuned copper passes plan-38-H's DRC-clean guarantee (the twelve kinds are 0).

### Non-goals (explicit constraints)

- The longest member is never lengthened or shortened: the target is the longest
  routed length (so a group can always be met upward, never downward).
- A via contributes 0 to a net's length (board thickness is not modelled).
- No serpentine on kept (hand-drawn/locked) copper: meanders go only into traces
  this run created (`auto`), unless the member net has no router-made trace, in
  which case it is reported `residual` with reason "no routed copper to tune".
- Multi-drop nets (more than two pins) are not matched; they are reported.
- No change to the file format.

## 2. Current State

- Groups exist as data and a sidebar editor (plan-38-B, C); nothing measures a
  net's length or edits copper for them.
- plan-38-H's pipeline ends with verified, cleaned copper per net; its verifier
  can test a candidate polyline against all other copper in value types.
- `connectivity()` computes `padsOfPin` ("<chip index>/<designator>" and
  "edge/<name>" → pad indices) as a local and drops it (`Ratsnest.swift`).
- Net length is not stored anywhere; a net's copper is the traces of its
  connectivity root group (`BoardConnectivity.root`, `traces`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Two-pin nets in AFU / LFU (candidates for length groups) | 41 of 53 / 38 of 59 | `planning/plan-38-data/spike/results.txt` § Connections per net |
| `DesignRuleIssue.Kind` cases at the start of plan-38 | 14 (`short` … `solderMask`); plan-38-B adds `layerRule` (15), this letter adds `lengthMatch` (16) | `sed -n 19,22p schematical/schematical/DesignRuleCheck/DesignRuleCheck.swift` at the commit before plan-38-B |

### Verified properties

- A net with exactly one connection (two pad groups) is a two-pin net or a net
  whose two pins each have several pads; counting **pins** (`padsOfPin` groups),
  not pads or components, is what distinguishes a true two-pin net from a
  multi-pad one: an edge finger on each board side shares one pin
  (`Ratsnest.swift`: both fingers append to `padsOfPin["edge/<name>"]`).

## 3. Design Overview

**Length.** `length(net) = Σ segment lengths` of the net's traces (kept and new),
`nil` when the net is not fully joined (more than one connectivity group) or
does not have exactly two pins. The engine computes it from result geometry,
the document from `connectivity()`; one shared function over `[RoutedTrace]`
(`RoutingLength.of(segments:)`) so they agree to 1e-9.

**Tuning** (inside `Router.route`, after plan-38-H's verify, for each group with
`tuneGroups`):

1. Eligible members: two-pin nets routed complete in this run; others →
   `ineligible` (reported, not tuned).
2. `T = max length`. For each member with `T − L > tolerance`, deficit `d = T − L −
   tolerance/2` (aim inside the band, not at its edge).
3. **Hosts**: straight segments of the member's *new* traces, longest first
   (ties by trace order), with `≥ 2 × (width + gap)` of room from either end and
   from any anchor, where `gap = max(net clearance, rules.minClearance, 2 × width)`
   is the edge-to-edge spacing between a meander's own legs.
4. **Bump**: a U on the segment's left or right (the side with more verified
   room first): two perpendicular legs of amplitude `a ≤ maxMeander`, leg centre
   spacing `width + gap`, flat top, corners chamfered 45° (plan-38-H chamfer) unless
   `angle: .orthogonal`. Consecutive bumps are one leg-spacing apart, alternating
   direction so the net's overall shape stays on its line (an accordion).
5. **Place** bumps greedily along a host while `d > 0`, each at the largest
   amplitude `≤ maxMeander` whose exact polyline passes the verifier; the last bump's
   amplitude is solved by bisection (to 1e-4 mm) so the measured length lands
   within the band. A failing bump is retried at 0.5 × the amplitude, then on
   the host's other side, then at the next host.
6. Stop when `T − L ≤ tolerance` (`matched`) or no host accepts a bump
   (`residual(spread)`).

Verification of every placement uses plan-38-H's `RoutingVerifier` on the changed
chain only (cost proportional to the chain). The result's other nets are never
moved, so tuning can't break another net.

**DRC.** `Kind.lengthMatch` (after `solderMask`): for each group, members with
`netLength` non-nil; if there are ≥ 2 and `max − min > tolerance + 1e-4` → "Group
HDMI: nets differ by 1.42 mm (CLK+ 41.30 mm, D0+ 39.88 mm); tolerance 0.25 mm";
each ineligible member → one line "Net X has more than two pins; its length is not
matched". Unrouted members are skipped (they already have `unrouted` lines).

Where correctness risk concentrates: the legality of each bump (every one goes
through the verifier) and the agreement of the engine's and the document's length.
Gate: behavior; the DRC-clean guarantee is the safety net.

Rejected: shortening the longest net by re-routing (not guaranteed to terminate and
it would move legal copper; reported as `residual` instead — the user can loosen
the tolerance or add room); 90° meanders only (the chamfer pass already exists);
tuning kept copper (it is the user's).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — measuring and the DRC check

- [x] `Canvas/Ratsnest.swift`: `BoardConnectivity.pinOfPad: [Int: String]` (the `padsOfPin` keys inverted) and `pinCount(ofNet:)`. (A pad of no pin, an unnumbered centre pad or a hole, counts as a pin of its own.)
- [x] `Routing/RoutingLength.swift`: `RoutingLength.of(segments:)`; `SchematicDocument.netLength(ofNet:in:)`. (Plus `RoutingLength.of(net:in:)` over a `BoardConnectivity`, which the DRC and the sidebar use, and `of(_ traces:)` for the engine.)
- [x] `DesignRuleCheck.swift`: `Kind.lengthMatch` and `checkLengthMatch()` (§ 3), after `checkSolderMask()` in `run()`.
- [x] Tests: `schematicalTests/NetLengthTests.swift` — a two-pin net routed with a 10 mm trace has length 10 (±1e-9); an unrouted net, a three-pin net and a net with two connectivity groups give nil; a group of two nets 1.0 mm apart with tolerance 0.25 gives one `.lengthMatch` line naming both; with tolerance 2.0 none; an ineligible member gives its line; the existing 11 DRC tests and `DesignRuleGoldenTests` stay green (no group, no new line). (The routed length is checked against its own trace's sum and the hand-computed 5 + Δx + 5 mm rather than a 10 mm trace: an LED pair can't sit exactly 10 mm apart on the 0.635 mm grid.) — `-only-testing:NetLengthTests -only-testing:DesignRuleCheckTests -only-testing:DesignRuleGoldenTests` (with `MeanderTests`) → NetLength 4/4, DRC 15/15, Meander 5/5; the golden's one failure in that run was the run-to-run order fixed in d45c7574 (plan-38-D Corrections), after which the golden passed 11 of 11 unchanged.

Acceptance: cases pass; goldens unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/NetLengthTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/DesignRuleGoldenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: 14fb77ba

### Phase 2 — the meander generator (pure geometry)

- [x] `Routing/Meander.swift`: `Meander.bump(on segment:, side:, amplitude:, spacing:, width:, chamfer:)` → replacement points; `Meander.added(length:)` closed form; accordion repetition; bisection helper `amplitude(forAdded:)`. (As `bump(from:to:at:side:amplitude:spacing:cut:)`, `added(amplitude:cut:)`, `accordion(...)`, `amplitude(forAdded:cut:spacing:in:)`; the cut is clamped to half a leg and half the top.)
- [x] Tests (`MeanderTests`): a rectangular bump of amplitude 2 mm adds exactly 4 mm; the chamfered one adds the same minus the hand-computed chamfer loss (`cut × (2 − √2)` per corner, 4 corners — corrected, see Corrections); an accordion of 5 bumps alternates sides and its end points equal the host segment's; `amplitude(forAdded:)` hits a target to 1e-4 mm; legs are `width + gap` apart centre to centre. — `-only-testing:MeanderTests` → 5 tests, 0 failures (in the Phase 1 run).

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/MeanderTests` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: ecb476ef

### Phase 3 — group tuning in the engine

- [x] `Routing/RoutingTuning.swift`: eligibility, host selection, greedy placement with the verifier, bisection, `GroupOutcome`; `Router.route` calls it after verification when `options.tuneGroups`; `RoutingResult.groups`. (Each bump is checked exactly before it lands, with plan-38-H's `RoutingCleanup.isLegal`; `Router.tuned` then runs the verifier over the whole result and puts back any tuned trace it faults. Pins come from `RoutingNet.pins`, new, set by `routingProblem` from `pinCount(ofNet:)`. Lengths count kept copper too.)
- [x] Tests (`RoutingTuningTests`): three parallel two-pin nets of 20, 24 and 30 mm on an open board, tolerance 0.25 → all within 0.25 of 30 (`matched`) and the DRC-clean kinds are 0 after applying; tolerance 5 → the 24 mm net gets no bump (`matched` without tuning); a net boxed in by foreign copper → `residual` with the measured spread and no violation; a three-pin member → `ineligible`; a member with only kept (hand-drawn) copper → `residual` "no routed copper to tune"; the longest net's geometry is identical before and after; engine length equals `netLength` of the applied document to 1e-6; two runs equal. (See Corrections: "tolerance 5" is 7, and the DRC and document-length checks of these engine-only boards are in Phase 4's AFU test.) — `-only-testing:RoutingTuningTests -only-testing:RouterTests` → 14 tests, 0 failures, `** TEST SUCCEEDED **` (RoutingTuningTests 7/7 after the tolerance fix). This commit also carries plan-38-J's engine code that shares these files (`routePairs`, frozen cleanup pieces, search start costs, `RoutingPair`, `PairGeometry`); J's tasks are ticked with J's own tests.

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/RoutingTuningTests -only-testing:schematicalTests/RouterTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: a6d46d84

### Phase 4 — fixtures and the sidebar

- [x] `RouterFixtureTests`: on AFU, a group of the first 4 two-pin nets by net index (tolerance 0.5 mm): after route + apply, the status is `matched` or `residual`, and `outcome.spread` equals the spread measured from the document; the twelve DRC kinds are 0; print group lengths before and after. — `testLengthGroupOnAFU` passed (with `NetLengthTests`, `NetListTests`: 11 tests, 0 failures, `** TEST SUCCEEDED **`).
- [x] `PCBSidebarView.swift` `NetGroupsSection`: each member row shows its length (`netLength`, "—" when nil) and the difference from the group's longest; a red mark beyond the tolerance. (In `Canvas/NetsSidebar.swift`: `NetLengthLabel`; `NetRow.length` from `RoutingLength.of(net:in:)`. Built in the same run.)
- [x] Results (the output of the AFU run, filled in then): lengths 15.37, 15.75, 30.61, 12.68 → 26.03, 15.75, 30.61, 12.68 mm, status `residual` (spread 17.93 mm; AFU is dense: only the first net found room for bumps, and the longest, 30.61, is never touched).

Acceptance: AFU group consistent as stated; build succeeds.
  Check: `-only-testing:schematicalTests/RouterFixtureTests -only-testing:schematicalTests/NetLengthTests` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: 04d191fd

## Validation Plan

- Tests: `NetLengthTests`, `MeanderTests`, `RoutingTuningTests`, `RouterFixtureTests`, `DesignRuleCheckTests`, `DesignRuleGoldenTests`.
- Coverage check: the residual, ineligible and "no routed copper" branches each have a test that fails if removed.
- Runtime proof: plan-38-K drives a group through the real app; here the fixture run.
- Doc sync: `docs/file-format.md` is unchanged (no format change); the Length Match DRC line is described in `docs/routing.md` (plan-38-M).
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions (owner: length matching by meander tuning, 2026-11-13). Author decisions: longest net is the target; length counts trace copper only; two-pin nets only; tune only router-made copper; aim at the middle of the tolerance band; accordion on straight hosts with chamfered corners.

## Corrections

- Phase 2's chamfer loss was stated as `2 × cut × (2 − √2)` per corner. A 90°
  corner cut by `cut` along each of its two sides replaces `2 × cut` of path
  with the diagonal `cut × √2`, a loss of `cut × (2 − √2)`; the test checks
  that hand-computed value against the measured polyline length.
- Phase 3's "tolerance 5 → the 24 mm net gets no bump" can't hold: 30 − 24 =
  6 mm is outside a 5 mm tolerance, and the engine tuned it, rightly. The test
  uses 7 mm: 24 (6 short) is left as routed, 20 (10 short) is tuned.
- Phase 3's synthetic boards are engine problems with no document, so "the
  DRC-clean kinds are 0 after applying" and "engine length equals
  `netLength` of the applied document" are checked there with the verifier
  (held to the DRC by plan-38-H's parity tests) and on the engine's own
  lengths; both document-level checks run on a real board in Phase 4's AFU
  test (`testLengthGroupOnAFU`).

## Summary

The risk is a bump that is illegal; each goes through the verifier before it
lands. Unknown until run: how often a dense board leaves a `residual` — the
fixture run records it. Untouched: pair coupling (J), the UI beyond the member
lengths.
