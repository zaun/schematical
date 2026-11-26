# plan-70-J: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-70-I

J runs plan-70's landing tier once, on the landed tree on `main`, and
archives the plan. Every gate's command is in the table, and the observed
values are recorded under it.

References:

- plan-70-A § 1 and § Prerequisites (the baselines T0 77, P0 40832, R0
  40832, F0 3993, E0 40832); plan-70-G § 1, plan-70-H § 1, plan-70-I § 1.
- `planning/completed/plan-48-I-timer-opamp-landing.md` (the precedent).

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-I complete | `ls planning/completed/plan-70-I-*.md` → exists, or every I box ticked | NOT MET (2026-11-25: I not started) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work. A failing gate is fixed in the letter that owns the area,
  and recorded in that letter's § Corrections. Then J re-runs.
- `planning/todo.md` has no item for this plan's parts, apart from the
  reference-board line "optocoupler: 2 parts" (`planning/todo.md:232`). J
  does not tick that line: one of its two parts is a magnetic sensor (A §
  2), and the reference-board list is plan-43's.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: Landing tier

- [ ] Run every gate of the table on a fresh build dir
      (`/tmp/logic-p70-land`) and record the observed values here.
- [ ] Move `plan-70-A` … `plan-70-J` to `planning/completed/` (A–I are
      archived as each completes; J with this commit).

| Gate | Command | Expected |
|---|---|---|
| Size and coverage rules (plan-68; owner, 2026-11-25) | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0: no in-scope file over 1000 code lines and no uncovered line in the backend, app or scripts tiers (est. 30 min: the app tier is measured only by plan-68's full Debug-suite run, 1,634 s at plan-68's planning, and nothing smaller shows the new Swift lines run) |
| Backend tests | `cmake -S backend -B /tmp/logic-p70-land -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p70-land -j8 && ctest --test-dir /tmp/logic-p70-land -j8` | `100% tests passed`, T0 + 1 programs (78 with T0 = 77), plus any program other plans added meanwhile (`git log --oneline <plan-70-G first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-70`) |
| Goldens only grow | `git diff <plan-70-G first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'`, counting only plan-70's commits | 0, apart from the per-kind count lines of G § 3.7 |
| Part count | `grep -c '\|isolator\|' backend/tests/golden/parts.txt` | the `add` rows + 2 (`awk -F'\t' 'NR>1 && $23=="add"' planning/plan-70-data/candidates.tsv \| wc -l`); at most 2105 |
| Per-maker cap | `awk -F'\t' 'NR>1 && $23=="add"{n[$1" "$2]++} END{for(k in n)print k,n[k]}' planning/plan-70-data/candidates.tsv` | 12 lines (the makers of plan-70-A § 1), each at most 200 |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F2 (plan-70-H § Prerequisites) + the merged objects of `footprints-E.json` and `footprints-F.json`, less the ids H recorded as already present, plus any other plan's |
| Datasheets | `find datasheets/isolator -iname '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite |
| Datasheet URLs | `node scripts/check-datasheets.js --host <host>` for the ten hosts of plan-70-H Phases 2–3 | each exits 0 for plan-70's entries |
| Skipped parts listed | `python3 -I planning/plan-70-data/census/final.py --verify-skipped` | `<n> skip rows, 0 missing from skipped-parts.md` |
| Stock | `awk -F'\t' 'NR>1 && $23=="add" && $15<=0' planning/plan-70-data/candidates.tsv \| wc -l` | 0 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p70-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild … test -only-testing:schematicalTests/IsolatorTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p70-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/isolator-app.swift /tmp/p70-harness && /tmp/p70-harness/H.app/Contents/MacOS/harness` | every line `ok` |

Acceptance: every gate's observed value matches.
  Check: the table's commands. About 45 min: the fresh backend build, the
  app tests and the harness. This is the plan's one landing run, and
  nothing smaller covers the backend, the part files and the app together.
Commit:

## Validation Plan

- The table above is the plan's landing tier. The full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **the reference-board todo line stays as it is** (§ 1 Non-goals).

## Corrections

## Summary

One run of every gate; no new work.
