# plan-28-D: PNG and JPG, the Export menu, docs, landing

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-28-C

Adds the raster writers, the save-panel action on `SchematicFileController`, and
the **Schematical > Export** submenu (SVG Schematic, PNG Schematic, JPG
Schematic). The behavioral outcome of D, and of plan-28: **choosing any of the
three items in the running app writes the focused window's schematic to the
chosen file — SVG and PNG black on transparent, JPG black on white, PNG/JPG at
4 px per canvas point.**

References:

- `planning/plan-28-A-export-painter.md` § 3, `planning/plan-28-B-export-scene.md` § 3,
  `planning/plan-28-C-export-svg.md` § 3 (all archived in `planning/completed/` once done).
- `schematical/schematical/Document/SchematicFileController.swift` — `saveAs()` (the
  save-panel pattern to mirror), `present(_:title:)`, `run(_:)`, `SchematicFileCommands`.
- `schematical/schematical/schematicalApp.swift:36` — the `.commands { … }` list.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-28-C complete | `ls planning/completed/plan-28-C-* \| wc -l` → `1` | NOT MET (2026-11-07: C not started) |
| `SchematicExport.svg` exists | `rg -c 'static func svg' schematical/schematical/Document/SchematicExport.swift` → `1` | NOT MET (2026-11-07: file absent) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `SchematicExport.png(_:)` and `.jpg(_:)` return `Data?` (nil for an empty scene).
  PNG has an alpha channel and a fully transparent background; JPG has no alpha and a white background.
- Pixel size = `ceil(padded width) × scale` by `ceil(padded height) × scale`, where
  `scale = SchematicExport.pixelScale(for:)`: 4, or less when the longest side would exceed 8 192 px
  (then `8192 / longest`, so the longest side is 8 192).
- `Schematical > Export > SVG Schematic / PNG Schematic / JPG Schematic` exist, act on the
  focused schematic window, and are disabled with no schematic window focused.
- `docs/export.md` documents the formats, the palette and the rule for new symbols.

### Non-goals (explicit constraints)

- No export options dialog, no scale field, no color choice (owner: fixed 4 px/pt, black line art).
- No keyboard shortcuts for the three items.
- No change to the `.schematical` file format or to what a schematic window shows.
- Exports never include selection, hover, grid, pin markers, simulation state or the PCB.

## 2. Current State

- `SchematicFileController` (`Document/SchematicFileController.swift`) owns the window's
  `SchematicDocument` (`private let document`), builds `NSSavePanel`s in `saveAs()` with
  `panel.allowedContentTypes`, `panel.nameFieldStringValue`, `panel.directoryURL`, runs them
  as a sheet with `run(_ panel:)`, and reports failures with `present(_:title:)`.
- `ContentView.swift:408` publishes it: `.focusedSceneValue(\.schematicFile, file)`.
  `SchematicFileCommands` reads it with `@FocusedValue(\.schematicFile)` and disables
  its items when it is `nil`.
- `schematicalApp.swift` lists `SchematicFileCommands()` and `BillOfMaterialsCommands()` in `.commands`.
- The target has the App Sandbox with `ENABLE_USER_SELECTED_FILES = readwrite`
  (`project.pbxproj`, `ENABLE_APP_SANDBOX = YES`), so files chosen in a save panel are writable.
- The app's name in the menu bar is the product name; its first menu is the application menu
  (About, Services, Hide, Quit). **Verified** with a throwaway SwiftUI app (`/tmp/p28-probe/main.swift`,
  2026-11-07): `.commands { CommandGroup(after: .appInfo) { Menu("Export") { Button("SVG Schematic") {} … } } }`
  produced, in `NSApp.mainMenu.items[0].submenu`: `About probe`, `Export` → (`SVG Schematic`, `PNG Schematic`,
  `JPG Schematic`), a separator, `Services`, … `Quit probe`.
- **Verified** in the same probe: `ImageRenderer(content: Canvas{…}.frame(width: 60, height: 30))` with
  `scale = 4` → a `CGImage` 240×120; `NSBitmapImageRep(cgImage:)` has alpha, corner pixel alpha 0.0;
  `representation(using: .png, …)` writes `PNG image data, 240 x 120, 8-bit/color RGBA`, and `.jpeg` with
  `[.compressionFactor: 0.92]` writes a 240×120 JPEG.
- `UTType.svg`, `.png`, `.jpeg` are system types.

### Measured populations

| What | Count | Command |
|---|---|---|
| Entries in `.commands { }` today | 2 | `awk '/\.commands \{/,/^        }/' schematical/schematical/schematicalApp.swift \| rg -c '^            [A-Za-z]+\(\)'` |
| Test files in the app target | 17 | `ls schematical/schematicalTests/*.swift \| wc -l` (incl. `TestSupport.swift`) |

## 3. Design Overview

### 3.1 Raster

```swift
extension SchematicExport {
    static let maxPixels: CGFloat = 8192
    static let pointScale: CGFloat = 4
    static func pixelScale(for size: CGSize) -> CGFloat   // min(4, 8192 / max(w, h))
    @MainActor static func png(_ scene: SchematicScene) -> Data?
    @MainActor static func jpg(_ scene: SchematicScene) -> Data?
}
```
Both render with `ImageRenderer`: content =
`Canvas { context, _ in var c = context; c.translateBy(x: -rect.minX, y: -rect.minY); var p = ScreenPainter(c, palette: .export); scene.paint(into: &p) }.frame(width: W, height: H)`
where `rect = SchematicExport.bounds(of:)` (the same padded rectangle the SVG uses) and
`W = ceil(rect.width)`, `H = ceil(rect.height)`. `renderer.scale = pixelScale(for: CGSize(width: W, height: H))`.
PNG: the view as is (transparent), `NSBitmapImageRep(cgImage:).representation(using: .png, properties: [:])`.
JPG: the view `.background(Color.white)`, `.jpeg` with `[.compressionFactor: 0.92]`.

The 8 192 px cap is the author's call: it bounds the largest image to 8192 × 8192 × 4 bytes = 256 MiB of pixels.

### 3.2 Controller

```swift
enum SchematicExportFormat: CaseIterable, Identifiable { case svg, png, jpg
    var id: Self { self }
    var menuTitle: String      // "SVG Schematic" …
    var fileExtension: String  // "svg", "png", "jpg"
    var contentType: UTType    // .svg, .png, .jpeg
}
extension SchematicFileController {
    func export(_ format: SchematicExportFormat) async          // panel + write
    func writeExport(_ format: SchematicExportFormat, to url: URL) throws   // no UI; tested
}
```
`export`: builds `SchematicScene(document)`; an empty scene shows an alert (`present`-style
`NSAlert`, "Nothing to export" / "Add a part, wire or bus first.") and returns. Otherwise it runs
an `NSSavePanel` (`allowedContentTypes = [format.contentType]`, `nameFieldStringValue =
"\(displayName).\(format.fileExtension)"`, `directoryURL` = the schematic file's folder when it has one,
`canCreateDirectories = true`) as a sheet via `run(_:)`, then `writeExport(_:to:)`; a thrown error
goes through `present(_:title: "Couldn't export “name”")`. Export never changes `fileURL` or
`savedSnapshot` (an export is not a save). `writeExport` produces the data and writes it with `.atomic`.

### 3.3 Menu

`SchematicExportCommands: Commands` in `Document/SchematicExportCommands.swift`:
`@FocusedValue(\.schematicFile) private var file`, and
`CommandGroup(after: .appInfo) { Menu("Export") { ForEach(SchematicExportFormat.allCases) { format in Button(format.menuTitle) { Task { await file?.export(format) } }.disabled(file == nil) } } }`.
Added to `schematicalApp.swift`'s `.commands`. No shortcuts. The items stay enabled on an empty
schematic (the action explains itself); that avoids relying on the menu observing the document.

### 3.4 Correctness risk

The renderer (dimensions, alpha, white JPG background), the 8 192 px cap, and the focused-value
wiring of the menu. The first two are unit-tested on decoded bitmaps; the menu is proven in the
running app, since no headless check can see a menu bar.

Rejected: a scale picker in the save panel (owner chose fixed 4×); `Image`/`NSImage` cache
drawing (ImageRenderer is what the harness and the probe already prove); disabling the menu on an
empty schematic (needs the Commands body to observe the document).

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

### Phase 1 — Raster writers

- [ ] Re-run the Prerequisites commands; record outputs in the table.
- [ ] `Document/SchematicExport.swift`: `maxPixels`, `pointScale`, `pixelScale(for:)`, `png(_:)`, `jpg(_:)` per § 3.1.
- [ ] `schematicalTests/SchematicRasterTests.swift` (new), using `TestSupport`:
      - `pixelScale`: `(100, 100)` → 4; `(2048, 500)` → 4; `(3000, 500)` → `8192 / 3000`; `(500, 4096)` → 2.
      - A scene of `SN74HC00N` with a wire: the PNG decodes (`NSBitmapImageRep(data:)`) to
        `pixelsWide == Int(ceil(W) * 4)` and `pixelsHigh == Int(ceil(H) * 4)` for `W`, `H` from `SchematicExport.bounds`;
        `hasAlpha` is true; the corner pixel's alpha is 0; some pixel has alpha > 0.9 and red, green, blue < 0.1.
      - The JPG of the same scene: same dimensions; `hasAlpha` false; the corner pixel is white (each channel ≥ 0.98); some pixel is dark (each channel < 0.3).
      - Empty document → `png` and `jpg` are `nil`.
      - A scene wider than 2048 pt (two `SN74HC00N` placed 2500 apart) → `pixelsWide` is 8191 or 8192 and `pixelsWide / pixelsHigh` equals `W / H` to within 0.1%.

Acceptance: the raster tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/SchematicRasterTests CODE_SIGNING_ALLOWED=NO 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — Controller action and menu

- [ ] `Document/SchematicExportCommands.swift` (new): `SchematicExportFormat` and `SchematicExportCommands` per § 3.2–3.3.
- [ ] `Document/SchematicFileController.swift`: `export(_:)` and `writeExport(_:to:)` per § 3.2.
- [ ] `schematicalApp.swift`: add `SchematicExportCommands()` to `.commands`.
- [ ] `schematicalTests/SchematicExportActionTests.swift` (new): create a `SchematicFileController(document:catalog:willReplaceDocument:)`
      on a `TestSupport` document with one placed part; `writeExport` each format into `FileManager.default.temporaryDirectory`
      and read the file back: the SVG starts with `<?xml` and contains `<svg`; the PNG starts with bytes `89 50 4E 47`; the JPG with `FF D8`;
      `fileURL` is still `nil` and `hasUnsavedChanges` is unchanged by exporting; writing to a path in a nonexistent directory throws.

Acceptance: the tests pass, and the app builds with the menu.
  Check: `xcodebuild … test -only-testing:schematicalTests/SchematicExportActionTests -only-testing:schematicalTests/SchematicRasterTests CODE_SIGNING_ALLOWED=NO 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

### Phase 3 — Running app, docs

- [ ] Launch the built app (`xcodebuild … -derivedDataPath /tmp/p28-dd build`, then open
      `/tmp/p28-dd/Build/Products/Debug/schematical.app`), open `be-6502.schematical` with ⌘O, and with the real mouse:
      open the Schematical menu → Export; screenshot the open submenu showing the three items (record the screenshot path here);
      choose each item and save into `/tmp/p28-out/` (type the path in the save panel's name field).
      When done, move the pointer to the top-left corner of the screen (0, 0) (AGENTS.md).
- [ ] Check the three files: `xmllint --noout /tmp/p28-out/be-6502.svg`; `file /tmp/p28-out/be-6502.png /tmp/p28-out/be-6502.jpg` → `8-bit/color RGBA` for the PNG and a JPEG with 3 components;
      `magick identify -format '%[opaque]\n' /tmp/p28-out/be-6502.png` → `False`, and for the JPG `True`;
      `magick /tmp/p28-out/be-6502.png -background white -flatten /tmp/p28-out/flat.png`; view `flat.png`, the JPG and `rsvg-convert -z 2 … be-6502.svg` with the Read tool — all three show the same schematic, black on white, with nothing cut off at the edges.
- [ ] Export from a window with an empty schematic: the alert appears, no panel.
- [ ] `docs/export.md` (new): the three formats and their pixel rule; the ink table from plan-28-A § 3.1 (screen vs export);
      "a new part symbol draws through `SchematicPainter` with `Ink`/`PaintedText`, never `GraphicsContext`/`Color`, and needs a case in
      `SchematicScene.paint`'s switch (it has no `default`)"; text is live `<text>` in SVG; where the code lives.
- [ ] `docs/file-format.md` is not changed (no format change); `rg -n 'Export' docs/file-format.md` → nothing.

Acceptance: the submenu shows the three items and each writes a valid file; the PNG is non-opaque, the JPG opaque, both visually the whole schematic.
  Check: the commands above and the screenshot (est. 20 min; nothing headless can see a menu bar or a save panel).
Commit: —

### Phase 4 — Landing tier (once)

- [ ] `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO 2>&1 | tail -3` → `** TEST SUCCEEDED **`.
- [ ] Move `plan-28-D-export-menu.md` to `planning/completed/` (A, B and C were moved as each landed), with its `Commit:` lines filled.

Acceptance: the full app suite passes. The canvas pixel gates already ran in A and B and D does not touch drawing code, so none is repeated.
  Check: the `xcodebuild … test` command above → `** TEST SUCCEEDED **` (est. 15 min).
Commit: —

## Validation Plan

- Tests: `SchematicRasterTests` (dimensions, alpha, white JPG, cap, empty), `SchematicExportActionTests` (files, headers, no effect on save state, failure case).
- Coverage check: both new test files exercise `SchematicExport.png/jpg` and `SchematicFileController.writeExport`, the only code the menu calls besides `NSSavePanel`.
- Runtime proof: Phase 3, in the real app.
- Doc sync: `docs/export.md` (new).
- Final gate (ONCE, after Phase 3): Phase 4.

## Decisions

- Menu is **Schematical (application menu) > Export**, after About — as the owner wrote it, 2026-11-07. Verified buildable by probe.
- SVG and PNG black on transparent; JPG black on white — decided by the owner, 2026-11-07.
- PNG/JPG at 4 px per point, fixed — decided by the owner, 2026-11-07.
- JPEG quality 0.92 — author, 2026-11-07.
- Pixel cap 8 192 px on the longest side — author, 2026-11-07 (§ 3.1).
- Menu items stay enabled on an empty schematic and explain themselves in an alert — author, 2026-11-07 (§ 3.3).
- No keyboard shortcuts — author, 2026-11-07.

## Corrections

(Filled in during execution.)

## Summary

The risk is in the running app only: the menu and save panel, which no unit
test can see; Phase 3 exists for that. The renderer and file writing are covered
by decoded-bitmap tests. Left untouched: file format, simulation, the PCB view,
the backend, and all datasheet/part data.
