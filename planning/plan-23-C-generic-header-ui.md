# plan-23-C: Generic header sidebar options and Settings defaults

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-23-B

With a header selected, the Schematic sidebar's **Options** section shows
**Rows** (1 or 2), **Pins per row** (1–40), **Gender** (Male or Female) and
**Footprint** (vertical or right angle). A pick takes effect at once on the
canvas, the PCB and the saved file. Settings gets a **New Generic Header**
section with the same four controls, which a dropped header starts with
(1×2, Male, vertical unless changed).

Outcome: select a header and pick 2 rows, 5 per row, Female and right
angle. The symbol shows a 2×5 socket header, the PCB shows
`PINHDR-2X05-RA` with pad 1 where it was, and the file saves
`"partNumber": "GENERIC-HEADER-2X05"`, `"gender": "Female"`,
`"footprint": "PINHDR-2X05-RA"`. With Settings at 1×8 / Female / right
angle, the next dropped header is that.

References:

- plan-23-B (`setHeaderSize(_:to:)`, `setPartGender`, `HeaderFamily`,
  `PlacedChip.gender`)
- `planning/completed/plan-18-B-generic-led-ui.md` (the Options section,
  `ColorChoicePicker`, `FootprintChoicePicker`, `NewPartOptions`, the
  New Generic LED Settings section, the drop)
- `schematical/schematical/Drawer/PartOptionsSection.swift`
- `schematical/schematical/Preferences/PreferencesView.swift`
- `schematical/schematical/Canvas/SchematicCanvasView.swift:145-160` (the drop)

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-23-B complete | `ls planning/completed/plan-23-B-*` → 1 file | NOT MET (2026-11-07: written today) |
| The size setter exists | `grep -c 'func setHeaderSize' schematical/schematical/Canvas/SchematicModel.swift` → 1 | NOT MET (2026-11-07) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A selected header's sidebar shows Options with the four controls, set to
  its current values. Each control calls plan-23-B's setter.
- Settings ▸ New Generic Header sets the size, gender and footprint type of
  every header dropped afterwards. Empty settings mean 1×2, Male,
  vertical.

### Non-goals (explicit constraints)

- No PCB-sidebar control (as with the LED, the options live in the
  Schematic sidebar).
- No change to the model, file, backend or data (plan-23-A/B).
- A shrink removes wires without asking. The owner chose that, and the app
  has no undo. Each Pins per row menu item says how many wires that size
  would remove (see § Sidebar), so nothing is removed unannounced.
- Settings changes affect only headers dropped later.

## 2. Current State

(As of plan-23-B complete.)

- `PartOptionsSection` (`Drawer/PartOptionsSection.swift`) shows Options for
  a part with `PartOptionsSection.applies(to:)`: an LED with more than one
  color, or any part with footprint choices. Since every header has two
  footprint choices, a selected header already shows Options with only
  the Footprint picker. Its rows come from `ColorChoicePicker` and
  `FootprintChoicePicker`, which Settings shares.
- `NewPartOptions` (`PreferencesView.swift`) returns a stored Settings value
  for `GENERIC-LED`, or nil. The drop passes `color:` and `footprintID:`
  (`SchematicCanvasView.swift:150-157`). `PreferencesView` loads its own
  `PartCatalogStore` in `.task`.
- `FootprintChoicePicker` lists choices by footprint description, selected
  by footprint id. The two header choices differ by package (`Straight`,
  `Right angle`), and their ids change with the size.

### Measured populations

| What | Count | Command |
|---|---|---|
| Header controls | 4 | rows, pins per row, gender, footprint (owner request) |
| Preference keys today | 5 | `grep -n 'static let' schematical/schematical/Preferences/PreferencesView.swift` → 6 lines: 5 in `PreferenceKey` (3 toggles + 2 LED) and `NewPartOptions.genericLED` |

### Verified properties

- **The footprint type survives a resize.** plan-23-B's `resized(to:)` keeps
  the choice with the same `package`, so a Settings value stored as a
  package name (`"Right angle"`) applies to any size. (plan-23-B § Placed part.)

## 3. Design Overview

### Sidebar (`PartOptionsSection.swift`)

For a header (`part.kind == PART_HEADER`, rows and perRow from
`Details.header`), the Options section shows, in order:

```
Picker("Rows")          segmented: 1, 2
Picker("Pins per row")  menu: 1 … 40, labelled "8", or "4 — removes 2 wires"
Picker("Gender")        segmented: the part's genders (Male, Female)
FootprintChoicePicker   the part's two choices, by description
```

- Rows and Pins per row call
  `document.setHeaderSize(chip.id, to: catalog.part(named: HeaderFamily.name(rows:perRow:))!)`.
  `PartOptionsSection` gains a `catalog: PartCatalogStore` property, and
  `SchematicSidebarView` passes its catalog.
- Each Pins per row item's label comes from
  `static func pinsPerRowLabel(_ n: Int, rows: Int, wiredPins: [Int]) -> String`,
  where `wiredPins` has one entry per wire end on the header (a pin number,
  repeated for each wire on it). The label is `"\(n)"` when no wired pin is
  above rows × n, else `"\(n) — removes \(k) wire"` / `"… wires"`, where k
  counts the entries above rows × n. A wire with both ends on the header
  counts twice, as two ends.
- Gender calls `setPartGender`; Footprint calls `setPartFootprint` (as for the LED).

### Settings (`PreferencesView.swift`)

- Keys `PreferenceKey.headerRows` (Int, default 0), `headerPinsPerRow` (Int,
  default 0), `headerGender` (String, `""`), `headerFootprintType` (String,
  `""`: the package name, `Straight` or `Right angle`). 0 or `""` means the
  data default.
- `NewPartOptions.headerPart(for part: CatalogPart, catalog: PartCatalogStore, defaults:)
  -> CatalogPart`: for a header part, the catalog part
  `HeaderFamily.name(rows:perRow:)` from the stored rows and per row
  (each falling back to the dropped part's own when 0 or out of range), or
  the dropped part when that name isn't in the catalog. Other parts are
  returned unchanged.
- `NewPartOptions.gender(for:defaults:)` returns the stored gender for a
  header when it is non-empty, else nil (as `color(for:)`).
  `footprintID(for:defaults:)` returns, for a header, the id of the
  part's choice whose package is the stored type, else nil. The LED
  behavior is unchanged.
- `Section("New Generic Header")`: the same four controls bound to the keys,
  each showing the data default when its key is empty. Footer: "Used for
  headers dropped from now on."
- The drop (`SchematicCanvasView`): `let placed = NewPartOptions.headerPart(for: part, catalog: catalog)`,
  then `placeChip(part: placed, symbol: symbol(for: placed), …, color:, footprintID: NewPartOptions.footprintID(for: placed), gender: NewPartOptions.gender(for: placed))`.

Rejected alternatives:

- **A Stepper for pins per row.** Each step would resize and re-route the
  wires, one step at a time, possibly removing wires on the way down to
  the size wanted. A menu picks the size in one step.
- **Storing the Settings footprint as an id.** Ids include the size
  (`PINHDR-1X02-V`), so the stored id would stop matching once the size
  changed. The package name works for every size.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line as it lands.
> An unticked box means NOT DONE.

### Phase 1 — Sidebar Options for a header

- [ ] `Drawer/PartOptionsSection.swift`: the header controls and the wire
      caption (§ Sidebar); `catalog` property.
- [ ] `Drawer/SchematicSidebarView.swift`: pass the catalog.
- [ ] `scripts/harness/generic-header.swift`, add:
  10. A 1×8 header with wires on pins 1 and 7, selected:
      `Harness.renderSidebar(doc, to: "/tmp/p23-sidebar-header.png", size: 320×900)`.
      `pinsPerRowLabel(7, rows: 1, wiredPins: [1, 7])` → `"7"`,
      `(6, rows: 1, [1, 7])` → `"6 — removes 1 wire"`,
      `(3, rows: 2, [1, 7, 8, 8])` → `"3 — removes 3 wires"`.
  11. After `setHeaderSize` to 2×5 (as the Rows/Pins pickers do),
      `setPartGender` Female and `setPartFootprint` to `PINHDR-2X05-RA`,
      encoding gives `"partNumber" : "GENERIC-HEADER-2X05"`,
      `"gender" : "Female"` and `"footprint" : "PINHDR-2X05-RA"`; render the
      sidebar again → `/tmp/p23-sidebar-header-2x5.png`.

Acceptance: the four controls show the header's values, and the menu labels count removed wires correctly.
  Check: `rm -rf /tmp/p23-h && scripts/harness/build.sh scripts/harness/generic-header.swift /tmp/p23-h && /tmp/p23-h/H.app/Contents/MacOS/harness`
  → 11 `ok` lines, exit 0. Read `/tmp/p23-sidebar-header.png`: Options shows
  Rows 1, Pins per row 8, Gender Male, and Footprint "1x8 pin header, 2.54 mm
  pitch, vertical".
  Read the 2×5 image: Rows 2, Pins per row 5, Female, the right-angle
  description, and Details Rows 2 / Pins per row 5 / Gender Female (est. 2 min).
Commit: —

### Phase 2 — Settings and the drop

- [ ] `Preferences/PreferencesView.swift`: the four keys, the
      `NewPartOptions` header functions, the New Generic Header section.
- [ ] `Canvas/SchematicCanvasView.swift`: the drop uses
      `NewPartOptions.headerPart` and passes `gender:`.
- [ ] `generic-header.swift`, add (with a cleared `UserDefaults(suiteName: "p23-test")`):
  12. Empty settings: dropping `GENERIC-HEADER-1X02` the way the drop does
      gives `GENERIC-HEADER-1X02`, Male, `PINHDR-1X02-V`.
  13. Settings rows 1, per row 8, Female, `Right angle` → `GENERIC-HEADER-1X08`,
      Female, `PINHDR-1X08-RA`. A `GENERIC-LED` drop is unchanged (Red,
      `LED-RADIAL-5MM`).
  14. Settings per row 99, gender `Neutral`, type `Sideways` → 1×2, Male,
      vertical.
  15. `Harness.renderHosted(PreferencesView().defaultAppStorage(suite), size: 420×700,
      to: "/tmp/p23-settings.png", settle: 0.5)` with the suite cleared.

Acceptance: a dropped header gets the Settings size, gender and footprint
type, and bad or empty settings fall back to 1×2, Male, vertical.
  Check: the Phase 1 command → 15 `ok` lines, exit 0. Read
  `/tmp/p23-settings.png`: New Generic LED as before, then New Generic
  Header with Rows 1, Pins per row 2, Gender Male and Footprint "1x2 pin
  header, 2.54 mm pitch, vertical" (est. 2 min).
Commit: —

## Validation Plan

- Tests: `generic-header.swift` (15 checks) covers model, file, sidebar and
  Settings. `generic-led-ui.swift` must still pass, since
  `PartOptionsSection` and `NewPartOptions` are shared.
- Coverage check: checks 13 and 14 cover both the stored and the fallback
  paths of all four Settings.
- Runtime proof: the three images. Also, built and launched
  (`open /tmp/p23-dd/Build/Products/Debug/schematical.app`): drop a
  header, wire pins 1 and 7, open Pins per row and see "4 — removes 1
  wire", pick it, and see the wire on pin 7 gone.
- Doc sync: none beyond plan-23-B.
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/p23-build -j8` →
  21/21; `generic-header.swift` 15 `ok`, `generic-led-file.swift` 7 `ok`,
  `generic-led-ui.swift` 10 `ok`, each from a fresh out dir;
  `xcodebuild … -derivedDataPath /tmp/p23-dd CODE_SIGNING_ALLOWED=NO build`
  → `BUILD SUCCEEDED` (est. 7 min).

## Decisions

- The controls are rows, pins per row, Male/Female and footprint. Owner,
  2026-11-07.
- New headers default to 1×2, Male, vertical, configurable in Settings.
  Owner, 2026-11-07.
- Shrinking removes the wires on the removed pins. Owner, 2026-11-07. The
  menu labels are how this plan shows that before it happens.

## Corrections

## Summary

This sub-plan is views and preference strings on top of plan-23-B's
tested setters. The only logic is choosing a catalog part from the Settings
values and labelling each size with the wires it would remove, and both are
covered by harness checks.
