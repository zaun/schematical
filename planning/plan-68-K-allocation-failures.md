# plan-68-K: Backend — every allocation can fail, and every failure path is tested

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-J

A large share of the backend's uncovered lines are what happens when memory
runs out: `if (s == NULL) { … return NULL; }` after a `calloc`, and the
`"out of memory"` errors after it. No test can reach them, because nothing can
make `malloc` fail. This letter routes every backend allocation through one
small wrapper that a test can tell to fail the *k*-th allocation, and adds a
sweep test that, for every public entry point that allocates, fails each of
its allocations in turn and checks the call fails cleanly. That covers the
out-of-memory paths everywhere at once, before the per-area coverage letters
(L–S) take on the rest.

Prerequisites: plan-68-A's table, and plan-68-J complete (`node
scripts/source-size.js | grep -c backend/` → 0).

References:

- `backend/src/**/*.c` (the 194 allocation calls, § 2), `backend/src/third_party/cjson/cJSON.h` (`cJSON_InitHooks`).
- `backend/include/parts.h` (`part_library_create`, `load_parts`, `load_footprints`, `load_datasheets`, `load_part_file`), `backend/include/circuit.h` (`circuit_create`, `circuit_add_part`, …), `backend/include/memory_image.h`.
- `backend/tests/support/read_file.c` (`load_parts_dir`, `load_datasheets_file`).

## 1. Goal

- No `malloc`, `calloc`, `realloc` or `strdup` call is left in `backend/src`
  outside `third_party/` and the wrapper itself; cJSON allocates through the
  wrapper too.
- `oom_test` (new) runs, for every entry point in § 4, the operation once per
  allocation with that allocation failing, and passes: each run returns the
  entry point's failure value, sets its error text where it has one, and
  frees everything it allocated (checked by the wrapper's live-allocation
  count returning to its value before the call).
- `COV --backend`: no uncovered line remains on an out-of-memory path (§ 5's
  check), and the ratchet rows are lowered to the new numbers.

### Non-goals

- No behavior change when memory does not run out: `CT`'s goldens unchanged.
- The app does not call the failure hooks; in the app the wrapper is a
  pass-through (one predictable branch per allocation).

## 2. Current State

- Allocation calls: 194 (`grep -rnoE '\b(malloc|calloc|realloc|strdup)\('
  backend/src --include='*.c' | grep -v third_party | wc -l`, at
  `e2f6811a`; plan-68-I/E only moved them between files).
- Uncovered lines next to a failed allocation, at `e2f6811a`: 50 uncovered
  `return NULL;` directly after `if (s == NULL) {`, plus `b`, `device`,
  `bound`, `d`, `bus`, `state`, `list` variants; 30 uncovered lines contain
  `out of memory` (`grep -cE '^ *[0-9]+\| *0\|.*out of memory'` over the
  `llvm-cov show` of `backend/src`, script in `planning/plan-68-data/show.sh`).
- cJSON uses its default `malloc`/`free` (`grep -rn cJSON_InitHooks backend/src
  | grep -v third_party` → no output).

## 3. Design: the wrapper

`backend/src/logic_alloc.{c,h}` (internal; tests include `src/` already):

```c
void *logic_malloc(size_t n);
void *logic_calloc(size_t count, size_t size);
void *logic_realloc(void *p, size_t n);
char *logic_strdup(const char *s);
void  logic_free(void *p);

/* Tests only. */
void logic_alloc_fail_at(long k);   /* the k-th allocation from now fails (k >= 1); 0 = never */
long logic_alloc_count(void);       /* allocations made so far */
long logic_alloc_live(void);        /* allocations not yet freed */
```

`logic_free` exists so the live count is exact; every `free(` of memory that
came from the wrapper becomes `logic_free(` (the same edit pass, by file).
`logic_alloc_init()` installs `cJSON_InitHooks({logic_malloc, logic_free})`
and is called by `part_library_create` and `circuit_create` (idempotent, a
static flag). The counters are `_Atomic long` with relaxed increments:
the app may use the backend from more than one thread, and a plain `static`
counter would be a data race there even though only tests read it.

## 4. Design: the sweep (`tests/oom_test.c`)

For an operation `op`:

```
for k = 1 .. :
    live0 = logic_alloc_live(); logic_alloc_fail_at(k)
    result = op()
    logic_alloc_fail_at(0)
    if op did not reach its k-th allocation: expect success, stop
    expect failure value, error text set (where the API has one),
    expect logic_alloc_live() == live0
```

Operations, each on the smallest input that reaches every loader branch it
has:

- `part_library_create`; `load_footprints` (the real `footprints.json`);
  `load_datasheets` (the real `datasheets.json`).
- `load_parts`, once per kind: for each `Parts/<kind>.json` the test builds a
  one-device file (the file's `devices[0]` with its `parts[0]`) with cJSON and
  sweeps loading that text into a library that already holds the footprints
  and datasheets. 49 kinds (`grep -c '^extern const PartKindOps'
  backend/src/library/kinds.c` → 49).
- `load_part_file` on `part_file_test.c`'s `TEXT` single-part file.
- `circuit_create`, then for each kind `circuit_add_part` of that one part,
  then one `circuit_run` step (the models' create and start paths).
- `memory_image` load of a 16-byte Intel HEX file.

A crash, a leak (live count not back) or a wrong return the sweep finds is a
bug in that loader; it is fixed in this letter with the sweep as its test,
and listed under Corrections.

### Rejected alternatives

- **`-Wl,-wrap,malloc` or interposing `malloc`** — macOS's linker has no
  `-wrap`, and interposing catches the C library's and the test harness's own
  allocations too, so *k* would not mean "the k-th backend allocation".
- **Deleting the NULL checks** ("allocation never fails") — the owner ruled
  out exclusion lists, and the app is a long-running process where a failed
  allocation must not crash the simulation.

### Where the risk is

The `free` → `logic_free` pass: a pointer from cJSON or from the C library
(`fopen`, `strdup` in a third-party file) freed with `logic_free` would make
the live count go negative. The sweep's live-count check would catch the
imbalance only on the paths it runs; so the wrapper also checks that
`logic_alloc_live()` never goes below zero, and every test program asserts
`logic_alloc_live() == 0` after destroying its library (a one-line addition
to each test's ending, through `tests/support`).

## 5. Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — the wrapper

- [x] `backend/src/logic_alloc.{c,h}` (§ 3); in CMake.
- [x] Replace the 194 calls, and the `free(` calls on what they return (256
  `free(` calls in `backend/src` outside `third_party`: `grep -rnoE '\bfree\('
  backend/src --include='*.c' | grep -v third_party | wc -l`), file by file;
  the cJSON hooks.
- [x] `tests/support`: every test program's ending asserts `logic_alloc_live()
  == 0` after its last destroy.
- [x] `tests/logic_alloc_test.c`: the counters, `fail_at`, the negative-live
  guard.

Acceptance: nothing allocates around the wrapper, nothing changed behavior,
nothing leaks in the existing tests.
  Check: `grep -rnE '\b(malloc|calloc|realloc|strdup)\(' backend/src
  --include='*.c' | grep -v -e third_party -e logic_alloc.c` → no output; `CT`
  → all pass, goldens unchanged (est. 2 min).
  Observed: the grep → no output (0 lines); `CT` → `100% tests passed, 0
  tests failed out of 106` (with `logic_alloc_test`); `git status --short
  backend/tests/golden` → no output. Every test program now also fails if it
  leaves backend memory: three did (Corrections), and were fixed.
Commit: —

### Phase 2 — the sweep

- [ ] `tests/oom_test.c` (§ 4) and its CMake line.
- [ ] Fix each bug the sweep finds (Corrections lists each).
- [ ] `COV --backend --write-ratchet`; commit the lowered rows.

Acceptance: every out-of-memory path is run.
  Check: `CT -R oom_test` → pass (its runtime recorded here when it runs);
  over `bash planning/plan-68-data/show.sh <backend/src files>`, `grep -cE
  '^ *[0-9]+\| *0\|.*(out of memory|logic_(m|c|re)alloc|logic_strdup)'` → 0,
  and no uncovered `return NULL;`/`return -1;` line directly follows an
  uncovered `if (<x> == NULL) {` whose `<x>` was assigned from a
  `logic_*alloc` on the line before (the script
  `planning/plan-68-data/oom-lines.js`, written in this phase, prints them;
  it must print nothing) (est. 3 min).
Commit: —

## Validation Plan

- Tests: `logic_alloc_test`, `oom_test`; the live-count assertion in every
  test program.
- Coverage: the ratchet rows only go down; `COV --backend --check` → exit 0.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`; the
  app build (plan-68-I Phase 2) (est. 6 min).

## Decisions

- (author) A counting wrapper with fail-at-*k*, not linker interposition (§ 4).

## Corrections

- **Memory the backend hands to its caller is disowned, not counted.** The
  `char **error` messages of `load_*` and `memory_image` are freed by the
  caller with plain `free` (the public headers say so), which the wrapper
  cannot see; `logic_alloc_disown(p)` takes them out of the live count where
  they leave (`lib_report`, the four `lib_copy_message` assignments,
  `memory_image.c`). The public API is unchanged.
- **The live check is an `atexit` hook in every test program**
  (`tests/support/alloc_check.c`, added to `logic_test`'s sources), not a
  line in each `main`: it covers the programs whose `main` returns early.
  `logic_alloc_test` builds without it, since it frees foreign memory on
  purpose. The negative-live guard counts such frees (`logic_alloc_unmatched`)
  instead of aborting, and the hook fails the program when it is not 0.
- **The check found three test-side leaks**, none in the backend:
  `erc_pins_check` never destroyed its library (1,164 allocations; `main`
  now destroys it after the check), `library_logic_check` freed an internal
  `KindContext.error` with `free` (now `logic_free`), and `serial_bench.c`
  freed `cJSON_PrintUnformatted`'s text with `free` (now `cJSON_free`).
  Found with `planning/plan-68-data/trace_leak.sh`, which prints the live
  count after each call in a test's `main`.
- **There are no `strdup` calls** in the backend (the 194 were 117 `calloc`,
  66 `malloc`, 11 `realloc`); `logic_strdup` exists for the tests and the
  sweep's own use.

## Summary

A mechanical rename (194 calls) plus one systematic test. The rename is
guarded by the goldens and the live-allocation assertion in every test; the
sweep may find real cleanup bugs on failure paths, which it then keeps fixed.
