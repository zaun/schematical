# plan-57-C: Test point Options: Size menu and Through hole toggle

Last updated: 2026-11-23
Effort: medium
Depends on: plan-57-B

After plan-57-A and plan-57-B a placed test point is correct but its Options
section shows the flat ten-entry **Footprint** menu (`Pad 1.0 mm` …
`Through hole 3.0 mm`). This sub-plan replaces that menu, for a test point only,
with the two controls the owner asked for: a **Size** menu (1.0, 1.5, 2.0, 2.5,
3.0 mm) and a **Through hole** toggle (off by default). Both only choose one of
the part's existing footprints, so no file format changes. It also lands plan-57:
the end-to-end app check and archiving the three sub-plans.

References:

- plan-57-A § 4.2–4.3 (the ten footprints and the part's `footprintChoices`),
  plan-57-B (`TestPointTests.swift`, the symbol).
- `schematical/schematical/Drawer/PartOptionsSection.swift` (the Options
  section; `FootprintChoicePicker` at line 304), its only caller
  `Drawer/SchematicSidebarView.swift:72–73`.
- `schematical/schematical/Canvas/SchematicModel.swift:2273` `setPartFootprint`
  (the single, undoable edit both controls make).
- `schematical/schematical/Backend/HeaderFamily.swift` (the precedent for a
  small `enum` that maps option values to catalog names).
- `docs/file-format.md` § Parts (`footprintID` is saved per placed part).

## Prerequisites

The overall prerequisites are in plan-57-A § Prerequisites; this sub-plan adds
one.

| Must be true | Command | Status |
|---|---|---|
| plan-57-B is complete | `rg -c 'case testPoint' schematical/schematical/Canvas/SchematicModel.swift` → 1 and `test -f schematical/schematical/Canvas/TestPointSymbolView.swift` | NOT MET (B not landed) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue. If you stop,
> report the status of all prerequisites (A's and B's included).

## 1. Goal

- A placed test point's Options show a **Size** menu (1.0 mm … 3.0 mm, default
  1.5 mm) and a **Through hole** toggle (default off) and no Footprint menu.
  Changing either swaps the placed part's footprint to the matching one of the
  ten, keeping the other control's value, in one undoable step; the PCB view
  shows the new pad (a larger pad; a plated hole when the toggle is on); the
  choice is saved in the file and survives reopening.
- Every other part's Options are unchanged.

### Non-goals (explicit constraints)

- No new setting in Settings (no default size for new test points: the part's
  own default, 1.5 mm pad, is what a drop places).
- No change to `docs/file-format.md`: the choice is the existing `footprintID`.
- No change to the automation API: `set_part` still names a footprint by its
  `package` (`Pad 2.0 mm`, `Through hole 2.0 mm`).
- The drawer row still reads "10 footprints" (`ChipDrawerView.swift:420`, the
  generic label for any part with choices).

## 2. Current State

- `PartOptionsSection.body` (`PartOptionsSection.swift:50`) shows the header's
  Rows / Pins / Gender, an LED's Color, a resistor's Ohms, a capacitor's
  Capacitance and, last, `FootprintChoicePicker` when
  `current.part.footprintChoices` is non-empty (lines 98–103), bound to
  `current.footprintID ?? current.part.footprint?.id ?? ""` and
  `document.setPartFootprint(chip.id, $0)`.
- `PartOptionsSection.applies(to:)` is true for any part with choices
  (`PartOptionsSection.swift:40`), so the section already appears for a test
  point.
- `setPartFootprint` (`SchematicModel.swift:2273`) is one `recordEdit` (undoable),
  ignores an id that is not one of the part's choices, and keeps pad 1's board
  point so the part does not jump.
- The saved choice is validated against the part's choices on open
  (`SchematicFile.swift:1000`).
- plan-57-A's footprint ids are `TESTPOINT-PAD-<d>` and `TESTPOINT-THT-<d>`,
  `<d>` one decimal (`1.0` … `3.0`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Callers of `PartOptionsSection` | 1 | `rg -n 'PartOptionsSection' schematical/schematical` → `SchematicSidebarView.swift:72–73` (plus its own definition) |
| Footprint choices of the part | 10 | defined by plan-57-A § 4.3; `TestPointTests` (plan-57-A Phase 2) asserts `footprintChoices.count == 10` |

### Verified properties

| Claim | How verified |
|---|---|
| The two controls need no model change: `setPartFootprint` accepts any of the part's choices and nothing else | read `SchematicModel.swift:2273–2294` |
| The size and the mounting are recoverable from the footprint id alone, so no per-chip state is needed | by construction: plan-57-A § 4.2 fixes the id grammar; `TestPointFamily.parse` (below) is its inverse and a test round-trips all ten |
| The flat picker is the only footprint control for the part | `rg -n 'FootprintChoicePicker' schematical/schematical` → `PartOptionsSection.swift:99`, `PreferencesView.swift` (LED, pull, header defaults only) |

## 3. Design Overview

One small pure `enum`, one UI branch.

- **`TestPointFamily`** (`Backend/TestPointFamily.swift`, `nonisolated enum`, as
  `HeaderFamily`): `sizes = [1.0, 1.5, 2.0, 2.5, 3.0]`, `defaultSize = 1.5`;
  `footprintID(size:throughHole:)` → `"TESTPOINT-PAD-1.5"` or `"TESTPOINT-THT-1.5"`
  (`String(format: "%.1f", size)`); `parse(_ footprintID: String) -> (size:
  Double, throughHole: Bool)?` (nil for any other id); `label(_ size: Double)`
  → `"1.5 mm"`.
- **The UI**: in `PartOptionsSection.body`, for `current.part.isTestPoint`:
  a `Picker("Size", …)` (menu style) over `TestPointFamily.sizes` and a
  `Toggle("Through hole", …)`; each binding reads
  `TestPointFamily.parse(current.footprintID ?? current.part.footprint?.id ?? "")`
  and writes `document.setPartFootprint(chip.id, TestPointFamily.footprintID(…))`
  with the other control's current value. The existing `FootprintChoicePicker`
  branch gets `&& !current.part.isTestPoint`.

**Correctness risk** is small: only the id grammar must agree between the
footprint data (A) and `TestPointFamily`; a test builds all ten ids and finds
each in `part.footprintChoices`, so a drift fails loudly.

The gate is behavior (tests plus one real-app check), not byte-identity. Expected
changes: only a test point's Options differ.

**Rejected alternatives:** storing size and mounting as new per-chip fields
(file-format change for no gain: the footprint id already holds both); a free-form
size field (the footprint library is a closed list); a segmented size control
(five entries do not fit the sidebar's width; a menu does).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` for partial; mark a moot task
> `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` when a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — `TestPointFamily` and the Options controls

- [ ] `schematical/schematical/Backend/TestPointFamily.swift` (new): the enum of
      § 3.
- [ ] `schematical/schematical/Drawer/PartOptionsSection.swift`: the Size menu
      and Through hole toggle for `current.part.isTestPoint`, placed where the
      footprint picker was, and `&& !current.part.isTestPoint` on the
      `FootprintChoicePicker` branch (lines 98–103); update the doc comment at the
      top of the file ("A test point gets Size and Through hole").
- [ ] `schematical/schematicalTests/TestPointTests.swift`:
      - `TestPointFamily`: all ten `(size, throughHole)` combinations build an id
        that is one of `GENERIC-TESTPOINT`'s `footprintChoices`, and `parse`
        inverts each; `parse("RES-0805")` and `parse("TESTPOINT-PAD-9.9")` are nil.
      - A freshly placed test point is `(1.5, false)` by `parse`.
      - Applying the two controls' writes (the same `setPartFootprint` calls the
        bindings make): size 2.0 → `TESTPOINT-PAD-2.0`; then through hole →
        `TESTPOINT-THT-2.0`; then size 3.0 → `TESTPOINT-THT-3.0` (each keeps the
        other value); the placed chip's `footprint` has one pad, of width 3.0 and
        drill > 0 for the last; with the toggle off the pad has no drill.
      - `doc.undo()` after each step restores the previous `footprintID`.
      - A file round trip (`SchematicFile(document:fileURL:catalog:).encoded()`,
        then `SchematicFile.decode` and `apply(to:catalog:fileURL:)` into a new
        document, as `PullResistorTests.open` decodes) keeps `TESTPOINT-THT-2.5`.
      - `GENERIC-LED` (choices, not a test point) is not `isTestPoint`, so it
        keeps the flat picker.
- [ ] `docs/parts-format.md` § Test points (written in plan-57-A): add one
      sentence, "A placed test point's Options show Size and Through hole,
      which pick one of the ten footprints."

Acceptance: both controls swap the footprint, keep each other's value, undo and save.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/TestPointTests` → `** TEST SUCCEEDED **` (est. 8 min incl. build; scoped to the one test class that holds every new assertion).
Commit: —

### Phase 2 — The real-app check and landing

- [ ] Real-app check (the `run` skill; the drawing and the pointer cannot be
      seen by unit tests): launch the app, place `GENERIC-TESTPOINT` from
      Connector › Test Point, select it, and in Options set Size to 2.0 mm and
      switch Through hole on; switch to the PCB view: the pad is 2.0 mm with a
      drilled hole; switch Through hole off: a plain pad. Record what the PCB
      view showed in the Commit line's note. When driving the real mouse, move
      the pointer to the top-left corner (0, 0) when done (`AGENTS.md` § Testing
      in the app).
- [ ] Run the DRC on a board with one test point of each mounting
      (`mcp__schematical__run_drc` after placing and routing both to a net): no
      violation whose message names a test point's pad (annular ring, drill
      or clearance).
- [ ] Archive: `git mv planning/plan-57-A-test-point-part.md
      planning/plan-57-B-test-point-schematic.md planning/plan-57-C-test-point-options.md
      planning/completed/` in the last commit, with every phase's `Commit:` line
      filled in.

Acceptance: the real app shows the Size menu, the Through hole toggle and the
changing pad; the DRC is clean for both mountings; the plans are archived.
  Check: the screenshots / `render` output of the PCB view for the 1.5 mm pad, the
  2.0 mm pad and the 2.0 mm through-hole (est. 10 min).
  Check: `mcp__schematical__run_drc` → no violation naming a test point (est. 2 min).
Commit: —

## Validation Plan

- Tests: `TestPointTests` (family grammar, controls' writes, undo, file round
  trip, non-regression of the flat picker for other parts).
- Coverage check: the new `Backend/TestPointFamily.swift` and the changed
  `PartOptionsSection.swift` are in the app target the tests import; the controls'
  writes go through the same `setPartFootprint` the bindings call, which the
  test calls directly (the SwiftUI bindings themselves are one line each and are
  exercised by the real-app check).
- Runtime proof: Phase 2's real-app and DRC checks.
- Doc sync: `docs/parts-format.md` § Test points gets one sentence (the Phase 1
  task); no format change.
- Final gate (run once, after Phase 2): the plan-57-A final gate
  (`ctest --test-dir /tmp/logic-build`, est. 10 min) and
  `xcodebuild … test -only-testing:schematicalTests/TestPointTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/CardSlotTests -only-testing:schematicalTests/DrawerCategoriesTests -only-testing:schematicalTests/PartKindKeysTests`
  (same flags as Phase 1, est. 12 min) → `** TEST SUCCEEDED **`; the full app
  suite runs in the background per the project's policy, not here.

## Decisions

- A **Size** menu and a **Through hole** toggle, default off, in the part's
  Options — decided by the owner, 2026-11-23.
- Sizes 1.0–3.0 mm in 0.5 mm steps, default 1.5 mm — decided by the author
  (plan-57-A § Decisions).
- No Settings default for new test points — decided by the author: the part's
  default is the common 1.5 mm pad and a drop is cheap to change.

## Corrections

(none yet)

## Summary

The only coupling is the footprint id grammar shared by plan-57-A's data and
`TestPointFamily`; one test pins it. Untouched: the file format, the automation
API, every other part's Options.
