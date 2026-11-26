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

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  New programs: `library_reject_check`, `library_footprint_more_check`,
  `engine_misuse_test`, `circuit_misuse_test`, `voltage_more_test`,
  `erc_more_test`; oom_test's new sweeps (Corrections).
- [x] Ratchet rows removed for each file at 0.

Acceptance: these files have no uncovered line.
  Observed (measured in a copy of HEAD plus this letter's files, since the
  model letters were mid-edit in the shared checkout): `LOGIC_COV_WORK=…
  node scripts/coverage.js --backend` → no `path:line` under
  `backend/src/library/`, `engine/`, `circuit.c`, `voltage.c`, `erc_*.c`,
  `memory_image.c`; `CT` in the checkout's own build → the programs of this
  letter and every program they touch pass.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `footprints.c`, `voltage.c`, `kind_util.c`, `net.c`, `erc_check.c`, `memory_image.c`, `kinds.c`, `erc_pins.c`

- [x] As Phase 1, for these files (same run: no `path:line`).

Acceptance: as Phase 1, for these files.
  Check: as Phase 1; `SZ` → exit 0 (est. 2 min per run).
Commit: COMMIT_L

### Phase 3 — the chunked arena over every kind's loader

- [ ] After plan-68-M … S land: oom_test sets `kind_arena_chunk` over the
  per-kind `load_parts` sweep too, so every kind's `kind_alloc` NULL paths
  are run; each crash or leak it finds is fixed (the converter loader's
  `cvl_name_id` is one, Corrections), and the lines the model letters left
  as "kind_alloc NULL paths left for the parent" are gone.

Acceptance: no `kind_alloc`/`kind_strdup` NULL check is uncovered anywhere
in `backend/src`.
  Check: `node scripts/coverage.js --backend` → none of the listed lines is a
  `return` after a NULL `kind_alloc`/`kind_strdup`; oom_test passes.
Commit: —

## Validation Plan

- Tests: the cases added to the programs in § 2 (and any `_more_test`).
- Coverage check: `COV --backend --check` → exit 0 with every § 2 row gone.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`
  (est. 3 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.

## Corrections

- **The table's numbers were before plan-68-K**; the list measured at the
  start (`node scripts/coverage.js --backend` after K) was 461 lines in
  these files, split pieces included (`library_load.c`,
  `library_load_device.c`, `sim_contacts.c`).
- **Out-of-memory paths in the loaders' arena** (`kind_alloc`/`kind_strdup`
  returning NULL) were not reached by plan-68-K's sweep: the arena takes
  64 KiB chunks, so an allocation fails only at a chunk's start. A test-only
  knob, `kind_arena_chunk` (`src/library/kind.h`), set to 16 gives every
  `kind_alloc` a chunk of its own; oom_test sets it around the footprint,
  datasheet and part-file sweeps and a `load_parts` sweep of six kinds.
  With it set over every kind's loader, the converter loader crashed
  (`cvl_name_id` on a NULL from `cvl_pin_member`) — a bug in plan-68-M's
  files, left to that letter; the knob over all kinds runs once the model
  letters land (plan-68-L's last task).
- **oom_test also sweeps** `part_library_retire_part` and
  `part_library_unload_last_part_file` (their table rebuilds could fail
  unseen), a part file with an inline datasheet, a slide switch resting on
  closed contacts, a tactile switch with unwired joined pins, and the
  17-diode part with its pins wired.
- **Unreachable lines restructured (each caller read):**
  `load_footprint_choices`' NULL guard (its one caller checks first);
  `load_part_file`'s "`devices` must hold exactly one part" (one device whose
  `parts` holds one part, checked above, always adds one); `engine_failed`'s
  no-error branch (every caller runs after an engine call failed, and the
  engine sets its error then); `circuit_net_value`'s and `voltage_describe`'s
  returns after a full switch (now `default:`); the `set_switch` result checks
  in `circuit_set_switch_state`, `circuit_set_connector_plug` and
  `circuit_start` (they loop over the model's own positions, which it
  takes); `circuit_shorted_net`'s fall-through (the engine's nets are the
  circuit's); `put_spec` with no mirror (`put_size` now: it cannot fail); the
  last tie-break of `compare_problems` folded into the pin comparison.
- **Line-coverage artifacts:** `snprintf` is a macro, so a call spread over
  lines counted only its first line, and a closing brace after a `return`
  never counts; `voltage_describe`'s texts are now one line each (the tests
  check each text exactly, unchanged).
- **A bug the tests found:** the voltage check joins nets only through switch
  contacts (`switch_contact_pins` is `PART_SWITCH` only), so a switched
  jack's contact never joined its nets there: pulling the plug of a PJ-002A
  between 5 V and 3.3 V did not stop the run though `circuit_set_connector_plug`
  re-runs the check for that. Fixed with `connector_contact_pins`
  (`connector.c`) used by the voltage check's join; the regression test is
  `circuit_misuse_test`'s `check_unplug_stops`.


## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
