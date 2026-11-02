# plan-12-B: Pull-up and Pull-down toolbar tools

Last updated: 2026-11-02
Effort: medium (1h–2h)
Depends on: plan-12-A

The owner asked for generic pull-up and pull-down parts "the same as VCC and
GND in the app (in the toolbar)". A added the parts (`PULLUP-5V` …
`PULLUP-12V`, `PULLDOWN`); B adds two schematic tools that place them, and
their symbols.

Outcome: the schematic toolbar has **Pull-up** and **Pull-down** tools next
to VCC and GND. Pull-up places `PULLUP-<rail>` for the rail picked in the
existing supply menu (5V by default), Pull-down places `PULLDOWN`. Both draw
as a resistor to a rail (pull-up) or to ground (pull-down), fixed upright
like VCC and GND, and are wired like any one-pin part.

References: plan-12-A §3.6, §4.2; `planning/completed/plan-06-A-board-model.md`
§ Harness (the headless app harness); `planning/completed/plan-09-E-cpu-app.md`
§5 (its use for a runtime proof).

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-A landed | `grep -c '"PULLUP-5V"' schematical/schematical/Parts/supply.json` → 1; `grep -n 'int pull' backend/include/parts.h` → one line | NOT MET (2026-11-02) |

## 1. Goal

- The toolbar's tool picker lists, in order: Pointer, Eraser, Line, Flip,
  VCC, GND, Pull-up, Pull-down.
- With the supply menu at `3V3`, a click with Pull-up places `PULLUP-3V3`;
  Pull-down places `PULLDOWN`. Picking a rail in the menu while Pull-up is the
  tool keeps Pull-up (it selects VCC otherwise, as today).
- Both draw with `PartSymbol.pullUp` / `.pullDown`: pin stub on the bottom
  (pull-up) or top (pull-down) edge, like VCC/GND; Flip does nothing to them.
- Checks: `xcodebuild … build` → `** BUILD SUCCEEDED **`; the harness
  commands of Phase 1 print the lines given there.

### Non-goals

- No change to VCC/GND behavior, the supply menu's rails or saved files of
  existing schematics.
- No resistance value (the parts are logic-level pulls, plan-12-A §3.6).
- Pulls stay out of the parts drawer (`"drawer": []`, as supplies).

## 2. Current State

- `Canvas/SchematicModel.swift`: `SchematicTool` (`pointer eraser line flip vcc
  ground`, `title`, `symbol`, `supplyName(voltage:)` → the VCC rail name or
  `"GND"`), `SupplyChoice.names = ["5V", "3V3", "2V5", "1V8", "1V2", "12V"]`,
  `PartSymbol` (`vcc`, `ground`, `hasVerticalPins`, `canFlip`).
- `ContentView.swift:204-226`: the tool `Picker` over `SchematicTool.allCases`
  and the supply `Menu`, whose buttons set `supplyVoltage` and `tool = .vcc`.
- `Canvas/SchematicCanvasView.swift:327-330`: a click with a tool whose
  `supplyName` is non-nil places that part.
- `Backend/PartCatalog.swift:498-499`: a supply's symbol is `.ground` when its
  `level` is 0, else `.vcc`.
- `Canvas/SupplySymbolView.swift`: draws VCC (bar) and GND (ground lines).

## 3. Design

- `SchematicTool` gains `.pullUp` (title "Pull-up", SF Symbol
  `arrow.up.circle`) and `.pullDown` ("Pull-down", `arrow.down.circle`), after
  `.ground`. `supplyName(voltage:)`: `.pullUp` → `"PULLUP-" + voltage`,
  `.pullDown` → `"PULLDOWN"`.
- The supply menu's buttons set `supplyVoltage` and then `tool = .pullUp` when
  the tool is `.pullUp`, else `.vcc`; its help text becomes "Supply voltage for
  the VCC and Pull-up tools".
- `PartSymbol` gains `.pullUp` and `.pullDown`; `hasVerticalPins` is true for
  them (so `canFlip` is false).
- `PartCatalog`: a supply part with `pull` set gets `.pullUp` (level 1) or
  `.pullDown` (level 0); other supplies as today.
- `SupplySymbolView`: for `.pullUp`, a rail bar labelled with the rail
  (`5V`, …) at the top, a vertical zig-zag resistor below it, the pin stub at
  the bottom; for `.pullDown`, the pin stub at the top, the resistor, then the
  GND symbol. Same size and colors as VCC/GND; the resistor is 6 grid steps
  tall.
- Every switch over `SchematicTool` or `PartSymbol` gets the new cases
  (`SchematicCanvasView.swift:165, 327`, `ChipBlockView.swift`'s color
  switch): pulls take the VCC/GND branches.

Rejected: one Pull tool with an up/down menu (the owner asked for them like
VCC and GND, which are separate tools); a 2-pin resistor part (the owner
picked one-pin pulls that follow the VCC menu).

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Tools and symbols

- [ ] `SchematicModel.swift`, `ContentView.swift`, `PartCatalog.swift`,
  `SupplySymbolView.swift`, `SchematicCanvasView.swift`, `ChipBlockView.swift`:
  §3. `xcodebuild -project schematical/schematical.xcodeproj -scheme
  schematical -configuration Debug -derivedDataPath /tmp/p12-dd
  CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **`.
- [ ] Harness (`/tmp/p12/harness/build.sh`, plan-06-A § Harness) command
  `pulls`: prints `SchematicTool.allCases` titles, `supplyName` for
  `.pullUp` with `3V3` and for `.pullDown`, and the catalog symbol of
  `PULLUP-5V`, `PULLDOWN`, `5V`, `GND`. Expected: `Pointer Eraser Line Flip
  VCC GND Pull-up Pull-down`; `PULLUP-3V3`; `PULLDOWN`; `pullUp pullDown vcc
  ground`.
- [ ] Harness command `pullproof`: builds a document with `SN74HC266N` (device `74_266`,
  open-drain XNOR), `PULLUP-5V` on its output and `5V`/`GND` on its power, runs
  1 µs, prints the output net value with input A = B = 0 (expected `1`, XNOR
  of equal inputs releases the pin and the pull-up wins) and renders the
  canvas to `/tmp/p12-proof/pulls.png`. The PNG is looked at: the pull-up
  symbol shows its rail label, resistor and pin stub.

Acceptance: the build succeeds and the two harness commands print as above;
the PNG is looked at (about 3 min).
Commit: —

## Validation Plan

No Swift test target exists; the harness drives the new paths (tool list,
part names, catalog symbol, a running circuit with a pull-up). The landing
tier is plan-12-K's.

## Decisions

- Pull-up uses the supply menu's rail (owner); picking a rail keeps the
  Pull-up tool when it is active, so the menu serves both tools.
- Pull parts are upright, unflippable symbols like VCC/GND.

## Corrections

(none yet)
