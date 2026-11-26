# plan-68-N: Backend coverage — the ns16550 core, ext, LPT and Exar

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-M

The 16550 core (`ns16550.c` and its plan-68-I pieces), the enhanced features (`ns16550_ext.c`), the printer port and the Exar options. Every line of these files ends up run by a test that checks what the
line did. 846 lines are uncovered in them today (5 files, 5966
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-M complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

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
letter N). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/ns16550_exar.c` | 331 | 2451 |
| `src/models/ns16550.c` | 326 | 2381 |
| `src/models/ns16550_ext.c` | 185 | 886 |
| `src/models/ns16550_lpt.c` | 3 | 205 |
| `src/models/ns16550_int.h` | 1 | 43 |
| **Total** | **846** | **5966** |

Where the tests go: Core and ext paths in `serial_ns16550_test`, `serial_d_test`, `serial_d_more_test`; Exar paths in `serial_exar_test`, `serial_exar_v_test`; loader rejections in the same programs' loader cases.

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

### Phase 1 — `ns16550_exar.c`, `ns16550.c`

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  `LOGIC_COV_WORK=/tmp/logic-cov-N node scripts/coverage.js --backend --only backend/src/models/ns16550`
  → for this letter's files only kind_alloc NULL lines print (left for the
  parent's `kind_arena_chunk` sweep, see Corrections):
  `ns16550_exar.c:73-74`, `ns16550_exar_load.c:51-52,99-100,198-199`,
  `ns16550_kind.c:297-298,474-475,582-583`, `ns16550_load.c:328-329,382-383`;
  `ns16550.c`, `ns16550_io.c`, `ns16550_exar_flow.c`, `ns16550_exar_pins.c`,
  `ns16550_core_int.h`, `ns16550_exar_int.h`: no line.
- [x] Ratchet rows removed for each file at 0. Left for the parent (this
  agent does not edit `scripts/coverage-ratchet.tsv`); `ns16550.c`,
  `ns16550_io.c`, `ns16550_exar_flow.c`, `ns16550_exar_pins.c`, `ns16550_lpt.c`,
  `ns16550_ext.c` are at 0; the others reach 0 with the parent's kind_alloc
  sweep.
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 10 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `ns16550_ext.c`, `ns16550_lpt.c`, `ns16550_int.h`

- [x] As Phase 1, for these files. Same command: `ns16550_ext.c`,
  `ns16550_lpt.c`, `ns16550_int.h` print no line. `SZ`
  (`node scripts/source-size.js --check`) → exit 0; new test files at most
  671 code lines (serial_n68_ext_test.c). `CT`
  (`ctest --test-dir /tmp/p68-build-N`) → "100% tests passed, 0 tests failed
  out of 161".

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

- **Where the tests went.** § 2 names `serial_ns16550_test`, `serial_d_test`,
  `serial_d_more_test`, `serial_exar_test`, `serial_exar_v_test`; all five
  were already near 700 lines, so the cases are in new programs (§ 3 step 4):
  `serial_n68_load_test` (every loader rejection: bus, clock, channels,
  device keys, Phase 2 keys, printer port, bind, Exar keys),
  `serial_n68_core_test` (reset X, INT/RXRDY rules, DLAB X, head errors,
  selects, XIN X), `serial_n68_ext_test` (650/752/NS-EFR banks, software and
  hardware flow, IrDA, RS-485, prescaler, DLF, TLR, sleep),
  `serial_n68_exar_test` (650-style Exar flow, CTS/RTS interrupts, sleep,
  MF, 854/864/M752 registers and pins), `serial_n68_5g_test` (XR16V798),
  `serial_n68_bridge_test` (core paths only a bridge reaches), on
  `tests/support/serial_bench_n68.{c,h}` (variants of the ns16550.json /
  bridges.json fixture devices, X writes, the sent log).
- **Model bug fixed (NS16C2552/2752 Table 8, "Interrupt Sources and
  Clearing"):** "Xoff or Special character — Read from IIR register or
  reception of Xon character (or reception of next character if interrupt is
  caused by Special character)"; the EFR[5] text adds "Special character
  interrupts are cleared automatically after the next received character".
  The model cleared the Xoff interrupt only on an Xon and the special one
  only by an IIR read. `ns_ext_iir_read` now clears `iir_xoff` on the IIR
  read for `extended: nsEfr` (TX stays halted until the Xon) and
  `ns_ext_rx_char` clears `iir_special` on the next character for the same
  style. Regression tests: X21 and X8b in `serial_n68_ext_test`. No golden
  line changed (`listing_check`, `trace_check` pass).
- **Unreachable code removed or restructured** (callers read; no behavior
  change, every existing test passes):
  - `ns_lpt_selected` deleted (no caller anywhere in `backend/`).
  - `is_select_pin`, `ns_x_is_select_pin`, `ns_x_pin_changed`: the
    `SERIAL_NO_PIN` guards (the one caller, `ns_pin_changed`, passes a real pin).
  - `nsc_bus_select`: the final `return common` (installed only for devices
    with a channel pin, per-channel selects, a printer port or Exar select
    rules, and `ns_x_select` hands back only the first three).
  - `nsc_bus_read_done` / `nsc_bus_write`: the `unit >= channel_count`
    guards (units past the channels are the printer port and the Exar
    units, handled first); `nsc_bus_peek` / `nsc_bus_read_done` /
    `nsc_bus_write`: the `unknown` guards (the parallel bus is disabled while
    unknown and runs no cycle, the bridges' host port checks `unknown`
    itself; the one read_done that can still come, a read cut short, only
    touches state the clean reset clears).
  - `int_pin_level`: `default` of an exhaustive `NsIntPin` switch;
    `ns_ext_write_reg`: the `R_CORE`/`R_X`/`R_FIFORDY`/`R_DREV`/`R_DVID` arms
    (never passed for a write); `queue_flow_pair` txmode 0 (both callers send
    only with a TX mode); the `auto_flow` switch's empty default;
    `exs_rxrdy_pin`/`exs_txrdy_pin` now call the core's
    `ns_rxrdy_level`/`ns_txrdy_level` (the same rules, duplicated);
    `enhanced()` (XS_NONE's EFR stays 0); `ns_x_peek`/`ns_x_write` trailing
    returns (exhaustive `XReg` switches); `exs_dma_requests` level-0 guard
    (the 864 is 854-style, level >= 1); `sleep_enabled` default;
    `ns_x_thre_at_empty`'s threshold-0 guard (its caller checks it).
  - Brace-only lines llvm-cov counted after a `return` (lpt `data_driven`,
    ext `ns_ext_pins` case 2, the `ns_x_peek` FLVL case, an Exar loader
    block) rewritten without the block.
- **Spec readings the first drafts of tests got wrong (not model bugs):**
  SC16C752B SPR is not reset (X at power-up); TL16C554 shared TXRDY is active
  after reset with every TX empty (P3 covers only the reset itself); on the
  5G an RX-data interrupt (code 001) outranks the timeout code at trigger 1.
- **kind_alloc NULL paths left for the parent** (owner instruction, the
  `kind_arena_chunk` sweep): `ns16550_exar.c:73`, `ns16550_exar_load.c:51,99,198`,
  `ns16550_kind.c:297,474,582`, `ns16550_load.c:328,382`.
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
