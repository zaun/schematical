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
| plan-58-G is complete | `ls planning/completed/plan-58-G-* \| wc -l` → 1 | MET (2026-11-24, P-58: `ls planning/completed/plan-58-G-*` → 1, archived 6031215a) |
| A–F are archived | `ls planning/completed/plan-58-[A-F]-* \| wc -l` → 6 | MET (2026-11-24, P-58: `ls planning/completed/plan-58-[A-F]-*` → 6) |

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

- [x] Backend: `cmake -S backend -B /tmp/logic-p58 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8` → `100% tests passed, 0 tests failed out of 62` (est. 15 min).
- [x] Generator idempotence: `python3 -I planning/plan-58-data/gen-parts.py --maker 'C&K' --maker NKK --maker E-Switch --maker CIT --dry-run` → no change (est. 1 min).
- [x] Spec checks: `for m in 'C&K' NKK E-Switch CIT; do python3 -I planning/plan-58-data/census/specs-check.py --maker "$m"; done` → each `specs ok` (est. 1 min).
- [x] Datasheet URLs: `node scripts/check-datasheets.js --host www.littelfuse.com`, `--host www.nkkswitches.com`, `--host www.citrelay.com`, and `--host <h>` for each host of the E-Switch URLs in `Parts/datasheets.json` → each exits 0 (est. 10 min).
- [x] App tests: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p58-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SlideSwitchTests -only-testing:schematicalTests/TactileSwitchTests -only-testing:schematicalTests/PlaySwitchResetTests -only-testing:schematicalTests/PowerSwitchTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/ReferencePrefixTests` → `** TEST SUCCEEDED **` (est. 30 min: these are the plan-51-J landing classes plus G's; no smaller run covers the catalog, file and export paths a new switch kind touches).
- [x] Harness: `rm -rf /tmp/p58-h && scripts/harness/build.sh scripts/harness/slide-switch.swift /tmp/p58-h && /tmp/p58-h/H.app/Contents/MacOS/harness /tmp` → every line `ok`, exit 0; the symbol and sidebar images read (est. 6 min).
- [x] Docs: `grep -c 'slide' docs/parts-format.md docs/file-format.md docs/agent-control.md docs/backend.md` → each ≥ 1.
- [x] Record each result next to its line.

Acceptance: every line above ticked with its result.
  Check: the lines above, each its own command (est. 65 min in all; the app tests dominate).
  Measured: main had advanced 60 commits, so `git merge main` was done first (merge d55b515a: goldens regenerated, `footprints.json` / docs unioned, three Swift conflicts resolved) and every gate ran on the merged tree. Backend: `ctest -N | tail -1` → `Total Tests: 66`, `ctest -j8` → `100% tests passed, 0 tests failed out of 66`. Goldens: main's lines plus exactly 271 / 271 / 134 added (parts / traces / footprints), 0 removed; `grep -c 'states=' parts.txt` → 271 = `awk '$18=="add"' candidates.tsv | wc -l` → 271; slide devices `grep -c '"id": *"slide_'` → 34. Generator: `gen-parts.py --maker 'C&K' --maker NKK --maker E-Switch --maker CIT --dry-run` → `no changes`. Specs: `specs ok (105 rows, 58 specs)`, `(67 rows, 29 specs)`, `(56 rows, 33 specs)`, `(43 rows, 15 specs)`. Datasheets: `www.littelfuse.com` 193 of 193, `www.nkkswitches.com` 12 of 12, `www.citrelay.com` 12 of 12, `configured-product-images.s3.amazonaws.com` 69 of 69 URLs serve a PDF, 0 failed. App tests: the plan's 16 classes → `** TEST SUCCEEDED **`, 233 tests, 0 failures. Harness: `slide-switch.swift` → 8 `ok` lines, `all checks ok`, exit 0; `/tmp/p58g-gallery-1.png` and `-2.png` read: every circuit and position draws as intended (SPST on/off; SPDT 1/2 and 1/2/3; DPST on/off; DPDT 1/2 and 1/2/3, knob and blade agree). Docs: `grep -c slide` → parts-format.md 7, file-format.md 1, agent-control.md 1, backend.md 5.
Commit: 9dc8e4ea

### Phase 2 — close the plan

- [x] Tick `- [x] Add Slide Switches, top 3 manufactuerers and their active switches` in `planning/todo.md`, adding "(plan-58: C&K, NKK, CIT, E-Switch)".
- [x] Archive H to `planning/completed/` (`planning/plan-58-data` stays in place, as the other plans' `-data` folders do).
- [x] Fill the Commit lines of A–H.

Acceptance: A–H are under `planning/completed/` and only `plan-58-data` is left in `planning/`.
  Check: `ls planning | grep -c '^plan-58-[A-H]'` → 0 and `ls planning/completed | grep -c '^plan-58-[A-H]'` → 8 (est. 1 min).
  Measured: `ls planning | grep -c '^plan-58-[A-H]'` → 0 after the archive move, `ls planning/completed | grep -c '^plan-58-[A-H]'` → 8; every letter A-G carries its phase hashes (`grep -c '^Commit: —' planning/completed/plan-58-[A-G]-*` → 0 each); `planning/todo.md` item ticked in the main checkout after the merge (Corrections).
Commit: 4c694ada

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

- **2026-11-24, Phase 1: `ctest` runs 66 programs, not 62.** plan-58-B counted 62 (61 + `slide_test`); main's plans landed while plan-58 ran (plan-47 and others: `relay_test` and three more) and the merge brought them in, so the landing tier ran on 66 with `100% tests passed, 0 tests failed out of 66`. The slide-switch counts do not depend on it: 271 parts, 134 footprints, 34 devices.
- **2026-11-24, Phase 1: `mm.digikey.com` serves 403 to `curl`** (`check-datasheets.js --host mm.digikey.com` → `0 of 17 URLs serve a PDF; 17 failed`: every DigiKey-hosted datasheet of the library, the one slide switch URL `ss-12d07-vg_ga_pa` among them; Cloudflare, as in plan-58-A § 2). The plan's host list excludes it; that PDF was fetched through Safari (A Phase 2) and is placed under `datasheets/switch/slide/ck/`. Not a defect of plan-58.
- **2026-11-24, Phase 2: `planning/todo.md` is ticked in the main checkout, not on `worktree-P-58`.** The main checkout's `todo.md` carries the owner's uncommitted edits, so a commit on the branch would conflict with them at the merge and a commit on main would sweep them in: the `- [x]` and the "(plan-58: C&K, NKK, CIT, E-Switch)" note are edited in place there after the merge and left uncommitted.

## Summary

H adds no risk of its own; it proves A–G together once. A failing gate is
fixed in the letter that owns the failing file and recorded in that letter's
Corrections.
