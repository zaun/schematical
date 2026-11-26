# plan-82-J: Indicator LEDs: landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-82-I

J runs the landing tier once for the whole of plan-82, checks the behavioral outcome of
plan-82-A, ticks the todo, and archives the plan.

References: plan-82-A (the outcome); `AGENTS.md` § Testing in the app (pointer to (0, 0) after
driving the mouse); memory notes on app driving are not prerequisites.

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-I complete | `ls planning/completed/plan-82-I-*.md` → exists, or every I box ticked | NOT MET (2026-11-25) |

## 1. Goal

Every line of § 4 passes; plan-82 A–J are in `planning/completed/`.

### Non-goals

No new work: a failing gate goes back to the letter that owns it (recorded in that letter's
Corrections).

## 2. Current State

After I: every part ships.

## 3. Design

The targeted gates for every area plan-82 touched: backend (`led` kind, goldens), footprints, app
(LED classes, Part Editor), the real window.

## 4. Phases

### Phase 1: the landing tier

- [ ] Backend: `ctest --test-dir /tmp/logic-p82` → `100% tests passed` out of T0 + 1 (78 if no
      other plan added programs; J records the actual count) — 1 min.
- [ ] Sizes and coverage: `node scripts/source-size.js --check` and
      `node scripts/coverage.js --check --backend` exit 0 — 5 min.
- [ ] Parts: `grep -c '|led|' backend/tests/golden/parts.txt` = 1 + the `add` rows of
      `verdicts.tsv` — 1 min.
- [ ] App: `xcodebuild … test -only-testing:schematicalTests/LedTests
      -only-testing:schematicalTests/PartKindKeysTests
      -only-testing:schematicalTests/PartKindRoundTripTests
      -only-testing:schematicalTests/SlideSwitchTests` → `** TEST SUCCEEDED **`
      (`SlideSwitchTests` places `GENERIC-LED`: the unchanged path) — ~10 min.
- [ ] Real window: open the app, place `APT1608EC` and one RGB part from Displays & Indicators →
      LED, wire the RGB's K2 low through a switch, Run: only the green die glows; the red part
      shows no colour picker. Move the pointer to (0, 0) after — 10 min.

Acceptance: every line above passes; results recorded here.

Commit:

### Phase 2: close

- [ ] Tick any `planning/todo.md` item plan-82 finished (none is listed by name today; todo.md:56 is
      not this plan's).
- [ ] Move `planning/plan-82-*.md` to `planning/completed/`.

Acceptance: `ls planning/plan-82-*.md 2>/dev/null | wc -l` → 0 — 1 min.

Commit:

## 5. Validation Plan

This letter is the landing tier.
