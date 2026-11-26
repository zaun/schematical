# plan-77-H: STM32: landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-77-G

H runs the landing tier for every area plan-77 touched, records the results, and archives the
plan.

References: `planning/plan-77-A-stm32-data.md` (outcome), `docs/backend.md` § tests,
AGENTS.md § Git.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-77-A … G complete | `ls planning/completed/plan-77-{A,B,C,D,E,F,G}-*.md` → 7 files | NOT MET (2026-11-25) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

The landing gates below pass; the plan's letters move to `planning/completed/`.

### Non-goals

- `planning/todo.md:166` ("4-layer RP2040 or STM32 board with USB") stays open: it asks for a
  board example, which plan-77 does not build.

## 2. Validation Plan (the landing tier)

| Gate | Command | Expect | Time |
|---|---|---|---|
| Backend | `cmake --build /tmp/logic-p77 -j8 && ctest --test-dir /tmp/logic-p77 -j8` | 100% of T1 | ~1 min |
| plan-68 ratchets | `node scripts/source-size.js --check`; `node scripts/coverage.js --check --backend` | exit 0 | ~5 min |
| Part counts | `grep -c '\|mcu\|' backend/tests/golden/parts.txt` | M0 + 300 = 1724 | s |
| STM32 facts | `grep "^$(awk -F'\t' '$2=="add"{print $1; exit}' planning/plan-77-data/verdicts.tsv)|" backend/tests/golden/parts.txt` | the `core=`, `clock=`, `flash=`, `ram=` of that part's row of `facts.tsv` | s |
| App tests | `xcodebuild test … -only-testing:schematicalTests/McuTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests` | `** TEST SUCCEEDED **` | ~3 min |

The check part is `verdicts.tsv`'s first `add` row (the most-stocked STM32).

## 3. Phases

### Phase 1 — Run and archive

- [ ] Run each gate; record each command's output here.
- [ ] `git mv planning/plan-77-*.md planning/completed/`.

Acceptance: every gate's recorded output matches its Expect column.

Commit:
