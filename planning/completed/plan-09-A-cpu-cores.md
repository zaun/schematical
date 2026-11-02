# plan-09-A: CPU datasheets, vendored CPU cores and their test vectors

Last updated: 2026-11-01
Overall Effort: x-large (1d–3d) — plan-09 is A → B → C → D → E
Effort: medium (1h–2h)
Depends on: plan-07 (all sub-plans), plan-08 and plan-10 complete (§ Prerequisites)

plan-09 adds three microprocessors as parts that run programs inside the
simulation: the NMOS **6502**, the WDC **65C02** (W65C02S) and the Zilog
**Z80** (Z84C00). Each CPU is a pin-level part: it is clocked by a net,
drives its address, data and control pins at the datasheet's times, and
reads the data bus at the datasheet's sampling edges. A CPU wired to an
EEPROM holding a program and to an SRAM runs that program, and the app shows
its registers in a window.

The instruction-level behaviour comes from Andre Weissflog's cycle-stepped
cores (`floooh/chips`: `m6502.h`, `w65c02.h`, `z80.h`, plus the matching
disassemblers), which are zlib-licensed. The owner allowed zlib for these
files. The plan's own code wraps each core in a **bus adapter** that turns the
core's once-per-clock pin mask into datasheet waveforms (B for the 6502
family, C for the Z80).

| Letter | Content | Effort |
|---|---|---|
| **A** | datasheets; vendored cores and disassemblers; SingleStepTests subsets; `cpu_core_test` | medium |
| B | `cpu` kind, CPU state API, 6502/65C02 bus adapter, `cpu_test` (6502 family) | large |
| C | Z80 bus adapter, `cpu_test` Z80 cases | large |
| D | 3 footprints, `cpu.json` (19 parts), goldens, whole-system tests, docs | medium |
| E | app: catalog details, sidebar facts, the processor window | large |

This sub-plan puts the third-party code and the test data in the tree and
proves the unmodified cores against the test vectors, with no part and no
model yet.

Outcome of A:

- `datasheets/cpu/` holds the 6 PDFs of §4.1, each matching its SHA-256.
- `backend/src/third_party/chips/` holds the 6 headers of §4.2 byte for byte
  (SHA-256 below), plus their licence as `LICENSE-chips`.
- `backend/tests/cpu/` holds three SingleStepTests subsets (2,114 tests,
  1,549,129 bytes), the script that made them, and the two MIT licences.
- `ctest` gains `cpu_core_test`. It passes:
  - every subset test except the 27 named deviations of §4.4;
  - and each of those 27 still fails, so a future core update that fixes one
    is noticed.

References:

- `AGENTS.md` (datasheets, part files, goldens), `docs/backend.md`
  (layout, tests, adding a kind), `docs/parts-format.md`.
- `backend/CMakeLists.txt` (the `logic` library source list, `logic_test`).
- `backend/src/third_party/cjson/` (the precedent for vendored code: source
  plus `LICENSE`, compiled into `logic` and by the Xcode synchronized folder).
- floooh/chips at commit `9e88298ce56319953ac7a43213a1120359f7a3a6`
  (2026-09-26): https://github.com/floooh/chips
- SingleStepTests: https://github.com/SingleStepTests/65x02 at
  `2f6980a2d95757486c7bee24355c360e40e2a224`, https://github.com/SingleStepTests/z80
  at `ebe1875d48f374bcfd4b505d8eb8ee751568b5f7` (both MIT).

## Prerequisites

This table is the gate for all of plan-09. B–E point here and add their own
rows.

| Must be true | Command | Status |
|---|---|---|
| plan-07 complete (owner: plan-09 waits for plan-07 and plan-08) | `ls planning/plan-07-* 2>/dev/null` → no output; `ls planning/completed/plan-07-G-*.md` → exists | MET (2026-11-01 at 1da038c: no `planning/plan-07-*`; `planning/completed/plan-07-G-voltage-app.md` exists) |
| plan-08 complete | `ls planning/plan-08-* 2>/dev/null` → no output; `ls planning/completed/plan-08-*.md` → exists | MET (2026-11-01 at 1da038c: `planning/completed/plan-08-schematic-sidebar.md` exists) |
| plan-10 complete (owner, 2026-11-01: plan-10 lands before plan-09) | `ls planning/plan-10-*.md 2>/dev/null` → no output; `ls planning/completed/plan-10-J-app.md` → exists | MET (2026-11-01 at 1da038c: no `planning/plan-10-*.md`; `planning/completed/plan-10-J-app.md` exists. Command corrected, see Corrections) |
| `vcc` is a required key (plan-07-F landed) | `grep -n '"vcc"' docs/parts-format.md` → the key is listed as required, with no "required from plan-07-F" text | MET (2026-11-01 at 1da038c: parts-format.md:198 requires it, no "required from plan-07-F" text) |
| Backend tests green | `cmake -S backend -B /tmp/logic-p09 && cmake --build /tmp/logic-p09 -j8 && ctest --test-dir /tmp/logic-p09` → `100% tests passed` | MET at 1da038c, 15 of 15 (2026-11-01). **T0 = 15** |
| No `cpu` kind yet | `ls schematical/schematical/Parts/cpu.json backend/src/models/cpu.c 2>/dev/null` → no output | MET (2026-11-01 at 1da038c) |
| Downloads reachable | `curl -sfIL https://raw.githubusercontent.com/floooh/chips/9e88298ce56319953ac7a43213a1120359f7a3a6/chips/z80.h \| head -1` → `HTTP/2 200` | MET (2026-11-01 at 1da038c) |

At the start of A, also record the golden line counts as **P0**
(`wc -l < backend/tests/golden/parts.txt`; it equals `traces.txt`) and
**F0** (`footprints.txt`). Plan-07 changes them, so they are measured
then, not now (at 5992f1c they are 4006 and 140).

Measured at the start of A (1da038c, 2026-11-01): **P0 = 6170**
(`traces.txt` also 6170), **F0 = 160**.

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify — a
> prerequisite recorded NOT MET may have landed since, and one recorded MET may
> have regressed.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `shasum -a 256 datasheets/cpu/*.pdf` matches §4.1 (6 files).
- `shasum -a 256 backend/src/third_party/chips/*.h` matches §4.2 (6 files).
- `ctest -R cpu_core_test` passes, printing
  `6502: 241 pass, 15 expected failures`,
  `65C02: 254 pass, 0 expected failures`,
  `Z80: 1592 pass, 12 expected failures`.
- `ctest` → T0 + 1 tests, all passing.

### Non-goals (explicit constraints)

- The vendored headers are **not modified**. zlib clause 2 requires an altered
  copy to be marked, and an unmodified copy keeps the SHA-256 check meaningful.
  Gaps in the cores (RES level, SO, VPB, MLB, BE, BUSREQ, Z80 RESET pin, the
  int-ack address) are covered by the adapters in B and C, never by edits here.
- No `cpu` kind, part, model or app change. No golden changes.
- Klaus Dormann's functional tests (GPL-3) and ZEXALL/ZEXDOC (GPL-2+) are not
  vendored or downloaded; they are not MIT or zlib.

## 2. Current State

- Third-party code: only `backend/src/third_party/cjson/` (`cJSON.c`,
  `cJSON.h`, `LICENSE`). `cJSON.c` is in the `logic` source list
  (`backend/CMakeLists.txt`), and the app compiles it through the Xcode
  synchronized folder on `backend/src/`. The `LICENSE` file there sets the
  precedent: a vendored library keeps its licence file next to it.
- Xcode adds `backend/src/third_party/cjson` to the header search paths
  (`project.pbxproj` lines 276, 318). This plan does **not** edit
  `project.pbxproj`. It has uncommitted owner changes, and the wrapper files
  include the vendored headers by relative path (§4.3), so no search path is
  needed.
- Tests: `logic_test(NAME)` in `backend/CMakeLists.txt` builds
  `tests/NAME.c` against `logic_test_support` and defines `PARTS_DIR` and
  `GOLDEN_DIR`. A new test that reads data files needs its own directory
  define (§4.5).
- `datasheets/<group>/` holds committed PDFs (e.g. `datasheets/eeprom/`, 12
  files). No `datasheets/cpu/` yet.

### Measured populations

| What | Count | Command / source |
|---|---|---|
| Vendored header lines | m6502.h 3107, w65c02.h 2925, z80.h 2838, m6502dasm.h 485, w65c02dasm.h 247, z80dasm.h 492 | `wc -l` on the clone at 9e88298 |
| SingleStepTests files | 6502: 256, wdc65c02: 256 (2 empty: `cb.json`, `db.json` = WAI, STP), z80: 1604 | git tree API at the two commits (`gh api repos/SingleStepTests/<repo>/git/trees/<sha>?recursive=1`) |
| Subset tests (first test of each non-empty file) | 6502 256, 65C02 254, Z80 1604 = 2114 | `wc -l` on the generated `.jsonl` files |
| Subset bytes | 90,171 + 83,491 + 1,375,467 = 1,549,129 | `wc -c` |
| Unmodified-core results on the subsets | 6502 241/256, 65C02 254/254, Z80 1592/1604 | the probe harnesses `/tmp/p08/run/sst65.c`, `sstz80.c` (2026-11-01), logic in §4.5 |

### Verified properties

- **Licences** (read from the files): `floooh/chips/LICENSE` is
  "zlib/libpng license, Copyright (c) 2018 Andre Weissflog". The header
  comments repeat it (`z80.h`: Copyright (c) 2021). SingleStepTests/65x02
  `LICENSE`: MIT, "Copyright (c) 2024 Thomas Harte et al".
  SingleStepTests/z80 `LICENSE`: MIT, "Copyright (c) 2024 SingleStepTests".
- **The cores build cleanly here.** Each header compiled alone with
  `CHIPS_IMPL` / `CHIPS_UTIL_IMPL` under
  `cc -std=c11 -Wall -Wextra -Wcomma -Wdocumentation -Wstrict-prototypes -Wshorten-64-to-32`:
  0 warnings for all six. That covers the app's enabled `CLANG_WARN_*`
  settings, and warnings are not errors (`grep -c TREAT_WARNINGS_AS_ERRORS
  project.pbxproj` → 0). `nm` over the six objects shows no duplicate global
  symbol. All six headers included together without `IMPL` compile in one
  translation unit, which the adapters in B and C rely on.
- **Cycle-level agreement.** The probe ran each subset test on the
  unmodified core.
  - 6502 / 65C02: the per-cycle address, data and R/W of every bus cycle,
    then the final registers and RAM. P is loaded with bits 4–5 set and
    compared without them, because the core keeps B and bit 5 in P.
  - Z80: the final registers (incl. WZ, R, I, IFF, IM), RAM, the T-state
    count, and the ordered list of bus transactions (type, address, data).
  - Results: 6502 241/256, 65C02 254/254, Z80 1592/1604. The failures are
    exactly the 27 tests of §4.4.
- **Disassembler output** (probe `/tmp/p08/run/dasm.c`, `dasmz.c`):
  - `A9 41` → `LDA #$41`; `8D 00 02` → `STA $0200`;
  - `3A 34 12` → `LD A,(1234h)`; `DD CB 05 06` → `RLC (IX+5)`;
  - `DB` (65C02) → `STP`.

## 3. Design Overview

Three independent pieces, each a file drop plus checks:

1. **Datasheets** (`datasheets/cpu/`): the facts B–D read.
2. **Cores** (`backend/src/third_party/chips/`) plus four one-line wrapper
   translation units in `backend/src/models/` that instantiate them, so
   CMake and Xcode each build every core exactly once.
3. **Vectors and `cpu_core_test`**: a fixed, committed subset of
   SingleStepTests and a CTest that runs the cores against it. It is the
   regression net for the vendored code. B and C test the adapters
   separately.

Risk is low. Nothing calls the cores yet, and the gate is a test that already
passed in the planning probe. If `cpu_core_test` disagrees with §1, the
harness differs from the probe's logic (§4.5). Compare it against the probe
files and fix it. The cores themselves are not edited.

Rejected alternatives:

- **Our own MIT-only cores** (the user's first constraint). No MIT core in C
  is cycle-stepped with deferred reads (survey 2026-11-01: CLK and kosarev are
  C++ with synchronous reads; vrEmu6502, superzazu, ZOT run whole
  instructions). Writing three cores would add 3–5 days. The owner chose zlib
  floooh/chips.
- **Vendoring all of SingleStepTests**: 1,081,886,574 bytes (6502) +
  1,000,495,993 (wdc65c02) + 1,374,635,394 (z80), summed from the git tree
  sizes.
  One test per opcode file (1.5 MB) catches a broken vendoring or build. The
  cores were verified on the full sets upstream.
- **Downloading vectors at test time.** CTest must run offline.
- **Header search paths in Xcode** instead of relative includes. That needs a
  `project.pbxproj` edit, which carries owner changes.

## 4. Detailed Design

### 4.1 Datasheets (`datasheets/cpu/`)

Download each with
`curl -sfL -A "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15" -o datasheets/cpu/<file> '<url>'`
and check `shasum -a 256`.

| File | What | URL | Bytes | SHA-256 |
|---|---|---|---|---|
| `w65c02s.pdf` | WDC W65C02S datasheet, Feb 16 2024 (pins p.12–13, DC p.24, AC p.25, timing diagram p.26, ordering p.32) | https://www.wdc65xx.com/wdc/documentation/w65c02s.pdf | 392075 | 4fb4a2c1055e319c6d8df5a6f7b348d9cffa958fdabc982a494bda54e5e17195 |
| `mos_6500_mpu_nov_1985.pdf` | CSG/MOS NMOS 6500 family (order code p.1, 1–2 MHz timing p.5, 3–4 MHz p.6, 6502 pinout p.10) | https://6502.org/documents/datasheets/mos/mos_6500_mpu_nov_1985.pdf | 6261893 | ccd72376d1e5db1fb13777de538417f4799ab38e1f746452f9695465c422dcd7 |
| `ps0178.pdf` | Zilog Z84C00 product spec PS017801-0602 (pinouts PDF p.1–2, DC p.30, AC printed p.35–36) | https://www.zilog.com/docs/z80/ps0178.pdf | 1389263 | 06198d3c22a79a3f10c049751ef212e84dab501e32fd7bd40a3566950cdafcba |
| `um0080.pdf` | Zilog Z80 CPU User Manual UM008011-0816 (timing chapter from printed p.7) | https://www.zilog.com/docs/z80/um0080.pdf | 1587333 | e3c83da5a5d8e372364c20fa53665e6fbb165ec6ac38c8c1eebc359603447b5e |
| `ps0072.pdf` | Zilog packaging spec PS007233-1120 (40-PDIP MKT71C0005 p.6, 44-PLCC MKT71C1001 p.10, 44-LQFP MKT71C1182 p.16) | https://www.zilog.com/docs/ps0072.pdf | 1235338 | 710a8284c6743ed83ef4b222479515183de2a396794ccd6a51caefecdf0d8c09 |
| `Z84C00-PCN-ZAC24-0029.pdf` | Zilog PCN ZAC24-0029, Z84C00 end of life, last orders 14 Jun 2024 | https://web.archive.org/web/20240418172336id_/https://www.mouser.com/PCN/Littelfuse_PCN_Z84C00.pdf | 389391 | 503caad78f4fa98f5591d0e50fb734c8c59b3d0bd38cd307497c307ba3e0665d |

`datasheets/obsolete-not-included.md` does **not** change. The owner made a
one-off exception to AGENTS.md § Datasheets for plan-09: the all-obsolete
NMOS 6502 and Z84C00 parts are added, flagged `obsolete` (D).

### 4.2 Vendored code (`backend/src/third_party/chips/`)

Fetch from
`https://raw.githubusercontent.com/floooh/chips/9e88298ce56319953ac7a43213a1120359f7a3a6/<path>`:

| File (repo path) | SHA-256 |
|---|---|
| `chips/m6502.h` | b3fc32a01f1f7cdc62f919ccba06cc20c2a333e7ca30c38189e31aa65868d46e |
| `chips/w65c02.h` | de958aad423db1112a1b91e3c68c8bf84b299e04036b6840fceb0f99e9d762c5 |
| `chips/z80.h` | 6ca70ffd91b1bdbaf00f7a41e7092b326145fd1c95dcdd7bb01183370de734f3 |
| `util/m6502dasm.h` | ed669afd5f256672f918c1a51643e33153b6c1af2a376496afd463db56150a24 |
| `util/w65c02dasm.h` | 0816aa4b266d00de0a7722a606ef87fcc1518c4c81f2e94dfb21ac1a0a3b7d31 |
| `util/z80dasm.h` | 59e1cacb2c73db9e3b77d3607dae3f726bc75d0aea0528ddbb77d111b3b0dc47 |
| `LICENSE` → `LICENSE-chips` | (content copied as is; renamed, see Corrections) |

All seven land flat in `backend/src/third_party/chips/`. Add a `README.md`
there with:

- the upstream URL and commit;
- the line "Unmodified copies; see plan-09-A";
- one line per file: what it is.

### 4.3 Wrapper translation units (`backend/src/models/`)

Four files, each only a comment and two lines. For example
`chips_m6502.c`:

```c
/* The m6502 core (floooh/chips, zlib; see third_party/chips/README.md). */
#define CHIPS_IMPL
#include "../third_party/chips/m6502.h"
```

- `chips_w65c02.c` and `chips_z80.c` are the same for their core.
- `chips_dasm.c` defines `CHIPS_UTIL_IMPL` and includes the three
  disassemblers. All three in one translation unit compiled cleanly in the
  probe.

Each core header needs `<stdint.h>` and `<stdbool.h>`, so the wrappers
include them first.

Add the four files to `add_library(logic …)` in `backend/CMakeLists.txt`.
Xcode picks them up through the synchronized folder. The `.h` files and
`README.md` are not compiled. `LICENSE-chips` and `README.md` follow the cJSON
precedent for non-source files in that folder.

### 4.4 Vectors (`backend/tests/cpu/`)

- `make_sst_subset.py`: the probe's `first_tests.py`. For each listed file
  it fetches the head with an HTTP Range request (the whole file on 416),
  keeps the first test object, and writes it as one minified JSON line. It
  skips empty files and writes lines sorted by path. Usage:
  `python3 -I make_sst_subset.py <repo> <sha> <listfile> <prefix-regex> <out.jsonl>`.
  The list file comes from
  `gh api "repos/SingleStepTests/<repo>/git/trees/<sha>?recursive=1" -q '.tree[] | select(.path|test("<regex>")) | "\(.path) \(.size)"'`.
  Both commands go in the script's header comment.
- `sst-6502.jsonl` (prefix `^6502/v1/`): 256 lines, SHA-256
  ee8f34c0ea83e076aaf8aebb152790128a80a4e828d805f7e26ddb6a505ae2bb.
- `sst-65c02.jsonl` (prefix `^wdc65c02/v1/`): 254 lines, SHA-256
  61b5c7c6335bf610674b655f55ec8c8ba92f6b2cd762b07d067fabd623e2aff7.
- `sst-z80.jsonl` (prefix `^v1/`, repo z80): 1604 lines, SHA-256
  a8944b56285aa0d277fa1bf03fa571ad146f46614ed7b863c72beba79da737d9.
- `LICENSE-SingleStepTests-65x02` and `LICENSE-SingleStepTests-z80`: the
  two MIT texts, verbatim.

**Expected failures** (by test `name`). The test asserts that each fails.

- 6502, 15 tests:
  - the twelve JAM opcodes `02 1`, `12 1`, `22 1`, `32 1`, `42 1`, `52 1`,
    `62 1`, `72 1`, `92 1`, `b2 1`, `d2 1`, `f2 1`. The core re-reads
    `FFFF`; SingleStepTests records `FFFE`/`FFFF` and other bytes. The CPU
    is hung either way.
  - `93 1`, `9c 1`, `9e 1` (SHA/SHY/SHX). On a page cross the core forms
    the high address byte differently from the real chip. These are the
    "unstable" undocumented opcodes.
- 65C02: none.
- Z80, 12 tests:
  - `37 0000`, `FD 37 0000`, `FD 3F 0000` (SCF/CCF). The core does not
    model the Q register, which sets undocumented flag bits 3 and 5 (XF/YF).
  - `ED B1 0000`, `ED B2 0000`, `ED B8 0000`, `ED B9 0000`, `ED BA 0000`,
    `ED BB 0000` (CPIR, INIR, LDDR, CPDR, INDR, OTDR repeating). The flags
    differ from the 2018-documented values for an interrupted block
    repeat.
  - `76 0000`, `DD 76 0000`, `FD 76 0000` (HALT). The core keeps PC on the
    HALT opcode and re-fetches it; the real chip advances PC. C records the
    effect on the bus as a known deviation.

### 4.5 `cpu_core_test` (`backend/tests/cpu_core_test.c`)

It reads the three `.jsonl` files with `read_file` and cJSON.
`backend/CMakeLists.txt`:

```cmake
logic_test(cpu_core_test)
target_compile_definitions(cpu_core_test PRIVATE
    CPU_VECTORS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/cpu")
```

**6502 and 65C02** (one routine; the core is chosen by a function-pointer
table over `m6502_*` / `w65c02_*`):

1. Clear a 64 KB array and load `initial.ram`.
2. `init`, then set A X Y S, P = `initial.p | 0x30`, and PC.
3. Pins = SYNC | RW with address = PC and data = the byte at PC (the
   prefetch).
4. For each entry *i* of `cycles`:
   - compare the pins (address, data, RW) with entry *i*;
   - `tick`;
   - then serve the returned pins: a read sets the data from the array; a
     write stores the data into the array.
5. Compare A X Y S, `P & 0xCF` against `final.p & 0xCF`, and every
   `final.ram` byte.

**Z80**:

1. `z80_init`, then set every register from `initial`: a f b c d e h l i r
   wz ix iy sp, the shadow registers af_ bc_ de_ hl_, im, iff1, iff2.
2. Pins = `z80_prefetch(pc)`.
3. Tick `len(cycles) + 1` times. Serve MREQ reads/writes from the array.
   Serve IORQ reads from `ports` (default `FF`). Record each read, write,
   in and out as `(type, address, data)`.
4. Drop the final M1 fetch.
5. Compare:
   - every final register, with `pc` = core PC − 1;
   - RAM;
   - `z80_opdone`;
   - the transaction list against `cycles`. A transaction there is the
     first entry carrying `r`/`w` with `m` or `i` (flags `rwmi`: `m` is
     position 3, `i` position 4), with its
     data taken from the first non-null data in that run.

Output: one line per CPU in the §1 form. Each unexpected failure is printed
by name; so is each expected failure that passed. The exit status is
non-zero for either.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes** — never in a
>   batch at the end of a phase, never "I'll update the plan after". If it is a
>   separate step it will be skipped, and the list becomes a lie.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>` rather than
>   deleting it. A vanished task is indistinguishable from a forgotten one.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed; an unlisted task that got done is a
>   task the next reader will do twice.
>
> **An unticked box means NOT DONE.** Someone reading this file — including the
> person who has to decide whether to stop — is entitled to trust that.

### Phase 1 — Datasheets

Files only. Nothing reads them yet.

- [x] `datasheets/cpu/`: the 6 PDFs of §4.1. `wc -c` + `shasum -a 256 *.pdf` →
      all 6 sizes and SHA-256 equal §4.1 (11,255,293 bytes total).

Acceptance: every file matches its size and SHA-256.
  Check: `cd datasheets/cpu && shasum -a 256 *.pdf` → the six hashes of §4.1 (est. 1 min).
Commit: d012540

### Phase 2 — Cores, wrappers, vectors and `cpu_core_test`

The vendored code builds in both build systems, and the cores are proven on
the vectors.

- [x] `backend/src/third_party/chips/`: the 6 headers, `LICENSE-chips` and
      `README.md` (§4.2). `shasum -a 256 *.h` → the 6 hashes of §4.2;
      `wc -l` → 3107/2925/2838/485/247/492.
- [x] `backend/src/models/chips_m6502.c`, `chips_w65c02.c`, `chips_z80.c`,
      `chips_dasm.c` (§4.3), added to `backend/CMakeLists.txt`. Fresh
      `cmake --build /tmp/logic-land-p09a` → 0 `warning:` lines.
- [x] `backend/tests/cpu/`: `make_sst_subset.py`, the three `.jsonl`
      subsets, the two licence files (§4.4). Regenerated with the committed
      script from fresh `gh api` tree lists → "empty: wdc65c02/v1/cb.json,
      db.json"; `wc -lc` → 256/254/1604 lines, 1,549,129 bytes; SHA-256 of
      all three equal §4.4.
- [x] `backend/tests/cpu_core_test.c` and its CMake lines (§4.5).
      `ctest --test-dir /tmp/logic-p09 -R cpu_core_test -V` → the three §1
      lines. Negative check: dropping `"93 1"` from the 6502 list and
      changing one 65C02 vector's final A → `6502: FAIL 93 1`,
      `65C02: FAIL 00 27 ce`, exit 1; restored → exit 0.
- [x] `docs/backend.md`: Layout gains `third_party/chips/` and
      `tests/cpu/`. Tests gains `cpu_core_test` (what it runs and the
      expected-failure rule). Also the four `chips_*.c` under `models/`.
- [x] The app still builds with the new translation units. `xcodebuild …
      -derivedDataPath /tmp/p09-dd …` → `** BUILD SUCCEEDED **`, 4
      `chips_*.c` compile lines, 0 warnings from them (after the
      `LICENSE-chips` rename; see Corrections).

Acceptance: the cores pass the vectors exactly as in §1, and the app builds.
  Check: `shasum -a 256 backend/src/third_party/chips/*.h backend/tests/cpu/*.jsonl` → the 9 hashes of §4.2 and §4.4; `cmake --build /tmp/logic-p09 -j8 && ctest --test-dir /tmp/logic-p09 -R cpu_core_test -V | grep -E '^[0-9]+: (6502|65C02|Z80):'` → the three lines of §1 (est. 2 min); `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p09-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 2 min; the only check that the synchronized folder compiles the wrappers).
Commit: 5203c6c

## Validation Plan

- Tests: `cpu_core_test`, with negative cases built in (each expected failure
  must fail).
- Coverage check: the subsets touch every opcode file of each CPU: 256, 254
  and 1604 lines, against 256, 256 − 2 empty and 1604 files.
- Runtime proof: none beyond the test. No part uses the cores until B.
- Doc sync: `docs/backend.md` (Phase 2).
- Final gate (once, after Phase 2):
  `rm -rf /tmp/logic-land-p09a && cmake -S backend -B /tmp/logic-land-p09a && cmake --build /tmp/logic-land-p09a -j8 && ctest --test-dir /tmp/logic-land-p09a`
  → `100% tests passed`, T0 + 1 tests (est. 2 min). Plus the Phase 2
  `xcodebuild`.

## Decisions

- **The 6502, 65C02 and Z80 are added even though the NMOS 6502 and every
  Z84C00 are obsolete.** Decided by the owner, 2026-11-01. This is a
  one-off exception to AGENTS.md § Datasheets: the parts carry
  `"flags": ["obsolete"]` (D), and AGENTS.md is not changed.
- **zlib-licensed floooh/chips cores are allowed** (in place of the
  original MIT-only rule). Decided by the owner, 2026-11-01, because no MIT
  C core is cycle-stepped. MIT code (SingleStepTests) is also used. GPL test
  suites are not.
- **A CPU that reads an unknown value, or has not been reset, goes "lost"
  until reset.** Decided by the owner, 2026-11-01 (B §4.4, C).
- **The app gets a read-only processor window** (registers, flags, last
  instruction, cycle count). Decided by the owner, 2026-11-01 (E).
- **The W65C02S QFP-44 part (W65C02S6TQG-14) is left out.** Decided by the
  owner, 2026-11-01: WDC publishes no outline for it.
- **plan-09 runs after plan-07 and plan-08.** Decided by the owner,
  2026-11-01. CPUs carry voltage data, and E extends plan-08's sidebar facts.
- **Plan number 09** (08 was taken by the sidebar plan). Owner, 2026-11-01.

## Corrections

- **Chips licence file name** (Phase 2). With the upstream file named
  `LICENSE`, `xcodebuild` failed: "Multiple commands produce
  '…/schematical.app/Contents/Resources/LICENSE'". The synchronized folder
  copies non-source files from `backend/src` into the app's resources, and
  `third_party/cjson/LICENSE` already has that name. The project has no
  membership exceptions (`grep -c membershipExceptions project.pbxproj` → 0),
  and §2 rules out editing `project.pbxproj`. The licence keeps its upstream
  content as `LICENSE-chips`; the README says why. §4.2, the Outcome and
  Phase 2 are corrected. Afterwards → `** BUILD SUCCEEDED **`.
- **Z80 cycle flags** (§4.5). The SingleStepTests z80 flags are four
  characters `rwmi` (seen: `----`, `r-m-`, `-wm-`, `r--i`, `-w-i`), so `m` is
  position 3, not "flag 4". §4.5 is corrected. The harness groups each run of
  consecutive transaction cycles with the same type and address, and gives
  the §1 counts.
- **Phase 2 check grep.** `ctest -V` puts a `<n>: ` prefix on every test
  output line, so `grep -E '^(6502|65C02|Z80):'` matched nothing. Corrected
  to `'^[0-9]+: (6502|65C02|Z80):'` → the three §1 lines.
- **`/tmp/logic-p09`** was first configured from the main checkout (gate run).
  CMake refuses another source dir for the same cache, so it was deleted and
  reconfigured from the plan worktree. B–D use the same path.
- **Prerequisites, plan-10 row.** The command `ls planning/plan-10-*` lists
  the committed data folder `planning/plan-10-data/` (README, counter/latch/
  shift notes), so it never prints nothing even with plan-10 archived. It is
  data, not an open sub-plan. Command corrected to `ls planning/plan-10-*.md`
  (2026-11-01: no output; `planning/completed/plan-10-J-app.md` exists).

## Summary

A is low-risk plumbing: third-party files under fixed hashes, and a test that
passed in planning. The engineering risk of plan-09 is in B and C, where the
adapters turn a once-per-clock core into datasheet waveforms.
