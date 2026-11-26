# plan-68-O: Backend coverage — the bridges and serial targets

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-N

The SC16IS bridges and their XR20M1280 variant, the SC18IS604, and the SPI/I2C target front end. Every line of these files ends up run by a test that checks what the
line did. 645 lines are uncovered in them today (4 files, 3620
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-N complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

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
letter O). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/ns16550_bridge.c` | 237 | 1383 |
| `src/models/ns16550_xr1280.c` | 200 | 719 |
| `src/models/serial_sc18is604.c` | 111 | 780 |
| `src/models/serial_target.c` | 97 | 738 |
| **Total** | **645** | **3620** |

Where the tests go: `serial_bridge_test`, `serial_sc18is604_test`; the target front end in `serial_maxim_test` (its JF cases).

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

### Phase 1 — `ns16550_bridge.c`, `ns16550_xr1280.c`

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  Start (after plan-68-K): `ns16550_bridge.c` 128, `ns16550_bridge_kind.c` 107,
  `ns16550_xr1280.c` 200 uncovered. New programs `serial_bridge_more_test`
  (loader rejections, SC16IS/PI7C9X/XR20M1170 decode, GPIO, X, host port,
  flow, IrDA) and `serial_xr1280_more_test`; shared variant/X helpers in
  `tests/support/serial_bench_cov.c`. Check: `LOGIC_COV_WORK=/tmp/logic-cov-O
  node scripts/coverage.js --backend --only backend/src/models/` → no line of
  `ns16550_bridge.c` or `ns16550_xr1280.c`; `ns16550_bridge_kind.c` only
  `:19`-`:20` (the `kind_alloc` NULL return, left for the parent's
  `kind_arena_chunk` oom sweep).
- [x] Ratchet rows removed for each file at 0. Remains: the parent runs
  `COV --backend --write-ratchet` once its `kind_arena_chunk` sweep covers
  `ns16550_bridge_kind.c:19` and `serial_sc18is604.c:939` (the sub-agent brief
  forbids editing the ratchet).
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 5 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: d0605ba6

### Phase 2 — `serial_sc18is604.c`, `serial_target.c`

- [x] As Phase 1, for these files. Start: `serial_sc18is604.c` 107,
  `serial_target.c` 97 uncovered. New programs `serial_sc18is604_more_test`
  (loader, RESET X, unknown commands and registers, count errors, busy,
  I2CClock 0, clock stretching in every wait step, X on SDA) and
  `serial_target_more_test` (bus loader, SPI modes 1/3, SCLK X, hold,
  disable, I2C abandon on X, strap X, R/W X, I2C-only bus, SERIAL_NO_PIN).
  Check (same command) → no line of `serial_target.c`; `serial_sc18is604.c`
  only `:939`-`:940` (`kind_alloc` NULL, left for the parent). `CT` (own
  build dir) → `100% tests passed, 0 tests failed out of 153`; `SZ` → exit 0
  (new test files 348-432 code lines).

Acceptance: as Phase 1, for these files.
  Check: as Phase 1; `SZ` → exit 0 (est. 2 min per run).
Commit: d0605ba6

## Validation Plan

- Tests: the cases added to the programs in § 2 (and any `_more_test`).
- Coverage check: `COV --backend --check` → exit 0 with every § 2 row gone.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`
  (est. 3 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.

## Corrections

- Tests went into new `_more_test` programs (`serial_bridge_more_test`,
  `serial_xr1280_more_test`, `serial_sc18is604_more_test`,
  `serial_target_more_test`), not the § 2 programs: other letters were
  editing the shared files at the same time. They share
  `tests/support/serial_bench_cov.c` (device variants from
  `fixtures/serial/bridges.json`, X-capable SPI/I2C helpers).
- Unreachable code removed or restructured (callers read):
  - `serial_target.c`: `spi_shift`'s `!active` return (its one caller,
    `serial_spi_pin`, calls it only when `active`); `i2c_rising`'s
    `bits >= 8` break (the SCL fall after the 8th bit always leaves
    ADDRESS/WRITE; START resets `bits`; an X on SCL abandons); the idle strap
    sample in `serial_i2c_start` (every caller is a component start, when no
    net is driven yet: `net.c` starts each pin at Z and only events change it,
    so SCL/SDA read X); `serial_host_i2c_address`'s `index >= table_count`
    (`load_i2c` sizes the table 4^straps); `update_mode`'s case block braces
    (a brace-only line after `break`).
  - `ns16550_bridge.c`: the `bridge == NULL` guards of `ns_bridge_decode` and
    `ns_bridge_read_done` (their callers, `decode_any` and the core's read
    path, check `dev->bridge`); `advance`'s channel guard (`host_write` and
    `host_read_done` return first); `ns_bridge_peek_reg`'s non-XR `default`
    (`sc_decode` names only the B_ registers the switch handles). Also the
    SC16IS740 `gpio_count == 0` return in the IOControl write: reachable but
    with no effect anyone can see (IOControl reads X on the 740, its GPIO
    style is NONE so `brs_sc_gpio_pending` returns 0, and it has no GPIO or
    modem pins) — removed; the parent may prefer to keep it.
  - `ns16550_bridge_kind.c`: `ns_bridge_open_drain`'s NO_PIN / NULL guard (its
    caller `ns_open_drain` runs for a bridge device with a pin of the part);
    `ns_bridge_factor`'s case braces.
  - `ns16550_xr1280.c`: the `default:` branches of `xr_peek_reg` and
    `xr_write_reg` (xr_decode names only the X_ registers they handle);
    `X_DLD` and `X_MCR` writes moved to `write_dld` / `write_mcr`, the peek's
    case blocks unbraced (brace-only lines).
  - `serial_sc18is604.c`: `E_IDLE` (only written, never read: stale step
    timers are rejected by the generation) and the `engine_step` default;
    `i2c_command`'s `default: return` (spi_end passes 0x00-0x03 only; 0x03 is
    now the default); `sp_timer`'s `default` (only the four kinds are
    scheduled); the empty `bits % 8` block in `spi_end`; `sp_start`'s
    "RESET low at power-on" branch (RESET reads Z, inactive, at start, as
    above); `sp_line` / `sp_async` (serial.c calls them only for a channel
    below `channels()` = 0; now NULL).
- Model bug found, not fixed (outside this letter's files): XR20M1280 EMSR[4]
  = 1 must 3-state RTS# (plan-12-data/specs/bridges.md § XR20M1280: "EMSR[4]
  3-states TX/RTS#/DTR#"). `xr_rts` returns Z, but `ns16550.c`'s modem
  output loop does `level = or3(level, loop)`, which turns Z into X, so RTS#
  reads X. TX and DTR# are 3-state as they should be. Fix belongs in
  `ns16550.c` (skip the `or3` when the level is Z); the assertion is left out
  of `serial_xr1280_more_test` (case_rs485 says why) until it lands.
- No golden line changes: no model behavior was changed.
- **(parent) The SC16IS740 IOControl return stays removed:** it has no observable effect (the reasons above).
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
