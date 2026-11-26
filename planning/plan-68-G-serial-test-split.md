# plan-68-G: Backend — the serial test files split into test programs

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-A (the backend track; plan-68-A § the dependency graph)

`backend/tests/serial_test.c` (6,408 code lines) is one test program for every
serial part: the 16550 bench, the TI parts, the Maxim SPI/I2C UARTs, the
classic UARTs, the bridges and the Exar parts, with two of its cases pulled in
through `serial_test_d.h` (809) and `serial_test_i.h` (1,151). This letter
splits it into thirteen test programs, one per family, plus shared benches in
`tests/support/`. Every case still runs, with the same name and the same
checks; nothing in `backend/src` changes.

Prerequisites: plan-68-A's table, and plan-68-A complete (`ls
scripts/source-size.js scripts/coverage.js scripts/coverage-ratchet.tsv` →
all three).

References:

- `backend/tests/serial_test.c`, `serial_test_d.h`, `serial_test_i.h`;
  `backend/tests/support/serial_fixture*.h`, `bench.c` (the precedent for a
  shared bench).
- `backend/CMakeLists.txt` (`logic_test`, the `serial_test` entry).
- `docs/backend.md` § Tests (the list of test programs).
- `planning/plan-68-data/sections.js` (per-section code counts used below);
  `planning/plan-68-data/test-output-before/serial_test.txt` (the program's
  output at `e2f6811a`: 115 lines, 113 `ok` lines).

## 1. Goal

- `serial_test.c`, `serial_test_d.h`, `serial_test_i.h` are gone; the
  programs and benches of § 3 replace them, each at or under 800 code lines.
- The sorted `ok`/`FAIL` case lines of the new programs together equal the
  sorted case lines of `test-output-before/serial_test.txt`.
- `SZ` passes with the `serial_test.c` and `serial_test_i.h` rows removed from
  `scripts/source-size-ratchet.tsv`.

### Non-goals

- No case is added, removed, renamed or changed; no check's expected value
  changes. (Coverage work comes in letters N–P, into these new files.)
- `backend/src` is untouched, so `COV --backend --check` gives exactly the
  numbers it gave before (the tests run the same code).

## 2. Current State

`serial_test.c:main` loads every part file plus five fixture files
(`load_parts_dir`, `load_serial_fixture`, `load_new_footprints`,
`footprints_test.json`, `ns16550.json`, `bridges.json`,
`load_serial_fixture_classic`), then calls 37 case functions in order (`awk` over `main` → 37) and
prints `FAIL: <n>` or `PASS`. The helpers (`check`, `case_begin`,
`case_end`, the `DRIVER`/`CLOCK` components, `bench_open_cfg`, `set`, `run`,
`get`, `bus_write`, `bus_read`, the frame helpers) are `static` in the file.
`serial_test_d.h` is included at the "Outcome: the terminal" section and
`serial_test_i.h` at "plan-12-I"; both are case files using those statics.

Measured (`node planning/plan-68-data/sections.js backend/tests/serial_test.c
backend/tests/serial_test_d.h backend/tests/serial_test_i.h`), code lines per
section, in file order:

| Section (`serial_test.c` unless named) | Code |
|---|---|
| (top) + Bench components + The bench | 29 + 39 + 286 |
| 1. Reset values … 12. 16450 | 525 |
| Outcome: the terminal (`case_terminal` 22; the shipped-compare helpers `strip_active_low`, `compare_shipped`, `read_fixture`, `case_shipped`, `case_d24` 149) | 172 |
| plan-12-J head | 150 |
| plan-12-H head | 163 |
| SPI master, I2C master, Line helpers | 29 + 66 + 42 |
| JF, J1 MAX3100, J2 MAX3140 | 224 + 208 + 104 |
| J3–J6 the max310x family, end of plan-12-J | 598 + 17 |
| H1, H2, H3 | 310 + 108 + 186 |
| H4 8251A, H5 82510 | 316 + 385 |
| H6 CDP65C51, plan-14-E W65C51N | 200 + 239 |
| H7 82C52/HD-6406, H8 COM78C80x | 334 + 291 |
| plan-12-E head | 153 |
| E1–E15 | 568 |
| E16–E28 (with `case_host`, the ns16550 host-port case, at the end) | 665 |
| `serial_test_d.h`: Bench configurations | 143 |
| `serial_test_d.h`: D1–D8 | 345 |
| `serial_test_d.h`: D10–D24 | 321 |
| `serial_test_i.h`: (top) | 232 |
| `serial_test_i.h`: I1–I7 | 506 |
| `serial_test_i.h`: I8/I9 SC18IS604 | 413 |

## 3. Design

**Shared benches** go in `tests/support/`, each a `.c` with a `.h`, their
functions made non-`static` with unchanged names. They are compiled into the
serial programs only, through a new CMake function, so no other test program
links names like `set` or `run`:

```cmake
function(logic_serial_test NAME)
    logic_test(${NAME})
    target_sources(${NAME} PRIVATE tests/support/serial_bench.c ${ARGN})
endfunction()
```

| Support file | From |
|---|---|
| `serial_bench.{c,h}` | (top), Bench components, The bench, SPI master, I2C master, Line helpers, and `serial_bench_load()` (the loading block of `main`, unchanged) |
| `serial_bench_d.{c,h}` | `serial_test_d.h` Bench configurations (`extern const BenchConfig …`) |
| `serial_bench_maxim.{c,h}` | plan-12-J head |
| `serial_bench_classic.{c,h}` | plan-12-H head |
| `serial_bench_exar.{c,h}` | plan-12-E head |
| `serial_bench_bridge.{c,h}` | `serial_test_i.h` (top) |

**Programs** (each `main` = `serial_bench_load()`, then its cases in the
order the old `main` called them, then the same `FAIL: <n>`/`PASS` ending):

| Program | Cases | Code (sections) | Extra sources |
|---|---|---|---|
| `serial_ns16550_test` | 1–12, `case_terminal`, `case_host` | 525 + 22 + `case_host` | — |
| `serial_d_test` | D1–D8 (`run_d_cases`' first half) | 345 | `serial_bench_d.c` |
| `serial_d_more_test` | D10–D24, `case_d24`, `case_shipped` | 321 + 149 | `serial_bench_d.c` |
| `serial_maxim_test` | JF SPI/I2C, J1, J2 | 536 | `serial_bench_maxim.c` |
| `serial_max310x_test` | J3–J6 | 615 | `serial_bench_maxim.c` |
| `serial_ay1013_test` | H1 (both), H2, H3 | 604 | `serial_bench_classic.c` |
| `serial_intel_test` | H4, H5 | 701 | `serial_bench_classic.c` |
| `serial_acia_test` | H6, plan-14-E | 439 | `serial_bench_classic.c` |
| `serial_hs_com_test` | H7, H8 | 625 | `serial_bench_classic.c` |
| `serial_bridge_test` | I1–I7 | 506 | `serial_bench_bridge.c` |
| `serial_sc18is604_test` | I8, I9 | 413 | `serial_bench_bridge.c` |
| `serial_exar_test` | E1–E15 | 568 | `serial_bench_exar.c` |
| `serial_exar_v_test` | E16–E28 | 665 minus `case_host` | `serial_bench_exar.c` |

`run_d_cases`, `run_i_cases`, `run_e_cases` and `run_p14_e_cases` become the
case lists of the programs that hold their cases.

### Rejected alternatives

- **One program built from many `.c` files** — keeps one 113-case program
  that `ctest -j` cannot run in parallel, and a failure still names only
  `serial_test`.
- **Shared benches in `logic_test_support`** — that library links into every
  test program, and the bench's short global names (`set`, `run`, `get`)
  would be one `static` slip away from a clash in another program.

### Where the risk is

A case that silently stops running (left out of a `main`). The output-diff
gate catches exactly that.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — the shared benches

- [x] `tests/support/serial_bench.{c,h}` and the five family benches (§ 3),
  moved verbatim from the sections named; `static` dropped on the moved
  functions and data, declarations in the headers.
- [x] `backend/CMakeLists.txt`: `logic_serial_test`; `serial_test` builds
  from `serial_test.c` + the benches (the old program still runs, now on the
  shared benches).

Acceptance: the old program, on the moved benches, prints what it printed.
  Check: `CT -R serial_test` passes, and `/tmp/p68-build/serial_test | sort`
  equals `sort planning/plan-68-data/test-output-before/serial_test.txt`
  (`diff` → no output) (est. 1 min).
  Observed: moot as a separate step — Phase 1 and Phase 2 landed together
  (Corrections); Phase 2's check, which is strictly stronger, passed.
Commit: —

### Phase 2 — the thirteen programs

- [x] Create the thirteen `tests/serial_*_test.c` of § 3, moving each case
  section verbatim; delete `serial_test.c`, `serial_test_d.h`,
  `serial_test_i.h`.
- [x] `backend/CMakeLists.txt`: replace `logic_test(serial_test)` with the
  thirteen `logic_serial_test(...)` lines.
- [x] `scripts/source-size-ratchet.tsv`: remove the `serial_test.c` and
  `serial_test_i.h` rows.
- [x] `docs/backend.md` § Tests: the serial programs listed by family.

Acceptance: every case runs once, in a program of its family, and each new
file is at or under 800 code lines.
  Check: `for t in /tmp/p68-build/serial_*_test; do $t; done | grep -E '^(ok|FAIL) ' | sort`
  equals `grep -E '^(ok|FAIL) ' planning/plan-68-data/test-output-before/serial_test.txt | sort`
  (`diff` → no output); `node scripts/source-size.js --all | grep -E 'tests/(support/)?serial_'`
  → every count ≤ 800; `SZ` → exit 0 (est. 2 min).
  Observed: the sorted 112 `ok` lines of the 13 programs equal the 112 of
  `test-output-before/serial_test.txt` (`diff` → no output), each program
  prints `PASS`; the largest new file is `serial_intel_test.c` at 711 code
  lines; `SZ` → exit 0; `CT` → `100% tests passed, 0 tests failed out of 90`.
Commit: —

## Validation Plan

- Tests: the thirteen programs replace `serial_test` in `ctest`.
- Coverage check: `COV --backend --check` → exit 0 with no ratchet row
  changed (the same production lines are run).
- Final gate (once, after Phase 2): `CT` → all pass; `SZ` → exit 0;
  `COV --backend --check` → exit 0 (est. 2 min).

## Decisions

- (author) One program per family, with shared benches compiled only into the
  serial programs (§ 3, rejected alternatives).

## Corrections

- **Phases 1 and 2 landed as one change.** The split is generated by a
  script (`planning/plan-68-data/split_serial.py`, with `csplit.py`): it
  writes the benches and the thirteen programs from the old files in one
  run, so there was no intermediate `serial_test` on the shared benches.
  Phase 2's gate (every case line, once) is the stronger of the two and is
  the one that ran.
- **The benches differ from § 3 where the code's dependencies said so:**
  - The SPI and I2C masters and the line helpers use the Maxim bench's `MB`
    type, so they are in `serial_bench_maxim`, not `serial_bench`; and the
    JF section's target component (`Tgt`, `TARGET`, the `TG_*` pins) is
    there too, because the SC18IS604 cases (I8, I9) drive it.
  - `serial_bench_bridge` builds on `serial_bench_maxim`, and the exar
    benches on `serial_bench_d` (E cases use the TI bench configurations
    and `program_unit`, `drive_named`, `wait_level`), so those programs link
    both.
  - `h_expect_text` / `h_expect_sent` (in the "end of plan-12-J" section)
    and `AC_XHALF` (in H6) are used by every classic program: they are in
    `serial_bench_classic`. `m3_start_bit` and `within` (J1) are in the
    Maxim bench; `e_motorola_dual` and `e_powersave` (E9) in the Exar bench.
  - `case_d24` re-runs cases 1-12 on the shipped PC16550D, so it is in
    `serial_ns16550_test`, not `serial_d_more_test`.
- **Two names were renamed** because, no longer `static`, they clash:
  `clock_timer` with `backend/src/models/display_clock.c`'s, and `send`
  with the C library's; they are `sb_clock_timer` and `sb_send` (found with
  `nm -g` over `liblogic.a`; no other bench name clashes).

## Summary

Code motion in test code only. The only way to get it wrong is to lose a case,
and the sorted case-line diff against the recorded output proves none was lost.
