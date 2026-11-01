# plan-06-C: The context-aware PCB sidebar

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-06-B

The PCB page gets a sidebar on the right (the slot the parts drawer uses on
the Schematic page). It changes with the selection:

- **Nothing selected — Board:** width and height, copper layers, board
  colour.
- **Footprint selected:** the part's name at the top; reference as prefix
  + number; origin point; X/Y of that point; rotation (read-only); a
  Hidden | Document | Silkscreen switch each for bounding box, pin 1 and
  reference.
- **Text selected:** a placeholder header until plan-06-D adds the text
  panel; D owns the text fields.

A mm | in switch at the top sets the units of every length field.

Outcome: on the PCB page with nothing selected, change the board to 4
layers, blue, 4 × 3 in: the canvas shows a blue 101.6 × 76.2 mm board.
Select the LED, rename it `D` `1`, set origin Center, X 10 mm, pin 1
Hidden: the canvas shows `D1` with its centre on (10, y), no pin-1 dot;
save and reopen and all of it comes back.

References:

- plan-06-A §4.1–§4.3 (types, document methods), plan-06-B (drawing).
- `schematical/schematical/ContentView.swift` (inspector, toolbar),
  `Drawer/ChipDrawerView.swift` (the existing inspector content).

## Prerequisites

See plan-06-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-06-B complete | `ls planning/completed/plan-06-B-board-drawing.md` → exists | MET (2026-11-01, aee8240) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- `PCBSidebarView` in the PCB page's `.inspector`, toggled by a toolbar
  button, shown by default.
- Board panel, footprint panel and units switch as above, editing through
  plan-06-A's document methods.
- No running checks: the simulation never runs on the PCB page
  (plan-06-B Phase 1).

### Non-goals (explicit constraints)

- The Schematic page's drawer, its toggle and its remembered visibility are
  unchanged.
- No text panel fields (plan-06-D), no toolbar tools (plan-06-D).
- No model or file change beyond what plan-06-A built.

## 2. Current State

- `ContentView.body` (`ContentView.swift`): `.inspector(isPresented:
  drawerBinding)` with `ChipDrawerView`, width `min: 270, ideal: 270, max:
  500`; `drawerBinding` is true only on the Schematic page; the
  "Parts Drawer" toggle (`sidebar.trailing`) is in `.primaryAction` on the
  Schematic page only.
- After plan-06-A: `document.board`, `renameChip`, `setFootprintOrigin`,
  `moveFootprintOrigin`, `setFootprintMarks`, `setBoardSize`,
  `BoardLayout.originPoint`, `ReferenceParts`.

## 3. Design Overview

1. `ContentView`: one inspector whose content and visibility depend on the
   page.
2. `Canvas/PCBSidebarView.swift` (new): units switch + one of three panels.
3. `Canvas/LengthField.swift` (new): a length text field in the board's
   units.

No correctness risk in data (every edit goes through A's tested methods);
the risk is field commit behaviour (reverting bad input), proven by the
harness driving the commit functions.

Rejected alternatives:

- *A second inspector* — SwiftUI allows one `.inspector` per view; content
  switching on the page is the supported shape.
- *Live commit on every keystroke* — a half-typed "1" would shrink the
  board to 1 mm and snap parts mid-typing; fields commit on Return or focus
  loss.

## 4. Detailed Design

### 4.1 ContentView

- `@State private var isPCBSidebarShown = true`.
- `inspectorBinding`: get `page == .schematic ? isDrawerShown :
  isPCBSidebarShown`; set writes the matching one.
- Inspector content: `ChipDrawerView` on Schematic (as today),
  `PCBSidebarView(document:)` on PCB; same column width.
- The toggle in `.primaryAction` shows on both pages: Schematic — "Parts
  Drawer", help "Show or hide the parts drawer" (as today); PCB —
  "Inspector", help "Show or hide the inspector", bound to
  `isPCBSidebarShown`; both `sidebar.trailing`.

### 4.2 PCBSidebarView

`Form { … }.formStyle(.grouped)`.

- Top section: `Picker("Units", selection: board.units)` segmented,
  "mm" | "in".
- `document.selectedChipID` → footprint panel when that chip has a
  placement; `selectedTextID` → `Text("Text")` header (D replaces it);
  otherwise the board panel.

**Board panel** (section "Board"):

- Width, Height: `LengthField`; commit calls `setBoardSize` (clamped
  1–1000 mm, so the field shows the clamped value).
- Layers: `Picker` menu, 1, 2, 4, 6, labelled "1 layer", "2 layers", ….
- Colour: a row of 7 swatch buttons (20-point circles in `BoardColor.fill`,
  2-point accent ring on the current one, `.help` = the title) plus the
  title as text below.

**Footprint panel**:

- Header: reference in `.title2` bold; part number (`part.name`) in
  `.headline`; `part.description` in `.callout` secondary; footprint
  `id` and `description` in `.caption` secondary.
- Section "Reference": `TextField("Prefix")` and `TextField("Number")` on
  one row, filled from `ReferenceParts(reference)`; commit (Return or focus
  loss) calls `renameChip(prefix:number:)` with the number parsed by
  `Int`; on a message, show it in red `.caption` below and put the fields
  back to the current reference.
- Section "Position": `Picker("Origin")` menu with the five
  `FootprintOrigin` titles (Center, Top Left, Center Left, Bottom Left,
  Pad 1 Center) → `setFootprintOrigin`; X, Y: `LengthField`s
  showing `BoardLayout.originPoint`, commit → `moveFootprintOrigin` (the
  result is snapped, so the field shows the snapped value); "Rotation"
  `LabeledContent` with "\(rotation)°".
- Section "Marks": three segmented `Picker`s, "Bounding Box", "Pin 1",
  "Reference", options Hidden | Document | Silkscreen →
  `setFootprintMarks`.

### 4.3 LengthField

`LengthField(title:, millimetres: Double, units: BoardUnits, commit:
(Double) -> Void)`: a `TextField` holding a string; shows mm with 2
decimals or inches (mm / 25.4) with 3, trailing zeros trimmed; on commit
parses a `Double` (a trailing " mm"/" in"/`"` is ignored), converts to mm
and calls `commit`; text that doesn't parse puts the field back. The shown
string refreshes when `millimetres` or `units` changes and the field is not
focused.

The commit and formatting logic lives in `static func format(_ mm: Double,
units:) -> String` and `static func parse(_ text: String, units:) ->
Double?` so the harness can drive it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Inspector slot and board panel

- [x] `ContentView.swift`: §4.1. `harness window-c1`: PCB toolbar ends
      `… Zoom In, Inspector`, Schematic toolbar ends `… Zoom In, Parts
      Drawer`; `c1-schematic.png` `cmp` with `c1-schematic-before.png`
      (taken at aee8240, before any plan-06-C change) → identical.
- [x] `Canvas/LengthField.swift` (new): §4.3, with `commit` returning the
      length the document kept (Corrections 1) and outside changes shown
      unless the user is mid-typing (Corrections 2). `harness lengths`:
      `4`, `0.64`, `0.025`, `100`, `101.6`, `76.19999999999999` (76.2 to
      float precision), `12.5`, `nil`.
- [x] `Canvas/PCBSidebarView.swift` (new): units switch, panel choice,
      board panel (§4.2). `c1-board.png`: Units mm|in, Width 100, Height
      80, Layers "2 layers", seven swatches with green ringed, "Green";
      `xcodebuild` → `** BUILD SUCCEEDED **`.

Acceptance: the PCB page shows the sidebar with the board panel; the
Schematic page is unchanged.
  Check: harness `lengths`: `format(101.6, .inch)` → `4`, `format(0.635,
  .mm)` → `0.64`, `format(0.635, .inch)` → `0.025`, `parse("4 in", .inch)`
  → 101.6, `parse("3\"", .inch)` → 76.2, `parse("abc", .mm)` → nil
  (est. 2 min).
  Check: harness `window-c1`: host `ContentView` in an offscreen 1200 × 800
  `NSWindow` (plan-03-D's runtime-proof method: snapshot the window's frame
  view with `bitmapImageRepForCachingDisplay` + `cacheDisplay`), page PCB,
  nothing selected → `/tmp/p06/c1-board.png` shows Units, Width 100,
  Height 80, Layers "2 layers", seven swatches with green ringed; page
  Schematic → `c1-schematic.png` matches a capture taken before the change
  (drawer, "Parts Drawer" toggle) by eye (est. 5 min).
  Check: plan-06-A `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
Commit: ae7e122

### Phase 2 — Footprint panel

- [x] `Canvas/PCBSidebarView.swift`: footprint panel (§4.2). `harness
      window-c2` (types into the real fields, picks from the real menus and
      segments, Corrections 3): `c2-led.png` U3 / LED-AMBER-0603 / Amber LED
      / `LED-0603 — 0603 chip LED (1.5 x 0.75 mm)`, fields U 3, origin Top
      Left, X 2.54, Y -0.64, 0°, marks Document/Silkscreen/Silkscreen;
      `rename d 1 -> reference D1: true`; oscillator to D 1 → `c2-dup.png`
      red "D1 is already used by another part.", fields back `U`, `2`,
      `reference kept U2: true`; `origin center: true`; X 10 → `origin x =
      10.16: true`; Pad 1 Center → `X/Y show pad 1: true` (9.56, 0); `pin1
      hidden: true`; units in → X `0.4` (`c2-inches.png`); `reopen: D1 true
      origin center true pin1 hidden true units in true`; board 4 layers,
      Blue swatch, 4 × 3 in → `101.6 × 76.19999999999999 mm, 4 layers,
      blue: true` (`c2-board.png`). `xcodebuild` → `** BUILD SUCCEEDED **`.

Acceptance: the footprint panel edits reference, origin, position and marks.
  Check: harness `window-c2`: open `test.schematical`, select the LED →
  `c2-led.png` shows its reference, `LED-AMBER-0603`, description,
  footprint id, prefix `U` / number, origin Top Left, X/Y, rotation 0°,
  Marks Document / Silkscreen / Silkscreen. Drive the panel's commit
  closures: rename (`"d"`, `"1"`) → reference `D1`; rename the oscillator
  to `D1` → the red message `D1 is already used by another part.` in
  `c2-dup.png` and its fields back to its old reference; origin Center, X
  `10` → origin point x = 10.16 (snapped); origin Pad 1 Center → X/Y show
  pad 1's centre; pin 1 Hidden; units in →
  `c2-inches.png` shows X `0.4`; save, reopen → reference `D1`, origin
  center, pin1 hidden, units in (each printed `true`) (est. 10 min: one run
  covers every field; no smaller check exercises the window).
  Check: plan-06-A `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
Commit: ae7e122

## Validation Plan

- Tests: the harness modes above.
- Coverage check: every panel row is either rendered (`c1`, `c2` PNGs) or
  driven through its commit closure (`window-c2`).
- Runtime proof: the §Outcome sequence in `window-c2`, plus `c2-board.png`
  after setting 4 layers, blue, 4 × 3 in: a blue 101.6 × 76.2 mm board.
- Doc sync: none (format unchanged since A).
- Final gate (once): the `xcodebuild` (est. 1 min) + `lengths`,
  `window-c1`, `window-c2` (est. 15 min). Backend untouched.
- Results (2026-11-01): as ticked above; one commit carries both phases
  (Phase 1 and 2 share `PCBSidebarView.swift`).

## Decisions

- Sidebar contents: board info (size in mm/in, layers, colour) with nothing
  selected; name, reference, origin and the three mark switches with a
  footprint selected — owner, 2026-11-01. Origin options: center, top
  left, center left, bottom left, pad 1 center — owner, 2026-11-01.
- Reference as two fields, prefix and number — owner, 2026-11-01.
- One mm/in switch for every length — owner, 2026-11-01.
- Author's choices: the sidebar uses the inspector slot, own toggle
  ("Inspector"), shown by default; fields commit on Return or focus loss;
  X/Y show the origin point; rotation shown read-only (the Rotate button
  changes it); colour picked from swatches; mm shown to 2 decimals, inches
  to 3.

## Corrections

1. §4.3's `commit: (Double) -> Void` can't show a clamped value that equals
   the old one (board 1000 mm, type 5000 → document keeps 1000, so
   `millimetres` doesn't change and the field kept "5000"). `commit` now
   returns the length the document kept, and the field shows that.
2. §4.3's "refreshes when … the field is not focused" left a field stale:
   after Return the field keeps focus, so switching units left X at
   `10.16` (mm) while Y showed `-0.025` (in) — `window-c2` first run. The
   field now refreshes unless its text differs from what it last showed
   (the user is typing), and focus loss without an edit commits nothing
   (otherwise the rounded display value, e.g. `-0.64` for −0.635, would be
   committed and could move a part).
3. Driving the window: a grouped `Form` gives `TextField`s no placeholder
   and SwiftUI builds a menu `Picker`'s items only while the menu is open,
   so the harness names sidebar fields by order and opens each pop-up with
   `performClick`, picking the item from a timer during menu tracking; the
   swatch is clicked with a synthesized mouse event. The duplicate message
   is checked in `c2-dup.png` (SwiftUI `Text` exposes no AppKit view).

## Summary

UI over plan-06-A's tested methods; the risk is field commit and revert
behaviour, driven through the harness. The Schematic page's drawer is left
as it is.
