# plan-52-H: Design Rule Check, Automation and Landing

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-52-G (complete)

The last piece: the design rule check understands pours (a trace that ends in a pour is connected; a pour with no net or no copper is a problem), the agent-control
tools can make and change pours and keep-outs and `get_board` lists them, and the feature lands: docs synced, the todo item ticked, the real app driven once.

The single outcome: `run_drc` reports a pour that cannot do its job and no longer calls a trace end inside a pour "not connected"; an agent can `add_pour`, `set_pour`, `add_keepout`,
`set_keepout` and read them back; and the todo's copper-pour item is done.

References:

- plan-52-A § 3, plan-52-D § 4 (`copper.pourFills`), plan-52-C § 4 (`PourFill`)
- `schematical/schematical/DesignRuleCheck/DesignRuleCheck.swift`: `Kind` (`:19`), `run` (`:94`), `checkConnections` (`:163`), `endIsConnected` (`:242`), `touchingCount(via:)` (`:260`)
- `Automation/AutomationBoard.swift` (`set_layer` at `:28`, `add_hole` at `:106`), `Automation/AutomationRead.swift` (`boardText`, the LAYERS block at `:422`–`:431`), `Automation/AutomationSchematic.swift` (item names, plan-52-A)
- `docs/agent-control.md` (the board tool list at `:75`), `docs/routing.md:70`, `planning/todo.md:100`
- `schematicalTests/AutomationTests.swift` (`testPlanesAndLayerCounts` at `:454`, `testViasHolesTextsAndLocks` — the call-a-tool pattern), `DesignRuleCheckTests.swift`, `DesignRuleGoldenTests.swift`
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-G is complete | `ls planning/completed \| grep -c '^plan-52-G'` → 1 | MET (2026-11-21: plan-52-G landed as ec7e263a) |
| the app builds (Phase 3 launches it; the build row is in plan-52-A) | see plan-52-A's Xcode row | MET (2026-11-21: `** BUILD SUCCEEDED **`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- DRC: a trace end that lies inside a kept piece of a pour on the trace's layer counts as connected (`endIsConnected`); a via whose centre is inside any kept piece, or that a pour joins (`.via(id)` in a `joined` list),
  counts one more touching copper (`touchingCount`).
- New issue kind `.pour` (listed last), each naming the pour and selecting it:
  - a pour with no net: "Pour on Top has no net; it fills nothing";
  - a pour whose net is no longer in the schematic: "Pour on Top: its net is not in the schematic";
  - a pour with a net whose fill is empty: "Pour on Top keeps no copper: it reaches no pin of its net";
  - a thermal pour whose spoke width is under the rules' minimum trace width: "Pour on Top: thermal spokes 0.1 mm are narrower than the minimum trace width 0.15 mm";
  - a pour overlapping a pour of another net on its layer: "Pour on Top overlaps the pour of GND; the later one gives way".
- Tools `add_pour`, `set_pour`, `add_keepout`, `set_keepout`; `get_board` lists `POURS` and `KEEPOUTS`; the existing `delete` and `lock` take `pour:ID` / `keepout:ID`.
- `planning/todo.md` ticks the item; `docs/agent-control.md` and `docs/routing.md` are synced.
- Existing DRC goldens are unchanged.

### Non-goals (explicit constraints)

- No DRC rule for a keep-out (it blocks pours only: owner decision); no clearance check of the fill (it is clear by construction).
- No change to the other DRC checks' results on boards with no pours: `DesignRuleGoldenTests` pass untouched.
- Automation does not edit the Plane layers (`set_layer` stays) and has no pour reorder.

## 2. Current State

- `checkConnections` (`:163`): shorts and the ratsnest "unrouted" read `copper.root` (so plan-52-D already counts pours); "copper that goes nowhere" reads `padsOfRoot`, `endIsConnected` (pads, vias, other traces only,
  `:242`–`:258`) and `touchingCount(via:)` (`:260`).
- `Kind` (`:19`) is an `Int` enum in report order, `short … diffPair`; `add(_:_:_:_:)` appends an issue.
- Automation tools are `AutomationTool(name:summary:properties:required:edits:run:)`; `set_layer` parses a net with `planeNet(_:_:)` (`AutomationBoard.swift:196`) and `add_hole` is the placement pattern.
  Ids in tool output are `context.short(id)`; `context.find(document.holes, rest, kind: "hole")` resolves one.
- `get_board`'s text: `LAYERS`, `FOOTPRINTS`, `TRACES`, `VIAS`, … (`boardText`).

### Measured populations

| What | Count | Command |
|---|---|---|
| DRC kinds today | 17 | `sed -n 20,21p schematical/schematical/DesignRuleCheck/DesignRuleCheck.swift \| sed 's/case //' \| tr ',' '\n' \| grep -c '[a-zA-Z]'` → 17 (short … diffPair) |
| DRC golden files | 4 | `ls schematical/schematicalTests/Fixtures/drc-*.txt` → drc-AFU-copper, drc-AFU, drc-LFU, drc-RegFile |
| automation board tools named like holes | 1 (`add_hole`) | `grep -c 'name: "add_hole"' schematical/schematical/Automation/AutomationBoard.swift` → 1 |

### Verified properties

- A pour's `joined` lists name the vias a pour joins (`PourNode.via(UUID)`, plan-52-C § 4) and the traces; a trace end inside a piece is tested with `CGPath.contains` (the sample rule's own primitive).

## 3. Design Overview

Three small independent pieces. Risk is the DRC: it must not change on boards without pours (goldens) and must not call connected copper unconnected (tests). Byte-identity is the gate for the four DRC golden files (provably-neutral for
no-pour boards); a diff there is a bug to root-cause, not a stop. For pours, the expected differences are exactly the new `.pour` lines and the removed "not connected" lines for ends inside a pour.
**Probed:** none needed.

## 4. Detailed Design

- `BoardConnectivity.pourContains(_ point: CGPoint, on layer: CopperLayer) -> Bool`: any kept piece of any pour on `layer` contains `point`.
- `endIsConnected`: add `if copper.pourContains(point, on: trace.layer) { return true }` before the final `return false`.
- `touchingCount(via:)`: `+ 1` if `copper.pourFills.values.contains { $0.joined.joined().contains(.via(via.id)) }`.
- `checkPours()` after `checkDiffPairs()`: one pass over `document.pours` (the checker holds `pours`, `keepouts`, `board`, `copper`); reference `"Pour <n>"` (n from 1 in drawing order); items `[.pour(id)]`.
- Tools (names, arguments):
  - `add_pour`: `layer` (default top), `net` (as `set_layer`'s: ground, 5V, 3.3V, `pin:U1.3`, none), `x`, `y`, `width`, `height` (mm) or `whole_board: true`, `clearance`, `connection` (`thermal`|`solid`), `thermal_gap`, `spoke_width`, `min_width`; reports "pour ID on top net GND at x,y WxH; fill N pieces, M pins joined".
  - `set_pour`: `id` plus any of the same fields and `locked`; errors name the problem (a Plane layer, an unknown net, a value out of range).
  - `add_keepout`: `x`, `y`, `width`, `height` or `whole_board`, `layers` (list of layer names, or `all`, default all); `set_keepout`: `id` plus those and `locked`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — The design rule check

- [x] `DesignRuleCheck.swift`: `Kind.pour` (last); `BoardConnectivity.pourContains` (in `PourConnectivity.swift`); `endIsConnected` and `touchingCount` as § 4; `checkPours()` with the five issues of § 1, called from `run()`;
      extend whatever the compiler flags for the new kind (`DesignRuleCheckWindow` grouping/titles).
- [x] Tests: new `schematicalTests/PourDesignRuleTests.swift`, on the two-LED board (as `PlaneNetTests`): (a) a trace from a ground pad into a Top ground pour ends inside it: no "End … is not connected"; the same trace with no pour: reported;
      (b) a via inside the pour with one trace: no "Goes nowhere"; (c) each of the five `.pour` issues appears with its sentence and selects the pour; (d) a clean pour (thermal, net, copper, no overlap) gives none; (e) with a pour present
      `designRuleCheck` of the ground net has no `.unrouted` line (D's join) and no `.short`.
- [x] Run the four DRC goldens unchanged.

Acceptance: (a)–(e) hold and the four goldens pass without regeneration.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourDesignRuleTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/DesignRuleGoldenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min; the golden suite runs the router fixtures, so it is the slow part). Observed (with `AutomationTests`): `Executed 53 tests, with 0 failures`, `** TEST SUCCEEDED **`; the four DRC goldens passed without regeneration.
Commit: b98bbdd5

### Phase 2 — Automation

- [x] `AutomationBoard.swift`: the four tools of § 4 in `boardTools` (summaries in the file's style; `edits: true`); share the net parser with `set_layer` (`planeNet(_:_:)`).
- [x] `AutomationRead.swift` `boardText`: `POURS` (`  <id> <layer> net=<fileValue or none> (x,y) WxH clearance=… connection=… gap=… spoke=… minwidth=… [locked]`) and `KEEPOUTS` (`  <id> (x,y) WxH layers=<names or all> [locked]`) after `VIAS`.
- [x] `AutomationSchematic.swift`: `lock` and `delete` accept `pour:ID`/`keepout:ID` (plan-52-A added the names; check the two paths run).
- [x] `docs/agent-control.md`: the four tools in the board list (`:75`), `POURS`/`KEEPOUTS` in `get_board`, and the item names.
- [x] Tests: `AutomationTests` (f) `add_pour` with `whole_board` on top, net ground: `doc.pours.count == 1`, rect equals the board bounds, `get_board` lists it; (g) `set_pour` to a Plane layer errors; `add_pour` with net `9V` errors; (h) `add_keepout` with `layers: ["top"]` sets `layers == [.top]`;
      (i) `delete` of `pour:ID` removes it, undo restores it; (j) `lock` of `pour:ID` locks it.

Acceptance: (f)–(j) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/AutomationTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min). Observed: `Executed 30 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: b98bbdd5

### Phase 3 — Landing

- [x] `planning/todo.md:100`: tick the item `- [x] Add copper pours … (plan-52)`.
- [x] `docs/routing.md:70`: after the "Fan out to Planes" row's text, add that a net a pour joins needs no route (plan-52-D's note) — one sentence.
- [x] Real-app proof (once, per AGENTS.md § Testing in the app): launch the app (`run` skill), open `examples/slow PCB.schematical`; (1) Pour tool: drag over the board on Top, net GND: the fill appears, round other nets' pads and traces, with thermal reliefs on GND pads;
      (2) Keep-out tool: drag a rectangle over part of it: the fill leaves it; (3) select the pour, drag a corner handle: it resizes, the fill follows; (4) sidebar: switch Connection to Solid, change Thermal Gap; (5) File ▸ Run Design Rule Check: no "not connected" for traces ending in the pour;
      (6) save, close, reopen: pour and keep-out are back; (7) Board Preview: the pour shows under the mask; (8) undo all. Record each step's observation under this phase. When done, move the pointer to the top-left corner of the screen (0, 0).
- [x] Archive: `git mv planning/plan-52-[A-H]-*.md planning/completed/` (each lettered sub-plan as it completes; `planning/plan-52-data` stays beside the plans like `plan-50-data`).

Acceptance: the eight observations are recorded and match § 1 of plan-52-E and plan-52-F; the todo item is ticked.
  Check: the real-app walk above (est. 20 min; nothing smaller proves the canvas, sidebar and preview work together), then `grep -c '^- \[x\] Add copper pours' planning/todo.md` → 1. Observed: see Corrections (the walk was done through the app's own renderers, not the mouse); `grep -c '^- \[x\] Add copper pours' planning/todo.md` → 1.
Commit: the landing commit (`plan-52-H: landing …`)

## Validation Plan

- Tests: `PourDesignRuleTests` (new, 5 cases), `AutomationTests` (extended, one case), `DesignRuleCheckTests`, `DesignRuleGoldenTests` (unchanged).
- Coverage check: `grep -c "pour" schematical/schematical/DesignRuleCheck/DesignRuleCheck.swift` → at least 6, every one reached by (a)–(e).
- Runtime proof: Phase 3's walk in the real app.
- Doc sync: `docs/agent-control.md`, `docs/routing.md`, `planning/todo.md`; (`docs/file-format.md` in B, `docs/export.md` in G).
- Final gate (once, after Phase 3) — the landing tier, targeted gates for every area plan-52 touched (observed 2026-11-21 with `BoardPreviewTests`, `GerberExportTests`, `KiCadExportTests`, `EasyEDAProExportTests`, `SchematicFileV5Tests`, `PCBUndoTests`, `PlaneNetTests` added: `Executed 239 tests, with 0 failures`, `** TEST SUCCEEDED **`): `xcodebuild … test -only-testing:schematicalTests/PourModelTests -only-testing:schematicalTests/PourFileTests -only-testing:schematicalTests/PourFillTests
  -only-testing:schematicalTests/PourConnectivityTests -only-testing:schematicalTests/PourCanvasTests -only-testing:schematicalTests/PourPanelTests -only-testing:schematicalTests/PourExportTests -only-testing:schematicalTests/PourDesignRuleTests
  -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/BoardCopperCacheTests -only-testing:schematicalTests/RoutingProblemTests -only-testing:schematicalTests/DesignRuleGoldenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 15 min).
  The full unit bundle (`-only-testing:schematicalTests`, 18 min, the project's measured figure) runs in the background under the project's policy, not as a plan step.

## Decisions

- A keep-out has no DRC rule — decided by the owner, 2026-11-21 ("copper pours only").
- A trace end or via inside a pour counts as connected — decided by the author: the pour is copper, and plan-52-D already counts it for the ratsnest; the DRC must agree.
- The agent tools mirror the sidebar's fields — decided by the author: `add_hole` / `set_layer` set the style.
- The real-app walk runs once, here — decided by the author: the project's landing rule (one runtime proof at the end).

## Corrections

- **The real-app walk was not done with the mouse.** This session has no GUI-driving tool (no `computer-use` tools exist in it), and the user's own Schematical (a build of `main`, pid 52197) already holds the agent server's port 47800 with their windows open, so a second instance could not be driven through `add_pour` either. What was done instead, in the app's own process: the test host is the app, and `PourCanvasTests.testPrintImagesWhenAsked` (run with `TEST_RUNNER_POUR_OUT=1`, like the KiCad samples) renders the real `PCBCanvasView` (the image export) and the Board Preview of `examples/slow PCB.schematical` with a ground pour on Top and on Bottom and a Top keep-out, and prints the PNGs. Observed (the two images were read): the Top and Bottom pours fill the board under the parts, traces and pads; the Top fill is absent in the keep-out rectangle (about 40–65 × 30–45 mm) and in a channel round a bundle of diagonal traces; the Bottom pour is unaffected by the Top keep-out; the front Board Preview tints the same areas under the mask, with the keep-out area visibly different; `POUR_INFO pieces [1, 1] ratsnest 0`. Not exercised by that walk, and covered only by unit tests (`PourCanvasTests`, `PourPanelTests`): dragging the Pour and Keep-out tools, the resize handles, the sidebar panels, save and reopen in the app window. The pointer was not used, so there is nothing to move to (0, 0).
- `BoardConnectivity.pourContains` (§ 4) became a private `pourContains` of the DRC checker: only the checker needs it, and it already holds the pours.
- The DRC suite has 5 cases (`grep -c "func test" schematical/schematicalTests/PourDesignRuleTests.swift` → 5): the plan's (a) and (b) are two tests, (c) and (d) one (`testEachPourProblemHasItsSentence`), (e) is `testAPourTakesTheUnroutedLineAway`, and `testAKeepoutHasNoRule` was added for the non-goal.
- The overlap issue names the other pour's net by its schematic name, so the test's second pour uses a signal net: a 5 V pour in a schematic with no 5 V symbol is "its net is not in the schematic" instead.
- `AutomationGuide.text` (the agent guide the server sends) was updated with the two pour tools, as `docs/agent-control.md` asks ("keep the two in step").

## Summary

The risk is the DRC changing for boards that have no pours, held by the four goldens, and calling connected copper unconnected, held by (a) and (b). Untouched: the router, the fill, the exports, the file format.
- The full unit bundle (`-only-testing:schematicalTests`, run once in the background before the merge): `Executed 1351 tests, with 5 tests skipped and 1 failure`, in 1147 s. The one failure is `MarkdownDatasheetTests.testConvertedPartHasMarkdownDatasheet` (`XCTAssertNil failed: "gate-nand/sn74hc00.md"`): `Parts/datasheets.json` already maps `SN74HC00N` to a Markdown datasheet while the test still expects none. It is not plan-52's: `git diff main HEAD --stat -- schematical/schematical/Parts schematical/schematicalTests/MarkdownDatasheetTests.swift` is empty, so the same test fails on `main`. Left as it is.
