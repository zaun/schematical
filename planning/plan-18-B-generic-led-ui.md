# plan-18-B: Generic LED sidebar options and Settings defaults

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-18-A

plan-18-A made `GENERIC-LED` the one LED part, with a color and footprint on
each placed LED. This sub-plan adds the controls. With a Generic LED
selected, the Schematic sidebar shows **Color** and **Footprint** pickers.
Changing the footprint keeps the LED's pad 1 where it was on the PCB.
Settings gets a **New Generic LED** section that sets the color and
footprint a dropped LED starts with (Red and Radial unless changed).

Outcome: select an LED, pick Blue and 0805. The schematic symbol glows blue
in play mode, and the PCB shows the 0805 pads with pad 1 in the same spot.
The file saves `"color": "Blue", "footprint": "LED-0805"`. With Settings set
to Green / 0603, the next dropped LED is Green / 0603.

References:

- plan-18-A (the data model this builds on: `PlacedChip.color`,
  `footprintID`, `footprintChoice`, `CatalogPart.colors`, `footprintChoices`)
- `schematical/schematical/Drawer/SchematicSidebarView.swift`
- `schematical/schematical/Canvas/PartInfoViews.swift` (`PartHeader`, `ReferenceSection`)
- `schematical/schematical/Canvas/PCBSidebarView.swift:270-280` (the Style picker this mirrors)
- `schematical/schematical/Canvas/SchematicModel.swift:setFootprintStyle` (pad-1 anchoring this mirrors)
- `schematical/schematical/Preferences/PreferencesView.swift`
- `schematical/schematical/Canvas/SchematicCanvasView.swift:145-157` (the drop)
- `scripts/harness/support/Harness.swift` (`renderSidebar`, `render`)

## Prerequisites

plan-18-A must be complete. If it is not, this plan cannot start.

| Must be true | Command | Status |
|---|---|---|
| plan-18-A archived | `ls planning/completed/plan-18-A-*` → 1 file | MET (2026-11-07, in the P-18 worktree: `planning/completed/plan-18-A-generic-led-data.md`) |
| `PlacedChip` has a footprint choice | `grep -c 'var footprintID' schematical/schematical/Canvas/SchematicModel.swift` → 1 | MET (2026-11-07: 1) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A selected `GENERIC-LED` shows a sidebar section with a Color picker (its 5
  colors) and a Footprint picker (its 11 choices). A choice takes effect at
  once on the canvas, the PCB and the saved file.
- A footprint change keeps the centre of pad 1 (K) where it was on the board.
  Rotation, origin and marks are kept.
- Settings ▸ New Generic LED sets the color and footprint each newly dropped
  `GENERIC-LED` gets. It defaults to the part's data default, Red / Radial.

### Non-goals (explicit constraints)

- No PCB-sidebar control. The owner chose the Schematic sidebar only. The PCB
  sidebar already shows the chosen footprint through `PlacedChip.footprint`.
- No change to the file format, the backend or the part data (plan-18-A owns them).
- Changing color or footprint does not stop or rebuild a running simulation.
  Neither one feeds the netlist (plan-18-A Non-goals), and the pickers stay
  enabled while it runs, like the Reference fields.
- Settings changes affect only LEDs dropped afterwards, never placed ones.
- No undo. The app has none today (`grep -ci undo …/SchematicModel.swift` → 0).

## 2. Current State

(As of plan-18-A complete.)

- The Schematic sidebar's part panel is `Form { PartHeader; ReferenceSection;
  Section("Details") }` (`SchematicSidebarView.swift:54-66`), with a fresh
  `.id(chip.id)` per part. The header already reads `U<n>` / `GENERIC-LED` /
  `Generic LED`.
- `setFootprintStyle` (`SchematicModel.swift:1172-1190`) swaps the footprint and
  keeps pad 1. It finds the pad of `part.firstPinDesignator` in the old and
  new footprints and shifts `board.centre` by the difference between their
  board positions. It doesn't re-snap. The footprint swap here is the same
  move.
- The Style picker in the PCB sidebar is the UI precedent: a `Picker` bound
  with `Binding(get:set:)` that calls the document method
  (`PCBSidebarView.swift:271-280`).
- `PreferencesView` is a `Form` of `@AppStorage` toggles keyed by
  `PreferenceKey` (`PreferencesView.swift:1-35`). The Settings scene has no
  catalog: each window makes its own `PartCatalogStore` (`ContentView.swift:63`)
  and loads it in `.task` (`:338`).
- The drop calls `document.placeChip(part:symbol:centeredAt:)`
  (`SchematicCanvasView.swift:150`). After plan-18-A it takes optional
  `color:` and `footprintID:`. Nil means the part's default, and unknown
  values are normalized to the default by `PlacedChip.init`.
- The footprint descriptions shown in the picker come from `footprints.json`,
  for example `"0603 chip LED (1.5 x 0.75 mm)"`,
  `"4-lead through-hole LED, 5.08 mm square"` and
  `"5 mm (T-1 3/4) radial LED, 2.54 mm lead spacing"`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Colors / footprint choices on `GENERIC-LED` | 5 / 11 | plan-18-A §6 (`grep -c '"footprint":"LED-' …/Parts/led.json` → 12: 11 choices + the default) |
| Call sites of `placeChip` in the app | 2 | `grep -c 'document.placeChip(' schematical/schematical/Canvas/SchematicCanvasView.swift` → 2 (drop at :150, supply tool at :464; only the drop can place an LED) |
| Preference keys today | 3 | `grep -c 'static let' schematical/schematical/Preferences/PreferencesView.swift` → 3 |

### Verified properties

- **Pad 1 exists in every choice.** All 11 LED footprints have a pad `1`
  (`grep -E '^LED-' backend/tests/golden/footprints.txt` shows `pads=1:…` first
  on all 11 lines). So the pad-1 anchor always finds a pad.
- **The harness can render the sidebar and Settings.** `Harness.renderSidebar(document, to:)`
  and `Harness.render(view, size:, to:)` exist
  (`scripts/harness/support/Harness.swift:39, 79`).

## 3. Design Overview

Three pieces, in risk order: the document setters (pure model, tested
headlessly), the sidebar section, then the Settings defaults and the drop.
The correctness risk is the footprint swap's pad-1 anchor, which moves the
part on the board. It copies `setFootprintStyle`'s arithmetic, and Phase 1
checks it on every one of the 11 choices at all four rotations.

**Gate class:** behavior change. The checks are the harness runs and their
rendered images, not byte-identity.

Rejected alternatives:

- **Re-snapping the footprint to the grid after a swap.** That moves pad 1, so
  the board traces and parts lined up on it would be off. `setFootprintStyle`
  doesn't re-snap either.
- **Keeping the footprint's centre fixed instead of pad 1.** A 0402 → Radial
  swap would then move pad 1 by several mm, which is worse for anything
  already lined up on pad 1.
- **Settings reading the per-window catalog.** The Settings scene has no
  window. It loads its own `PartCatalogStore` in `.task`, which is the same
  load a window does.
- **Storing the Settings default as a concrete value from the start.** The
  stored value is `""` until the user picks one, and `""` means "the part's
  default". So the default follows the data if `led.json` ever changes its
  default.

## 4. Document setters (`SchematicModel.swift`)

- `setPartColor(_ id: UUID, _ color: String)`: does nothing unless the part is
  an LED and `color` is in `part.colors`. Otherwise it sets `chips[i].color`.
- `setPartFootprint(_ id: UUID, _ footprintID: String)`: does nothing unless
  `footprintID` is one of `part.footprintChoices` and differs from the
  current one. Otherwise it sets `chips[i].footprintID`. If the chip has a
  `board`, it then shifts `board.centre` so the pad of
  `part.firstPinDesignator` has the same board point as before, using the
  same arithmetic as `setFootprintStyle` (`board.board(old.centre)` vs
  `changed.board(new.centre)`), with no re-snap. Rotation, origin, marks and
  style are unchanged.

## 5. Sidebar (`Drawer/PartOptionsSection.swift`, new)

`struct PartOptionsSection: View { let document: SchematicDocument; let chip: PlacedChip }`,
shown in `SchematicSidebarView`'s form between `ReferenceSection` and
Details, only when the part has more than one color or has footprint choices.

```
Section("Options") {
    Picker("Color", …)       // part.colors; each row a filled circle in
                             // DiodeSymbolView.lightColor's color + the name
    Picker("Footprint", …)   // part.footprintChoices; each row the footprint
                             // description
}
.pickerStyle(.menu)
```

Bindings read the chip from `document.chips` by id (as `ReferenceSection` does)
and write through `setPartColor` / `setPartFootprint`. For this,
`DiodeSymbolView.lightColor(for chip:)` gets a sibling
`lightColor(named: String) -> Color`, and the existing function calls it.
The Details rows (plan-18-A `PartFacts.rows(for: chip)`) follow the change
because the panel reads the chip again on every document change.

## 6. Settings and the drop

- `PreferenceKey.genericLEDColor = "genericLEDColor"` and
  `.genericLEDFootprint = "genericLEDFootprint"`. Both are strings, default
  `""`, which means the part's default.
- `enum NewPartOptions` (in `PreferencesView.swift`):
  `static func color(for part: CatalogPart, defaults: UserDefaults = .standard) -> String?`
  and `footprintID(for:defaults:)`. They return the stored value for
  `GENERIC-LED` when it is non-empty, and nil otherwise. Nil, or a value the
  part doesn't have, ends up as the part's default through `PlacedChip.init`.
- `PreferencesView` adds `@State private var catalog = PartCatalogStore()`,
  `.task { catalog.load() }`, and, when `catalog.part(named: "GENERIC-LED")`
  exists:
  ```
  Section("New Generic LED") {
      Picker("Color", selection: color-binding)        // same rows as §5
      Picker("Footprint", selection: footprint-binding)
  } footer: { Text("Used for LEDs dropped from now on.") }
  ```
  Each binding maps `""` to the part's default for display and writes the
  picked value.
- `SchematicCanvasView.swift:150` passes
  `color: NewPartOptions.color(for: part)` and
  `footprintID: NewPartOptions.footprintID(for: part)`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line as it lands.
> An unticked box means NOT DONE.

### Phase 1 — Document setters

`setPartColor` and `setPartFootprint`, with no UI yet. Safe alone: nothing
calls them.

- [x] `Canvas/SchematicModel.swift`: §4, both methods, under `// MARK: Chips`.
- [x] New `scripts/harness/generic-led-ui.swift` (kept), first part. It prints
      one `ok`/`FAIL` line per check and exits 1 on any FAIL:
  1. `setPartColor(id, "Blue")` → `chip.color == "Blue"`;
     `setPartColor(id, "Purple")` → unchanged; on a non-LED part (SN74HC00N)
     → `color` stays nil.
  2. For each rotation 0/90/180/270 and each of the 11 choices: from Radial,
     `setPartFootprint` to the choice. Then `chip.footprint?.id` is the
     choice, and the board point of pad `1` (from `chip.boardPads`) is
     within 1e-9 mm of its point before the swap. That is 44 cases, reported
     as one `ok` line with the count.
  3. `setPartFootprint(id, "LED-9999")` and setting the current id again →
     `board` unchanged.
  4. After (1)+(2), encoding gives `"color" : "Blue"` and the last footprint id.
     (The loop's last choice is Radial, the default, so the script then sets
     `LED-1206` and checks 3 and 4 use that id.)

Acceptance: both setters change only what §4 says, and pad 1 never moves.
  Check: `rm -rf /tmp/p18-h && scripts/harness/build.sh scripts/harness/generic-led-ui.swift /tmp/p18-h && /tmp/p18-h/H.app/Contents/MacOS/harness`
  → 4 `ok` lines, exit 0 (est. 1.5 min).
  Result: 4 `ok` lines, check 2 printed `44 cases`, `exit=0`.
Commit: 7d3a74b

### Phase 2 — Sidebar Options section

The pickers appear for a selected Generic LED.

- [x] New `Drawer/PartOptionsSection.swift` (§5). Add
      `DiodeSymbolView.lightColor(named:)`.
- [x] `Drawer/SchematicSidebarView.swift`: `PartOptionsSection(document:chip:)`
      between `ReferenceSection` and `Section("Details")`.
- [x] `generic-led-ui.swift`, add:
  5. Select a `GENERIC-LED` set to Blue / `LED-0805`, then
     `Harness.renderSidebar(doc, to: "/tmp/p18-sidebar-led.png")`.
  6. Select `SN74HC00N` and render `/tmp/p18-sidebar-gate.png`.

Acceptance: a selected Generic LED shows Options with Color "Blue" and
Footprint "0805 chip LED (2.05 x 1.25 mm)". Its Details show Package 0805 and
Color Blue. A gate shows no Options section.
  Check: the Phase 1 command → 6 `ok` lines. Then read both PNGs: the LED image
  shows the header `GENERIC-LED` / `Generic LED`, an Options section with
  those two values and the Details rows; the gate image has no Options
  section (est. 2 min).
  Result: 6 `ok` lines, exit 0. `p18-sidebar-led.png` (rendered 320×900 so
  the Details Color row is in frame): header `U1` / `GENERIC-LED` / `Generic LED`;
  Options with Color "● Blue" and Footprint "0805 chip LED (2.05 x 1.25 mm)";
  Details Package 0805, Footprint `LED-0805 — …`, Color Blue.
  `p18-sidebar-gate.png`: Reference then Details, no Options section.
Commit: 01c3a84

### Phase 3 — Settings defaults and the drop

New LEDs use the Settings choice.

- [x] `Preferences/PreferencesView.swift`: the two keys, `NewPartOptions`, and
      the New Generic LED section with its own catalog (§6).
- [x] `Canvas/SchematicCanvasView.swift:150`: pass both options.
- [x] `generic-led-ui.swift`, add (using a `UserDefaults(suiteName: "p18-test")`
      that is cleared first):
  7. Empty defaults → `NewPartOptions` gives nil/nil, and `placeChip` gives
     Red / `LED-RADIAL-5MM`.
  8. Set `"Green"` / `"LED-0603"` → `placeChip` with `NewPartOptions` gives
     Green / `LED-0603`. `SN74HC00N` gives nil/nil.
  9. Set `"Purple"` / `"LED-9999"` → Red / `LED-RADIAL-5MM`.
  10. `Harness.render(PreferencesView(), size: 420×400, to: "/tmp/p18-settings.png")`
      after the catalog loads (`renderHosted`, settle 0.5 s).
      (The view gets `.defaultAppStorage` of the cleared `p18-test` suite, so
      the render never reads or writes the app's own Settings.)
- [x] (added) The sidebar and Settings share the pickers:
      `ColorChoicePicker` and `FootprintChoicePicker` in
      `Drawer/PartOptionsSection.swift` (§6's "same rows as §5").

Acceptance: a dropped LED gets the Settings color and footprint, and bad
or empty settings fall back to Red / Radial.
  Check: the Phase 1 command → 10 `ok` lines. Then read `/tmp/p18-settings.png`:
  it shows a New Generic LED section with Color Red and Footprint "5 mm (T-1 3/4)
  radial LED, 2.54 mm lead spacing" under the three part toggles (est. 2 min).
  Result: 10 `ok` lines, exit 0. `p18-settings.png`: "Include in parts list"
  with the three toggles, then "New Generic LED" with Color "● Red" and
  Footprint "5 mm (T-1 3/4) radial LED, 2.54 mm lead spacing", footer "Used for
  LEDs dropped from now on."
Commit: —

## Validation Plan

- Tests: `generic-led-ui.swift` (10 checks) covers the setters, including bad
  values, a non-LED part and all 44 pad-1 cases; it also covers the sidebar
  rendering and the defaults (empty, set, bad). plan-18-A's
  `generic-led-file.swift` keeps covering the file side.
- Coverage check: check 2 reports 44 cases (the loop really ran over every
  choice and rotation). Checks 5 and 10 produce images that show the new views.
- Runtime proof: build and launch the app
  (`xcodebuild … -derivedDataPath /tmp/p18-dd build`, then
  `open /tmp/p18-dd/Build/Products/Debug/schematical.app`). Open
  `be-6502.schematical`, select U22 and pick Blue and 0805. Play: the LED
  glows blue when lit. On the PCB page, the 0805 pads are drawn with pad 1
  where the radial pad 1 was.
- Doc sync: none beyond plan-18-A (the file format is unchanged here).
  `docs/file-format.md` already describes `color`/`footprint`.
- Final gate (once, after Phase 3): `ctest --test-dir /tmp/p18-build -j8` →
  21/21; both harness scripts (`generic-led-file.swift` 7 ok,
  `generic-led-ui.swift` 10 ok), each from a fresh out dir;
  `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p18-dd CODE_SIGNING_ALLOWED=NO build`
  → `BUILD SUCCEEDED` (est. 5 min total).

## Decisions

- Options appear in the Schematic sidebar only. Decided by the owner,
  2026-11-07. Settles §5 and the PCB non-goal.
- New Generic LEDs start Red / Radial, configurable in Settings. Decided by
  the owner, 2026-11-07. Settles §6.
- The header is the part number `GENERIC-LED` with the title "Generic LED".
  Decided by the owner, 2026-11-07. Already true after plan-18-A, so
  `PartHeader` is not changed.

## Corrections

- **Prerequisite "plan-18-A archived".** Under `/follow-plan 18`, A and B are
  done in one `P-18` worktree, so A was archived there
  (`git mv` to `planning/completed/`, commit 953bd2a) before B started. The
  gate command then holds in that worktree, and on `main` once it merges.
- **§5 layout.** With the Footprint picker in a normal labelled row, the
  first render cut the value to "0805 chip LED (2.05 x 1.…", which fails
  the Phase 2 acceptance. The Footprint picker now sits under its label at
  full row width (labels hidden), and the full description shows.

## Summary

The only arithmetic is the pad-1 anchor on a footprint swap. It copies
`setFootprintStyle` and is checked on all 44 choice × rotation cases. The
rest is views and two preference strings. The file format, backend, part data,
simulation and the PCB sidebar are untouched.
