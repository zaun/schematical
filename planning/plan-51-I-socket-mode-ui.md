# plan-51-I: IC sockets — the mode control and the compatible-sockets dialog

Last updated: 2026-11-21
Effort: large (3h–1d)
Depends on: plan-51-H

I adds the controls. A placed DIP or PLCC chip that a socket fits gets, in the
Schematic sidebar's **Options** section, a three-way **IC / IC + Socket / Socket
Only** control; in a socket mode a **Socket** row shows the part and a **Choose
Socket…** button opens a small dialog listing the sockets that fit the chip.

Outcome of I: selecting a placed `SN74HC00N` shows the control; choosing **IC +
Socket** selects `GENERIC-SOCKET-DIP-14-300` and shows its name; **Choose Socket…**
opens a sheet listing every socket that fits `PDIP-14`, with a search field and
Mounting and Manufacturer filters; choosing a row and **Use Socket** switches the chip
to it (one undo step); **Cancel** changes nothing; a chip no socket fits shows no
control; the harness drives the real sidebar and sheet and prints `ok` for every check.

References:

- `schematical/schematical/Drawer/PartOptionsSection.swift` (the Options section:
  `applies(to:)` `:40`, `body` `:49`, `FootprintChoicePicker` `:304`),
  `Drawer/SchematicSidebarView.swift:72–74` (where the section is shown),
  `Drawer/ChipDrawerView.swift` (list rows, search field, filters: the look to match).
- plan-51-H § 3 (`ChipSocket`, `setPartSocket`, `sockets(fitting:)`, `genericSocket(fitting:)`).
- `scripts/harness/generic-led-ui.swift`, `scripts/harness/pull-resistor-ui.swift`,
  `scripts/harness/build.sh` (the harness precedent: real views rendered offscreen, document driven directly).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-H complete | `ls planning/completed/plan-51-H-*.md` → exists | NOT MET (2026-11-21: not started) |
| `SocketModeTests` passes | the check of plan-51-H Phase 1 | NOT MET (H's output) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- The harness `scripts/harness/socket-mode-ui.swift` prints `ok` for every check and exits 0.
- `xcodebuild … test -only-testing:schematicalTests/SocketPickerTests -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **`.

### Non-goals (explicit constraints)

- No change to the schematic canvas drawing of a chip (the symbol is the same in all three modes).
- No change to the Parts drawer, the PCB sidebar or the simulation.
- The dialog does not edit parts and has no datasheet button; the sidebar's Details and
  datasheet button show the chip's own as today.

## 2. Current State

- `PartOptionsSection.applies(to:)` is a static function of a part (`:40–42`): colors or footprint
  choices. The section gets `catalog`, `document`, `chip`; its `body` re-reads the chip from
  the document so the pickers follow every change (`:49–:51`).
- Options rows are plain SwiftUI: a segmented `Picker` for Rows and Gender, a menu `Picker`
  in `FootprintChoicePicker` (`:304`).
- The drawer's `ChipDrawerView` has the app's search field and filter-menu idiom
  (`part.package`, `manufacturer`).
- Sheets in the app are presented with `.sheet` (e.g. `BoardDetailsSheet`, `Canvas/BoardDetailsSheet.swift`).

## 3. Design

### 3.1 The control

`PartOptionsSection.applies(to:)` becomes `applies(to chip: PlacedChip, in catalog:)`: its
old test, or `!catalog.sockets(fitting: base footprint).isEmpty` for the chip's base
footprint. The sidebar passes the chip and catalog (`SchematicSidebarView.swift:72`).
Rows, top to bottom, in the Options section:

- **Mounting**, a segmented `Picker`: **IC**, **IC + Socket**, **Socket Only** → `document.setPartSocket(id, mode:, socket:)`;
  from IC the socket is `catalog.genericSocket(fitting:)`; between the two socket modes it is the current one.
- In a socket mode: `LabeledContent("Socket")` with the socket's part number, a caption with
  its manufacturer and `DIP socket 0.3 in, through hole` style text, and a **Choose Socket…**
  button.

### 3.2 The dialog

`SocketPickerSheet(chip:, catalog:, onUse:)`, presented by `.sheet` from the button. Title
`Sockets for <chip footprint description>`. A search field (part number, manufacturer,
description, case-insensitive substring), a **Mounting** menu (All, Through hole, Surface
mount) and a **Manufacturer** menu (All and the makers present), and a SwiftUI `Table`
of the filtered `catalog.sockets(fitting:)` in the catalog order (generics first): columns
**Part**, **Manufacturer**, **Mounting**, **Finish**, **Row spacing** (DIP only, in inches).
The row of the chip's current socket is selected on open. Buttons **Cancel** (Escape) and
**Use Socket** (Return, disabled with no selection; a double-click on a row is Use Socket).
**Use Socket** calls `onUse(part)` → `setPartSocket(id, mode: current mode, socket: part)`.
The filter logic is a pure function (`SocketPickerSheet.rows(_:search:mounting:maker:)`) so tests need no view.

Rejected: a menu of sockets in the sidebar (hundreds of rows, no filtering: the owner asked for a dialog);
a popover (a table with filters wants the width of a sheet).

### Risk

The sheet over a sidebar that re-reads the chip: the sheet holds only the chip's id and
re-reads the document, as the section does. The harness drives it end to end.

## 4. Phases

### Phase 1 — the control and the filter

- [ ] `PartOptionsSection.swift`: `applies(to:in:)`, the Mounting picker, the Socket row and button; `SchematicSidebarView.swift:72`.
- [ ] `Drawer/SocketPickerSheet.swift`: the view and `rows(_:search:mounting:maker:)`.
- [ ] `schematicalTests/SocketPickerTests.swift`: `applies` is true for `SN74HC00N` and false for `GENERIC-LED`, `GENERIC-HEADER-1X02`, a 7-segment display and a 4-pin DIP oscillator; `rows` with search `mill`, with Mounting `Surface mount`, with a maker, and with all three together; generics first; every row's `fits` holds `PDIP-14` for the 14-pin chip; the pure rows function never returns a socket that does not fit.

Acceptance: `SocketPickerTests` passes.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p51-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SocketPickerTests -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **` (est. 13 min).
Commit: —

### Phase 2 — the harness

- [ ] `scripts/harness/socket-mode-ui.swift` (the `generic-led-ui.swift` shape, `check(name) { … }` per line): places `SN74HC00N` and a `GENERIC-LED`; the sidebar of the gate renders the Mounting control and the LED's does not; `setPartSocket` through the same closure the picker's `onUse` calls changes the chip's `footprint.id`; the BOM of the document lists `XU1` in `IC + Socket` and drops the chip in `Socket Only`; the sheet renders (`Harness.render`) with the generic first and writes `p51-socket-sheet.png`; one undo returns the chip to IC.
- [ ] A real-window pass on the running app (AGENTS § Testing in the app: afterwards the pointer goes to (0, 0)): place a gate, switch to IC + Socket, open Choose Socket…, pick a Mill-Max row, check the PCB footprint changed and Edit › Undo restores it.

Acceptance: every harness line is `ok`.
  Check: `rm -rf /tmp/p51-harness && APP=/tmp/p51-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/socket-mode-ui.swift /tmp/p51-harness && /tmp/p51-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0 (est. 10 min).
Commit: —

## Validation Plan

- Tests: `SocketPickerTests`; `DrawerPerformanceTests` (the sidebar and drawer are on its path).
- Coverage check: the harness renders the real `PartOptionsSection` and `SocketPickerSheet` with the real catalog.
- Runtime proof: the real-window pass of Phase 2.
- Doc sync: none (`docs/` is H's and F's).
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions. Author decisions:

- **The control is named Mounting with the options the owner named**: IC, IC + Socket, Socket Only.
- **The dialog is a sheet with a table, search and two filters**; no datasheet button.
- **The control appears only for a chip a socket fits**, by the catalog's `fits` lists.

## Corrections

## Summary

I's risk is the sheet and sidebar re-reading the chip; the harness bounds it.
Untouched: the canvas, the drawer, the PCB sidebar, the simulation, every part file.
