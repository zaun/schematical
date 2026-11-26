# plan-68-H: Backend — the other nine over-limit C test files split

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-G

Nine more backend test files are over 1000 code lines. This letter splits each
into test programs by the area they test, with shared benches in
`tests/support/`, and moves the three big JSON fixtures that are written as C
string literals into `.json` files under `tests/fixtures/`. Every case still
runs with the same name and checks; nothing in `backend/src` changes.

Prerequisites: plan-68-A's table, and plan-68-G complete
(`ls backend/tests/serial_test.c` → no such file).

References:

- The nine files below; `backend/tests/support/` (benches and `*_fixture.h`
  precedents); `backend/tests/fixtures/serial/` and `serial_test.c`'s old
  `read_fixture` (the precedent for JSON fixtures read through `GOLDEN_DIR`,
  now `tests/support/serial_bench.c` after plan-68-G).
- `backend/CMakeLists.txt` (`logic_test`); `docs/backend.md` § Tests.
- `planning/plan-68-data/test-output-before/<program>.txt` — each program's
  output at `e2f6811a`.

## 1. Goal

- `library_check.c`, `cpu_test.c`, `counter_test.c`, `charlcd_test.c`,
  `shift_test.c`, `pio_test.c`, `circuit_check.c`, `serialmem_test.c`,
  `latch_test.c` are each split per § 3; no file in `backend/tests/` is over
  800 code lines.
- For each old program, the sorted case lines of its new programs equal the
  sorted case lines of its recorded output.
- The nine rows are gone from `scripts/source-size-ratchet.tsv`; `SZ` passes.

### Non-goals

- No case added, removed, renamed or changed. `backend/src` untouched;
  `COV --backend --check` is unchanged.
- The fixture JSON's content does not change: each new `.json` file is the
  string literal's value, byte for byte.

## 2. Current State

Measured (`node planning/plan-68-data/sections.js <file>`), code lines:

| File | Code | Sections (code) |
|---|---|---|
| `library_check.c` | 3,307 | (top: `failures`, `json`, `load_fixture_footprints`, `expect_fail`, `load`, and 11 `check_*`) 603; Manufacturer and datasheet keys 119; plan-23-A header kind 77; `check_footprint_choices` … `check_logic_family` 608; `check_supply_cases` … `check_terminal_connector` 486; `check_sram_cases` … `check_ball_cases` 408; Footprints 586; header footprint style 109; Voltage keys 311 |
| `cpu_test.c` | 2,617 | top 22; Fixtures 114; The bench 321; 5.1 Both cores 310; 5.2 W65C02S 146; 5.3 NMOS 18; Z80 bench 358; 5.1 Z80 cases 357; Whole-system boards 45; plan-14-H W65C816S 760; W65C02S I/O board 166 |
| `counter_test.c` | 2,580 | top 921 (of which `FIXTURE` and `FIXTURE_FOOTPRINTS`, two JSON string literals); Bench helpers 92; 74_161 95; 74_190 102; 74_561 163; 74_590 223; CD4000 157; 4522B 40; ripple 97; Loader 32; johnson 178; seg7 108; divn 87; rate 122; plan-10-E loader 42; real parts 121 |
| `charlcd_test.c` | 2,182 | top 20; Fixture 120; Case 20 loader 81; plan-35-D case 1 loader 109; Character ROM 43; Bus helpers 173; Image checks 57; Cases 1–13 302; Cases 14–16 186; plan-35-D 2–9 331; Cases 17–18 60; Case 19 22; plan-35-E 4.1 138; exec/facts 48; CPU board 217; 20x4/40x4 programs 174; main 101 |
| `shift_test.c` | 1,944 | top 638 (the `FIXTURE`/`FIXTURE_FOOTPRINTS` literals); Bench 93; 164/264/165/166 78; 194/195 65; 299/323 59; CD4000 99; 595… 125; TPIC 60; 4094 34; ls673/674 145; 4517B/4034B 76; FIFO 108; SN74LV8153 102; Loader and voltage 51; real parts 12; high-voltage drivers 199 |
| `pio_test.c` | 1,493 | top 18; test component 84; Bench 61; A bus65 234; B pia6521 314; C via6522 463; D via6522 shift register 319 |
| `circuit_check.c` | 1,278 | top (helpers) 133; plan-12-C terminal API 184; plan-12-H 8251A sync 418; plan-24-G ADC read by a CPU 543 |
| `serialmem_test.c` | 1,264 | top 19; Fixture devices 71; SPI master 135; E40 264; E320 64; F-RAM 71; Flash 301; BP0/BPL 61; Facts… 72; Random pins 29; Loader rejections 177 |
| `latch_test.c` | 1,151 | top 333 (the `FIXTURE`/`FIXTURE_FOOTPRINTS` literals); Bench 68; transparent 209; addressable 123; sr 72; bcd7seg 56; bus-mux 50; real parts 144; loader and voltage 96 |

## 3. Design

Same pattern as plan-68-G: shared helpers to `tests/support/<area>_bench.{c,h}`
(non-`static`, compiled only into that area's programs with a
`target_sources` line), one program per area, each `main` running its cases
in the old order with the old ending.

JSON fixtures: `counter_test`, `shift_test` and `latch_test`'s `FIXTURE` and
`FIXTURE_FOOTPRINTS` become `tests/fixtures/<area>/parts.json` and
`footprints.json`, read with a `read_test_fixture(area, file)` added to
`tests/support/read_file.c` (the serial bench's `read_fixture`, generalized;
the serial bench then calls it too).

| Old file | New programs (sections) | Support |
|---|---|---|
| `library_check.c` | `library_check` (top's checks, manufacturer keys, header kind); `library_logic_check` (`check_footprint_choices` … `check_logic_family`); `library_parts_check` (`check_supply_cases` … `check_terminal_connector`); `library_pins_check` (`check_sram_cases` … `check_ball_cases`); `library_footprint_check` (Footprints, header footprint style); `library_voltage_check` (Voltage keys) | `library_check_bench.{c,h}`: `failures`, `json`, `load_fixture_footprints`, `expect_fail`, `load` |
| `cpu_test.c` | `cpu_test` (5.1, 5.2, 5.3, whole-system boards); `cpu_z80_test` (Z80 bench, Z80 cases); `cpu_65816_test` (plan-14-H); `cpu_io_board_test` (W65C02S I/O board) | `cpu_bench.{c,h}`: Fixtures, The bench |
| `counter_test.c` | `counter_test` (161, 190, 561, 590, CD4000, Loader); `counter_div_test` (4522B, ripple, johnson, seg7, divn, rate, plan-10-E loader, real parts) | `counter_bench.{c,h}`: Bench helpers and the top's globals; `fixtures/counter/` |
| `charlcd_test.c` | `charlcd_test` (both loader cases, cases 1–19); `charlcd_4row_test` (plan-35-D 2–9, plan-35-E 4.1, exec/facts); `charlcd_board_test` (CPU board, 20x4/40x4 programs) | `charlcd_bench.{c,h}`: Fixture, Character ROM, Bus helpers, Image checks |
| `shift_test.c` | `shift_test` (164 … 4094, Loader and voltage); `shift_special_test` (ls673 … SN74LV8153, real parts, high-voltage drivers) | `shift_bench.{c,h}`: Bench; `fixtures/shift/` |
| `pio_test.c` | `pio_test` (A, B); `pio_via_test` (C, D) | `pio_bench.{c,h}`: test component, Bench |
| `circuit_check.c` | `circuit_check` (plan-12-C, plan-12-H); `circuit_adc_check` (plan-24-G) | `circuit_bench.{c,h}`: top's helpers |
| `serialmem_test.c` | `serialmem_test` (E40, E320, F-RAM, facts, random pins, loader rejections); `serialmem_flash_test` (Flash, BP0/BPL) | `serialmem_bench.{c,h}`: Fixture devices, SPI master |
| `latch_test.c` | `latch_test` (every case section) | `latch_bench.{c,h}`: Bench; `fixtures/latch/` |

Sizes after the split, from the section counts: every program ≤ 795 code
lines (`counter_div_test` is the largest: 40 + 97 + 178 + 108 + 87 + 122 + 42
+ 121 = 795) and every bench ≤ 435 (`cpu_bench`: 114 + 321).

### Rejected alternatives

- **Keeping the JSON fixtures as C literals in headers** — a header holding
  921 lines of string literal is still code by § 3.1 of plan-68-A, and data
  belongs in `.json` beside the existing serial fixtures.

### Where the risk is

A fixture whose C escapes are unescaped wrongly (`\"`, `\\`, `\n`). Phase 1
proves each `.json` equals the literal's value before the literal is deleted.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — the JSON fixtures out of C

- [x] ~~`read_test_fixture` in `tests/support/read_file.{c,h}`; the serial
  bench's `read_fixture` calls it.~~ — moot: `tests/support/read_file.h`
  already has `char *read_file(const char *path)`, and every test program
  has `GOLDEN_DIR` (`logic_test`'s compile definition), so each program reads
  `read_file(GOLDEN_DIR "/../fixtures/<area>/<file>")` directly; a second
  helper would only wrap that one call.
- [x] Write `tests/fixtures/{counter,shift,latch}/{parts,footprints}.json`
  from the literals by a throwaway C program that prints each literal (so the
  compiler does the unescaping); then `counter_test.c`, `shift_test.c`,
  `latch_test.c` read the files and the literals are deleted.

Acceptance: the three programs load the same text and print what they printed.
  Check: before deleting a literal, a temporary `strcmp(literal, file) == 0`
  assertion in each program passes once (then removed); `CT -R
  'counter_test|shift_test|latch_test|serial_'` → pass; each program's sorted
  output equals its `test-output-before` file (est. 2 min).
  Observed: the files were written by compiling each literal into a
  throwaway program that prints it (`planning/plan-68-data/fixtures_out.py`),
  so the compiler did the unescaping; what it printed is the literal after
  the `dequote` every `load` applies (`'` → `"`), which makes the files plain
  JSON and leaves `load`'s dequote a no-op on them (the script asserts no
  `'` is left). The three programs' sorted output equals their
  `test-output-before` files (`diff` → no output, exit 0 each).
Commit: —

### Phase 2 — library_check, cpu_test, counter_test

- [ ] Split per § 3; `backend/CMakeLists.txt` lists the new programs; the
  three ratchet rows removed.

Acceptance: no case lost.
  Check: for each old program `P`, `cat <its new programs' outputs> | grep -E
  '^(ok|PASS|FAIL)' | sort` equals `grep -E '^(ok|PASS|FAIL)'
  planning/plan-68-data/test-output-before/P.txt | sort` — except the one
  final summary line per program (`PASS` alone), which each new program
  prints once (est. 2 min).
Commit: —

### Phase 3 — charlcd, shift, pio, circuit, serialmem, latch

- [ ] Split per § 3; CMake; the six ratchet rows removed; `docs/backend.md`
  § Tests lists every new program.

Acceptance: as Phase 2, for these six; and nothing in `backend/tests/` is
over 800.
  Check: the Phase 2 diff for each; `node scripts/source-size.js --all | grep
  backend/tests/ | awk '$1>800'` → no output; `SZ` → exit 0 (est. 2 min).
Commit: —

## Validation Plan

- Tests: the new programs replace the nine in `ctest`.
- Coverage check: `COV --backend --check` → exit 0, no ratchet row changed.
- Final gate (once, after Phase 3): `CT` → all pass; `SZ` → exit 0;
  `COV --backend --check` → exit 0 (est. 2 min).

## Decisions

- (author) The three C-literal JSON fixtures become `.json` files (§ 3).

## Corrections

(none yet)

## Summary

Test code motion plus three fixtures moved to data files. The case-line diff
per old program proves nothing was lost, and the `strcmp` step proves the
fixtures did not change.
