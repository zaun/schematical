# plan-58-H: Slide switches — the landing tier

Last updated: 2026-11-24
Effort: small (< 1h)
Depends on: plan-58-G

H runs the gates of the whole plan once, on the finished tree, records the
numbers, ticks `planning/todo.md` and archives the plan. No design, no new code
beyond a fix for a failure a gate finds (that fix belongs to the letter that owns
it and is recorded in its Corrections).

Outcome: every gate of § Phases passes on the tree after G; plan-58 A–H sit in
`planning/completed/`; `planning/todo.md`'s "Add Slide Switches" item is ticked.

References:

- `planning/plan-58-A-slide-switch-data.md` … `plan-58-G-slide-switch-app.md`
  (each letter's Acceptance and Validation Plan); `AGENTS.md` (§ Datasheets,
  § Testing in the app); `docs/backend.md` (tests, goldens).
- `planning/completed/plan-51-J-*.md` (the precedent landing letter).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-G is complete | `ls planning/completed/plan-58-G-* \| wc -l` → 1 | NOT MET until G lands |
| A–F are archived | `ls planning/completed/plan-58-[A-F]-* \| wc -l` → 6 | NOT MET until they land |

If plan-58-G is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Backend, generator, datasheet URLs, app tests, harness and docs gates pass.
- The counts recorded in this file match the tree: `ctest` programs 62; slide
  parts in `parts.txt` = the sum of the four makers' `add` rows of `candidates.tsv`.

### Non-goals (explicit constraints)

- No new feature, no data change, no golden regeneration unless a gate found a
  defect and its owning letter's Corrections say so.

## 2. Current State

All letters A–G have landed; their Commit lines hold the hashes.

### Measured populations

| What | Count | Command |
|---|---|---|
| ctest programs | 62 | `ctest --test-dir /tmp/logic-p58 -N \| tail -1` → `Total Tests: 62` |
| Slide parts in the golden | the sum of the `add` rows | `grep -c 'states=' backend/tests/golden/parts.txt` vs `awk -F'\t' '$18=="add"' planning/plan-58-data/candidates.tsv \| wc -l` (equal) |
| Slide devices | the generated region's size | `grep -c '"id": *"slide_' schematical/schematical/Parts/switch.json` |

## 3. Design Overview

Nothing to design. The gates are the targeted gates of every area the plan
touched (backend, part data, datasheets, app, harness, docs); the whole-suite
tier is not a plan step (it runs in the background per write-bug's Test tiers).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; fill the `Commit:` line when it lands. An
> unticked box means NOT DONE.

### Phase 1 — gates

- [ ] Backend: `cmake -S backend -B /tmp/logic-p58 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8` → `100% tests passed, 0 tests failed out of 62` (est. 15 min).
- [ ] Generator idempotence: `python3 -I planning/plan-58-data/gen-parts.py --maker 'C&K' --maker NKK --maker E-Switch --maker CIT --dry-run` → no change (est. 1 min).
- [ ] Spec checks: `for m in 'C&K' NKK E-Switch CIT; do python3 -I planning/plan-58-data/census/specs-check.py --maker "$m"; done` → each `specs ok` (est. 1 min).
- [ ] Datasheet URLs: `node scripts/check-datasheets.js --host www.littelfuse.com`, `--host www.nkkswitches.com`, `--host www.citrelay.com`, and `--host <h>` for each host of the E-Switch URLs in `Parts/datasheets.json` → each exits 0 (est. 10 min).
- [ ] App tests: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p58-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SlideSwitchTests -only-testing:schematicalTests/TactileSwitchTests -only-testing:schematicalTests/PlaySwitchResetTests -only-testing:schematicalTests/PowerSwitchTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/ReferencePrefixTests` → `** TEST SUCCEEDED **` (est. 30 min: these are the plan-51-J landing classes plus G's; no smaller run covers the catalog, file and export paths a new switch kind touches).
- [ ] Harness: `rm -rf /tmp/p58-h && scripts/harness/build.sh scripts/harness/slide-switch.swift /tmp/p58-h && /tmp/p58-h/H.app/Contents/MacOS/harness /tmp` → every line `ok`, exit 0; the symbol and sidebar images read (est. 6 min).
- [ ] Docs: `grep -c 'slide' docs/parts-format.md docs/file-format.md docs/agent-control.md docs/backend.md` → each ≥ 1.
- [ ] Record each result next to its line.

Acceptance: every line above ticked with its result.
  Check: the lines above, each its own command (est. 65 min in all; the app tests dominate).
Commit: —

### Phase 2 — close the plan

- [ ] Tick `- [x] Add Slide Switches, top 3 manufactuerers and their active switches` in `planning/todo.md`, adding "(plan-58: C&K, NKK, CIT, E-Switch)".
- [ ] Archive H to `planning/completed/` (`planning/plan-58-data` stays in place, as the other plans' `-data` folders do).
- [ ] Fill the Commit lines of A–H.

Acceptance: A–H are under `planning/completed/` and only `plan-58-data` is left in `planning/`.
  Check: `ls planning | grep -c '^plan-58-[A-H]'` → 0 and `ls planning/completed | grep -c '^plan-58-[A-H]'` → 8 (est. 1 min).
Commit: —

## Validation Plan

- Tests: the gates above.
- Coverage check: the new parts and footprints are inside `listing_check`,
  `trace_check` and `footprint_check` (counts of Phase 1 equal the `add` rows).
- Runtime proof: the harness run (Phase 1) and its images.
- Doc sync: the docs line of Phase 1.
- Final gate: Phase 1, once.

## Decisions

- No decision belongs to H: the scope decisions are in plan-58-A § Decisions.

## Corrections

(Filled in during execution.)

## Summary

H adds no risk of its own; it proves A–G together once. A failing gate is
fixed in the letter that owns the failing file and recorded in that letter's
Corrections.
