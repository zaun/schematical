# plan-28-A: Painter — draw the symbols through one interface

Last updated: 2026-11-07
Overall Effort: large (3h–1d) — plan-28 is four sub-plans, A–D
Effort: medium (1h–2h)
Depends on: nothing

plan-28 adds **Schematical > Export > SVG Schematic / PNG Schematic / JPG
Schematic**. SVG and PNG are black line art on a transparent background; JPG is
the same art, black on white. The PNG and JPG are 4 px per canvas point.

Today every symbol is drawn by SwiftUI `Canvas { context, _ in … }` closures that
call `GraphicsContext` directly with `Color` and `Text` values. Neither can be
read back, so the same code cannot also write SVG. This sub-plan puts one small
interface between the drawing code and the target (`SchematicPainter`), moves
the eight symbol views onto it, and changes nothing on screen. The behavioral
outcome of A: **the live canvas renders byte-identical PNGs before and after**.
B moves the wire/bus/port layers, C writes SVG, D writes PNG/JPG and the menu.

The four sub-plans are strictly ordered; each is complete in itself and names its
predecessor. The whole design (this section through § 4) is stated here once; B–D
cite it.

References:

- `schematical/schematical/Canvas/ChipBlockView.swift`, `DiodeSymbolView.swift`,
  `SupplySymbolView.swift`, `DipSwitchSymbolView.swift`,
  `SevenSegmentSymbolView.swift`, `CharacterLCDSymbolView.swift`,
  `BarrelJackSymbolView.swift`, `HeaderSymbolView.swift` — the eight views A migrates.
- `schematical/schematical/Canvas/SchematicCanvasView.swift:196` (`worldLayer`) —
  the draw order the export repeats (B).
- `scripts/harness/build.sh`, `scripts/harness/support/Harness.swift` (`renderCanvas`)
  — the headless harness that renders the app's real views with `ImageRenderer`.
- `docs/parts-format.md` is not touched; there is no backend work in plan-28.

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.

| Must be true | Command | Status |
|---|---|---|
| Main builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p28-dd CODE_SIGNING_ALLOWED=NO build 2>&1 \| tail -1` → `** BUILD SUCCEEDED **` | MET (2026-11-07, main at 05f609d: `** BUILD SUCCEEDED **`) |
| No other symbol view is half-migrated | `ls schematical/schematical/Canvas/ \| rg -c 'SchematicPainter'` → `0` | MET (2026-11-07: `ls … \| rg -c SchematicPainter` → 0) |
| The symbol set is the 12 cases measured here | `awk '/enum PartSymbol/,/^}/' schematical/schematical/Canvas/SchematicModel.swift \| rg -c '^    case '` → `12` | MET (2026-11-07: 12) |

The third row is why plan-28-A must land before plan-27-J: plan-27-J adds
`PartSymbol.tactileSwitch` and its own `TactileSwitchSymbolView`. plan-28 does
**not** depend on plan-27 (A–I of plan-27 are datasheet/backend work). plan-27-J
carries the prerequisite "plan-28-A complete" (`planning/plan-27-J-tactile-app.md:42`,
landed on main in 05f609d by the plan-27 session) and draws its new view through the
painter (its § 4.2); the arrangement was agreed with that session (see § Decisions).
If `PartSymbol` has gained a case by the time this plan starts, the row above
fails and the stop is real: the new view is a thirteenth drawer and belongs in
this sub-plan's scope, so re-author the scope tables before starting.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `SchematicPainter` (protocol), `Ink`, `PaintedText`, `ScreenPainter` and
  `ExportPalette` exist in `Canvas/SchematicPainter.swift`.
- The eight symbol views draw through `SchematicPainter`. Each exposes
  `static func paint<P: SchematicPainter>(_ painter: inout P, chip: PlacedChip, …)`
  and its `body` is `Canvas { context, _ in var painter = ScreenPainter(context); Self.paint(&painter, …) }`.
- No `GraphicsContext` or `Color` appears in those eight files' drawing code:
  `rg -c 'GraphicsContext|with: \.color\(' <the eight files>` → no match (0 in each).
- **The live canvas is pixel-identical:** `cmp` of the harness PNGs (§ 3) before
  and after the migration reports no difference for all four images.

### Non-goals (explicit constraints)

- No visible change in the app: same colors, widths, fonts, hover/selection
  tint, play-mode lighting. Anything that changes a pixel is a bug in A.
- `WireLayer`, `BusLayer`, `PortLayer`, `GridBackground`, `PinMarkerLayer` and
  `PCBCanvasView` are not touched here (B moves the first three; the grid, pin
  markers and PCB are never exported).
- The eight views keep their `init` signatures (`chip:isSelected:isHovered:` plus
  `level:`, `levels:`, `frame:`, `backlight:` where they exist) so
  `SchematicCanvasView.swift` and the harness call them unchanged.
- No file format, backend, part-data or `docs/file-format.md` change.

## 2. Current State

- Eight views each hold one `Canvas { context, _ in … }` (`rg -c 'Canvas \{'` over
  the eight → 8) and draw with `context.fill/stroke/draw`:
  `rg -c '(context|turned)\.(fill|stroke|draw)\(' <the eight>` → 75 lines in total
  (ChipBlock 13, Diode 8, Supply 3, Header 6, BarrelJack 11, DipSwitch 12,
  CharacterLCD 10, SevenSegment 12).
- Three have helper statics taking `inout GraphicsContext`:
  `ChipBlockView.drawSineGlyph`/`drawBarrelGlyph` (`ChipBlockView.swift:95,119`),
  `SevenSegmentSymbolView.drawFace` (`:83`), `CharacterLCDSymbolView.drawFace` (`:128`).
- Colors are SwiftUI semantic values: `.primary`, `.secondary`, `.accentColor`,
  `Color(nsColor: .controlBackgroundColor)`, `.red`/`.blue` (pin names,
  `ChipBlockView.nameColor`), `Color.green.opacity(0.85)` and `.white` (DIP lever
  ON), `Color.white.opacity(0.1)` (unlit segment), `Color(white: 0.12)` (7-segment
  face), the LCD's computed `RGB.color`, and the LED light colors from
  `DiodeSymbolView.lightColor(named:)` (`DiodeSymbolView.swift:24`).
- Text is `Text(string).font(.system(size:weight:design:))` with optional
  `.foregroundStyle(.secondary)`; anchors are `.center`, `.leading`, `.trailing`
  (a `UnitPoint`).
- The live canvas stacks, in `worldLayer` (`SchematicCanvasView.swift:196`): grid,
  `BusLayer`, `PortLayer`, `WireLayer`, then one view per chip at `chip.viewOrigin`
  (`SchematicModel.swift:669`), then pin markers.
- The harness compiles every app source except `schematicalApp.swift` with one
  extra `main.swift` and renders real views offscreen: `Harness.renderCanvas`
  (`scripts/harness/support/Harness.swift`), `Harness.open`, `Harness.newDocument`.
- Verified in a throwaway probe (`/tmp/p28-probe/main.swift`, 2026-11-07):
  `ImageRenderer` on a `Canvas` with `scale = 4` gives a 240×120 image for a
  60×30 view, `hasAlpha` true, corner pixel alpha 0.0, and
  `NSBitmapImageRep` writes it as an RGBA PNG and a JPEG.

### Measured populations

| What | Count | Command |
|---|---|---|
| Draw-call lines in the eight views | 75 | `rg -c '(context\|turned)\.(fill\|stroke\|draw)\(' <eight files>` summed |
| `Canvas` closures in the eight views | 8 | `rg -c 'Canvas \{' <eight files>` summed |
| `PartSymbol` cases | 12 | `awk '/enum PartSymbol/,/^}/' …SchematicModel.swift \| rg -c '^    case '` |
| Draw-call lines in Wire/Bus/Port layers (B) | 20 | `awk 'NR>=845' …SchematicCanvasView.swift \| rg -c '(context\|turned)\.(fill\|stroke\|draw)\('` |
| `GraphicsContext` mentions in the eight views | 4 | `rg -c 'GraphicsContext' <eight files>` → ChipBlock 2, SevenSegment 1, CharacterLCD 1 |

### Verified properties

| Claim | How verified |
|---|---|
| `GraphicsContext.stroke(_:with:lineWidth:)` equals `stroke(_:with:style:)` with `StrokeStyle(lineWidth:)` (butt cap, miter join, miter limit 10) | Apple's documentation defines the first as the second with that style; A's pixel gate (§ 3) is the check that would fail if not |
| `Ink`, `PaintedText` and the painter can be plain value types: `GraphicsContext` is a struct, copied for the rotated port labels (`var turned = context`, `SchematicCanvasView.swift:~1010`) | Read `PortLayer` in full |
| The compiled harness can instantiate each view directly | `Harness.renderSidebar` instantiates `SchematicSidebarView(catalog:document:simulation:)` the same way (`Harness.swift`) |

## 3. Design Overview

### 3.1 Ink: a role, not a color

```swift
nonisolated struct Ink: Hashable {
    enum Role: Hashable {
        case primary, secondary, accent, paper       // semantic colors
        case power, ground                            // pin-name colors (red, blue)
        case switchOn, knobOn                         // DIP lever ON: track, knob
        case unlit, displayFace, faceEdge             // 7-segment / LCD face
        case lamp(String)                             // LED / segment light by name ("Red", "White" …)
        case display(Double, Double, Double)          // LCD face colors, r g b in 0…1
        case color(Color)                             // screen-only: simulation wire colors, tool previews
    }
    var role: Role
    var opacity = 1.0
    func opacity(_ value: Double) -> Ink     // multiplies, as Color.opacity does
}
```

Static shorthands (`Ink.primary` …) so call sites read `Ink.primary.opacity(0.5)`.

**Screen palette** (what `ScreenPainter` with `.screen` resolves, so pixels do not change):

| Role | Screen color |
|---|---|
| primary / secondary / accent | `Color.primary` / `Color.secondary` / `Color.accentColor` |
| paper | `Color(nsColor: .controlBackgroundColor)` |
| power / ground | `Color.red` / `Color.blue` |
| switchOn / knobOn | `Color.green.opacity(0.85)` / `Color.white` |
| unlit | `Color.white.opacity(0.1)` |
| displayFace | `Color(white: 0.12)` |
| faceEdge | none (not drawn on screen) |
| lamp(name) | `DiodeSymbolView.lightColor(named: name)` |
| display(r,g,b) | `Color(red: r, green: g, blue: b)` |
| color(c) | `c` |

**Export palette** (`ExportPalette.alpha(for:)` → `Double?`; every drawn thing is
black at that alpha; `nil` means *not drawn*; the final alpha is this times
`ink.opacity`):

| Role | Export alpha | Why |
|---|---|---|
| primary, accent, power, ground, lamp, knobOn, faceEdge | 1 | black line art; pin names are black (owner decision) |
| secondary | 0.6 | keeps the screen's gray text (designators, function labels) as lighter black |
| unlit | 0.15 | faint 7-segment bars so the display is still recognizable |
| switchOn | 0.25 | DIP lever ON: a light track with the black knob; OFF is an empty track with a black knob |
| paper, displayFace, display, color | nil | chip bodies are unfilled (transparent; JPG's white comes from its background); the LCD dot matrix and face fill are dropped, `faceEdge` outlines the face |

`ExportPalette` lives next to `Ink` in A and is unit-tested there; C and D use it.

### 3.2 PaintedText

```swift
nonisolated struct PaintedText {
    var string: String
    var size: CGFloat
    var weight: Font.Weight = .regular
    var monospaced = false
    var ink: Ink = .primary
}
```
`ScreenPainter` turns it into `Text(string).font(.system(size:weight:design:))` and
applies the ink as the code does today: `.secondary` → `.foregroundStyle(.secondary)`
(the hierarchical style, not `Color.secondary`), `.primary` at opacity 1 → no
modifier, anything else → `.foregroundStyle(color)`.

### 3.3 The protocol

```swift
protocol SchematicPainter {
    mutating func fill(_ path: Path, with ink: Ink)
    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle)
    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint)
    mutating func translateBy(x: CGFloat, y: CGFloat)
    mutating func rotate(by angle: Angle)
}
extension SchematicPainter {
    mutating func stroke(_ path: Path, with ink: Ink, lineWidth: CGFloat) { stroke(path, with: ink, style: StrokeStyle(lineWidth: lineWidth)) }
}
```
Painters are value types; copying one and transforming the copy is how the
rotated port labels work (B).

`ScreenPainter(_ context: GraphicsContext, palette: ScreenPalette = .screen)`
resolves ink through the palette (`.screen` table above, `.export` = black at the
export alpha; `nil` → the call returns without drawing). The `.export` palette is
used by D's raster export and by nothing in A.

### 3.4 Correctness risk and the gate

Risk: a color or width changes by accident in 75 mechanical edits. **This is
provably-neutral work, so byte-identity of the rendered PNGs is its gate.** The
harness script `scripts/harness/painter-gate.swift` renders four images with
`ImageRenderer` (`Harness.render`, 2×, light appearance):

1. `symbols.png` — a document placing every symbol kind: `SN74HC00N` (block),
   the same flipped and selected (`document.select(.chip(id))`), `ECS-100A-010`
   (oscillator), `GENERIC-LED` and a flipped one, `TE-GDS0204` (DIP, lever 1
   turned on with `toggleSwitch(id, position: 1)`), `LTC-4727JR` (7-segment),
   `NHD-0216K1Z-FL-YBW` (LCD), `PJ-002A` (barrel jack), `GENERIC-HEADER-1X02`
   male and, via `setPartGender`, female, `5V`, `GND`, `PULLUP-5V`, `PULLDOWN`;
   two wires, one selected; one bus with a port.
2. `lit.png` — `DiodeSymbolView(…, level: 1)` and `level: 0.4`, and
   `SevenSegmentSymbolView(…, levels:)` with every segment at 1 and at 0.5,
   instantiated directly and rendered with `Harness.render`.
3. `be-6502.png` — `be-6502.schematical` (21 parts, 148 wires, 2 buses: `jq` counts)
   via `Harness.open` + `Harness.renderCanvas`, origin at the top-left of the chips' bounding box minus (40,40), size the bounding box plus 80.
4. `test.schematical.png` — the same for `test.schematical` (5 parts, 6 wires).

Not covered by the gate: the LCD's lit/unknown dot colors (needs an `LCDFrame`,
whose custom `init(_ state: LcdState)` removes the memberwise init, and a
simulation); those two lines are mechanical `Ink.display` swaps reviewed by diff.

Rejected alternatives:

- **PDF → SVG.** `ImageRenderer` to a PDF context gives vectors, but turning
  PDF operators and embedded fonts into SVG needs a converter; `pdftocairo`
  exists on this machine (`which pdftocairo` → `/Users/justinzaun/local/brew/bin/pdftocairo`)
  but cannot ship in a sandboxed app.
- **A second, SVG-only copy of every symbol's geometry.** Doubles the drawing
  code (75 lines, 8 views) and drifts; every new part symbol would need two drawers.
- **Making the views generic over `GraphicsContext` only.** `Color`, `Shading`
  and `Text` cannot be inspected, so SVG could not be produced from them.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>`; never delete it.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Baseline, from the unmodified tree

Creates the gate and captures what "unchanged" means before any drawing code is touched.

- [ ] Run the Prerequisites commands; record each output in the table above.
- [ ] `scripts/harness/painter-gate.swift` (new): builds the four images of § 3.4
      with only APIs that exist today (`Harness.newDocument`, `Harness.open`,
      `Harness.renderCanvas`, `Harness.render`, `PlacedChip` views by their current
      `init`), writes them to the directory in `CommandLine.arguments[1]`.
- [ ] Build and run it on the unmodified tree:
      `rm -rf /tmp/p28-h && scripts/harness/build.sh scripts/harness/painter-gate.swift /tmp/p28-h && /tmp/p28-h/H.app/Contents/MacOS/harness /tmp/p28-base`
      → four `wrote /tmp/p28-base/….png` lines.
- [ ] Run it a second time into `/tmp/p28-base2` and `cmp` each pair → no output
      (the render is deterministic, so a later `cmp` difference is a code change).
- [ ] View `symbols.png` with the Read tool and note in the Corrections section any
      part that did not render (a missing part name, an empty box).

Acceptance: the four baseline PNGs exist, and the second run is `cmp`-identical to the first.
  Check: `for f in symbols lit be-6502 test.schematical; do cmp /tmp/p28-base/$f.png /tmp/p28-base2/$f.png && echo same $f; done` → four `same` lines (est. 10 min: one `xcodebuild` of the app plus one `swiftc` of the harness; nothing smaller exercises the real views).
Commit: —

### Phase 2 — Painter, Ink, palettes (no callers)

Adds the interface with no caller, so it cannot change a pixel.

- [ ] `Canvas/SchematicPainter.swift` (new): `Ink`, `PaintedText`, `SchematicPainter`,
      `ScreenPalette`, `ScreenPainter`, `ExportPalette` exactly as § 3.1–3.3. No
      `project.pbxproj` edit: `schematical/` and `schematicalTests/` are
      `PBXFileSystemSynchronizedRootGroup`s (`rg -n 'path = (schematical|schematicalTests);' schematical/schematical.xcodeproj/project.pbxproj` → both under that section), so new files in them are built automatically.
- [ ] `schematicalTests/SchematicPainterTests.swift` (new): `ExportPalette.alpha` for
      every role against the § 3.1 table (primary 1, secondary 0.6, unlit 0.15,
      switchOn 0.25, paper nil, display nil, color nil, faceEdge 1); `Ink.opacity`
      multiplies (`.primary.opacity(0.5).opacity(0.5).opacity == 0.25`); `ScreenPalette.screen`
      returns nil only for `faceEdge`.

Acceptance: the app builds and the new tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicPainterTests CODE_SIGNING_ALLOWED=NO 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 3 — Migrate the eight symbol views

Mechanical, view by view; the pixel gate decides.

- [ ] `ChipBlockView.swift`: `paint` static; `outline: Ink`; `nameColor` returns `Ink`
      (`.power` / `.ground` / `.primary`); `drawSineGlyph`/`drawBarrelGlyph` take
      `inout P`. Covers block and oscillator symbols.
- [ ] `HeaderSymbolView.swift`, `SupplySymbolView.swift`, `BarrelJackSymbolView.swift`,
      `DiodeSymbolView.swift` (keep `lightColor(for:)`/`lightColor(named:)` returning
      `Color`; the drawer uses `Ink.lamp(name)` with the chip's color name).
- [ ] `DipSwitchSymbolView.swift` (track `isOn ? .switchOn : .paper`, knob
      `isOn ? .knobOn : outline`), `SevenSegmentSymbolView.swift` (`displayFace` fill
      plus a `faceEdge` stroke of the same rounded rect, width 1; segments `Ink.unlit`
      and `Ink.lamp(name).opacity(level)`), `CharacterLCDSymbolView.swift`
      (`RGB.color` becomes `RGB.ink` → `Ink.display(r,g,b)`; `displayFace`-style fill of the
      face is `.display` too; add the `faceEdge` stroke).
- [ ] Each view: replace every `Text(…).font(…).foregroundStyle(…)` with `PaintedText`
      of the same size, weight, monospaced flag and ink.
- [ ] Re-run the harness into `/tmp/p28-new` and `cmp` against `/tmp/p28-base`.
      A difference is a bug to root-cause: diff the two PNGs with
      `compare -metric AE a.png b.png /tmp/diff.png`, find the symbol in
      `/tmp/diff.png`, and fix that view.

Acceptance: all four PNGs are `cmp`-identical to the Phase 1 baseline, and
`rg -c 'GraphicsContext|with: \.color\(' ChipBlockView.swift DiodeSymbolView.swift SupplySymbolView.swift DipSwitchSymbolView.swift SevenSegmentSymbolView.swift CharacterLCDSymbolView.swift BarrelJackSymbolView.swift HeaderSymbolView.swift` (run in `schematical/schematical/Canvas`) finds nothing.
  Check: `scripts/harness/build.sh scripts/harness/painter-gate.swift /tmp/p28-h && /tmp/p28-h/H.app/Contents/MacOS/harness /tmp/p28-new && for f in symbols lit be-6502 test.schematical; do cmp /tmp/p28-base/$f.png /tmp/p28-new/$f.png && echo same $f; done` → four `same` lines (est. 10 min; a rendered-pixel comparison is the only check that sees a changed color or width).
Commit: —

## Validation Plan

- Tests: `SchematicPainterTests` (Phase 2). The migration itself is gated by pixel identity, not unit tests.
- Coverage check: the gate's `symbols.png` places all 12 `PartSymbol` kinds (block and oscillator both go through `ChipBlockView`), so each migrated view is exercised.
- Runtime proof: the four-image `cmp` in Phase 3.
- Doc sync: none in A (`docs/export.md` is written in D).
- Final gate (run ONCE, after the last phase of A): the Phase 3 check above plus
  `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicPainterTests CODE_SIGNING_ALLOWED=NO` (est. 15 min together). The full app suite runs once at the end of plan-28-D, not here.

## Decisions

- The export is black line art: pin names, LED/7-segment/DIP/LCD colors all become black (grays kept as lower-alpha black) — decided by the owner, 2026-11-07. Settles `ExportPalette`.
- Plan-28-A lands before plan-27-J; plan-27-J draws `TactileSwitchSymbolView` through `SchematicPainter` and carries the prerequisite "plan-28-A complete" — requested by the owner ("talk to the plan27 agent to coordinate"); the plan-27 session agreed and made that edit to its own plan files (05f609d, 2026-11-07). (§ Prerequisites)
- Pixel identity of rendered PNGs is the gate for the refactor (a provably-neutral change); there is no "premise" a diff can falsify — a diff is a bug to root-cause — author, 2026-11-07.

## Corrections

(Filled in during execution.)

## Summary

The risk is silent color/width drift across 75 mechanical edits; the harness
`cmp` against a baseline taken before the first edit is what catches it. Left
untouched: wires, buses, ports, grid, pin markers, PCB (B and never), and every
file format and part file.
