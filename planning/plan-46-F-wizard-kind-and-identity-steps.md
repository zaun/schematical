# plan-46-F: Part Wizard — step 1 (what are you making?) and step 2 (part number)

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-E

F replaces the separate New Part picker window with the wizard's first
step, inside the editor window, and gives the part-number step its guided
form. Step 1 lists every kind in plain words, grouped (`KindInfo`, D),
with a blurb and example parts, and leads with two shortcuts: **Start from
a similar part** (a catalog search; choosing a match copies it, as Copy to
Editor… does, and the wizard continues with everything filled in) and
**Just a symbol and footprint (no simulation)** (the `generic` kind, B).
Step 2 asks for the part number, manufacturer and package, with
suggestions from the catalog, and derives the internal keys (device id,
function, drawer path) until the user edits them under **More options**.

Behavioral outcome: File › New Part… opens one editor window on "What are
you making?"; typing `74HC00` under Start from a similar part and choosing
SN74HC00N fills a gate draft (new uuid) and moves to Part number; going
back and choosing LED asks before clearing the gate's own keys, and keeps
the name, manufacturer, datasheet, pins and footprint.

References:

- `Editor/PartKindPicker.swift` (retired by F), `Editor/PartEditorCommands.swift`
  (New Part… opens the picker), `schematicalApp.swift:146–151` (the
  picker's `Window`).
- `Editor/PartEditorTarget.swift`, `Editor/PartEditorModel.swift:73–118`
  (`init`, `copy(named:catalog:)`), `Editor/PartDraft.swift:205–225`
  (`init(newOf:)`: device id `custom`, drawer `["Custom"]`, manufacturer
  `Generic`).
- `Backend/PartCatalog.swift`: `groups(matching: DrawerFilter)` (:1592, the
  drawer's search), `packages` (:1400), `allParts` (:1394),
  `CatalogPart.manufacturer` (:413).
- `Editor/KindInfo.swift` (D), `Editor/Wizard/` (E).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-E complete | `ls planning/completed/plan-46-E-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-E-wizard-window-shell.md`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `PartEditorTarget.Source.wizard`: File › New Part… opens the editor with
  it; the model starts a `generic` placeholder draft with `kindChosen =
  false` and the current step `kind`. `PartKindPicker` and its `Window` are
  deleted; `.new(kind:)` stays (tests and Copy paths use it).
- Step 1 page `KindStepPage`: the similar-part search (first 20 matches of
  `groups(matching:)` with `query` = the text, flattened, each row "name —
  function, package, manufacturer"); the no-simulation row; then
  `KindGroup.allCases` sections with each kind's name, blurb and examples.
- Choosing a kind: `PartDraft.changingKind(to:)` keeps the generic keys
  (identity, datasheet, pinouts, footprint, flags, note) and replaces
  `kindDevice`/`kindPart` with the new kind's required empty values (and
  `"simulated": false` for unsimulated kinds via their schema). When the
  old kind keys hold values, an alert asks first ("Change to LED? The
  gate's own settings will be cleared. Pins, name, datasheet and
  footprint are kept.").
- Choosing a similar part: the draft becomes `PartEditorModel.copy(named:)`
  of it (new uuid), `kindChosen = true`, step → identity; name gets the
  suffix the library would give (`libraryName`), so it never collides.
- Step 2 page `IdentityStepPage`: Part number (required), Manufacturer
  (text with a menu of the catalog's manufacturers, and "My own design /
  unknown" which sets `Generic`), Package (text with a menu of the
  catalog's packages filtered by the text), Description (one line).
  **More options**: device id, function, drawer path, note, flags.
- Derived keys while untouched (`PartEditorModel.autoKeys: Set<String>`,
  window state): device id = the part number in uppercase with characters
  outside `A-Z 0-9 . -` dropped; function = `KindInfo.name`; drawer =
  `["Custom", KindInfo.group.rawValue]`. Editing a key in More options
  removes it from `autoKeys`.

### Non-goals (explicit constraints)

- **All fields mode is unchanged**, except that a new part's kind is
  shown there as a read-only line with a "Change…" button that jumps to
  step 1 in Guided mode.
- **No catalog changes.**

## 2. Current State

- New Part… → `PartKindPicker` window → list of `PartDraft.kindNames`
  with "Coming soon" rows (none after C) → `openWindow(… .new(kind:))`.
- The catalog has 32897 parts, 297 distinct packages and 65 distinct
  manufacturers (planning count over `Parts/*.json` part objects:
  `package`, `manufacturer`).
- A new draft's device id is `custom` and drawer `["Custom"]`; nothing
  derives them.

### Measured populations

| What | Count | Command |
|---|---|---|
| Distinct packages / manufacturers | 297 / 65 | python over `schematical/schematical/Parts/*.json` (all but footprints, datasheets, boards): sets of each part's `package` and `manufacturer` |
| `PartKindPicker` references | 5 | `grep -rn 'PartKindPicker' schematical/schematical schematical/schematicalTests \| wc -l` (re-measure before deleting; every hit goes) |

## 3. Design notes

- The search reuses the drawer's index, so it is as fast as the drawer
  (DrawerPerformanceTests' budget); it hides obsolete parts as the drawer
  does by default.
- A copied part's datasheet stays the bundled id (no inline entry); the
  datasheet step (G) shows it and lets the user replace it with a URL.

Rejected: keeping the picker window — two places choose a kind, and the
picker cannot offer "start from a similar part" with the draft it
produces.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Kind changes and derived keys (model)

- [x] `PartDraft.changingKind(to:)`; `PartEditorModel`: `.wizard` source,
      `kindChosen`, `autoKeys`, the derivations, `chooseKind`,
      `chooseSimilar(named:)`. (Also `similarParts(matching:)`,
      `kindChangeClearsSettings(to:)`, `issues(of:)`; § Corrections 1–3.)
- [x] `schematicalTests/WizardIdentityTests.swift`: `changingKind` keeps
      name, manufacturer, datasheet, pinouts, footprint and drops `gates`;
      `chooseSimilar("SN74HC00N")` gives kind gate, a new uuid and the
      library's suffixed name; device id from `"74hc00-x/y"` is
      `74HC00-XY`; editing the drawer stops its derivation.
      (`xcodebuild … test -only-testing:schematicalTests/WizardIdentityTests`
      → `Executed 7 tests, with 0 failures`, `** TEST SUCCEEDED **`; the
      suffixed name is `SN74HC00N.2`.)

Acceptance: the model rules hold.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/WizardIdentityTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — The two pages; the picker retired

- [x] `Editor/Wizard/KindStepPage.swift`, `IdentityStepPage.swift`; the
      step list shows `kind` first; `PartEditorCommands` opens `.wizard`;
      `PartKindPicker.swift` and its `Window` deleted;
      ~~`PartEditorWindowTests` updated for the removed window id~~ — moot:
      no test names the picker or its window id (`grep -rn
      'PartKindPicker\|new-part' schematical/schematicalTests` → none; the 5
      references measured in § 2 were all in the app, and all are gone:
      the same grep over `schematical/schematical` → none).
      `WizardModelTests` is the test that changed: the step list now starts
      with `kind`. (`xcodebuild … test
      -only-testing:schematicalTests/PartEditorWindowTests` → `Executed 9
      tests, with 0 failures`, `** TEST SUCCEEDED **`.)
- [ ] Runtime check (run skill): New Part… → search `74HC00` → choose
      SN74HC00N → Part number shows it; back → LED → the alert → OK;
      New Part… → no-simulation → name `SENSOR-1`, two pins, a 2-pad
      footprint → Add to Library → place it → Simulation › Run is
      disabled with "U1 (SENSOR-1) is not simulated". Pointer to (0, 0).

Acceptance: steps 1–2 work in the real app, including a generic part placed and refused by Run.
  Check: the run-skill walk above (est. 15 min); `xcodebuild … test -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

## Validation Plan

- Tests: `WizardIdentityTests`, `PartEditorWindowTests`.
- Runtime proof: Phase 2's walk (also B's runtime proof).
- Doc sync: N (help: New Part… no longer opens a picker).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions.

## Corrections

1. **Derived keys follow edits in both modes.** § 1 says "editing a key in
   More options removes it from `autoKeys`". The model does it where the
   draft changes (`draft.didSet`): a change to the device id, function or
   drawer that the derivation itself did not make removes that key from
   `autoKeys`, so an edit in All fields counts too, not only one under
   More options. An empty part number gives the device id `custom` (the
   loader refuses an empty id).
2. **A copied part derives nothing.** `chooseSimilar` clears `autoKeys`:
   the copy has the library part's own device id, function and drawer
   path, and renaming it must not rewrite them
   (`testStartingFromASimilarPart`).
3. **Before the kind is chosen the step list has no kind steps**
   (`kind, identity, datasheet, pinout, footprint, review`): the draft is a
   `generic` placeholder, and its Supply/Essentials steps would be the
   placeholder's, not the user's. The kind step shows "Choose what you are
   making" until then, and the window is titled `New Part`.
4. **A new part of a known kind (`.new(kind:)`) also lists the kind step**
   and starts on Part number (one Back from it), so the kind can be
   changed in any new-part window; `WizardModelTests` asserts the list.

## Summary

Small model rules behind tests; the visible risk is the picker's removal,
checked by the window tests and the walk.
