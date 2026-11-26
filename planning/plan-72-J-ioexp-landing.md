# plan-72-J: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-72-I

J runs plan-72's landing tier once, on the landed tree on `main`, ticks the
todo item, updates plan-43's two backlog counts, and archives the plan. Every
gate's command is in the table; the observed values go under it.

References: plan-72-A § 1 and § Prerequisites (T0 77, P0 40832, R0 40832,
F0 3993 at planning; A re-records them), the § 1 of every letter;
`planning/completed/plan-48-I-timer-opamp-landing.md` (precedent).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-72-I complete | `grep -c '^- \[ \]' planning/plan-72-I-ioexp-app.md` → 0 | NOT MET (2026-11-25) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals

- No new work: a failing gate is fixed in the letter that owns the area (recorded in that letter's Corrections), then J re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — gates

| Gate | Command | Expected |
|---|---|---|
| Backend | `cmake -S backend -B /tmp/logic-p72-land -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p72-land -j8 && ctest --test-dir /tmp/logic-p72-land -j8` | `100% tests passed`, T0 + 2 programs |
| Goldens only grew | `git diff <A's first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part counts | `grep -c '\|ioexpander\|' backend/tests/golden/parts.txt`; `grep -c '\|i2cbus\|' …` | the `add` rows of each family in `candidates.tsv` |
| Datasheets | `python3 -I planning/plan-72-data/census/place.py --verify` | `0 problems` |
| Skipped parts | `python3 -I planning/plan-72-data/census/decide.py --verify-skipped` | `0 … missing` |
| Size and coverage rules | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0 |
| App | the plan-72-I Phase 1–3 test classes in one `xcodebuild … test` | `** TEST SUCCEEDED **` |
| Harness | plan-72-I Phase 3's harness command | every line `ok` |

- [ ] Run every gate on a fresh build dir and record the observed values here.
- [ ] `planning/todo.md`: tick `Add IO Extenders` (`- [x] Add IO Extenders (plan-72: TI, ADI, NXP, Microchip expanders; I2C buffers and muxes)`); rewrite the plan-43 backlog lines `bus-switch: 17 parts.` and `driver: 8 parts.` to the counts left after the plan-43 rows this plan added (`awk` of `planning/plan-43-data/candidates.tsv` rows whose part is now in `parts.txt`).
- [ ] Move `plan-72-A` … `plan-72-J` to `planning/completed/`.

Acceptance: every gate shows its expected value in one run.
  Check: the table's commands (est. 45 min: fresh backend build, app tests, harness).
Commit:

## Validation Plan

- This letter is the validation plan of plan-72: the table above, once.

## Decisions

- As plan-72-A.

## Corrections

(none yet)
