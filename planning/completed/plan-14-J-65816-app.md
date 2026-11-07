# plan-14-J: App — the W65C816S in the processor window and facts; plan-14 landing tier

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-14-I

J shows the W65C816S in the app: its core name in the facts panel, its
registers and its two flag layouts (emulation and native) in the processor
window, and 24-bit instruction addresses. Then it runs plan-14's landing
tier and archives the plan.

Outcome of J (the plan's outcome), in the headless harness: `facts
W65C816S6TPG-14` → `Core 65C816 (WDC)`, `Max clock 14 MHz`; the W65C816S
board from the app's own parts, reset with a switch, runs to its loop and the
processor window shows `Running`, `C 3344`, `E 0`, `PBR 00`, flags
`N V M X D I Z C` with M clear and X set, and last instruction
`008029  80 FE  BRA $8029`.

References: `schematical/schematical/Cpu/CpuWindow.swift` (`CpuSnapshot`:
registers, `flags`, `instructionText`), `Backend/PartFacts.swift`
(`coreName`); `planning/completed/plan-09-E-cpu-app.md` (§4 window, §5 the
harness `cpu` command — the pattern); plan-14-H §3.5 (registers, 24-bit
`instruction_address`), plan-14-I §3.2 (the board and program).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-I landed | `grep -c '^W65C816S' backend/tests/golden/parts.txt` → 2 | MET (2026-11-07: `2`) |

## 1. Goal

- App builds; harness `facts W65C816S6TPG-14` and `cpu cpu816.schematical`
  print the outcome lines; PNGs rendered.
- Landing tier (§ Validation Plan) green; every plan-14 sub-plan archived.

### Non-goals

- No backend change (a bug found here gets a test and a fix in the backend
  with a Correction in the letter that owns it).
- The 6502 and Z80 views are unchanged (the plan-09-E harness commands print
  the same lines).

## 2. Current State

- `PartFacts.coreName` maps `6502`, `65c02`, `z80`; anything else shows raw.
- `CpuSnapshot.flags` (CpuWindow.swift:115–131): `P` → `N V – B D I Z C`; `AF`
  → Z80 labels. `instructionText` formats `C004  A0 07  LDY #$07` with 4 hex
  digits of address. After H the address is `UInt32`.

## 3. Design

- `coreName("65816")` → `"65C816 (WDC)"`.
- `CpuSnapshot.flags`: when a register `E` exists (only the 65816 has one):
  E = 1 → `N V – B D I Z C` (emulation: bit 5 reads 1, bit 4 is B, as the
  65C02); E = 0 → `N V M X D I Z C` (native; `w65c816s.pdf` p.9, Table 2-1).
  Otherwise as today.
- `instructionText`: 6 hex digits of address when the processor has a `PBR`
  register (`008029  80 FE  BRA $8029`), else 4 as today.
- Registers grid: unchanged code; the 65816's 10 registers show with the
  widths the backend gives (`C 3344`, `DBR 00`, `E 0`).

## 4. Runtime proof (harness `/tmp/p14/harness`, plan-06-A's `build.sh`)

Fixtures in `/tmp/p14-proof/`:

- `prog816.bin`: 32768 bytes, the plan-14-I §3.2 program at offset 0,
  `00 80` at `7FFC`, `FF` elsewhere.
- `cpu816.schematical`: the plan-14-I §3.2 board from app parts — `5V`/`GND`,
  NJU6366DF1-TE1 (2 MHz) FOUT → PHI2 and the gates' PHI2 inputs,
  TE-GDS0204 position 1 between RESB and GND (OFF at start: RESB reads Z =
  inactive), the EEPROM's load file `prog816.bin`. Written with the
  harness's document API.

Commands:

1. `facts W65C816S6TPG-14` → Kind Processor, Function 65C816, Pins 40, Core
   65C816 (WDC), Speed grade 14, Max clock 14 MHz, Supply range 1.71–5.25 V,
   Footprint PDIP-40-600.
2. `cpu cpu816.schematical`: run 50 µs → `Unknown — waiting for reset`;
   switch 1 ON 5 µs, OFF; run 2 ms → the outcome lines; `readMemory` of the
   SRAM at 00100 → `11`, 08100 → `22`; render `cpu-816.png`.
3. `cpu` on the plan-09-E `cpu6502.schematical` → the plan-09-E lines
   unchanged (flags `N V – B D I Z C`, 4-digit address).

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — App

- [x] `PartFacts.swift`: `coreName` (`"65816"` → `65C816 (WDC)`).
- [x] `CpuWindow.swift`: the E-dependent flag labels and the 6-digit
      address (§3). `xcodebuild … -derivedDataPath /tmp/p14j-dd build` →
      `** BUILD SUCCEEDED **`.
- [x] Harness commands 1–3 (§4); PNGs.
      1. `facts W65C816S6TPG-14` → `Kind: Processor`, `Function: 65C816`,
         `Footprint: PDIP-40-600 — …`, `Pins: 40`, `Core: 65C816 (WDC)`,
         `Speed grade: 14`, `Max clock: 14 MHz`, `Supply range: 1.71–5.25 V`.
      2. `cpu cpu816.schematical` → `at 50 us: Unknown — waiting for reset`;
         after the switch and 2 ms: `Running · 3999 cycles`, registers
         `C 3344  X 0000  Y 0000  S 01FF  D 0000  DBR 00  PBR 00  PC 802B
         P 15  E 0`, flags `N V M X D I Z C` with X I C set (M clear),
         `last: 008029  80 FE  BRA $8029`; `sram 00100 = 11`,
         `sram 08100 = 22` (and 00200..00203 = 11 22 44 33);
         `/tmp/p14-proof/cpu-816.png`.
      3. `cpu cpu6502.schematical` → `Stopped (STP)`, `A 10  X FF  Y 00  S FF
         P 07  PC 8013`, flags `N V – B D I Z C`, `last: 8012  DB  STP`,
         `sram 00200 = 10`; `/tmp/p14-proof/cpu-6502.png`.

Acceptance: build succeeds; the harness prints §4's lines.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p14j-dd build` → `** BUILD SUCCEEDED **` (est. 2 min); `/tmp/p14/harness/run facts W65C816S6TPG-14`, `run cpu /tmp/p14-proof/cpu816.schematical` → the §4 lines (est. 1 min).
Commit: e8ecc42

### Phase 2 — Landing tier and archive

- [x] Run the Validation Plan's final gate; record the counts below.
      `cmake … && cmake --build … && ctest --test-dir /tmp/logic-p14` →
      `100% tests passed, 0 tests failed out of 21`; `wc -l` parts.txt `7411`, traces.txt `7411` (P0 7399 +
      12), footprints.txt `233` (F0 232 + 1); `|pio|` → `7`, `^W65C51N` →
      `3`, `^W65C816S` → `2`; `node backend/tools/jsmooch-gen/run.js
      /tmp/g.c && cmp …` → no output (exit 0); coverage grep → `9` (all 8
      sources, see Corrections); `xcodebuild … build` →
      `** BUILD SUCCEEDED **`.
- [x] Archive every `planning/plan-14-*.md` still open to
      `planning/completed/` (A–I archived as each landed; J in this
      commit).

Acceptance: the final gate's expected results, all observed.
  Check: the commands of § Validation Plan → their results (est. 6 min).
Commit: 41b8383

## Validation Plan

- Tests: `pio_test` (A1–A7, B1–B10, C1–C14, D1–D8), `serial_test` (E1–E7
  plus plan-12's), `cpu_core_test` (65816 line), `cpu_test` (H1–H13,
  `io_board`, `board_w65c816s`, and the plan-09 cases), `library_check`
  (the `pio` and `65816` refusals), the golden programs.
- Coverage check: each new source file is in the `logic` library —
  `grep -c -E 'pio\.c|bus65\.c|pio_pia6521\.c|pio_via6522\.c|cpu65816\.c|wdc65816' backend/CMakeLists.txt`
  → 8 (the five model files and the three jsmooch C files, one per line) —
  and each part is in the goldens (counts below).
- Runtime proof: `cpu_test io_board` (W65C02S + VIA + PIA + ACIA on library
  parts), `cpu_test board_w65c816s`, and the J harness run.
- Doc sync: `docs/parts-format.md` (§ `pio` with both models, § `serial`
  `acia6551` `"w65c51n"`, § `cpu` core `65816`), `docs/backend.md` (layout,
  tests), `datasheets/skipped-parts.md`, `third_party/jsmooch/README.md`.
- **Final gate** (run once, here):
  - `cmake -S backend -B /tmp/logic-p14 && cmake --build /tmp/logic-p14 -j8
    && ctest --test-dir /tmp/logic-p14` → `100% tests passed`, **T0 + 1**
    programs.
  - `wc -l < backend/tests/golden/parts.txt` → **P0 + 12**; `traces.txt` the
    same; `footprints.txt` → **F0 + 1**.
  - `grep -c -F '|pio|' backend/tests/golden/parts.txt` → 7;
    `grep -c '^W65C51N' …` → 3; `grep -c '^W65C816S' …` → 2.
  - `node backend/tools/jsmooch-gen/run.js /tmp/g.c && cmp /tmp/g.c
    backend/src/third_party/jsmooch/wdc65816_generated_opcodes.c` → no output.
  - `xcodebuild … build` → `** BUILD SUCCEEDED **`.
  - Estimate: about 6 min (the backend build and ctest about 3, the app
    build 2, the harness 1). This is the smallest set that touches every
    area plan-14 changed: the backend suites cover the kinds and goldens,
    the app build covers the Swift changes and the bundled part files.

## Decisions

- Author: the native-mode flag row is `N V M X D I Z C`, emulation keeps the
  65C02 row (§3).

## Corrections

- §4 harness: built at `/tmp/p14/outj` (plan-06-A `build.sh` against the
  Debug app in `/tmp/p14j-dd`), run as
  `/tmp/p14/outj/H.app/Contents/MacOS/harness <cmd>`, not
  `/tmp/p14/harness/run`. Its `board 816|6502 <dir>` command writes the
  `.schematical` files with the document API (`placeChip`, `connect`,
  `setMemoryFile`, `SchematicFile` save); `cpu <file>` reloads one, builds
  the netlist with `NetlistBuilder` and drives the circuit API directly
  (the processor window's `CpuInspector` needs a running
  `SimulationController`). The PNG is a view of the same `CpuSnapshot`
  (status, register grid, flag row, `instructionText`) through
  `ImageRenderer`.
- §4 command 3: no plan-09-E `cpu6502.schematical` survives in
  `/tmp`; the harness rebuilds that board (W65C02S6TPG-14, AT28C256-15PU,
  AS6C62256-55PCN, SN74HC00N, the plan-09-D program) the same way.
- Validation Plan coverage check: the grep counts `9`, not 8. The ninth
  line is `CMakeLists.txt:145`, the generator test's
  `JSMOOCH_C=…/wdc65816_generated_opcodes.c` path; the eight source lines
  (38, 65–68, 70–72) are all there.
- §4 command 2: the 816 board also ties the AS6C1008's CE2 high and leaves
  RDY open (plan-14-I Corrections).

## Summary

plan-14's risk was in C–D (the VIA's many modes, now under 22 `pio_test`
cases) and G–H (a fixed third-party core, proven on 5120 SingleStepTests
cases, and a bus adapter with a multiplexed bank). J only presents state the
backend already proves. Untouched: the 6502/Z80 adapters' behavior, the
`serial` kind outside the W65C51N key, and every existing part's golden line.
