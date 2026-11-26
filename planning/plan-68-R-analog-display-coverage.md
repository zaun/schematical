# plan-68-R: Backend coverage — memories, passives, power, timers and displays

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-Q

Serial memories, EEPROM, SRAM, relays, regulators, transistors, power switches, the passives, the timer and op-amp, and the display models. Every line of these files ends up run by a test that checks what the
line did. 1264 lines are uncovered in them today (35 files, 10796
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-Q complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

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
letter R). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/serialmem.c` | 130 | 1395 |
| `src/models/charlcd.c` | 93 | 927 |
| `src/models/relay.c` | 89 | 461 |
| `src/models/eeprom.c` | 87 | 838 |
| `src/models/regulator.c` | 83 | 537 |
| `src/models/sram.c` | 65 | 692 |
| `src/models/display7.c` | 60 | 375 |
| `src/timer_rc.c` | 52 | 427 |
| `src/models/timer.c` | 51 | 428 |
| `src/models/transistor.c` | 48 | 368 |
| `src/models/powerswitch.c` | 48 | 462 |
| `src/models/lcd.c` | 40 | 264 |
| `src/models/dekatron.c` | 40 | 330 |
| `src/models/display_out.c` | 37 | 442 |
| `src/models/resistor.c` | 31 | 206 |
| `src/models/nixie.c` | 31 | 228 |
| `src/models/diode.c` | 30 | 207 |
| `src/models/display_ccsi.c` | 29 | 204 |
| `src/models/connector.c` | 23 | 218 |
| `src/models/buzzer.c` | 22 | 143 |
| `src/models/opamp.c` | 21 | 158 |
| `src/models/switch.c` | 20 | 250 |
| `src/models/crystal.c` | 17 | 147 |
| `src/models/oscillator.c` | 16 | 143 |
| `src/models/display_clock.c` | 15 | 217 |
| `src/models/terminals.c` | 14 | 104 |
| `src/models/capacitor.c` | 14 | 110 |
| `src/models/fuse.c` | 14 | 137 |
| `src/models/inductor.c` | 14 | 91 |
| `src/models/generic.c` | 9 | 29 |
| `src/models/led.c` | 8 | 81 |
| `src/models/header.c` | 4 | 38 |
| `src/models/socket.c` | 4 | 84 |
| `src/models/supply.c` | 3 | 40 |
| `src/models/model_pins.c` | 2 | 15 |
| **Total** | **1264** | **10796** |

Where the tests go: The model's own program (`serialmem_test`/`serialmem_flash_test`, `eeprom_test`, `sram_test`, `relay_test`, `regulator_test`, `transistor_test`, `powerswitch_test`, `resistor_test`, `diode_test`, `connector_test`, `buzzer_test`, `switch_test`/`slide_test`/`tactile_test`, `crystal_test`, `inductor_test`, `fuse_test`, `capacitor_test`, `generic_test`, `socket_test`, `timer_test`, `opamp_test`, `charlcd_*`, `display7_test`, `lcd_test`, `dekatron_test`, `display_out_test`, `nixie_test`); `oscillator.c`, `terminals.c`, `led.c`, `header.c`, `supply.c`, `model_pins.c` in `circuit_check` or the program of a part that uses them.

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

### Phase 1 — `serialmem.c`, `charlcd.c`, `relay.c`, `eeprom.c`, `regulator.c`, `sram.c`, `display7.c`, `timer_rc.c`

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  Observed: the letter's files went from 1203 uncovered lines (first run,
  `LOGIC_COV_WORK=/tmp/logic-cov-R … --only backend/src`) to 138; what is left
  is 135 lines that run only when `kind_alloc` / `kind_strdup` (or
  `contacts_new`, which calls it) returns NULL, left for the parent's
  `kind_arena_chunk` sweep in oom_test (not set by any test here), one
  `logic_realloc` failure (timer_rc.c:202-203, add_link growing for a pull
  link), and display_out.c:568-569 (see Corrections: a loader gap in
  converter_load.c, another letter's file). New programs: kind_reject_cov_test
  (276 loader cases as data in tests/fixtures/kind_reject/), model_pins_cov_test,
  display_cov_test, tlc6983_cov_test, sram_cov_test, power_cov_test,
  charlcd_cov_test, eeprom_cov_test, serialmem_cov_test, pcf1179c_cov_test,
  display_out_cov_test (fixtures in tests/fixtures/model_cov/).
  `ctest --test-dir /tmp/p68-build-R` → "100% tests passed, 0 tests failed out of 163".
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 39 coverage-ratchet rows removed.
- [x] Ratchet rows removed for each file at 0.
  Not done here: the letter does not edit scripts/coverage-ratchet.tsv (the
  parent writes it once the kind_alloc paths are covered).
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 39 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `timer.c`, `transistor.c`, `powerswitch.c`, `lcd.c`, `dekatron.c`, `display_out.c`, `resistor.c`, `nixie.c`, `diode.c`, `display_ccsi.c`, `connector.c`, `buzzer.c`, `opamp.c`, `switch.c`, `crystal.c`, `oscillator.c`, `display_clock.c`, `terminals.c`, `capacitor.c`, `fuse.c`, `inductor.c`, `generic.c`, `led.c`, `header.c`, `socket.c`, `supply.c`, `model_pins.c`

- [x] As Phase 1, for these files (the same runs and programs; the same three
  kinds of leftover lines). `node scripts/source-size.js --check` → exit 0;
  every new test program is under 400 lines.
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 39 coverage-ratchet rows removed.

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

- **TLC6983 chip-index read (model bug, fixed).** display_ccsi.c decoded a
  read only for IDs `AA6x` (`(id & 0xF0) == 0x60`), so the chip-index read
  `AA70h` its own comment lists was never answered. The datasheet's register
  map (tlc6983.pdf § 8.7, Table 8-5: "Chip Index R/W AA10h / AA70h") has it.
  Fixed (`ID_INDEX_READ`); regression: tlc6983_cov_test "after CHIP_INDEX the
  one chip on the chain reads index 0 (AA70)". No golden line changes
  (trace_check and listing_check pass).
- **TLC6983 scan after SCAN_NUM changes (behavior change).** A VSYNC that
  applied a smaller SCAN_NUM left a line past the new count on until the next
  scan step; display_out.c's CCSI pin path now re-applies the outputs when
  the line count changes, so the scan restarts within the new count at once
  (and apply's slot clamp is reachable). Regression: tlc6983_cov_test "after
  SCAN_NUM 1 at VSYNC only LINE0 and LINE1 are scanned".
- **Contract fix: inductor_fixed_contact / fuse_fixed_contact** answered for
  any part (model_pins.h: "0 … for a part of another kind"); each now checks
  its kind (a fuse passed to the inductor call read the wrong bound). Test:
  model_pins_cov_test.
- **Loader gap, not fixed (another letter's file).** converter_load.c checks
  "action lcdload needs an lcd display with a load section" only inside
  `if (dev->disp != NULL)`, so a device with an `lcdload` command and no
  "display" loads, and disp_lcd_byte's `c == NULL` guard
  (display_out.c:568-569) is the only thing between it and a NULL
  dereference. Moving the check out of that `if` makes the whole guard
  unreachable (the loader then guarantees an lcd display with a load).
- **Dead code removed / restructured (callers read).** connector_contact_pins
  (no caller then; the parent has since re-added it with a voltage.c caller);
  ccsi_frames_dropped / ccsi_state and clock_time (no caller); the dropped
  counter. Unreachable checks, each with the reason in a comment:
  generic_bind_pinout's empty pinout (lib_load_pinout refuses it first);
  crystal_create / osc_create hz == 0 (the loaders refuse it); diode's link
  capacity and resistor's tie capacity (counting bound in the comment);
  terminals_contact(NULL) (callers pass a bound TerminalPins); store / fetch
  range checks in sram.c and read_low_active's missing pin (every caller
  passes a known address / a bound pin); the NULL checks in sram_destroy,
  eeprom_destroy and sm_destroy (the engine destroys only created states);
  eeprom spoil_last's valid check (its only caller checked it); relay
  first_pin(NULL) (coil names are required) and relay_timer's channel check;
  the timer-id range checks in regulator, powerswitch, transistor and timer
  (each schedules only its own k < count); timer_rc analyze's section check;
  regulator's "fixed and adjustable … as arrays" error (a scalar vout is every
  element's, so an earlier check refuses it); display7 expand_side's
  found == 0 (bind checked every pin first); charlcd's status "RAM unknown,
  AC known" bit (lose() makes both unknown); serialmem's zero-duration
  EEPROM branch (an EEPROM's writeCycle is required above 0), sm_timer's
  busy / passive guard (no operation starts while busy) and apply_pending's
  P_NONE case (now an if-chain); display_out's DISP_CLOCK slot clamp (a clock
  scans all its backplanes) and disp_timer's NULL guard; display_clock's
  timer-kind check (tid() makes only the three kinds).
  Restructured only for an llvm-cov zero-count artifact on a reachable line:
  relay's pole binding (moved to bind_poles), charlcd's cursor/display shift
  (moved to cursor_shift), display_out's ASCII7 case.
- **Expectations corrected while writing tests** (the model was right): an X
  on SI is a data bit, not an X transaction (serialmem's transaction X comes
  from CS# / SCK / HOLD#, serial_target.c); the ICM7244 font's code for 'a'
  (61h) is 21h, '!' (six bits, not upper-casing).
- **(parent) The loader gap is fixed.** converter_load.c now refuses an `lcdload` on any device that lacks an lcd display with a load section (regression: converter_load_cov_test "lcdload without a display", plan-68-M's Corrections), so `disp_lcd_byte`'s guard (display_out.c:568-569) is unreachable and is gone.
- **(parent) The last lines:** oom_test (plan-68-L Phase 3) now sweeps a generic part file (generic.c's `kind_alloc`), a socket part with a `finish` (socket.c), and a timer whose first RC link is a pull-up (timer_rc.c:202, the link list's first growth).
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
