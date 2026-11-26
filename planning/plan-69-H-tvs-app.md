# plan-69-H: App — TVS details, facts, kind names, the TVS symbols, reference `D`, exports, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-69-G

H makes the app show TVS parts: their facts in the sidebar, their kind names, the
unidirectional and bidirectional TVS symbols, reference `D`, the board preview's
diode body, and the exports' symbol boxes. The voltage check's new problem reaches
the user through the existing "is rated" message.

References:

- plan-69-E § 3.1 (`PartTvs`), § 3.2 (`VOLTAGE_TVS_OVER`); plan-69-F § 3.1 and
  plan-69-G § 3.1 (drawer paths, pin names `K`/`A` and `1`/`2`).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details`, its builder,
  `symbol(for:)`: the `PART_DIODE` branch), `Backend/PartFacts.swift`
  (`kindName(_ kind:)`, `kindName(_ part:)`, `rows(for:)`),
  `Preferences/ReferencePrefixes.swift` (`ReferenceCategory.of`),
  `Canvas/SchematicModel.swift` (`PartSymbol`), `Canvas/DiodeSymbolView.swift`
  (`DiodeMark`), `BoardPreview/PartLook.swift`, `Simulation/VoltageCheck.swift`
  (`message`).
- `scripts/harness/diode-app.swift`, `scripts/harness/build.sh` (the harness
  precedent); `planning/completed/plan-48-H-timer-opamp-app.md`,
  `planning/completed/plan-47-M-ifrb-app-inductor-fuse.md` (the precedents).

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-G complete | `ls planning/completed/plan-69-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25: G not started) |
| The app does not know the kind's views yet | `grep -c 'PART_TVS' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift schematical/schematical/Preferences/ReferencePrefixes.swift` → 0, 0, 0 | MET (2026-11-25: 0, 0, 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

On the app's real catalog and document (`scripts/harness/tvs-app.swift`, every line
`ok`):

- The drawer has **Discrete → TVS diodes** with the sub-groups `Unidirectional`,
  `Bidirectional` (leaves: the standoff bands) and `ESD arrays` (leaves: the line
  counts).
- The most-stocked unidirectional SMB part placed: reference `D1`, symbol
  `.plainDiode` with mark `.tvs`; kind name "TVS diode"; facts rows `Standoff`,
  `Breakdown (min)`, `Clamping (max)`, `Peak pulse` (`65.2 A (10/1000 µs)`),
  `Peak power`, `Direction` (`Unidirectional`), `Play mode` (`Not simulated:
  drives nothing; rated by its standoff voltage`).
- The most-stocked bidirectional part placed: symbol `.tvsBidirectional`, kind
  name "TVS diode", `Direction` `Bidirectional`.
- The most-stocked steering part placed: a block symbol, reference `D2`, kind name
  "ESD array", rows `Lines` (`4`), `Rail` (`VBUS`), `Standoff`, `Capacitance`.
- Wiring the unidirectional part's `K` to a 12 V supply is refused with "Can't
  connect: D1 pin 1 (K) gets 12 V; <part> is rated 5 V." (for a 5 V part); wiring
  it to 5 V is accepted, and Run stays available (`SimulationAvailability.canStart`).

### Non-goals (explicit constraints)

- No play-mode drawing (no conduction, no glow): a TVS drives nothing.
- No change to the saved file format: a TVS is a placed chip like any other.
- No new reference category: TVS parts are diodes (`D`).

## 2. Current State

- `CatalogPart.Details` has one case per kind with facts (`.diode(type:diodes:vrMV:ioMA:vzMV:pdMW:)`
  at `Backend/PartCatalog.swift:751`); `symbol(for:)` draws a single diode with pins
  `A` and `K` as `.plainDiode`, else a block (`PartCatalog.swift:2049`).
  `PartFacts.kindName(_ kind:)` is at `PartFacts.swift:377`, `kindName(_ part:)` at
  `:478`. `ReferenceCategory.of` (`Preferences/ReferencePrefixes.swift:75`) gives
  `.diode` / `.zenerDiode` for `PART_DIODE`.
- `DiodeMark` (`Canvas/DiodeSymbolView.swift:7`) has `.led`, `.plain`, `.schottky`,
  `.zener`, chosen from `.diode(type:…)`.
- `.plainDiode` is matched in 8 files (`grep -rln plainDiode schematical/schematical | wc -l`
  → 8): `PartCatalog.swift`, `KiCadSchematicWriter.swift:467`,
  `EasyEDAProSchematicWriter.swift:128` (the two-terminal body box),
  `SchematicModel.swift:157`, `:241`, `:339`, `SchematicExportScene.swift:65`,
  `ReferenceLabel.swift:22`, `SchematicCanvasView.swift:1275`, `BoardModel.swift:253`.
- `BoardPreview/PartLook.swift:130`: `PART_DIODE`'s body looks (axial, SMD, TO).
- `VoltageCheck.message` (`Simulation/VoltageCheck.swift:137`) words every rating
  problem it does not name in its `default` case: "… gets N V; <part> is rated M V."

## 3. Design

### 3.1 Catalog (`Backend/PartCatalog.swift`)

```swift
struct TvsInfo: Hashable, Sendable {
    let type: String; let diodes: Int; let bidirectional: Int; let lines: Int; let hasRail: Bool
    let vrwmMV: UInt32; let vbrMV: UInt32; let vcMV: UInt32; let ippMA: UInt32; let ippWave: String?
    let ppkW: UInt32; let cjFF: UInt64
}
```

`Details.tvs(TvsInfo)`; the builder branches on `part.tvs?.pointee`; accessor
`tvs`. `detailLabels`: the standoff (`"5 V"`) and, for a steering array, the line
count. `symbol(for:)`: `PART_TVS` with one diode, unidirectional, pins `K` and `A`
→ `.plainDiode`; one diode, bidirectional, pins `1` and `2` → `.tvsBidirectional`;
anything else → `.block`.

### 3.2 Symbols (`Canvas/`)

- `DiodeMark.tvs`: the cathode bar with both ends bent the same way (the
  unidirectional TVS mark; Zener's ends bend opposite ways); `DiodeMark(part:)`
  returns it for `.tvs` details.
- `PartSymbol.tvsBidirectional` and `TvsSymbolView`: two filled triangles meeting
  at one TVS bar, pin `1` left and pin `2` right, the function label below — the
  `DiodeSymbolView` frame and margins. Every switch that names `.plainDiode` in the
  8 files of § 2 names `.tvsBidirectional` beside it (the export body boxes, the
  reference label place, the board's footprint orientation, the export scene).

### 3.3 Facts and names (`Backend/PartFacts.swift`)

- `kindName(_ kind:)`: `PART_TVS` "TVS diode". `kindName(_ part:)`: "ESD array" for
  `steering` or more than one diode.
- Rows: `Standoff` (`vrwm`), `Breakdown (min)` (`vbr`), `Clamping (max)` (`vc`),
  `Peak pulse` (`ipp` with its waveform, `"65.2 A (10/1000 µs)"`), `Peak power`
  (`ppk`, `"600 W"`, `"1.5 kW"`), `Capacitance` (`cj`, `"0.8 pF"`), `Direction`
  (`Unidirectional`, `Bidirectional`, or `Mixed` for an array with both), `Lines`
  and `Rail` (steering), each only when given; `Play mode`: `Not simulated: drives
  nothing; rated by its standoff voltage`.

### 3.4 Reference, board look

- `ReferenceCategory.of`: `case PART_TVS: return .diode`.
- `PartLook`: `PART_TVS` takes `PART_DIODE`'s branch (axial, SMD two-pad, TO); an
  array package falls through to the IC looks as other kinds do.

### 3.5 Harness (`scripts/harness/tvs-app.swift`)

From `diode-app.swift`: loads the real catalog, checks the drawer groups, places
the three parts of § 1 by name (the most-stocked of each, read from
`planning/plan-69-data/candidates.tsv` when the harness is written and named in
it), checks references, symbols, kind names and rows, then wires the
unidirectional part's `K` to a `+12V` supply (refused, the message) and to a `+5V`
supply (accepted), and checks `SimulationAvailability.canStart`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Catalog, facts, names, reference

- [ ] § 3.1 (without the symbol cases), § 3.3, § 3.4 (`grep -c 'PART_TVS'` in each
      file → 0 before; another plan's kind cases stay where they are).
- [ ] `schematicalTests/TvsTests.swift`: the details builder, the rows of § 1 for
      the three parts, `kindName`, `ReferenceCategory.of`, and
      `SimulationAvailability.canStart` with a TVS placed.

Acceptance: the catalog builds the details and the sidebar rows.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/TvsTests` → `** TEST SUCCEEDED **` (est. 8 min: an app build and one class).
Commit:

### Phase 2 — Symbols and exports

- [ ] § 3.1's symbol cases, § 3.2 (`DiodeMark.tvs`, `PartSymbol.tvsBidirectional`,
      `TvsSymbolView`, the 8 files).
- [ ] `TvsTests`: `symbol(for:)` for the three parts; a KiCad and an EasyEDA
      schematic export of a sheet with the bidirectional part give its body box the
      two-terminal span (the `.plainDiode` rule).

Acceptance: the three symbols are chosen and exported.
  Check: `xcodebuild … test -only-testing:schematicalTests/TvsTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/SchematicRotateTests` → `** TEST SUCCEEDED **` (est. 9 min; `SchematicRotateTests` holds the `.plainDiode` placement cases the new symbol sits beside).
Commit:

### Phase 3 — Harness

- [ ] `scripts/harness/tvs-app.swift` per § 3.5.

Acceptance: the real app shows and checks TVS parts.
  Check: `APP=/tmp/p69-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/tvs-app.swift /tmp/p69-harness && /tmp/p69-harness/H.app/Contents/MacOS/harness` → every line `ok` (est. 5 min).
Commit:

## Validation Plan

- Tests: `TvsTests`, `KiCadExportTests`, `EasyEDAProExportTests`, `SchematicRotateTests`; the harness.
- Coverage check: `grep -c 'PART_TVS' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift schematical/schematical/Preferences/ReferencePrefixes.swift schematical/schematical/BoardPreview/PartLook.swift` → each above 0.
- Runtime proof: the harness's refused 12 V connection and accepted 5 V one.
- Doc sync: `docs/parts-format.md` § `tvs` names the two symbols (one sentence).
- Final gate: plan-69-I.

## Decisions

- Author: **reference `D`** for every TVS part, arrays included: they are diodes,
  and KiCad's TVS and ESD-array symbols use `D`.
- Author: **the unidirectional TVS reuses the diode symbol with its own mark**; the
  bidirectional one gets its own view (two triangles), and arrays are blocks, as
  diode arrays are.
- Author: **the voltage message needs no new case**: "is rated M V" is exactly what
  a standoff rating says.

## Corrections

## Summary

Mostly additions beside the diode's branches; the one new view is the
bidirectional symbol, and the 8 files that name `.plainDiode` are where it must
be added too.
