# plan-62-H: AVR simulation — the app: firmware load file and the Processor window

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-62-G

After G an ATmega on a schematic runs, with empty flash. H gives the user
the two things that make it useful: a firmware file per chip, chosen in the
chip's Processor window and loaded at every start, and the Processor window
itself for an AVR (registers, flags, last instruction), opened by a
double-click.

Outcome: in the app, double-clicking an `ATMEGA328P-PU` opens its Processor
window; "Choose…" picks a `.hex`; Run executes it (an LED on PB5 lights);
the window shows PC, SP, SREG, R0–R31, the SREG flags and the last
instruction; the file survives save and reopen, and undo.

References:

- plan-62-E § 3.6 (the hooks: flash through `circuit_memory_*`,
  `circuit_cpu_state`'s 35 registers), plan-62-G.
- App sources (read 2026-11-24): `schematical/schematical/Memory/MemoryWindow.swift`
  (`LoadFileBar` 398–427, its use 162–197, `refreshStartImage` 322–331),
  `Memory/MemoryFileAccess.swift`, `Cpu/CpuWindow.swift`,
  `Backend/PartCatalog.swift` (748, 898–905, 1026, 1472),
  `Simulation/Simulation.swift` (384–408, 497–502, 731–737),
  `Canvas/SchematicCanvasView.swift` (178–200),
  `Canvas/SchematicModel.swift` (1090, 5167–5174),
  `Document/SchematicFile.swift` (783–786, 1048–1052),
  `Editor/KindInfo.swift` (95–97), `Help/Acknowledgements.swift`.
- `docs/file-format.md` (116–123, 143, 739–741); `AGENTS.md` § Testing in
  the app.
- `scripts/harness/` (`build.sh`, `support/Harness.swift`,
  `display-drivers-app.swift` as the model of a harness that runs a
  simulation).

## Prerequisites

The gate for the whole feature is in plan-62-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-62-G is complete | `ls planning/completed/plan-62-G-* \| wc -l` → 1 | MET (2026-11-24: 1, archived after 4f7dee9c) |

If plan-62-G is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A simulated `mcu` part carries a load file exactly as an EEPROM does:
  set and cleared with undo ("Change Load File"), saved in the document's
  `memoryFile`, read back on open, loaded into flash at every start, with
  the same error reporting.
- Double-clicking a simulated `mcu` part opens the Processor window. The
  window has the load-file bar at its top and, while running, the status,
  the registers, an SREG flag row `I T H S V N Z C` and the last
  instruction.
- The app's acknowledgements name avr8js.

### Non-goals (explicit constraints)

- No flash viewer: the Memory window does not open for an `mcu`
  (`isMemory` is unchanged).
- No change for EEPROMs, the `cpu` kind's Processor window, or unsimulated
  microcontrollers (no load file, no window, Run stays off).
- No automation (MCP) tool for the load file or the processor state.
- No on-screen pass with the real mouse in this letter; the windows are
  rendered offscreen by a harness. The owner looks at the real window.
- The document format gains no key: `memoryFile` is reused.

## 2. Current State

All read 2026-11-24 (file:line as in References):

- **Load file.** `PlacedChip.memoryFile` is kept only for `PART_EEPROM`
  (`SchematicModel.swift:1090`), set by `setMemoryFile`
  (`SchematicModel.swift:5167`, guarded by `kind == PART_EEPROM`), decoded
  only for EEPROMs (`SchematicFile.swift:1050`; encoding has no kind
  gate), and loaded at start for `chip.part.kind == PART_EEPROM` with
  `chip.part.memorySize` (`Simulation.swift:391`), through
  `MemoryFileAccess.image(file, size:)` and `circuit_memory_load`
  (`Simulation.swift:497`). A failure stops the start with
  `"<reference>: <message>"`.
- **The bar.** `LoadFileBar` is a `private struct` in `MemoryWindow.swift`
  taking `file`, `isRunning`, `choose`, `clear`; its texts are "Load file
  on start:", "None — starts erased (FF)", "Choose…", "Clear" and help
  strings. `MemoryInspector` owns the window reference
  (`.background(WindowReader { window = $0 })`) for
  `MemoryFileAccess.choose(attachedTo:)`.
- **Processor window.** `CpuInspector(chipID:document:simulation:)`
  (internal) shows nothing but "Run the simulation to see the processor's
  state." while stopped; running, it shows `statusText`, a register grid,
  a flags row from register `P` or `AF` (`CpuSnapshot.flags`), and the
  last instruction (`%04X` address unless a `PBR` register exists). It
  opens from `SchematicCanvasView.swift:189` when `chip.part.isCpu`
  (`kind == PART_CPU`). `CpuSnapshot` reads up to `CPU_MAX_REGISTERS`
  registers.
- **Facts.** `PartCatalog`'s `.mcu(core:maxHz:flash:ram:eeprom:psram:)`
  holds the flash size in bytes; `isSimulated` comes from the backend part;
  `memorySize` is nil for an mcu.
- **Kind info.** `KindInfo(kind: "mcu", … simulated: false)` with the blurb
  "… Symbol, footprint and facts only: they are not simulated."
  `KindInfoTests.testUnsimulatedKinds` expects `["generic", "mcu", "pld"]`.

### Measured populations

| What | Count | Source |
|---|---|---|
| `PART_EEPROM` gates on the load file | 4 (`Simulation.swift:391`, `SchematicModel.swift:1090`, `:5172`, `SchematicFile.swift:1050`) | app survey, 2026-11-24 |
| `PART_EEPROM` gates that stay (Memory window only) | 3 (`MemoryWindow.swift:164`, `:169`, `:323`) | same |
| `isCpu` uses | 1 (`SchematicCanvasView.swift:189`) | same |

### Verified properties

- **A new `.c` file under `backend/src` needs no project edit**: the app
  target has `backend/src` as a synchronized folder with no exceptions
  (`project.pbxproj:30–35`, `grep -c Exception` → 0), so
  `backend/src/models/avr_*.c`, `mcu_avr*.c` and `LICENSE-avr8js` are
  already in the app after A–F; `Acknowledgement.licenceFile` names a
  bundled resource the same way `LICENSE-jsmooch` is.
- **A test can run a simulation and read lights**
  (`SlideSwitchTests.testSimulation`: `SimulationController(library:)`,
  `start(document:)`, `RunLoop.main.run(until:)`, `simulation.lights[id]`),
  and round-trip a document (`BoardLayersTests.roundTrip`).

## 3. Design Overview

Four small pieces: part capabilities, the load file's gates, the Processor
window, the surrounding texts. Correctness risk is low and concentrates in
the start path (a wrong size silently truncating firmware); the test loads
a real `.hex` and watches an LED.

### 3.1 Part capabilities (`PartCatalog.swift`)

```swift
/// A microcontroller the backend runs (plan-62): it takes firmware and has
/// a processor window.
var isSimulatedMcu: Bool { kind == PART_MCU && isSimulated }
/// A part that takes a "load file on start": an EEPROM, or a simulated
/// microcontroller (its firmware).
var hasLoadFile: Bool { kind == PART_EEPROM || isSimulatedMcu }
/// The memory a load file fills: an EEPROM's words, a microcontroller's
/// flash bytes.
var loadFileSize: MemorySize? { … EEPROM: memorySize; simulated mcu: MemorySize(words: Int(flash), bits: 8) }
var hasProcessorWindow: Bool { isCpu || isSimulatedMcu }
```

### 3.2 The load file

The four gates of § 2 become `hasLoadFile` (and `loadFileSize` at
`Simulation.swift:391`). `setMemoryFile`'s doc comment and
`Simulation.start`'s comment name microcontrollers. Nothing else changes:
the same picker, bookmark, path rules and messages.

### 3.3 The Processor window (`CpuWindow.swift`)

- `LoadFileBar` moves to `Memory/LoadFileBar.swift`, internal, unchanged.
- `CpuInspector`, for a chip with `hasLoadFile`: the bar at the top
  (`Choose…` → `MemoryFileAccess.choose(attachedTo: window)` →
  `document.setMemoryFile`; `Clear`), a `Divider`, then the existing
  content. It gains `@State private var window: NSWindow?` with
  `WindowReader`, and `@State private var loadError: String?`, refreshed
  on appear, when the simulation stops and when the file changes, by
  parsing with `MemoryFileAccess.image(chip.memoryFile, size:)`. While
  stopped with a `loadError` the content is the same "Can't Load File"
  view the Memory window shows.
- `CpuSnapshot.flags`: a third branch, register `SREG` → labels
  `["I", "T", "H", "S", "V", "N", "Z", "C"]`.
- `SchematicCanvasView.swift:189`: `chip.part.hasProcessorWindow`.

### 3.4 Texts

- `KindInfo` mcu blurb: "Single-chip computers (AVR, ESP32, RP2040). The
  ATmega48/88/168/328 family runs firmware; the others are symbol,
  footprint and facts only." `simulated: false` stays (a new mcu part made
  in the Part Editor starts facts-only), so `KindInfoTests` is unchanged.
- `Acknowledgements.swift`: after jsmooch,
  `Acknowledgement(name: "avr8js", use: "AVR microcontroller simulation, by Uri Shaked and contributors. Ported to C for this app.", licenceFile: "LICENSE-avr8js")`.
- `docs/file-format.md`: the `memoryFile` row and the Open note cover
  simulated microcontrollers ("the firmware, loaded into flash; Intel HEX
  or raw binary"), with an `ATMEGA328P-PU` example line.

## Compatibility / Format Impact

- Document format: `memoryFile` is now honored on a simulated `mcu` part.
  A document written by this version opens in an older one with the key
  ignored on that part (the older reader drops it for non-EEPROMs).
- No backend change.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; fill `Commit:` when the phase lands.

### Phase 1 — the load file on a microcontroller

- [x] `Backend/PartCatalog.swift`: § 3.1.
- [x] `Canvas/SchematicModel.swift:1090`, `:5167–5174`;
      `Document/SchematicFile.swift:1048–1052`;
      `Simulation/Simulation.swift:384–408`: § 3.2.
- [x] `schematicalTests/AvrAppTests.swift` (new), with two firmware files
      written to a temporary folder by the test from byte arrays built
      with `oracle.js asm` and pasted in as Intel HEX strings (`led_on`:
      set DDRB5 and PORTB5, loop; `led_off`: loop):
      - `testFirmwareRuns`: `5V`, `GND`, `ATMEGA328P-PU`, `GENERIC-LED`
        from PB5 to GND; `setMemoryFile(led_on)`; start; settle; the LED is
        lit. Stop; `setMemoryFile(led_off)`; start; settle; not lit.
      - `testNoFirmwareStillRuns`: no file; start succeeds; the LED is not
        lit.
      - `testUnreadableFirmwareStopsTheStart`: a file path that does not
        exist; `isRunning` false; `errorMessage` starts with the chip's
        reference.
      - `testLoadFileUndo`: "Change Load File" is one undo step on the
        ATmega; on `ATTINY85-20PU` `setMemoryFile` leaves `memoryFile` nil.
      - `testLoadFileRoundTrip`: `BoardLayersTests.roundTrip` keeps the
        ATmega's `memoryFile` path.

Acceptance: the five tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p62-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/AvrAppTests -only-testing:schematicalTests/SchematicUndoTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 10 min, an incremental app build).
  Observed: 2026-11-24: `AvrAppTests` 10 tests, `SchematicUndoTests` 50, `KindInfoTests` 5, `McuTests` 4: 69 tests, 0 failures, `** TEST SUCCEEDED **`.
Commit: ca385021

### Phase 2 — the Processor window

- [x] `Memory/LoadFileBar.swift` (moved from `MemoryWindow.swift`).
- [x] `Cpu/CpuWindow.swift`: § 3.3; `Canvas/SchematicCanvasView.swift:189`.
- [x] `AvrAppTests.swift`: `testProcessorState`: with `led_on` running,
      `simulation.cpuState(of:)` is `.running`, has 35 registers named
      `PC`, `SP`, `SREG`, `R0`…`R31`, `flags` has the 8 labels of § 3.3
      and its `I` cell is clear, and `instructionText` ends with the
      loop's `RJMP .-2`; `testHasProcessorWindow`: true for
      `ATMEGA328P-PU` and `W65C02S6TPG-14`, false for `ATTINY85-20PU` and
      `SN74HC00N`.
- [x] `scripts/harness/avr-app.swift`: builds the Phase 1 circuit, renders
      `CpuInspector` with `Harness.renderHosted` stopped with no file
      (`/tmp/p62-h/avr-stopped.png`), stopped with `led_on`
      (`avr-file.png`) and running (`avr-running.png`), and prints `ok`
      lines for: the bar's file name in the stopped view's model, the LED
      lit, 35 registers.
- [x] `Editor/KindInfo.swift`, `Help/Acknowledgements.swift`,
      `docs/file-format.md`: § 3.4.

Acceptance: the tests pass; the harness prints only `ok` lines and the
three images show the bar, the file name, and the registers with the flag
row.
  Check: the Phase 1 `xcodebuild` line with `-only-testing:schematicalTests/AvrAppTests -only-testing:schematicalTests/KindInfoTests` → `** TEST SUCCEEDED **` (est. 10 min); `rm -rf /tmp/p62-h && APP=/tmp/p62-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/avr-app.swift /tmp/p62-h && /tmp/p62-h/H.app/Contents/MacOS/harness /tmp/p62-h` → every line `ok`, exit 0, then view the three PNGs (est. 5 min).
  Observed: 2026-11-24: the same run (69 tests, 0 failures); the harness printed 9 `ok` lines and no FAIL, exit 0; the three PNGs viewed (see Corrections).
Commit: ca385021

## Validation Plan

- Tests: `AvrAppTests` (7 tests), `SchematicUndoTests`, `KindInfoTests`.
- Coverage check: `testFirmwareRuns` fails if the file is not loaded (the
  LED stays dark) and if it is loaded into the wrong part.
- Runtime proof: the harness run and its three images.
- Doc sync: `docs/file-format.md`.
- Final gate: plan-62-I.

## Decisions

plan-62-A § Decisions: the firmware is chosen in the Processor window,
which a double-click opens; no flash viewer.

## Corrections

- `AvrAppTests` has 10 tests, not 7: `testPartCapabilities` (it holds the
  planned `testHasProcessorWindow` and checks `hasLoadFile`, `loadFileSize`
  and `isMemory` too), `testFirmwareRuns`, `testNoFirmwareStillRuns`,
  `testUnreadableFirmwareStopsTheStart`, `testFirmwareTooBigStopsTheStart`
  (a HEX byte at 0x8000, past 32 KB of flash), `testLoadFileUndo`,
  `testLoadFileRoundTrip`, `testProcessorState`, `testLoadProblem`,
  `testSregFlags`.
- § 3.3: the window's firmware check is a static function,
  `CpuInspector.loadProblem(of:)`, so a test can call it; it is nil when no
  file is chosen (an erased chip is no problem).
- Both phases were built and tested in one run and landed in one commit: the
  app builds once, and Phase 1's tests live in the file Phase 2 adds to.
- The harness's three images are kept as
  `planning/plan-62-data/h-avr-stopped.png`, `h-avr-file.png` and
  `h-avr-running.png`. Viewed: the bar with "led_on.hex" and Choose… / Clear
  (disabled while running), `Running · 1092568 cycles`, PC 0004, SP 08FF,
  SREG 00, R0–R31, the flag row I T H S V N Z C, `0004  FF CF  RJMP .-2`.

## Summary

The app change is four gates widened and one window extended; the test
that matters loads a real HEX file and watches an LED through the whole
start path.
