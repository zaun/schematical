# plan-38-K: PCB UI — the Autoroute sheet, Route menu, Route tool, live preview, results

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-J

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F, G, H, I, J) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The user-facing feature. A **Route** menu (PCB page only), an **Autoroute…**
sheet with every option of the plan — which nets (all, selected nets and parts,
only nets with no copper yet), what to do with existing copper (keep, clear
autorouted, clear all unlocked), which layers and their directions, vias and
plane fan-out, trace width and clearance overrides, strategy, grid, angle,
time limit, length groups, pairs — a progress view with a live preview of the
copper being laid, **Stop** and **Cancel**, and a result list that names every net
it could not finish. Plus a **Route tool** (click a pad to route its net, a
ratsnest line to route that one connection) and Route / Unroute on each row of
the Nets list.

References:

- plan-38-D (`RoutingOptions`, scope), plan-38-G (`RoutingSession`, `unroute`,
  `RoutingScope.fromSelection`), plan-38-C (Nets section, `highlightedNet`),
  plan-38-I/J (`GroupOutcome`).
- `ContentView.swift` (`.focusedSceneValue(\.openDesignRuleCheck, …)` at `:675`,
  the PCB toolbar at `:480-540`), `DesignRuleCheck/DesignRuleCheckWindow.swift:
  DesignRuleCheckCommands`, `schematicalApp.swift` (the `Commands` list at
  `:50-56`), `Document/ImageExportDialog.swift` (an options sheet), `Canvas/
  PCBCanvasView.swift` (`PCBTool` at `:781`, `drawRatsnest`), `Help/KeyboardHelp.swift`,
  `Preferences/PreferencesView.swift:PreferenceKey`.

## 1. Goal

- On the PCB page: **Route ▸ Autoroute…** (⌥⌘R) opens the sheet; **Route Selected
  Nets** (⌥⇧⌘R) routes the selection's nets with the last-used options, no sheet;
  **Unroute Selected** (⌥⌘U) and **Unroute All Autorouted** delete copper
  (plan-38-G); **Select Autorouted Copper** selects every `auto` trace and via.
  The menu is disabled off the PCB page and while the simulation plays.
- The sheet's options are `RoutingOptions` (plan-38-D) and are remembered between
  runs (all but the scope); it states, live, "N nets, M connections to route" and,
  for the chosen clear option, "Removes X traces and Y vias".
- Routing runs in the background with a progress view (round, routed / total,
  unrouted) and a live preview of the copper on the canvas; **Stop** applies what
  is routed, **Cancel** applies nothing; the apply is one Undo ("Autoroute").
- A finished run lists each net not completed, with its reason; clicking one
  highlights it (plan-38-C) and a **Design Rule Check** button opens the DRC window.
- The **Route tool** routes a clicked pad's net, or a clicked ratsnest line's one
  connection, with the last-used options.

### Non-goals (explicit constraints)

- No new engine behavior (D–J); no new file format; no change to the Trace tool
  (plan-38-L).
- The existing menus, shortcuts and tools keep their keys (⌘R and ⇧⌘R rotate; ⇧⌘D
  is the DRC).
- The sheet never starts a run on a board that is playing the simulation.
- The schematic page shows no Route menu items.

## 2. Current State

- The DRC window is opened through a focused scene value set only on the PCB page
  (`ContentView.swift:675`) and a `Commands` type in the app's command list
  (`DesignRuleCheckWindow.swift:DesignRuleCheckCommands`, `schematicalApp.swift:52`).
- An options sheet exists as a pattern: `ImageExportDialog` (a `View` with
  `@State options` and Cancel / Export closures).
- The PCB toolbar is a segmented `Picker` over `PCBTool.allCases` plus the Trace
  layer menu, Rotate, Preview and the layer popover (`ContentView.swift`).
- The canvas draws the ratsnest and the in-progress trace (`PCBCanvasView.draw`,
  `drawRatsnest`); a `Canvas`-based view with `visible: BoardVisibility`.
- Preferences keys are strings in `PreferenceKey` with `@AppStorage`.
- Shortcuts in use (`grep -rhn keyboardShortcut schematical/schematical`): ⌘- ⌘0
  ⌘= ⌘? ⌘A ⇧⌘B ⌘C ⇧⌘D ⇧⌘E ⌘F ⌘N ⇧⌘N ⌘O ⌘R ⇧⌘R ⌘S ⇧⌘S ⌘V ⌘X ⌘Z ⇧⌘Z ⌘⌫; none uses ⌥,
  so ⌥⌘R, ⌥⇧⌘R and ⌥⌘U are free.

### Measured populations

| What | Count | Command |
|---|---|---|
| Distinct `keyboardShortcut` forms in the app | 24 (incl. `.cancelAction`, `.defaultAction`) | `grep -rhn "keyboardShortcut" schematical/schematical \| sed 's/^[0-9]*: *//' \| sort -u \| wc -l` → 24 |
| `PCBTool` cases | 6 (`select, text, trace, via, hole, delete`) | `Canvas/PCBCanvasView.swift:781`; this letter adds `route` (7) |

### Verified properties

- A `Commands` type reads a `FocusedValue` set by the key window and disables its
  items when it is nil (`DesignRuleCheckCommands`), so a new `RouteCommands` with a
  `\.routeActions` value set only when `page == .pcb` is off the schematic page
  without further code.

## 3. Design Overview

- `Routing/AutorouteModel.swift` (`@MainActor @Observable`, testable without a view):
  `options: RoutingOptions` (loaded from / saved to `PreferenceKey.autorouteOptions`,
  JSON, scope excluded), `scopeChoice` (`.all`, `.selection`, `.nets([PlaneNet])`,
  `.footprints`), `problemSummary` (computed by a debounced
  `document.routingProblem(options:)` in a `.task(id:)`: nets, connections,
  traces/vias removed, impossible nets), `canRoute`, `layerRows`, `useAlternatingDirections()`
  (Top horizontal, Bottom vertical, inner layers alternating by stack order),
  `session: RoutingSession`.
- `Routing/AutorouteSheet.swift`: the sheet of § 1 in `Form` sections — Route,
  Existing copper, Layers (a row per copper layer: toggle, direction menu; Plane
  layers shown as "Plane (fan-out only)"), Vias, Rules (Default class values read
  from the DRC rules, class list, width/clearance overrides), Quality (Strategy,
  Grid, Angle, Time limit), Groups (Tune length groups, Route pairs coupled).
  While `session.state` is `running` the sheet shows the progress view instead; when
  `finished` it shows the result list.
- `RouteCommands` + `\.routeActions` (a struct of closures, as `EditActions`),
  set in `ContentView` for `page == .pcb`; items per § 1; the sheet is a
  `.sheet(isPresented:)` bound to `@State isAutorouteOpen`.
- The live preview: `RoutingSession.preview: (traces, vias)?`, drawn by
  `PCBCanvasView` (`var routePreview`) at 60 % alpha in each layer's colour, under the
  selection, never saved.
- The **Route tool** (`PCBTool.route`, toolbar symbol `point.topleft.down.curvedto.point.bottomright.up`,
  tooltip "Route — click a pad to route its net, a ratsnest line to route that one
  connection"): a click is hit-tested to a pad (the first of `connectivity().pads` whose
  `pad.rect` contains the point and whose layer is shown, as `traceAnchor` finds a pad) or
  within 6 screen points of a ratsnest line (`TraceDrawing.tolerance`); it starts a session with scope
  `.nets([that pad's net])` or `.connection(from:to:)` and the saved options; the
  sidebar's Nets rows get Route / Unroute / Highlight context-menu items that do the same.
- A finished run's `GroupOutcome`s are listed under the net results (status text).

Where correctness risk concentrates: the session/sheet lifecycle (closing the
window mid-run, Esc, starting twice) — covered by the model's tests; the rest is
layout. No byte-identity; the model's logic is the gate, the real app the proof.

Rejected: a separate window like the DRC (a run belongs to one document and must
block its edits — a sheet does); applying options live while the sheet is open
(a run is a deliberate act); a global Settings page for router options (they are
per-run choices, remembered, not preferences).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — the model

- [ ] `Preferences/PreferencesView.swift`: `PreferenceKey.autorouteOptions`.
- [ ] `Routing/AutorouteModel.swift`: the model of § 3 (load/save, scope choices, summary, layer rows, alternating directions, `route()` → `session.start`, `stop()`, `cancel()`).
- [ ] Tests: `schematicalTests/AutorouteModelTests.swift` — options save and reload equal with the scope excluded; with a selected footprint `scopeChoice.selection` resolves to `.footprints`; on AFU the summary says 53 nets / 115 connections for `.all` and 0 for `.untouchedNets` after one route; `.clearAuto` summary counts match `removedTraces`/`removedVias`; a 1-layer board offers no Vias section values (vias `.none`); `useAlternatingDirections` on 2 and 4 layers; `route()` while `isPlaying` does nothing and sets `editorMessage`.

Acceptance: cases pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/AutorouteModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 2 — sheet, menu, toolbar, preview, results

- [ ] `Routing/AutorouteSheet.swift`: the sheet (idle / running / finished states), keyboard: Return = Route, Esc = Cancel (when running: Esc = Cancel run).
- [ ] `Routing/RouteCommands.swift`: `RouteCommands`, `\.routeActions`; registered in `schematicalApp.swift` after `DesignRuleCheckCommands()`.
- [ ] `ContentView.swift`: `@State isAutorouteOpen`, the sheet, `.focusedSceneValue(\.routeActions, page == .pcb ? … : nil)`, toolbar button "Autoroute" next to Preview.
- [ ] `PCBCanvasView.swift`: `routePreview` drawing.
- [ ] Results list: failed nets with reasons, click → `highlightedNet`; Design Rule Check button → `openWindow(id: "drc", …)`.
- [ ] Tests (`AutorouteModelTests`): the results list for an LFU run names every unrouted net once with a non-empty reason and its count; the menu actions struct is nil for `.schematic`.

Acceptance: build succeeds; model tests pass.
  Check: `-only-testing:schematicalTests/AutorouteModelTests` → `** TEST SUCCEEDED **` (est. 3 min; includes a full LFU route at ≈ 10 s).
Commit: —

### Phase 3 — Route tool and Nets-row actions

- [ ] `PCBCanvasView.swift`: `PCBTool.route` (title, tooltip, symbol), click handling per § 3; `Interaction.idle` after the click; the toolbar picker picks it up (`allCases`).
- [ ] `PCBSidebarView.swift` `NetsSection`: row context menu Route Net, Unroute Net, Highlight.
- [ ] Tests: `schematicalTests/RouteToolTests.swift` — the tool's hit-test (pad → its net's scope; ratsnest line → `.connection`; empty space → nothing); the context-menu actions call the session with the row's `PlaneNet` scope.

Acceptance: cases pass; the 7 `TraceToolTests` stay green.
  Check: `-only-testing:schematicalTests/RouteToolTests -only-testing:schematicalTests/TraceToolTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 4 — help and the real app

- [ ] `Help/KeyboardHelp.swift` `pcb`: a "Route" section (⌥⌘R, ⌥⇧⌘R, ⌥⌘U, the Route tool) and a line under Tools; `KeyboardHelpTests` updated.
- [ ] Real app (per `AGENTS.md`, pointer left at 0,0 afterwards): build `schematical` Debug, open `schematical/schematicalTests/Fixtures/AFU.schematical`, PCB page; Route ▸ Autoroute… → Route; observe that the preview draws, the list ends with no failed nets, the canvas shows traces, and the DRC window shows no `Unrouted` lines; Edit ▸ Undo removes all of it; repeat with Stop pressed mid-run (some copper stays) and with Cancel (none). Record observations here.
- [ ] Observations (filled in by the run): __.

Acceptance: help tests pass; the real-app run behaves as stated.
  Check: `-only-testing:schematicalTests/KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 2 min); plus the real-app run above (est. 15 min).
Commit: —

## Validation Plan

- Tests: `AutorouteModelTests`, `RouteToolTests`, `KeyboardHelpTests`, `TraceToolTests`.
- Coverage check: `AutorouteModel` and the tool's hit-test are covered; the views are covered by the real-app run.
- Runtime proof: Phase 4.
- Doc sync: `Help/KeyboardHelp.swift`; `docs/routing.md` is written in plan-38-M.
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: a sheet, not a window; options persist except scope; shortcuts ⌥⌘R, ⌥⇧⌘R, ⌥⌘U; Stop applies, Cancel discards; the Route tool is a seventh `PCBTool`.

## Corrections

(Filled in during execution.)

## Summary

Most of the code is layout; the risk is the run lifecycle (a closed window, a
second start), which the model's tests pin. Untouched: the engine, the file
format, the Trace tool.
