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
| plan-51-H complete | `ls planning/completed/plan-51-H-*.md` → exists | MET (2026-11-24: `planning/completed/plan-51-H-socket-mode-model.md`, commit 44979c02) |
| `SocketModeTests` passes | the check of plan-51-H Phase 1 | MET (2026-11-24: `SocketModeTests` 15 of 15) |

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

- [x] `PartOptionsSection.swift`: `applies(to:in:)`, the Mounting picker, the Socket row and button; `SchematicSidebarView.swift:72`. — `PartOptionsSection`: `applies(to:in:)` (the old `applies(to: part)` stays for the LED and header options), `isSocketable`, `MountingChoice`, `choose(_:for:in:catalog:)`, the segmented Mounting control, the Socket row, its caption and Choose Socket…, the sheet; `SchematicSidebarView` passes the chip and catalog.
- [x] `Drawer/SocketPickerSheet.swift`: the view and `rows(_:search:mounting:maker:)`. — `SocketPickerSheet` (search, Mounting and Manufacturer menus, a `Table`, Cancel and Use Socket, a double-click is Use Socket) and the pure `rows(_:search:mounting:maker:)`.
- [x] `schematicalTests/SocketPickerTests.swift`: `applies` is true for `SN74HC00N` and false for `GENERIC-LED`, `GENERIC-HEADER-1X02`, a 7-segment display and a 4-pin DIP oscillator; `rows` with search `mill`, with Mounting `Surface mount`, with a maker, and with all three together; generics first; every row's `fits` holds `PDIP-14` for the 14-pin chip; the pure rows function never returns a socket that does not fit. — 6 cases; "applies" is the new `isSocketable` for the socket control (the LED and the header still have their own Options): true for `SN74HC00N` and a PLCC-44 CPU, false for `GENERIC-LED`, `GENERIC-HEADER-1X02`, `5V`, a 7-segment display, a 4-pin DIP oscillator and a socket.

Acceptance: `SocketPickerTests` passes.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p51-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SocketPickerTests -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **` (est. 13 min). Measured 2026-11-24 (with `SocketModeTests` and `TestPointTests` added): `SocketPickerTests` 6 of 6, `DrawerPerformanceTests`, `SocketModeTests` passed; the one failure was `TestPointTests.testTheDrawerListsTheLeafUnderConnector`, a list that had gone stale before this plan (§ Corrections), corrected afterwards. VERIFIED.
Commit: 86253641

### Phase 2 — the harness

- [x] `scripts/harness/socket-mode-ui.swift` (the `generic-led-ui.swift` shape, `check(name) { … }` per line): places `SN74HC00N` and a `GENERIC-LED`; the sidebar of the gate renders the Mounting control and the LED's does not; `setPartSocket` through the same closure the picker's `onUse` calls changes the chip's `footprint.id`; the BOM of the document lists `XU1` in `IC + Socket` and drops the chip in `Socket Only`; the sheet renders (`Harness.render`) with the generic first and writes `p51-socket-sheet.png`; one undo returns the chip to IC. — `scripts/harness/socket-mode-ui.swift`, 10 checks, built with `APP=/tmp/p51-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh` and run: every line `ok`, `all ok`, exit 0; the images (sidebar of the gate, the LED, a gate in IC + Socket, the sheet) were read.
- [~] A real-window pass on the running app (AGENTS § Testing in the app: afterwards the pointer goes to (0, 0)): place a gate, switch to IC + Socket, open Choose Socket…, pick a Mill-Max row, check the PCB footprint changed and Edit › Undo restores it. — NOT DONE as written: this session has no way to drive the real mouse (no computer-use tool, and the automation server's fixed port 47800 is held by the owner's running app), so the click-through on the running app (place a gate, IC + Socket, Choose Socket…, pick a Mill-Max row, check the PCB footprint, Edit › Undo) was not made. What stands in for it: the harness renders the app's real `SchematicSidebarView` and `SocketPickerSheet` in a hosted window and drives the same closures the controls call (checks 4 to 9). What remains: the owner's pass on the running app.

Acceptance: every harness line is `ok`.
  Check: `rm -rf /tmp/p51-harness && APP=/tmp/p51-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/socket-mode-ui.swift /tmp/p51-harness && /tmp/p51-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0 (est. 10 min). Measured 2026-11-24: `all ok`, exit 0 (10 of 10). VERIFIED for the harness; the real-window pass is the open item above.
Commit: 86253641

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

- **2026-11-24, "applies" for the socket control is `isSocketable`.** § 3.1 says `applies(to:)` becomes `applies(to chip:in:)`, and § 1's test text says it is false for `GENERIC-LED` and `GENERIC-HEADER-1X02`; but those two have Options of their own (colors, footprint choices, rows) and `applies` stays true for them. So `applies(to: part)` is kept, `applies(to: chip, in: catalog)` is it or `isSocketable`, and the test asserts `isSocketable`: false for the LED, the header, a supply, a 7-segment display, a 4-pin DIP oscillator and a socket.
- **2026-11-24, `TestPointTests.testTheDrawerListsTheLeafUnderConnector` was stale and moved.** It listed Connector's groups as `Barrel, Card slot, Header, USB`; plan-45 had already added JST and Terminal block, and plan-51 adds IC Socket. Now `["Barrel", "Card slot", "Header", "IC Socket", "JST", "Terminal block", "USB"]`.
- **2026-11-24, the sheet's picks are bundled sockets only** (H § Corrections). The table columns were widened after the harness image showed part numbers and maker names cut off (`width(min:ideal:)`).
- **2026-11-24, the real-window pass was not made.** See the Phase 2 line: no mouse-driving tool in this session. The remaining step is the owner's.

## Summary

I's risk is the sheet and sidebar re-reading the chip; the harness bounds it.
Untouched: the canvas, the drawer, the PCB sidebar, the simulation, every part file.
