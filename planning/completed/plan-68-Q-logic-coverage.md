# plan-68-Q: Backend coverage — the logic models

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-P

Counters, shift registers, latches, buffers, gates, flip-flops, registers, arithmetic, decoders, sequencers and multiplexers. Every line of these files ends up run by a test that checks what the
line did. 1048 lines are uncovered in them today (17 files, 10115
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-P complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

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
letter Q). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/counter.c` | 234 | 1571 |
| `src/models/shift.c` | 134 | 982 |
| `src/models/latch.c` | 96 | 761 |
| `src/models/counter_div.c` | 87 | 907 |
| `src/models/buffer.c` | 83 | 725 |
| `src/models/shift_special.c` | 58 | 726 |
| `src/models/buffer_reg.c` | 47 | 286 |
| `src/models/gate.c` | 44 | 442 |
| `src/models/jkff.c` | 43 | 329 |
| `src/models/dff.c` | 40 | 351 |
| `src/models/register.c` | 34 | 350 |
| `src/models/arith.c` | 34 | 641 |
| `src/models/arith_alu.c` | 30 | 319 |
| `src/models/decoder.c` | 30 | 686 |
| `src/models/seq.c` | 21 | 371 |
| `src/models/mux_stored.c` | 17 | 334 |
| `src/models/mux.c` | 16 | 334 |
| **Total** | **1048** | **10115** |

Where the tests go: The model's own program: `counter_test`/`counter_div_test`/`counter_sn74143_test`, `shift_test`/`shift_special_test`, `latch_test`, `buffer_test`, `register_test`, `arith_test`, `decoder_test`, `mux_test`; gate, jkff, dff and seq paths in `library_logic_check` (loader) and `engine_test` (behavior).

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

### Phase 1 — `counter.c`, `shift.c`, `latch.c`, `counter_div.c`

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  Every line is closed except the `kind_alloc` NULL checks (out of memory
  in a loader), which the parent covers with plan-68-K's `kind_arena_chunk`
  knob (owner's split, see Corrections). Observed:
  `LOGIC_COV_WORK=/tmp/logic-cov-Q node scripts/coverage.js --backend --only backend/src/models/`
  → the Phase 1 files list only `counter.c:186-187`,
  `counter_ripple.c:151-152`, `counter_seg7.c:76-77`,
  `counter_sync_load.c:585-586`, `shift.c:614-615`, `shift_load.c:424-425`,
  `latch.c:233-234, 607-608` (each a `kind_alloc` NULL return); 925 lines
  before (was 1048 in § 2). Tests: `counter_sync_cov_test`,
  `counter_div_cov_test`, `shift_cov_test`, `latch_cov_test` (new, on the
  new bench `tests/support/q_cov_bench.c`: one fixture or real device,
  edited by JSON path).
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 22 coverage-ratchet rows removed.
- [x] Ratchet rows removed for each file at 0. Left to the parent: the letter
  brief forbids editing `scripts/coverage-ratchet.tsv`, and the files are
  not at 0 until the `kind_alloc` NULL lines are covered.
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 22 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: e59d7ff1

### Phase 2 — `buffer.c`, `shift_special.c`, `buffer_reg.c`, `gate.c`, `jkff.c`, `dff.c`, `register.c`, `arith.c`, `arith_alu.c`, `decoder.c`, `seq.c`, `mux_stored.c`, `mux.c`

- [x] As Phase 1, for these files. Every line closed but the `kind_alloc`
  NULL checks: `arith.c:384`, `arith_alu.c:415`, `buffer.c:256, 454, 612,
  752, 807, 834, 923`, `buffer_reg.c:204`, `decoder.c:297`, `dff.c:161,
  211`, `jkff.c:130, 179`, `mux_stored.c:355`, `register.c:246`,
  `seq.c:54`, `shift_special.c:347` (with the line after each); `gate.c`,
  `mux.c` and the rest of the list print nothing. Total for the letter's
  files: 54 lines (27 checks) of 959 at the start. Tests:
  `logic_ff_cov_test` (gate, dff, jkff), `shift_special_cov_test`,
  `buffer_cov_test`, `logic_misc_cov_test` (register, arith, decoder, mux).
  `CT` (`ctest --test-dir /tmp/p68-build-Q`) → 100% tests passed, 0 tests
  failed out of 157. `SZ` (`node scripts/source-size.js --check`) → exit 0;
  the new programs are 113-318 lines, the bench 284.
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 22 coverage-ratchet rows removed.

Acceptance: as Phase 1, for these files.
  Check: as Phase 1; `SZ` → exit 0 (est. 2 min per run).
Commit: e59d7ff1

## Validation Plan

- Tests: the cases added to the programs in § 2 (and any `_more_test`).
- Coverage check: `COV --backend --check` → exit 0 with every § 2 row gone.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`
  (est. 3 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.

## Corrections

- **Owner's split (parent, mid-letter):** lines that run only when
  `kind_alloc` / `kind_strdup` returns NULL stay uncovered here and are not
  restructured; the parent covers them with the `kind_arena_chunk` knob
  (src/library/kind.h). They are listed under Phase 1 and Phase 2.
- **Bug fixed (seq.c):** `seq_check_model_keys` checked a device's keys
  against a copy of the library's device keys that lacked `"simulated"`, so
  a counter, latch, shift, decoder, arith, mux or buffer device with
  `"simulated": true` (or false) failed to load with `"simulated" is not a
  key of model "..."`. docs/parts-format.md (the `"simulated"` row) says any
  kind may set it. The list now has it; regression: `logic_misc_cov_test`
  check_simulated_key (one device of each kind). No part file sets the key
  on these kinds, so no golden line changes.
- **Unreachable code removed** (callers read; each condition cannot hold):
  - `counter.h` `CounterModel.destroy` and its call in `counter_destroy`:
    all six models (`sync`, `ripple`, `johnson`, `seg7`, `divn`, `rate`)
    set it NULL.
  - `counter_div.c` `divn_n`'s `default: return -1`: its three callers
    pass a mode `divn_mode` returned above 0 (2, 4, 5, 8, 10).
  - `counter_ripple.c` `ripple_next`: the div5 table and div6 step moved out
    of braced `case` blocks (a closing brace after `return` counted as a
    line never run); same behavior.
  - `gate.c` `gate_value`'s `default:` on the GateOp switch: every one of
    the 8 ops has a case.
  - `shift_load.c` `load_serial`: the serialSelect branch moved out of a
    braced block (same artifact); same behavior.
  - `buffer.c` `buf_drive`'s `o->pin == SEQ_NO_PIN` return: every output
    (tristate y, transceiver and registered a/b) is bound with `kind_pin`.
  - `buffer.c` `buf_dir_name`'s missing-"dir" error: both callers
    (`xcvr_unit`, `reg_load`) call it only when the unit has "dir".
  - `buffer.c` `voltage_b`'s `"inputMaxB" without "vccB"` error: a timing
    entry always has "vccB" (checked just before), and a part's base is its
    entry's, never NULL.
  - `buffer_reg.c` `maybe_load`'s `load == 0` branch: `update_store`, the
    only caller, returns before the loop when load is 0.
  - `register.c` `seq_check_model_keys` call: one model whose only key
    ("groups") is in the kind's device keys, which the library checks first
    (evidence: `logic_misc_cov_test` "units" → `unknown key "units"` from
    the library).
  - `register.c` `edge_of`'s `before == now` return: the engine calls
    `pin_changed` only when a net's value changed (sim_contacts.c).
  - `arith.c` compare eval's "output without a same-named cascade input"
    branch: with no "eq" cascade rows `cmp_load` rejects such an output
    (`has no cascade input of its name`); with eq rows every cascade
    combination matches a row first.
- **Kept, documented:** an 8-bit `adder` with a carry out (9 outputs) is
  rejected by `ARITH_MAX_OUTPUTS` (8) — a model limit, not a datasheet
  rule; no part in the catalog needs it (`logic_misc_cov_test` pins the
  message).
- **(parent) The app's drift test and the new key.** `PartKindKeysTests` read seq.c's `LIBRARY_DEVICE_KEYS` as keys of every sequential kind, so the `"simulated"` fix failed it for mux, latch, arith, shift, decoder, counter, register and buffer. `"simulated"` is a library-wide key (library_load_device.c); the form offers it only where a part may be facts-only, so the test now exempts it (`libraryOnlyKeys`). `XB -only-testing:schematicalTests/PartKindKeysTests` → 2 tests, 0 failures.
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
