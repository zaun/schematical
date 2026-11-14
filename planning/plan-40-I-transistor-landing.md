# plan-40-I: Landing tier

Last updated: 2026-11-14
Effort: small (< 1h)
Depends on: plan-40-H

I runs plan-40's landing tier once, on the landed tree, and archives the
plan. Every gate's command is in the table; the observed values are recorded
under it.

References:

- plan-40-A § 1 (the plan's outcome), plan-40-F § 1, plan-40-G § 1,
  plan-40-H § 1.
- `planning/completed/plan-34-H-resistor-landing.md` (the precedent).

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-H complete | `ls planning/completed/plan-40-H-*.md` → exists, or every H box ticked | NOT MET (H not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and
  recorded in that letter's § Corrections), then I re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p40-land`)
      and record the observed values here.
- [ ] Mark `- [ ] Add BJTs (2N3904, 2N3906, BC547…) and MOSFETs (2N7000, logic-level
      N-channel), top 3 manufacturers.` done (`[x]`) in `planning/todo.md`,
      touching only that line.
- [ ] Move `plan-40-A` … `plan-40-I` to `planning/completed/` (A–H are archived as
      each completes; I with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p40-land && cmake --build /tmp/logic-p40-land -j8 && ctest --test-dir /tmp/logic-p40-land` | `100% tests passed`, T0 + 1 programs (36 with T0 = 35), plus any program other plans added meanwhile (`git log --oneline <plan-40-F first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-40`) |
| Goldens vs plan-40 start | `git diff <plan-40-F first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-40's commits (`git log <plan-40-F first commit>^.. -- backend/tests/golden`) |
| Part counts | `grep -c '\|transistor\|' backend/tests/golden/parts.txt` | the `add` rows + 4 (`awk -F'\t' '$24=="add"' planning/plan-40-data/candidates.tsv \| wc -l`, plus the four generics) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (31608) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-40-A commit>.. -- schematical/schematical/Parts \| grep -v plan-40`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (1497) + the objects C–E added (G Phase 2's count), plus any other plan's |
| Datasheets | `find datasheets/transistor -name '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$24=="add" {print $27}' planning/plan-40-data/candidates.tsv \| sort -u \| wc -l`) |
| Datasheet URLs | `node scripts/check-datasheets.js $(python3 -I -c "import json; print(' '.join(x['id'] for x in json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets'] if x['id'].split('-')[0] in ('onsemi','nexperia','diodes','infineon') or x['id'].startswith('vishay-siliconix-')))")` (no id of those prefixes existed at 943924ba: `python3 -I` count → 0) | exits 0 |
| Skipped parts listed | `python3 -I planning/plan-40-data/census/verdicts.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-40-data/census/select.py /tmp/p40/raw planning/plan-40-data/candidates.tsv --stock planning/plan-40-data/stock-check.tsv --verdicts planning/plan-40-data/verdicts.tsv --report` | exits 0; no `NEEDS DEEPER LISTING` line beyond those in plan-40-A § Corrections |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p40-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p40-dd test -only-testing:schematicalTests/TransistorTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/ReferencePrefixTests -only-testing:schematicalTests/ResistorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p40-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/transistor-app.swift /tmp/p40-harness && /tmp/p40-harness/H.app/Contents/MacOS/harness` | `all checks ok`: `Q1` on `TO-92`, the drawer leaves of BJT / MOSFET, the LED lit with the NPN base on a pull-up and dark on a pull-down, the PMOS high-side LED lit, the symbols rendered |

Observed: (filled when the table is run)

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run).
Commit: —

## Validation Plan

- Final gate: Phase 1 is it (run once).

## Decisions

- **One landing run** — author: per-letter checks stayed scoped; this is where
  the whole set is checked together.
- Owner decisions: plan-40-A § Decisions.

## Corrections

(none yet)

## Summary

I only checks; the risk was spent in A–H. Left untouched: everything but this
file, `planning/todo.md`'s one line and the archive moves.
