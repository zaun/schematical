# plan-14-G: The vendored 65816 core (jsmooch, MIT), fixed in its generator; SingleStepTests/65816

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-14-F

G puts a cycle-stepped 65816 core in the tree and proves it, cycle by cycle,
against a committed subset of SingleStepTests/65816. The only MIT/zlib
cycle-stepped 65816 core in C is raddad772's `wdc65816` from jsmooch (MIT).
As shipped it passes native mode but fails much of emulation mode and
merges VDA/VPA/VPB into one flag; G fixes it **in its JavaScript code
generator** (also MIT) and regenerates the C, so the fixes stay reviewable
and reproducible. No part or adapter yet (H).

Outcome of G:

- `backend/src/third_party/jsmooch/` holds the core, stub headers,
  `LICENSE-jsmooch` and a `README.md` naming the source commit and every
  change; `backend/tools/jsmooch-gen/` holds its generator and our Node
  driver (outside `backend/src/`, so the app does not bundle the JS).
- `node backend/tools/jsmooch-gen/run.js` regenerates
  `wdc65816_generated_opcodes.c` byte for byte (a `cpu_core_test` check runs
  it when CMake finds `node`).
- `backend/tests/cpu/sst-65816.jsonl` (5120 tests) and its MIT licence.
- `cpu_core_test` prints `65816: 5120 pass, 0 expected failures`, checking
  every cycle's 24-bit address, data, RWB, VDA, VPA, VPB, MLB, E, M and X.

References:

- Source: `howprice/jsmooch-emus` at `57366f22500671b5e3c208d4283d33c66299acee`
  (2025-07-21), MIT, "Copyright (c) 2024 raddad772". The original
  `raddad772/jsmooch-emus` is gone (`gh api repos/raddad772/jsmooch-emus` →
  404, 2026-11-01); this copy is not a GitHub fork and carries the licence.
  Files: `jsmooch-lib/src/component/cpu/wdc65816/{wdc65816.c,wdc65816.h,
  wdc65816_generated_opcodes.c,wdc65816_disassembler.c,wdc65816_disassembler.h}`
  and `jsmooch-lib/code_gen/wdc65816_and_spc700/{generation.js,
  wdc65816_opcodes.js,wdc65816_core_generator.js}`.
- Tests: `SingleStepTests/65816` at `307c63f5673c8c6b861c52f71ddb89212a227480`
  (2026-10-16), MIT, "Copyright (c) Thomas Harte, et al"; 512 files
  `v1/XX.{e,n}.json`, 10,000 tests each; cycles are
  `[address, value|null, "dpvremxl"]` (VDA, VPA, VPB, RWB, E, M, X, MLB).
- `datasheets/cpu/w65c816s.pdf` (§7 caveats p.50–52).
- `planning/completed/plan-09-A-cpu-cores.md` (vendoring precedent,
  `cpu_core_test`, `make_sst_subset.py`).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-F landed (archived) | `ls planning/completed/plan-14-F-pio-app.md` → exists | MET (2026-11-07: exists) |
| Downloads reachable | `curl -sfIL https://raw.githubusercontent.com/howprice/jsmooch-emus/57366f22500671b5e3c208d4283d33c66299acee/LICENSE \| head -1` → `HTTP/2 200` | MET (2026-11-07: `HTTP/2 200`) |
| Node.js for the generator | `node --version` → v18 or later | MET (2026-11-07: `v24.12.0`) |

## 1. Goal

- `ctest -R cpu_core_test` → `65816: 5120 pass, 0 expected failures` (and
  the 6502/65C02/Z80 lines unchanged).
- `node backend/tools/jsmooch-gen/run.js /tmp/x.c && cmp /tmp/x.c
  backend/src/third_party/jsmooch/wdc65816_generated_opcodes.c` → no output.
- `ctest` 100%, T0 + 1 programs (no new program); goldens unchanged.

### Non-goals

- floooh/chips is not touched.
- No `cpu` kind change (H), no part (I).
- The generator's SPC700, Z80, SM83 and m6502 paths are not vendored.

## 2. Current State

### Measured populations (probes in `/tmp/p14`, 2026-11-01)

| What | Value | Command / source |
|---|---|---|
| Vendored file sizes | `wdc65816.c` 203 lines, `wdc65816.h` 134, `wdc65816_disassembler.c` 583, `.h` 16, `wdc65816_generated_opcodes.c` 54200; generator `wdc65816_core_generator.js` 2974, `wdc65816_opcodes.js` 975 | `wc -l` at 57366f2 |
| Generator reproduces the shipped C | byte-identical | the driver of §4.2 under Node: `cmp out.c wdc65816_generated_opcodes.c` → identical |
| Unmodified core on the first test of each file (512) | 478 pass (native 255/256, emulation 223/256) | harness `/tmp/p14/cores/harness/run_jsm` |
| Unmodified core on the first 10 of each file (5120) | 4766 pass (native 2550/2560, emulation 2216/2560) | same, `t10.txt` |
| Failing files (first 10 each) | 43: emulation `00 02 10 15 16 30 34 35 36 44 48 50 54 55 56 5a 68 70 74 75 76 7a 80 90 94 95 96 9a b0 b4 b5 b6 c2 cb d0 d5 d6 da f0 f5 f6 fa`, native `cb` | `run_jsm t10.txt \| grep FAIL` |
| With `WDC_PINS_SEPERATE_PDV = true` | VDA/VPA/VPB per cycle wrong only in files `0c 0e 1c 2e 4e 6e 7c cb ce ee fc` (both modes, all 10 tests each = 220 tests); register/RAM/bus results unchanged (4766) | `/tmp/p14/gen/run_sep` on `t10.txt` |
| Subset size | 5120 tests, 6,213,884 bytes as minified JSON lines | `/tmp/p14/gen/mk.py <heads> 10` |

### What the failures are (read from the failing tests, 2026-11-01)

- **Direct page indexed, emulation mode** (`15 16 34 35 36 55 56 74 75 76 94
  95 96 b4 b5 b6 d5 d6 f5 f6`): the core forms `(TA + X + D) & 0xFF`,
  dropping D's high byte. Correct: when E = 1 and DL = 0 the address is
  `D + ((TA + X) & 0xFF)` (wraps in the page); otherwise `(D + TA + X) &
  0xFFFF` (`w65c816s.pdf` §7.2 p.50).
- **Stack and width in emulation mode** (`48 5a 68 7a da fa 9a c2`): the
  generator does not force M = X = 1 and S high byte = 01 when E = 1, so
  PHA/PHX/PHY/PLA/PLX/PLY push/pull two bytes, TXS sets SH, REP clears M/X.
  Correct: in E mode M and X read 1 always, SH = 01, REP cannot clear them,
  and the 6502-era stack instructions wrap S within page 1.
- **BRK/COP in emulation mode** (`00 02`): E-mode sequence pushes PCH, PCL,
  P (B set for BRK), no PBR; vectors 00FFFE (BRK/IRQ) and 00FFF4 (COP); D
  cleared, I set.
- **Branches in emulation mode** (`10 30 50 70 80 90 b0 d0 f0`): a taken
  branch that crosses a page adds a cycle in E mode only (native never).
- **Block moves in emulation mode** (`44 54`): X and Y are 8-bit (XH = YH =
  0) while E = 1.
- **WAI** (`cb`, both modes): its idle cycles drive the wrong VDA/VPA/RWB
  pattern.
- **VDA/VPA/VPB** (`0c 0e 1c 2e 4e 6e ce ee` absolute RMW: the data read
  cycle is flagged VPA instead of VDA; `7c fc` (a,X) indirect: the pointer
  reads flagged as program reads; `cb` WAI).
- **MLB, E, M, X** are not driven by the core. SingleStepTests flags them
  per cycle.

## 3. Design

1. **Vendor** the 8 source files above unchanged first (commit 1, so the
   diff of every fix is visible), with `LICENSE-jsmooch`.
2. **Stub headers** `helpers/int.h`, `helpers/jsm_string.h`, `helpers/debug.h`,
   `helpers/debugger/debugger.h`, `helpers/debugger/debuggerdefs.h` under
   `third_party/jsmooch/` — our own files (MIT, ours): the integer types,
   a fixed-buffer `jsm_string` with `jsm_string_sprintf`/`quickempty`, empty
   debug macros (`DBG_EVENT` etc.), and the few struct names the headers use.
   Only what the five C files need to compile.
3. **Fix in the generator**, then regenerate (`backend/tools/jsmooch-gen/run.js`), so each fix is a
   JS diff plus a regenerated C diff:
   - `WDC_PINS_SEPERATE_PDV = true`;
   - the VDA/VPA/VPB flags of the 11 files above;
   - emulation-mode rules (direct page wrap, forced M/X/SH, BRK/COP, branch
     page-cross cycle, 8-bit X/Y block moves), and WAI's idle cycles;
   - **MLB**: a new `pins.ML` output, low on every cycle of a read-modify-write
     instruction from its first data read to its write (ASL ROL LSR ROR DEC
     INC TSB TRB on memory), as SingleStepTests' `l` flag;
   - **E, M, X pins**: `pins.E`, `pins.MX`-source fields set from the
     registers each cycle, the value the cycle runs with (SingleStepTests'
     `e m x` flags; `w65c816s.pdf` §2.20: MX shows M at PHI2 fall and X at
     PHI2 rise — the adapter in H multiplexes the pin).
4. **Hand edits to `wdc65816.c`** (not generated): remove the "CRAP!"
   `dbg_break` hack (`wdc65816.c:147-150`) and the dbglog trace code; add
   `regs.ABORT_pending`, serviced at TCU 1 before NMI and IRQ by running the
   generator's existing `S_ABORT` sequence (opcode 0x101; vectors 00FFF8 E /
   00FFE8 N, `wdc65816_opcodes.js:455-459`). Restoring the registers of the
   aborted instruction is the adapter's job (H), as on the chip (§2.13 p.12).
   And in `wdc65816_disassembler.c`: the 16-bit immediate operands are
   formatted with `snprintf(buf, 5, "#$%04x", n)`, which cuts `#$1234` to
   `#$12` (`wdc65816_disassembler.c:156` and `:171`, the X/Y
   immediate) — the size becomes 50, like every other formatter.
5. **`README.md`**: source repo and commit, the deleted upstream, licence,
   the file list, and a numbered list of every change (generator flag,
   each fix, the hand edits, the stubs), as zlib/MIT good practice and so a
   later update can re-apply them.
6. **Build**: the three C files (`wdc65816.c`, `wdc65816_generated_opcodes.c`,
   `wdc65816_disassembler.c`) compile directly, like `third_party/cjson/cJSON.c`:
   listed in `backend/CMakeLists.txt`'s `logic` sources, and compiled by
   the app through the Xcode synchronized folder on `backend/src/` (no
   wrapper file, which would compile them twice in Xcode). Their `#include
   "helpers/…"` lines resolve next to the including file, so no header
   search path is added (`project.pbxproj` is not edited).
7. **Tests**: `make_sst_subset.py` gains a count argument (default 1, so the
   existing subsets regenerate identically) and the 65816 line; the subset
   is the first 10 tests of each of the 512 files. `cpu_core_test` gains a
   65816 runner: initial state (with SingleStepTests' quirks: E-mode tests
   give S with a high byte other than 01 → force 01xx; X flag set → clear
   XH/YH), every cycle compared on the 10 flags and address/data (data only
   where the test gives one), final registers and RAM. Plus one abort case:
   `ABORT_pending` set during `LDA $1234` → after it, the core fetches the
   vector at 00FFE8/9 (native) with the pushes the datasheet gives.
   A generator check: when CMake finds `node`, `cpu_core_test` runs
   `run.js` to a temp file and compares it with the committed C.

Correctness risk sits in the generator fixes: one emulation-mode rule
applied too widely breaks native mode. The 5120-test gate covers both modes
and every opcode, ten operand patterns each, so a wide break shows at once.

Rejected alternatives:

- **Patch the generated C by hand.** 54,200 lines with five copies of each
  opcode (`mx Mx mX MX E` tables); a hand fix in one copy is easy to miss in
  another, and the next regeneration loses it.
- **LakeSnes `cpu.c`** (MIT, 498/512): instruction-stepped with callbacks; no
  address on idle cycles and no VDA/VPA, so the bus adapter could not drive
  the datasheet waveform.
- **CLK's 65816** (MIT, the generator of the tests): C++ templates, not a
  per-cycle pin interface; the project's cores are C.
- **jocasta-emu**: the same core ported to C++, with the same bugs.
- **MAME, bsnes/higan, ares, snes9x, Mesen, lib65816**: BSD-3, GPL, ISC or
  non-commercial — outside the owner's MIT/zlib rule.
- **All 10,000 tests per file**: about 3 GB. Ten per file (6.2 MB) catches
  the branch page-cross and BRK cases that one per file misses (the 1-test
  sample showed 34 failing files, the 10-test sample 43).

## 4. Detailed design

### 4.1 Layout

```
backend/src/third_party/jsmooch/
  LICENSE-jsmooch          MIT, raddad772
  README.md                source, commit, changes
  wdc65816.c  wdc65816.h  wdc65816_generated_opcodes.c
  wdc65816_disassembler.c  wdc65816_disassembler.h
  helpers/…                our stubs (§3 item 2)
backend/tools/jsmooch-gen/
  generation.js  wdc65816_opcodes.js  wdc65816_core_generator.js   (MIT, raddad772)
  run.js                   our Node driver (§4.2)
backend/tests/cpu/sst-65816.jsonl, LICENSE-SingleStepTests-65816
```

### 4.2 The driver (`backend/tools/jsmooch-gen/run.js`, verified 2026-11-01 to reproduce the shipped C)

```js
// Usage: node run.js [out.c]   (default: the vendored wdc65816_generated_opcodes.c)
const fs = require('fs'), vm = require('vm'), path = require('path');
const dir = __dirname;
const ctx = { console }; ctx.window = ctx; ctx.document = {};
vm.createContext(ctx);
for (const f of ['generation.js', 'wdc65816_opcodes.js', 'wdc65816_core_generator.js'])
  vm.runInContext(fs.readFileSync(path.join(dir, f), 'utf8'), ctx, { filename: f });
vm.runInContext("set_gentarget('c'); __out = decode_opcodes_c();", ctx);
fs.writeFileSync(process.argv[2] ||
  path.join(dir, '..', '..', 'src', 'third_party', 'jsmooch', 'wdc65816_generated_opcodes.c'), ctx.__out);
```

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — Vendor unchanged, build, test harness

- [x] Copy the 5 C/H files at 57366f2 to `backend/src/third_party/jsmooch/`
      and the 3 JS files to `backend/tools/jsmooch-gen/`; `LICENSE` →
      `LICENSE-jsmooch` in both; the stubs; `run.js`; `README.md` (source, no
      changes yet). → `wc -l`: 203, 134, 54200, 583, 16; 2974, 975, 175 (as
      §2). The README is `README-jsmooch.md` (Corrections). The stub
      `debugger.h` includes `jsm_string.h` (upstream's `wdc65816.h` gets
      `struct jsm_string` through it).
- [x] `backend/CMakeLists.txt`: the three C files in the `logic` sources.
      Build: no errors or warnings.
- [x] `make_sst_subset.py` count argument; generate `sst-65816.jsonl` (10 per
      file) and `LICENSE-SingleStepTests-65816`; regenerate the three existing
      subsets with count 1 → byte-identical. → `sst-65816.jsonl` 5120
      lines, 6213884 bytes (`wc -lc`, as §2); `cmp` of the regenerated 6502,
      65C02, Z80 subsets with the committed ones → identical. The licence is
      the repo's `LICENCE` (MIT, "Copyright (c) Thomas Harte, et al").
- [x] `cpu_core_test` 65816 runner with the unmodified core and its observed
      failures listed as expected failures (the 43 files' failing tests and
      the 220 flag failures once the flag is on), so the test passes and
      each later fix shrinks the list. → `65816: 4721 pass, 399 expected
      failures` (the 43 files of §2 exactly; Corrections for the count).
      Flags are compared from Phase 2, when the core drives them.
- [x] (added) App build with the core: `xcodebuild … -derivedDataPath
      /tmp/p14g-dd build` → `** BUILD SUCCEEDED **` (after the README
      rename).

Acceptance: `cpu_core_test` passes and prints `65816: <p> pass, <f> expected
failures` with p + f = 5120; `run.js` reproduces the shipped C.
  Check: `ctest --test-dir /tmp/logic-p14 -R cpu_core_test --output-on-failure` → passes; `node backend/tools/jsmooch-gen/run.js /tmp/g.c && cmp /tmp/g.c backend/src/third_party/jsmooch/wdc65816_generated_opcodes.c` → no output (est. 3 min).
  Result: `cpu_core_test … Passed`; `run.js` → `cmp` identical.
Commit: d9110bf

### Phase 2 — The fixes

- [x] Generator: separate VDA/VPA/VPB; the 11 files' flags; emulation-mode
      rules; WAI; MLB; E/M/X fields. Regenerate after each; shrink the
      expected-failure list as tests pass. → with all flags compared:
      4161 (separate pins on) → 4661 (MLB, RMW data VDA) → 4701 ((a,x)
      pointer VDA) → 4721 (E/M/X held per instruction) → 4921 (E-mode
      direct page indexed) → 4999 (E table built with M = X = 1) → 5019
      (REP, TXS in E) → 5025 (E-mode BRK/COP P) → 5100 (branches) → 5120
      (WAI). The E/M/X outputs are set in `wdc65816.c` (hand edit 13 of
      the README), not by the generator (Corrections). The list is empty.
- [x] `wdc65816.c`: remove the debug hack and trace code; `ABORT_pending`;
      the abort case in `cpu_core_test`. → `65816: abort ok` (LDA $1234
      completes, pushes 00 80 00 38 at 01FF..01FC, VPB on 00FFE8/9, I set,
      D clear, handler fetched at 009000). The generator's S_ABORT gained
      I = 1, D = 0 (Corrections).
- [x] `cpu_core_test`: compare all 10 flags per cycle; the generator check.
      → every cycle's address, data, VDA VPA VPB RWB E M X MLB; `65816:
      generator reproduces wdc65816_generated_opcodes.c` (with a line
      appended to the C file it prints `FAIL: output differs`, then
      regenerated). `SST65816="<name>"` prints one test's cycles.
- [x] `README.md` change list; `docs/backend.md` § Layout (`third_party/jsmooch/`, `tools/jsmooch-gen/`)
      and § Tests (`cpu_core_test`: the 65816 line and the generator check).
      (`README-jsmooch.md`: 15 numbered changes.)
- [x] App build (the third-party C compiles once in Xcode). →
      `xcodebuild … -derivedDataPath /tmp/p14g-dd build` → `** BUILD SUCCEEDED **`.
- [x] (added) `wdc65816_disassembler.c`: the 16-bit immediate formatter size
      (§3 item 4) — README change 15.
- [x] Acceptance: `/tmp/logic-p14/cpu_core_test` → `6502: 241 pass, 15
      expected failures`, `65C02: 254 pass, 0 expected failures`, `Z80: 1592
      pass, 12 expected failures`, `65816: 5120 pass, 0 expected failures`,
      `65816: abort ok`, generator line; `ctest` → `100% tests passed, 0
      tests failed out of 21`; the golden diff stat → empty.

Acceptance: `65816: 5120 pass, 0 expected failures`; the abort case passes;
the generator check passes; the app builds.
  Check: `/tmp/logic-p14/cpu_core_test` → the four lines, 65816 `5120 pass, 0 expected failures` (est. 1 min); `xcodebuild … -derivedDataPath /tmp/p14g-dd build` → `** BUILD SUCCEEDED **` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `cpu_core_test` (5120 SingleStepTests cases, both modes, every
  opcode; the abort case; the generator reproduction check).
- Doc sync: `docs/backend.md`; `third_party/jsmooch/README.md`.
- The landing tier is plan-14-J's.

## Decisions

- **The W65C816S is included on jsmooch, fixed through its generator.**
  Owner, 2026-11-01 (§ plan-14-A Decisions). Third-party CPU code is MIT or
  zlib only (owner).
- Author: the subset is the first 10 tests of each file (§3, rejected
  alternatives).
- Author: the vendored copy is modified (MIT allows it); the README lists
  every change, and the unmodified files are the first commit.

## Corrections

- §3 item 5, §4.1: the README is `third_party/jsmooch/README-jsmooch.md`,
  not `README.md`. The app copies `backend/src` files flat into its
  resources; with floooh/chips' `README.md` there, the build failed:
  `error: Multiple commands produce '…/Contents/Resources/README.md'`. (The
  same reason `LICENSE-jsmooch` is not `LICENSE`.)
- §2 baseline: the unmodified core passes **4721** of 5120 here (§2's probe:
  4766). The failures are exactly §2's 43 files (all ten tests or most of
  them: `00.e 2`, `02.e 4`, `10.e 9`, `30.e 8`, `44.e 9`, `54.e 9`,
  `70.e 8`, `80.e 7`, `90.e 7`, `b0.e 9`, `d0.e 9`, `f0.e 8`, the rest 10).
  This runner checks every cycle's address (idle cycles too), RWB and the
  given data, then all final registers and RAM — stricter than the probe;
  the gate is 5120, so the count only sets the starting list.
- §3 item 7 (runner): STP's last test cycle has a null address (the bus is
  stopped) and STP's final PC is past the opcode, not a fetch address; the
  runner accepts any bus state on a null-address cycle and checks `regs.PC`
  when `regs.STP` is set.
- §2 "What the failures are" was incomplete; fixed besides it, each found
  by the per-cycle flag comparison: (a) the C table builder generated the
  emulation table with M = X = 0 (`decode_opcodes_c`; the JS
  `decode_opcodes()` used 1) — the root of the 8-bit stack and block-move
  failures; (b) `REPE` cleared M and X in emulation mode; (c) E-mode BRK
  pushed `P | 0x38` (sets D) and COP `P & 0xF7` (clears D): both push P with
  bits 4–5 set and D as it is; IRQ/NMI/ABORT push B = 0; (d) S_ABORT did not
  set I or clear D; (e) the relative branch's second idle cycle was taken on
  every taken branch in emulation mode, not only on a page cross.
- §3 item 3 "E, M, X pins … set from the registers each cycle": the tests
  show an instruction's cycles keep the E/M/X it began with (REP, SEP, PLP,
  RTI, XCE change them from the next opcode fetch). Done in `wdc65816.c`
  after each cycle: updated only when the instruction ends (`TCU = 0`).
- §3 item 4 / §2 WAI: WAI is two idle cycles at PBR,PC with no VDA/VPA,
  then no bus cycles until an interrupt (the tests' fourth cycle is a null
  address); the generator's WAI re-presented the opcode fetch. The runner
  treats a waiting core like a stopped one (PC past the opcode).
- `make_sst_subset.py`: MVN/MVP tests (`54`, `44`) are large; ten do not fit
  the Range read, so the script fetches the whole file whenever fewer than
  `count` tests were found. The file lists came from the GitHub tree API
  (the same data `gh api …/git/trees` gives).
