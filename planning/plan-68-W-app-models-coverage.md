# plan-68-W: App coverage — Backend, Routing, Simulation, Memory, Boards, DesignRuleCheck, BillOfMaterials, DocumentInfo, Help

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-V

The app files in § 2 are brought to 100% line coverage: 1069 lines are
uncovered in them today (45 files, 10716 executable lines).

Prerequisites: plan-68-A's table, and plan-68-V complete (its rows are gone
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
| `Backend/HeaderFamily.swift` | 3 | 3 |
| `Backend/PartCatalog.swift` | 116 | 1738 |
| `Backend/PartFacts.swift` | 181 | 731 |
| `Backend/TimerText.swift` | 0 | 49 |
| `Backend/UserPartLibrary.swift` | 6 | 62 |
| `Routing/AssistRouter.swift` | 0 | 82 |
| `Routing/AutorouteModel.swift` | 19 | 110 |
| `Routing/AutorouteSheet.swift` | 56 | 203 |
| `Routing/MovableChains.swift` | 2 | 329 |
| `Routing/RouteCommands.swift` | 3 | 48 |
| `Routing/Router.swift` | 54 | 631 |
| `Routing/RoutingApply.swift` | 9 | 153 |
| `Routing/RoutingCleanup.swift` | 0 | 346 |
| `Routing/RoutingGrid.swift` | 3 | 248 |
| `Routing/RoutingOptions.swift` | 2 | 81 |
| `Routing/RoutingPair.swift` | 4 | 214 |
| `Routing/RoutingPlan.swift` | 6 | 243 |
| `Routing/RoutingPolish.swift` | 2 | 115 |
| `Routing/RoutingProblem.swift` | 3 | 16 |
| `Routing/RoutingProblemBuilder.swift` | 6 | 214 |
| `Routing/RoutingPush.swift` | 0 | 107 |
| `Routing/RoutingRepair.swift` | 14 | 191 |
| `Routing/RoutingSearch.swift` | 5 | 348 |
| `Routing/RoutingSession.swift` | 0 | 77 |
| `Routing/RoutingState.swift` | 2 | 252 |
| `Routing/RoutingTerminals.swift` | 21 | 80 |
| `Routing/RoutingTuning.swift` | 2 | 152 |
| `Routing/RoutingVerifier.swift` | 7 | 161 |
| `Simulation/BuzzerSound.swift` | 60 | 147 |
| `Simulation/Simulation.swift` | 51 | 586 |
| `Simulation/SimulationCommands.swift` | 9 | 36 |
| `Simulation/SimulationEngine.swift` | 69 | 279 |
| `Simulation/VoltageCheck.swift` | 13 | 117 |
| `Memory/LoadFileBar.swift` | 0 | 21 |
| `Memory/MemoryFileAccess.swift` | 34 | 94 |
| `Memory/MemoryWindow.swift` | 103 | 363 |
| `Boards/BoardCard.swift` | 1 | 100 |
| `Boards/BoardCatalog.swift` | 31 | 367 |
| `Boards/BoardExpression.swift` | 3 | 266 |
| `DesignRuleCheck/DesignRuleCheck.swift` | 64 | 774 |
| `DesignRuleCheck/DesignRuleCheckWindow.swift` | 26 | 156 |
| `BillOfMaterials/BillOfMaterialsWindow.swift` | 65 | 239 |
| `DocumentInfo/DocumentInfoWindow.swift` | 12 | 69 |
| `Help/Acknowledgements.swift` | 2 | 63 |
| `Help/KeyboardHelp.swift` | 0 | 55 |
| **Total** | **1069** | **10716** |

## 3. Design: the method

This is the method every app coverage letter (W–Z, AA–AJ) uses; the others
point here.

1. **List.** `node scripts/coverage.js --app --tests <classes> --only <file>`
   prints every uncovered `path:line` of the file under the named test
   classes (plan-68-A § 6). A scoped run can only under-count, so a file that
   shows no uncovered line under the scoped run has none under the full suite.
2. **Close each uncovered block the one way that fits:**
   - *logic in a model, value or controller type*: a unit test calling it.
   - *logic inside a view* (a SwiftUI view's `private func`, an action
     closure, a gesture handler, an `onChange`): move it into a type beside
     the view (`<View>Model` or `<View>Controller`, holding what the view's
     `@State` held, or a pure function), test that type, and have the view
     call it. This is the "logical units that can be tested on their own"
     rule applied to views, and it is also what shrinks
     `SchematicCanvasView` and `PCBCanvasView`.
   - *a SwiftUI `body` or subview*: a render test. Host the view in an
     `NSHostingView` in each state that selects a different branch, lay it
     out (`layoutSubtreeIfNeeded`), and assert on what is shown through the
     accessibility tree (labels, values, enabled state), with a small helper
     `schematicalTests/Support/ViewProbe.swift` (added by plan-68-W) that
     finds elements by label. The precedents host views the same way (12
     test files use `NSHostingView` or `ImageRenderer`).
   - *AppKit windows, panels and menu commands*: create them in the test
     host, drive them through their actions (`sendAction`, the menu item's
     action), assert the window's content, and close every window in
     `tearDown`. A code path that runs a modal (`NSAlert.runModal`, a save
     panel) gets an injectable presenter so the test answers it; no test
     leaves a modal on the owner's screen.
   - *NSView drawing*: render with `cacheDisplay(in:to:)` into a bitmap and
     assert colours at points the drawing code computes.
   - *NSView event handlers*: call `mouseDown(with:)` etc. with events built
     by `NSEvent.mouseEvent(with:location:…)` for the view's window.
   - *network or external tools* (DigiKey connections, KiCad exports): the
     code already takes or gains an injected session or executable path;
     tests use a `URLProtocol` stub or a fake executable, never the network.
   - *unreachable*: delete it or restructure it; the commit message says why
     no caller reaches it. No exclusion list (owner).
3. **Every new test asserts an observable result** — a returned value, a
   label shown, a pixel drawn, a document change. A test that only executes a
   line passes with the line deleted, and is not accepted.
4. **Where tests go.** `schematicalTests/<Type>Tests.swift` for the type under
   test; an existing class when it already covers that type. Every new test
   file stays at or under 800 code lines.
5. **Ratchet.** `COV --app --tests … --only <file> --write-ratchet` lowers or
   removes that file's row only (plan-68-A § 6), in the same commit as the
   tests.
6. **A test that finds a bug** is the bug's regression test: fix it in the
   same letter and list it under Corrections.

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

### Phase 1 — 25 files, `HeaderFamily.swift` … `RoutingState.swift`

- [x] `schematicalTests/Support/ViewProbe.swift` (§ 3 step 2) with its own tests.
  `schematicalTests/Support/ViewProbe.swift`, `schematicalTests/ViewProbeTests.swift`
  (parent, a26978f5): `XB -only-testing:schematicalTests/ViewProbeTests` → 3 tests,
  0 failures.
- [x] Close every uncovered line of these files per § 3. New classes (shared
  by both phases): `AutorouteSheetTests`, `BillOfMaterialsWindowTests`, `BoardCatalogEdgeTests`, `BuzzerSoundTests`, `DesignRuleCheckEdgeTests`, `DesignRuleCheckWindowTests`, `MemoryFileAccessTests`, `MemoryWindowTests`, `PartCatalogEdgeTests`, `PartFactsSweepTests`, `RouterRunEdgeTests`, `RoutingDocumentEdgeTests`, `RoutingEngineEdgeTests`, `SimulationControllerTests`, `SimulationEngineTests`, `VoltageCheckTests`, `WindowMenuItemsTests`. With them, the existing classes that name a type of
  a § 2 file (listed under Phase 2). Check:
  `LOGIC_COV_WORK=/tmp/logic-cov-W node scripts/coverage.js --app --tests <those
  200 classes> --only schematical/schematical/` → no `path:line` under any
  § 2 file or piece (`Backend/`, `Routing/`, `Simulation/`, `Memory/`,
  `Boards/`, `DesignRuleCheck/`, `BillOfMaterials/`, `DocumentInfo/`,
  `Help/`); app total 3386 uncovered of 50281. The same run's 5 failing
  tests are none of W's and touch no W change (`git diff` empty for their
  files): `AcknowledgementsTests` (2, expects no avr8js entry since
  plan-62-H), `DrawerCategoriesTests.testInterfaceAnalogAndClocks`,
  `UserLibraryCatalogTests.testAddListsUnderUserLibraryAndWritesTheFile` and
  `UserPartLibraryTests.testWriteThenFilesReturnsTheFile` (`/var` vs
  `/private/var` temporary paths).
- [x] ratchet rows: parent (every § 2 file and piece at 0 in the run above).
  Ratchet rows: removed by the parent in this letter's commit (41 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: —

### Phase 2 — 20 files, `RoutingTerminals.swift` … `KeyboardHelp.swift`

- [x] As Phase 1, for these files: the same run, no `path:line` under any of
  them. `node scripts/source-size.js --check` → exit 0 (new files staged
  for the counter; the largest new one, `MemoryWindowTests.swift`, 287
  code lines). Existing classes in the run: AcknowledgementsTests, AssistRouterTests, AutomationEditToolTests, AutomationSessionToolTests, AutomationTests, AutorouteModelTests, AvrAppTests, BillOfMaterialsTests, BlockPreviewViewTests, BoardCardTests, BoardCatalogTests, BoardCopperCacheTests, BoardDetailsSheetTests, BoardPreviewTests, BoardSettingsEditTests, BoardSideTests, BusPanelModelTests, BusPanelViewTests, BusTypeTests, CanvasModelCoverageTests, CanvasPanelsRenderTests, CanvasSymbolRenderTests, CapacitorTests, CardSlotTests, CharacterLCDTests, ChipDrawerViewTests, ChipLayoutSideTests, ChipPinActiveLowTests, ContentViewControllerTests, ContentViewRenderTests, CpuWindowTests, CrystalTests, CustomFootprintDraftTests, CustomFootprintEditorTests, CutoutDesignRuleTests, CutoutPicturesTests, CutoutPourTests, CutoutRoutingTests, DXFImportTests, DesignRuleCheckTests, DesignRuleCheckWizardTests, DesignRuleGoldenTests, DisplayDriverTests, DocumentMenuTests, DrawerCategoriesTests, DrawerConverterFilterTests, DrawerLayoutTests, DrawerPartMenuTests, EasyEDAProExportTests, FabCheckFixesTests, FileDesignRulesTests, FootprintPreviewTests, FootprintPreviewViewTests, FootprintStepPageTests, FootprintThumbnailTests, FreeFormOutlineTests, GateSymbolTests, GenericPartTests, HeaderBoardTests, HoleTests, InductorFuseTests, KeyboardHelpTests, KiCadExportTests, KindGuideTests, KindInfoTests, LooseEndTests, MarkdownDatasheetTests, McuTests, MeanderTests, NetClassTests, NetLabelTests, NetLengthTests, NetListTests, NetRulesTests, NetsSidebarViewTests, OppositePadTests, PCBCanvasControllerTests, PCBCanvasDrawingTests, PCBCanvasViewTests, PCBSidebarPanelsTests, PCBSidebarViewTests, PadHoverTests, PairGeometryTests, ParametricFootprintDraftTests, PartEditorLibraryTests, PartEditorModelTests, PartEditorSectionsTests, PartEditorWindowTests, PartEditorWindowViewTests, PartFileLoadingTests, PartFileSaveOpenTests, PartListTests, PartLookTests, PartOptionsModelTests, PartOptionsSectionTests, PartPreviewTests, PartRegistrationTests, PartUnitTests, PickAndPlaceExportTests, PinNameOverlineTests, PinNameTextTests, PlaneNetTests, PlaneShadingTests, PlaySwitchResetTests, PldTests, PourCanvasTests, PourConnectivityTests, PourDesignRuleTests, PourExportTests, PourFillTests, PourPanelTests, PowerFlagTests, PowerSwitchTests, PreferencesTests, ReferencePrefixTests, RegisterTests, RegisterWindowTests, RegulatorTests, RelayBuzzerTests, ResistorTests, RouteToolTests, RouterFixtureTests, RouterTests, RoutingAnchorTests, RoutingApplyTests, RoutingChainEngineTests, RoutingChainTests, RoutingCleanupTests, RoutingGraphTests, RoutingGridTests, RoutingNegotiationTests, RoutingOptionsTests, RoutingPairTests, RoutingPlanTests, RoutingPolishTests, RoutingProblemTests, RoutingPushIntegrationTests, RoutingPushTests, RoutingRepairTests, RoutingSearchTests, RoutingSessionTests, RoutingStateTests, RoutingTuningTests, RoutingVerifierTests, SchematicCanvasControllerTests, SchematicCanvasRenderTests, SchematicExporterTests, SchematicFileV5Tests, SchematicNSDocumentTests, SchematicSidebarViewTests, SchematicUndoTests, SelectionModelTests, SerialMemoryTests, SerialPortTests, SimulationAvailabilityTests, SimulationSpeedTests, SlideSwitchTests, SocketEditorTests, SocketModeTests, SocketPartsTests, SocketPickerTests, SymbolPaintSweepTests, TactileSwitchTests, TerminalConnectorTests, TerminalWindowTests, TestPointTests, TimerOpampTests, TraceToolTests, TransistorTests, UserLibraryCatalogTests, UserLibraryEndToEndTests, UserLibraryPromptsTests, UserLibraryUpdateTests, UserLibraryWindowTests, UserLibraryWindowViewTests, UserPartLibraryTests, VerifyDatasheetTests, ViewProbeTests, WizardIdentityTests, WizardIssueLocatorTests, WizardKindStepsTests, WizardReviewPagesTests, WizardStepPagesTests.
- [x] ratchet rows: parent (every file at 0 in the run above).
  Ratchet rows: removed by the parent in this letter's commit (41 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: —

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

- (2026-11-26) Code with no caller, deleted: `RoutingGrid.viaValue` (only
  `traceValue` is read); `BoardFormula.init(_:)`; in `Router.Run.fanOut` the
  "no trace" fallback (the tie is a pad centre, never a via target, so
  `Router.copper` always makes the trace; the fallback could only have made a
  zero-length one); in `Router.Run.join` the guard that emptied a detour's
  victims when one was a chain (a relaxed search without a push price can't
  cross a chain's cells, `RoutingSearch` `tracePrice` returns -1 there).
- (2026-11-26) More code no caller reaches, deleted: `PartVisibility.allows`
  (the store filters flags with a bit mask); in the Design Rule Check's
  `checkLayers`, the lines for a trace on a layer the board lacks or on a
  Plane and for a part or text on Bottom of a 1-layer board — the editor
  refuses those states (`canSetLayerCount`, `canSetLayerType`, `addTrace`)
  and so does the file reader (`badPCB`), so only a Top part's opposite pad
  is left to check there.
- (2026-11-26) Loops whose end was unreachable code: `SimulationEngine.run`
  is `while runFrame() {}` (one frame per call, false to end the thread);
  `Router.Run.finishedCopper` loops `while done == nil` and returns after.
  `DesignRuleCheckWizard` sets its `@State` in an `init` (xccov counts no
  `@State` default).
- (2026-11-26) `PartFacts.inputRange(minMV:maxMV:)` is the regulator's and the
  power switch's Input row (it was written twice); `BackendCatalog` loads from
  a list of files (`load(into:files:)`, `loadSupportFiles(into:datasheets:footprints:)`)
  and `PartCatalogStore.load(using:)` from a loader, so tests give missing and
  broken files and a failing load. Backend failures the store can't meet (a
  loaded part not found by its uuid, a registered part the backend can't
  retire, the made-up supply text refused, a registered part with no text)
  are one guard line each.
- (2026-11-26) Restructured so every branch is reached: `RoutingSearch` and
  `RoutingDemand` `generation` are internal, so a test wraps them at
  `UInt32.max`; `RoutingState.replace` keeps an entry's place in one path (it
  is always in its net's list); `RoutingPair` folds the "two one-cell runs"
  case (two vias at one cell, which no cheapest path has) into one guard;
  `MemoryFileAccess.resolve` hands a stale bookmark to `kept(_:bookmark:stale:)`;
  `BoardCatalog.shared` is `load(_:)` of the bundle's URL; `UserPartLibrary`
  works its folder out in `directory(environment:)`; `SimulationController`'s
  `circuit_create` failure (out of memory only) is one guard line, and `apply`
  is internal with `generation` readable, so a test feeds it frames.
- (2026-11-26) Logic moved out of views into tested types, the views calling
  them: `MemoryInspectorModel` and `MemoryFormat` (the memory window's page,
  highlights, address field, EEPROM start contents and load file; it reads a
  `MemorySource`, which `SimulationController` is); `BillOfMaterialsModel`
  (checkmarks, summary, copy, both exports, its save panel and alert through
  an `ExportPresenter`); `DesignRuleCheckResults` (step 2's check, settle
  delay, selection, copy, subtitle) and `DesignRuleCheckRules.choose` /
  `set` (step 1's manufacturer and typed rules); `AutorouteModel.groupLine`.
  The menu items' views (`BillOfMaterialsMenuItem`, `DocumentInfoMenuItem`,
  `DesignRuleCheckMenuItem`, `SimulationMenuItems`) are internal so a render
  test draws them with an editor's closures.
- (2026-11-26) Injected so tests never open a device or a modal:
  `MemoryFileAccess.choose(attachedTo:run:)` (the Open panel through
  `ExportPresenter.run`, `openPanel()` its configuration; the real sheet is
  put up on a test window and cancelled); `BuzzerSound(makeEngine:)` and
  `idleDelay` (tests render offline), `SimulationController.soundAllowed` /
  `makeSound`.
- (2026-11-26) `ViewProbe` gained additive helpers: `type(_:intoField:)` and
  `type(_:intoFieldAt:)` (types and presses Return through the field editor)
  and `choose(_:inPopUp:)` (a menu-style picker's item, without opening it).
  Two SwiftUI pickers' bindings did not take a pop-up pick in a hosted window
  (the Design Rule Check's Manufacturer, the Bill of Materials' row
  checkmark); each binding is one line that only forwards to a tested model
  call (`DesignRuleCheckRules.choose`, `BillOfMaterialsModel.setChecked`).
- (2026-11-26) Test classes that open windows set
  `AppDelegate.quitsAfterLastWindowCloses = false`: an async test awaiting
  with no window open otherwise lets the host app quit (exit 0).

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
