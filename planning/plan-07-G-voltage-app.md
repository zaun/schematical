# plan-07-G: App: supply voltage dropdown, editor refusal, voltages shown, VCC → 5V

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-07-F, plan-06 (all sub-plans)

This sub-plan brings voltages into the app:

- The VCC toolbar tool gets a voltage dropdown: 5V, 3V3, 2V5, 1V8, 1V2,
  12V. A placed supply shows its voltage as its label.
- The editor refuses a wire, or a switch turned ON, that would put an
  out-of-range voltage on a part, and says why.
- A run that the backend refuses for voltage marks the bad net red.
- The parts drawer shows each part's voltage range.
- The library's old `VCC` part is removed. Files that use it open with
  `5V` in its place.

Outcome:

- In the headless harness (§ Validation), connecting a `5V` supply to
  `SN74AUC00RGYR` pin VCC is refused: no wire is added and the message
  names the pin and the 0.8–2.7 V range.
- `1V8` to the same pin is accepted.
- `test.schematical` (which uses `VCC`) opens, and its supply is `5V`.

References:

- plan-07-B §4.1 (`voltage.h`), plan-07-A § Decisions (owner rules)
- `docs/file-format.md` § Changing the format, § Example
- `schematical/schematical/ContentView.swift`, `Canvas/SchematicModel.swift`,
  `Canvas/SchematicCanvasView.swift`, `Simulation/Simulation.swift`,
  `Backend/PartCatalog.swift`, `Drawer/ChipDrawerView.swift`,
  `Document/SchematicFile.swift`
- `planning/completed/plan-06-A-board-model.md` (the headless harness
  `build.sh`)

## Prerequisites

The same as plan-07-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-07-F complete | `ls planning/plan-07-F-* 2>/dev/null` → no output | MET (2026-11-01: no matches; archived in 2996204) |
| plan-06 complete (it reworks the same toolbar) | `ls planning/plan-06-* 2>/dev/null` → no output | MET (2026-11-01: no matches; plan-06-A–D are in `planning/completed/`) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p07-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01, after plan-07-F: `** BUILD SUCCEEDED **`, 0 errors) |

If plan-06 is not complete, this sub-plan cannot start. Full stop. Line
numbers below are from before plan-06. Re-find each named symbol; the
symbols are what the tasks change.

## 1. Goal

- **Toolbar.** A voltage `Menu` sits right after the tool picker. Picking a
  voltage selects the VCC tool, and VCC placements use that supply. The
  choice persists (`@AppStorage("supplyVoltage")`, default `5V`).
- **Editor refusal.**
  - `SchematicDocument.connect` and `toggleSwitch` (turning a position ON)
    refuse an edit that adds a voltage problem. No wire, junction or toggle
    is made.
  - `document.editorMessage` shows the reason in an overlay, which clears
    after 4 s or on the next accepted edit.
- **Start refusal.** When `circuit_start` refuses for voltage, the sim
  error overlay shows the backend text, and the problem net's wires are
  drawn in the shorted style.
- **Drawer.** Row subtitles end with the voltage:
  - logic, memory and oscillator parts: `2–6 V`;
  - 7-seg: `VF 2.0–2.6 V`;
  - switches: `24 V max`;
  - LEDs and supplies: nothing.
- **Files.** A `partNumber` of `VCC` opens as `5V`. The file `version`
  stays 2.

### Non-goals (explicit constraints)

- No change to logic simulation or timing.
- An existing placed supply's voltage can't be changed. Delete it and place
  another one.
- A file that already has a voltage problem still opens. Only *new*
  problems are refused (§4.3), and the run is refused at start.
- No part inspector is added. The drawer subtitle is the only place
  voltages are shown.
- PCB page: unchanged.

## 2. Current State

- **Tools.** `SchematicTool` is in `Canvas/SchematicModel.swift:22-62`:
  - `supplyName` maps `.vcc` → `"VCC"` and `.ground` → `"GND"` (`:55-61`).
  - Placement looks up `tool.supplyName` in `SchematicCanvasView.swift:328-336`.
  - The tools are one segmented, icon-only `Picker` in
    `ToolbarItemGroup(placement: .principal)` (`ContentView.swift:172-184`).
  - The precedent for a menu is the speed `Menu` (`ContentView.swift:146-156`)
    with `@AppStorage` (`:43`).
- **Supply label.** `SupplySymbolView.swift:60-64` draws
  `Text(chip.part.function)`, so a `3V3` part labels itself. The body is
  40×20 (`SchematicModel.swift:147`).
- **Wires.** `connect(_:_:)` (`SchematicModel.swift:878-891`) is the only
  place a wire is created. It is called from `SchematicCanvasView.swift:392-395`.
  - Its anchors are `LineAnchor.pin(PinRef)` or `.point(CGPoint)` (`:526-529`).
  - `endpoint(for:)` → `junction(at:)` (`:894-937`) *mutates*: it splits the
    wire under a point.
  - Moving, flipping, bend-dragging and erasing never join nets (no paste,
    no undo exists).
- **Switches.** `toggleSwitch` (`SchematicModel.swift:781-786`) is called
  from `SchematicCanvasView.swift:419-423`, which then calls
  `simulation.setSwitch`.
- **Nets.** `NetlistBuilder.build(chips:wires:)` (`Simulation/Simulation.swift:32-113`)
  is a pure union-find. It returns `connections` as (part, designator, net)
  and `partNames`.
- **Messages.** No editor message exists. The sim error overlay is in
  `ContentView.swift:216-226`. A failed start sets `errorMessage` from
  `circuit_error` (`Simulation.swift:220-224`). `shortedNets`
  (`Simulation.swift:166,306,382`) turns a net red.
- **Catalog.** `CatalogPart` (`Backend/PartCatalog.swift:115-215`) is filled
  from the C `Part` (`:261-321`). `detailLabels` (`:203-214`) feeds the
  drawer subtitle (`Drawer/ChipDrawerView.swift:270-294`). The library
  pointer is `PartLibrary.pointer` (`PartCatalog.swift:9`).
- **Bridging header.** It includes `parts.h`, `circuit.h` and
  `memory_image.h`.
- **Files.** `SchematicFile.apply` (`Document/SchematicFile.swift:263-374`)
  looks up `partNumber`. An unknown name fails the whole open with
  `.unknownParts` (`:273-276,305-307`).
- **Backend tests.** They reference the library part `"VCC"` 18 times:
  - `circuit_check.c`: 8 (lines 51, 69, 87, 161, 207, 311, 343, 358)
  - `sim_bench.c`: 1 (124)
  - `display7_test.c`: 2 (57, 141)
  - `library_check.c`: 4 (128, 135, 186, 964)
  - `switch_test.c`: 3 (112, 152, 175)

  `engine_test.c:121`, `display7_test.c:55` and `eeprom_test.c:56` use
  `"VCC"` as a net or pin name, which is not the part.

### Measured populations

| What | Count | Command |
|---|---|---|
| backend test refs to the `VCC` part | 18 | `grep -n '"VCC"' backend/tests/*.c` → 21 lines, minus the 3 net/pin-name lines listed above |
| places that create a wire | 1 | read `SchematicModel.swift`: only `connect` appends to `wires` (`:890`) |
| places that close a contact | 1 | `toggleSwitch` (`:781`) |
| Swift test targets | 0 | `project.pbxproj` has one target, `schematical` |

## 3. Design Overview

The work splits into five pieces:

1. **Library rename.** Remove the `VCC` device from `supply.json`, point the
   backend tests at `5V`, and regenerate the goldens (one line fewer in
   each).
2. **`VoltageCheck.swift`.** A thin wrapper over `voltage_check` that takes
   the document's chips, wires and switch states and returns problems with
   chip IDs.
3. **Editor refusal.** It goes in `connect` and `toggleSwitch`.
4. **UI.** The menu, the message overlay, red nets, and drawer labels.
5. **File alias.**

**Correctness risk** is in two places:

- **`connect`'s refusal must not mutate.** `endpoint(for:)` splits wires.
  The check therefore runs on a candidate wire list built with a
  non-mutating lookup (§4.3), before any mutation.
- **The "new problems only" comparison.** It is covered by harness case 4.

**Gate class:** behavior change in the app.

- `parts.txt` and `traces.txt` each lose exactly the `VCC` line.
- Every other golden line stays the same.

**Rejected alternatives**

- *Taking the VCC segment out of the Picker as a split button.* The
  selected-tool highlight would be lost. The menu next to the Picker keeps
  it.
- *Keeping `VCC` in the library as a hidden alias part.* There would be two
  5 V parts, and the dropdown would have no clear owner of "5V".
- *Refusing every problem, old ones included.* An old file with a problem
  could not be edited at all.
- *Bumping the file version.* `partNumber` keeps its meaning (a library part
  name). Only one removed name is mapped, the same way an unknown part would
  be handled.

## 4. Detailed Design

### 4.1 Library and tests

- `supply.json`: delete the `VCC` device.
- In the 18 test references above, replace `"VCC"` with `"5V"`. In
  `library_check.c:135`, the function check becomes `"5V"`.
- Regenerate both goldens.

### 4.2 `Simulation/VoltageCheck.swift` (new)

The bridging header adds `#include "voltage.h"`.

```swift
struct VoltageIssue: Hashable { let kind: VoltageProblemKind.RawValue; let chipID: UUID; let pin: Int
                                let otherChipID: UUID; let net: Int; let mv: UInt32; let minMV: UInt32; let maxMV: UInt32 }
enum VoltageCheck {
    /// Problems in this drawing; `closed` = (chip, 1-based position) pairs that are ON.
    static func issues(chips: [PlacedChip], wires: [Wire], library: PartLibrary) -> Set<VoltageIssue>
    static func message(_ issue: VoltageIssue, chips: [PlacedChip]) -> String
}
```

- `issues` runs `NetlistBuilder.build`. It maps each chip to
  `part_library_find(library.pointer, chip.partNumber)` and each
  designator to its pin index. Closed contacts come from `switchesOn`.
  Then it calls `voltage_check` with `cap` = the returned count (call once
  to count, once to fill).
- `message` gives:
  - `Can't connect: 5 V on U3 pin 14 (VCC). SN74AUC00RGYR needs 0.8–2.7 V.`
  - `Can't connect: U3 pin 1 (1A) gets 5 V; SN74AUC00RGYR inputs take at most 3.6 V.`
  - `Can't connect: 3.3 V (U1) and 5 V (U4) supplies on one net.`
  - `Can't connect: U7 pin 2 gets 12 V; TE-GDS0204 is rated 24 V.`

  For switches the verb is "Can't turn on" instead. Volts are formatted
  with up to 2 decimals and no trailing zeros.

### 4.3 Refusal (`Canvas/SchematicModel.swift`)

- Split a pure `wire(at point:) -> Wire?` out of `junction(at:)`: the same
  hit-test, with no split. `junction(at:)` calls it.
- `connect(_:_:) -> String?` returns nil when it made the wire, or the
  message when it refused. Before any mutation:
  - Resolve each anchor to a stand-in `WireEnd`:
    - `.pin(ref)` → `.pin(ref)`;
    - `.point(p)` on a wire → that wire's `start`;
    - `.point(p)` on nothing → a fresh `.junction(UUID())`.
  - Build `candidate = wires + [Wire(start: a, end: b, interior: [])]`.
  - `new = issues(candidate) − issues(wires)`.
  - If `new` is not empty, set `editorMessage` to the message of `new`'s
    first issue (sorted by net, chip, pin) and return it.
  - Otherwise clear `editorMessage` and proceed as today.
- `toggleSwitch`: when the position would turn ON, run the same comparison
  with the position on. On a refusal it sets `editorMessage` and returns
  nil, the existing "no change" result. `SchematicCanvasView.swift:422`
  already returns early on nil, so `simulation.setSwitch` is never called
  for a refused toggle.
- `SchematicDocument` gains `var editorMessage: String?` and `library`
  access. The document already has the catalog through
  `placeChip(part:)` callers. Pass `PartLibrary` in at `init` from the
  catalog store, which `ContentView` owns.

### 4.4 UI

- **`ContentView`:**
  - `@AppStorage("supplyVoltage") var supplyVoltage = "5V"`.
  - Right after the tool `Picker` in the same group:
    `Menu(supplyVoltage) { ForEach(SupplyChoice.names, id: \.self) { n in Button(n) { supplyVoltage = n; tool = .vcc } } }`,
    with `.help("Supply voltage for the VCC tool")` and
    `.disabled(simulation.isRunning)`.
  - An editor-message overlay copies the sim-error capsule (`:216-226`)
    with an orange background. A `task(id: document.editorMessage)` clears
    it after 4 s.
- **`SchematicModel.swift`:**
  - `enum SupplyChoice { static let names = ["5V","3V3","2V5","1V8","1V2","12V"] }`.
  - `supplyName` takes the chosen voltage:
    `func supplyName(voltage: String) -> String?` gives `.vcc` → `voltage`
    and `.ground` → `"GND"`.
- **`SchematicCanvasView`:** a `supplyVoltage: String` property, passed from
  `ContentView`, is used at `:328`.
- **`Simulation.swift`:** when `build` fails, call
  `circuit_voltage_problem`. On 1, set
  `shortedNets = [Int(problem.net)]`, so the net draws red. The start
  error text is unchanged.
- **`PartCatalog.swift`:**
  - `CatalogPart` gains `let vcc: (minMV: UInt32, maxMV: UInt32, inputMaxMV: UInt32)?`,
    from `part.vcc`.
  - `.display` details gain `vfMinMV`/`vfMaxMV`, `.dipSwitch` gains
    `ratedMV`, and `.supply` gains `mv`.
  - `voltageLabel: String?` uses §1's formats.
  - `ChipDrawerView.subtitle(for:)` appends `voltageLabel`.

### 4.5 File alias (`Document/SchematicFile.swift`)

- In `apply`, before the catalog lookup:
  `let name = Self.renamedParts[part.partNumber] ?? part.partNumber`, with
  `static let renamedParts = ["VCC": "5V"]`.
- Saving writes the new name.
- `docs/file-format.md`:
  - The `partNumber` row lists the supplies as `5V`, `3V3`, `2V5`, `1V8`,
    `1V2`, `12V` and `GND`.
  - § Changing the format gains: "A part removed from the library may map
    to its replacement on open (`VCC` → `5V`)".
  - The § Example uses `5V`.

## Compatibility / Format Impact

- **Schematic files.** Version 2 is unchanged. Old files with `VCC` open as
  `5V`. Files that use the new supplies fail in older builds with "parts not
  in the catalog", which is the existing rule for new parts.
- **Part library.** The `VCC` part is gone. No external reader exists.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick boxes in the same
> commit as the work. Use `- [~]` for partial work. Mark moot tasks with
> `- [x] ~~text~~ — moot: <evidence>`. Fill in `Commit:`.
> **An unticked box means NOT DONE.**

### Phase 1 — `VCC` → `5V` in library, tests and files

- [x] §4.1, and §4.5, with the doc edits.

Acceptance:

- The backend passes, and each golden loses exactly one line.
- The app opens an old file and reads its supply as `5V`.

  Check: `ctest --test-dir /tmp/p07` → 12/12.
  `git diff -U0 backend/tests/golden | grep -c '^[-+][^-+]'` → 2, the two
  `-VCC|` lines. Then build the app and run harness case 1 (§ Validation)
  (est. 5 min).
  Result (2026-11-01): `ctest` → `100% tests passed, 0 tests failed out of
  12`; golden diff → `2`: `-VCC|supply|||VCC|||level=1 mv=5000|…` and
  `-VCC	1df7bc25f3ed7621`. `xcodebuild … build` → `** BUILD SUCCEEDED **`;
  harness `1` → `case 1 ok (supplies: 5V, GND, GND)` (and the re-encoded
  file names `5V`, not `VCC`). The remaining `"VCC"` strings in
  `backend/tests/*.c` are the three net/pin names (`display7_test.c:55`,
  `eeprom_test.c:56`, `engine_test.c:121`).
Commit: 4790eb5

### Phase 2 — Voltage check in the editor

- [x] Bridging header, `VoltageCheck.swift` (§4.2).
- [x] `wire(at:)`, `connect` and `toggleSwitch` refusal, `editorMessage`
      (§4.3). Update the caller at `SchematicCanvasView.swift:392-395`.
- [x] Added task: `CatalogPart.Details.supply` gains `mv` here, not in
      Phase 3 — the clash message names the lower supply first, which needs
      each supply's voltage.

Acceptance: harness cases 2–5 pass.
  Check: `MAIN=/tmp/p07/harness/main.swift sh /tmp/p07/harness/build.sh $PWD /tmp/p07/out /tmp/p07-dd/Build/Products/Debug/schematical.app && /tmp/p07/out/H.app/Contents/MacOS/harness`
  → `cases 2-5 ok` (est. 5 min, including the Debug build the harness
  copies).
  Result (2026-11-01): `xcodebuild … build` → `** BUILD SUCCEEDED **`;
  `MAIN=/tmp/p07/harness/h.swift sh …/build.sh … && …/harness 2-5` →
  `case 2 ok: Can't connect: 5 V on U2 pin 14 (VCC). SN74AUC00RGYR needs
  0.8–2.7 V. (also refused on a point of a wire, with no split)`, `case 3
  ok`, `case 4 ok`, `case 5 ok: Can't turn on: 1.8 V (U2) and 5 V (U1)
  supplies on one net.`, `cases 2-5 ok`. Case 2 also connects 5V to a grid
  point on an existing wire of the AUC VCC net: refused with the same wires
  and no junction (the split `endpoint(for:)` would make never happens).
Commit: —

### Phase 3 — Toolbar, overlay, red net, drawer labels

- [ ] §4.4.

Acceptance: harness case 6 passes, and the app shows the menu.
  Check: the harness → `case 6 ok`. Launch the Debug app, take a screenshot
  of the toolbar with the menu open, and a screenshot of a refused wire with
  its overlay. Save both under `/tmp/p07/shots/` and write their paths on
  this phase's `Commit:` line (est. 5 min).
Commit: —

## Validation Plan

- **Harness.** `/tmp/p07/harness/build.sh` copies plan-06-A's `build.sh`
  verbatim. `main.swift` loads `PartCatalogStore` and drives
  `SchematicDocument` / `SchematicFile` / `SimulationController` directly.
  It prints `case N ok` or exits 1 with the failing case. The cases:
  1. Open `test.schematical`. Its `VCC` chip now has `partNumber == "5V"`.
  2. Place `5V` and `SN74AUC00RGYR`. `connect(5V pin 1, AUC VCC pin)`
     returns a message containing `0.8–2.7 V`. `wires.count` is unchanged,
     with no new junction.
  3. Place `1V8`. Connecting it to the AUC VCC pin returns nil, and
     `wires.count` is +1.
  4. Load a document that already has 5V on an AUC VCC pin. Connecting an
     unrelated `GND` to an unrelated LED pin returns nil (an old problem is
     not refused).
  5. A switch with `5V` on one contact and the AUC VCC net (on `1V8`) on the
     other. `toggleSwitch` ON returns nil and sets `editorMessage`
     (CLASH, 1.8 V and 5 V).
  6. For the document in case 4, `SimulationController.start` fails,
     `errorMessage` starts with `voltage:`, and the problem net is in the
     shorted style.

  Plus: `CatalogPart(SN74HC00D).voltageLabel == "2–6 V"`,
  `LTC-4727JR` → `"VF 2–2.6 V"`, and `TE-GDS0204` → `"24 V max"`.
- **Coverage check.** The harness compiles every app Swift file except
  `schematicalApp.swift`, so `connect`, `toggleSwitch` and `VoltageCheck`
  are the shipped code.
- **Runtime proof.** The two screenshots in Phase 3.
- **Doc sync.** `docs/file-format.md` (Phase 1). `docs/backend.md` already
  covers `voltage.h` (B).
- **Final gate (once):**
  - `ctest --test-dir /tmp/p07` → 12/12.
  - The `xcodebuild … build` → `** BUILD SUCCEEDED **`.
  - The harness → `cases 1-6 ok`.

  Est. 6 min.

## Decisions

See plan-07-A § Decisions. Owner decisions that apply here:

- A dropdown on the VCC toolbar button.
- The editor refuses an out-of-range wire.
- `VCC` opens as `5V`.
- G waits for plan-06.

Author decisions:

- The menu sits next to the Picker, not in place of the VCC segment.
- Only new problems are refused.
- No file version bump. Instead there is a `renamedParts` alias.
- A refused toggle never reaches the running sim. The backend's switch stop
  (B) is the second guard.

## Corrections

- **The backend test references to the `VCC` part were not the plan's 18
  at the plan's lines when G started.** plan-07-F had already moved two of
  them: `circuit_check.c`'s oscillator test powers a 1.8 V LMK6 part and
  uses `1V8`, and `sim_bench.c` runs on `3V3` (plan-07-F Corrections). The
  other 16 became `"5V"`: `circuit_check.c` 7, `display7_test.c` 2,
  `switch_test.c` 3, `library_check.c` 4. `voltage_test.c` (new in B) also
  named `VCC` once; it now uses two `5V` parts.
- **`library_check.c`'s four references are to its own inline `VCC_DEVICE`
  fixture**, not the library's part. The fixture is renamed to `5V` (id,
  function, part name) so they match, and the two duplicate-name cases that
  build on it (`duplicate part in one file`, `check_duplicate_across_calls`)
  and the duplicate-id case now name `5V`.
- **Added task: the VCC tool kept working between phases.** Removing the
  `VCC` part in Phase 1 left `SchematicTool.supplyName` pointing at a part
  that no longer exists, so Phase 1 points `.vcc` at `5V` until Phase 3's
  voltage menu.
- **The harness entry file is `h.swift`, not `main.swift`**: swiftc treats
  a file named `main.swift` as top-level code, which can't hold the
  `@main` type the harness uses (plan-06's was `a.swift`). The Phase 2
  check's `MAIN=…/main.swift` is `MAIN=/tmp/p07/harness/h.swift`.

## Summary

The risk is in two places. `connect` must not mutate before the check,
which the stand-in ends and the pure `wire(at:)` handle. The new-only
comparison is covered by case 4. The logic sim, the PCB page and the file
version are untouched.
