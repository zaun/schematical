# plan-65-C: Pour Fill and Design Rule Check Read the Board Edge

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-B (complete)

Cutouts and a free-form outline exist in the document and the file (A, B) but nothing reads the cutouts. This sub-plan makes the two readers a person sees first read
`BoardEdge`: the copper pour fill (it stays out of a cutout, with the board-edge clearance) and the design rule check (copper must clear a cutout like the board edge, a
cutout that crosses the edge or another cutout is a problem, and a body may overhang a cutout).

The single outcome: with a cutout on the board, a pour's fill has a hole round it, a pad or trace in or near one is reported by `run_drc` with its own message, a part's body over one is not reported, and a document without cutouts gets
exactly the fills and the issue list it gets today.

References:

- plan-65-A § 3 (`BoardEdge`, the union rule, where the risk concentrates), § 4.2
- `schematical/schematical/Canvas/PourFill.swift` (`PourFiller.fill` at `:98`), `Canvas/PourConnectivity.swift` (`pourFills(for:plain:)`, `computePourFills`), `Canvas/SchematicModel.swift` (`keptPourFills` at `:1639`)
- `DesignRuleCheck/DesignRuleCheck.swift`: `outline`/`outlinePoints` (`:59`, `:88`), `checkPlacement` (`:333`), `rectIsOnBoard` (`:365`), `rectTouchesBoard` (`:374`), `inside` (`:380`), `checkEdge` (`:534`), `Kind` (`:19`)
- `docs/routing.md`, `docs/file-format.md` § Pours (`:281`)
- `schematicalTests/PourFillTests.swift`, `PourDesignRuleTests.swift`, `DesignRuleCheckTests.swift`, `DesignRuleGoldenTests.swift`, `PourConnectivityTests.swift`

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-B is complete | `ls planning/completed \| grep -c '^plan-65-B'` → 1 | NOT MET (2026-11-25: B is written, not landed) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `PourFiller.fill` takes a `BoardEdge`; a pour's region is its rectangle inside the outline pulled back by the copper-to-edge, less every cutout grown by the copper-to-edge.
- The pour fill cache is invalidated by a change of the edge (a cutout added, moved, resized or deleted; a free-form outline set).
- The DRC's edge check measures copper against every loop of the edge; copper whose centre is in a cutout reads `Copper is in a cutout`; copper off the outline still reads `Copper is off the board`.
- The DRC's placement check is unchanged in effect: a body outline over a cutout is not an issue.
- Two new `.edge` issues: `Cutout N crosses the board edge` and `Cutouts N and M overlap`, each selecting the cutouts.

### Non-goals (explicit constraints)

- No router change (D); no drawing, tool or sidebar (E, F); no export (G).
- No change for a document without cutouts: the same fills and the same issue list, byte for byte (`DesignRuleGoldenTests`, `PourFillTests` pass unedited).
- No new DRC category or `Kind` (the two issues are `.edge`), and no new design rule number (a cutout uses `rules.minCopperToEdge`, the board's own).
- No check that a cutout is big enough to mill (no such rule exists in `DesignRules`).

## 2. Current State

- `PourFiller.fill(pours:nets:keepouts:copper:outline:edge:rules:)` builds `board` from `outline`, strokes it `2 * edge` wide and subtracts the stroke to get `inner`, and intersects each pour's rectangle with it (`PourFill.swift:98`–`:112`, `:122`).
- `computePourFills` passes `board.outline` and `board.rules.minCopperToEdge` (`computePourFills`); `pourFills(for:plain:)` reuses `keptPourFills` while `$0.pours == pours && $0.keepouts == keepouts` — it does not look at the outline.
- `DesignRuleChecker` keeps `outlinePoints = board.outline` and `outline: [(CGPoint, CGPoint)]` (`:88`–`:89`). `inside(_:)` (`:380`) is an even-odd test on `outline`; `checkPlacement` and `checkEdge` both use it, through `rectIsOnBoard` (`:365`, which also tests a notch cutting into the rectangle: `!outline.contains { segment(…, touches: inner) }`) and directly (`:549`).
- `checkEdge` (`:534`): for each copper shape, `gap` = the smallest `shape.gap(to:)` over the outline's segments; `off = !inside(centre)`; it keeps the worst per owner and reports `Copper is off the board` or `Copper X from the board edge; min Y` (`:545`–`:566`).

### Measured populations

| What | Count | Command |
|---|---|---|
| lines in the DRC that use `outline`, `outlinePoints`, `inside(` (the sites § 3 rewrites; the compiler finds the same set) | 16 | `rg -n "outlinePoints\|\boutline\b\|inside\(" schematical/schematical/DesignRuleCheck/DesignRuleCheck.swift \| wc -l` → 16 |
| callers of `PourFiller.fill(` | 3: `PourConnectivity.swift:62` and `PourFillTests.swift:37`, `:245` | `rg -n "PourFiller\.fill\(" schematical --glob '*.swift'` → those 3 (the other 11 matches of `PourFiller.fill` are `PourFiller.fillCount`) |
| tests that call the filler or the checker | `PourFillTests`, `PourConnectivityTests`, `PourDesignRuleTests`, `DesignRuleCheckTests`, `DesignRuleGoldenTests` | `ls schematical/schematicalTests \| grep -E "^(PourFillTests\|PourConnectivityTests\|PourDesignRuleTests\|DesignRuleCheckTests\|DesignRuleGoldenTests)"` → all five |

### Verified properties

- `CGPath.subtracting`/`union`/`componentsSeparated` are exact on shapes like these (`planning/plan-52-data/README.md`: `mismatch 0 of 200000`), and `PourFiller.grown` (the shape plus its round-join stroke) is how every other obstacle is grown, so a cutout is cut out as `grown(cutoutPath, by: edge)`, the same code path as a non-plated hole (`PourFiller.fill`: `if item.isHole { cuts.append(grown(item.shape, by: edge)) }`).
- KiCad does the same on its side: a zone over the whole outline stays out of an inner Edge.Cuts loop and keeps 0.5 mm from its edge (`planning/plan-65-data/README.md`, `probe_zone_edge_cuts.py`).

## 3. Design Overview

**Pour fill.** `PourFiller.fill(pours:nets:keepouts:copper:board:edge:rules:)` — `board: BoardEdge` replaces `outline: [CGPoint]`. Where it built one polygon path it builds the outline path, pulls it back by `edge`
(as now), and subtracts `grown(path(of: each cutout polygon), by: edge)` once: `inner = pulledBack.subtracting(combine(grownCutouts))`. A cutout that crosses the outline or overlaps another still gives a defined fill (the union).

**Cache.** `keptPourFills` becomes `(revision, pours, keepouts, edge: BoardEdge, fills)`; `same` also requires `$0.edge == boardEdge`. This is the correctness risk of the fill: without it a moved cutout leaves the old fill on screen and in the Gerber.

**DRC.** The checker holds `edge: BoardEdge` and, for placement only, `outline` segments of the board outline alone (`insideOutline`, the old `inside`). The two jobs:

- *Placement* (bodies and texts): `rectIsOnBoard`/`rectTouchesBoard` use the outline alone, so a body over a cutout is fine (the owner's decision). A pad in a cutout is copper, so the edge check finds it.
- *Edge* (copper): `edges` are `edge.segments` (outline and every cutout); `off = !edge.contains(centre)`. The message is `Copper is in a cutout` when the centre is inside the outline and inside a cutout, else `Copper is off the board`.
- *Speed*: `edges.map { shape.gap(to: $0) }.min()` is shapes × segments, and a 3.2 mm round cutout is 29 segments (A § 2). The loop skips a segment whose bounds are farther from the shape's bounds than `rules.minCopperToEdge` (only gaps under the minimum, and the `off` state, are ever reported, so the result is unchanged).
- *New checks* (`checkCutouts()`, after `checkEdge`): for cutout i (1-based in drawing order), `Cutout i crosses the board edge` when one of its points is not inside the outline or any of its segments crosses an outline segment (`CopperGeometry.segmentsCross`); `Cutouts i and j overlap` for i < j when a segment of one crosses a segment of the other or one's first point is inside the other. Reference `Cutout i`, kind `.edge`, items `[.cutout(id)]` (one cutout) or both ids.

**Byte-identity is the gate for the no-cutout case** (the goldens), and the change is behaviour for the cutout case (new tests). A diff in a golden is a bug in the new code to be root-caused with one fixture.

Rejected: a new `Kind.cutout` (it would renumber every golden's group order for a document that has none; `.edge` is the right group and a person reads the message); rounding `inside` to even-odd over all loops (it would call a point in two overlapping cutouts material).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — The pour fill

- [ ] `Canvas/PourFill.swift`: `fill(…board: BoardEdge, edge:…)`; the `inner` of § 3; the guard is `board.outline.count >= 3`.
- [ ] `Canvas/PourConnectivity.swift`: `computePourFills` passes `boardEdge`; `keptPourFills` (in `SchematicModel.swift:1639`) and `pourFills(for:plain:)` carry and compare `edge`.
- [ ] Update every caller of `PourFiller.fill` (the document; the tests that call it directly, with `BoardEdge(outline:cutouts: [])`).
- [ ] Tests `schematicalTests/CutoutPourTests.swift`: a 40 × 40 pour on a 60 × 40 board with a 10 × 10 cutout inside it has no fill point inside the cutout or within `minCopperToEdge` of its edge (sample 2000 points against `BoardEdge.contains` and the segment distance) and has fill 5 mm from it; a round 6 mm cutout likewise; a cutout that crosses the outline still gives a fill; the cache: with `PourFiller.fillCount` set to 0, `connectivity()` twice with no change fills once (`fillCount == 1`), and after `moveCutout` a third call fills again (`fillCount == 2`) and its fill has the hole at the new place and none at the old one (the pattern of `PourConnectivityTests.swift:113`–`:124`); a free-form L-shaped outline keeps the fill out of the notch.

Acceptance: the fill has a clean hole round a cutout, follows a moved one, and the existing fill tests pass unedited.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutPourTests -only-testing:schematicalTests/PourFillTests -only-testing:schematicalTests/PourConnectivityTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min; the two existing classes are the no-cutout byte-identity check for the fill).
Commit: —

### Phase 2 — The design rule check

- [ ] `DesignRuleCheck/DesignRuleCheck.swift`: hold `edge: BoardEdge` and the outline-only segments; rename `inside` to `insideOutline` for placement; `checkEdge` uses `edge.segments`, `edge.contains`, the `Copper is in a cutout` message and the bounds prefilter of § 3; add `checkCutouts()` and call it after `checkEdge()` in `run()`.
- [ ] Tests `schematicalTests/CutoutDesignRuleTests.swift`: a pad in a cutout → `Copper is in a cutout`; a trace 0.1 mm from a cutout's edge with `minCopperToEdge` 0.3 → `Copper 0.1 mm from the board edge; min 0.3 mm`; the same trace 0.5 mm away → no issue; a body (an `R` footprint with no pad near it) over a cutout → no issue; a pad off the outline → `Copper is off the board` (unchanged text); a cutout with a corner outside the outline → `Cutout 1 crosses the board edge` selecting `.cutout(id)`; two overlapping cutouts → `Cutouts 1 and 2 overlap`; two apart → none; no cutouts → `designRuleCheck` equals the result with `cutouts` never set (empty).
- [ ] The edge-check speed: a test that builds a board with 40 round cutouts (29 segments each) and 400 pads, runs `designRuleCheck` once and prints the elapsed seconds; the phase is not done while it is over 10 s in the Debug test build (record the observed time in Corrections).

Acceptance: the cutout cases above give exactly those messages and the existing DRC tests and goldens pass unedited.
  Check: `XB -only-testing:schematicalTests/CutoutDesignRuleTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/DesignRuleGoldenTests -only-testing:schematicalTests/PourDesignRuleTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min; `DesignRuleGoldenTests` is the no-cutout byte-identity check, and nothing smaller covers it).
Commit: —

## Validation Plan

- Tests: `CutoutPourTests`, `CutoutDesignRuleTests` (negative cases included: a cutout that is fine, a body over one).
- Coverage check: the filler and the checker are exercised by the existing classes named in each check, so an edit that breaks the no-cutout path fails there.
- Runtime proof: none beyond the tests; the real window shows it after E (J drives it).
- Doc sync: `docs/file-format.md` § Pours (the fill also stays out of cutouts; `:281`–`:300`) and § Cutouts (the DRC messages); `docs/routing.md` is D's.
- Final gate (run ONCE, after Phase 2): the union of both phases' test classes (est. 6 min).

## Decisions

- Copper, not bodies, must clear a cutout; a body may overhang one — decided by the owner, 2026-11-25. Settles the placement/edge split in § 3.
- Cutout problems are `.edge` issues named by index — decided by the author, 2026-11-25. Settles § 3 (rejected `Kind.cutout` above).

## Corrections

<Filled in during execution.>

## Summary

Risk: the fill cache (a stale fill after a cutout edit) and the DRC's two uses of one `inside`. Left untouched: the router (D), every export (G), the board's design rule values.
