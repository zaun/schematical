# plan-51-H: IC sockets — a placed chip's socket mode (model, file, undo, BOM, facts, automation)

Last updated: 2026-11-21
Effort: large (3h–1d)
Depends on: plan-51-G

H gives every placed chip a **socket mode** and everything that follows from it,
without any new view: the data on the chip, the footprint it puts on the PCB, the
file, undo and redo, copy and paste, the Bill of Materials, the sidebar facts, the
KiCad and EasyEDA exports and the automation commands. plan-51-I adds the controls.

Outcome of H: `PlacedChip.socket` (nil = IC) names a socket part and a mode
(**IC + Socket** or **Socket only**); `chip.footprint` is the socket's footprint
in both socket modes, so `boardPads`, the PCB, the design-rule check, the exports and
the ratsnest follow with pad numbers unchanged; `SchematicDocument.setPartSocket`
switches a chip's mode with the pad-1 anchor kept; the file saves and loads it
(`"socket"` on a part; version stays 4); the Bill of Materials lists the chip, the
chip and its socket (`XU1`), or the socket alone; `set_part` takes `socket` and
`socket_mode`; Run builds the same circuit in all three modes.

References:

- `schematical/schematical/Canvas/SchematicModel.swift` (`PlacedChip` `:724`: `resized` `:842`, `footprintChoice` `:854`; `setPartFootprint` `:2033–:2054`;
  the paste copy `:2289–:2294`), `Canvas/BoardModel.swift:163, :171` (`PlacedChip.footprint`,
  `boardPads`), `Canvas/UndoHistory.swift:93–95, :210–220`, `Document/SchematicFile.swift`
  (`Part` `:70–:112`, write `:740`, read `:938–:978`),
  `BillOfMaterials/BillOfMaterialsWindow.swift:40–:95`, `Backend/PartFacts.swift:8–:30`,
  `Backend/PartCatalog.swift` (`load()` `:1859`), `Automation/AutomationSchematic.swift`
  (`setPart` `:513–:560`), `Automation/AutomationRead.swift:374`.
- `docs/file-format.md` (§ Parts, § Validation on open, § Changing the format),
  `docs/agent-control.md` (`set_part`).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-G complete | `ls planning/completed/plan-51-G-*.md` → exists | MET (2026-11-24: `planning/completed/plan-51-G-socket-part-editor.md`, commit 94613cea) |
| The catalog has sockets | `grep -c 'GENERIC-SOCKET' schematical/schematical/Parts/socket.json` → 15 or more | MET (2026-11-24: 15 generic sockets) |
| The app builds and `SocketPartsTests` passes | the check of plan-51-F Phase 3 | MET (2026-11-24: `SocketPartsTests` 7 of 7, the app builds) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- `xcodebuild … test -only-testing:schematicalTests/SocketModeTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/AutomationTests` → `** TEST SUCCEEDED **`.
- A schematic with no socket mode saves byte-identically to before (the `"socket"` key is
  written only for a chip in a socket mode): `SchematicFileV5Tests`' fixtures round-trip unchanged.

### Non-goals (explicit constraints)

- No view or control (I). No change to the simulation: the netlist is built from `chips`
  as before; **Run ignores the mode** (owner decision).
- File `version` stays 4 (an optional field, `docs/file-format.md` § Changing the format).
- No change to the footprint of a chip in IC mode; no socket for a part with
  `footprintChoices` (generic LED, headers, pull resistors, capacitors: none has a `fits` match).

## 2. Current State

- `PlacedChip` stores `footprintID` for a part with footprint choices; `footprint`
  (`BoardModel.swift:162–166`) is `board?.style == .connector ? part.connectorFootprint ?? base : base`
  with `base = footprintChoice?.footprint ?? part.footprint`. Every consumer of the board
  geometry reads `chip.footprint` or `chip.boardPads`: the KiCad writer (`KiCadBoardWriter.swift:323`),
  the EasyEDA writer (`EasyEDAProBoardWriter.swift:58`), the DRC, the PCB view.
  Direct reads of `part.footprint` elsewhere are not geometry: BOM filter
  (`BillOfMaterialsWindow.swift:47`), the DRC's "unplaced" loop (`DesignRuleCheck.swift:300`),
  `PartFacts` (`:18`, `:90`), automation (`AutomationRead.swift:372`).
- `setPartFootprint` (`SchematicModel.swift:2033–2054`) swaps the footprint, zeroes
  `referenceOffset`, and moves the placement so the first pin's pad keeps its board point.
- The file's `Part` carries optional per-part fields (`gender`, `ohms`, `farads`, `footprint`)
  validated on read with `badPartOption` (`SchematicFile.swift:930–950`); an optional
  field keeps `version` 4 (`docs/file-format.md` § Changing the format).
- Undo: `UndoHistory.sameFootprint` compares `board`, `footprintID`, `part.name`
  (`:93–95`) and a footprint undo restores `board` and `footprintID` (`:216–219`);
  a chip is `Hashable`, so `.part` changes include any new stored property.
- The Bill of Materials has one line per `(part number, package, value, color)`,
  references sorted naturally, lines ordered by first reference (`:43–:56`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts a socket can hold | 2405 | `python3 -I planning/plan-51-data/census/chips.py` → `parts using them: 2405` |
| … of them with an absent pin (a hidden pad) | 0 | same → `with an absent pin: 0` (so a socket footprint needs no hidden-pad rule) |
| `PlacedChip(` constructor calls (6 in the app, 2 in the tests; the new parameter has a default, so none must change) | 8 | `grep -rn 'PlacedChip(' schematical/schematical schematical/schematicalTests --include='*.swift' \| wc -l` → 8 |

### Verified properties

- A chip's pin *n* and the socket's pad *n* share a designator, for every socket and every
  footprint it `fits`: the loader's `check_pads` (plan-51-A) rejects a `fits` footprint
  that lacks a pad of any pin's designator, and F's `library_check` loads all of them.
- Parts with `footprintChoices` hold no socket: their footprints (`LED-…`, `PINHDR-…`,
  `RES-…`, capacitor ones) are in no `fits` list of `fits.tsv`.

## 3. Design

### 3.1 The data

```swift
struct ChipSocket: Hashable, Sendable {
    enum Mode: String, Sendable { case withSocket, socketOnly }   // the file's values
    var part: CatalogPart        // kind PART_SOCKET
    var mode: Mode
}
// PlacedChip: var socket: ChipSocket?      nil = IC
```

The `PlacedChip` init takes `socket: ChipSocket? = nil` and keeps it only when
`part.footprintChoices.isEmpty`, `socket.part.kind == PART_SOCKET` and
`socket.part.fits` contains the chip's base footprint (`part.footprint`); otherwise nil.
`footprint` becomes `socket?.part.footprint ?? base` (a socket ignores the
connector style: only the character LCDs have one and none is in a `fits` list).

### 3.2 The catalog index

`PartCatalogStore.sockets(fitting footprint: Footprint) -> [CatalogPart]`: the
`PART_SOCKET` parts whose `fits` hold the footprint, generics first, then by
manufacturer, then by part number in natural order; built once in `load()`.
`genericSocket(fitting:)`: its first `Generic` entry. `PlacedChip.isSocketable(in:)`:
the chip has a base footprint and a non-empty list.

### 3.3 The operations

- `SchematicDocument.setPartSocket(_ id: UUID, mode: ChipSocket.Mode?, socket: CatalogPart?)`:
  `mode` nil = IC. Records "Change Socket"; refuses (no change) a socket that does not fit.
  The pad-1 anchoring and `referenceOffset` reset of `setPartFootprint` move into one
  private helper both call; going from IC to a socket mode or back anchors, going between
  the two socket modes only changes the mode.
- File: `SchematicFile.Part.socket: SocketEntry?` with `part` (name) and `mode`
  (`"withSocket"` / `"socketOnly"`); written only for a chip in a socket mode; read
  through `catalog.part(named:)` with `badPartOption` errors `"X" is not an IC socket`
  and `"X" does not fit <footprint id>`; an unknown mode is `badPartOption` too.
- Undo: `sameFootprint` also compares `socket`; the restore copies `socket` with `board` and `footprintID`.
- Paste: the chip copy passes `socket`.
- Bill of Materials: IC → the chip's line; `withSocket` → the chip's line and a socket
  line; `socketOnly` → the socket line only. A socket line: part number the socket's name,
  package its `package` (`DIP socket`), description its description; its reference is `X`
  + the chip's reference (`XU1`); same-socket chips share a line (`XU1, XU2`). A standalone
  placed socket is an ordinary chip line (`XS1`).
- Facts (`PartFacts.rows`): Footprint shows `chip.footprint`'s; a **Socket** row
  `<name> (IC + Socket)` / `<name> (Socket only)` for a chip in a socket mode.
- Automation: `set_part` takes `socket` (part name) and `socket_mode` (`ic`, `ic+socket`,
  `socket-only`): `ic` clears; a mode without `socket` picks the generic; the result
  line says `socket`. `read` prints `socket: <name> (<mode>)`. `docs/agent-control.md` updated.

### Where the risk is

The footprint swap on a placed, routed chip (pad positions move; traces keep their
nets): the existing `setPartFootprint` has the same exposure and the same anchoring.
The file read is the other: a bad file must fail with a message, not a crash.
**Byte-identity is a check here**: a document with no chip in a socket mode saves exactly as before.

## 4. Phases

### Phase 1 — the model

- [x] `SchematicModel.swift`: `ChipSocket`, `PlacedChip.socket`, the init parameter and its validation, `resized` passes it; `BoardModel.swift`: `footprint`; `PartCatalog.swift`: `sockets(fitting:)`, `genericSocket(fitting:)`, the index in `load()`. — `ChipSocket` (+ `ChipSocket.fits`), `PlacedChip.socket` kept only when the socket holds the chip's base footprint, `resized` passes it; `BoardModel.swift` `footprint` is the socket's; `PartCatalogStore.sockets(fitting:)` and `genericSocket(fitting:)` (an index dropped whenever `partsByName` changes).
- [x] `setPartSocket` and the shared anchoring helper (behavior of `setPartFootprint` unchanged: its tests are the check). — `SchematicDocument.setPartSocket` and the private `changingFootprint(of:_:)` that `setPartFootprint` now calls too (`PullResistorTests`, `HeaderBoardTests` pass).
- [x] `schematicalTests/SocketModeTests.swift`: for `SN74HC00N` (14 pins): `footprint` is `PDIP-14` in IC mode and the generic socket's in both socket modes; `boardPads` has 14 pads with designators `1…14` in each; pad 1's board point is equal before and after each switch; a socket that does not fit (a 28-pin one) is refused; the generic list for `PDIP-14` and `CDIP-14` holds `GENERIC-SOCKET-DIP-14-300` first. — `SocketModeTests` (15 cases in all): the three modes keep pad 1's board point to 0.001 mm, `PDIP-14` → `SKT-DIP14-300-TH-MM-110` and back, 14 pads `1…14` in each; a 28-pin socket, a PLCC one and a non-socket are refused with no undo step; the generic is first in the list for `PDIP-14` and `CDIP-14`.

Acceptance: `SocketModeTests` passes and the footprint-choice tests (`PullResistorTests`, `HeaderBoardTests`) still do.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p51-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SocketModeTests -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/HeaderBoardTests` → `** TEST SUCCEEDED **` (est. 12 min). Measured 2026-11-24 with every phase's classes in one run (the three phases' checks below): `Executed 179 tests, with 0 failures`, `** TEST SUCCEEDED **`; `SocketModeTests` 15 of 15, `PullResistorTests` 10, `HeaderBoardTests` 7. VERIFIED.
Commit: (next commit)

### Phase 2 — file, undo, paste

- [x] `SchematicFile.swift`: `Part.socket`, write, read and errors per § 3.3; `docs/file-format.md`: a `socket` row in § Parts' field table (after `gender`), the three errors in § Validation on open. — `SchematicFile.Part.socket: SocketEntry?`, written only for a chip in a socket mode; the read checks the mode, that the part is a socket and that it fits (`U1: "X" is not an IC socket`, `… does not fit PDIP-14`, `… is not a socket mode`); `docs/file-format.md` (field row and the three errors).
- [x] `UndoHistory.swift`: compare and restore `socket`; `SchematicModel.swift` paste copy. — `UndoDiff.sameFootprint` compares `socket`, the footprint restore copies it; the paste copy passes `socket: chip.socket`.
- [x] `SocketModeTests`: save then open keeps mode and socket; a file naming a non-socket part, a socket that does not fit and an unknown mode each fail with their message; undo after `setPartSocket` restores IC and the old footprint and board; redo re-applies; copy and paste keeps the socket; a document with no socket saves the same bytes as before (compare with the fixture). — save and open keep mode, socket and board placement; the three bad entries fail with their messages; undo and redo of `Change Socket`, and of a mode-only change; copy and paste keep the socket; a chip with no socket writes no `socket` key, and `SchematicFileV5Tests.testPlainSchematicIsVersion4AsBefore` (the golden bytes) passes.

Acceptance: the file and undo cases pass with the existing file and PCB undo classes.
  Check: the Phase 1 command with `-only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests` replacing the two footprint classes → `** TEST SUCCEEDED **` (est. 13 min). Measured 2026-11-24 (same run): `SchematicFileV5Tests` 10 of 10, `PCBUndoTests` 34 of 34. VERIFIED.
Commit: (next commit)

### Phase 3 — Bill of Materials, facts, automation, exports

- [x] `BillOfMaterialsWindow.swift`: the three modes per § 3.3; `PartFacts.swift`: footprint and Socket rows; `AutomationSchematic.swift`, `AutomationRead.swift`, `docs/agent-control.md`. — the BOM (chip, chip + `X<ref>` socket line, or the socket line only; chips with one socket part share a line; a chip in a socket is not a machine-placed line), `PartFacts` (Footprint from the socket, a Socket row), `set_part` `socket_mode` and `socket`, `get_design` prints `socket=<name> (<mode>)`, `docs/agent-control.md`; `PickAndPlaceExport` and the BOM use `PlacedChip.fabricationReference` (`XU1`) so the placement file and the BOM agree.
- [x] `SocketModeTests`: the three BOM line sets for one chip (`[chip]`, `[chip, XU1]`, `[XU1]`) and for two chips sharing a socket (`XU1, XU2`); the facts rows; `set_part` with `socket_mode` `ic+socket` and `socket-only` and `ic`; a KiCad board export of a chip in a socket mode names the socket footprint (`KiCadExportTests` helper); a `Simulation` run builds with the same net count in all three modes. — the three BOM line sets and two chips sharing a socket (`XU1, XU2`), a maker socket line with its maker, the facts rows, `set_part` through `AutomationTests.testSetPartSocketModes` (a mode alone picks the generic, a mode-only change keeps the socket, a named socket keeps the mode, the three refusals, `ic`), the KiCad board names `SKT-DIP14-300-TH-MM-110` and the placement file lists `XU1` for a surface-mount socket, and `Simulation` starts in all three modes.

Acceptance: the cases pass with the BOM, automation and export classes.
  Check: the Phase 1 command with `-only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/KiCadExportTests` replacing the two footprint classes → `** TEST SUCCEEDED **` (est. 14 min). Measured 2026-11-24 (same run): `BillOfMaterialsTests` 7, `AutomationTests` 31, `KiCadExportTests` 20, `EasyEDAProExportTests` and `PickAndPlaceExportTests` pass. VERIFIED.
Commit: (next commit)

## Validation Plan

- Tests: `SocketModeTests` with the negative file cases; the existing classes named in each phase.
- Coverage check: `SocketModeTests` uses the real catalog (the generic sockets F ships) and the real `SchematicFile` path, so a dropped field fails the round trip.
- Runtime proof: the plan-51-I harness drives the same operations through the real sidebar.
- Doc sync: `docs/file-format.md`, `docs/agent-control.md`.
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions. Author decisions:

- **The socket line's reference is `X` + the chip's reference** (`XU1`), the KiCad / IEC convention; the schematic and PCB silkscreen keep showing `U1`.
- **The file stores `part` and `mode`**, strings `"withSocket"` and `"socketOnly"`.
- **A part with footprint choices cannot take a socket.**
- **Switching between the two socket modes changes no geometry.**

## Corrections

- **2026-11-24, one run for the three phases.** Each phase's check names the Phase 1 classes plus its own; the three were built and run once together (12 classes, 179 tests, 0 failures) because the work was written in one pass and the app build is the slow part. Every class a phase names is in that run.
- **2026-11-24, custom sockets are not offered.** `PartCatalogStore.sockets(fitting:)` lists bundled sockets only (`uuid == nil`): a chip's file names its socket by part number, and a file carries no custom part but a placed chip's own (`customParts`), so a custom socket would make the file fail to open on another machine. A custom socket can still be placed on its own (it is an ordinary part).
- **2026-11-24, the placement file and the BOM share the fab reference.** The plan only says the BOM's socket line is `X` + the chip's reference. The pick-and-place file (`PickAndPlaceExport.rows`) also lists a chip in a socket mode under that reference (`PlacedChip.fabricationReference`), since the part a fab places is the socket and its pads are the socket's (`boardPads`); the chip's own BOM line is not marked surface mount while it sits in a socket.
- **2026-11-24, `get_design`, not `read`, shows the socket**: the part's options line gets `socket=<name> (ic+socket | socket-only)`; `docs/agent-control.md` says so.
- **2026-11-24, the same-footprint shortcut.** `setPartSocket` between two sockets that share a footprint id (the mode alone, or the same part) moves nothing; every other switch (IC ↔ a socket mode, or a socket with another footprint) goes through `changingFootprint(of:_:)`, which keeps pad 1 at its board point.

## Summary

H's risk is the footprint swap on a routed chip and the file read; both reuse an existing
mechanism and are covered by round-trip and undo tests. Untouched: the engine, the
netlist, the backend and every part file.
