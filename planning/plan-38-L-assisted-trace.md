# plan-38-L: Router-assisted Trace tool — walk-around, live legality, the V key

Last updated: 2026-11-13
Effort: medium (1h–2h)
Depends on: plan-38-K

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F, G, H, I, J, K) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

Gives the hand-drawn Trace tool the router's knowledge, behind a toolbar toggle
(**Assist**, default off, so existing behavior is untouched): while dragging, the
live leg to the cursor *walks around* pads, other nets' copper and the board edge
instead of crossing them, draws red when no legal route exists (and the release
adds nothing), uses the starting net's class width and clearance, and ends on
the net's own copper freely. Independent of Assist, the **V key** drops a via
and continues on the next layer — the one thing a 2-layer board cannot do by
hand today.

References:

- plan-38-C (`netRules(at:layer:)`), plan-38-E (`RoutingGrid`, search), plan-38-H
  (`straighten`, `RoutingVerifier`), plan-38-K (toolbar, `PCBTool`).
- `Canvas/PCBCanvasView.swift`: `TraceDrawing` (`anchor`, `preview`, `finish`),
  `installDrawingKeyMonitor` (`:173`; keys: Esc `keyCode 53`, Space `49`, `/`,
  `A`), `Interaction.drawTrace`; `Canvas/BoardTrace.swift:TraceDraft`;
  `SchematicModel.swift:addTrace(points:layer:width:)`, `addVia(at:)`,
  `beginGesture`/`endGesture` (`:1612`).

## 1. Goal

- With Assist on, dragging from a pad toward a pad of the same net across a
  foreign pad draws a path that goes around it at the net's clearance; the
  committed trace equals the preview, passes the DRC clearance check, and is one
  undo step.
- A cursor position with no legal route draws the preview red; the release adds
  nothing and sets `editorMessage` "No legal route there."
- With Assist off, every existing Trace tool behavior and test is unchanged.
- **V** while drawing (Assist on or off): the trace so far is committed, a via
  (the net's class size) is added at the cursor's anchor, the drawing layer
  becomes the next trace layer, and drawing continues from the via; the whole
  gesture is one undo step; on a 1-layer board V does nothing.

### Non-goals (explicit constraints)

- No push-and-shove (owner decision, plan-38-A § Decisions): the assist never moves
  existing copper.
- No autorouting beyond the single leg under the cursor.
- Assist does not change `TraceRoute` or the angle keys (`A`, `/`, Space, Esc).
- The Trace tool still draws on one layer at a time apart from V.

## 2. Current State

- `TraceDrawing.preview` returns `draft.path(to: anchor, angle:)` — the draft's
  locked legs plus one 1-bend leg to the cursor; it does not look at other copper.
  `TraceDrawing.finish` calls `document.addTrace(points:layer:)` and selects the
  trace (`PCBCanvasView.swift:finish`).
- The drawing key monitor handles Esc, Space, `/` and `A` while `drawTrace` is the
  interaction (`PCBCanvasView.swift:173-205`).
- A gesture is one undo step: `beginGesture` / `endGesture`
  (`SchematicModel.swift:1612`), as the Trace tool's press/release already use.
- `traceLayer` is `ContentView` state, changed by the layer menu
  (`ContentView.swift`, the Menu over `traceLayers`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Existing Trace tool / angle tests (must stay green) | 22 | `grep -c "func test" schematical/schematicalTests/TraceToolTests.swift schematical/schematicalTests/TraceAngleTests.swift` → 7 + 15 |
| Time to rasterize the AFU grid | measured in Phase 1 (gate: < 1 s) | the `AssistRouterTests` timing assertion |

### Verified properties

- `V` is unused by the drawing monitor and by the app's shortcuts
  (`PCBCanvasView.swift:173-205` handles only Esc, Space, `/`, `A`; no
  `keyboardShortcut("v", …)` except ⌘V paste, which needs ⌘).

## 3. Design Overview

`Routing/AssistRouter.swift` (`@MainActor`, built at press): `init(document:,
layer:, net: Int?)` makes a `RoutingProblem` with scope `.all`, **every** pad and
trace as obstacle (nothing is "being routed"), the drawn net's id as the passable
owner, and a `RoutingGrid` at `grid = 0.1` mm on the one layer (vias off). `route(from:
CGPoint, to: CGPoint) -> [CGPoint]?`: A* (plan-38-E search, windowed to the
start–cursor box grown by 5 mm, capped at 200 000 expansions) from the cell
nearest `from` to the cell nearest `to`, then straighten (plan-38-H) and verify
(`RoutingVerifier`) the result with the net's width and clearance. nil = no legal route.

`TraceDrawing.preview` gains `assist: AssistRouter?`: with it, the live leg is
`assist.route(from: draft.legStart, to: anchor)`; without, as today. The result
carries `isLegal` for the red tint. `finish` adds the trace only when legal (assist
on); the release of an illegal one sets `editorMessage`.

`netRules(at:layer:)` (plan-38-C) picks the width; with Assist the start net also
fixes the passable owner. A start with no net (empty board space) treats every pad
and trace as foreign.

**V key**, in `installDrawingKeyMonitor`: `TraceDrawing.dropVia(document:, draft:,
cursor:, layer:, scale:, angle:, nextLayer:)` — inside the open gesture: commit the
current preview path as a trace, `addVia(at: anchor)` with the class size, return a new
`TraceDraft(start: viaCentre)` and the next layer (`board.traceLayers` after the current
one, wrapping). `ContentView` receives the layer through an `onLayerChange` callback
(as `onShowLayer`).

Where correctness risk concentrates: the preview equals the committed trace
(both from one function) and V leaving the document consistent when the cursor
anchor is illegal for a via (then V does nothing and says why).

Behavior gate; existing tests are the regression gate for Assist off.

Rejected: always-on assist (changes a 22-test-covered behavior and surprises users
who draw deliberately off-grid); rebuilding the grid per mouse move (the rasterization is
the costly part — once per press); a freehand "stop at first obstacle" (a legal walk-around
is strictly better and uses the same engine).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — `AssistRouter`

- [ ] `Routing/AssistRouter.swift`: as § 3.
- [ ] Tests: `schematicalTests/AssistRouterTests.swift` — two pads of one net with a foreign pad between: the route goes around it with the foreign copper ≥ the clearance away (checked with `CopperGeometry.gap`); two pads of one net with a solid foreign wall (a row of pads closing the board) → nil; a route may end on and cross the net's own pad; a Power-class start gives 20 mil clearance-aware routing (the path keeps the class clearance); `AssistRouter.init` on AFU takes < 1 s and `route` across the board < 0.25 s (wall clock, `XCTAssertLessThan`); the path equals itself on a second call (deterministic).

Acceptance: cases pass. A timing assertion that fails is a performance bug to fix (coarser grid for the assist, e.g. 0.127 mm), not to loosen.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/AssistRouterTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 2 — Assist in the Trace tool

- [ ] `PCBCanvasView.swift`: `var traceAssist = false`; build the `AssistRouter` when the trace press is anchored; `TraceDrawing.preview` / `finish` per § 3; the red tint (`drawTrace` preview colour) and release rule; `editorMessage`.
- [ ] `ContentView.swift`: a toolbar `Toggle` "Assist" (symbol `wand.and.rays`, help "Trace assist — walks around obstacles and stops at illegal routes") stored in `@AppStorage("traceAssist")`, passed to the canvas.
- [ ] Tests (`TraceToolTests`): with `assist: nil` all 7 existing tests pass unchanged; with an `AssistRouter` the Phase 1 foreign-pad case commits a trace equal to the preview and `designRuleCheck` has no `.clearance`; an unreachable target adds no trace, one `undo` step count unchanged, and `editorMessage` set.

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/TraceAngleTests -only-testing:schematicalTests/AssistRouterTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 3 — the V key

- [ ] `PCBCanvasView.swift`: `TraceDrawing.dropVia`, the key (`charactersIgnoringModifiers == "v"`, no modifiers) in `installDrawingKeyMonitor`, the `onLayerChange` callback; `ContentView.swift` sets `traceLayer` from it.
- [ ] `Help/KeyboardHelp.swift` "Trace tool (while dragging)": `V` "Drop a via and continue on the other layer"; the Tools list: Trace assist toggle; `KeyboardHelpTests` updated.
- [ ] Tests (`TraceToolTests`): on a 2-layer board V mid-trace gives one trace on Top, one via, then the finished trace on Bottom starting at the via centre; the whole gesture is one `undo(on: .pcb)`; the via has the start net's class size; V on a 1-layer board changes nothing; V where a via would violate clearance (anchor next to a foreign pad) adds nothing and sets `editorMessage`; with a 4-layer board with a Plane at Layer 2, V cycles Top → Bottom (never the Plane).

Acceptance: cases pass.
  Check: `-only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 4 — the real app

- [ ] Real app (per `AGENTS.md`, pointer left at 0,0 afterwards): open the AFU fixture, Trace tool with Assist on, drag between two pads of one net across another part's pad → path walks around; drag to a sealed spot → red, nothing added; press V mid-drag on a 2-layer board → via appears, layer menu changes; ⌘Z undoes the whole drag. Record observations here.
- [ ] Observations (filled in by the run): __.

Acceptance: behaves as stated.
  Check: the real-app run above (est. 15 min).
Commit: —

## Validation Plan

- Tests: `AssistRouterTests`, `TraceToolTests`, `TraceAngleTests`, `KeyboardHelpTests`, `DesignRuleCheckTests`.
- Coverage check: the nil-route, V-refusal and one-layer branches each have a test that fails if removed.
- Runtime proof: Phase 4.
- Doc sync: `Help/KeyboardHelp.swift`.
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions (owner: interactive aids, no push-and-shove, 2026-11-13). Author decisions: Assist is a default-off toggle remembered across launches; V works with or without Assist; an illegal release adds nothing; one grid per press.

## Corrections

(Filled in during execution.)

## Summary

Small code on top of the engine. Risk: preview/commit divergence and the
press-time rasterization cost (timing-gated). Untouched: `TraceRoute`, the angle
keys, every file format.
