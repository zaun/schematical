# plan-68-L: Backend coverage — library, engine, circuit, ERC, voltage

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-K

The part library loader, the event engine, the circuit API, the ERC checks, the voltage checks and the memory-image reader. Every line of these files ends up run by a test that checks what the
line did. 673 lines are uncovered in them today (11 files, 5975
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-K complete (`ls backend/tests/oom_test.c` → the file, and its phase ticked).

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
letter L). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/library/library.c` | 196 | 1470 |
| `src/engine/sim.c` | 138 | 947 |
| `src/circuit.c` | 117 | 825 |
| `src/library/footprints.c` | 110 | 876 |
| `src/voltage.c` | 42 | 849 |
| `src/library/kind_util.c` | 32 | 361 |
| `src/engine/net.c` | 16 | 121 |
| `src/erc_check.c` | 10 | 164 |
| `src/memory_image.c` | 6 | 193 |
| `src/library/kinds.c` | 4 | 61 |
| `src/erc_pins.c` | 2 | 108 |
| **Total** | **673** | **5975** |

Where the tests go: Loader rejections go in the `library_*_check` program for the key (plan-68-H § 3); engine paths in `engine_test`; circuit API paths in `circuit_check`/`circuit_adc_check`; ERC in `erc_test`; voltage in `voltage_test`; `memory_image.c` in `memory_image_test`.

## 3. Design: the method

This is the method every backend coverage letter (L–S) uses; letters M–S
point here.

1. **List.** `node scripts/coverage.js --backend --only <path>` prints every
   uncovered `path:line` of a file. Work file by file, largest first.
2. **Classify each uncovered block** and close it the one way that fits:
   - *a loader rejection* (a `kind_error(…)` / `return -1` in a loader): one
     rejection case per distinct message, `expect_fail(<name>, <json>,
     <message>)`, in the program named in § 2 for the file. The JSON is the
     smallest device that reaches the line.
   - *a model state no bench drives* (an X input, a timing violation, a mode
     bit, a register no case writes): a bench case in the file's program that
     drives the state and checks what the datasheet section in the model's
     header comment says happens (pin level, register value, `LOST`).
   - *an API misuse path* (NULL argument, out-of-range index, wrong kind):
     a direct call in the file's program checking the documented return.
   - *unreachable*: a condition no caller can make true. Delete it, or
     restructure so that what remains is reachable (an exhaustive `switch`
     without a `default:` on an enum the compiler checks with `-Wswitch`; a
     guard moved to the one caller that needs it). The commit message names
     the callers read and why the condition cannot hold. There is no
     exclusion list (owner).
3. **Every new test asserts an observable result.** A case that only runs a
   line without checking what it did is not accepted: it would pass with the
   line deleted.
4. **Where tests go.** Into the program § 2 names. A program that would pass
   800 code lines gets a sibling `<program>_more_test.c` with its own `main`,
   using the program's bench in `tests/support/`.
5. **Ratchet.** When a file reaches 0 uncovered lines, its row leaves
   `scripts/coverage-ratchet.tsv` in the same commit (`COV --backend
   --write-ratchet`, then check the diff only removes or lowers rows).
6. **A test that finds a bug** (the model disagrees with the datasheet): fix
   the model in the same letter, the new test is its regression test, and the
   golden line for an affected part is regenerated per `AGENTS.md` §
   Datasheets and `docs/backend.md` § Tests, with the diff reviewed and listed
   under Corrections.

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

### Phase 1 — `library.c`, `sim.c`, `circuit.c`

- [ ] Re-run `COV --backend --only` for each file; close every line per § 3.
- [ ] Ratchet rows removed for each file at 0.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `footprints.c`, `voltage.c`, `kind_util.c`, `net.c`, `erc_check.c`, `memory_image.c`, `kinds.c`, `erc_pins.c`

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
