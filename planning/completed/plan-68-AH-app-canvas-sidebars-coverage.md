# plan-68-AH: App coverage — Canvas

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-AG

The app files in § 2 are brought to 100% line coverage: 964 lines are
uncovered in them today (3 files, 1499 executable lines).

Prerequisites: plan-68-A's table, and plan-68-AG complete (its rows are gone
from `scripts/coverage-ratchet.tsv` and its Commit lines are filled).

References:

- The files in § 2; `planning/plan-68-data/app-missed.tsv` (per-file census:
  the full Debug suite with coverage at `e2f6811a`, `app-coverage.txt`).
- `docs/testing.md`; `AGENTS.md` § Source size and coverage, § Testing in the
  app (park the pointer at (0, 0) after any real-mouse test).
- Memory of earlier plans: test hosts are the real app — run `XB` under a
  time limit, never give a test document a file the test did not write.

## 1. Goal

- `node scripts/coverage.js --app --tests <the classes that test them> --only
  <file>` prints no uncovered line for any file in § 2, and their rows are gone
  from the coverage ratchet.
- The test classes this letter adds or extends pass; `SZ` passes.

### Non-goals

- No user-visible behavior change, except a bug a new test proves (§ 3 step
  6). Logic moved out of a view into a model or controller type keeps the
  view's behavior exactly.

## 2. Current State

Uncovered lines per file under the full Debug suite at `e2f6811a`, counted
per line from `xccov view --archive` (`planning/plan-68-data/app-missed.tsv`; paths under `schematical/schematical/`).
A file that plan-68-B–F or plan-68-V split appears under its old name; its row
stands for all of its pieces. The letter starts by re-running step 1 of § 3
for each file; that list is the work.

| File | Uncovered | Executable |
|---|---|---|
| `Canvas/NetsSidebar.swift` | 361 | 379 |
| `Canvas/NixieSymbolView.swift` | 117 | 117 |
| `Canvas/PCBCanvasView.swift` | 486 | 1003 |
| **Total** | **964** | **1499** |

## 3. Design

The method is plan-68-W § 3, unchanged: list with a scoped `COV --app`,
close each block (unit test; logic out of a view into a tested type; render
test through the accessibility tree; drive windows through their actions
with injectable modals; bitmap drawing checks; synthesized events; stubbed
network and tools; delete the unreachable), assert an observable result,
lower the file's ratchet row in the same commit, fix any bug a test finds.

### Where the risk is

Moving logic out of a view: the moved code must keep reading the same state
at the same moment (a `@State` read inside a gesture closure now reads the
controller's property). The view's existing test classes and the new
controller tests both run in the phase gate.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — `NetsSidebar.swift`

- [x] Close every uncovered line of these files per § 3.
  New `NetsSidebarViewTests` (13 tests: the Nets list, its filter and empty
  states, the row menu, the layer-rule items, the classes' empty state and
  **+**, an open class's every field, Move Up / Down and Delete, the width
  calculator's sheet, the members menu, the groups' **+** items, an open pair
  and a group with a missing net, the Add Net items, the length label). The
  menus' contents are views of their own (`NetRowMenu`, `NetLayerRuleItems`,
  `NetMemberItems`, `NetGroupAddItems`, `NetGroupFreeItems`) so a test hosts
  and presses them; the highlight toggle is `NetsSection.toggleHighlight`, **+**
  is `NetClassesSection.addClass` / `NetGroupAddItems.add`; the three views with
  `@State` set it in an explicit `init`.
  `LOGIC_COV_WORK=/tmp/logic-cov-AH node scripts/coverage.js --app --tests NetsSidebarViewTests,NixieSymbolViewTests,PCBCanvasControllerTests,PCBCanvasDrawingTests,PCBCanvasViewTests,PourCanvasTests,CutoutCanvasTests,PlaneShadingTests,FabCheckFixesTests,RouteToolTests,TraceToolTests,ReferenceDragTests,BoardDetailsSheetTests --only schematical/schematical/Canvas/`
  → no `path:line` for `NetsSidebar.swift` (was 361; `xccov`: 394 executable,
  0 uncovered).
- [x] ratchet rows: parent (`NetsSidebar.swift` at 0)
  Ratchet rows: removed by the parent in this letter's commit (3 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: 74f1314c

### Phase 2 — `NixieSymbolView.swift`, `PCBCanvasView.swift`

- [x] As Phase 1, for these files (`PCBCanvasView.swift`'s row stands for its
  plan-68-V piece `PCBCanvasTypes.swift` too). New `NixieSymbolViewTests`
  (recorded paint calls: the dark front cathode, lit glyphs and dots at their
  level, a long symbol, hover / selection / wired pins; the view's frame and
  face). `PCBCanvasView`'s press, pinch, hover, scroll and drawing-key state and
  steps moved into `PCBCanvasController` (new, beside it), its drawing into
  `PCBCanvasDrawing.swift` (an extension; the press it draws is a
  `DrawState`), and the view's body only calls them. New
  `PCBCanvasControllerTests` (every tool's press, Select's ⌘ / handle / item /
  box / pan, a scroll mid-press, pinch, hover, Esc / Space / `/` / V, the
  monitor), `PCBCanvasDrawingTests` (bitmaps: board, grid, traces and their
  underlays, vias, holes, pads both sides, a diode's band, card fingers and
  header pads, pours, keep-outs, handles, a pour's fill, a Plane's tint, the
  ratsnest, a highlighted net, an Autoroute preview, the Trace / Pour / Cutout
  previews, an illegal assisted leg, selection boxes, texts, the export) and
  `PCBCanvasViewTests` (the hosted canvas: its size, the key monitor on and
  off, a synthesized drag that pans, the hover tag following the controller,
  the box overlay; `PCBHoverLayer`, `PadLabel`, pad outlines, a via on a
  Plane's net, `ReferenceDragging`, `RouteTool`, `dropVia` on an illegal leg).
  Same command → no `path:line` for `NixieSymbolView.swift` (was 117),
  `PCBCanvasView.swift` (was 553), `PCBCanvasTypes.swift` (was 61) or the new
  `PCBCanvasController.swift` / `PCBCanvasDrawing.swift` (`xccov`: 117, 168,
  190, 244, 410 executable, 0 uncovered). The gate, `XB -only-testing` of these
  classes and every other class that touches these files (25 classes) →
  `Executed 289 tests, with 0 failures`. `node scripts/source-size.js --check`
  → exit 0.
- [x] ratchet rows: parent (`NixieSymbolView.swift`, `PCBCanvasView.swift`,
  `PCBCanvasTypes.swift` at 0)
  Ratchet rows: removed by the parent in this letter's commit (3 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: 74f1314c

## Validation Plan

- Tests: the classes added or extended, each named in its commit.
- Coverage check: every § 2 row gone from the ratchet; `COV --app --check
  --only` each file → exit 0.
- Final gate (once, after the last phase): all of this letter's classes
  together under `XB`; `SZ` (est. 15 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.
- (author) Logic leaves views for tested types; views are covered by render
  tests through the accessibility tree (§ 3).

## Corrections

- (2026-11-26) `PCBCanvasView.swift` was split by plan-68-V into it and
  `PCBCanvasTypes.swift` (61 uncovered at the start, its own ratchet row of
  33): that piece is covered here too. The letter also adds two pieces,
  `PCBCanvasController.swift` and `PCBCanvasDrawing.swift`, both at 0.
- (2026-11-26) The scoped list at the start was 553 lines in `PCBCanvasView.swift`
  (not the census's 486: the census counted the whole suite).
- (2026-11-26) No bug was found. The moved press state is an `@Observable`
  controller; `PCBCanvasViewTests.testTheHostedCanvasRedrawsAsItsControllerChanges`
  checks the hosted canvas redraws from it (the hover tag goes during a pan
  and comes back), as the view's `@State` did.
- (2026-11-26) A menu-style picker in a grouped Form never fills its menu in
  the test host (plan-68-AI): the picker tests host the section in a columns
  Form.
- (2026-11-26) `PourCanvasTests.testImageExportShowsPoursButNotKeepouts` (not
  this letter's) failed once before any change here and once after, passing
  in between: two renders of the same board gave PNGs 85559 and 85562 bytes
  long. Left to its owner; it passed in the final gate.
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
