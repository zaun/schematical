# plan-38-C: PCB sidebar — Nets section (classes, layer rules, groups); Trace and Via tools use the net's class

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-B

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The first letter the user sees. The PCB sidebar, with nothing selected, gains a
**Nets** section: every net on the board (name, pads, unrouted connections),
each with a class menu and a layer-rule menu; a **Net Classes** editor; a **Net
Groups** editor. Clicking a net highlights its copper on the canvas. The Trace
and Via tools take their width and via size from the class of the net they start
on; a selected trace shows its net and class. Nothing routes.

References:

- plan-38-A (§ 3, § Decisions), plan-38-B (§ 3).
- `Canvas/PCBSidebarView.swift` (`PCBSidebarView.body` `.board` case at the end
  of `body`; `PlanesSection`; `TracePanel`; `ViaPanel`), `Canvas/LengthField.swift`,
  `Canvas/PCBCanvasView.swift` (`TraceDrawing`, the `drawing` trace in the
  ratsnest/preview code), `DesignRuleCheck/DesignRuleCheckWindow.swift`
  (`DesignRuleCheckInput`, the debounced `.task` that reruns a check).

## 1. Goal

- With nothing selected on the PCB page, the sidebar lists: Grid, Board, Layers,
  Planes (as today), then **Nets**, **Net Classes**, **Net Groups**.
- A net row shows its name, its pad count and how many connections are unrouted;
  its menus set its class and its layer rule; a click highlights the net's pads,
  traces, vias and ratsnest lines on the canvas and dims the rest; a second click
  or Esc clears it.
- Classes and groups are created, renamed, edited and deleted from the sidebar,
  each edit one undo step (plan-38-A/B edit functions).
- A trace drawn from a pad of a net in class *Power* (20 mil) is 20 mil; a via
  placed on such a net has that class's via size; with no class, 10 mil and
  0.4 / 0.8 mm as before.

### Non-goals (explicit constraints)

- No routing, no Autoroute control (K). No lengths shown (I adds them).
- The Trace tool's drawing keys and the existing `PCBTool` cases do not change.
- A trace already on the board keeps its width when its net's class changes
  (class width applies to new copper and to the router; the sidebar's Width
  field on a selected trace still sets that trace).
- Nothing in the Schematic page changes.

## 2. Current State

- The board inspector is a SwiftUI `Form`; with no selection it builds
  `GridSection`, `BoardPanel`, optional card sections, `LayersSection`,
  `PlanesSection` (`PCBSidebarView.swift`, the `.board` case).
- `TracePanel` shows Width, Layer, Lock, "Connects" (`PCBSidebarView.swift:TracePanel`).
- New traces: `TraceDrawing.finish` → `SchematicDocument.addTrace(points:layer:)`
  with `BoardTrace.defaultWidth` (`PCBCanvasView.swift:finish`,
  `SchematicModel.swift:2952`); the live preview trace is built with the same
  constant (`PCBCanvasView.swift:449`). New vias: `addVia(at:)` (`:3226`).
- A check or list that needs connectivity recomputes it on a debounce, from an
  `…Input: Hashable` struct, in `.task(id:)` (`DesignRuleCheckWindow.swift`,
  `DesignRuleCheckTable`).
- `connectivity()` is `O(pads × traces)` for joins (`Ratsnest.swift`); the DRC
  window already runs it per edit, so a debounced run in the sidebar is the
  existing pattern's cost.

### Measured populations

| What | Count | Command |
|---|---|---|
| Nets that need a row on the biggest example | 59 (LFU), 54 (AFU) | `planning/plan-38-data/spike/results.txt` (nets with a pad) |
| Test files for sidebars | `SelectionSidebarTests.swift`, `PlaneNetTests.swift` | `ls schematical/schematicalTests \| grep -i -E "sidebar\|PlaneNet"` |

### Verified properties

- `BoardConnectivity` can name a net by a stable key: rails by `railNets`
  (plan-38-A), plain nets by the first pad's `(chip UUID, designator)`; the same
  key a Plane's `net` and `NetClass.members` use (`PlaneNet`).

## 3. Design Overview

- `Canvas/NetList.swift` (new): `NetRow { net: Int, key: PlaneNet, name: String,
  pads: Int, unrouted: Int, classID: UUID?, layerRule: LayerRule?, groupID: UUID? }`
  and `SchematicDocument.netRows() -> [NetRow]`, built from one `connectivity()`
  (`netNames`, `netGroups()` for unrouted counts, plan-38-A/B resolution).
  `BoardConnectivity.planeNet(ofNet:) -> PlaneNet?` gives the key: `.rail(mv)` for a
  rail net, else the `.pin` of the net's first pad by chip index then designator.
- `NetListInput: Hashable` (chips, wires, buses, labels, board, traces, vias) and a
  debounced `.task(id:)` in the Nets section, as `DesignRuleCheckTable`.
- Highlight: a per-window `@State highlightedNet: PlaneNet?` owned by the PCB
  content view and passed to `PCBCanvasView` (`var highlightedNet: PlaneNet?`);
  `draw` strokes the net's pads, traces and vias in white and draws everything
  else at 35 % alpha; it is never saved and never undone. (The window state
  precedent is `BoardVisibility`: per window, never saved.)
- Class width / via size at a point: `SchematicDocument.netRules(at point:
  layer:) -> (width: Int, viaDrill: Double, viaDiameter: Double)`: the net of the
  pad whose rect contains `point` on `layer` (through-hole: any layer), else of a
  via or trace vertex at `point` (via the connectivity root's pad net), else the
  Default class. `addTrace(points:layer:width:)` and `addVia(at:)` use it.
- Net class editor rows use `LengthField` for mm values (units follow
  `board.units`) and a Stepper for mil, as `TracePanel` does.

Where correctness risk concentrates: the live-preview and committed trace
must have the same width (both read `netRules`); highlight must not slow the
canvas (only a `Set` lookup per item). Acceptance is behavior; no byte-identity.

Rejected: a separate Nets window (the sidebar is where Planes live and the layer
rules sit next to Layers); recomputing connectivity on every SwiftUI body pass
(debounced task instead).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — net rows

- [ ] `Canvas/Ratsnest.swift`: `BoardConnectivity.planeNet(ofNet:)`.
- [ ] `Canvas/NetList.swift`: `NetRow`, `netRows()` sorted by name (natural order), rails first.
- [ ] Tests: `schematicalTests/NetListTests.swift` — a two-LED board with one wired pair gives one row with 2 pads, 1 unrouted, key `.pin`; after `addTrace` between the pads, unrouted is 0; a `GND` net's key is `.ground`; a class member assignment shows in `classID`.

Acceptance: rows as listed.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/NetListTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: —

### Phase 2 — Nets section and highlight

- [ ] `Canvas/PCBSidebarView.swift`: `NetsSection` after `PlanesSection` in the `.board` case: a filter field, the rows (name, pads, a red count when unrouted > 0), a class menu (Default + classes) calling `assignNet`, a layer menu (Any / Only… / Prefer… with one toggle per trace layer) calling `setNetLayerRule`; the net list rebuilds in a debounced `.task(id: NetListInput)`.
- [ ] `Canvas/PCBCanvasView.swift`: `highlightedNet` property and the dimmed/strokes drawing; `ContentView.swift`: the `@State`, a row click sets it, Esc (the PCB key handler) clears it.
- [ ] Tests (`NetListTests`): the pure part — `SchematicDocument.copperItems(ofNet:)` (pads, trace ids, via ids of a net) returns exactly the net's items for a routed pair; the filter function matches by name and pad label.

Acceptance: item lookup as listed; `xcodebuild … build` succeeds (UI compiled).
  Check: `-only-testing:schematicalTests/NetListTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 3 — Net Classes and Net Groups editors

- [ ] `PCBSidebarView.swift`: `NetClassesSection` — list, **+** adds "Class N", **−** removes, name field, Width (mil, stepper), Clearance (a toggle "DRC minimum" and a `LengthField`), Via drill and diameter (`LengthField`), Layer rule (mode picker, layer toggles), and a "Nets: N" line with a menu listing members; wired to the plan-38-A/B edit functions. A new document shows no classes and a button "Add Power Class" that creates *Power* (20 mil, via 0.4 / 0.8) and assigns every supply rail, ground included (every `NetRow` whose key is `.rail`), to it.
- [ ] `NetGroupsSection` — list, **+** (menu: Length Match / Differential Pair), name, mode, Tolerance, and for a pair Gap and Max uncoupled, and Max meander; add members from a menu of nets not in another group; a pair's menu is disabled at 2 members; an ineligible net (more than one connection) is listed with a "multi-drop" note and is accepted.
- [ ] `PCBSidebarView.swift:TracePanel` and `ViaPanel`: read-only rows "Net" and "Class" from `netRows()`.
- [ ] Tests (`NetRulesTests`): "Add Power Class" creates one class with the board's rails as members; a second press changes nothing.

Acceptance: the Power-class test passes; build succeeds.
  Check: `-only-testing:schematicalTests/NetRulesTests -only-testing:schematicalTests/NetListTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 4 — tools use the class

- [ ] `Canvas/SchematicModel.swift`: `netRules(at:layer:)`; `addTrace(points:layer:width:)` (the existing `addTrace(points:layer:)` calls it with `netRules(at: points[0]…).width`); `addVia(at:)` uses the class via sizes.
- [ ] `Canvas/PCBCanvasView.swift:449` and `TraceDrawing.finish`: the live preview and the committed trace use the same width.
- [ ] Tests: `TraceToolTests` — a trace started on a Power-class pad is 20 mil; a trace starting on an empty spot is 10 mil; the live `drawing` trace and the committed trace have equal width; a via on a Power-class pad has the class's size; the existing 7 trace-tool tests stay green.

Acceptance: as listed.
  Check: `-only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/ViaTests -only-testing:schematicalTests/NetRulesTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `NetListTests` (new), `NetRulesTests`, `TraceToolTests`, `ViaTests`.
- Coverage check: `NetList.swift`, `netRules(at:)` and the highlight item lookup are exercised by the tests above.
- Runtime proof (this letter; real mouse, pointer left at 0,0 afterward per `AGENTS.md`): open `planning/plan-38-data/fixtures/AFU.schematical`, PCB page, nothing selected — the sidebar lists Nets, Net Classes, Net Groups; click the GND row → its pads brighten, the rest dim; Esc clears. Add Power Class, set a net's class, draw a trace from one of its pads → 20 mil in the Trace panel.
- Doc sync: `Help/KeyboardHelp.swift` PCB help gets the "Nets: click a net to highlight; Esc clears" lines.
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: Nets live in the board inspector, not a window; the highlight is window state, not document state; class width applies to new copper and the router, never rewrites existing traces; "Add Power Class" is a one-click helper that assigns every supply rail (including ground), not a default class, because a document with no classes must stay byte-identical (plan-38-A).

## Corrections

(Filled in during execution.)

## Summary

UI and a width lookup. Risk: the live preview and committed trace widths must
agree, and the net list must not make the sidebar sluggish on LFU (59 nets).
Untouched: routing, file format (A/B), schematic page.
