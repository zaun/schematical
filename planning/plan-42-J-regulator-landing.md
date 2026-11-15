# plan-42-J: Landing tier

Last updated: 2026-11-15
Effort: small (< 1h)
Depends on: plan-42-I

J runs plan-42's landing tier once, on the landed tree on `main`, and
archives the plan. Every gate's command is in the table; the observed values
are recorded under it.

References:

- plan-42-A § 1 (the plan's outcome), plan-42-G § 1, plan-42-H § 1,
  plan-42-I § 1.
- `planning/completed/plan-40-I-transistor-landing.md` (the precedent).

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-I complete | `ls planning/completed/plan-42-I-*.md` → exists, or every I box ticked | NOT MET (2026-11-15: not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and
  recorded in that letter's § Corrections), then J re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p42-land`)
      and record the observed values here.
- [ ] Mark `Add voltage regulators: 78xx, AMS1117, LM317, LDOs, and common buck
      modules.` done (`[x]`) in `planning/todo.md` with a note on the same
      item: "(plan-42: TI, ADI, Microchip; AMS1117 and buck modules out of
      scope)", touching only that item.
- [ ] Move `plan-42-A` … `plan-42-J` to `planning/completed/` (A–I are
      archived as each completes; J with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p42-land && cmake --build /tmp/logic-p42-land -j8 && ctest --test-dir /tmp/logic-p42-land` | `100% tests passed`, T0 + 1 programs (37 with T0 = 36), plus any program other plans added meanwhile (`git log --oneline <plan-42-G first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-42`) |
| Goldens vs plan-42 start | `git diff <plan-42-G first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-42's commits |
| Part counts | `grep -c '\|regulator\|' backend/tests/golden/parts.txt` | the `add` rows + 3 (`awk -F'\t' '$29=="add"' planning/plan-42-data/candidates.tsv \| wc -l`, plus the three generics) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (32854) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-42-A first commit>.. -- schematical/schematical/Parts \| grep -v plan-42`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (1624) + the objects D–F added (H Phase 2's count), plus any other plan's |
| Datasheets | `find datasheets/regulator -iname '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$29=="add" {print $32}' planning/plan-42-data/candidates.tsv \| sort -u \| wc -l`) plus any a footprint note cites (H records them) |
| Datasheet URLs | `node scripts/check-datasheets.js <every datasheets.json id H added>` (H Phase 2 lists them) | exits 0 |
| Skipped parts listed | `python3 -I planning/plan-42-data/census/verdicts.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-42-data/census/select.py /tmp/p42/raw planning/plan-42-data/candidates.tsv --stock planning/plan-42-data/stock-check.tsv --verdicts planning/plan-42-data/verdicts.tsv --report` | exits 0; no `NEEDS DEEPER LISTING`; every `add` row `stock_checked` 1 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p42-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p42-dd test -only-testing:schematicalTests/RegulatorTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/ResistorTests -only-testing:schematicalTests/TransistorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p42-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/regulator-app.swift /tmp/p42-harness && /tmp/p42-harness/H.app/Contents/MacOS/harness` | `all checks ok`: the drawer leaves, `U1` on `TO-220-3`, the LED lit through `GENERIC-REG-3V3` and dark with IN unwired, the 2.7 V part refused at 3.3 V, the divider at 5 V and 3.28 V |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run, and nothing
  smaller covers the backend, the part files and the app together).
Commit: —

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **ResistorTests and TransistorTests are in the app gate**: I changes
  how a placed `GENERIC-RESISTOR`'s value reaches the backend, and the
  voltage check's level classes, which those kinds' tests exercise.

## Corrections

## Summary

One run of every gate; no new work.
