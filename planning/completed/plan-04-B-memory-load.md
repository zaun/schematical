# plan-04-B: Backend memory load API and load-file parser

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-04-A (the `eeprom` kind, its model and `eeprom.json`)

plan-04 adds Microchip parallel EEPROMs. plan-04-A made them a part kind
that powers up erased (every byte FF). This sub-plan adds the backend side
of "load a file on start": a circuit call that writes words into a memory
part before the simulation starts, and a parser that turns a load file (raw
binary or Intel HEX) into those words. plan-04-C wires both into the app.

Outcome of B: `circuit_memory_load()` fills an EEPROM before
`circuit_start` and is refused afterwards; `memory_image_parse()` turns a
binary or Intel HEX file into a full memory image (FF where the file has no
data) or a message naming the line and the problem. Both are covered by
CTests; nothing else in the backend changes behavior.

References:

- `docs/backend.md` (API, layout, tests), `backend/include/circuit.h`,
  `backend/src/circuit.c` (`circuit_memory_size`, `circuit_memory_read`:
  the pattern the new call mirrors).
- `backend/src/engine/sim.h:SimComponentType` (the optional
  `memory_size`/`memory_read` hooks; the new hook goes after them).
- `backend/src/models/eeprom.c` (from plan-04-A: `EepromState`, storage
  arrays `value[]`/`known[]`, created erased).
- `planning/completed/plan-04-A-eeprom-kind.md` §Decisions (owner decisions for all of
  plan-04).

## Prerequisites

See plan-04-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-04-A complete | `ls planning/completed/plan-04-A-eeprom-kind.md` → exists | MET (2026-11-01: archived after A's final gate, 703cbfb) |
| EEPROM parts load | `grep -c '"pinout":' schematical/schematical/Parts/eeprom.json` → 174 (corrected from `'"name"'`, which counts pin lines too; plan-04-A Corrections) | MET (2026-11-01: 174) |
| Backend tests green | `cmake -S backend -B /tmp/logic-plan04 && cmake --build /tmp/logic-plan04 -j8 && ctest --test-dir /tmp/logic-plan04` → `100% tests passed` (10 tests after plan-04-A: plan-05's `engine_test` came first) | MET (2026-11-01: 10 of 10, 3.9 s, in `/tmp/logic-land-p04a`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all*
> prerequisites.

## 1. Goal

- `circuit_memory_load(c, part, start, values, count)` returns 0 and the
  words read back through `circuit_memory_read` (all bits known) when called
  on an EEPROM part before `circuit_start`; it returns -1 with the exact
  messages of §4.1 after start, on a non-loadable part (SRAM, gate), past
  the end, and for a value wider than the word.
- `memory_image_parse()` passes every case of §4.3 in the new
  `memory_image_test` CTest.
- `ctest` → `100% tests passed, 0 tests failed out of 11` (10 after
  plan-04-A + `memory_image_test`).

### Non-goals (explicit constraints)

- SRAM gets no load hook: it still powers up all X (owner: the load file is
  an EEPROM feature). `circuit_memory_load` on an SRAM fails.
- No change to any part file, golden or existing model behavior:
  `traces.txt` and `parts.txt` stay byte-identical.
- No file I/O in the backend: the parser takes bytes; the app reads the file
  (it owns the sandbox access, plan-04-C).
- Only 8-bit memories: every EEPROM in plan-04-A is x8 (plan-04-A §2).

## 2. Current State

- `circuit.h` has `circuit_memory_size` and `circuit_memory_read`, both
  dispatching to optional `SimComponentType` hooks (`circuit.c:310-336`).
  Building calls are refused after start through `check_building`
  (`circuit.c`, message `"circuit already started"`).
- A part's model state exists from `circuit_add_part` on:
  `sim_add_instance` calls `type->create(config)` (`sim.c:228`), so a load
  between add and start writes into the same state `start` later runs on.
- `SimComponentType` is initialized positionally everywhere (`SRAM_MODEL`
  in `sram.c` lists 11 fields; the test bench `BENCH_TYPE` in
  `sram_test.c` lists 11). A 12th field added at the end is
  zero-initialized in every existing initializer (C11 §6.7.9), so no other
  model changes.
- plan-04-A's `eeprom.c` creates storage erased (`value` = 0xFF,
  `known` = 0xFF for every word) and implements `memory_size` and
  `memory_read`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `SimComponentType` initializers | 12 today, 14 after plan-04-A (`EEPROM_MODEL`, the `eeprom_test` bench) | `grep -rn 'SimComponentType [A-Za-z_]* = {' backend/src backend/tests \| wc -l` → 12 (2026-11-01); all positional: `grep -rn -A2 'SimComponentType [A-Za-z_]* = {' backend/src backend/tests \| grep -c '\.[a-z_]* *='` → 0. The new trailing field needs an edit only in `EEPROM_MODEL` |
| CTests before this sub-plan | 9 | after plan-04-A (8 today + `eeprom_test`) |

## 3. Design Overview

Two independent pieces:

1. **Load hook + circuit call.** `SimComponentType` gains
   `memory_load`; `eeprom.c` implements it; `circuit_memory_load` checks
   building state, the part, the range and the values, then calls it.
2. **Load-file parser.** A new public header `memory_image.h` and
   `backend/src/memory_image.c`: pure functions, bytes in, words out.

Correctness risk is in the Intel HEX parser (address arithmetic across
record types 02/04, checksums, line endings). It lands with a test of every
rule and every message. The hook is a few lines.

Byte-identity of `traces.txt`/`parts.txt` is a gate here (nothing in this
sub-plan may change a part's behavior); the new behavior is gated by
`circuit_check` and `memory_image_test`.

Rejected alternatives:

- **Parsing in Swift.** The app has no test target
  (`find schematical -name '*Tests*'` → none), so a Swift parser could only
  be proven by hand. In C it gets a CTest.
- **A file-path API (`circuit_memory_load_file`).** The sandboxed app must
  read the file itself through its bookmark; the backend never opens files.
- **Loading after start.** The owner's rule is "set it when stopped, not
  when running"; the API mirrors it, and an EEPROM's contents mid-run come
  only from its own write cycles.

## 4. Detailed Design

### 4.1 `circuit_memory_load` (`circuit.h`, `circuit.c`)

```c
/* Memory load (EEPROM): writes count words from start, before circuit_start.
   Each value must fit the word width. 0, or -1 with circuit_error set. */
int circuit_memory_load(Circuit *circuit, size_t part, uint32_t start,
                        const uint32_t *values, size_t count);
```

Checks, in order, with these messages:

1. started → `"circuit already started"` (`check_building`).
2. `part >= part_count` → `"no part %zu"` (`check_part`).
3. hook missing → `"part %zu (%s) has no loadable memory"` (`%s` = part name).
4. `count > words || start > words - count` →
   `"memory load of %zu words at %u is past the end of %s (%u words)"`.
5. any `values[i] >> bits != 0` →
   `"word %u: 0x%X does not fit %u bits"` (index = `start + i`).

`count == 0` succeeds and changes nothing. On success the words are stored
with every bit known.

### 4.2 Hook (`sim.h`, `eeprom.c`)

```c
/* Memories: stores count words from start, every bit known. The caller has
   checked the range and widths. NULL = not loadable. */
void (*memory_load)(void *state, uint32_t start, const uint32_t *values, size_t count);
```

Appended after `memory_read`. `eeprom.c`: `eeprom_memory_load` copies into
`value[]` and sets `known[]` to `0xFF`; `EEPROM_MODEL` gains it as the 12th
initializer. No other model sets it.

### 4.3 Load-file parser (`backend/include/memory_image.h`, `backend/src/memory_image.c`)

```c
typedef enum { MEMORY_IMAGE_BINARY, MEMORY_IMAGE_INTEL_HEX } MemoryImageFormat;

/* The format for a file name: .hex, .ihx, .ihex (any case) → Intel HEX;
   anything else → binary. */
MemoryImageFormat memory_image_format_for(const char *file_name);

/* Fills values[0 .. words) from a load file's bytes: 0xFF (erased) where the
   file has no data. bits must be 8. 0 on success; -1 and a malloc'd message
   in *error (free it). values is untouched on failure. */
int memory_image_parse(const unsigned char *data, size_t size, MemoryImageFormat format,
                       uint32_t words, unsigned bits, uint32_t *values, char **error);
```

Parsing writes into a scratch buffer and copies to `values` only on
success.

**Binary.** Byte i is word i. `size > words` →
`"the file is %zu bytes; the memory holds %u"`. Shorter files leave the rest
FF; an empty file is all FF.

**Intel HEX.** Lines end in LF, CRLF or CR; blank lines (only whitespace)
are skipped; a trailing newline is optional. Each record is
`:LLAAAATT<data>CC` in hex digits of either case. The base address starts
at 0.

| Record | Meaning |
|---|---|
| `00` data | bytes at base + AAAA + i |
| `01` end of file | LL = 00; must appear; any record after it is an error |
| `02` extended segment address | LL = 02; base = value × 16 |
| `04` extended linear address | LL = 02; base = value << 16 |
| `03`, `05` start address | LL = 04; ignored |

Messages (line numbers count from 1, blank lines included):

| Case | Message |
|---|---|
| `bits != 8` | `only 8-bit memories can be loaded from a file` |
| line not starting with `:` | `line %zu: a record starts with ':'` |
| odd digit count or a non-hex character | `line %zu: not a hex record` |
| LL disagrees with the line length | `line %zu: the record length does not match its data` |
| checksum | `line %zu: bad checksum 0x%02X (expected 0x%02X)` |
| unknown type | `line %zu: unknown record type %02X` |
| wrong LL for 01/02/03/04/05 | `line %zu: record type %02X needs %u data bytes` |
| byte at or past `words` | `line %zu: address 0x%X is past the end of the memory (%u bytes)` |
| record after 01 | `line %zu: data after the end-of-file record` |
| no 01 | `no end-of-file record` |
| the same address written twice | later data wins (no error) |

`size == 0` in HEX → `no end-of-file record`.

### 4.4 Docs

- `docs/backend.md`: `memory_image.h` beside `parts.h`/`circuit.h` in the
  header list; a "Memory load" note under Notes (before start only, EEPROM
  only, FF elsewhere); `memory_image.c` and `memory_image_test` in Layout
  and Tests.

## Compatibility / Format Impact

- Public API: two new functions in a new header and one in `circuit.h`;
  nothing existing changes signature or behavior. The app compiles
  `backend/src` through a synchronized folder, so `memory_image.c` builds
  into it with no project edit; Swift sees the header once plan-04-C adds
  it to the bridging header.
- `SimComponentType` grows one trailing field (internal header).
- Part files, goldens, app: unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Load hook and `circuit_memory_load`

Lands alone: a new call with no caller in the app.

- [x] `backend/src/engine/sim.h`: `memory_load` after `memory_read` (§4.2) —
      as the last field, after plan-05's `light_time` (see Corrections).
- [x] `backend/src/models/eeprom.c`: `eeprom_memory_load`; 13th
      initializer of `EEPROM_MODEL`.
- [x] `backend/include/circuit.h`, `backend/src/circuit.c`:
      `circuit_memory_load` (§4.1), its comment in the header's memory block.
- [x] `backend/tests/circuit_check.c`: `check_memory_load` — on
      `AT28C256-15PU`: load `{0x12, 0x34}` at 0x7FFE → read back
      `0x12, 0x34`, `known` 0xFF; word 0 still 0xFF; each §4.1 message
      (after start; part 99; an SRAM `AS6C62256-55PCN`; 3 words at 0x7FFE;
      value 0x100); `count == 0` → 0. (11 PASS, messages compared with
      `strcmp`; also checks the loaded words survive `circuit_start`.)

Acceptance: loads land before start and every refusal has its message;
nothing else moves. **Met:** `ctest -R 'circuit_check|trace_check|listing_check'`
→ 3 of 3; the golden diff exits 0 (build dir `/tmp/logic-wt-p04`).
  Check: `cmake --build /tmp/logic-plan04 -j8 && ctest --test-dir /tmp/logic-plan04 -R 'circuit_check|trace_check|listing_check' --output-on-failure` → 3 of 3 pass (est. 15 s).
  Check: `git diff --exit-code backend/tests/golden` → no diff (est. seconds).
Commit: d20b12e

### Phase 2 — Load-file parser

Lands alone: pure functions with their own test.

- [x] `backend/include/memory_image.h`, `backend/src/memory_image.c`
      (§4.3); `src/memory_image.c` in `add_library(logic …)` in
      `backend/CMakeLists.txt`.
- [x] `backend/tests/memory_image_test.c` + `logic_test(memory_image_test)`:
      `memory_image_format_for` on `a.hex`, `A.HEX`, `b.ihx`, `c.ihex`,
      `d.bin`, `e`, `f.hex.bin`; binary: empty, 3 bytes, exactly `words`,
      `words + 1`; HEX: one data record, CRLF and CR endings, blank line,
      lowercase digits, type 02 and type 04 bases, 03/05 ignored, overlap
      (later wins), and one case per message row of §4.3, each compared with
      `strcmp` to the exact text; on every failure `values` is unchanged
      (prefilled with 0x5A, still 0x5A). (48 PASS, `0 failure(s)`; the
      wrong-LL row is tested for each of 01, 02, 03, 04, 05.)
- [x] `docs/backend.md` (§4.4).

Acceptance: every §4.3 rule and message has a passing case. **Met:**
`ctest -R memory_image_test` → `100% tests passed, 0 tests failed out of 1`;
`grep -n 'memory_image' docs/backend.md` → lines 11 (header), 81 (note),
96/108 (layout), 167 (tests).
  Check: `cmake -S backend -B /tmp/logic-plan04 && cmake --build /tmp/logic-plan04 -j8 && ctest --test-dir /tmp/logic-plan04 -R memory_image_test --output-on-failure` → passes (est. 10 s).
  Check: `grep -n 'memory_image' docs/backend.md` → the header, layout and test lines (est. seconds).
Commit: 41f140c

## Validation Plan

- Tests: `circuit_check` (`check_memory_load`), `memory_image_test`
  (all rules and messages, failure leaves output untouched).
- Coverage check: both new source files are in `add_library(logic …)` and
  every new function is called by a test (`grep -c 'circuit_memory_load\|memory_image_parse\|memory_image_format_for' backend/tests/*.c`).
- Runtime proof: none beyond the tests in this sub-plan (no caller yet);
  plan-04-C's runtime proof loads a real file through the app.
- Doc sync: `docs/backend.md`.
- Final gate (once, after Phase 2): `rm -rf /tmp/logic-land-p04b && cmake -S backend -B /tmp/logic-land-p04b && cmake --build /tmp/logic-land-p04b -j8 && ctest --test-dir /tmp/logic-land-p04b` → `100% tests passed, 0 tests failed out of 11` (est. 1 min; 11, see Corrections); `git diff --exit-code backend/tests/golden` (est. seconds).
  **Ran (2026-11-01, after Phase 2):** `100% tests passed, 0 tests failed out of 11`
  (2.9 s); golden diff exit 0. Coverage grep: `circuit_check.c` 8,
  `memory_image_test.c` 9.

## Decisions

Owner decisions for all of plan-04 are in plan-04-A §Decisions. Author's
choices for this letter:

- Load only before start; the backend never opens files; parsing in C for
  test coverage (§3).
- Format by file extension (`.hex`, `.ihx`, `.ihex` → Intel HEX), so a
  binary that happens to start with `:` is never misread.
- The end-of-file record is required, so a truncated HEX file is caught;
  overlapping data takes the later record, as most programmers do.

## Corrections

- 2026-11-01 (Phase 1): §2 and §4.2 say `memory_read` is the last
  `SimComponentType` field and the new hook is the 12th initializer.
  plan-05 had already appended `light_time` after `memory_read` (`led.c`
  and `display7.c` set it positionally), so `memory_load` goes after
  `light_time` as the 13th field; inserting it after `memory_read` would
  have shifted `light_time` into the wrong slot. Initializers measured:
  `grep -rn -A3 'SimComponentType [A-Za-z_]* = {' backend/src backend/tests
  | grep -c SimComponentType` → 16 (plan-05 added two), all positional.
- 2026-11-01: every `/tmp/logic-plan04` build dir in the checks ran as
  `/tmp/logic-wt-p04` (the former is the main checkout's CMake cache; see
  plan-04-A Corrections). The ctest total is 11 after this letter, not 10
  (plan-05's `engine_test`).

## Summary

Small, contained backend work: one hook, one call, one parser. The risk is
the Intel HEX address arithmetic, pinned by a case per record type and
message. SRAM, every part file and every golden are untouched.
