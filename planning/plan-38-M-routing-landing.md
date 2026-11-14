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

- [ ] `docs/routing.md` (new): what the system is; the Route menu, sheet and Route tool (each option and its default, from `RoutingOptions`); net classes, layer rules and net groups (what each does to the router; how a pair is routed; how length is measured; the `auto` flag); the engine in one page (grid, profiles, A*, negotiation, cleanup, verification, tuning), its guarantees (DRC-clean by verification; deterministic; legal on stop) and its limits (through vias only, two-pin groups, no push-and-shove, kept copper immovable, no impedance); the DRC kinds it adds (`layerRule`, `lengthMatch`, `diffPair`); the fixtures and the recorded benchmark results (copy the tables from plan-38-E/F/H/I/J); code map (`Routing/*.swift`, tests).
- [ ] `docs/file-format.md`: check that the sections written by plan-38-A and B (`netClasses`, `netLayerRules`, `netGroups`, `auto`) are present and link to `docs/routing.md`; the top "version" text still says 4/5 (nothing changed).
- [ ] `docs/export.md`: no change (no export impact) — check by `git diff --stat <first plan-38 commit>^..HEAD -- docs/export.md` → empty.

Acceptance: `docs/routing.md` exists and names every `RoutingOptions` field; links resolve.
  Check: `grep -c "" docs/routing.md` → more than 0, and for each field name in `RoutingOptions` (`grep -o "^    var [a-zA-Z]*" schematical/schematical/Routing/RoutingOptions.swift | awk '{print $2}'`) `grep -c <field> docs/routing.md` → ≥ 1 (est. 2 min).
Commit: —

### Phase 2 — the landing tier (run once)

- [ ] Neutrality: `git diff --stat <first plan-38 commit>^..HEAD -- backend datasheets schematical/schematical/Parts` → empty (est. 1 min).
- [ ] Goldens: `DesignRuleGoldenTests` (the three `Fixtures/drc-*.txt` unchanged since plan-38-D: `git diff <plan-38-D Phase 1 commit>..HEAD -- schematical/schematicalTests/Fixtures/drc-AFU.txt schematical/schematicalTests/Fixtures/drc-LFU.txt schematical/schematicalTests/Fixtures/drc-RegFile.txt` → empty; est. 1 min).
- [ ] App, full: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` and `Executed N tests, with 0 failures` with N ≥ 668 + the plan's new tests (est. 15–20 min; nothing smaller checks that no existing PCB, file or undo test broke across 12 letters).
- [ ] Router benchmark, once, from the real test run: AFU `.balanced` 115/115 and the twelve DRC kinds 0; LFU ≥ 127/188; RegFile ≥ 12/24 (the floors of plan-38-F, H) — copy the actual numbers into `docs/routing.md` and § Results below.
- [ ] Real app, once, on this commit (per `AGENTS.md`, pointer left at 0,0 afterwards): (1) AFU fixture → Route ▸ Autoroute… (Balanced) → completes; DRC window: no `Unrouted`, no router-caused issue. (2) Undo → the board is as opened; Redo → copper back. (3) Net Classes: Add Power Class → run again with Clear autorouted → power nets are 20 mil. (4) A layer rule: set a net to Only Top → run → no Bottom copper on it; set another to Prefer Bottom → it takes Bottom. (5) A length group of 3 two-pin nets (tolerance 0.5) → run → the sidebar shows the spread within tolerance (or the reported residual). (6) A pair → run → the two traces run side by side; DRC has no `diffPair` line. (7) LFU → run → the result list names each unrouted net; Stop mid-run keeps copper. (8) Route tool: click a pad → its net routes; click a ratsnest line → one connection. (9) Trace tool with Assist: walks around a pad; V drops a via. (10) Unroute Selected on a part, Unroute All Autorouted. Record the observation per item.
- [ ] Results (the output of this tier, filled in then): suite __ tests, 0 failures; AFU __/115; LFU __/188; RegFile __/24; real-app items 1–10: __.

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

(Filled in during execution.)

## Summary

The tier checks the whole against the owner's scope: classes, layer rules, groups,
pairs, clear and unroute options, undo, and a DRC-clean result on the fixtures.
Left untouched: push-and-shove, teardrops, copper pours, saving routed copies of
the example boards (the todo's "Route the AFU/LFU PCB" lines).
