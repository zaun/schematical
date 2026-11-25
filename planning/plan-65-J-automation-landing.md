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
| plan-65-I is complete | `ls planning/completed \| grep -c '^plan-65-I'` → 1 | NOT MET (2026-11-25: I is written, not landed) |

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
- `todo.md:115`: `- [ ] Custom board outlines: rounded corners, cutouts, DXF import.`

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

- [ ] `Automation/AutomationBoard.swift`: `add_cutout`, `set_cutout`, their functions and `describe(_ cutout:)`; `set_board`'s `outline` and `rectangle` and the new "nothing to change" text.
- [ ] `Automation/AutomationRead.swift`: `outline=free-form` on the `BOARD` line and the `CUTOUTS` block after `KEEPOUTS`.
- [ ] `Automation/AutomationTools.swift`: the PCB paragraph of `AutomationGuide.text` names `add_cutout` and the free-form outline.
- [ ] Tests `schematicalTests/CutoutAutomationTests.swift` (the `call` pattern of `AutomationTests`): each shape through `add_cutout` and the kind it reports; a circle's centre; a bad shape (side 0.1) is an error naming the 0.5 mm minimum; `set_cutout` of x/y/width/radius/locked and the locked refusal; a polygon accepts `x`, `y`, `locked` and refuses `width`; `get_board` lists them and the `outline=free-form` marker; `set_board outline` with an L-shape sets width/height to its bounds and `rectangle: true` resets; `delete cutout:ID`; `run_drc` on a board with a pad in a cutout reports `Copper is in a cutout`; `AutomationTests` passes unedited.

Acceptance: an agent can make, change, list and delete cutouts and set the outline, in one undo step per call.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutAutomationTests -only-testing:schematicalTests/AutomationTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min; `AutomationTests` is the no-cutout check).
Commit: —

### Phase 2 — Docs and the todo

- [ ] `docs/agent-control.md`: the PCB list (`:75`–`:81`) names `add_cutout`, `set_cutout` and `set_board`'s `outline`; `get_board` lists `CUTOUTS`; ids gain `cutout:ID` (`:117`).
- [ ] `docs/export.md`: a `## Import` section: File ▸ Import ▸ DXF… (modes, layers, units, the origin offset, what is read and what is skipped, that a binary DXF is refused), and § Gerber/KiCad/EasyEDA already say cutouts (G). `docs/routing.md`: a paragraph that cutouts are edges (D's doc line). `docs/file-format.md` is done in B and C.
- [ ] `planning/todo.md` § PCB: replace `- [ ] Custom board outlines: rounded corners, cutouts, DXF import.` with `- [x] Board cutouts (rectangle, circle, slot, polygon), a free-form board outline and File ▸ Import ▸ DXF… (plan-65).` and a new `- [ ] Rounded corners of the board rectangle.`

Acceptance: every tool and menu item the plan added is named in the docs, and no doc line says a cutout or the DXF import does not exist.
  Check: `rg -n "add_cutout\|set_cutout" docs/agent-control.md schematical/schematical/Automation/AutomationTools.swift` → each file lists both; `rg -n "Import ▸ DXF\|Import › DXF" docs/export.md` → a heading line; `rg -n "plan-65" planning/todo.md` → the ticked line (est. 1 min).
Commit: —

### Phase 3 — The real app, then the landing gates

- [ ] Build the Debug app (`xcodebuild … build`), start it with an empty untitled window, and drive it with the live automation tools (not the owner's open documents): `set_board` 60 × 40, `place_part` a through-hole part, `add_cutout` a 6 mm circle, a 10 × 2 slot and a rectangle, `render` the PCB (front) and the board preview (angled) and look at both pictures (holes visible, the preview's hole wall visible); `run_drc` → no issue for a part whose body overhangs a cutout, `Copper is in a cutout` after moving a pad into one; `add_pour` on GND with a net → the render's fill has a hole round each cutout; `export_gerber` → the zip's `.GKO` has 4 loops; `undo` once per edit.
- [ ] Through the real File menu: File ▸ Import ▸ DXF… with `planning/plan-65-data/kicad_edge_probe.dxf` in *Board outline and cutouts* mode: the sheet's preview shows the 40 × 30 outline in the accent colour and the circle and half disc in red; Import; the PCB page shows the 40 × 30 board with the two holes; ⌘Z once restores the previous board. Then *Cutouts only* with a fresh board: one cutout (the rectangle) and `2 inside a cutout` ignored. Screenshots of the sheet and the PCB page are looked at; the app is killed at the end of each command; no dialog is left up; the pointer is moved to (0, 0) at the end (`AGENTS.md`).
- [ ] Autoroute a board with one cutout between two pads through `route`/the autoroute tool the tools expose (or `Route` in the window) and look at the picture: the trace goes round the cutout.
- [ ] Record in this phase's Commit line: what was driven, the pictures' findings in one line each, and any fix made (a fix is its own commit with a test).

Acceptance: the pictures show what the tests assert; any defect found is fixed with a test before the landing gate.
  Check: the three tasks' observed results, written in the Commit line (est. 40 min of driving; no smaller check covers the real window and the real File menu, which no test reaches).
Commit: —

## Validation Plan

- Tests: `CutoutAutomationTests` (positive and the error cases above).
- Coverage check: `AutomationTests` runs the same `AutomationBoard`/`AutomationRead` paths without cutouts.
- Runtime proof: Phase 3.
- Doc sync: Phase 2 (`docs/agent-control.md`, `docs/export.md`, `docs/routing.md`, the todo); the keyboard help was E's (`Help/` has only `KeyboardHelp.swift` and `Acknowledgements.swift`).
- Final gate (run ONCE, after Phase 3 — the landing tier for plan-65, the targeted gates of every area it touched): `xcodebuild … test` with `-only-testing:` for `BoardCutoutTests`, `CutoutModelTests`, `FreeFormOutlineTests`, `CutoutFileTests`, `CutoutPourTests`, `CutoutDesignRuleTests`, `CutoutRoutingTests`, `CutoutCanvasTests`, `CutoutPanelTests`, `CutoutExportTests`, `DXFReaderTests`, `DXFLoopTests`, `DXFImportTests`, `CutoutAutomationTests`, and the no-cutout classes `PourFillTests`, `PourConnectivityTests`, `PourDesignRuleTests`, `PourCanvasTests`, `DesignRuleCheckTests`, `DesignRuleGoldenTests`, `RoutingGridTests`, `RoutingVerifierTests`, `RoutingPlanTests`, `GerberExportTests`, `KiCadExportTests`, `EasyEDAProExportTests`, `PrintExportTests`, `BoardPreviewTests`, `PourFileTests`, `AutomationTests`, `KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 15 min; run under `timeout 3000` so a hung host is killed). The full unit bundle (about 18 min) is not a plan step: it runs in the background per the project's policy.

## Decisions

- Cutouts: rectangle, circle, slot, and polygons from a DXF that are moved and deleted whole — the owner, 2026-11-25.
- DXF import: both modes, chosen in the import sheet — the owner, 2026-11-25.
- DRC: copper must clear a cutout, bodies may overhang — the owner, 2026-11-25.
- No `import_dxf` automation tool; the agent builds polygons with `add_cutout` — the author, 2026-11-25 (§ 1 non-goals).

## Corrections

<Filled in during execution.>

## Summary

The real engineering risk of plan-65 is in G (byte-identical exports without cutouts) and C/D (the one `inside` rule that must differ for bodies and copper, and the stale pour-fill cache); J's job is to see it work. Left untouched: rounded corners of the rectangle (still in the todo), DXF export, polygon vertex editing, and an `import_dxf` tool.
