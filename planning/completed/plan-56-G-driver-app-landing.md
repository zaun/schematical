# plan-56-G: App for `leddriver` and `lcddriver`, harness proof and the landing tier

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-F

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

The app lists, places, draws, edits and simulates the two new kinds: block symbols with the bus and display pins grouped, the drawer groups Interface › Driver › LED / LCD and Displays & Indicators › LCD glass, sidebar facts, Part Editor (File › New Part…), the harness proof, docs and the final gate.

## Phases

### Phase 1 — app
- [x] `PartCatalog.swift`, `PartFacts.swift`, `SchematicModel.swift`, `ReferenceLabel.swift`, `PartLook.swift`, `KindInfo.swift`, `KindSchema.swift`, `<Kind>Draft.swift` for both kinds (the `docs/backend.md` § Adding a kind list) and their Swift tests (`PartKindKeysTests`, `PartKindRoundTripTests`, `KindGuideTests`, `WizardStepsTests`, `DrawerCategoriesTests`).
Acceptance: `xcodebuild test` of those suites → TEST SUCCEEDED (all but `KindGuideTests`, whose failures are the `crystal`, `powerswitch` and `decoder` ones of other work; see Corrections). Commit: 5f696fcf

### Phase 2 — harness proof
- [x] A `scripts/harness/display-drivers-app.swift` script (the benches in backend/tests drive each device through its real bus; the app proof needs no bus stimulus part: the PCF1179C is bus-less, and the ICM7228C takes its write from a tactile switch on WRITE with a pull-up): (1) PCF1179C + GENERIC-LCD-CLOCK-1:2 in the real SimulationController; (2) ICM7228CIPIZ + LTC-5623G, digit 1 = 5 written by pressing and releasing the switch. `scripts/harness/build.sh scripts/harness/display-drivers-app.swift /tmp/p56-h && /tmp/p56-h/H.app/Contents/MacOS/harness /tmp/p56-out` -> `lit cells: 2B 2C 3A 3B 3C 3D 3E 3F 4A 4B 4C 4D 4E 4F 0L1 0L2 0AM`, `lit segments: 1A 1C 1D 1F 1G ...`, 13 PASS lines, `display-drivers-app: all checks passed`.
Acceptance: the script prints the digit lit (above). Commit: 1e2fa815

### Phase 3 — landing tier (once)
- [x] Landing tier (2026-11-24, in the worktree, after merging the SN74143 branch; main had not moved): `ctest --test-dir /tmp/logic-build-p56 -j8` -> `100% tests passed, 0 tests failed out of 60`; `xcodebuild test` of PartKindKeysTests, PartKindRoundTripTests, DisplayDriverTests, DrawerCategoriesTests, KindInfoTests, WizardStepsTests, WizardKindStepsTests -> all passed, and KindGuideTests `** TEST SUCCEEDED **` after blurbs for the crystal and powerswitch models were added (it failed on main without them); `node scripts/check-datasheets.js tlc6983` -> `1 of 1 URLs serve a PDF` (the 26 earlier ids passed on 2026-11-23); docs in sync (`docs/parts-format.md` § `lcddriver` `clock`, § `leddriver` `ccsi`, `bus` style none, `lcd` AM/PM). `planning/todo.md` has no plan-56 item. The full Swift suite is the full tier (never a landing step).
Acceptance: all pass (above). Commit: see the commits listed in plan-56-D and plan-56-F; blurbs bea78343

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

- Phase 1, what the app work was: no change to `SchematicModel.swift`, `ReferenceLabel.swift` or `PartLook.swift` (a driver takes the `U` prefix as an `integratedCircuit` and draws as `.block` by default). `PartDetails.displayDriver` (no facts) beside `.rtc`; `PartCatalog.hasRegisters` includes both kinds (register window, `Simulation.registerState`); `PartFacts.kindName` "LED driver" / "LCD driver"; `PartKinds.name(of:)` (`BundledPartFile.swift`), `PartKinds.all`, `info` (group displays) and `infoOrder`; `PartKinds.conditions` (bus conditions as rtc); `LeddriverDraft.swift` / `LcddriverDraft.swift`, hand-written copies of `RtcDraft` with `clock` and `ready` replaced by a required JSON field `display` (analog and timing optional); not in `scripts/part-editor-schema.py` `KIND_FILES` (it cannot see the `display` keys); `NOT_CALLED` there and `PartKindKeysTests.notCalled` skip the four `load_display_*` arrays for converter and rtc, which `PartKindKeysTests` already failed on after plan-56-B added them to `converter_load.c`; for the two display kinds the same arrays are the keys inside `display`, so `testDisplayDriversEditDisplayAsJSON` checks the form has `display`. Counts measured from the run: leddriver 31 parts, lcddriver 20, kinds 41, wizard combinations 95 (25 without a variant); footprints unchanged (944 explicit). New `DisplayDriverTests`.
- `scripts/part-editor-schema.py rtc converter` regenerates both drafts with different `suggested` lists (the shipped parts have moved on since they were generated); not regenerated here.
- 2026-11-24 (follow-plan): `KindGuideTests.testEveryModelHasABlurb` failed on main (crystal: crystal, resonator; powerswitch: limited, ideal-diode, load have no blurb; the count 65 is 70); the blurbs were added to `VariantBlurbs.swift` and the counts changed (70 models, 16 kinds with blurbs). `SevenSegmentSymbolView` draws the new glass names `AM` and `PM`. The harness picture `/tmp/p56-out/lcd-clock.png` shows the lit state only faintly (a gray glass on a dark face): the proof is the printed lit cells.

