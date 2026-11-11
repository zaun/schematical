# plan-34-H: Landing tier

Last updated: 2026-11-11
Effort: small (< 1h)
Depends on: plan-34-G

H runs plan-34's landing tier once, on the landed tree, and archives the
plan. Every gate's command is in the table; the observed values are
recorded under it.

References:

- plan-34-A § 1 (the plan's outcome), plan-34-E § 6, plan-34-G § 1.
- `planning/completed/plan-29-N-diode-landing.md` (the precedent).

## Prerequisites

plan-34-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-34-G complete | `ls planning/completed/plan-34-G-*.md` → exists | NOT MET at writing (2026-11-11) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area
  (and recorded in that letter's § Corrections), then H re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir
      (`/tmp/logic-p34-land`) and record the observed values here.
- [ ] Mark `- [ ] Add top 3 Resitor manufactuerers and their active
      resistors` done (`[x]`) in `planning/todo.md`, touching only that line.
- [ ] Move `plan-34-A` … `plan-34-H` to `planning/completed/` (A–G are
      archived as each completes; H with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p34-land && cmake --build /tmp/logic-p34-land -j8 && ctest --test-dir /tmp/logic-p34-land` | `100% tests passed`, T0 + 1 programs (31 with T0 = 30), plus any program other plans added meanwhile (`git log --oneline <plan-34-A commit>.. -- backend/CMakeLists.txt \| grep -v plan-34`) |
| Goldens vs plan-34 start | `git diff <plan-34-E first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-34's commits (`git log <plan-34-E first commit>^.. -- backend/tests/golden`) |
| Part counts | `grep -c '\|resistor\|' backend/tests/golden/parts.txt` | the `add` rows + 1 (`awk -F'\t' '$22=="add"' planning/plan-34-data/candidates.tsv \| wc -l`, plus `GENERIC-RESISTOR`) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (26729) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-34-A commit>.. -- schematical/schematical/Parts \| grep -v plan-34`) |
| Datasheets | `ls datasheets/resistor/*/*.pdf \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$22=="add" {print $21}' planning/plan-34-data/candidates.tsv \| sort -u \| wc -l`) |
| Skipped parts listed | `python3 -I planning/plan-34-data/census/verdicts.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-34-data/census/select.py /tmp/p34/raw planning/plan-34-data/candidates.tsv --stock planning/plan-34-data/stock-check.tsv --verdicts planning/plan-34-data/verdicts.tsv --report` | exits 0; no `NEEDS DEEPER LISTING` line beyond those in plan-34-A § Corrections |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p34-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p34-dd test -only-testing:schematicalTests/ResistorTests -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `scripts/harness/build.sh scripts/harness/resistor-app.swift /tmp/p34-harness && /tmp/p34-harness/H.app/Contents/MacOS/harness` | `all checks ok`: R1 1kΩ on `RES-AXIAL-10.16`, the 4.7 kΩ round trip and undo, the LED lit through the resistor for inverter input 0 and dark for 1, the drawer groups in ohms order |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 30 min: the fresh backend build, the
  app tests and the harness; this is the plan's one landing run).
Commit: —

## Validation Plan

- Final gate: Phase 1 is it (run once).

## Decisions

- **One landing run** — author: per-letter checks stayed scoped; this is
  where the whole set is checked together.
- Owner decisions: plan-34-A § Decisions.

## Corrections

(none yet)

## Summary

H only checks; the risk was spent in A–G. Left untouched: everything but
this file, `planning/todo.md`'s one line, and the archive moves.
