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

- [ ] Re-run `COV --backend --only` for each file; close every line per § 3.
- [ ] Ratchet rows removed for each file at 0.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `timer.c`, `transistor.c`, `powerswitch.c`, `lcd.c`, `dekatron.c`, `display_out.c`, `resistor.c`, `nixie.c`, `diode.c`, `display_ccsi.c`, `connector.c`, `buzzer.c`, `opamp.c`, `switch.c`, `crystal.c`, `oscillator.c`, `display_clock.c`, `terminals.c`, `capacitor.c`, `fuse.c`, `inductor.c`, `generic.c`, `led.c`, `header.c`, `socket.c`, `supply.c`, `model_pins.c`

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
