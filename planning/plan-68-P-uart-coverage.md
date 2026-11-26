# plan-68-P: Backend coverage — the other UARTs and the serial plumbing

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-O

The Maxim UARTs, the classic UARTs, the USB UART and the shared serial line/bus/utility code. Every line of these files ends up run by a test that checks what the
line did. 1074 lines are uncovered in them today (13 files, 8984
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-O complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

References:

- The files in § 2 and the test programs that exercise them (§ 2).
- `planning/plan-68-data/backend-missed-by-letter.tsv` (the per-file census
  this letter is written from: `llvm-cov report` of the instrumented `CT` at
  `e2f6811a`).
- `docs/backend.md` § Tests (each program); `docs/parts-format.md` (each
  kind's keys, for loader rejection cases).
- `AGENTS.md` § Datasheets (goldens) and § Source size and coverage.

## 1. Goal

- `node scripts/coverage.js --backend --only <file>` prints no uncovered line
  for any file in § 2.
- Their rows are gone from `scripts/coverage-ratchet.tsv`; `COV --backend
  --check` → exit 0.
- `CT` passes; `SZ` passes (no test program over 1000; new ones at or under
  800).

### Non-goals

- No change to simulated behavior except a bug a new test proves (§ 3,
  step 6), each listed under Corrections with its golden diff.
- No production file grows past 800 code lines; no new test helper is added
  to `logic_test_support` (benches stay per area).

## 2. Current State

Uncovered lines at `e2f6811a` (`planning/plan-68-data/backend-missed-by-letter.tsv`,
letter P). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/serial_max310x.c` | 336 | 1770 |
| `src/models/serial_i82510.c` | 139 | 1106 |
| `src/models/serial_max3100.c` | 94 | 772 |
| `src/models/serial_i8251.c` | 76 | 733 |
| `src/models/serial.c` | 70 | 399 |
| `src/models/serial_acia6551.c` | 66 | 621 |
| `src/models/serial_hs82c52.c` | 66 | 701 |
| `src/models/serial_com78c80x.c` | 63 | 748 |
| `src/models/serial_ay1013.c` | 63 | 844 |
| `src/models/serial_util.h` | 35 | 177 |
| `src/models/serial_line.c` | 35 | 494 |
| `src/models/serial_usbuart.c` | 29 | 319 |
| `src/models/serial_bus.c` | 2 | 300 |
| **Total** | **1074** | **8984** |

Where the tests go: `serial_maxim_test`, `serial_max310x_test`, `serial_ay1013_test`, `serial_intel_test`, `serial_acia_test`, `serial_hs_com_test`, `usbuart_test`; `serial.c`/`serial_line.c`/`serial_util.h`/`serial_bus.c` paths in whichever of those runs the part that reaches them.

## 3. Design

The method is plan-68-L § 3, unchanged: list, classify (loader rejection,
undriven model state, API misuse, unreachable), assert an observable result,
put the test in the program named in § 2, lower the ratchet row in the same
commit, fix any bug a test finds with that test as its regression.

### Where the risk is

A new case that pins today's behavior where today's behavior is wrong. The
assertion comes from the datasheet section the model cites, not from what the
model happens to do; where the two differ it is a bug (§ 3, step 6), never a
test written to match.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — `serial_max310x.c`, `serial_i82510.c`, `serial_max3100.c`

- [ ] Re-run `COV --backend --only` for each file; close every line per § 3.
- [ ] Ratchet rows removed for each file at 0.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `serial_i8251.c`, `serial.c`, `serial_acia6551.c`, `serial_hs82c52.c`, `serial_com78c80x.c`, `serial_ay1013.c`, `serial_util.h`, `serial_line.c`, `serial_usbuart.c`, `serial_bus.c`

- [ ] As Phase 1, for these files.

Acceptance: as Phase 1, for these files.
  Check: as Phase 1; `SZ` → exit 0 (est. 2 min per run).
Commit: —

## Validation Plan

- Tests: the cases added to the programs in § 2 (and any `_more_test`).
- Coverage check: `COV --backend --check` → exit 0 with every § 2 row gone.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`
  (est. 3 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.

## Corrections

(none yet)

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
