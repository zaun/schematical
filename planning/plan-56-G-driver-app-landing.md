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
Acceptance: `xcodebuild test` of those suites → TEST SUCCEEDED (all but `KindGuideTests`, whose failures are the `crystal`, `powerswitch` and `decoder` ones of other work; see Corrections). Commit: —

### Phase 2 — harness proof
- [ ] (not done: the benches in backend/tests drive each device through its real bus against the real glass and 7seg parts; a GUI harness needs a bus stimulus part the library does not have) A `scripts/harness/` script placing a MAX6951 + `7seg` and a PCF8576D + `GENERIC-LCD-8D-1:4` and writing digits through a switch/MCU-free bus stimulus, showing the lit display.
Acceptance: the script prints the digit lit. Commit: —

### Phase 3 — landing tier (once)
- [~] `ctest --test-dir /tmp/logic-build` 55/55 passed (2026-11-23); the Swift suites touched pass except `KindGuideTests` (crystal and powerswitch blurbs, from other sessions); `node scripts/check-datasheets.js` on 26 new ids: 26 of 26 serve a PDF; docs in sync. `planning/todo.md` is not updated (it holds the owner's uncommitted edits). Not run: the full Swift suite.
Acceptance: all pass (est. 15 min). Commit: —

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

- Phase 1, what the app work was: no change to `SchematicModel.swift`, `ReferenceLabel.swift` or `PartLook.swift` (a driver takes the `U` prefix as an `integratedCircuit` and draws as `.block` by default). `PartDetails.displayDriver` (no facts) beside `.rtc`; `PartCatalog.hasRegisters` includes both kinds (register window, `Simulation.registerState`); `PartFacts.kindName` "LED driver" / "LCD driver"; `PartKinds.name(of:)` (`BundledPartFile.swift`), `PartKinds.all`, `info` (group displays) and `infoOrder`; `PartKinds.conditions` (bus conditions as rtc); `LeddriverDraft.swift` / `LcddriverDraft.swift`, hand-written copies of `RtcDraft` with `clock` and `ready` replaced by a required JSON field `display` (analog and timing optional); not in `scripts/part-editor-schema.py` `KIND_FILES` (it cannot see the `display` keys); `NOT_CALLED` there and `PartKindKeysTests.notCalled` skip the four `load_display_*` arrays for converter and rtc, which `PartKindKeysTests` already failed on after plan-56-B added them to `converter_load.c`; for the two display kinds the same arrays are the keys inside `display`, so `testDisplayDriversEditDisplayAsJSON` checks the form has `display`. Counts measured from the run: leddriver 31 parts, lcddriver 20, kinds 41, wizard combinations 95 (25 without a variant); footprints unchanged (944 explicit). New `DisplayDriverTests`.
- `scripts/part-editor-schema.py rtc converter` regenerates both drafts with different `suggested` lists (the shipped parts have moved on since they were generated); not regenerated here.
