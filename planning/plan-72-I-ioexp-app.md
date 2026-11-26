# plan-72-I: App — drawer, symbol, details, register window, Part Editor, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-72-H

Part of plan-72 (see plan-72-A for the goal, prerequisites and decisions).
I makes the two kinds first-class in the app: the drawer lists them, the
canvas draws them, the sidebar shows their facts, the register window shows
an expander's or a mux's registers in play mode (as it does a converter's),
File › New Part… and Copy to Editor… work for both kinds, and a harness
proves the drawer-to-run path on the real app.

References:

- `docs/backend.md` § Adding a kind steps 5–6 (symbol; Part Editor:
  `KindInfo`, `PartKinds.all` / `infoOrder`, `BundledPartFile.name(of:)`,
  `PartFacts`, `scripts/part-editor-schema.py` `KIND_FILES`,
  `PartKindKeysTests.files` / `notCalled`, the hand-written draft for a kind
  with an inline-checked section, `PartKinds.schemas`,
  `PartKindRoundTripTests`, `KindGuideTests`, `WizardStepsTests`).
- The `leddriver` precedent: `Editor/LeddriverDraft.swift` (hand-written,
  `display` a required `.json` field), `Backend/PartCatalog.swift`,
  `Backend/PartFacts.swift`, `Editor/Wizard/FieldConditions.swift`,
  `schematicalTests/DisplayDriverTests.swift` (file names as before plan-68;
  after plan-68-V's split, grep `PART_LEDDRIVER` for the current files).
- `schematical/schematical/Simulation/SimulationEngine.swift`
  (`circuit_converter_registers`, the register window), `schematicalTests/RegisterTests.swift`.
- `scripts/harness/build.sh`, `scripts/harness/display-drivers-app.swift`
  (the harness this one copies).
- Memory of past letters: a new part key needs its Draft, the wizard Guide and
  three test classes (plan-67).

## Prerequisites

plan-72-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-72-H complete | `grep -c '^- \[ \]' planning/plan-72-H-i2cbus-parts.md` → 0 | NOT MET (2026-11-25) |
| The app builds at I's start | the A Prerequisites `xcodebuild … build` → `** BUILD SUCCEEDED **` | recorded at I's start |
| Last kind in the app's lists | `grep -n 'PART_' schematical/schematical/Backend/PartCatalog.swift \| tail -3` | recorded at I's start; plans 69–71, 73, 74 may have added theirs: I appends after whatever is last |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- Drawer: **Interface → I/O Expanders → I2C / SPI** (by bus; an `i2c-spi`
  part under both) and **Interface → I2C Bus → Buffers / Muxes & Switches**
  (`type` `switch`/`mux` → Muxes & Switches, the rest → Buffers); the drawer
  table in `docs/parts-format.md` § The drawer's categories gains both rows.
- Symbol: the generic IC block with pins grouped by `group` (ports, address,
  bus, interrupt), as `leddriver`.
- Sidebar details: Kind "I/O expander" (ports, bus, I2C address from the
  placed straps, interrupt pins) / "I2C bus" (type, channels, domains).
- Register window: an expander's and a mux's registers live in play mode.
- Part Editor: `KindInfo` entries, `IoexpanderDraft.swift` and
  `I2cbusDraft.swift` (hand-written, `io` a required `.json` field, the
  `LeddriverDraft.swift` shape), schemas, guides, `PartKindRoundTripTests`
  counts (0 mismatched, 0 with extras, 0 rejected).
- `schematicalTests/IoExpanderTests.swift`: drawer paths, details strings,
  the register window's reset values for TCA9555 and TCA9548A.
- Harness `scripts/harness/ioexp-app.swift`: on the real app's catalog and
  `SimulationController`, (1) a PCA9515B with EN tied to 3.3 V, a pulled-up
  slide switch on SDA0 and an LED with its resistor on SDA1 (pulled up):
  closing the switch darkens the LED's net level to 0 within one run, opening
  it restores 1; (2) a placed TCA9555 and TCA9548A run, and the register
  window lists their reset values (Configuration 0xFF, control 0x00); (3) an
  LTC4316 in the document turns Run off with the unsimulated-part message;
  renders the placed parts to `/tmp/p72-symbols.png`.

### Non-goals

- No new canvas mechanics; no I2C decoder view; no analog rendering of
  pull-ups. A host program (an AVR sketch) driving an expander in the app is
  not part of this plan: the toolchain to build one is not installed
  (`which avr-gcc avra` → not found, 2026-11-25) and the backend tests cover
  the protocol (plan-72-D, -E, -F, -G, -H).

## Phases

> **NOTE — keep the checkboxes current as you go.**

### Phase 1 — catalog, facts, drawer, symbol

- [ ] Grep first, then add the two kinds to `PartCatalog.swift` (kind names, drawer paths), `PartFacts.swift` (facts of plan-72-D § 4.1 and -E § 4.3), the symbol choice.
- [ ] `IoExpanderTests` drawer and details cases.

Acceptance: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p72-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/IoExpanderTests -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit:

### Phase 2 — Part Editor

- [ ] `KindInfo`, `PartKinds.all` / `infoOrder`, `BundledPartFile.name(of:)`, the two drafts, `PartKinds.schemas`, `KIND_FILES`, `PartKindKeysTests.files` / `notCalled`, guides, `PartKindRoundTripTests` counts.

Acceptance: `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/KindInfoTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit:

### Phase 3 — register window and harness

- [ ] Register window for both kinds (the converter path; a test in `IoExpanderTests`).
- [ ] `scripts/harness/ioexp-app.swift` per § 1.

Acceptance: `rm -rf /tmp/p72-harness && APP=/tmp/p72-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/ioexp-app.swift /tmp/p72-harness && /tmp/p72-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; `xcodebuild … test -only-testing:schematicalTests/IoExpanderTests -only-testing:schematicalTests/RegisterTests` → `** TEST SUCCEEDED **` (est. 15 min).
Commit:

## Validation Plan

- Tests: `IoExpanderTests` (new), `RegisterTests`, the Part Editor classes, `DrawerPerformanceTests`; the harness.
- Coverage: `node scripts/coverage.js --app --check` exit 0 (plan-68's rule).
- Doc sync: `docs/parts-format.md` § The drawer's categories; `docs/backend.md` § Adding a kind needs nothing new.
- Final gate (once): the three acceptance commands above in one run (est. 30 min).

## Decisions

- Drawer paths and kind display names as § 1 — the author's call, following `leddriver` (Interface → Driver).
- No in-app host program — the author's call (§ Non-goals: no AVR toolchain on the machine).

## Corrections

(none yet)
