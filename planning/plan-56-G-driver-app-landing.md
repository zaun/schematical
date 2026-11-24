# plan-56-G: App for `leddriver` and `lcddriver`, harness proof and the landing tier

Last updated: 2026-11-23
Effort: large (3h–1d)
Depends on: plan-56-F

Part of plan-56 (display driver ICs; see plan-56-A for the goal, prerequisites and decisions, which apply to every letter). All DigiKey stock lookups use Safari, never `scripts/digikey.js` (owner, § Decisions of plan-56-A): `scripts/safari-digikey.sh` (added in B) loads a DigiKey category URL in a temporary tab of the owner's Safari window and prints the rows.

## 1. Goal

The app lists, places, draws, edits and simulates the two new kinds: block symbols with the bus and display pins grouped, the drawer groups Displays & Indicators › LED drivers / LCD drivers / LCD glass, sidebar facts, Part Editor (File › New Part…), the harness proof, docs and the final gate.

## Phases

### Phase 1 — app
- [ ] `PartCatalog.swift`, `PartFacts.swift`, `SchematicModel.swift`, `ReferenceLabel.swift`, `PartLook.swift`, `KindInfo.swift`, `KindSchema.swift`, `<Kind>Draft.swift` for both kinds (the `docs/backend.md` § Adding a kind list) and their Swift tests (`PartKindKeysTests`, `PartKindRoundTripTests`, `KindGuideTests`, `WizardStepsTests`, `DrawerCategoriesTests`).
Acceptance: `xcodebuild test` of those suites → TEST SUCCEEDED. Commit: —

### Phase 2 — harness proof
- [ ] A `scripts/harness/` script placing a MAX6951 + `7seg` and a PCF8576D + `GENERIC-LCD-8D-1:4` and writing digits through a switch/MCU-free bus stimulus, showing the lit display.
Acceptance: the script prints the digit lit. Commit: —

### Phase 3 — landing tier (once)
- [ ] `ctest --test-dir /tmp/logic-build` 100%; `xcodebuild test` of the Swift suites touched; `node scripts/check-datasheets.js` for the new datasheet ids; `planning/todo.md` updated; `docs/parts-format.md`, `docs/backend.md` in sync; tick the plan checkboxes.
Acceptance: all pass (est. 15 min). Commit: —

## Decisions

- Real models, not facts-only parts — decided by owner, 2026-11-23.
- Safari for DigiKey lookups — decided by owner, 2026-11-23.
- Part names, packages and flags follow AGENTS.md § Datasheets (in stock only; obsolete flagged; PDFs whose parts are all obsolete go to `datasheets/obsolete-not-included.md`).

## Corrections

(none yet)
