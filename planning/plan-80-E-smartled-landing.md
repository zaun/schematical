# plan-80-E: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-80-D

E runs plan-80's landing tier once on the landed tree on `main`, records the observed values, ticks
`planning/todo.md` where it applies, and archives the plan.

References:

- plan-80-A § 1 and § Prerequisites (T0 77, P0 = R0 40832, F0 3993), B § 1, C § 1 (N_add, N_fp),
  D § 1.
- `planning/plan-74-K-ethernet-landing.md` (the precedent).

## Prerequisites

plan-80-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-80-D complete | `ls planning/completed/plan-80-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work. A failing gate is fixed in the letter that owns the area (and recorded in that
  letter's § Corrections), and then E re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill each `Commit:` line with the hash when the phase lands.

### Phase 1: the gates

| Gate | Command | Expected | Observed |
|---|---|---|---|
| Backend | `cmake --build /tmp/logic-p80 -j8 && ctest --test-dir /tmp/logic-p80 -j8` | `100% tests passed`, T1 + 1 programs (T1 = B's start count) | |
| Size and coverage | `node scripts/source-size.js --check && node scripts/coverage.js --check --backend` | exit 0 | |
| Goldens | `git diff --numstat <commit before B> -- backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt` | N_add, N_add, N_fp added; 0 deleted | |
| The chain | `ctest --test-dir /tmp/logic-p80 -R smartled_test --output-on-failure \| grep 'lights three'` | one `ok` line | |
| App classes | `xcodebuild test … -only-testing:` the five classes of D Phase 1 | `** TEST SUCCEEDED **` | |
| Harness | D Phase 3's command | every line `ok` | |

- [ ] Run every gate; fill Observed.
- [ ] `planning/todo.md` has no WS2812 or addressable-LED item (`grep -n -i 'ws2812\|addressable'
      planning/todo.md` → nothing, 2026-11-25), so E ticks no todo line.
- [ ] `git mv planning/plan-80-*.md planning/completed/` (the data folder stays).

Acceptance: the table's Observed column equals Expected on every row. (≈ 45 min: backend 5, app 30,
harness 5.)

Commit:

## Corrections
