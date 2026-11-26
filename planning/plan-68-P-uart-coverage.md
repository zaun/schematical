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

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  `LOGIC_COV_WORK=/tmp/logic-cov-P node scripts/coverage.js --backend --only backend/src/models/serial`
  → `serial_max310x*.c`: no line; `serial_i82510*.c`: only `serial_i82510_regs.c:546-547`;
  `serial_max3100.c`: only `:952-953` — both `kind_alloc` NULL returns, left for the
  `kind_arena_chunk` OOM sweep (parent). New programs `serial_max310x_cov_test`,
  `serial_max310x_cov2_test`, `serial_max3100_cov_test`, `serial_i82510_cov_test` pass.
- [x] Ratchet rows removed for each file at 0. Left to the parent: the files reach 0 only
  once the `kind_alloc` NULL lines are covered, and this letter does not edit the ratchet.
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 16 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `serial_i8251.c`, `serial.c`, `serial_acia6551.c`, `serial_hs82c52.c`, `serial_com78c80x.c`, `serial_ay1013.c`, `serial_util.h`, `serial_line.c`, `serial_usbuart.c`, `serial_bus.c`

- [x] As Phase 1, for these files. Same command → no line in `serial_com78c80x.c`,
  `serial_util.h`, `serial_line.c`, `serial_bus.c`; the only lines left are `kind_alloc`
  NULL returns: `serial_acia6551.c:795-796`, `serial_ay1013.c:1098-1099`,
  `serial_hs82c52.c:880-881`, `serial_i8251.c:923-924`, `serial_usbuart.c:405-406, 451-452`,
  `serial.c:211-212, 279-280, 324-325, 328-329, 371-372, 409-410, 434-435` (parent's OOM
  sweep). `backend: 11828 lines, 122 uncovered` for the whole prefix (the rest is
  `serial_target.c`/`serialmem*`, not this letter). New programs `serial_i8251_cov_test`,
  `serial_acia_cov_test`, `serial_ay1013_cov_test`, `serial_hs_com_cov_test`,
  `usbuart_cov_test`, `serial_core_cov_test` pass; `SZ` → exit 0. Ratchet rows: as Phase 1.

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

- **82510 RTS/DTR straps never applied (model bug, fixed).** classic-b.md § 82510
  (82510.pdf p8, p38): RTS/DTR are sampled as RESET falls, held 0..Tcy-20 after it. The
  model read the pins at the instant it handled RESET falling: a strap released at that
  instant read as the weak pull-up (1); one held longer shorted the chip's own RTS/DTR
  drive. Now each strap pin's last change is recorded and the level held before RESET
  fell is used (`serial_i82510_regs.c`, three fields in `serial_i82510_int.h`).
  Regression: `serial_i82510_cov_test` `case_straps` (failed before, passes after). No
  golden line changes (golden tests pass).
- **Unreachable code removed or restructured (callers read, behavior unchanged):**
  - `serial_max310x*.c`: the in-reset check in `rts_internal` (`m31x_cts_internal` only
    runs from `tx_fetch` on a baud tick; the generator is stopped in reset);
    `SERIAL_NO_PIN` in `m31x_mx_open_drain` (only via `part_pin_open_drain` from
    `voltage.c`/`erc_pins.c` with a real pin); `ch >= uarts` in `mx_timer` (timers carry
    `c->index` or a loop channel); `reg_peek`'s 0x1F "no RevID, not global" branch (the
    variant table makes 0x1F valid only there); `i2c_ch < 0` in the I2C received/next/
    acked callbacks (`serial_target.c` passes data only after an address the model ACKed,
    which sets `i2c_ch`); `reg_peek` case braces after `return`.
  - `serial_i82510*.c`: in-reset / unknown / power-down returns in `serial_tick_tx`/`_rx`
    (callers `i82_sysclk_edge`, `i82_sclk_edge`, `brg_ticked` return on those states
    first); the `unknown` return in `bus_read_done` (all it skips is overwritten by
    `reset_regs` before the chip leaves unknown); the TMST read made one line.
  - `serial_i8251.c`: `unknown` guards in `bus_peek`/`bus_write`/`bus_read_done`
    (`apply_reset` disables the bus whenever it sets `unknown`); the `mode_known` TSR X
    branch in `tx_edge` (`is_async()` is -1 only when the mode is unknown, which goes to
    PH_X).
  - `serial_acia6551.c`: `write_reg` case 2 moved to `write_command()`; `tx_bit_ends()`
    without a `default` (only reached from `tx_tick` while busy).
  - `serial_hs82c52.c`: `unknown` guards in `bus_peek`/`bus_write` (same reason as the
    8251); `serial_com78c80x.c`: the `in->count == 0` branch of `in_level` (every `CmIn`
    comes from `bind_in`, which fails on a missing pin), two brace blocks llvm-cov
    counted as unrun.
  - `serial_line.c`: `parity_bit`'s NONE case (callers `tx_bit_level`, `rx_deliver`,
    `sender_bit` never pass NONE; it still returns 1), the dead `return LOGIC_X` after
    `tx_bit_level`'s full switch, braces in `serial_tx_tick`.
  - `serial_util.h` `su_load_names`: the "1 to cap names" branch (all 15 callers pass a
    non-zero `exact` <= `cap`); the check is now `n != exact || n > cap`.
- **Not bugs, noted:** i8251 async ignores the receiver while RxE is X (sync loads with
  RxRDY X); AY-5-1013 mode 1 holds the transfer while CTS is X; HS-82C52 leaves DR 0
  with REN X; the 82510 after crystal sleep keeps register value bits 0 while unknown
  (PMD 00H makes DSR/TA/OUT0, DCD/ICLK/OUT1 outputs).
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
