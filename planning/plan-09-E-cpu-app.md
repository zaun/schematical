# plan-09-E: CPUs in the app — catalog, sidebar facts and the processor window

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-09-D

This sub-plan brings the processors (plan-09-D) into the app:

- The parts drawer lists them under **Processor**, with their maximum clock.
- The schematic sidebar (plan-08) shows their facts.
- Double-clicking a placed CPU opens a read-only **processor window**. It
  shows the status, every register, the flags, the last completed
  instruction (address, bytes, disassembly) and the cycle count, updated
  after every simulation frame. This is the same mechanism as the memory
  window.

Outcome, in the headless harness:

- A 5V board runs an EEPROM program:
  - W65C02S6TPG-14, AT28C256-15PU, AS6C62256-55PCN and SN74HC00N, wired as
    in plan-09-D §5;
  - clocked by an NJU6366DF1-TE1 at 2 MHz;
  - reset with a TE-GDS0204 switch.
- Before the reset switch is cycled, the processor window reads "Unknown —
  waiting for reset". Afterwards it reads Stopped, with A `10`, X `FF` and
  last instruction `STP`.
- The Z80 board's window shows Halted, its 18 registers and `HALT`.

References:

- plan-09-B §4.3 (`CpuState`, `circuit_cpu_state`), plan-09-D §4.2 and §5
  (parts, boards).
- `planning/plan-08-schematic-sidebar.md` (by then archived under
  `planning/completed/`): `Backend/PartFacts.swift`, `PartFacts.rows(for:)`.
- `planning/completed/plan-04-C-eeprom-app.md` (memory window, harness
  commands, EEPROM load file) and `planning/completed/plan-06-A-board-model.md`
  (§ Harness, `build.sh`).
- App files:
  - `Backend/PartCatalog.swift` (`Details`, `detailLabels`, `isMemory`,
    `CatalogPart.init`);
  - `Memory/MemoryWindow.swift` (`MemoryWindowTarget`,
    `SchematicSessions`);
  - `Canvas/SchematicCanvasView.swift` (double-click, `onOpenMemory`);
  - `ContentView.swift` (`memoryWindows`, `openWindow`, `dismissWindow`);
  - `schematicalApp.swift` (the `"memory"` `WindowGroup`);
  - `Simulation/SimulationEngine.swift` (`readMemory` under the lock);
  - `Simulation/Simulation.swift` (`stateVersion`, `readMemory(of:)`).

## Prerequisites

plan-09-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-09-D complete | `ls planning/plan-09-D-* 2>/dev/null` → no output; `ls planning/completed/plan-09-D-*.md` → exists | MET (2026-11-01 at 5adb55b; plan-09-A's rows re-run at 1da038c, all MET) |
| plan-08's facts table exists | `grep -n 'static func rows(for' schematical/schematical/Backend/PartFacts.swift` → one match | MET (2026-11-01 at 5adb55b: `PartFacts.swift:6`) |
| The proof board's parts accept 5 V | `grep -E '^(NJU6366DF1-TE1\|AT28C256-15PU\|AS6C62256-55PCN\|SN74HC00N\|W65C02S6TPG-14\|Z84C0010PEG)\|' backend/tests/golden/parts.txt` → 6 lines, each with a `vcc=<min>-<max>` where min ≤ 5000 ≤ max | MET (2026-11-01 at 5adb55b: NJU6366DF1-TE1 2000–5500, AT28C256-15PU 4500–5500, AS6C62256-55PCN 2700–5500, SN74HC00N 2000–6000, W65C02S6TPG-14 1710–5250, Z84C0010PEG 4500–5500). From the datasheets: NJU6366 2.0–5.5 V ("Operating Voltage", `datasheets/osc/NJU6366 Series.pdf`), AT28C256 5 V ± 10 %, AS6C62256 2.7–5.5 V, HC 2–6 V, W65C02S 1.71–5.25 V, Z84C00 4.5–5.5 V. |
| The supply part `5V` exists (plan-07-A/G) | `grep -c '^5V|' backend/tests/golden/parts.txt` → 1 | MET (2026-11-01 at 5adb55b: 1) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p09e-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01 at 5adb55b, `-derivedDataPath /tmp/p09-dd`, plan-09-D's gate) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of *all*
> prerequisites.**

## 1. Goal

- Harness `catalog`, with obsolete parts shown: `Processor: 19 parts;
  functions 6502 (4), 65C02 (2), Z80 (13)`. With the default Settings
  (obsolete hidden): `Processor: 2 parts`. `W65C02S6TPG-14: 14 MHz`.
- Harness `facts W65C02S6TPG-14` prints the rows of §4.2.
- Harness `cpu` produces the §5 outputs and PNGs.

### Non-goals (explicit constraints)

- No backend or part-data change (B–D are done). No file-format change: a
  CPU has no saved settings, unlike an EEPROM's load file.
- Read-only: no register editing, no breakpoints, no single-step. The owner
  chose the window without stepping controls.
- No change to the memory window, and no change to obsolete-part visibility
  (Settings → "Obsolete parts" stays off by default).

## 2. Current State

- `CatalogPart.Details` has 8 cases (`PartCatalog.swift:124-133`); plan-08
  does not add any. `CatalogPart.init` picks the case from the non-NULL info
  pointer, ending in `else { details = .supply(level: 0) }`. A `PART_CPU`
  part therefore shows as a supply until this sub-plan.
- There are four exhaustive `switch details` sites
  (`PartCatalog.swift:164,183,193,204`; `grep -n 'switch details'` → 4),
  plus plan-08's `PartFacts.rows`.
- `symbol(for:)` (`PartCatalog.swift:431`) defaults to `.block`, so CPUs draw
  as blocks with no change. Pins group by `group`: address and data buses
  get their own column runs (`SchematicModel.swift:344-361`).
- Memory window wiring:
  - double-click → `onOpenMemory(chip.id)` when `chip.part.isMemory`
    (`SchematicCanvasView.swift:93-96`);
  - → `ContentView` inserts a `MemoryWindowTarget` and calls
    `openWindow(id: "memory", value:)` (`ContentView.swift:112-115`). It
    dismisses them when the schematic window closes (`:288-291`).
  - `MemoryWindowView` finds its session through
    `SchematicSessions.shared` and re-reads on `simulation.stateVersion`
    (`Simulation.swift:177`).
- `SimulationEngine.readMemory(part:…)` takes the engine lock and calls the
  C API (`SimulationEngine.swift:230-239`).
- The headless harness (plan-06-A § Harness, plan-04-C phases 1–3) compiles
  the app's Swift sources, except `schematicalApp.swift`, into a CLI. Its
  commands print state and render views to PNG.

### Measured populations

| What | Count | Command |
|---|---|---|
| `switch details` sites | 4 | `grep -n 'switch details' schematical/schematical/Backend/PartCatalog.swift` |
| Window groups | 2 (main, `"memory"`) + Settings | `grep -n 'WindowGroup' schematical/schematical/schematicalApp.swift` |
| CPU registers shown | 6 (6502 family), 18 (Z80) | plan-09-B §4.3, plan-09-C §4.1 |

## 3. Design Overview

1. **Catalog:** a `.cpu` details case, labels, `isCpu`; the drawer's
   **Processor** group then follows from the `drawer` key.
2. **Facts:** `PartFacts` rows for `.cpu`.
3. **State plumbing:** `SimulationEngine.cpuState(part:)` and
   `Simulation.cpuState(of:)` return a Swift `CpuSnapshot` copied out of the
   C `CpuState`.
4. **Window:** `Cpu/CpuWindow.swift`, built and opened like the memory
   window.

Risk is low and local to the app. The C struct copy (fixed-size C arrays
import as tuples) is the one place a mistake hides; the harness prints
every field. The gate is the harness output and PNGs, not byte-identity.

Rejected alternatives:

- **Folding CPU state into `SimulationSnapshot` every frame.** It is read
  only while a window is open. Reading on `stateVersion` (the memory-window
  pattern) costs nothing otherwise.
- **One generic "inspector" window for memories and CPUs.** The two show
  unrelated data; separate small windows keep `MemoryWindow.swift`
  untouched.

## 4. Detailed Design

### 4.1 Catalog (`Backend/PartCatalog.swift`)

- `Details.cpu(core: String, grade: String, maxHz: UInt64)`, filled from
  `part.cpu?.pointee` before the final `else`.
- `detailLabels`: `["\(maxHz / 1_000_000) MHz"]`, e.g. `14 MHz`,
  `1 MHz` (MPS6502), `20 MHz` (Z84C0020…).
- `var isCpu: Bool { kind == PART_CPU }`.
- The three other `switch details` sites return `nil` for `.cpu`.

### 4.2 Facts (`Backend/PartFacts.swift`, plan-08)

- Kind: `cpu` → `"Processor"`.
- Kind facts for `.cpu`, in order:
  - **Core**: `6502` → `"6502 (NMOS)"`, `65c02` → `"65C02 (WDC)"`,
    `z80` → `"Z80"`.
  - **Speed grade**: the grade.
  - **Max clock**: `"\(maxHz / 1_000_000) MHz"`.

So `facts W65C02S6TPG-14` adds `Core: 65C02 (WDC)`, `Speed grade: 14` and
`Max clock: 14 MHz` to plan-08's common rows. The common rows are Kind
`Processor`, Function `65C02`, Package `PDIP`, Footprint `PDIP-40-600 — …`
and Pins `40`. `isHidden` is `role == .absent` (`PartCatalog.swift:53`),
and the W65C02S PDIP has no absent pin; its NC pin 35 is counted.

### 4.3 State plumbing

- `Simulation/SimulationEngine.swift`: `func cpuState(part: Int) ->
  CpuSnapshot?`. It locks, then calls `circuit_cpu_state`; nil on -1 or
  after destroy.
- `Simulation/Simulation.swift`: `func cpuState(of chipID: UUID) ->
  CpuSnapshot?`, the same shape as `readMemory(of:)`.
- `CpuSnapshot` (new, in `Cpu/CpuWindow.swift`):
  - status;
  - cycles;
  - `[(name: String, value: UInt32, bits: Int)]`;
  - instruction (`address`, `bytes: [UInt8]`, `text`) or nil.

  Copied with `withUnsafeBytes` over the C tuple fields.

### 4.4 Window (`Cpu/CpuWindow.swift`)

- `CpuWindowTarget { sessionID, chipID }` (Codable, Hashable), the twin of
  `MemoryWindowTarget`.
- `schematicalApp.swift`: `WindowGroup("Processor", id: "cpu", for:
  CpuWindowTarget.self)`, `.defaultSize(width: 420, height: 360)`,
  `.restorationBehavior(.disabled)`.
- `SchematicCanvasView`: double-click on `chip.part.isCpu` →
  `onOpenCpu(chip.id)`.
- `ContentView`: a `cpuWindows` set, opened with `openWindow(id: "cpu",
  value:)` and dismissed with the memory windows.
- Title: `"<reference> — <part number>"`, as the memory window.
- Content, top to bottom:
  - **Status**:
    - Running · `<cycles>` cycles
    - Unknown — waiting for reset
    - Held in reset
    - Waiting for interrupt (WAI)
    - Stopped (STP)
    - Halted (HALT)
    - Jammed — reset to continue
    - Bus released (BUSACK)

    While not running: "Run the simulation to see the processor's state."
  - **Registers**, a monospaced grid of `name value`, the value in hex with
    `bits / 4` digits (1 bit shows 0/1). Order as in `CpuState`. Hidden
    while the register count is 0.
  - **Flags**, one cell per bit, lit when set:
    - 6502 family: `P` as `N V – B D I Z C`;
    - Z80: the low byte of `AF` as `S Z Y H X P/V N C`.
  - **Last instruction**: `C004  A0 07  LDY #$07` (address, bytes, text),
    or "—".
- Re-reads on `simulation.stateVersion`, as `MemoryWindowView` does.

## 5. Runtime proof (harness `/tmp/p09/harness`, plan-06-A's `build.sh`)

Fixtures in `/tmp/p09-proof/`:

- `prog6502.bin`: 32768 bytes. The plan-09-D §5 W65C02S program at offset
  0, `00 80` at `7FFC`, `FF` elsewhere.
- `progz80.bin`: the plan-09-D §5 Z80 program at offsets 0 and `0020`,
  `FF` elsewhere.
- `cpu6502.schematical` and `cpuz80.schematical`: the plan-09-D §5 boards
  built from app parts:
  - `5V` and `GND` supplies;
  - NJU6366DF1-TE1 (2 MHz) FOUT → the CPU clock;
  - TE-GDS0204 position 1 between the CPU's reset pin and GND (OFF at
    start, so reset reads Z = inactive);
  - the EEPROM's load file = the matching `.bin` (plan-04-C).

  Written with the harness's document API and saved.

Harness commands:

1. `catalog`: the §1 lines, with and without obsolete parts.
2. `facts W65C02S6TPG-14`: the §4.2 rows.
3. `cpu cpu6502.schematical`:
   - Run; advance 50 µs. `cpuState` → status LOST (printed `Unknown —
     waiting for reset`); render `cpu-lost.png`.
   - Switch 1 ON, advance 5 µs, OFF; advance 2 ms. `cpuState` → Stopped,
     A `10`, X `FF`, S `FF`, last `STP`. `readMemory` of the SRAM at
     `0200` → `10`. Render `cpu-6502.png`.
4. `cpu cpuz80.schematical`: the same reset dance. Then Halted, 18
   registers, `BC`/`A` values per plan-09-D §5, last `HALT`. Render
   `cpu-z80.png`.
5. Stop: the window reads "Run the simulation…"; `cpu-stopped.png`.

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

### Phase 1 — Catalog and facts

- [x] `PartCatalog.swift`: §4.1. `xcodebuild … -derivedDataPath /tmp/p09e-dd`
      → `** BUILD SUCCEEDED **`.
- [x] `PartFacts.swift`: §4.2. `run facts W65C02S6TPG-14` → Kind
      Processor, Function 65C02, Package DIP, Footprint PDIP-40-600 — …,
      Pins 40, Core 65C02 (WDC), Speed grade 14, Max clock 14 MHz, Supply
      range 1.71–5.25 V, Input max 5.25 V.
- [x] Harness (`/tmp/p09/harness/build.sh`, plan-06-A approach): commands
      `catalog` and `facts`. `run catalog` → `obsolete shown: Processor: 19
      parts; functions 65C02 (2), 6502 (4), Z80 (13)`, `obsolete hidden:
      Processor: 2 parts; functions 65C02 (2)`, `W65C02S6TPG-14: 14 MHz`,
      `MPS6502: 1 MHz`, `Z84C0020AEG: 20 MHz`.

Acceptance: the drawer group and facts print as §1.
  Check: `xcodebuild … -derivedDataPath /tmp/p09e-dd … build` → `** BUILD SUCCEEDED **` (est. 2 min); `/tmp/p09/harness/run catalog` and `run facts W65C02S6TPG-14` → the §1 / §4.2 lines (est. seconds).
Commit: 741a628

### Phase 2 — State plumbing and the processor window

- [x] `SimulationEngine.swift`, `Simulation.swift`: §4.3. `CpuSnapshot`
      copies the C tuples with `withUnsafeBytes`; the harness prints every
      field (below).
- [x] `Cpu/CpuWindow.swift`, `schematicalApp.swift`, `SchematicCanvasView.swift`,
      `ContentView.swift`: §4.4. `xcodebuild … /tmp/p09e-dd` → `** BUILD
      SUCCEEDED **` (the synchronized folder compiles `Cpu/`).
- [x] Harness: the `cpu` command and the fixtures of §5 (`run fixtures`
      writes the two `.bin` and `.schematical` files: 8 parts, 71 / 73
      wires, no refused connection). `run cpu cpu6502.schematical`:
      - at 50 µs: `Unknown — waiting for reset`, 0 registers, last `—`;
      - after the switch and 2 ms: `Stopped (STP)`, `A=10 X=FF Y=00 S=FF
        P=07 PC=8013`, last `8012  DB  STP`, SRAM 0200 = `10`.
      `run cpu cpuz80.schematical`:
      - LOST first;
      - then `Halted (HALT)` with 18 registers (`AF=1042` → A 10, `BC=FFFF`
        at its reset value, `SP=0000`);
      - last `0010  76  HALT`, SRAM 0200/0201 = `10 10`;
      - after Stop, `cpuState` nil.
      The four PNGs were looked at: each status line, register grid, flag
      row and last instruction reads as stated.

Acceptance: §5 steps 3–5 print their values and write the four PNGs. Each
PNG is looked at: the status line, the register grid and the last
instruction all read as stated.
  Check: `/tmp/p09/harness/run cpu /tmp/p09-proof/cpu6502.schematical` and `run cpu /tmp/p09-proof/cpuz80.schematical` → the §5 values; `ls /tmp/p09-proof/*.png` → `cpu-6502.png cpu-lost.png cpu-stopped.png cpu-z80.png` (est. 1 min). Measured as in the task above; `ls` → the four PNGs.
Commit: —

## Validation Plan

- Tests: none in Swift (no test target). The backend paths are covered by
  `cpu_test` (B–D); the app paths by the harness.
- Coverage check: each harness step drives one new path:
  - catalog (`.cpu` details);
  - facts;
  - `cpuState` for each core;
  - the LOST, Stopped, Halted and stopped-simulation window states.
- Runtime proof: §5, plus on the desktop (`run` skill, signed Debug
  build):
  - open `cpu6502.schematical`;
  - Settings shows the W65C02S under Processor;
  - double-click U1, Run, flip the reset switch on and off;
  - the window goes from "Unknown — waiting for reset" to Stopped with A
    `10`.

  Screenshot `/tmp/p09-proof/desktop-cpu.png`.
- Doc sync: `docs/backend.md` § Using it gains one line on the processor
  window's data source (`circuit_cpu_state`). No file-format change.
- Final gate for plan-09 (once, after Phase 2):
  `rm -rf /tmp/logic-land-p09 && cmake -S backend -B /tmp/logic-land-p09 && cmake --build /tmp/logic-land-p09 -j8 && ctest --test-dir /tmp/logic-land-p09`
  → `100% tests passed`, T0 + 2 tests (est. 2 min). Then the `xcodebuild`
  (est. 2 min), and the harness §5 re-run on the final code (est. 2 min).

## Decisions

Owner: the read-only processor window without stepping (plan-09-A
§ Decisions). Author: § 3 Rejected alternatives (read on `stateVersion`; a
separate window from the memory window).

## Corrections

- **Package `DIP`** (§4.2 said Package `PDIP`). plan-09-D wrote the PDIP
  parts as `"DIP"` to match the library (plan-09-D Corrections), so the
  facts row reads `Package: DIP`.
- **Function order in the drawer.** §1 lists `6502 (4), 65C02 (2), Z80
  (13)`. The drawer orders a group's functions itself and gives `65C02 (2),
  6502 (4), Z80 (13)`. The counts and the total (19; 2 with obsolete parts
  hidden) are as planned.
- **Advance steps overshoot.** §5 says "advance 50 µs", "5 µs", "2 ms".
  The app's engine runs on its own thread, paced by the speed setting. The
  harness sets the speed to 1000 (flat out) and pumps the run loop until
  `circuitTime` passes each target, so it lands later: LOST read at 458 µs
  and 375 µs; reset held about 5 ms and 15 ms. Every state the plan names
  holds over those spans: LOST until reset, at least 3 reset cycles. The
  values match §5.
- **PNG capture.** The first captures came out blank but for the lit flag
  cells. The harness ran in Dark Mode with no window background, so the
  text was white on transparent. The harness now renders with the Aqua
  appearance and a window background and captures the window frame, like
  plan-08's harness. This is a harness fix only; the app's window code did
  not change.
- **Desktop proof not done** (Validation Plan: "plus on the desktop … run
  skill … desktop-cpu.png"). This session has no tool that drives a
  desktop GUI (double-click, play, flip a switch), so it was not run. The
  harness drives the same `CpuWindowView` in an `NSWindow`, with the app's
  `SimulationController` and `SchematicSessions` and a document loaded
  from the saved `.schematical`. The double-click → `onOpenCpu` →
  `openWindow(id: "cpu")` path is compiled but not exercised. It is left
  for an interactive check.
- **The harness source file is `harness.swift`.** `swiftc` treats a file
  named `main.swift` as top-level code, which conflicts with `@main`
  (plan-08's harness used `a.swift`).

## Summary

E is UI work over a finished backend. The one subtle spot is copying the C
`CpuState` (tuples for fixed arrays) into Swift; the harness prints every
field. Users see the obsolete NMOS 6502 and Z80 parts only with Settings →
"Obsolete parts" on. A CPU starts "Unknown — waiting for reset" until its
reset pin is pulsed, as on real hardware.
