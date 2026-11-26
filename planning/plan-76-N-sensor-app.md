# plan-76-N: App: sensor details, facts, kind names, drawer, Part Editor, tests, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-M

N makes the app show the sensor parts: their facts, kind names, the drawer's new **Sensors**
category, and the Part Editor kind. A sensor already turns Run off, since `"simulated": false` is
handled; N proves it on a real schematic.

References:

- plan-76-L § 3.1–3.2 (`PartSensor`, units ×1000).
- `docs/backend.md` § Adding a kind, steps 5–6 (the Part Editor's lists, `part-editor-schema.py`,
  `PartKindRoundTripTests`, `PartKindKeysTests`, `KindGuideTests`, `variantBlurbs`).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details` enum line 680, the builder chain's
  opamp branch line 1710, `symbol(for:)` line 2028, `detailLabels` line 1250); `Backend/PartFacts.swift`
  (`kindName(_ kind:)`, line 427 `case PART_OPAMP: return "Op-amp"`; `kindName(_ part:)`; fact
  rows); `Preferences/ReferencePrefixes.swift:75` (`ReferenceCategory.of`);
  `Editor/KindInfo.swift` (`KindGroup` line 5, `infoOrder` line 214);
  `Editor/KindSchema.swift` (`PartKinds.all` line 202, `schemas` line 218);
  `Editor/BundledPartFile.swift:136`; `Editor/Wizard/Guides/VariantBlurbs.swift:78`;
  `scripts/part-editor-schema.py:44` (`KIND_FILES`).
- Test classes in `schematical/schematicalTests/`: `PartKindKeysTests` (`files` at :47),
  `PartKindRoundTripTests` (shipped counts at :31), `KindInfoTests` (count at :11, groups at
  :53), `PartEditorWindowTests` (`enabledKinds.count` at :14), `KindGuideTests` (variant count at
  :191), `WizardStepsTests`, `DrawerPerformanceTests`.
- Precedent: `planning/plan-74-J-ethernet-app.md` (a facts-only kind's app letter);
  `scripts/harness/` (`build.sh` and one `<kind>-app.swift` per kind).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-M complete | `ls planning/completed/plan-76-M-*.md` → exists, or every M box ticked | NOT MET (2026-11-25: M not started) |
| The app does not know the kind yet | `grep -c 'PART_SENSOR' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift schematical/schematical/Editor/BundledPartFile.swift` → 0, 0, 0 | MET (2026-11-25) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p76-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | run at N's start (plans 69–75 change the app first) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

On the app's real catalog (`scripts/harness/sensor-app.swift`, every line `ok`):

- The drawer has **Sensors → Temperature, Humidity, Pressure, Motion, Magnetic, Light, Current**.
- The most-stocked temperature `add` part with an I2C output (the harness reads it from
  `planning/plan-76-data/candidates.tsv` and `specs-temperature.tsv`): reference `U1`, kind name "Temperature sensor", a block
  symbol with its pins; facts rows `Measures` Temperature, `Output` I2C, `Range` "−55 … 150 °C"
  (its own values), `Accuracy` "±0.1 °C" when given, `I2C address` "0x48, 0x49, …" when given,
  `Supply` its range, and `Play mode` `Not simulated`. Run is off
  (`SimulationAvailability.canStart` false, with the existing reason).
- A pressure part: `Pressure type` row; a magnetic switch: `Switch` row ("Omnipolar" …); a
  current part with `isolated`: `Isolated` Yes; a motion part: `Axes` 3.
- File › New Part… offers the Sensor kind: `PartKindRoundTripTests` round-trips every shipped
  `sensor` part with 0 mismatched.

### Non-goals (explicit constraints)

- No new symbol: a sensor is a block, like an Ethernet IC.
- No new reference category: sensors are `U` (`.integratedCircuit`, the default of
  `ReferenceCategory.of`).
- No change to `SimulationAvailability`.
- No unit conversion setting: ranges show in the kind's fixed units.

## 2. Current State

- `CatalogPart.Details` has a case per facts shape; an unknown kind falls to the generic block
  details with no facts rows.
- `kindName(_ kind:)` has no `PART_SENSOR` case and returns "" for an unnamed kind.
- `BundledPartFile.name(of:)` names a part file per kind (`PART_OPAMP: "opamp"`, line 136).
- `KindGroup` has nine groups (`Editor/KindInfo.swift:5-15`); none is for sensors.

## 3. Design

### 3.1 Catalog

```swift
struct SensorQuantityInfo: Hashable, Sendable { let quantity: String; let range: ClosedRange<Double>?; let accuracy: Double? }
struct SensorInfo: Hashable, Sendable {
    let family: String; let quantities: [SensorQuantityInfo]; let outputs: [String]
    let axes: Int?; let pressureType: String?; let switchType: String?; let isolated: Bool; let addresses: [UInt8]
}
```

- `Details.sensor(SensorInfo)`, built from `part.sensor?.pointee` (values ÷ 1000; outputs in
  `SENSOR_OUT_*` bit order, quantity names from the `SENSOR_*` order).
- `symbol(for:)`: `.sensor` → `.block`.
- `detailLabels`: the family and the outputs, e.g. `["Temperature", "I2C"]`.

### 3.2 Facts and names

- `kindName(_ kind:)`: `PART_SENSOR` → "Sensor".
- `kindName(_ part:)`: "<Family> sensor" ("Temperature sensor", …, "Current sensor").
- Rows: `Measures` (capitalized names, comma list), `Output` (I2C, SPI, I3C, 1-Wire, Analog,
  Bridge, Capacitance, Resistance, PWM, SENT, UART, Frequency, Switch), `Range` one row per
  quantity with a range ("−40 … 125 °C", "0 … 100 %RH", "0 … 206.843 kPa", "±16 g", "±2000 °/s",
  "±50 mT", "0 … 120000 lux", "±30 A"; symmetric ranges as ±), `Accuracy` per quantity, `Axes`,
  `Pressure type`, `Switch`, `Isolated` (only when set), `I2C address` (hex, comma list),
  `Supply`, and `Play mode` `Not simulated`.

### 3.3 Part Editor

- `KindGroup`: a new case `sensors = "Sensors"` after `displays`; `KindInfoTests` groups updated.
- `KindInfo` entry "Sensor": blurb "A temperature, humidity, pressure, motion, magnetic, light or
  current sensor (facts only, not simulated)", examples the most-stocked shipped part of the
  temperature, pressure and magnetic families (`KindInfoTests` checks each exists),
  group `.sensors`, `simulated: false`. `PartKinds.all` and `infoOrder` get `"sensor"`.
- `BundledPartFile.name(of:)`: `PART_SENSOR: "sensor"`.
- `scripts/part-editor-schema.py`: `KIND_FILES["sensor"] = ["sensor.c"]`, and the same in
  `PartKindKeysTests.files`. `python3 scripts/part-editor-schema.py . sensor` writes
  `Editor/SensorDraft.swift` (it must print `missing []`). `family` is the `variantKey`, so
  `VariantBlurbs.swift` gets `"sensor"` with one blurb per family (seven), and
  `KindGuideTests`' variant count grows by 7.
- `PartKindRoundTripTests`: `testSensor` and the shipped count.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Catalog, facts, names

- [ ] `Backend/PartCatalog.swift` per § 3.1; `Backend/PartFacts.swift` per § 3.2 (each case only if
      absent; plans 69–75 add kinds to the same switches).
- [ ] `schematicalTests/SensorTests.swift`: kind names, facts rows and detail labels of one part
      per family; `ReferenceCategory.of` → `.integratedCircuit`; `symbol(for:)` → `.block`;
      `SimulationAvailability` refuses a document with a placed temperature part.

Acceptance: the tests pass.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p76-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/SensorTests` → `** TEST SUCCEEDED **` (est. 6 min: one build and one test class).
  Result:
Commit:

### Phase 2 — Part Editor

- [ ] § 3.3: `KindGroup`, `KindInfo`, `PartKinds`, `BundledPartFile`, `part-editor-schema.py`,
      `SensorDraft.swift`, `VariantBlurbs.swift`.
- [ ] Update `PartKindKeysTests`, `PartKindRoundTripTests`, `KindInfoTests`,
      `PartEditorWindowTests`, `KindGuideTests`, `WizardStepsTests` for the new kind (read each
      count from the test at this step: plans 69–75 change the same counts).

Acceptance: the editor tests pass with the new kind.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p76-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/WizardStepsTests` → `** TEST SUCCEEDED **`, round trip 0 mismatched (est. 8 min).
  Result:
Commit:

### Phase 3 — Harness on the real catalog

- [ ] `scripts/harness/sensor-app.swift` per § 1 (drawer paths, one part per family, Run off);
      build with `scripts/harness/build.sh scripts/harness/sensor-app.swift /tmp/p76-harness`.
- [ ] `DrawerPerformanceTests` with the larger catalog.

Acceptance: every harness line is `ok`, and the drawer stays within its time budget.
  Check: `scripts/harness/build.sh scripts/harness/sensor-app.swift /tmp/p76-harness && /tmp/p76-harness/H.app/Contents/MacOS/harness` → every line `ok`; `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p76-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **` (est. 8 min).
  Result:
Commit:

## Validation Plan

- N's checks are its three acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **a new `KindGroup` "Sensors"**, matching the new drawer category; folding sensors into
  "Other" would hide 3,000 parts under a catch-all.
- Author: **ranges shown in the kind's units**, symmetric ones as ±, so the facts read as the
  datasheets' headline figures.
- Author: **no symbol**: sensor pinouts vary from 2 to 30+ pins; a block shows them all.

## Corrections

## Summary

N shows the sensor facts, adds the Sensors drawer category and Part Editor kind, and proves Run is
off with a sensor placed.
