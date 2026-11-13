# plan-36-J: Explicit footprint pads named by ball

Last updated: 2026-11-13
Effort: small (< 1h)
Depends on: plan-36-B (the pinouts that need it). Needed by: plan-36-D, plan-36-G.

Added while D was being worked (§ Corrections of D). B's pinouts give
three microcontrollers designators that are ball names but whose pads
are no regular grid:

- the dual-row QFNs `44MC` (ATmega164A/164PA, terminals `A1`…`A24`,
  `B1`…`B20`) and `64MC` (ATmega169PV), the ATDF's and the datasheet's
  numbering of the two staggered rows;
- the `12U-3` WLCSP of ATtiny20 (balls `A1 A3 A5 B2 B4 B6 …`, a 4 × 6
  staggered array: 0.3465 mm rows, 0.20 mm columns).

The loader matches a part's pins to a footprint's pads by designator. An
`explicit` footprint names its pads `"1"`…`"<pins>"` and a `grid` footprint
has one pitch for rows and columns. Neither can carry these pads, and D's
design ("`explicit` … with designators as the datasheet numbers them")
assumed a format that did not exist. No letter of plan-36 covered it.

Outcome: an `explicit` pad may say `"ball": "A1"` instead of `"pin": n`.
The ball name is its designator, so a ball-pinout part loads on it.

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| Explicit pads are numbered only | `grep -c '"ball"' backend/src/library/footprints.c` → 0 (the grid style's `balls` key aside) | MET 2026-11-13 (0) |

## Design

- `PAD_KEYS` gains `"ball"`. A pad has `"pin"` or `"ball"`, not both. A
  ball must be a JEP95 ball name (`kind_is_ball_name`).
- A footprint uses one scheme: all designated pads `"ball"`, or all
  `"pin"`. `"pin": 0` (unnumbered) goes with either.
- A ball footprint names exactly `pins` distinct balls (`N balls for M
  pins`, `more than M balls`). Pads come out grouped by ball in order of
  first use, then the unnumbered pads, the rule numbered pads already follow.
- No change to any existing footprint, golden or the app (the app reads the
  backend's pad designators).

## Phases

### Phase 1 — Loader, tests, docs

- [x] `backend/src/library/footprints.c` `load_explicit`: the `"ball"` key and its checks.
      Built with no warnings.
- [x] `backend/tests/library_check.c` `check_explicit_balls`: a ball footprint's pad order
      (B2, B2, A1, then the unnumbered pad), a ball-pinout LED on it, and the five
      rejections (`1 balls for 2 pins`, `more than 2 balls`, mixed schemes, `pin` and
      `ball`, `"I1" is not a ball name`) → all `ok`.
- [x] `docs/parts-format.md` § `explicit` and the `pins` row.

Acceptance: the loader takes ball pads, and nothing else moved.
  Check: `ctest --test-dir /tmp/logic-p36` → `100% tests passed, 0 tests failed out of 32`; `git diff --stat backend/tests/golden` → empty.
  Result 2026-11-13: `100% tests passed, 0 tests failed out of 32`; golden diff empty.
Commit: —

## Corrections

## Summary

One optional pad key, used by D's three dual-row-QFN and WLCSP footprints.
