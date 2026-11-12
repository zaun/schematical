# plan-37-G: App — registers in the catalog, the drawer filters, the sidebar and the Part Editor

Last updated: 2026-11-12
Effort: medium (1h–2h)
Depends on: plan-37-F

G makes the app treat `register` parts as the logic chips they are:

- `PartFacts.kindName(PART_REGISTER)` → **"Register"** (the sidebar's kind
  line and every place that names a kind).
- `CatalogPart.isLogic` includes `PART_REGISTER`, so the drawer's family
  and count filters apply to register parts (today they skip any part that
  is not `isLogic`: `PartCatalog.swift` filter, `if part.isLogic { … }`),
  and `logicCounts` counts them.
- The drawer's category header (`ChipDrawerView.categoryHeader`) lists
  `PART_REGISTER` among the kinds that show the family and count menus.
- The Part Editor form `RegisterDraft.swift` is regenerated from the
  shipped parts (C generated it from `EXTRA` only), and
  `PartKindRoundTripTests` gains `testRegister` (every shipped register
  part round-trips).

The drawer place already exists: category **Logic**, subcategory
**Registers** (the shift registers and 74_670 use it), and function groups
sort by name (`PartCatalog.functionGroups`), so the new parts appear as a
**Register** group, before "Register file" and "Shift register".

The single behavioral outcome: in the running app, Logic → Registers
lists a "Register" group holding SN74LS173AN; choosing family `HC` in the
Logic family menu hides it and choosing `LS` shows it; placing it shows
Kind "Register", Family "LS", Units "4" in the sidebar. `RegisterTests`
proves the same without the GUI.

References:

- `schematical/schematical/Backend/PartFacts.swift` (`rows(for:)`, the
  `.logic` case :28–30; `kindName(_ kind:)` :145–176).
- `Backend/PartCatalog.swift` (`isLogic` :423–427; `details` from
  `part.logic` :670–671; `functionGroups` :1344–1361; `logicCounts`
  :1120).
- `Drawer/ChipDrawerView.swift` (`categoryHeader` :185–205).
- `Editor/KindSchema.swift`, `Editor/RegisterDraft.swift` (C),
  `scripts/part-editor-schema.py`.
- Tests: `schematicalTests/CapacitorTests.swift` (the per-kind `kindName`
  precedent), `PartKindRoundTripTests.swift` (:31–39), `TestSupport.swift`
  (`allParts`).
- `AGENTS.md` § Testing in the app (pointer to (0, 0) after driving the
  mouse).

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-F complete | `ls planning/completed/plan-37-F-*.md` → exists | NOT MET (2026-11-12: plan written) |
| SN74LS173AN ships | `grep -c '^SN74LS173AN|register|' backend/tests/golden/parts.txt` → 1 | NOT MET until D. If A made it not `add`, use D's replacement part (D § Corrections) here and in § 1 |
| The owner's uncommitted edits are not in G's files | `git status --short schematical/schematical/Backend/PartFacts.swift schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Drawer/ChipDrawerView.swift schematical/schematical/Editor schematical/schematicalTests/PartKindRoundTripTests.swift` → empty | MET (2026-11-12: empty) |

## 1. Goal

- `RegisterTests` (new) passes: `kindName(PART_REGISTER) == "Register"`;
  SN74LS173AN's `details == .logic(family: "LS", units: 4)` and `isLogic`;
  the catalog's Logic group has a Registers subcategory with a function
  group "Register" containing it; with the drawer filter family `HC` it is
  not listed, with `LS` it is (this fails before G: the filter skips
  non-`isLogic` parts).
- `PartKindRoundTripTests.testRegister` passes.
- The real-app check of the outcome, with a screenshot in
  `planning/plan-37-data/g-app-check.png`.

### Non-goals (explicit constraints)

- No Register window for register parts (owner's answer, plan-37-A § Decisions).
- No new symbol: the block symbol draws them (`symbol(for:)`'s default).
- No new drawer category, no reference-prefix change (registers take the
  default `U`; `Preferences/ReferencePrefixes.swift` is the owner's
  uncommitted file and is not touched).
- No file-format change: placed parts are saved by name.

## 2. Current State

- `kindName` has no `PART_REGISTER` case (`default: return ""`), so the
  sidebar shows an empty kind for register parts after C.
- `isLogic` lists ten kinds (gate … arith), not `register`; the details
  chain already builds `.logic` for any part with `part.logic`
  (`PartCatalog.swift:670–671`), so the sidebar's Family / Units rows
  already show for register parts, but the drawer's family and count
  filters and `logicCounts` skip them.
- `categoryHeader("Logic")` shows the family and count menus because the
  category holds gates; listing `PART_REGISTER` there keeps the rule
  "every logic kind" true on its own.
- C's `RegisterDraft.swift` was generated with `register.json` empty.

## 3. Design

- `PartFacts.kindName`: `case PART_REGISTER: return "Register"`.
- `CatalogPart.isLogic`: `|| kind == PART_REGISTER`; the doc comment names
  registers.
- `ChipDrawerView.categoryHeader`: `|| kinds.contains(PART_REGISTER.rawValue)`.
- Regenerate `RegisterDraft.swift` (`python3 scripts/part-editor-schema.py . register`);
  drop the `EXTRA["register"]` lines whose keys shipped parts now use
  (the generator infers them).
- `RegisterTests.swift` (§ 1) and `testRegister` in
  `PartKindRoundTripTests`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Kind name, logic, drawer, form, tests

- [ ] § 3's four code changes.
- [ ] `RegisterTests.swift`, `testRegister`.

Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p37-dd test -only-testing:schematicalTests/RegisterTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO`
→ `** TEST SUCCEEDED **` (est. 8 min: the build; no narrower Swift check).

Commit: —

### Phase 2 — Real-app check

- [ ] Build and launch the app; open the drawer at Logic → Registers;
      place SN74LS173AN; read the sidebar's Kind / Family / Units and
      the category's family menu; save the screenshot to
      `planning/plan-37-data/g-app-check.png`; move the pointer to (0, 0).

Check: the screenshot shows the "Register" group and the sidebar values
of § 1 (est. 5 min).

Commit: —

## Decisions

- **"Register"** is the kind name and the device `function`, matching
  "Latch", "Counter", "Shift register".
- **The form is regenerated, not hand-written**: plan-32's generator and
  the drift test (`PartKindKeysTests`) keep it in step with `register.c`.

## Corrections

(none yet)

## Summary

G names the kind, joins it to the logic filters and the Part Editor round
trip, and proves it in the running app.
