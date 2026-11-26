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

- [ ] Re-run `COV --backend --only` for each file; close every line per § 3.
- [ ] Ratchet rows removed for each file at 0.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `buffer.c`, `shift_special.c`, `buffer_reg.c`, `gate.c`, `jkff.c`, `dff.c`, `register.c`, `arith.c`, `arith_alu.c`, `decoder.c`, `seq.c`, `mux_stored.c`, `mux.c`

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
