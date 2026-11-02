# plan-09-B: The `cpu` kind, the CPU state API and the 6502/65C02 bus adapter

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-09-A

This sub-plan adds the `cpu` part kind and the 6502-family model:

- the loader for `"core": "6502"` and `"core": "65c02"` devices;
- `PartCpu` facts;
- a public CPU-state read-back (`circuit_cpu_state`);
- the bus adapter that runs the vendored `m6502` / `w65c02` core
  (plan-09-A) from the clock pin and drives the pins at the datasheet's
  times.

The Z80 follows in C, and the real parts in D.

Outcome of B: `cpu_test` passes every 6502-family case of §5. The cases run
on inline fixture devices that carry exactly the data D will ship. A
fixture W65C02S, clocked at 1 MHz and fed by a bench memory, comes out of
reset and runs `LDA #$41 / STA $0200 / STP`. Then:

- the bench sees the write of `41` to `0200` with the W65C02S's write timing;
- `circuit_cpu_state` reports A = `41`, status Stopped, last instruction
  `STP`.

References:

- plan-09-A (cores, § Decisions), `docs/backend.md` § Adding a kind,
  `docs/parts-format.md`.
- `backend/src/models/eeprom.c` and `backend/tests/eeprom_test.c`: the
  precedent for a kind with grades, an inline fixture, and a bench component
  that drives pins.
- `backend/src/engine/sim.h`: hooks, `sim_schedule_drive` (transport) vs
  `sim_schedule_inertial_drive`.
- `datasheets/cpu/w65c02s.pdf` p.12–13, 25–26; `datasheets/cpu/mos_6500_mpu_nov_1985.pdf` p.5–6, 10.
- plan-07-A §4.1–4.2 (`vcc`, `inputMax`, `kind_voltage`).

## Prerequisites

plan-09-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-09-A complete | `ls planning/plan-09-A-* 2>/dev/null` → no output; `ls planning/completed/plan-09-A-*.md` → exists | MET (2026-11-01 at 1e5e729; plan-09-A's own rows re-run at 1da038c the same day, all MET) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of *all*
> prerequisites.**

## 1. Goal

- `cpu_test` passes every case of §5.1–§5.3 (6502-family part).
- `library_check` passes, including the new `cpu` rejection cases of §5.4.
- `listing_check`, `trace_check` and `footprint_check` are unchanged: no
  part file ships until D, so the goldens do not move.
- `ctest` → T0 + 2 tests (A's `cpu_core_test` plus `cpu_test`), all passing.

### Non-goals (explicit constraints)

- No change to any existing model, part or golden line.
- The vendored cores are not edited (plan-09-A Non-goals).
- Not modeled, and stated in `docs/parts-format.md` § `cpu`:
  - the NMOS 6502's minimum clock (50 kHz, a dynamic-logic limit);
  - the read-data hold time tDHR (data is sampled at PHI2↓);
  - the cycle-accurate IRQ pipeline while RDY holds the CPU;
  - timing at supply voltages other than the grade's (W65C02S: the 5 V
    column only);
  - the 12 JAM opcodes' exact bus pattern, and the SHA/SHX/SHY page-cross
    address (plan-09-A §4.4).
- No Z80 (C), no part file (D), no app change (E). The new enum value
  `PART_CPU` reaches the app, where `symbol(for:)` falls to `.block` and
  `CatalogPart.init` to its last `else` (`PartCatalog.swift:316`). No part
  of that kind exists until D, so nothing shows.

## 2. Current State

- Kinds: `KINDS[]` in `backend/src/library/kinds.c` lists 13 kinds after
  plan-10 (which appends `latch`, `counter`, `shift`). `PartKind` ends
  `… PART_SUPPLY, PART_EEPROM, PART_LATCH, PART_COUNTER, PART_SHIFT`
  (`parts.h`). The app and tests use the names, never numbers (re-checked for
  plan-04-A). So appending `PART_CPU` renumbers nothing.
- One `SimComponentType` per kind (`PartKindOps.model`, `kind.h`). A kind
  with several behaviours dispatches inside its model. The engine config is
  the `const Part *`; model data comes through `part_internal(part)`.
- Hooks: `memory_size`/`memory_read`/`memory_load`, `lights`,
  `light_time` and `set_switch` (`sim.h`). The circuit API wraps each one
  (`circuit.c:301-393`), returning 0 or -1 when the hook is NULL.
- Drives:
  - `sim_schedule_drive` is transport: every scheduled drive happens.
  - `sim_schedule_inertial_drive` cancels earlier pending drives on the
    pin.
  - The adapter's X-then-value windows need transport, the same as the
    SRAM read window.
- Net resolution (`net.c:87-126`): a strong X against a strong 0/1 gives X,
  not a short. Only strong 0 against strong 1 is "smoke". So a lost CPU
  driving X never stops the run.
- Test support needs updating for a new kind:
  - `listing.c` `listing_kind_name` and `print_details`: one case each.
  - `trace_check.c`: the file-level `TRACE_LAST_KIND` (plan-10-B; `PART_SHIFT`
    after plan-10), which becomes `PART_CPU`.
- Voltage keys after plan-07: grades of `sram`/`eeprom` take `"vcc"`,
  `"inputMax"` (after `"note"` in `TIMING_NAMES`), and parts take the same
  as overrides, resolved by `kind_voltage` (plan-07-A §4.2). `cpu` follows
  the same pattern.

### Measured populations

| What | Count | Command |
|---|---|---|
| Last kind in tests | 1 (`TRACE_LAST_KIND`) | `grep -n 'TRACE_LAST_KIND' backend/tests/trace_check.c` → the `#define` and its uses (after plan-10) |
| Listing switch cases to extend | 2 | `grep -n 'case PART_SHIFT' backend/tests/support/listing.c` → 2 (after plan-10) |
| Circuit hook wrappers to copy | 1 pattern (`circuit_memory_read`, `circuit.c:380`) | read |
| 6502-family pins per device | W65C02S 40 (PDIP) / 44 (PLCC); NMOS 6502 40 | datasheets (D §4.2) |

### Verified properties

- **Core tick contract** (`m6502.h` / `w65c02.h` headers, and the plan-09-A
  probe):
  - `tick(cpu, pins)` consumes the data bits of `pins` (when the previous
    cycle was a read) plus IRQ/NMI/RDY/RES.
  - It returns the address, RW and SYNC of the *next* bus cycle; on a
    write, it returns the data too.
  - So one tick per PHI2↓ fits the 6502 bus: data sampled at PHI2↓, next
    address valid tADS after it.
- **Core input semantics** (read in `w65c02.h:530-600`, the same in
  `m6502.h:717+`):
  - NMI is edge-detected inside the core against the stored `PINS`.
  - IRQ is level-sampled.
  - RDY stalls only read cycles.
  - RES is a request taken at the next SYNC.
  - `init()` starts the 7-cycle reset sequence (stack reads, then
    `FFFC`/`FFFD`).
  - `wait_flag` (WAI) and `stop_flag` (STP) exist on `w65c02_t` only.
- **No SO, VPB, MLB or BE in the cores**: the header pin lists
  (`M6502_PIN_*`, `W65C02_PIN_*`) end at RES (bit 30). The adapter provides
  them (§4.5).
- **Struct copies are cheap and complete.** `m6502_t` and `w65c02_t` are
  plain structs with no pointers to themselves (`m6502_t` holds optional
  6510 callback pointers, NULL here). A copy ticked forward does not
  disturb the original, so the peek and the differential rule (§4.4) are
  sound.

## 3. Design Overview

1. **Kind and facts** (`models/cpu.c`): keys, pin binding and grades for
   the 6502 family; `PartCpu`; a per-core table of adapter functions behind
   one `SimComponentType`.
2. **State API** (`include/cpu_state.h`, `sim.h` hook, `circuit.c`):
   `circuit_cpu_state` fills a `CpuState` with status, cycle count,
   registers and the last instruction (disassembled with the vendored
   disassembler).
3. **6502-family adapter** (`models/cpu6502.c`): one tick per PHI2↓,
   datasheet output windows, the reset/lost/held rules, RDY, SO, BE, VPB,
   MLB, WAI/STP, and the differential rule for unknown inputs.
4. **Tests** (`tests/cpu_test.c`): a bench that clocks the CPU and serves
   memory, on fixture devices.

Correctness risk sits in the adapter's timing and in the lost/differential
rules (§4.4). Each rule has a `cpu_test` case at its datasheet times.
Everything here is new behaviour: the gate is `cpu_test`, not byte-identity.
The goldens must not change, because no part ships in B.

Rejected alternatives:

- **One kind per CPU** (`6502.json`, `z80.json`). Same loader and state API;
  three kinds would triple `KINDS[]`, listing and app cases for no gain. The
  file's `"core"` key selects the adapter.
- **Pass RDY to the core** (its built-in read stall). The W65C02S also
  stalls write cycles, and the NMOS chip ignores RDY on writes. The adapter
  handles both by not ticking. One rule for both CPUs, in our code.
- **Lost on every X read.** The reset sequence's three dummy stack reads
  hit uninitialised SRAM (X) on any real board, so the CPU could never
  start. The differential rule makes the CPU lost only when the unknown
  bits change its state.
- **Hold-time checks on read data.** A memory turning its outputs off right
  after PHI2↓ would make CPUs lost in correct circuits. The datasheet's
  tDHR (10 ns) is shorter than every SRAM/EEPROM output hold in the library
  allows for. Not modeled (Non-goals).
- **Inertial drives for the address windows.** The later "value" drive
  would cancel the earlier "X" drive (`sim.h`), so the window would vanish.
  Transport drives keep both.

## 4. Detailed Design

### 4.1 Part file keys (`kind: "cpu"`)

Device keys (6502 family):

| Key | Value |
|---|---|
| `"core"` | `"6502"` (NMOS) or `"65c02"` (WDC W65C02S). C adds `"z80"`. |
| `"address"` | 16 pin names, `address[i]` = A*i* |
| `"data"` | 8 pin names, `data[i]` = D*i* |
| `"clock"` | the clock input (NMOS `PHI0IN`, W65C02S `PHI2`) |
| `"rw"`, `"sync"` | outputs: R/W (1 = read), SYNC (1 = opcode fetch) |
| `"irq"`, `"nmi"`, `"reset"`, `"so"` | active-low inputs: IRQ, NMI, RES, SO |
| `"rdy"` | RDY (high = run). An input on NMOS; an `inout` on the W65C02S (driven low during WAI). |
| `"phi1Out"`, `"phi2Out"` | clock outputs (NMOS Φ1/Φ2 OUT, W65C02S PHI1O/PHI2O) |
| `"vectorPull"`, `"memoryLock"`, `"busEnable"` | `65c02` only: VPB, MLB (active-low outputs), BE (input, high = bus driven) |
| `"grades"` | grade → timing (ns): `tcyc tpwh tpwl tads tah tsync tmds tdhw tdsr tpcs tbvd`, plus `"vcc"`, `"inputMax"`, `"note"` |

- Each control key names exactly one pin (`kind_pin`). `address` and `data`
  must have exactly 16 and 8 names.
- A key belonging to another core is rejected:
  `"vectorPull" is not a key of core "6502"`.
- Every key of the device's core is required.
- `tcyc`, `tpwh` and `tpwl` must be > 0; the other timing names default to 0.

Part keys: `"grade"` (a key of `grades`), `"vcc"`, `"inputMax"`.

Timing names (all ns; 6502 family). Each one, the edge it is measured from,
and how the model uses it:

| Name | Measured from | Model use |
|---|---|---|
| `tcyc` | — | minimum clock period (fall to fall) |
| `tpwh`, `tpwl` | — | minimum clock high and low widths |
| `tads` | PHI2↓ | address and R/W valid |
| `tah` | PHI2↓ | address and R/W held; X after it |
| `tsync` | PHI2↓ | SYNC valid |
| `tmds` | PHI2↑ | write data valid |
| `tdhw` | PHI2↓ | write data held, then Z |
| `tdsr` | before PHI2↓ | read data setup |
| `tpcs` | before PHI2↓ | setup for RDY, IRQ, NMI, RES, SO |
| `tbvd` | BE edge | BE to bus valid / Z |

`PartCpu` (`parts.h`), with `Part` gaining `const PartCpu *cpu;` after
`eeprom`:

```c
/* core: "6502", "65c02" or "z80". max_hz = floor(1e12 / tcyc_ps). */
typedef struct { const char *core; const char *grade; uint64_t max_hz; } PartCpu;
```

Listing details: `core=<core> grade=<grade> max_hz=<n>` (`listing.c`), and
`listing_kind_name` → `"cpu"`.

### 4.2 Model layout

- `models/cpu.c`:
  - `CPU_KIND` (the `PartKindOps`): device and part keys, load, bind,
    part.
  - The one `CPU_MODEL` `SimComponentType`. `create` picks the per-core
    `CpuCoreOps { create, destroy, start, pin_changed, timer, state }` by
    `core`; every other hook forwards to it.
  - Shared helpers: clock-width checks, setup tracking, the instruction
    tracker (§4.6).
- `models/cpu6502.c`: the `CpuCoreOps` for `6502` and `65c02`. It includes
  the declarations of both cores (`../third_party/chips/m6502.h`, `w65c02.h`)
  and keeps a tagged union of `m6502_t` / `w65c02_t`. The two cores differ
  only in the function called and in the 65C02 extras (§4.5).
- `KINDS[]` gains `&CPU_KIND`; `PART_CPU` is appended to `PartKind`.

### 4.3 CPU state API

New public header `backend/include/cpu_state.h`, included by `circuit.h`:

```c
typedef enum {
    CPU_RUNNING, CPU_LOST, CPU_RESET, CPU_WAITING, CPU_STOPPED,
    CPU_HALTED, CPU_JAMMED, CPU_BUS_RELEASED
} CpuStatus;
typedef struct { const char *name; uint32_t value; unsigned bits; } CpuRegister;
enum { CPU_MAX_REGISTERS = 24 };
typedef struct CpuState {
    CpuStatus status;
    uint64_t cycles;          /* clock cycles since the last reset ended */
    size_t register_count;    /* 0 while LOST or RESET */
    CpuRegister registers[CPU_MAX_REGISTERS];
    int has_instruction;      /* 1 once an instruction has completed since reset */
    uint16_t instruction_address;
    uint8_t instruction_bytes[4];
    size_t instruction_length;
    char instruction_text[32]; /* the vendored disassembler's text, e.g. "LDA #$41" */
} CpuState;
```

- `sim.h`: forward-declare `struct CpuState;` and add the last hook
  `int (*cpu_state)(const void *state, struct CpuState *out);`, which
  returns 1 when filled.
- `circuit.h`:
  `int circuit_cpu_state(const Circuit *circuit, size_t part, CpuState *out);`
  It returns 0, or -1 with `circuit_error` set ("part N is not a
  processor") for other kinds, the same as the memory wrappers.
- Registers, 6502 family, in this order: `A`, `X`, `Y`, `S` (8 bits), `P`
  (8 bits, bit 5 and B as the core holds them), `PC` (16 bits).
- `WAITING` and `STOPPED` are W65C02S WAI and STP. `JAMMED` is an NMOS JAM
  opcode: the core's current opcode is one of the 12 of plan-09-A §4.4.
  `BUS_RELEASED` is Z80 only (C).

### 4.4 The 6502-family adapter: clock, reset, lost

Inputs that read Z count as inactive: IRQ/NMI/RES/SO high, RDY high, BE
high (the SRAM/EEPROM precedent). X is unknown.

**Status at start: LOST.** Address, data, R/W, SYNC, VPB and MLB are driven
X (strong). PHI1O/PHI2O follow the clock from the start.

**Clock pin changes** (any time). Drive `phi2Out` = the clock and
`phi1Out` = its complement (X stays X), with no added delay; the sheets give
no input-to-output delay. Then:

- **PHI2↑ at t:**
  - Lost if the low time since the last PHI2↓ is below `tpwl`.
  - Otherwise, if the current bus cycle is a write and BE is high: drive
    D0–D7 X at t and the write data at t + `tmds`.
- **PHI2↓ at t:**
  - Lost if the high time is below `tpwh`, or the time since the previous
    PHI2↓ is below `tcyc`.
  - Otherwise run **the commit** below.
- **Clock X:** lost.

**The commit** (at PHI2↓, time t). Samples:

- the data pins, if the current cycle is a read;
- RDY, IRQ, NMI, RES and SO.

A sampled pin that changed within `tdsr` (data) or `tpcs` (controls) before
t counts as X.

1. **Write release.** If the current cycle is a write, drive D0–D7 Z at
   t + `tdhw`.
2. **Reset.** RES = X → lost; stop. RES = 0 → status RESET:
   - count this cycle;
   - drive address X, data Z, R/W 1, SYNC 0, VPB 1, MLB 1 (from
     t + `tah`);
   - stop.

   RES = 1 while status is RESET:
   - if at least 2 cycles were counted: `init` the core, then tick once,
     which gives the first reset-sequence cycle. Status RUNNING, cycles 0.
     Go to step 7.
   - else lost; stop.
3. **Lost or reset**: stop. Only a reset leaves LOST.
4. **RDY.** RDY = X → lost; stop. RDY = 0 holds the cycle:
   - on a read cycle (NMOS and 65C02);
   - on a write cycle (65C02 only; the NMOS chip ignores RDY on writes).

   While holding: no tick, outputs unchanged, `cycles` +1; stop.
5. **SO.** A 1→0 change of SO between the previous commit and this one
   sets V in the core's P before the tick. SO = X → lost, unless V is
   already 1.
6. **Tick, with the differential rule.** Build the input pins:
   - data bits (read cycle only);
   - IRQ = (IRQ pin = 0);
   - NMI = (NMI pin = 0);
   - RDY and RES are never passed (steps 2 and 4).

   If any of those bits is X, tick two copies of the core, one with every X
   bit 0 and one with every X bit 1. Compare both:
   - the whole core struct, with the data bits and the IRQ bit of its
     stored `PINS` masked;
   - and the returned pins.

   Equal: continue with the first copy. Different: lost; stop.
   With no X bit, tick once. `cycles` +1.
7. **Drive the next cycle** from the returned pins:
   - for each address bit and R/W that changes: X at t + `tah`, the value
     at t + `tads` (transport drives);
   - SYNC: X at t + `tah`, its value at t + `tsync`;
   - VPB and MLB (65C02, §4.5): their value at t + `tads`;
   - all of these only while BE is high;
   - a write cycle's data is driven at the next PHI2↑ (above).
8. **Status.** RUNNING, or WAITING/STOPPED from the core's flags (65C02),
   or JAMMED (NMOS).

**Lost entry**, from any rule: status LOST, `register_count` 0. Drive
address, data, R/W, SYNC, VPB and MLB X (strong) now; Z while BE is low.

### 4.5 65C02 extras

- **VPB** low on a read cycle whose address is `FFFA`–`FFFF` while the
  core's opcode is `00` (BRK, and the forced IRQ/NMI/RESET sequence). The
  opcode is `IR >> 3`, as the core sets it.
- **MLB** low on the three bus cycles of a read-modify-write: the read of
  the effective address, the repeat read, and the write. When the new cycle
  is a read and the opcode is in the RMW set below, the adapter ticks a
  copy of the core two more times (inputs = the current pins). If those two
  cycles are a read and then a write at the same address, MLB goes low for
  this cycle and the next two.

  RMW set, 52 opcodes:
  - `06 0E 16 1E` ASL, `26 2E 36 3E` ROL, `46 4E 56 5E` LSR,
    `66 6E 76 7E` ROR, `C6 CE D6 DE` DEC, `E6 EE F6 FE` INC;
  - `04 0C 14 1C` TSB/TRB;
  - `07 17 … F7` RMB/SMB (16 opcodes).
- **BE.** A change of BE schedules, at + `tbvd`:
  - BE 0: address, data and RWB Z;
  - BE 1: their current values (data only during a write's data window);
  - BE X: X.

  The CPU keeps running while BE is low.
- **WAI.** While the core's `wait_flag` is set, drive RDY 0 (strong) from
  the commit that set it; release it (Z) at the commit that clears it.
  STP: status STOPPED until reset.

### 4.6 Instruction tracker (shared with C)

- At each commit whose returned pins start an instruction (6502: SYNC; Z80
  in C: an M1 fetch with no prefix active), the previous instruction is
  complete. The tracker:
  - disassembles its bytes with the vendored `*dasm_op` into
    `instruction_text`;
  - sets `has_instruction`.
- The new instruction starts at that cycle's address. Each read cycle at
  start + n (n < 4) inside the instruction appends its sampled data byte.
- On a write, or a read at another address, recording stops until the next
  start.

### 4.7 Fixture devices (`cpu_test`)

Two inline devices with exactly D's data (plan-09-D §4.2):

- `W65C02S`: PDIP-40 pinout, grade `14`;
- `MCS6502`: PDIP-40 pinout, grade `1`.

The 1 MHz bench clock (period 1000 ns, 500 ns high) meets both grades.

## 5. Tests (`backend/tests/cpu_test.c`)

The bench is a component in the `eeprom_test.c` style. It:

- drives the clock (1 MHz, 50 % duty, unless a case says otherwise);
- serves a 64 KB array: on a read it drives the byte from PHI2↑ + 100 ns
  until PHI2↓ + 20 ns; on a write it stores the data sampled at PHI2↓;
- holds every other input at a value the case sets;
- logs each cycle as (address, R/W, SYNC, VPB, MLB, data), sampled at
  PHI2↓ − 1 ps.

Reset is RES held 0 for 3 cycles, then 1. The reset vector `FFFC` →
`C000`.

### 5.1 Both cores

1. **Power-up lost.** Run 5 cycles with no reset. Address, R/W and SYNC
   read X; `circuit_cpu_state` status LOST, `register_count` 0.
2. **Reset sequence.** The cycles after release are reads at `0000`,
   `0100`, `01FF`, `01FE`, `FFFC`, `FFFD`, then a SYNC read at `C000`.
   This was measured on both unmodified cores (`/tmp/p08/run/rst.c`,
   2026-11-01): after `init`, PC = 0 and S = 0. The bench serves X at
   `0000` and `01xx`, and the CPU stays RUNNING (the differential rule).
3. **Short reset.** RES 0 for 1 cycle, then 1: status stays LOST.
4. **Address window.** After the PHI2↓ that ends the SYNC cycle of
   `A9 41`: A0 reads X at t + `tah` + 1 ns, and the new address at
   t + `tads` + 1 ns. Before t + `tah` the old address is still there.
5. **Write timing.** `8D 00 02` (STA $0200) with A = `41`:
   - R/W 0 at t + `tads`;
   - D0–D7 X at PHI2↑, `41` at PHI2↑ + `tmds`;
   - Z at PHI2↓ + `tdhw`;
   - the bench stores `41` at `0200`.
6. **Used X → lost.** `AD 00 03` (LDA $0300) with the bench serving X at
   `0300`: LOST.
7. **Setup violation.** The opcode byte changes `tdsr` − 1 ns before PHI2↓
   on a SYNC cycle (`A9` to `EA`): LOST. The same change `tdsr` + 1 ns
   before it: RUNNING, and the opcode executed is `EA` (NOP).
8. **IRQ and NMI.**
   - `58` (CLI), then IRQ 0: the vector reads are at `FFFE`/`FFFF`, and
     the pushed P has B = 0.
   - NMI 1→0: `FFFA`/`FFFB`.
   - NMI held at 0 does not re-trigger.
9. **RDY on a read.** RDY 0 during the cycle that reads `0300`: the same
   address repeats for every held cycle, and `cycles` grows.
10. **SO.** SO 1→0: P has V set (`circuit_cpu_state`).
11. **State read-back.** After `A9 41 A2 05 A0 07`:
    - registers A `41`, X `05`, Y `07`;
    - last instruction `LDY #$07` at `C004`, bytes `A0 07`;
    - the cycle count equals the logged cycles since reset.
12. **Clock too fast.** One high phase shorter than `tpwh`: LOST.
    `tcyc` too short: LOST.

### 5.2 W65C02S only

13. **RDY on a write.** RDY 0 during STA's write cycle: R/W stays 0 and the
    address holds; the bench stores once, at release.
14. **VPB.** Low exactly on the `FFFE`/`FFFF` reads of an IRQ, and on the
    `FFFC`/`FFFD` reads of reset.
15. **MLB.** `EE 00 03` (INC $0300): MLB low on exactly the read, the
    repeat read and the write at `0300`; high on every other cycle.
16. **BE.** BE 0 mid-program: address, data and RWB read Z after `tbvd`.
    The CPU keeps running: the cycle count advances. BE 1 restores the
    pins.
17. **WAI.** `CB`: status WAITING and RDY driven 0. IRQ 0 with I set: it
    resumes at the next instruction (no vector). STP `DB`: status
    STOPPED, and only a reset leaves it.
18. **Program.** `A9 41 8D 00 02 DB`: the outcome at the top of this
    sub-plan (the store, A = `41`, status Stopped, last instruction `STP`).

### 5.3 NMOS only

19. **RDY on a write is ignored.** RDY 0 during STA's write cycle: the write
    completes in one cycle.
20. **JAM.** Opcode `02`: status JAMMED.

### 5.4 `library_check` rejection cases

Each one is a fixture with one fault. Each message names the place, e.g.
`cpu.json: devices[0] (X): …`.

- unknown core `"6809"`;
- `"vectorPull"` on a `"6502"` device;
- a missing `"sync"`;
- 15 address names;
- 7 data names;
- a control naming a pin absent from one pinout;
- `tcyc` 0;
- a part `"grade"` not in `grades`;
- `"inputMax"` below the grade's `vcc` max (plan-07 rule).

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

### Phase 1 — Kind, facts, state API

A loadable kind with no behaviour beyond "lost", landable alone.

- [x] `parts.h`: `PART_CPU`, `PartCpu`, `Part.cpu` (§4.1).
- [x] `include/cpu_state.h`; `sim.h` hook; `circuit.h`/`circuit.c`
      `circuit_cpu_state` (§4.3). `cpu_test` case 1 → "circuit_cpu_state
      reports LOST" and "on a gate: \"part 1 is not a processor\"" PASS.
- [x] `models/cpu.c` (+ the internal header `models/cpu.h`):
  - `CPU_KIND` with the 6502-family keys and the plan-07 voltage keys;
  - `CPU_MODEL` dispatching to a `CpuCoreOps` table;
  - for now, a core whose `start` drives every output X and reports
    LOST (placed in `models/cpu6502.c`, the file Phase 2 fills; see
    Corrections).
- [x] `kinds.c`: `&CPU_KIND`. `backend/CMakeLists.txt`: `models/cpu.c` and
      `models/cpu6502.c`.
- [x] `listing.c`: kind name and details. `trace_check.c`: `TRACE_LAST_KIND`
      → `PART_CPU`. `listing_check`, `trace_check` pass; golden diff empty.
- [x] `library_check.c`: the §5.4 cases, plus a valid 65c02 and a valid
      6502 file (`max_hz` 14285714, vcc 1710–5250). All 11 print `ok` with
      the §5.4 messages (e.g. `devices[0] (C): pinouts.p40b: so: no pin
      named "SOB"`, `parts[0] (C1): "inputMax" 5 is below the "vcc" max 5.25`).
- [x] `tests/cpu_test.c` with the fixture devices (§4.7), case 1 only.
      `logic_test(cpu_test)`. `/tmp/logic-p09/cpu_test` → 7 PASS, 0 FAIL.

Acceptance: the fixture loads, and a part reads back LOST; every loader
rule rejects with its message; the goldens are unchanged.
  Check: `cmake --build /tmp/logic-p09 -j8 && ctest --test-dir /tmp/logic-p09 -R 'library_check|cpu_test|listing_check|trace_check'` → 4/4 passed; `git diff --stat backend/tests/golden` → empty (est. 2 min).
Commit: 3441b51

### Phase 2 — The 6502-family adapter

- [x] `models/cpu6502.c`: §4.4–§4.5 for `6502` and `65c02`. Fresh build
      (`/tmp/logic-land-p09b`) → 0 warnings; `cpu_test` cases 2–20 PASS.
- [x] `models/cpu.c`: the instruction tracker (§4.6) and `state` for the
      6502 family (§4.3). Case 11 → `LDY #$07` at C004 and `JSR $C100`
      (bytes 20 00 C1) PASS; case 18 → `STP` PASS.
- [x] `cpu_test.c`: cases 2–20 (§5.1–§5.3). `/tmp/logic-p09/cpu_test` →
      81 PASS, 0 FAIL, `cpu_test: all passed`.
- [x] `docs/parts-format.md`: the `cpu` row in the kind table, and a § `cpu`
      section:
  - the keys of §4.1;
  - the behaviour of §4.4–§4.5, written the way the `eeprom` section is;
  - the Non-goals list.
- [x] `docs/backend.md`:
  - `circuit_cpu_state` in the Notes;
  - `cpu.c`, `cpu6502.c` and the `chips_*` wrappers in Layout;
  - `cpu_test` in Tests.

Acceptance: every case of §5.1–§5.3 passes, each at the datasheet times it
names.
  Check: `ctest --test-dir /tmp/logic-p09 -R cpu_test -V | grep -c ': PASS'` → the number of `expect` calls the run makes (81: 50 call sites in the file, the §5.1 ones run once per core), and `grep -c ': FAIL'` → 0 (est. 1 min). Measured: 81 and 0.
Commit: —

## Validation Plan

- Tests: `cpu_test` (behaviour, negative cases 3, 6, 7, 12) and
  `library_check` (rejections).
- Coverage check: `cpu6502.c` is reached only through the fixture devices
  in `cpu_test`. No golden covers it until D adds parts, so every rule in
  §4.4–§4.5 has its own case (map in §5).
- Runtime proof: case 18. A program runs from reset to STP through the
  real engine, with the bench as memory.
- Doc sync: `docs/parts-format.md` § `cpu`, `docs/backend.md`.
- Final gate (once, after Phase 2):
  `rm -rf /tmp/logic-land-p09b && cmake -S backend -B /tmp/logic-land-p09b && cmake --build /tmp/logic-land-p09b -j8 && ctest --test-dir /tmp/logic-land-p09b`
  → `100% tests passed`, T0 + 2 tests; `git diff --stat backend/tests/golden`
  → empty (est. 2 min). Plus `xcodebuild … -derivedDataPath /tmp/p09-dd … build`
  → `** BUILD SUCCEEDED **` (est. 2 min; the new enum value and `Part.cpu`
  reach Swift).

## Decisions

See plan-09-A § Decisions (owner). Author decisions for B are recorded
under § 3 Rejected alternatives: one kind, adapter-side RDY, the
differential rule, no read-hold check, transport drives.

## Corrections

- **Phase 1 stub location.** The LOST-only core is `CPU6502_OPS` in
  `models/cpu6502.c` (added to CMake in Phase 1), not a core inside `cpu.c`:
  Phase 2 then replaces that file's body and `cpu.c` does not change shape.
  The adapter interface lives in a new internal header `models/cpu.h`
  (§4.2 names its contents but not the file).
- **Fixture names and footprint** (§4.7). The fixture devices carry D's
  data, but their ids and part names are prefixed `FIXTURE-`
  (`FIXTURE-W65C02S6TPG-14`, `FIXTURE-MPS6502`), and they name a fixture
  footprint `FIXTURE-PDIP-40` loaded by `cpu_test`. `PDIP-40-600` does not
  exist until D (`grep -c '"PDIP-40-600"' footprints.json` → 0), every
  non-supply part needs a footprint (`library.c`: `missing "footprint"`),
  and D's system tests load the real `cpu.json` into the same library, where
  the real names must not clash.
- **Same-instant input changes** (§4.4 setup rule). Measured: case 3
  failed at first. Bench and test drive RES 0 at t = 0, the same engine
  batch as the clock's first edge. The engine calls `pin_changed` net by
  net, so the clock's commit ran before RES's change was recorded, saw
  RES = 0 with no change time, and counted a reset cycle. `cpu_sample` now
  also takes the value `pin_changed` last reported for the pin; a different
  value now is a change at this instant, so it reads X. Case 3 PASS after.
- **Instruction tracker** (§4.6). "A write, or a read at another address,
  stops recording" loses operand bytes read after a stack access. The core's
  JSR is opcode, low byte, a stack read, two pushes, then the high byte
  (`m6502.h:1069-1074`). The tracker instead appends each read at
  start + n (n = bytes so far, n < 4) at any point in the instruction, and
  ignores other cycles. The disassembler takes the bytes it needs, and
  `instruction_length` is what it took. Case 11 gained a JSR check
  (`JSR $C100`, bytes `20 00 C1`). Also:
  - An instruction that never reaches another SYNC (JAM) completes when the
    CPU jams.
  - A SYNC while STP or WAI holds the core (the core returns the same pins)
    starts nothing, so the last instruction stays `STP` / `WAI` (case 18).
- **RDY while waiting** (§4.4 step 4). WAI drives RDY low itself (§4.5), so
  step 4 would hold every cycle and the core would never see the interrupt
  that ends the wait. RDY is not checked while the status is WAITING.
  Case 17 PASS.
- **BE gates address, data and RWB only** (§4.4 step 7 said "all of
  these", which includes SYNC, VPB and MLB). §4.5 and the W65C02S sheet
  float only address, data and RWB, and SYNC/VPB/MLB keep being driven.
- **Case 12, tCYC on the W65C02S.** Its `tcyc` (70) equals `tpwh` + `tpwl`
  (35 + 35), so a period below it always has a short phase. The case runs
  a period of tCYC − 10 ns with 30 ns phases (LOST). The NMOS grade 1 run
  isolates tCYC: 480 + 460 = 940 ns, both phases legal.
- **Case 13, "the bench stores once".** The bench stores at every PHI2 fall
  with R/W low, like a memory strobed by R/W and PHI2. A write held by RDY
  is therefore sampled on every held cycle. The checkable form is the
  stronger one now in the case: 0200, R/W 0 and data 41 on every held
  cycle; after release, memory holds 41 and the next cycle is the fetch at
  C005.
- **Case 16, BE.** With BE low the CPU's R/W floats, so the bench cannot
  tell a read from a write; but the CPU keeps fetching. The bench acts as
  the other bus master: it drives address C000 and R/W 1 while BE is low, so
  the CPU reads NOPs. It releases them when BE returns.
- **Case 17, reset leaves STP.** The program is `DB` at C000, so after the
  reset the CPU runs STP again and stops. The case checks RUNNING during the
  reset sequence (the FFFC read), not after it.
- **Case 1 also covers the circuit API**: `circuit_cpu_state` on a running
  circuit (LOST) and on a gate (`part 1 is not a processor`), so the public
  wrapper of §4.3 has a test in B.

## Summary

B is where the 6502 timing lives. The risk is in the commit sequence of
§4.4: which samples count as X, when the CPU becomes lost, and the windows it
drives. Each has a datasheet-timed test. Existing parts and goldens are
untouched.
