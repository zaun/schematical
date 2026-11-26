# plan-68-S: Backend coverage — CPUs, PIOs, AVR, MCU and PLD

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-R

The CPU adapters, the 65xx bus, the PIA/VIA, the AVR core and peripherals, the MCU kind and the PLD kind. Every line of these files ends up run by a test that checks what the
line did. 509 lines are uncovered in them today (26 files, 8466
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-R complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

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
letter S). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/pio_via6522.c` | 60 | 1006 |
| `src/models/pio.c` | 46 | 244 |
| `src/models/avr_timer.c` | 41 | 479 |
| `src/models/cpu.c` | 40 | 293 |
| `src/models/cpuz80.c` | 39 | 700 |
| `src/models/mcu_avr.c` | 38 | 475 |
| `src/models/mcu_avr_serial.c` | 36 | 445 |
| `src/models/mcu.c` | 25 | 216 |
| `src/models/avr_cpu.c` | 25 | 289 |
| `src/models/pio_pia6521.c` | 18 | 432 |
| `src/models/cpu6502.c` | 17 | 485 |
| `src/models/avr_twi.c` | 15 | 138 |
| `src/models/avr_chip.c` | 14 | 191 |
| `src/models/avr_timer_tiny.c` | 14 | 384 |
| `src/models/bus65.c` | 12 | 342 |
| `src/models/cpu65816.c` | 12 | 460 |
| `src/models/avr_usi.c` | 11 | 202 |
| `src/models/avr_instruction.c` | 9 | 725 |
| `src/models/avr_adc.c` | 8 | 145 |
| `src/models/avr_gpio.c` | 8 | 334 |
| `src/models/pld.c` | 6 | 62 |
| `src/models/avr_spi.c` | 5 | 87 |
| `src/models/avr.h` | 3 | 18 |
| `src/models/avr_clock.c` | 3 | 44 |
| `src/models/avr_usart.c` | 2 | 179 |
| `src/models/avr_eeprom.c` | 2 | 91 |
| **Total** | **509** | **8466** |

Where the tests go: `cpu_test`, `cpu_z80_test`, `cpu_65816_test`, `cpu_io_board_test`, `cpu_core_test`; `pio_test`, `pio_via_test`; `avr_core_test`, `avr_periph_test`, `avr_serial_test`, `avr_tiny_test`, `avr_board_test`, `mcu_test`; `pld_test`.

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

### Phase 1 — `pio_via6522.c`, `pio.c`, `avr_timer.c`, `cpu.c`, `cpuz80.c`, `mcu_avr.c`

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  Observed: `LOGIC_COV_WORK=/tmp/logic-cov-S node scripts/coverage.js --backend
  --only backend/src/models/` (2026-11-26, after this letter) lists for these
  files only `kind_alloc` NULL lines, left to the parent's `kind_arena_chunk`
  sweep (Corrections, S-9): `cpu.c:175-176, 219-220`, `pio.c:179-180,
  222-223, 259-260`, `pio_via6522.c:580-581`. `avr_timer.c`, `cpuz80.c`,
  `mcu_avr.c`, `pio_via6522_regs.c`: no line. `CT` 161/161 passed.
- [x] Ratchet rows removed for each file at 0. Remains: the parent runs
  `COV --backend --write-ratchet` once the S-9 lines are swept. (Not this sub-agent's to edit:
  the brief forbids `scripts/coverage-ratchet.tsv`; the parent writes it.)
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 26 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: c279db59

### Phase 2 — `mcu_avr_serial.c`, `mcu.c`, `avr_cpu.c`, `pio_pia6521.c`, `cpu6502.c`, `avr_twi.c`, `avr_chip.c`, `avr_timer_tiny.c`, `bus65.c`, `cpu65816.c`, `avr_usi.c`, `avr_instruction.c`, `avr_adc.c`, `avr_gpio.c`, `pld.c`, `avr_spi.c`, `avr.h`, `avr_clock.c`, `avr_usart.c`, `avr_eeprom.c`

- [x] As Phase 1, for these files. Observed: the same run lists only the
  `kind_alloc` NULL lines `mcu.c:67-68, 133-134, 196-197` and `pld.c:27-28,
  75-76` (S-9); every other file of this phase: no line. `SZ`
  (`node scripts/source-size.js --check`) exit 0; the new test files are
  96-687 lines.

Acceptance: as Phase 1, for these files.
  Check: as Phase 1; `SZ` → exit 0 (est. 2 min per run).
Commit: c279db59

## Validation Plan

- Tests: the cases added to the programs in § 2 (and any `_more_test`).
- Coverage check: `COV --backend --check` → exit 0 with every § 2 row gone.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`
  (est. 3 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.

## Corrections

- S-1 (§ 2 table). The re-run list at the start (2026-11-26, after plan-68-K)
  was 474 lines, not 509; it, not the table, was the work. Tests went into
  new programs, not the § 2 ones (the brief: new files only):
  `s68_loader_test` (loader rules, via `tests/support/s68_mutate.c`: one
  change to a shipped device), `pio_cov_test`, `cpu_cov_test`,
  `cpu_step_cov_test` (Z80 and 65816 on the generic pin bench),
  `avr_cov_test` (bare chips), `mcu_cov_test` (circuit, hand-assembled
  firmware).
- S-2 (bug, fixed). `avr_timer.c` `check_force_compare`: FOCnA/FOCnB with
  COMnx = 00 put an override on the pin (avr8js's `compToOverride(0)` =
  Enable), after which the pin stopped following PORT. ATmega328P 15.7.2
  table 15-2 and 15.9.2: COM = 00 is normal port operation, OCnx
  disconnected. Fixed for every timer; regression `avr_cov_test` "timer0:
  FOC0A with COM0A = 00 leaves PD6 a port pin". The avr8js oracle runs
  (`avr_periph_test`, `avr_core_test`) still match: no scenario forces with
  COM 00. No golden line changed.
- S-3 (bug, fixed outside `exact`). `avr_timer_tiny.c` `update_comp_config`
  reconnected OC1A/OC1B with the PORT bit on every TCCR1 and GTCCR write
  (avr8js), which undid a FOC1x or a compare match. Atmel 2586 table 12-1:
  OC1x is connected while COM1x != 00; other writes do not change it. Now
  the pin switches only when COM1x goes to or from 00 (`connected_a/_b`);
  `exact` keeps avr8js. Regression `avr_cov_test` "timer1: FOC1B with
  COM1B = 01 toggles OC1B". Oracle tests pass; no golden line changed.
- S-4 (bug, fixed). `avr_twi_reset` (every chip reset: the mcu's start and
  reset pin, a watchdog reset) set TWINT through `update_status`, so TWCR
  read 0x80 after reset. ATmega328P 22.9.2: TWCR's initial value is 0x00.
  Now only TWSR's status is set (F8). Regression `avr_cov_test` "twi: after
  a reset TWCR 00, TWSR F8". `avr_twi_init` (avr8js's constructor) still
  sets TWINT; `avr_serial_test` passes; no golden line changed.
- S-5 (found, not fixed). TWSR's status bits (TWS7:3) are read-only on the
  chip (22.9.3) but the port stores them on a write, as avr8js does
  (`tests/avr/src/twi.asm` writes the prescaler with status bits 0, and the
  oracle compares that). `avr_cov_test` uses this only in exact mode.
- S-6 (found, not fixed). `mcu_avr_serial.c` queues up to 8 UDR writes
  (`MCU_TX_QUEUE`) whatever UDRE says; 20.11.1: a write with UDRE0 = 0 is
  ignored. The tests here write at most two bytes back to back (the
  buffer and the shift register), which the chip does send.
- S-7 (unreachable, removed or restructured; callers read):
  `cpu.c` `cpu_timer` and `CpuCoreOps.timer` (all three cores pass NULL and
  none schedules a timer); `pio.c` `in_list(NULL)` (both callers pass a
  list); `bus65_level` (no caller) and `held_level`'s `held >= 0` (both
  callers pass a slot); `pio_via6522_regs.c` the T1/T2 time-out with
  `armed < 0` (`t1_start`/`t2_start` and the unknown reset set `run` to -1
  and clear the time-out whenever they set `armed` to -1); `cpuz80.c` the
  clock's "same level" return (every change to 0/1 sets `s->clock`, every
  X/Z sets it X), `mc_rise`'s `MC_NONE` (now starts the next machine cycle,
  the caller's other branch), MREAD/MWRITE's even-phase fall (falls see
  phases 1, 3, 5), the refresh tick's lost check (same INT/NMI as the data
  tick just passed: NMI X fails there, `same_core` compares the NMI pin; INT
  X only with iff1, which M1_T3 does not change); `mcu.c` a NULL pin name
  (the pinout loader requires "name"); `mcu_avr.c` `mcu_avr_pin`'s
  no-letter return (serial pins are on the layout's own ports), the NULL
  guard of `mcu_avr_destroy` (mcu.c checks; `mcu_avr_create` passes its
  own); `avr_cpu.c` the NULL guard of `avr_cpu_destroy`
  (`avr_cpu_create_data`, `avr_chip.c` pass a CPU they made);
  `mcu_avr_serial.c` the stale USART tx/rx timer checks (a serial reset
  always starts a new timer generation), the SPCR hook's "not running"
  check (only running firmware writes SPCR), the timer-index check (indices
  come from this file); `avr_timer.c` channel C (OCRnC: no layout has one;
  the config fields stay for the avr8js layout check) and
  `comp_to_override`'s default; `avr_timer_tiny.c` the COM = 0 default of
  `update_comp_pin_non_pwm` (every caller checks COM); `avr_gpio.c` the
  `NONE` case inside the not-NONE branch, the EICR search loop (a helper);
  `avr.h` `avr_usart_rx_busy` and `avr_clock.c` `avr_clock_time_nanos` (no
  caller).
- S-8 (assumption). The Z80 and 65816 cases assert the differential rule
  (cpu.h): an X that could change what the CPU does is LOST, one that cannot
  is not. The mcu SPI slave case asserts the model's documented
  simplification (a slave's byte ends with 0), not chip behavior.
- S-9 (left for the parent, per its instruction): the `kind_alloc` NULL
  lines `cpu.c:175-176, 219-220`, `mcu.c:67-68, 133-134, 196-197`,
  `pio.c:179-180, 222-223, 259-260`, `pio_via6522.c:580-581`, `pld.c:27-28,
  75-76`.
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
