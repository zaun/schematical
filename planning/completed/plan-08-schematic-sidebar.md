# Context-aware schematic sidebar Plan

Last updated: 2026-11-01
Effort: medium (1h–2h)

The Schematic page's sidebar follows the selection, as the PCB sidebar
does (plan-06-C). With no part selected it is the part library (today's
parts drawer). With a part selected it shows the part: reference, part
number and description at the top; the reference as prefix + number fields,
the same as on the PCB sidebar; and the part's details: kind, function,
package, footprint, flags, and the facts of its kind (logic family, memory
size and timings, oscillator range, colours, switch positions, supply
level). Dropping a part from the library no longer selects it, so the
library stays open for the next drop.

Outcome: open `test.schematical`, click the LED on the Schematic page: the
sidebar shows `U…`, `LED-AMBER-0603`, its description, prefix/number
fields and Kind LED, Package, Footprint `LED-0603 — …`, Color (the part's LED colour). Rename
it `D` `1`: the PCB sidebar and memory-window titles show `D1`. Click empty
canvas: the library comes back with its search text, filters and open
groups as they were. Drag a part in: it is placed, not selected, and the
library stays.

References:

- `planning/completed/plan-06-C-pcb-sidebar.md` (sidebar shape, reference
  fields, window-capture method) and `planning/completed/plan-06-A-board-model.md`
  (§ Harness, `renameChip`, `ReferenceParts`).
- `schematical/schematical/ContentView.swift` (inspector),
  `Canvas/PCBSidebarView.swift` (`FootprintPanel`),
  `Drawer/ChipDrawerView.swift`, `Backend/PartCatalog.swift`
  (`CatalogPart`, `Details`, `Flags`), `Canvas/SchematicModel.swift`
  (`placeChip`).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-06 complete | `ls planning/completed/plan-06-{A,B,C,D}-*.md \| wc -l` → 4 | MET (2026-11-01, re-run at a64fb17 → 4) |
| App builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01, re-run at a64fb17 → `** BUILD SUCCEEDED **`) |

Everything below is written against a tree where both hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- Schematic inspector: the parts library with nothing selected; the part
  panel (header, reference fields, details) with a part selected.
- The library keeps its search, filters, open groups and scroll position
  while the part panel is shown.
- `placeChip` (drop from the library, VCC and GND tools) does not select
  the new part.
- The PCB footprint panel's header and reference fields are shared with
  the schematic panel and look and behave as before.

### Non-goals (explicit constraints)

- No file-format, model-data or backend change. References are still
  renamed only through `SchematicDocument.renameChip`.
- No new facts from the backend: the panel shows what `CatalogPart`
  already holds (no timing, no pin table, no connections — owner's choice).
- The inspector toggle ("Parts Drawer", `isDrawerShown`) and column width
  are unchanged; play mode keeps clearing the selection at Run and never
  selects, so the library (disabled) shows while running, as today.
- The PCB sidebar's footprint, text and board panels are otherwise
  unchanged.

## 2. Current State

- `ContentView.swift:131`: one `.inspector(isPresented: inspectorBinding)`;
  Schematic shows `ChipDrawerView(catalog:)` (`.disabled` while running),
  PCB shows `PCBSidebarView(document:)`.
- `ChipDrawerView` keeps its search filter (`@State filter`) and open
  groups (`@State toggled`) as view state (`ChipDrawerView.swift:24,31`),
  so removing it from the view tree resets them.
- `PCBSidebarView.swift:219` `FootprintPanel`: header (reference
  `.title2.bold`, part name `.headline`, description `.callout` secondary,
  footprint caption) then Section "Reference" (prefix/number fields,
  commit on Return or leaving the pair, red message, `resetReference`) —
  lines 234–273, private to that file.
- `SchematicModel.swift:744`: `placeChip` ends with
  `selectedChipID = placed.id`. Callers: the drop handler
  (`SchematicCanvasView.swift:108`) and the VCC/GND tools (`:330`).
- Schematic selection: `beginPointerInteraction`
  (`SchematicCanvasView.swift:366`) selects a clicked part and clears it on
  wires or empty canvas; play mode only pans (`:254`). The selection is
  shared with the PCB page.
- `CatalogPart` (`PartCatalog.swift:115`): `name`, `kind`, `package`,
  `function`, `description`, `flags` (obsolete, military, automotive,
  enhanced), `pins`, `details`, `footprint`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Part kinds | 10 | `sed -n 25,26p backend/include/parts.h` → `PART_GATE, PART_DFF, PART_JKFF, PART_SRAM, PART_SWITCH, PART_LED, PART_7SEG, PART_OSCILLATOR, PART_SUPPLY, PART_EEPROM` |
| `Details` cases | 8 | `sed -n 124,133p schematical/schematical/Backend/PartCatalog.swift \| grep -c '^        case '` → 8 (logic, sram, eeprom, dipSwitch, led, display, oscillator, supply) |
| `placeChip` callers | 2 | `grep -n 'placeChip(' schematical/schematical/Canvas/SchematicCanvasView.swift` → lines 108, 330 |
| Frequency formatters in the app | 0 | `grep -rn 'MHz\|kHz' schematical/schematical \| grep -c '\.swift:'` → 0 |

### Verified properties

- `.hidden()` / opacity keeps a view in the tree, so its `@State` survives;
  the design keeps `ChipDrawerView` in a `ZStack` under the panel instead
  of swapping it out (`grep -c 'onDisappear\|\.task' schematical/schematical/Drawer/ChipDrawerView.swift`
  → 0, so nothing in it re-runs). Its `List` keeps its scroll offset for the
  same reason.
- Nothing else relies on `placeChip` selecting: `grep -rn selectedChipID
  schematical/schematical` lists the canvas, `ContentView` (Run clears it,
  Rotate reads it), the two sidebars and the document — none reads it
  right after placing.

## 3. Design Overview

1. **Shared part views** (`Canvas/PartInfoViews.swift`, new): `PartHeader`
   and `ReferenceSection`, moved out of `FootprintPanel` unchanged.
2. **Facts** (`Backend/PartFacts.swift`, new): `PartFacts.rows(for:) ->
   [(label: String, value: String)]`, a pure function the harness can print.
3. **Schematic sidebar** (`Drawer/SchematicSidebarView.swift`, new): drawer
   kept alive underneath; part panel on top when a part is selected.
4. **No selection on place** (`SchematicModel.swift`).

Risk is low: no data changes. The one behaviour change is the PCB panel
refactor (must look and act the same — window capture before/after) and
`placeChip` not selecting.

Rejected alternatives:

- *Swap `ChipDrawerView` out of the tree* — resets search, filters, open
  groups and scroll on every selection.
- *Lift the drawer's state into an observable object* — keeps search and
  groups but loses scroll position; more code than keeping the view alive.
- *A copy of the reference fields in the schematic panel* — two copies of
  commit/revert logic drift; one shared view.

## 4. Detailed Design

### 4.1 PartInfoViews.swift

- `struct PartHeader: View { let chip: PlacedChip }` — reference, part
  name, description, exactly as today's `FootprintPanel` header without
  the footprint caption. `FootprintPanel` shows `PartHeader` and then its
  footprint caption in the same `Section`, so the PCB look is unchanged.
- `struct ReferenceSection: View { let document: SchematicDocument; let
  chip: PlacedChip }` — today's Section "Reference" with its `@State`
  (`prefix`, `number`, `referenceError`, focus), `commitReference`,
  `resetReference`, moved verbatim. `FootprintPanel` uses it; its
  reference state and functions are deleted there.

### 4.2 PartFacts.swift

`nonisolated enum PartFacts { static func rows(for part: CatalogPart) ->
[(label: String, value: String)] }`, rows in this order, a row left out
when its value is empty:

- Kind — gate "Logic gate", dff "D flip-flop", jkff "JK flip-flop", sram
  "SRAM", eeprom "EEPROM", switch "DIP switch", led "LED", 7seg
  "7-segment display", oscillator "Oscillator", supply "Supply".
- Function — `part.function`.
- Package — `part.package`.
- Footprint — "\(id) — \(description)"; left out with no footprint.
- Pins — count of pins that are not `isHidden`.
- Flags — "Obsolete", "Military", "Automotive", "Enhanced" joined by ", ".
- Kind facts from `details`:
  - logic: Family; Units (the count).
  - sram: Size; Access "\(n) ns"; Grade.
  - eeprom: Size; Access; Grade; Page "\(n) bytes"; Write cycle
    (`writeCycleUs` < 1000 → "\(n) µs", else ms with up to 3 decimals,
    trailing zeros trimmed).
  - dipSwitch: Positions; Actuator.
  - led: Color.
  - display: Digits; Color; Digit size; Type "Common anode" / "Common
    cathode"; LED chip.
  - oscillator: Family; Range "\(min) – \(max)"; Default; Enable pin (the
    designator, or "None").
  - supply: Level "High (VCC)" for level ≠ 0, "Low (GND)" for 0.
- Size: "\(words) × \(bits)" then the byte total in parentheses —
  `words × bits / 8` bytes as "N bytes" under 1024, else KB (÷1024) or MB
  (÷1 048 576) with up to 2 decimals, trimmed: `32768 × 8 (32 KB)`.
- Frequencies: `PartFacts.hertz(_:)` — ≥ 1e6 → MHz, ≥ 1e3 → kHz, else Hz,
  up to 3 decimals, trailing zeros trimmed: 32768 → "32.768 kHz",
  110 000 000 → "110 MHz".

### 4.3 SchematicSidebarView.swift

```swift
struct SchematicSidebarView: View {
    let catalog: PartCatalogStore
    let document: SchematicDocument
    let simulation: SimulationController
    // selected = document.selectedChipID's chip, if any
    var body: some View {
        ZStack {
            ChipDrawerView(catalog: catalog)
                .disabled(simulation.isRunning)
                .opacity(selected == nil ? 1 : 0)
                .allowsHitTesting(selected == nil)
                .accessibilityHidden(selected != nil)
            if let chip = selected {
                Form {
                    Section { PartHeader(chip: chip) }
                    ReferenceSection(document: document, chip: chip)
                    Section("Details") {
                        ForEach(rows) { LabeledContent($0.label, value: $0.value) }
                    }
                }
                .formStyle(.grouped)
                .id(chip.id)   // fresh field state per part
            }
        }
    }
}
```

`ContentView`: the Schematic branch of the inspector becomes
`SchematicSidebarView(catalog:document:simulation:)`.

### 4.4 placeChip (`SchematicModel.swift`)

Delete `selectedChipID = placed.id`; update its doc comment ("Places a
block… The new part is not selected.").

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Shared views and facts

The PCB panel moves onto the shared views; facts exist with no UI yet.

- [x] Before the change: plan-06-A § Harness, plan-06-C's `window-c2`
      capture of the LED's footprint panel → `/tmp/p08/pcb-before.png`. —
      `harness pcb <repo> before` (built at a64fb17, no code change):
      `LED fields: ["U", "3", "2.54", "-0.64"]`, `captured
      /tmp/p08/pcb-before.png` (U3 / LED-AMBER-0603 / Amber LED / LED-0603
      caption, Reference U 3, Position, Marks); same run renames and
      captures the duplicate message → `pcb-dup-before.png`.
- [x] `Canvas/PartInfoViews.swift` (new) and `Canvas/PCBSidebarView.swift`:
      §4.1. — `PartHeader` is its own leading `VStack` (Corrections 3);
      `git diff --stat` → PCBSidebarView 2 insertions, 51 deletions.
- [x] `Backend/PartFacts.swift` (new): §4.2, plus the voltage rows of
      Corrections 2.

Acceptance: the PCB footprint panel is unchanged; facts read right for
every kind.
  Check: re-run the capture → `/tmp/p07/pcb-after.png` matches
  `pcb-before.png` by eye; rename through the moved fields: (`"d"`, `"1"`)
  → `D1`, a duplicate → the red message (est. 5 min).
  Check: harness `facts`: print `PartFacts.rows` for the first catalog part
  of each of the 10 kinds and for `OSC-SIT1534-1508` → every row in §4.2's
  order with no empty value; the oscillator's Range and Default end in
  `Hz`; `PartFacts.hertz(32768)` → `32.768 kHz`, `hertz(110_000_000)` →
  `110 MHz`; an SRAM with 32768 words × 8 → Size `32768 × 8 (32 KB)`
  (est. 2 min).
  Check: §Prerequisites `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
  Result: `harness pcb <repo> after` → `cmp pcb-before.png pcb-after.png`
  exit 0 (byte-identical, stronger than by eye) and `cmp pcb-dup-before.png
  pcb-dup-after.png` exit 0; `rename d 1 -> reference D1: true`; `dup:
  fields ["U", "2", …] reference kept U2: true` with the red "D1 is already
  used by another part." in `pcb-dup-after.png`. `harness facts` → `kinds:
  10`, every row in §4.2 order (LTC-4727JR, SN74HCS72D, AT28C64B-15JU
  `Size: 8192 × 8 (8 KB)` `Write cycle: 10 ms`, 5962-8403701VCA `Flags:
  Military`, 5962-8515301CA, LED-RED-0402 `Color: Red`, OSC-SIT1534-2012,
  AS6C6264-55PCN, GND `Level: Low (GND)`, TE-GDS0204, OSC-SIT1534-1508
  `Range: 1 Hz – 32.768 kHz` `Default: 32.768 kHz`, 5V `Voltage: 5 V`);
  `no empty value: true`; `osc Range/Default end in Hz: true`;
  `hertz(32768) = 32.768 kHz true`; `hertz(110_000_000) = 110 MHz true`;
  `size(32768, 8) = 32768 × 8 (32 KB) true`. `xcodebuild` → `** BUILD
  SUCCEEDED **`.
Commit: c5df266

### Phase 2 — Schematic sidebar and placing

- [x] `Drawer/SchematicSidebarView.swift` (new): §4.3, plus the hidden
      drawer `.disabled` (Corrections 4) and `isActive` (Corrections 5).
- [x] `ContentView.swift`: Schematic inspector uses it (§4.3), kept in the
      tree on the PCB page (Corrections 5).
- [x] `Canvas/SchematicModel.swift`: §4.4. — `grep -c 'selectedChipID =
      placed' …/SchematicModel.swift` → 0.

Acceptance: §Outcome.
  Check: harness `window-07` (plan-06-C's offscreen-window method), open
  `test.schematical`, Schematic page: nothing selected → `s-library.png`
  shows the library; type `74HC00` in the search field and open one
  group; select the LED → `s-led.png` shows header, Prefix `U` / Number,
  Details with Kind LED, Package, Footprint `LED-0603 — …`, Color; rename
  (`"D"`, `"1"`) → reference `D1`, and the PCB page's sidebar header shows
  `D1` (`s-pcb.png`); clear the selection → `s-library-back.png` shows the
  search text `74HC00` and the same group open; `placeChip` of
  `SN74HC00N` → `selectedChipID` nil and the library shown (each printed
  `true`) (est. 10 min: the only check that exercises the sidebar in a
  window).
  Check: `grep -c 'selectedChipID = placed' schematical/schematical/Canvas/SchematicModel.swift`
  → 0 (est. 1 s).
  Check: §Prerequisites `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
  Result: `harness window-07 <repo>` → `nothing selected: true library
  fields: [""] panel shown: false` (`s-library.png`); `search 74HC00 rows 2;
  NAND opened -> rows 14: true` (`s-library-open.png`); `LED selected; panel
  shown: true fields: ["74HC00", "U", "3"] search disabled: true`, `shows
  LED-0603 footprint: true`, `s-led.png` shows U3 / LED-AMBER-0603 / Amber
  LED, fields U 3, Details Kind LED, Function LED, Package 0603, Footprint
  `LED-0603 — 0603 chip LED (1.5 x 0.75 mm)`, Pins 2, Color Amber; `rename D
  1 -> reference D1: true`; `pcb sidebar shows D1: true` (`s-pcb.png`
  header D1, fields D 1); `library back: panel shown: false search text
  74HC00: true rows 14 same group open: true` (`s-library-back.png`: NAND
  open, 12 parts); `placeChip SN74HC00N: placed true selectedChipID nil:
  true library shown: true`. `grep -c 'selectedChipID = placed'` → 0.
  `xcodebuild` → `** BUILD SUCCEEDED **`. Phase 1's PCB check re-run after
  Corrections 5: `cmp pcb-before.png pcb-after2.png` exit 0, `cmp
  pcb-dup-before.png pcb-dup-after2.png` exit 0.
Commit: 9da03b3

## Validation Plan

- Tests: no Swift test target; harness modes `facts`, `window-07` and the
  PCB capture.
- Coverage check: `facts` drives every `Details` case and every row
  formatter; `window-07` drives the sidebar switch, the shared reference
  fields and the drawer's kept state; the PCB capture covers the refactor.
- Runtime proof: the §Outcome sequence in `window-07`, PNGs in `/tmp/p07/`.
- Doc sync: none (no format or part-file change).
- Final gate (once, after Phase 2): `cmake -S backend -B /tmp/logic-p07 &&
  cmake --build /tmp/logic-p07 -j8 && ctest --test-dir /tmp/logic-p07`
  (est. 1 min; backend unchanged) + the `xcodebuild` (est. 1 min) + the
  harness modes (est. 15 min).
- Results (2026-11-01, at 9da03b3, build dir `/tmp/logic-p08` per
  Corrections 1): `ctest` → `100% tests passed, 0 tests failed out of 12`;
  `xcodebuild` → `** BUILD SUCCEEDED **`; `facts`, `window-07`, `pcb` as
  in the phases' Result lines. `main` had not moved (`git merge-base HEAD
  main` = `main` = a64fb17), so no merge-in or re-run; no Rust workspace in
  this repo, so no `cargo fmt`.

## Decisions

- The schematic sidebar shows the part library with nothing selected and
  the part (name, reference prefix/number fields as on the PCB, details)
  with a part selected — owner, 2026-11-01.
- Placing a part does not select it, so the library stays — owner,
  2026-11-01. Applies to the VCC/GND tools too (author: one rule for every
  placement).
- Details are the overview and the kind's facts only; no pin table,
  timing or connections — owner, 2026-11-01.
- Author's choices: the drawer stays alive under the panel so its state
  and scroll survive; supplies get the panel too (they have references);
  the header and reference fields are shared views used by both sidebars;
  the row labels and formats in §4.2.

## Corrections

1. **Scratch directory**: the plan was written as plan-07 and names
   `/tmp/p07/`, which plan-07's runs already use (`ls /tmp/p07` → its CMake
   build and harness). This plan's harness and PNGs live in `/tmp/p08/`
   (`/tmp/p08/harness/{build.sh,a.swift,window.swift}`; `window.swift` is
   plan-06's `HostedWindow` driver copied unchanged plus imports, which
   `MemberImportVisibility` needs per file). Mode names are kept.
2. **§4.2 predates plan-07's voltage facts**: at the plan's base (060c69a)
   `Details` had `dipSwitch(positions:actuator:)` and `supply(level:)`; now
   (`sed -n 124,135p schematical/schematical/Backend/PartCatalog.swift`)
   they carry `ratedMV` and `mv`, `DisplayInfo` has `vfMinMV`/`vfMaxMV`, and
   `CatalogPart.vcc` holds the supply range and input max. These are facts
   `CatalogPart` holds, so the panel shows them (formatted with
   `VoltageCheck.volts`, as the drawer does), each left out when zero:
   dipSwitch Rating "24 V max"; display Forward voltage "2–2.6 V"; supply
   Voltage "5 V" after Level (high only); and, last for any part with
   `vcc`, Supply range "2–6 V" and Input max "6 V". Also: the VCC part left
   the library in plan-07-G, so the "VCC/GND tools" are now the supply
   tools that place `5V`/`3V3`/… or `GND` (`SchematicCanvasView.swift:327`
   `case .vcc, .ground` → `tool.supplyName(voltage:)`); the `placeChip`
   callers are at lines 110 and 332, still 2.
3. **§4.1 `PartHeader` as one view**: three bare `Text`s in a `Form`
   `Section` would be three rows on the Schematic panel, so `PartHeader` is
   a `VStack(alignment: .leading, spacing: 4)` with `.textSelection`, and
   `FootprintPanel` nests it in its own same-spaced `VStack` with the
   footprint caption — the PCB capture is byte-identical (Phase 1 result).
4. **The hidden drawer could still take keyboard focus**: opacity 0 and
   `allowsHitTesting(false)` keep the search field in the AppKit tree
   (`window-07`: fields `["74HC00", "U", "3"]` with the panel shown), so Tab
   could land on an invisible field. The drawer is also `.disabled` while
   hidden; `disabled` keeps `@State`, so nothing resets (`search
   disabled: true`, and the text and group survive).
5. **§4.3 missed the page switch**: the inspector's `switch page` removes
   the Schematic sidebar while the PCB page shows, which resets the drawer.
   The §Outcome / Phase 2 check visits the PCB page between selecting and
   clearing, and the first `window-07` run measured it: `library back: …
   search text : false rows 22 same group open: false` (search empty,
   groups back to default). Fix: `ContentView` keeps `SchematicSidebarView`
   in the inspector on both pages (`isActive: page == .schematic`) with
   `PCBSidebarView` on top on the PCB page; inactive, the drawer is hidden
   and disabled and the part panel is not built (so it is rebuilt on return
   and its fields re-read the reference — no stale copy of the reference
   fields). After: `search text 74HC00: true rows 14 same group open:
   true`. The PCB captures stay byte-identical.
6. **Driving the window** (as plan-06-C Corrections 3): synthesized clicks
   don't reach the schematic canvas's `DragGesture` in the offscreen
   window (first run: `click on empty canvas -> selection cleared: false`),
   so `window-07` clears the selection as the canvas's empty-click does,
   `selectedChipID = nil` (canvas code unchanged by this plan). A plain
   mouse-down on the drawer's outline view hung the harness (event-tracking
   loop), so the NAND group is opened with `NSOutlineView.expandItem` on
   its row, as its chevron does. Grouped-Form labels expose no
   accessibility text here, so "panel shown" is measured as the sidebar
   having the Prefix/Number fields after the kept search field.

## Summary

A UI change over existing data: one shared refactor of the PCB panel
(checked by before/after captures), a pure facts formatter (checked for
every kind), and `placeChip` no longer selecting.
