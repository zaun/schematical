# plan-63-F: The Electrical Rules Check window, the markers and the automation tool

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-63-E

Plan 63's overview and Prerequisites are in `planning/completed/plan-63-A-power-flag.md`.

The outcome of 63-F: **Window › Electrical Rules Check (⌥⌘E), on the Schematic
page, opens a window with two steps, as the Design Rule Check has.** Step 1 shows the
rules: a severity for each (error, warning, ignore), the 12×12 pin map, and a
reset to KiCad's defaults. Step 2 runs the check and lists one line per violation
(number, severity, rule, reference, message); it reruns as the drawing changes;
selecting a line selects what it names on the schematic and brings its point into
view; a violation can be excluded and put back. While the window is open each
violation is marked on the schematic (a red or yellow marker at its point, never
in an export). The automation tool `run_erc` returns the same list.

References:

- `planning/completed/plan-63-E-erc-checker.md` (`ErcViolation`, `ErcSettings`, the document API).
- `schematical/schematical/DesignRuleCheck/DesignRuleCheckWindow.swift` (the two-step
  window this mirrors), `Document/ChildWindows.swift`, `Document/EditorMenus.swift`,
  `Canvas/SchematicCanvasView.swift` (`worldLayer`), `Canvas/CanvasViewport.swift`,
  `Automation/AutomationRead.swift` (`run_drc`), `docs/agent-control.md`,
  `Help/KeyboardHelp.swift`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-63-E is complete | `grep -c 'func electricalRulesCheck' schematical/schematical/ElectricalRulesCheck/ErcChecker.swift` → 1 | MET (`1`, 2026-11-25) |
| The rest as plan-63-A § Prerequisites | its four commands | MET (2026-11-25) |

If plan-63-E is not complete, this plan cannot start, full stop.

## 1. Goal

- The menu item exists, is enabled on the Schematic page only, and opens one window per document.
- Step 1 edits `ErcSettings` through the document (one undo step per change); step 2
  lists `electricalRulesCheck()` and keeps up with edits.
- A selected line selects its items on the Schematic page and brings its point into view.
- Exclude / Include act on the selected lines; excluded violations are listed
  under a switch "Excluded" and are not marked.
- Markers show while the window is open, are cleared when it closes, and appear in no
  export (SVG, PNG, PDF, print).
- `run_erc` returns the violations one per line.

### Non-goals (explicit constraints)

- The check never blocks an edit, a simulation, a save or an export.
- The Design Rule Check window and its menu item are unchanged.
- No Settings ▸ ERC Defaults pane: a new document starts from KiCad's defaults; "Reset"
  in step 1 restores them (the DRC's app-wide defaults exist for fab manufacturers;
  ERC has none to name).
- No change to the PCB page.

## 2. Current State

The DRC window: `DesignRuleCheckCommands` (`DesignRuleCheckWindow.swift:19-45`, a
`Commands` with a menu button enabled on `editor?.page == .pcb`, shortcut ⇧⌘D);
`DesignRuleCheckWindowView` → `DesignRuleCheckWizard` with `showsResults`
(`:60-80`), step 1 `DesignRuleCheckRules` (a grouped `Form`, `Reset` and `Run Check`
buttons, `:85-150`), step 2 `DesignRuleCheckTable` (a `Table` of issues, `.task(id:
input)` rerunning after 150 ms, selection → `document.selectBoardItems`, copy
command, `:166-240`). It is opened through `ChildWindowCenter.open` in
`ChildWindowOpener` (`Document/ChildWindows.swift:113`, `DesignRuleCheckTarget`),
reached from `menus.openDesignRuleCheck` (`Document/EditorMenus.swift:31`, installed in
`ContentView.swift:350`), registered in `schematicalApp.swift:93`, and closed with
its document (`ContentView.swift:812`).

The canvas: `SchematicCanvasView.worldLayer` (`Canvas/SchematicCanvasView.swift:255-311`)
stacks layers in world coordinates, the last `PinMarkerLayer`; the export scene
(`Canvas/SchematicExportScene.swift`) is separate, so a layer added to
`worldLayer` is on screen only (as the gray loose end is: "Exports leave it false",
`:1087`). `CanvasViewport` (`Canvas/CanvasViewport.swift`): `world(_:)` is
`(screen − offset) / scale` (`:42`), `visibleWorldRect`, `pan(to:)`.

Selection: `select(_ item: SchematicItem)` and `schematicSelection.set(_:)`
(`SchematicModel.swift:1829`, `:1963`); the PCB's `selectBoardItems(_:)` (`:1958`) is
the precedent for a DRC line.

Automation: `run_drc` (`Automation/AutomationRead.swift:68`) returns `no issues` or
`<n> issues` and one line per issue; the MCP instructions (`AutomationTools.swift:344`)
name `run_drc` in the workflow; `docs/agent-control.md:59` lists the read tools.

### Measured populations

| What | Count | Command |
|---|---|---|
| Files that name `DesignRuleCheck` | 7 | `grep -rl 'DesignRuleCheck' schematical/schematical \| wc -l` → 7: `ContentView`, `DesignRuleCheck/DesignRuleCheck`, `DesignRuleCheck/DesignRuleCheckWindow`, `Document/ChildWindows`, `Document/EditorMenus`, `Routing/AutorouteSheet`, `schematicalApp` (the fab check in `SchematicExporter.swift:331` calls the lowercase `designRuleCheck`, which ERC does not touch) |
| Free key for the item | ⌥⌘E | `grep -rn 'keyboardShortcut("e"' schematical/schematical` → only `⇧⌘E` (`SchematicExportCommands.swift:17`) |

### Verified properties

- `⌥⌘` letters in use: P (`SimulationCommands.swift:60`), W (`SchematicNSDocument.swift:300`),
  R and U (`RouteCommands.swift:45,52`); E is free (above).
- The export path never reads `worldLayer` (`SchematicExportScene.swift` builds its
  own scene), so markers added to `worldLayer` cannot reach an export.

## 3. Design Overview

`ElectricalRulesCheck/ElectricalRulesCheckWindow.swift` mirrors
`DesignRuleCheckWindow.swift`: `ElectricalRulesCheckTarget` (document id),
`ElectricalRulesCheckCommands`, `ElectricalRulesCheckWindowView`, the wizard, the rules
step, the results step.

The logic that is not drawing lives in `ErcResultsModel` (a plain `@MainActor`
class the table owns; tests drive it): runs the check, holds the list and the
excluded list, maps selected rows to items and a point, applies exclude/include,
and writes `document.ercMarkers`.

The rules step: a `Form` section per group of rules (Pins, Connections, Labels,
Other) with a `Picker` per rule over error / warning / ignore (the `pin_to_pin`
row reads "By pin map"); a pin map grid, twelve by twelve, a button per cell that
cycles ok → warning → error and writes the cell and its mirror (the no_connect row and
column are drawn and disabled: `pin_to_pin` never reads them, README); buttons "Reset
to KiCad Defaults" (settings and pin map; exclusions stay) and "Run Check".

Markers: `SchematicDocument.ercMarkers: [ErcMarker]` (point, severity, id of the
violation), editor state like the selection: not in `DocumentState`, not saved, no
undo. `ErcMarkerLayer(markers:, area:)` in `worldLayer` after `PinMarkerLayer`: a
triangle 14 units wide in `Color.red` (error) or `Color.yellow` (warning), stroked dark.
`document.ercFocus: ErcFocus?` (point, a counter) is set by a selected row;
`SchematicCanvasView` observes it and, when the point is outside
`viewport.visibleWorldRect` inset by 40, calls `viewport.pan(to:)` so the point is at
the view's centre: `offset = size/2 − point × scale`.

Risk: low and in the UI's wiring: a leaked marker after the window closes (the
results model clears `ercMarkers` in `onDisappear` and when the document closes),
and the `pin map` editing keeping the map symmetric (a test). Byte-identity is not
a gate; the one invariant is "no export changes", tested by comparing an SVG with
markers present to one without.

Rejected: markers that outlive the window (the list reruns only while the window is
open, so they would go stale); a sidebar panel instead of a window (the DRC is a
window, and the owner chose parity); drawing markers into the export scene.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` with one line on what remains; moot tasks keep their line
> as `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` as a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — The results model, the window and the menu

- [x] New `ElectricalRulesCheck/ErcResultsModel.swift`: `violations`, `excluded`,
      `selection`, `run()` (the document's `electricalRulesCheck()` split by
      exclusion), `exclude(rows:)`, `include(rows:)` (both through the document's
      `excludeErc` / `includeErc`), `items(of:)` → the union of the rows' items and the
      first row's point.
- [x] New `ElectricalRulesCheck/ElectricalRulesCheckWindow.swift` as § 3:
      menu command `Button("Electrical Rules Check")` with
      `.keyboardShortcut("e", modifiers: [.command, .option])`, disabled unless
      `editor?.page == .schematic`; the wizard; step 1 (severity pickers, pin map
      grid, reset, Run Check); step 2 (a `Table`: #, Severity, Rule, Reference,
      Message; a segmented switch Violations / Excluded (n); toolbar Exclude / Include;
      copy as the DRC's; `.task(id:)` rerun 150 ms after an edit; the subtitle
      `Step 2 of 2 · n errors, m warnings`; the empty state `No Violations`).
      A row's reference is the part and pin ("U1.14"), the label name, or "Wire".
- [x] `Document/ChildWindows.swift`: a `case let target as ElectricalRulesCheckTarget`
      beside the DRC's (title "Electrical Rules Check", 760×440). `Document/EditorMenus.swift`:
      `openElectricalRulesCheck`, cleared in `removeAll()`. `ContentView.swift`:
      install it beside `openDesignRuleCheck` (`:350`), an `isElectricalRulesCheckOpen`
      state and its dismissal where `isDesignRuleCheckOpen` is dismissed (`:812`).
      `schematicalApp.swift`: `ElectricalRulesCheckCommands()` beside
      `DesignRuleCheckCommands()` (`:93`).
- [x] `Canvas/SchematicModel.swift`: `selectSchematicItems(_ items: Set<SchematicItem>)`
      beside `selectBoardItems` (`:1958`).
- [x] `Help/KeyboardHelp.swift`: `entry("⌥⌘E", "Electrical Rules Check")` beside
      `entry("⇧⌘B", "Bill of Materials")` (`:60`, the Schematic's window commands; the
      DRC's own line is in the PCB help's Checks section, `:171`).
- [x] Tests (new `schematicalTests/ErcResultsModelTests.swift`): `run()` lists and
      numbers; exclude moves a row to the excluded list and survives an unrelated edit;
      include puts it back; selecting rows selects exactly their items; step 1's edits
      reach `document.erc` as undo steps; the pin map cell edit writes both halves;
      reset restores defaults and keeps exclusions. `KeyboardHelpTests` still passes.

Acceptance: the window opens from the menu on the Schematic page, lists, selects, excludes.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/ErcResultsModelTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/DesignRuleCheckTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 8 min).
  Result (2026-11-25, in the worktree): `ErcResultsModelTests` 7, `KeyboardHelpTests` 6, `DesignRuleCheckTests` 17: `Executed 30 tests, with 0 failures`, `** TEST SUCCEEDED **`; the window, the menu command and the child-window wiring compile in the app target. The real-window pass (⌥⌘E on the Schematic page) is the owner's (plan-63-G Validation Plan).
Commit: c92ee10c

### Phase 2 — Markers and bringing a line into view

- [x] `Canvas/SchematicModel.swift`: `ErcMarker`, `var ercMarkers: [ErcMarker]` and
      `var ercFocus: ErcFocus?` (editor state, not in `DocumentState`).
      `ErcResultsModel` writes the markers of the non-excluded violations after each
      run and clears them (and the focus) in `deinit`/`onDisappear` of the window
      and when the document closes (`SchematicNSDocument.close()` already closes child
      windows, `ChildWindows.swift`: the window's disappearance clears them).
- [x] New `Canvas/ErcMarkerLayer.swift` and its place in `SchematicCanvasView.worldLayer`
      after `PinMarkerLayer` (`:301`); `SchematicCanvasView` handles `ercFocus` with
      `viewport.pan(to:)` as § 3 says (a `CanvasViewport.centre(on:)` helper, tested).
- [x] Tests (new `schematicalTests/ErcMarkerTests.swift`): markers follow the results
      and are empty when the model is released; a violation's marker point is the pin's tip;
      `CanvasViewport.centre(on:)` puts the point at the view's centre
      (`world(size/2)` equals the point); `SchematicExport.svg(SchematicScene(doc))` (as `SchematicSVGTests.svgText`) of a
      drawing with markers equals that of the same drawing without.

Acceptance: markers appear for each shown violation, clear with the window, and never reach an export.
  Check: `xcodebuild test … -only-testing:schematicalTests/ErcMarkerTests -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/SchematicSceneTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 8 min).
  Result (2026-11-25): `ErcMarkerTests` 6, `ErcResultsModelTests` 7, `SchematicSVGTests` 6, `SchematicSceneTests` 9: `Executed 28 tests, with 0 failures`, `** TEST SUCCEEDED **`. "Empty when the model is released" is `clearMarkers()` (a model cannot touch the document from `deinit`); the window calls it on `onDisappear`, which runs when the child window closes (`ChildWindowCenter.forget` replaces its root view). A marker is 7 units, 11 when its line is selected.
Commit: c0e91c98

### Phase 3 — Automation and docs

- [x] `Automation/AutomationRead.swift`: `run_erc` beside `run_drc` (read-only):
      `no violations`, or `<n> violations` and one line each, `<i>. [<severity>]
      [<rule>] <reference>: <message>`; honours severities and exclusions. A
      `schematic` mention in `Automation/AutomationTools.swift:344` workflow text:
      "... check → run_erc → ... run_drc". `docs/agent-control.md:59`: `run_erc`.
- [x] Tests in `AutomationTests`: `run_erc` on an empty document says `no violations`;
      on a drawing with an unwired input it returns `1 violations` and the line with
      `pin_not_connected`.
- [x] Docs: `docs/agent-control.md` lists `run_erc` (the ERC's own rules are in `docs/file-format.md`, 63-E, and `docs/backend.md`, 63-B and 63-D; no other doc describes menus).

Acceptance: `run_erc` returns the list the window shows.
  Check: `xcodebuild test … -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/AutomationWindowToolsTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 7 min).
  Result (2026-11-25): `AutomationTests` 34 (1 skipped: `testFileToolsAreOffWithoutASharedFolder`, a folder is shared on this Mac) and `AutomationWindowToolsTests` 5: `Executed 39 tests, with 1 test skipped and 0 failures`, `** TEST SUCCEEDED **`; the new `testRunErcListsTheViolations`: an empty document says `no violations`, a placed `SN74HC00N` lists `[error] [pin_not_connected] U1.1: Pin not connected`, an ignored rule is gone.
Commit: BATCHF

## Validation Plan

- Tests: `ErcResultsModelTests`, `ErcMarkerTests` (new); `AutomationTests`; `KeyboardHelpTests`; `DesignRuleCheckTests` (unchanged behavior).
- Coverage check: `grep -rl 'ElectricalRulesCheck' schematical/schematical` lists `ContentView`, `Document/ChildWindows`, `Document/EditorMenus`, `schematicalApp` and the new folder's files (the wiring files of § 2 that the DRC also has, except `AutorouteSheet`).
- Runtime proof: in the real app (the owner's pass, per AGENTS.md "Testing in the app": move the pointer to (0, 0) afterwards): open `examples/8bit-cpu/AFU.schematical` (a real file, in the app), ⌥⌘E, run, select a line and see the marker and the selection, exclude it, close the window and see the markers go.
- Doc sync: `docs/agent-control.md`.
- Final gate (once, after Phase 3): the three Phase checks together (est. 20 min). The landing gates of the whole plan are 63-G's. Result (2026-11-25): all eight classes together: `Executed 90 tests, with 1 test skipped and 0 failures`, `** TEST SUCCEEDED **`. Result (2026-11-25): all eight classes together: `Executed 90 tests, with 1 test skipped and 0 failures`, `** TEST SUCCEEDED **`.

## Decisions

- Full parity: severities, the pin map, exclusions, markers — decided by the owner, 2026-11-25.
- Markers show only while the window is open — decided by the author, 2026-11-25: the list reruns only while it is open.
- The shortcut is ⌥⌘E — decided by the author, 2026-11-25 (free, § 2).
- No app-wide ERC defaults — decided by the author, 2026-11-25 (§ 1 non-goals).

## Corrections

1. § 2 counted 7 files that name `DesignRuleCheck`; the ERC adds the same wiring in `ContentView`, `Document/ChildWindows`, `Document/EditorMenus` and `schematicalApp` and no `AutorouteSheet` equivalent (the autorouter opens the DRC; nothing opens the ERC). The window's file is `ElectricalRulesCheck/ElectricalRulesCheckWindow.swift` and its model `ErcResultsModel.swift`, in a new `ElectricalRulesCheck` folder, as 63-E's `ErcSettings.swift` and `ErcChecker.swift`.
2. Phase 2 added `ErcMarker` / `ErcFocus` types to `Canvas/ErcMarkerLayer.swift` and `ercSelectedKeys` to the document (the selected lines' keys, so their marks are drawn larger), which § 3 did not list. `CanvasViewport` got `centre(on:)` and `reveal(_:margin:)` (a point more than 40 screen points inside the view stays where it is).
3. The "markers are empty when the model is released" test is `clearMarkers()` called by the window's `onDisappear`: a model's `deinit` is nonisolated and cannot clear the document's marks. If the window is closed without `onDisappear` firing, the marks stay until the window is opened and closed again; `ChildWindowCenter.forget` replaces the hosted root view, which does run `onDisappear` (read at `Document/ChildWindows.swift`, `forget`).
4. Phase 3's `run_erc` prints `<n> violations` even for 1 (as `run_drc` prints `<n> issues`).
5. The window itself was compiled and its model, markers and menu wiring tested; it was not opened in a real window by this plan's executor. The owner's real-window pass is plan-63-G's Validation Plan.

## Summary

The risk is wiring: the files where the DRC is plugged in, and a marker that must
not leak into exports or outlive the window. Untouched: the DRC, the PCB page, every export.
