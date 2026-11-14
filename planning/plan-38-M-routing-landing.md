# plan-38-M: Routing — landing tier, docs, todo ticks, archive

Last updated: 2026-11-13
Effort: small (< 1h of work; the gates themselves run ≈ 40 min)
Depends on: plan-38-L

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F, G, H, I, J, K, L) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The one place the whole of plan-38 is checked together, once: the full app test
suite, an end-to-end run of the real app over the three fixtures with every
feature (classes, layer rules, a length group, a differential pair, a clear
option, unroute, Undo), the proof that nothing outside the PCB code changed
behavior, the router's documentation, and the todo and archive bookkeeping.

References:

- `AGENTS.md`; plan-38-A § Decisions (the owner's scope); the results recorded in
  plan-38-E/F/H/I/J Phase "fixtures"; `planning/completed/plan-36-I-mcu-landing.md`
  (the form of a landing letter); `docs/export.md` (the form of a feature doc).

## 1. Goal

- On one commit: the full app suite passes; the DRC goldens of the three fixtures
  are byte-identical to plan-38-D's; every feature of the plan is observed working in
  the real app; `docs/routing.md` describes the system; the todo is ticked; plan-38
  is archived.

### Non-goals (explicit constraints)

- No code changes except fixes the gates force (each recorded in § Corrections).
- No backend, parts-library or datasheet changes: `git diff --stat <plan-38 first
  commit>^..HEAD -- backend datasheets schematical/schematical/Parts` is empty.
- The full test tier of the project's policy (sweeps, corpus) is not run here.

## 2. Current State

- plan-38-A … L are complete; each recorded its own scoped checks.
- Landing form precedent: plan-36-I (five gates, real app once, close phase).
- The app test suite has 668 `func test` methods before plan-38
  (`grep -rc "func test" schematical/schematicalTests | awk -F: '{s+=$2} END {print s}'`
  → 668); plan-36-I's full run took est. 15 min for 652.

### Measured populations

| What | Count | Command |
|---|---|---|
| Test methods before plan-38 | 668 | the command above, run on cb6259f |
| Plan files | 13 (A–M) | `ls planning/plan-38-*.md \| wc -l` → 13 |
| Todo lines this plan resolves | 2 of the PCB section (per-net trace widths; autorouter) | `grep -n "Per-net trace widths\|simple autorouter" planning/todo.md` |

### Verified properties

- `docs/` holds one document per subsystem (`backend.md`, `boards-format.md`,
  `export.md`, `file-format.md`, `parts-format.md`), so routing gets
  `docs/routing.md` (not appended to `file-format.md`, which stays the format
  reference and only gains the sections plan-38-A/B wrote).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — documentation

- [x] `docs/routing.md` (new): what the system is; the Route menu, sheet and Route tool (each option and its default, from `RoutingOptions`); net classes, layer rules and net groups (what each does to the router; how a pair is routed; how length is measured; the `auto` flag); the engine in one page (grid, profiles, A*, negotiation, cleanup, verification, tuning), its guarantees (DRC-clean by verification; deterministic; legal on stop) and its limits (through vias only, two-pin groups, no push-and-shove, kept copper immovable, no impedance); the DRC kinds it adds (`layerRule`, `lengthMatch`, `diffPair`); the fixtures and the recorded benchmark results (copy the tables from plan-38-E/F/H/I/J); code map (`Routing/*.swift`, tests). (Also the Trace assist and V; the pair-breakout limit plan-38-J recorded; the Results table gains M's numbers in Phase 2.)
- [x] `docs/file-format.md`: check that the sections written by plan-38-A and B (`netClasses`, `netLayerRules`, `netGroups`, `auto`) are present and link to `docs/routing.md`; the top "version" text still says 4/5 (nothing changed). — all four present (`grep -n "netClasses\|netLayerRules\|netGroups\|auto"`: lines 166–168, 197, 220); they had no link, so § Net classes now ends with one to `routing.md#rules-per-net`; the top still says version 5 = 4 plus custom parts.
- [x] `docs/export.md`: no change (no export impact) — check by `git diff --stat <first plan-38 commit>^..HEAD -- docs/export.md` → empty. — `git diff --stat 2ebd69d0^..HEAD -- docs/export.md` → empty.

Acceptance: `docs/routing.md` exists and names every `RoutingOptions` field; links resolve.
  Check: `grep -c "" docs/routing.md` → more than 0, and for each field name in `RoutingOptions` (`grep -o "^    var [a-zA-Z]*" schematical/schematical/Routing/RoutingOptions.swift | awk '{print $2}'`) `grep -c <field> docs/routing.md` → ≥ 1 (est. 2 min). — 233 lines; scope 5, include 1, existing 1, layers 5, vias 10, angle 1, strategy 2, grid 5, widthOverride 1, clearanceOverride 1, fanoutPlanes 1, tuneGroups 1, routePairs 1, seed 3, timeLimit 1.
Commit: —

### Phase 2 — the landing tier (run once)

- [x] Neutrality: `git diff --stat <first plan-38 commit>^..HEAD -- backend datasheets schematical/schematical/Parts` → empty (est. 1 min). — `git diff --stat 2ebd69d0^..HEAD -- backend datasheets schematical/schematical/Parts` → empty (also after merging main, 095ce688: main's own changes there are main's, not this plan's — the check ran before the merge, on 553364c2).
- [x] Goldens: `DesignRuleGoldenTests` (the three `Fixtures/drc-*.txt` unchanged since plan-38-D: `git diff <plan-38-D Phase 1 commit>..HEAD -- schematical/schematicalTests/Fixtures/drc-AFU.txt schematical/schematicalTests/Fixtures/drc-LFU.txt schematical/schematicalTests/Fixtures/drc-RegFile.txt` → empty; est. 1 min). — `git diff --stat aec77008..HEAD -- …drc-AFU.txt …drc-LFU.txt …drc-RegFile.txt` → empty; `DesignRuleGoldenTests` passed after the merge with main (with `DesignRuleCheckTests` and `KeyboardHelpTests`: 22 tests, 0 failures).
- [x] App, full: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` and `Executed N tests, with 0 failures` with N ≥ 668 + the plan's new tests (est. 15–20 min; nothing smaller checks that no existing PCB, file or undo test broke across 12 letters). — on the merge commit 095ce688: `Executed 946 tests, with 0 failures (0 unexpected) in 2623.071 seconds`, `** TEST SUCCEEDED **` (946 ≥ 668 + this plan's; no crash, no restart).
- [x] Router benchmark, once, from the real test run: AFU `.balanced` 115/115 and the twelve DRC kinds 0; LFU ≥ 127/188; RegFile ≥ 12/24 (the floors of plan-38-F, H) — copy the actual numbers into `docs/routing.md` and § Results below. — from that run (`RouterFixtureTests`, which also asserts the router-caused DRC kinds are 0): Balanced AFU 115/115, 115 vias, 1582 mm, 27.99 s (2 rounds, 113 rips); LFU 147/188, 246 vias, 2528 mm, 456.44 s (12 rounds) ≥ 127; RegFile 24/24, 31 vias, 314 mm, 29.77 s ≥ 12. Fast: AFU 114, LFU 149, RegFile 24. Thorough: AFU 115, LFU 147, RegFile 24. In `docs/routing.md` § Results.
- [x] Real app, once, on this commit (per `AGENTS.md`, pointer left at 0,0 afterwards): (1) AFU fixture → Route ▸ Autoroute… (Balanced) → completes; DRC window: no `Unrouted`, no router-caused issue. (2) Undo → the board is as opened; Redo → copper back. (3) Net Classes: Add Power Class → run again with Clear autorouted → power nets are 20 mil. (4) A layer rule: set a net to Only Top → run → no Bottom copper on it; set another to Prefer Bottom → it takes Bottom. (5) A length group of 3 two-pin nets (tolerance 0.5) → run → the sidebar shows the spread within tolerance (or the reported residual). (6) A pair → run → the two traces run side by side; DRC has no `diffPair` line. (7) LFU → run → the result list names each unrouted net; Stop mid-run keeps copper. (8) Route tool: click a pad → its net routes; click a ratsnest line → one connection. (9) Trace tool with Assist: walks around a pad; V drops a via. (10) Unroute Selected on a part, Unroute All Autorouted. Record the observation per item. — 2026-11-14 10:46–11:06, the Debug build of the merge commit 095ce688 (built by the full-suite run), run from its binary; pointer to 0,0 after; fixtures unchanged:
  1. AFU, Balanced: "Routed 115 of 115 connections, 115 vias, 1582 mm, 32.4 s"; DRC: 16 issues, all silkscreen (the fixture's own), no `Unrouted`, nothing router-caused.
  2. ⌘Z: the board as opened (ratsnest only); ⇧⌘Z: the copper back.
  3. Add Power Class → "Power · 20 mil · 2 nets" (5V, GND); Clear autorouted ("Removes 231 traces and 115 vias") → 113 of 115, 143 vias, 76.1 s (B3, XOR2.11 "no legal way found": the 20 mil power nets take more room); a GND trace: Width 20 mil, Net GND, Class Power.
  4. A0 Only Top, A1 Prefer Bottom → 112 of 115, 144 vias, 78.5 s; highlighted, A0's copper is all Top (red, no via); A1 runs mostly on Bottom.
  5. AFU with a "Lengths" group of ADDR2.7, XOR1.3, XOR1.6 (tolerance 0.5; a board made by a throwaway test) → 115 of 115, "Lengths: 14.86 mm apart", the residual reported (AFU's density, as plan-38-I found).
  6. The adjacent-header pair board (plan-38-K's) → "Pair: coupled, 4.09 mm apart", 0 vias; DRC: only the two headers' Reference-over-pad lines, no `diffPair`.
  7. LFU, Stop after 60 s → "Routed 111 of 188 connections (stopped), 134 vias, 1500 mm, 61.0 s"; the copper stayed; the list names each unrouted net once (/GT 1, /LFU 3, 5V 3, A0 4, …), each "run stopped first".
  8. Route tool: click C8.1 → its net only: "Routed 20 of 20 connections, 1 via, 110 mm, 0.4 s" (5V, 20 mil); click the AND1–NOR1 unrouted line → "Routed 1 of 1 connections, 0 vias, 33 mm, 0.2 s", the trace's Connects AND1.1, NOR1.1. (The sheet's Nets picker shows blank for these scopes: it has rows only for All and Selection. Cosmetic.)
  9. Trace with Assist: V in open board → a Top trace to a via (0.4 / 0.8) and the layer menu at Bottom; a release by the board edge → "No legal route there.", nothing added. The walk-around around pads: plan-38-L Phase 4.
  10. ⌥⌘U with an A0 trace selected → A0's copper gone, its unrouted line back (a click on a part's body over copper selects the copper, which is drawn over the footprint, so the selection was a trace of the net rather than the part; the part case is now `RouteToolTests.testUnrouteSelectedOnAPart`, added here); Route ▸ Unroute All Autorouted → the board bare, ratsnest only.
- [x] Results (the output of this tier, filled in then): suite 946 tests, 0 failures; AFU 115/115; LFU 147/188; RegFile 24/24; real-app items 1–10: all behaved as stated (observations above; item 10 by a trace's net, the part case added as `RouteToolTests.testUnrouteSelectedOnAPart`, `-only-testing:RouteToolTests` → 6 tests, 0 failures).

Acceptance: every line above passes on one commit.
  Check: the lines above, with their results recorded here and the commit hash (est. 40 min in total).
Commit: —

### Phase 3 — close

- [ ] `planning/todo.md`: in § PCB, tick "Per-net trace widths (net classes): power nets wider by default." and add `(plan-38)`; in "Later: teardrops, a simple autorouter, push-and-shove routing." remove "a simple autorouter," and append "(autorouter: plan-38)"; leave teardrops and push-and-shove. The owner's working copy may hold other edits to this file, so commit only these hunks, without an interactive add: `git show HEAD:planning/todo.md > /tmp/todo.head`, copy it to `/tmp/todo.new`, make the two edits in the copy, `git diff --no-index /tmp/todo.head /tmp/todo.new` rewritten with the paths `a/planning/todo.md` and `b/planning/todo.md` into `/tmp/todo.patch`, then `git apply --cached /tmp/todo.patch` and `git commit -m "plan-38: todo ticked"` with no path argument, so only the index is committed and the working-tree edits of others stay unstaged. The "Route the AFU PCB" and "Route the LFU PCB" lines stay unticked: they are about saving routed example files, which this plan does not do.
- [ ] Move `planning/plan-38-*.md` to `planning/completed/` (one commit); `planning/plan-38-data/` stays where it is, as `plan-36-data` does (the spike and fixtures are cited from `docs/routing.md`).

Acceptance: `grep -c "plan-38" planning/todo.md` → 2 or more; `ls planning/plan-38-*.md 2>/dev/null | wc -l` → 0 and `ls planning/completed/plan-38-*.md | wc -l` → 13 (est. 2 min).
Commit: —

## Validation Plan

- Tests: the full app suite (Phase 2).
- Coverage check: Phase 2's benchmark line fails if any engine stage is skipped (the DRC-clean guarantee needs cleanup and verification; the 115/115 needs negotiation).
- Runtime proof: Phase 2's real-app items 1–10.
- Doc sync: `docs/routing.md`, `docs/file-format.md` (Phase 1).
- Final gate: this letter.

## Decisions

See plan-38-A § Decisions. Author decisions: no backend/ctest in this tier (nothing in `backend/` changes, shown by the diff check); the full app suite runs once here and nowhere else; `plan-38-data/` is kept for the docs to cite.

## Corrections

- **Main moved 111 commits during the plan**, so the tier ran after merging
  it (095ce688). Two conflicts:
  - `DesignRuleCheck.swift`: main's reference rectangle (plan-41) with
    plan-38-D's renamed `CopperShape`.
  - `KeyboardHelpTests.swift`: both key lists kept.
- **Unroute Selected on a part couldn't be clicked in the real app.**
  Clicking a part's body on a routed board selects the copper over it (it
  is drawn over the footprint), so item 10 used a trace's net. No test
  covered the part selection, so `testUnrouteSelectedOnAPart` was added.
- **The owner's review of item 1: too many vias.** AFU gets one via per
  connection (115 for 115). The owner (2026-11-14) prefers slower, cleaner
  routing to fast and messy, and asked for it after the plan lands; not
  this plan's scope.

## Summary

The tier checks the whole against the owner's scope: classes, layer rules, groups,
pairs, clear and unroute options, undo, and a DRC-clean result on the fixtures.
Left untouched: push-and-shove, teardrops, copper pours, saving routed copies of
the example boards (the todo's "Route the AFU/LFU PCB" lines).
