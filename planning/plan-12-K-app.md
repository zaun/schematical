# plan-12-K: App — the serial kind and the terminal window; plan-12 landing tier

Last updated: 2026-11-02
Effort: large (3h–1d)
Depends on: plan-12-J

K brings the `serial` kind into the app — catalog details, the facts panel,
the drawer group **Interface**, a chip-box symbol — and adds the **terminal
window**: double-click a serial part (one with at least one UART channel) to
open a window that shows what each channel transmits and sends what you type
into the channel's RX pin. K ends plan-12 with its landing tier.

Outcome of K (the plan's outcome): in the app harness, a W65C02S running the
§5 program from an AT28C256 prints `HELLO` through a TL16C550CFNR; the
terminal window for the TL16C550CFNR shows `HELLO`; typing `x` makes the
program echo
`x`, and the window shows it.

References: plan-12-A §3.7 (terminal decisions), plan-12-C §3.5 (circuit
API); `planning/completed/plan-09-E-cpu-app.md` (catalog details, facts,
processor window, harness proof — the pattern), `Memory/MemoryWindow.swift`
(window per part, refresh per step), `planning/completed/plan-06-A-board-model.md`
§ Harness.

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-J landed | `grep -c '\|serial\|' backend/tests/golden/parts.txt` → 219 (or the sum of F–J's recorded counts) | NOT MET (2026-11-02) |

## 1. Goal

- `xcodebuild … build` → `** BUILD SUCCEEDED **`.
- Harness `catalog` → `Interface: <n> parts; functions Bridge UART (<b>),
  I2C controller (1), UART (<u>)` with n = the `|serial|` line count, and the
  obsolete-hidden count.
- Harness `facts TL16C550CPTR` → Kind Serial controller, Function UART,
  Package TQFP, Footprint TQFP-48-7X7, Pins 48, Model ns16550, Channels 1,
  Speed grade (its grade), Supply range 3–5.25 V.
- Harness `terminal` (§5) prints `HELLO\r\n` then `x`, and writes
  `/tmp/p12-proof/terminal.png`, which shows `HELLO` and `x`.

### Non-goals

- No backend change (C–J own it). No PCB change.
- The terminal does not decode a channel's line by itself: it shows the
  model's sent log (plan-12-C §3.4).

## 2. Current State

- `Backend/PartCatalog.swift`: per-kind `details` (`.supply`, `.cpu`, …)
  and drawer grouping by `drawer` + `function`; `Backend/PartFacts.swift`:
  `kindName` per kind and the facts rows.
- `Canvas/SchematicCanvasView.swift`: double-click opens the memory window
  (`onOpenMemory`) or the processor window (`onOpenCpu`); `ContentView.swift`
  tracks open windows per session; `schematicalApp.swift` declares one
  `WindowGroup` per window kind (`memory`, `cpu`).
- `Simulation/SimulationEngine.swift`, `Simulation.swift`: wrappers over the
  circuit API used by the windows; windows read after every simulation step.

## 3. Design

### 3.1 Catalog and facts

- `PartCatalog`: `PART_SERIAL` → details `.serial(model: String, channels:
  Int, grade: String)`; symbol `.block`; the drawer groups by `drawer`
  (`Interface`) and `function` as for every kind.
- `PartFacts`: Kind `Serial controller`; rows Model, Channels, Speed grade,
  then the common rows (supply range, input max, package, footprint, pins).

### 3.2 Terminal window

- `Terminal/TerminalWindow.swift`: `TerminalWindowTarget { sessionID, chipID }`
  (`Codable, Hashable`), `TerminalWindowView`. `schematicalApp.swift`:
  `WindowGroup("Terminal", id: "terminal", for: TerminalWindowTarget.self)`,
  default size 640 × 420, restoration disabled (as memory/cpu windows).
- Double-click on a chip whose details are `.serial` with `channels > 0`
  opens it (`onOpenTerminal`, beside `onOpenMemory`/`onOpenCpu`); SC18IS604
  (0 channels) opens nothing.
- Layout: title "<reference> — <part>"; a segmented channel picker (`A`, `B`,
  … when `channels > 1`); a monospaced transcript: printable ASCII as text,
  CR LF / LF as a new line, CR alone as a new line, every other byte as
  `⟨hh⟩`; frames with more than 8 data bits as `⟨hhh⟩`; an input field: Return
  sends the typed characters followed by CR (0x0D); a Clear button (clears
  the view, not the log); an error line showing `circuit_error` when a send
  is refused.
- Data: `SimulationEngine` gains `serialChannels(part)`, `serialSentCount(part,
  channel)`, `serialSent(part, channel, from:, count:)`, `serialSend(part,
  channel, bytes) -> String?` (nil on success, the error text otherwise).
  After every simulation step the window reads the frames since its last
  count. While the simulation is stopped the transcript of the last run stays
  and the input is disabled; a new run starts an empty transcript (the log
  restarts at `circuit_start`).

Rejected: decoding the TX net in the app (the owner wants what the channel
sends; the model knows its format and the log is exact); a terminal per
channel window (one window per part with a channel picker, as the memory
window is one per part).

## 4. Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Catalog and facts

- [ ] `PartCatalog.swift`, `PartFacts.swift`: §3.1. `xcodebuild … build` →
  `** BUILD SUCCEEDED **`.
- [ ] Harness (`/tmp/p12/harness/build.sh`) commands `catalog` and `facts`
  (§1 lines).

Acceptance: the build and the two harness commands (about 3 min).
Commit: —

### Phase 2 — Terminal window

- [ ] `SimulationEngine.swift`, `Simulation.swift`, `Terminal/TerminalWindow.swift`,
  `schematicalApp.swift`, `SchematicCanvasView.swift`, `ContentView.swift`:
  §3.2. Build succeeds.
- [ ] Harness `terminal` (§5): prints and PNG as §1; the PNG is looked at.

Acceptance: §5's printed values; the PNG shows `HELLO` and `x` (about 3 min).
Commit: —

## 5. Runtime proof (harness `terminal`)

Fixture `/tmp/p12-proof/uart6502.schematical`, built by the harness from app
parts:

- `5V`, `GND`; `NJU6366DF1-TE1` (2 MHz, plan-09-E's CPU clock) → the CPU's
  PHI2 and the gates; `ECS-100A-018` (1.8432 MHz, 5 V) → the TL16C550C's XIN;
  a TE-GDS0204 position between the CPU reset pin and GND (plan-09-E's reset
  switch); the CPU's other inputs (RDY, IRQB, NMIB, BE, SOB) tied as on
  plan-09-E §5's board.
- `W65C02S6TPG-14`; `AT28C256` at $8000–$FFFF holding the program below;
  `TL16C550CFNR` (PLCC-44) selected at $6000–$7FFF by its three chip
  selects: CS0 ←
  A14, CS1 ← A13, CS2 (active low) ← A15; A2..A0 ← A2..A0; D7..D0 shared;
  MR tied to GND; the active-high read and write strobes tied to GND; SIN,
  SOUT, modem inputs and INTR unconnected.
- One `SN74HC00N`: RDBAR = NAND(PHI2, R/W) → the UART's active-low read
  strobe and the EEPROM's OE#; nRW = NAND(R/W, R/W); WRBAR = NAND(PHI2, nRW)
  → the UART's active-low write strobe; CE# = NAND(A15, A15) → the EEPROM's
  CE#; the EEPROM's WE# to 5V.

Program (W65C02S, at $8000; reset vector $FFFC = `00 80`; every other byte
`FF`):

```
8000 A9 83     LDA #$83      ; LCR: DLAB, 8N1
8002 8D 03 60  STA $6003
8005 A9 0C     LDA #$0C      ; divisor 12: 9600 baud at 1.8432 MHz
8007 8D 00 60  STA $6000
800A A9 00     LDA #$00
800C 8D 01 60  STA $6001
800F A9 03     LDA #$03      ; LCR: 8N1
8011 8D 03 60  STA $6003
8014 A2 00     LDX #$00
8016 AD 05 60  LDA $6005     ; L1: wait for THRE
8019 29 20     AND #$20
801B F0 F9     BEQ $8016
801D BD 3B 80  LDA $803B,X
8020 F0 0A     BEQ $802C     ; end of message
8022 8D 00 60  STA $6000
8025 E8        INX
8026 4C 16 80  JMP $8016
8029 EA EA EA  NOP NOP NOP
802C AD 05 60  LDA $6005     ; ECHO: wait for data ready
802F 29 01     AND #$01
8031 F0 F9     BEQ $802C
8033 AD 00 60  LDA $6000
8036 8D 00 60  STA $6000
8039 80 F1     BRA $802C
803B 48 45 4C 4C 4F 0D 0A 00 ; "HELLO\r\n"
```

Steps: open the fixture; Run; flip the reset switch on and off; run 20 ms;
print `circuit_serial_sent` of channel 0 as text → `HELLO\r\n`; open the
terminal window for the TL16C550CFNR and type `x` (no Return); run 5 ms (one 9600-baud frame in, one out: about
2.1 ms); print
the frames since the first 7 → `x`; render the window to
`/tmp/p12-proof/terminal.png`.

## Validation Plan — plan-12 landing tier

Run once, at the end of K, with the targeted gates for everything plan-12
touched (supply model and voltage check, the `serial` kind and its ten
models, the part files, the footprints, the app):

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p12 && cmake --build /tmp/logic-p12 -j8 && ctest --test-dir /tmp/logic-p12` | `100% tests passed`, T0 + 1 programs (about 20 s) |
| `serial_test` cases | `/tmp/logic-p12/serial_test \| grep -c '^ok'` | 12 (C) + 24 (D) + 28 (E) + 1 (`shipped`) + 8 (H) + 9 (I) + 6 (J) = 88 |
| Golden sizes | `wc -l < backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` | P0 + 226, P0 + 226, F0 + 24 (minus stock-check drops recorded in F–J) |
| Goldens vs the start of plan-12 | `git diff <plan-12-A's first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part counts | `grep -c '\|serial\|' backend/tests/golden/parts.txt`; `grep -c '^PULL' backend/tests/golden/parts.txt` | 219 (minus drops), 7 |
| Datasheets | `ls datasheets/controller-*/*.pdf \| wc -l` | 80 |
| App | `xcodebuild … build`; harness `terminal` | `** BUILD SUCCEEDED **`; §5's output |

Each check is the cheapest that would fail if its area were wrong: `ctest`
runs every model test and the golden comparisons in seconds; the harness
drives the only app paths plan-12 adds.

## Decisions

- Return sends CR (a terminal's Enter); the transcript treats CR, LF and CR LF
  as one new line each, so both conventions read well.
- The proof uses plan-09-E's CPU, clock and reset switch so the UART path is
  the only new thing in it.

## Corrections

(none yet)

## Summary

K shows the serial parts in the app and gives every UART a terminal; the
landing tier closes plan-12.
