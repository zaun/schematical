# plan-82-I: SunLED and QT Brightek parts; goldens; `skipped-parts.md`

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-82-H

I runs H's `gen-led.py` unchanged for SunLED (`led-sl-` devices) and QTB (`led-qtb-`), adds their
`datasheets.json` entries, regenerates the goldens, and records every `skip` row of plan-82 in
`datasheets/skipped-parts.md` § "Indicator LEDs (plan-82)".

References: plan-82-H § 3; `datasheets/skipped-parts.md` (format).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-H complete | `ls planning/completed/plan-82-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25) |
| Goldens at I's start | `wc -l < backend/tests/golden/parts.txt` | I records P3 |

## 1. Goal

- `led.json` has every `add` row of the four makers as a part:
  `python3 -I planning/plan-82-data/gen-led.py --check all` → `parts N = add rows N`.
- `skipped-parts.md` lists each `skip` row with its reason.

### Non-goals

No change to `gen-led.py`'s rules (a case they miss is an H correction).

## 2. Current State

After H: Kingbright and Broadcom parts ship.

## 3. Design

plan-82-H § 3, maker names `SunLED` and `QT Brightek`.

## 4. Phases

### Phase 1: SunLED and QTB parts

- [ ] Run `gen-led.py` for both; `datasheets.json`; goldens; review (only added `led` lines).

Acceptance: `gen-led.py --check all` → `parts N = add rows N`; `git diff
backend/tests/golden/parts.txt | grep '^-[^-]' | wc -l` → 0; `ctest --test-dir /tmp/logic-p82` →
`100% tests passed` — 3 min.

Commit:

### Phase 2: skipped parts

- [ ] `datasheets/skipped-parts.md` § Indicator LEDs (plan-82).

Acceptance: the section's row count equals `grep -c 'skip' planning/plan-82-data/verdicts.tsv` —
1 min.

Commit:

## 5. Validation Plan

I's gate: Phase 1's full `ctest`. The landing tier is J's.
