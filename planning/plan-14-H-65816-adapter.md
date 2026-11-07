# plan-14-H: The `65816` bus adapter in the `cpu` kind; 24-bit state and tracker

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-14-G

H makes the fixed jsmooch core (G) a pin-level CPU: a new `cpu` core
`"65816"` whose adapter drives the W65C816S bus at the datasheet's times —
the bank address multiplexed on D0–D7 while PHI2 is low, VDA/VPA/VPB, MLB,
the E and MX outputs, ABORTB, RDY/WAI, STP, BE — and reads it back through
`circuit_cpu_state` with 24-bit addresses. The instruction tracker and
`CpuState` widen to 24-bit addresses for it.

Outcome of H: `cpu_test` cases `w65c816s_*` (H1–H13) pass on an inline copy
of the `W65C816S` device; every existing `cpu_test` case and golden line is
unchanged.

References: `datasheets/cpu/w65c816s.pdf` — pins and functions §2.12–2.25
p.10–14 (pinouts p.10; §2.15 BE p.12), AC Table 4-2 p.27, general timing
Figure 4-1 p.28, §5 bank address latching Figure 5-1 p.44, §7 caveats
p.50–52 (§7.4 ABORTB, §7.6 DB/BA with RDY low, §7.7 MX); `backend/src/models/cpu.c`, `cpu.h` (core table,
controls, timing, `cpu_sample`, `cpu_sample_reset`, `CpuTracker`),
`cpu6502.c` (the 65C02 adapter H mirrors), `backend/include/cpu_state.h`;
`docs/parts-format.md` § `cpu`; plan-14-G (the core's pins: `Addr`, `BA`,
`D`, `RW`, `VDA`, `VPA`, `VPB`, `ML`, `E`, M/X fields; `ABORT_pending`).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-G landed | `/tmp/logic-p14/cpu_core_test \| grep '^65816'` → `65816: 5120 pass, 0 expected failures` | MET (2026-11-07: that line, then `65816: abort ok`, the generator line) |

## 1. Goal

- `cpu_test` → H1–H13 `ok`; every earlier `cpu_test` case `ok`.
- `ctest` 100%, T0 + 1 programs; `git diff --stat backend/tests/golden` →
  empty.

### Non-goals

- The 6502/65C02 and Z80 adapters' behavior does not change (their
  `cpu_test` cases and golden traces are the gate). Their tracker calls move
  to the widened types only.
- Not modeled (as for the W65C02S): read-data hold tDHR; timing at supply
  columns other than 5 V; tACC (a memory requirement, not a CPU output).

## 2. Current State

- `cpu.h` (138 lines): `CpuCoreDesc { name, controls, timing, positive, ops }`,
  `CpuDevice`, `CpuPins` (16 address, 8 data, up to `CPU_MAX_CONTROLS` = 16
  controls), `CpuCoreOps { create, destroy, start, pin_changed, timer, state }`;
  `cpu_level`, `cpu_sample`, `cpu_sample_reset`; `CpuTracker` with 16-bit
  `start`/`address` and a floooh-style `CpuDasm` (`uint16_t pc`, byte-in and
  char-out callbacks).
- `cpu_state.h`: `CpuState.instruction_address` is `uint16_t`;
  `CPU_MAX_REGISTERS` 24; statuses include `CPU_WAITING`, `CPU_STOPPED`.
- `cpu6502.c` (657 lines): ticks the core once per PHI2 fall; drives address
  and R/W X at fall + tAH and valid at fall + tADS; write data X at the rise,
  valid at rise + tMDS, Z at fall + tDHW; the differential rule for X inputs
  (tick twice on a copy, LOST if they differ); VPB, MLB, BE, WAI's RDY drive,
  STP.
- App: `CpuWindow.swift:81` builds `Instruction(address:
  state.instruction_address …)`; J changes the app side.

## 3. Design

### 3.1 Device keys (core `"65816"`)

```json
"core": "65816",
"address": ["A0","…","A15"], "data": ["D0","…","D7"],
"clock": "PHI2", "rw": "RWB", "irq": "IRQB", "nmi": "NMIB", "reset": "RESB",
"rdy": "RDY", "abort": "ABORTB", "vda": "VDA", "vpa": "VPA",
"vectorPull": "VPB", "memoryLock": "MLB", "emulation": "E", "mx": "MX",
"busEnable": "BE",
"grades": {"14": {"tcyc": 70, "tpwh": 35, "tpwl": 35, "tads": 30, "tah": 10,
                  "tbas": 33, "tbh": 10, "tdsr": 10, "tmds": 30, "tdhw": 10,
                  "tpcs": 10, "tes": 10, "teh": 5, "tbvd": 25,
                  "vcc": [1.71, 5.25]}}
```

- Controls, in order: `clock rw irq nmi reset rdy abort vda vpa vectorPull
  memoryLock emulation mx busEnable` (14 ≤ `CPU_MAX_CONTROLS`). `"abort"`,
  `"vda"`, `"vpa"`, `"emulation"`, `"mx"` are new control keys, accepted only
  by core `65816` (`"abort" is not a key of core "65c02"`); `"sync"`, `"so"`,
  `"phi1Out"`, `"phi2Out"` are refused on `65816`.
- Timing (Table 4-2, 5 V column): `tcyc tpwh tpwl tads tah tbas tbh tdsr
  tmds tdhw tpcs tes teh tbvd`; the first three must be > 0. `max_hz` =
  floor(10^12 / 70000) = 14285714.

### 3.2 The bus cycle (Figure 4-1, p.28)

The core is ticked once per PHI2 fall (the commit), as `cpu6502.c`. After
the tick the core's pins hold the next cycle: `Addr` (A0–A15), `BA` (bank),
`RW`, `VDA`, `VPA`, `VPB`, `ML`, `D` (write data), `E`, and the M/X fields.

From the PHI2 fall that starts a cycle:

1. A0–A15, RWB, VDA, VPA, VPB, MLB: each that changes goes X at + `tah` and
   to its value at + `tads`.
2. **D0–D7 carry the bank address while PHI2 is low**: they go X at +
   `tdhw` (the previous cycle's write data or the memory's read data has its
   hold time first) and to `BA` at + `tbas`.
3. At the PHI2 rise: the bank stays until rise + `tbh`; then a **read**
   cycle floats D0–D7 (Z) and a **write** cycle drives X until rise +
   `tmds`, then the write data, held until the next fall + `tdhw`.
4. **Read data** is sampled at the next PHI2 fall; a bit that changed less
   than `tdsr` before it reads X (the 65C02 rule).
5. A cycle with VDA = VPA = 0 (internal operation) still drives its address,
   bank and RWB as the core gives them (the datasheet: "invalid address");
   the read data is ignored by the core.

### 3.3 E and MX (§2.17, §2.20, §7.7)

- **E** = the emulation flag; changes at PHI2 fall + `teh` after the cycle in
  which XCE changed it (`tes` is then the setup it guarantees before the
  next edge).
- **MX** multiplexes M and X so that M is valid at the PHI2 fall and X at
  the rise: MX = M from rise + `teh` until fall + `teh`, and X from fall +
  `teh` until rise + `teh`. The values are those of the cycle in progress
  (the core's M/X fields). §7.7's "invalid during the cycle after REP, SEP,
  PLP" falls out: that cycle's M/X are the new values one cycle earlier
  than the old instruction's view.

### 3.4 Inputs at each PHI2 fall (the 65C02 rules, plus ABORTB)

Sampled with `cpu_sample`, setup `tpcs`; Z reads inactive (IRQB, NMIB,
ABORTB, RESB high; RDY high; BE high).

- **RESB**: `cpu_sample_reset`. Low: status RESET; address X, D Z, RWB 1,
  VDA = VPA = 0, VPB = MLB = 1, E = 1, MX = 1 from + `tah`. High after ≥ 2
  cycles low: `WDC65816_reset` (and `regs.STP = 0`: the core's own reset
  does not restart a stopped CPU — the generator's TODO), then the reset
  sequence ending in the vector read at 00FFFC/D with VPB low. Fewer than 2:
  LOST. X: LOST.
- **RDY**: low holds the cycle (reads and writes), as the W65C02S; X: LOST.
  While RDY holds a cycle, D0–D7 stay in the data-transfer state through
  PHI2 low (§7.6): a held read keeps them Z and a held write keeps the write
  data — no bank address until the cycle completes.
  WAI drives RDY low (`inout`) until IRQ (even with I set), NMI, ABORT or
  reset; status WAITING. **STP**: status STOPPED until reset.
- **IRQB**, **NMIB**: passed to the core (`WDC65816_set_IRQ_level`,
  `_set_NMI_level`); X follows the differential rule (tick twice; LOST if the
  results differ).
- **ABORTB** (§2.13, §7.4): a low sampled at a fall marks the instruction in
  progress aborted. At the instruction's end (the next opcode-fetch cycle,
  VDA = VPA = 1) the adapter restores every register to the snapshot it took
  at that instruction's opcode fetch (C, X, Y, S, D, DBR, PBR, PC, P, E) and
  sets `regs.ABORT_pending`; the core then runs the abort sequence, pushing
  the aborted opcode's address and reading the vector at 00FFE8/9 (native) or
  00FFF8/9 (emulation) with VPB low. Memory writes the aborted instruction
  already made stay (the sheet inhibits only register changes). ABORTB X:
  LOST unless no instruction is in progress (reset). ABORTB held low through
  the abort sequence aborts it again (§7.4.1: "not recommended"), which this
  rule does without a special case. The §7.4.1.1–3 exceptions (status after a
  modify cycle, RTI after cycle 3, PBR/DBR after cycle 2 of an interrupt)
  are not modeled: the snapshot restores all registers. Listed under "Not
  modeled".
- **BE**: low floats A0–A15, D0–D7 and RWB `tbvd` after the change (X while
  BE is X); the CPU keeps running; high drives them again `tbvd` later. VDA,
  VPA, VPB, MLB, E, MX stay driven (§2.15: BE controls the address and data
  buffers and RWB only).
- **Clock**: X, a high phase shorter than `tpwh`, a low phase shorter than
  `tpwl`, or falls closer than `tcyc`: LOST (the 65C02 rule; the W65C816S is
  static, "DC" in Table 4-2, so a stopped clock is legal).

### 3.5 State and the tracker

- `cpu_state.h`: `instruction_address` becomes `uint32_t` (24-bit for the
  65816; 16-bit values for the other cores unchanged in value).
- Registers (`circuit_cpu_state`), in order: `C` 16, `X` 16, `Y` 16, `S` 16,
  `D` 16, `DBR` 8, `PBR` 8, `PC` 16, `P` 8, `E` 1 (10 ≤ 24). None while LOST
  or RESET.
- `CpuTracker`: `start`, `address` become `uint32_t`; the disassembler hook
  becomes `typedef size_t (*CpuDasm)(uint32_t pc, const uint8_t *bytes,
  size_t count, void *user, char *out, size_t cap)` returning the length it
  used; `cpu.c` wraps the three floooh disassemblers in it (their output is
  unchanged: `cpu_test`'s existing `LDA #$41`-style checks are the gate).
  `cpu_tracker_begin` takes a `void *user` the hook receives.
- The 65816 hook calls `WDC65816_disassemble` with the E/M/X captured at the
  opcode fetch (the operand length depends on them), reading from the
  recorded bytes; the text is the disassembler's mnemonic and operand
  uppercased, cut at its effective-address suffix (the first run of two
  spaces before `[`): `AF 56 34 12` → `LDA $123456`; `A9 34 12` with M = 0 →
  `LDA #$1234`; `5C 00 80 01` → `JMP $018000`. Instructions are at most 4
  bytes, which the tracker's 4-byte buffer holds.
- Bytes are recorded on cycles with VPA = 1 (program reads), at their 24-bit
  address (bank from `BA`).

### 3.6 Rejected alternatives

- **Drive D0–D7 from the fall itself with the bank** (no `tdhw` gap): the
  bus would show two drivers during the memory's read hold on every
  read-then-anything cycle pair; the 65C02 adapter already treats `tdhw`
  as the first moment the CPU may change the data pins.
- **A 24-bit address bus in the part** (bank on 8 extra pins): the chip has
  no such pins; the board latches the bank (I does it with an SN74HC573AN).
- **Implement ABORT inside the core** (suppress register writes cycle by
  cycle): every generated opcode would change; the snapshot/restore gives the
  same visible result for every instruction the sheet does not list as an
  exception.

## 4. Tests (`cpu_test`, `w65c816s_*` cases)

Fixture: an inline `W65C816S` device (§3.1 data). Bench: PHI2 1 MHz; a
16 MB sparse memory served on the bus — the bench latches D0–D7 at each
PHI2 rise as the bank (Figure 5-1's transparent latch, closed at the rise)
and serves `bank:address` during PHI2 high; X where nothing was given.

| Case | Checks |
|---|---|
| H1 reset | LOST at power-up; RESB low 2 cycles → reset sequence; vector read at 00FFFC/D with VPB low; E = 1, MX high in both phases (M = X = 1) |
| H2 bank address | native `LDA $123456` (AF 56 34 12): during the data read's PHI2 low D0–D7 = 0x12, valid at fall + 33 ns, Z at rise + 10 ns; A = 3456; VDA 1 VPA 0 |
| H3 write cycle | `STA $7F0010` (8F): D0–D7 = bank 0x7F in PHI2 low, then X at rise + 10 ns, data at rise + 30 ns, held to the next fall + 10 ns |
| H4 VDA/VPA/VPB | a 6-instruction program: per cycle (VDA, VPA, VPB, RWB, 24-bit address) equals the core's own pins (the bench logs both) |
| H5 MX | `CLC XCE REP #$10` (native, M = 1, X = 0): MX high during PHI2 high, low during PHI2 low, changing at + 5 ns |
| H6 E pin | `CLC XCE`: E falls at the fall after XCE's last cycle + 5 ns; `SEC XCE` back |
| H7 MLB | `INC $10` (E6 10): MLB low on its read, modify and write cycles only |
| H8 ABORTB | native, `LDA #$1234` then ABORTB low for one cycle during `INX`: X unchanged after; stack holds the INX opcode's address; vector read at 00FFE8/9 with VPB low; a memory write made by an aborted `STA` stays |
| H9 RDY | RDY low holds a read and a write cycle; during the held read D0–D7 stay Z through PHI2 low and during the held write they keep the data (no bank, §7.6); RDY X → LOST |
| H10 WAI, STP | WAI pulls RDY low until IRQ (I set) ends it; STP → STOPPED; reset restarts |
| H11 BE | BE low: A, D, RWB Z after 25 ns; VDA/VPA keep driving; the CPU keeps counting cycles |
| H12 setup | read data changed 5 ns before the fall → X → used X → LOST; IRQB X while I = 1 → harmless |
| H13 state | registers `C X Y S D DBR PBR PC P E` after a short native program; `instruction_text` `LDA $123456`, `instruction_address` 0x018003 for code in bank 1 |

Also `library_check`: `"abort"` on a `65c02` device is refused;
`"sync"` on a `65816` device is refused.

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — Widen the tracker and state

- [x] `cpu_state.h` `instruction_address` → `uint32_t`; `cpu.h`/`cpu.c`
      `CpuTracker` → 32-bit addresses and the new `CpuDasm` (§3.5); wrap
      `m6502dasm_op`, `w65c02dasm_op`, `z80dasm_op`; `cpu6502.c`,
      `cpuz80.c` call sites. (`cpu_dasm_floooh` in `cpu.c` adapts a floooh
      disassembler; `cpu6502.c`/`cpuz80.c` hold the three wrappers;
      `cpu_tracker_begin` takes the per-instruction `user`; the next byte's
      address wraps within the start's bank.) Build: no errors or warnings.
- [x] App: `CpuWindow.swift` compiles with the `UInt32` address (format
      unchanged for 16-bit values; J adds the 24-bit format). (`Instruction.address`
      → `UInt32`.)
- [x] Acceptance: `/tmp/logic-p14/cpu_test` → 136 `PASS` lines, `io_board ok`,
      `cpu_test: all passed`; `ctest` → `100% tests passed, 0 tests failed out
      of 21`; the golden diff stat → empty; `xcodebuild … -derivedDataPath
      /tmp/p14h-dd build` → `** BUILD SUCCEEDED **`.

Acceptance: every existing `cpu_test` case `ok` (the instruction texts
unchanged); goldens unchanged; the app builds.
  Check: `/tmp/logic-p14/cpu_test` → all `ok`; `git diff --stat backend/tests/golden` → empty; `xcodebuild … -derivedDataPath /tmp/p14h-dd build` → `** BUILD SUCCEEDED **` (est. 4 min).
Commit: —

### Phase 2 — The 65816 adapter

- [ ] `cpu.c`: core `65816` in the core table with its controls and timing
      (§3.1); the new control keys and refusals.
- [ ] `backend/src/models/cpu65816.c`: §3.2–§3.5; `extern const CpuCoreOps
      CPU65816_OPS` in `cpu.h`; `backend/CMakeLists.txt`.
- [ ] `cpu_test` H1–H13 with the 24-bit bench; `library_check` refusals.
- [ ] `docs/parts-format.md` § `cpu`: core `65816` (keys, timing names,
      behavior §3.2–§3.5, not modeled: tDHR, §7.4.1 abort exceptions,
      non-5 V timing); `docs/backend.md` § Layout (`cpu65816.c`), § Tests
      (`cpu_test`).

Acceptance: H1–H13 and every earlier case `ok`; `ctest` 100%; goldens
unchanged.
  Check: `ctest --test-dir /tmp/logic-p14` → `100% tests passed`; `/tmp/logic-p14/cpu_test | grep -c '^ok w65c816s'` → 13 (est. 3 min).
Commit: —

## Validation Plan

`cpu_test` H1–H13 (the adapter at the datasheet times), the unchanged
earlier `cpu_test` cases and goldens (no regression in the 6502/Z80 path
from the tracker change). The landing tier is plan-14-J's.

## Decisions

- Author: ABORT restores a register snapshot taken at the aborted
  instruction's opcode fetch; the §7.4.1 exceptions are not modeled (§3.4).
- Author: D0–D7 change to the bank at fall + `tdhw` (X) and are valid at +
  `tbas` (§3.2, §3.6).
- Author: the 65816 instruction text is jsmooch's, uppercased and without
  its effective-address suffix, to match the 65C02's style (§3.5).

## Corrections

(none yet)
