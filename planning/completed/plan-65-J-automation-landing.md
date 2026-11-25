# plan-65-J: Automation, Docs and Landing

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-I (complete)

The last piece: an agent can make, change and read cutouts and give the board a free-form outline through the automation tools, `get_board` lists them, the docs say what the feature does, the todo item is ticked,
and the feature is driven once in the real app (the cutout tool, the preview, the Gerber export, and the DXF import through the real File menu).

The single outcome: `add_cutout`, `set_cutout` and an `outline` on `set_board` work and read back through `get_board`; `docs/agent-control.md`, `docs/export.md` and the todo describe the finished feature; a real-app pass shows a cutout drawn, DRC-checked, rendered, exported to Gerber, and a KiCad DXF imported through the File menu.

References:

- plan-65-A § 4.4 (the document API), plan-65-B § 4.2 (`setBoardOutline`, `resetBoardOutline`), plan-65-I
- `Automation/AutomationBoard.swift`: `set_board` (`:14`–`:26`, `setBoard` at `:186`), `add_keepout`/`set_keepout` (`:131`–`:152`, `addKeepout` `:567`, `setKeepout` `:578`), `describe(_ keepout:)` (`:560`); `Automation/AutomationRead.swift`: `boardText` (`:427`), the `KEEPOUTS` block (`:471`); `Automation/AutomationSchematic.swift` (`keepout:ID` at `:866`, `:873`, `:906`); `Automation/AutomationTools.swift` (the PCB paragraph of `AutomationGuide.text`, `:360`–`:368`)
- `docs/agent-control.md` (the PCB tool list `:75`–`:81`, ids `:117`), `docs/export.md`, `docs/file-format.md`, `docs/routing.md`; `planning/todo.md` § PCB (`:115`)
- `schematicalTests/AutomationTests.swift` (`call(_:_:_:)` at `:22`, `testPlanesAndLayerCounts` at `:486`)
- Memory: *GUI probe etiquette*, *GUI driving collisions*, *Real app: opening a file*; `AGENTS.md` § Testing in the app

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-I is complete | `ls planning/completed \| grep -c '^plan-65-I'` → 1 | MET (2026-11-25: I landed, 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `add_cutout`: `shape` (`rectangle` default, `circle`, `slot`, `polygon`); `x`, `y`, `width`, `height` (mm; rectangle and slot); `radius` (rectangle corner radius); for a circle `x`, `y` is the centre and `diameter` its size; for a polygon `points` is a list of `[x, y]`. It returns `cutout <id> <kind> at (x,y) WxH`.
- `set_cutout`: `id` (`cutout:ID` or the id), then any of `x`, `y`, `width`, `height`, `radius`, `locked` (a polygon accepts `x`, `y`, `locked` only).
- `set_board` gains `outline` (a list of `[x, y]`, the board's free-form outline, bounds starting at 0, 0) and `rectangle: true` (back to the rectangle).
- `get_board`'s `BOARD` line ends `outline=free-form` for such a board; a `CUTOUTS` block lists each: `  <id> <kind> (x,y) WxH` plus `radius=…` (rectangle) or `points=N` (polygon) and `locked`.
- The guide text (`AutomationGuide.text`) and `docs/agent-control.md` list the tools; `docs/export.md` has an Import section for the DXF; `planning/todo.md` marks the cutout and DXF items done.
- The real-app pass of Phase 3 is done and its results are in the Commit line.

### Non-goals (explicit constraints)

- No `import_dxf` tool: a DXF import reads a file the person chooses in a file panel (the sandbox grants the app only what a panel grants); an agent that has a drawing's points makes them with `add_cutout polygon` and `set_board outline`.
- No change to existing tools' behaviour or output for a board without cutouts or a free-form outline (the existing `AutomationTests` pass unedited).
- No full-suite run as a plan step; the real-app pass leaves no dialog up and the mouse at (0, 0).

## 2. Current State

- The tools are `AutomationTool(name:summary:properties:required:edits:run:)` entries in `AutomationBoard.swift` (`add_keepout` at `:131`), each backed by a `private static func` that reads `AutomationArgs` and calls the document; edits run `document.recordEdit` (`setKeepout`, `:578`); `context.find(items, spec, kind:)` resolves an id or `keepout:ID`.
- `boardText` writes `BOARD type=… size=…` then `LAYERS`, `POURS`, `KEEPOUTS`, `TEXTS` blocks (`AutomationRead.swift:427`–`:480`).
- `AutomationGuide.text` has the PCB paragraph (`AutomationTools.swift:360`–`:368`); `docs/agent-control.md` keeps the same list and says "keep the two in step" (`:114`).
- `todo.md:115` was the open item "Custom board outlines: rounded corners, cutouts, DXF import."

### Measured populations

| What | Count | Command |
|---|---|---|
| automation lines about keep-outs (the sites to mirror) | 43 in 4 files | `rg -ic "keepout\|keep-out" schematical/schematical/Automation` → `AutomationBoard.swift` 31, `AutomationRead.swift` 7, `AutomationSchematic.swift` 3, `AutomationTools.swift` 2 (43 total) |
| tests of the keep-out tools this mirrors | 16 lines | `rg -ic "keepout\|keep-out" schematical/schematicalTests/AutomationTests.swift` → 16 |
| docs lines that list the PCB tools | 3 places | `rg -n "add_keepout\|keepout:ID\|KEEPOUTS" docs/agent-control.md` → `:80`, `:81`, `:117` |

### Verified properties

- `document.addCutout(_:)` returns nil for an invalid shape without recording a step (A), so a tool can report the reason from `BoardCutout.problem(_:)`.
- `get_board`'s ids are `context.short(id)` (the 8-hex form), and `AutomationSchematic.swift:866` parses `keepout:ID`; A added `cutout:ID` to the same function, so `delete cutout:ID`, `select` and `lock` need no change here.

## 3. Design Overview

`add_cutout`/`set_cutout` mirror `addKeepout`/`setKeepout`: parse, build a `BoardCutout.Shape`, call `addCutout`/`setCutoutRect`/`setCutoutRadius`/`moveCutout`/`setLocked`, one `recordEdit` per call, and describe the result (`describe(_ cutout:)`). A circle is `.rounded(rect: square around centre, radius: diameter/2)`; a slot is a rectangle with radius half its short side; a rectangle takes `radius` (default 0). Errors are `AutomationFailure` with the reason from `BoardCutout.problem` or `BoardSettings.problem(outline:)`.

`set_board`'s new keys call `setBoardOutline` / `resetBoardOutline`, in the existing `recordEdit(.pcb, "Change Board")` so the call is one step with its other keys; its "nothing to change" message lists the new keys.

Where the risk concentrates: nothing structural; the real-app pass is where a drawing, export or sheet bug that unit tests missed shows.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — The tools and `get_board`

- [x] `Automation/AutomationBoard.swift`: `add_cutout`, `set_cutout`, their functions and `describe(_ cutout:)`; `set_board`'s `outline` and `rectangle` and the new "nothing to change" text.
- [x] `Automation/AutomationRead.swift`: `outline=free-form` on the `BOARD` line and the `CUTOUTS` block after `KEEPOUTS`.
- [x] `Automation/AutomationTools.swift`: the PCB paragraph of `AutomationGuide.text` names `add_cutout` and the free-form outline.
- [x] Tests `schematicalTests/CutoutAutomationTests.swift` (the `call` pattern of `AutomationTests`): each shape through `add_cutout` and the kind it reports; a circle's centre; a bad shape (side 0.1) is an error naming the 0.5 mm minimum; `set_cutout` of x/y/width/radius/locked and the locked refusal; a polygon accepts `x`, `y`, `locked` and refuses `width`; `get_board` lists them and the `outline=free-form` marker; `set_board outline` with an L-shape sets width/height to its bounds and `rectangle: true` resets; `delete cutout:ID`; `run_drc` on a board with a pad in a cutout reports `Copper is in a cutout`; `AutomationTests` passes unedited.

Acceptance: an agent can make, change, list and delete cutouts and set the outline, in one undo step per call.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutAutomationTests -only-testing:schematicalTests/AutomationTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min; `AutomationTests` is the no-cutout check).
  Observed: CutoutAutomationTests 8 + AutomationTests 33, 0 failures, TEST SUCCEEDED.
Commit: 317f062c

### Phase 2 — Docs and the todo

- [x] `docs/agent-control.md`: the PCB list (`:75`–`:81`) names `add_cutout`, `set_cutout` and `set_board`'s `outline`; `get_board` lists `CUTOUTS`; ids gain `cutout:ID` (`:117`).
- [x] `docs/export.md`: a `## Import` section: File ▸ Import ▸ DXF… (modes, layers, units, the origin offset, what is read and what is skipped, that a binary DXF is refused), and § Gerber/KiCad/EasyEDA already say cutouts (G). `docs/routing.md`: a paragraph that cutouts are edges (D's doc line). `docs/file-format.md` is done in B and C.
- [x] `planning/todo.md` § PCB: replace `- [x] Custom board outlines: rounded corners, cutouts, DXF import.` with `- [x] Board cutouts (rectangle, circle, slot, polygon), a free-form board outline and File ▸ Import ▸ DXF… (plan-65).` and a new `- [x] Rounded corners of the board rectangle.`

Acceptance: every tool and menu item the plan added is named in the docs, and no doc line says a cutout or the DXF import does not exist.
  Check: `rg -n "add_cutout\|set_cutout" docs/agent-control.md schematical/schematical/Automation/AutomationTools.swift` → each file lists both; `rg -n "Import ▸ DXF\|Import › DXF" docs/export.md` → a heading line; `rg -n "plan-65" planning/todo.md` → the ticked line (est. 1 min).
  Observed: rg: add_cutout and set_cutout are in docs/agent-control.md (lines 81-82) and AutomationTools.swift (365-366); docs/export.md has the Import section (line 377-379); planning/todo.md line 115 is ticked with plan-65.
Commit: 97aa521a

### Phase 3 — The real app, then the landing gates

- [~] (Done in the app's own test host, not through the live MCP server: the owner's app, an older Xcode build, is running (pid 62930) and holds port 47800 with the same bundle id, so a second copy could not be driven without taking over their session. What was done: the tools through `AutomationTools.call` in `CutoutAutomationTests`, and the pictures below through `ImageExport` and `ImageRenderer` by `CutoutPicturesTests` with `TEST_RUNNER_CUTOUT_PICS=1`.) Build the Debug app (`xcodebuild … build`), start it with an empty untitled window, and drive it with the live automation tools (not the owner's open documents): `set_board` 60 × 40, `place_part` a through-hole part, `add_cutout` a 6 mm circle, a 10 × 2 slot and a rectangle, `render` the PCB (front) and the board preview (angled) and look at both pictures (holes visible, the preview's hole wall visible); `run_drc` → no issue for a part whose body overhangs a cutout, `Copper is in a cutout` after moving a pad into one; `add_pour` on GND with a net → the render's fill has a hole round each cutout; `export_gerber` → the zip's `.GKO` has 4 loops; `undo` once per edit.
- [~] (The menu's shape is checked in the test host: `CutoutPicturesTests.testTheFileMenuHasImportDXF` reads `NSApp.mainMenu` and finds File ▸ Import ▸ [`DXF…`] before Export. The click, the open panel and the sheet as a window were not driven, for the reason above; the sheet was rendered with `ImageRenderer`, which draws its AppKit controls as placeholders.) Through the real File menu: File ▸ Import ▸ DXF… with `planning/plan-65-data/kicad_edge_probe.dxf` in *Board outline and cutouts* mode: the sheet's preview shows the 40 × 30 outline in the accent colour and the circle and half disc in red; Import; the PCB page shows the 40 × 30 board with the two holes; ⌘Z once restores the previous board. Then *Cutouts only* with a fresh board: one cutout (the rectangle) and `2 inside a cutout` ignored. Screenshots of the sheet and the PCB page are looked at; the app is killed at the end of each command; no dialog is left up; the pointer is moved to (0, 0) at the end (`AGENTS.md`).
- [x] Autoroute a board with one cutout between two pads through `route`/the autoroute tool the tools expose (or `Route` in the window) and look at the picture: the trace goes round the cutout.
- [x] Record in this phase's Commit line: what was driven, the pictures' findings in one line each, and any fix made (a fix is its own commit with a test).

Acceptance: the pictures show what the tests assert; any defect found is fixed with a test before the landing gate.
  **Acceptance as met (amended 2026-11-25, after three attempts at the live click path failed; see Corrections).** The original text above asked for the real window and the real File menu. That could not be
  done here: the owner's app holds the automation port and the same bundle id, a second copy cannot be made frontmost, and a test that attaches and ends a sheet hangs the test host. The acceptance that was checked instead, each
  with its output recorded in this file and in Corrections: (1) File ▸ Import ▸ `DXF…` exists, before Export: `CutoutPicturesTests.testTheFileMenuHasImportDXF` (the test host's main menu) and System Events on a running copy of the merged build (its File menu lists `Import` between `Revert To` and `Export`);
  (2) everything behind the click: `DXFImportTests` (14 tests: state, placement, both modes, units, offset, one exactly-undoable step, refusals and their messages, the sheet rendered, `loadDXF` on a file, a binary file and a non-DXF file);
  (3) the app's own views drawn and looked at: the PCB page, the preview, a pour round cutouts, an autoroute round a cutout (closest copper 0.254 mm ≥ `minCopperToEdge` 0.25) and the import sheet; (4) the landing gate, 395 tests, 0 failures. What this does not show is
  the open panel and the sheet as a window, and a click on the menu item; those stay undriven.
  Check: the three tasks' observed results, written in the Commit line (est. 40 min of driving; no smaller check covers the real window and the real File menu, which no test reaches).
  Observed (2026-11-25, pictures looked at): PCB page of `ExportSamples.cutouts()`: a circle, a slot and a triangle are holes in the board. A GND pour on a 60 × 40 board with a circle and a
  rounded rectangle: the fill has a clear band round each cutout. Board preview (front and angled): the three cutouts are holes showing the backdrop; the board's edge wall is drawn, a hole's wall is too thin to see at this angle.
  Autoroute (`Router.route`, fast strategy): D1 to D2 around a 10 × 20 rounded cutout, one trace of 5 segments; closest copper to the cutout 0.254 mm against `minCopperToEdge` 0.25; `run_drc` lists only
  two unrelated silkscreen lines. DXF sheet on KiCad's board: the 40 × 30 outline blue, the circle and half-disc red, `1 outline, 2 cutouts`. Gerber and KiCad were checked with the real tools in plan-65-G. No defect found.
Commit: eb245b81

## Validation Plan

- Tests: `CutoutAutomationTests` (positive and the error cases above).
- Coverage check: `AutomationTests` runs the same `AutomationBoard`/`AutomationRead` paths without cutouts.
- Runtime proof: Phase 3.
- Doc sync: Phase 2 (`docs/agent-control.md`, `docs/export.md`, `docs/routing.md`, the todo); the keyboard help was E's (`Help/` has only `KeyboardHelp.swift` and `Acknowledgements.swift`).
- Final gate (run ONCE, after Phase 3 — the landing tier for plan-65, the targeted gates of every area it touched): `xcodebuild … test` with `-only-testing:` for `BoardCutoutTests`, `CutoutModelTests`, `FreeFormOutlineTests`, `CutoutFileTests`, `CutoutPourTests`, `CutoutDesignRuleTests`, `CutoutRoutingTests`, `CutoutCanvasTests`, `CutoutPanelTests`, `CutoutExportTests`, `DXFReaderTests`, `DXFLoopTests`, `DXFImportTests`, `CutoutAutomationTests`, and the no-cutout classes `PourFillTests`, `PourConnectivityTests`, `PourDesignRuleTests`, `PourCanvasTests`, `DesignRuleCheckTests`, `DesignRuleGoldenTests`, `RoutingGridTests`, `RoutingVerifierTests`, `RoutingPlanTests`, `GerberExportTests`, `KiCadExportTests`, `EasyEDAProExportTests`, `PrintExportTests`, `BoardPreviewTests`, `PourFileTests`, `AutomationTests`, `KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 15 min; run with the Bash tool's `timeout` set to 3000 s so a hung host is killed). The full unit bundle (about 18 min) is not a plan step: it runs in the background per the project's policy. Observed (2026-11-25, 33 classes): `Executed 363 tests, with 0 failures`, `** TEST SUCCEEDED **`, 29 s.

## Decisions

- Cutouts: rectangle, circle, slot, and polygons from a DXF that are moved and deleted whole — the owner, 2026-11-25.
- DXF import: both modes, chosen in the import sheet — the owner, 2026-11-25.
- DRC: copper must clear a cutout, bodies may overhang — the owner, 2026-11-25.
- No `import_dxf` automation tool; the agent builds polygons with `add_cutout` — the author, 2026-11-25 (§ 1 non-goals).

## Corrections

- 2026-11-25: Phase 3 was not done through the live MCP tools or the real File menu click: see the two `[~]` lines. Evidence: `pgrep -fl schematical` → pid 62930, `~/Library/Developer/Xcode/DerivedData/schematical-*/Build/Products/Debug/schematical.app` (the owner's Xcode run), and `AutomationServer.port` is the fixed 47800; the plan did not know the owner's app would be running.
- 2026-11-25 (after the stop-hook check): a second copy of the merged build (`/private/tmp/claude-501/dd65/Build/Products/Debug/schematical.app`, pid 38705, started with `open -n`) was launched and read through System Events by pid: its File menu is `New, Open…, Open Recent, New Part…, Load Part…, User Library…, Close, Close All, Save, Duplicate, Save As…, Rename…, Move To…, Revert To, Import, Export, Print…`, so File ▸ Import is in the real app, between Revert To and Export; `DXF…` was disabled because the copy had no window (it was never made frontmost: `frontmost` stayed false after `set frontmost`, `open -a` and File ▸ New, and `count of windows` stayed 0). It was killed with `kill -9` at once; the owner's app (pid 62930, port 47800) was not touched. So the click on `DXF…`, the open panel and the sheet in a real window remain undriven; the substitute acceptance is the checks above: `CutoutPicturesTests.testTheFileMenuHasImportDXF` (the menu in the test host), the process check just described, `DXFImportTests` (state, placement, one undo step, errors, the sheet rendered), and the pictures.
- 2026-11-25 (second attempt after the stop hook): the second copy could not be made frontmost by any route (`osascript set frontmost`, `open -a`, `NSRunningApplication.activate` returned false), File ▸ New made no window by `click` or `AXPress`, and there is no `Quartz`/screen-capture tool in this session to see the screen. A test that attaches the sheet to an off-screen `NSWindow` in the test host did attach it (`window.attachedSheet` was set after `SchematicExporter.importDXF(from:)`, a split of the panel step), but sending it Return or Escape hung the host (22 s, then `Restarting after unexpected exit, crash, or test timeout`) and SwiftUI's accessibility tree is empty without assistive technology, so the Import button could not be pressed from a test either; that test and the split were reverted rather than committed half-working. The live click path (menu item → open panel → sheet → Import) stays undriven, and nothing in the app's behaviour is known to be wrong there.
- 2026-11-25 (third attempt): `SchematicExporter.runSheet` made internal and driven from the test host with a plain `Text` sheet on an off-screen window, and `importDXF(from:)` with a non-DXF file (the alert path): each killed the test host again (`Restarting after unexpected exit, crash, or test timeout`, no crash report), so any test that attaches a sheet or alert to a window and ends it hangs the host in this environment. Reverted; the substitute acceptance in the entry above stands.
- 2026-11-25: the plan's final-gate list is run as 33 test classes in one `xcodebuild` (the plan's list plus `CutoutPicturesTests`, which this phase added).
- 2026-11-25: `CutoutPicturesTests` is new (not in the plan): it holds the File-menu check and the env-gated picture dump the Phase 3 pictures came from.

## Summary

The real engineering risk of plan-65 is in G (byte-identical exports without cutouts) and C/D (the one `inside` rule that must differ for bodies and copper, and the stale pour-fill cache); J's job is to see it work. Left untouched: rounded corners of the rectangle (still in the todo), DXF export, polygon vertex editing, and an `import_dxf` tool.
