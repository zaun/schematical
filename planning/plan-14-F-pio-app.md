# plan-14-F: App — the `pio` kind (catalog, facts, Interface drawer, symbol)

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-14-E

F brings the `pio` kind into the app: the catalog reads `PartPio`, the facts
panel shows a PIA's or VIA's rows, the drawer lists them under
**Interface** next to plan-12's UARTs, and they draw as chip blocks with
their data and port pins grouped. The W65C51N needs no new app code
(plan-12-K's `serial` support); F checks it shows.

Outcome of F, in the headless harness: `catalog` prints `Interface: … PIA
(4), UART (n), VIA (3) …`; `facts W65C22S6TPG-14` prints the §3.2 rows;
`facts W65C51N6TQG-14` prints plan-12-K's serial rows with `Model acia6551`
and `Footprint LQFP-32-7X7`; a schematic with a W65C22S and a W65C21N renders
both as blocks (PNG).

References: `schematical/schematical/Backend/PartCatalog.swift`
(`CatalogPart.Details`, `symbol(for:)`), `Backend/PartFacts.swift`
(`kindName`, the details switch), `Canvas/ChipBlockView.swift` (pin groups);
`planning/completed/plan-12-K-app.md` (the `serial` kind's app work and its
harness — the pattern), `planning/completed/plan-09-E-cpu-app.md` (§ 5
harness), `planning/completed/plan-06-A-board-model.md` § Harness.

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-E landed | `grep -c '^W65C51N' backend/tests/golden/parts.txt` → 3 | NOT MET (2026-11-01) |

## 1. Goal

- `xcodebuild … build` → `** BUILD SUCCEEDED **`.
- Harness `catalog` → the Interface line of §3.3; `facts W65C22S6TPG-14` and
  `facts W65C21N6TPLG-14` → the §3.2 rows; `facts W65C51N6TQG-14` → serial
  rows incl. `Footprint LQFP-32-7X7`.
- Harness `render` of the two-chip schematic → a PNG with two blocks laid out
  as §3.4.

### Non-goals

- No backend change. No terminal-window change. No window for PIA/VIA state
  (no `circuit_*` state API for `pio` exists or is added).

## 2. Current State

(After plans 12 and 14-E.) `CatalogPart.Details` (PartCatalog.swift:150–170)
has a case per kind with facts (`cpu(core:grade:maxHz:)` at :165; plan-12-K
adds `serial`); `PartCatalog.swift:399` fills `.cpu` from `part.cpu`.
`PartFacts.kindName` (PartFacts.swift:73–91) maps `PartKind` → text, default
`""`; `symbol(for:)` (PartCatalog.swift:516) returns `.block` by default, so a
`pio` part already draws as a block. The drawer groups by `drawer` then
`function` (`groups(matching:)`, PartCatalog.swift:645).

## 3. Design

### 3.1 Catalog

- `CatalogPart.Details` + `case pio(model: String, grade: String, maxHz: UInt64)`;
  filled from `part.pio` where `.cpu` is filled.
- `symbol(for:)` unchanged (`.block`).

### 3.2 Facts

- `kindName(PART_PIO)` → `"Peripheral I/O"`.
- Rows for `.pio`: `Model` → `modelName(model)` (`pia6521` → `"6521 PIA"`,
  `via6522` → `"6522 VIA"`, otherwise the raw name), `Speed grade` → grade,
  `Max clock` → `"\(maxHz / 1_000_000) MHz"` (14 for every plan-14 part),
  then the common Supply rows.
- So `facts W65C22S6TPG-14` → Kind `Peripheral I/O`, Function `VIA`, Package
  `DIP`, Pins 40, Model `6522 VIA`, Speed grade `14`, Max clock `14 MHz`,
  Supply range `1.71–5.5 V`, Footprint `PDIP-40-600`. `facts
  W65C21N6TPLG-14` → Function `PIA`, Model `6521 PIA`, Pins 44, Supply
  range `4.5–5.5 V`, Footprint `PLCC-44`.

### 3.3 Drawer

No code: the parts' `"drawer": ["Interface"]` and functions `PIA`/`VIA`
place them. Harness `catalog` → the Interface category lists `PIA (4)` and
`VIA (3)` among plan-12's functions (UART count = plan-12's + 3 W65C51N).

### 3.4 Block symbol

No code. `ChipLayout(pins:)` (Canvas/SchematicModel.swift:306) puts power,
inputs and no-connects on the left and outputs and `inout` pins on the
right, each side split by `grouped` (default group first in pinout order,
then each named group in order of first appearance, sorted by name) with an
empty row between groups. So a W65C21N6TPLG-14 draws:

- left: VDD; then the inputs in pin order — 19 CB1, 23 RWB, 24 CS0,
  25 CS2B, 26 CS1, 28 PHI2, 37 RESB, 38 RS1, 39 RS0, 44 CA1; then the NC pins;
- right: CB2, IRQBB, IRQAB, CA2 (the default group, pin order 20, 41, 42,
  43); gap; D0–D7; gap; PA0–PA7; gap; PB0–PB7; then VSS at the bottom.

A W65C22S6TPG-14 draws the same way; its default output group is, in pin
order, 18 CB1, 19 CB2, 21 IRQB, 39 CA2 (CB1 is `inout` on the VIA: it is the
shift clock output in some modes).

## 4. Runtime proof (harness)

The plan-06-A harness approach (`/tmp/p14/harness/build.sh`, as plan-12-K's):

1. `catalog` → the Interface line.
2. `facts W65C22S6TPG-14`, `facts W65C21N6TPLG-14`, `facts W65C51N6TQG-14`.
3. `render /tmp/p14/pio.schematical /tmp/p14/pio.png` of a document with one
   W65C22S6TPG-14 and one W65C21N6TPLG-14 placed, written with the
   harness's document API. Look at the PNG.

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — Catalog and facts

- [ ] `PartCatalog.swift`: `.pio` details (§3.1).
- [ ] `PartFacts.swift`: `kindName`, `modelName`, the `.pio` rows (§3.2).
- [ ] Harness `catalog` and `facts` commands (§4 items 1–2).

Acceptance: the build and the harness output of §1.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p14f-dd build` → `** BUILD SUCCEEDED **` (est. 2 min); `/tmp/p14/harness/run catalog`, `run facts …` → the §3.2/§3.3 rows (est. seconds).
Commit: —

### Phase 2 — Symbol render

- [ ] Harness `render` of the two-chip document (§4 item 3); attach the PNG
      path in the commit message.

Acceptance: the PNG shows both chips as blocks with the pin order of §3.4.
  Check: `/tmp/p14/harness/run render /tmp/p14/pio.schematical /tmp/p14/pio.png` → the PNG, inspected (est. 1 min).
Commit: —

## Validation Plan

Harness output and the PNG. The landing tier is plan-14-J's.

## Decisions

- Author: Kind text `Peripheral I/O`; models shown as `6521 PIA`, `6522 VIA`
  (§3.2).

## Corrections

(none yet)
