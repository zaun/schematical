# plan-43-E: Reference-board parts — docs and landing tier

Last updated: 2026-11-15
Effort: small (< 1h)
Depends on: plan-43-D

E syncs the docs with what B–D added and runs every gate once. The
behavioral outcome: each gate of Phase 2's table shows its expected value in
one run.

References:

- `docs/parts-format.md` (§ `transistor`, § `connector`, § `switch`).
- plan-43-A … D (their § Corrections, for counts that moved).
- `planning/todo.md`.

## Prerequisites

plan-43-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-43-D complete | `ls planning/completed/plan-43-D-*.md` → exists | MET (2026-11-15: archived in 2d05625d) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 2's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area
  (and recorded in that letter's § Corrections), then E re-runs.
- No app code and no app test changes: plan-43 adds parts to existing
  leaves only.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Docs

- [x] `docs/parts-format.md` § `transistor`, the paragraph "The library's
      `transistor.json` (plan-40) holds …": add that plan-43 added parts of
      Alpha & Omega Semiconductor, Diotec, GOODWORK, MCC, Texas Instruments
      and further Diodes Incorporated, Nexperia and Vishay Siliconix parts
      by hand. — done with the makers that landed (§ Corrections).
- [x] § `connector`, the sentence "USB footprints (`USB-SS-…` Same Sky,
      `USB-AMP-…` Amphenol ICC)": add `USB-GCT-…` GCT.
- [x] § `switch`: one sentence after the tactile paragraph naming the
      hand-written DIP devices (`te_…`, `cts_219_08`).

Acceptance: the three edits are in place.
  Check: `grep -c 'USB-GCT' docs/parts-format.md` ≥ 1; `grep -c 'cts_219_08'
  docs/parts-format.md` ≥ 1; `grep -c 'plan-43' docs/parts-format.md` ≥ 1
  (est. 1 min). — 1, 1, 2.
Commit: 2d34dc87

### Phase 2 — Landing tier

- [x] Run every gate of the table on a fresh build directory
      (`/tmp/logic-p43-land`) and record the observed values below it. —
      observed 2026-11-15, below the table.
- [x] `planning/todo.md`: no item is ticked by plan-43 (the item A added
      stays open for the later families). — unchanged since plan-43-A.
- [x] Move `plan-43-A` … `plan-43-E` to `planning/completed/` (A–D as each
      completes; E with this commit). — A c94a9b47, B 937f0bea, C 6f1be566,
      D 2d05625d; E in this commit.

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p43-land && cmake --build /tmp/logic-p43-land -j8 && ctest --test-dir /tmp/logic-p43-land -j8` | `100% tests passed … out of 36`, plus any program another plan added meanwhile |
| Goldens vs plan-43 start | `git diff <plan-43-B first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | 32901 (P0 32854 + 47), less any row A § Corrections dropped, plus any part another plan landed meanwhile |
| The 47 are in | `python3 -I -c "import json,glob,csv; n={p['name'] for f in glob.glob('schematical/schematical/Parts/*.json') for d in json.load(open(f)).get('devices',[]) for p in d['parts']}; print(sum(r['mpn'] in n for r in csv.DictReader(open('planning/plan-43-data/census/fit.tsv'),delimiter='\t')))"` | 47 (the `fit.tsv` row count) |
| Datasheet URLs | `node scripts/check-datasheets.js $(cut -f12 planning/plan-43-data/targets.tsv \| tail -n +2 \| sort -u)` | exits 0 (46 ids) |
| Datasheet files | `python3 -I -c "import csv,os; print(sum(not os.path.exists(r['pdf']) for r in csv.DictReader(open('planning/plan-43-data/targets.tsv'),delimiter='\t')))"` | 0 |
| Skipped parts listed | `grep -c 'Reference boards (plan-43)' datasheets/skipped-parts.md` | 1 |
| Tactile generator idle | `python3 -I planning/plan-27-data/gen-parts.py --dry-run` | `no changes` |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p43-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p43-dd test -only-testing:schematicalTests/DatasheetCatalogTests -only-testing:schematicalTests/PartRegistrationTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |

Observed (2026-11-15, worktree `P-43`, `main` still at 7410acd9):

| Gate | Observed |
|---|---|
| Backend tests | `100% tests passed, 0 tests failed out of 36` (fresh `/tmp/logic-p43-land`, no compiler warning or error line) |
| Goldens vs plan-43 start | `git diff b01bf200^ -- backend/tests/golden \| grep -c '^-[^-]'` → 0 |
| Parts total | 32897 (32854 + 43) |
| The parts are in | 43 of 43 `fit.tsv` rows |
| Datasheet URLs | `42 of 42 URLs serve a PDF; 0 failed.`, exit 0 (ids passed with `xargs`; zsh does not split `$(…)` kept in a variable) |
| Datasheet files | 0 missing |
| Skipped parts listed | 1 |
| Tactile generator idle | `no changes` |
| App | `** BUILD SUCCEEDED **` |
| App tests | `Executed 14 tests, with 0 failures`; `** TEST SUCCEEDED **` |

Acceptance: every gate shows its expected value.
  Check: the table (est. 15 min: the app build and tests are most of it; no
  smaller check loads the bundled part files through the app).
Commit: 46b22fca

## Validation Plan

- Runtime proof: the app tests load the bundled library, so a part file the
  app cannot read fails them.
- Doc sync: Phase 1.
- Final gate: Phase 2's table, once.

## Decisions

- None beyond plan-43-A's.

## Corrections

- **plan-43 added 43 parts, not 47** (plan-43-A … D § Corrections: four
  parts were skipped for their datasheets — `MBT3904DW1T3G`, `2N7002PW`,
  `SI2371EDS-T1-BE3`, `MCP47A1T-A0E/LTY`). The gate table's 47 reads 43
  (the `fit.tsv` row count), 32901 reads 32897, and the datasheet gate has
  42 ids. "The 47" in the Summary is the 43.
- **The transistor paragraph names the makers that landed**: Alpha & Omega
  Semiconductor, GOODWORK, MCC, Texas Instruments, Diodes Incorporated,
  Nexperia, onsemi. The Diotec and Vishay Siliconix parts were skipped.

## Summary

E adds nothing; it proves the 47 parts load in the backend and the app and
that their datasheets answer.
