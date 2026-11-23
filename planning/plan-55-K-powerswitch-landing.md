# plan-55-K: Landing tier

Last updated: 2026-11-22
Effort: small (< 1h)
Depends on: plan-55-J

K runs plan-55's landing tier once, on the landed tree on `main`, records the todo item and archives
the plan. Every gate's command is in the table; the observed values are recorded under it.

References:

- plan-55-A § 1 (the plan's outcome), plan-55-H § 1, plan-55-I § 1, plan-55-J § 1.
- `planning/completed/plan-42-J-regulator-landing.md` (the precedent).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-J complete | `ls planning/completed/plan-55-J-*.md` → exists | NOT MET (2026-11-22: not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every gate of the table passes on a fresh build.
- `planning/todo.md` records plan-55 and `plan-55-A` … `plan-55-K` are in `planning/completed/`.

### Non-goals (explicit constraints)

- No new work: a gate that fails is fixed in the letter that owns it (a Correction there), then the table is re-run.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill `Commit:` the moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p55-land`) and record the
      observed values under it.
- [ ] `planning/todo.md`, section Parts: add `- [x] Add power switches: integrated ideal diodes, load
      switches, current-limited and USB switches (plan-55: top three makers per family, Active only; power
      muxes, external-FET controllers, eFuses and hot-swap controllers out of scope)`, touching only that
      item.
- [ ] Move `plan-55-A` … `plan-55-K` to `planning/completed/` (A to J are archived as each completes; K
      with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p55-land && cmake --build /tmp/logic-p55-land -j8 && ctest --test-dir /tmp/logic-p55-land` | `100% tests passed`, T0 + 1 programs (42 with T0 = 41), plus any program other plans added meanwhile (`git log --oneline <plan-55-A first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-55`) |
| Goldens vs plan-55 start | `git diff <plan-55-A first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-55's commits |
| Part count | `grep -c '\|powerswitch\|' backend/tests/golden/parts.txt` | the `add` rows (`python3 -I planning/plan-55-data/census/verdicts.py --report` → the sum of the three families' `add`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (the value at H's start) plus the objects H and I merged |
| Datasheets | `find datasheets/powerswitch -iname '*.pdf' \| wc -l` | the `add pdfs` of `verdicts.py --report` |
| Datasheet URLs | `node scripts/check-datasheets.js <every datasheets.json id H and I added>` | exits 0 |
| Skipped parts listed | `python3 -I planning/plan-55-data/census/verdicts.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-55-data/census/select.py planning/plan-55-data/raw planning/plan-55-data/candidates.tsv --stock planning/plan-55-data/stock-check.tsv --carry --report` | exits 0; every `add` row `stock_checked` 1 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p55-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild … test -only-testing:schematicalTests/PowerSwitchTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/RegulatorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p55-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/powerswitch-app.swift /tmp/p55-harness && /tmp/p55-harness/H.app/Contents/MacOS/harness` | `all checks ok` |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 60 min: the fresh backend build, the nine app test classes and the
  harness; this is the plan's one landing run, and nothing smaller covers the backend, the part files
  and the app together).
Commit: —

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the project's background
  policy, not as a plan step.

## Decisions

- Author: **RegulatorTests is in the app gate**: A adds a level class beside the regulated one in
  `voltage.c`, and the regulator's voltage tests exercise the same file.

## Corrections

(none yet)

## Summary

One run of every gate; no new work.
