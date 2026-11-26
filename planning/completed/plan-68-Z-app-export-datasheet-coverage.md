# plan-68-Z: App coverage — Document, Datasheet, BoardPreview

Last updated: 2026-11-26
Effort: large (3h–1d)
Depends on: plan-68-Y

The app files in § 2 are brought to 100% line coverage: 882 lines are
uncovered in them today (12 files, 2287 executable lines).

Prerequisites: plan-68-A's table, and plan-68-Y complete (its rows are gone
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
| `Document/SchematicNSDocument.swift` | 15 | 212 |
| `Document/UserLibraryPrompts.swift` | 12 | 90 |
| `Document/WindowHelpers.swift` | 25 | 37 |
| `Document/ZipArchive.swift` | 0 | 82 |
| `Datasheet/DatasheetFinder.swift` | 3 | 56 |
| `Datasheet/DatasheetLoader.swift` | 11 | 39 |
| `Datasheet/DatasheetWindow.swift` | 56 | 56 |
| `Datasheet/MarkdownDatasheetWindow.swift` | 118 | 233 |
| `Datasheet/VerifyDatasheetWindow.swift` | 293 | 410 |
| `BoardPreview/BoardPreviewWindow.swift` | 66 | 72 |
| `BoardPreview/BoardRenderer.swift` | 52 | 378 |
| `BoardPreview/PartLook.swift` | 231 | 622 |
| **Total** | **882** | **2287** |

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

### Phase 1 — 9 files, `SchematicNSDocument.swift` … `VerifyDatasheetWindow.swift`

- [x] Close every uncovered line of these files per § 3. New classes
  `WindowHelpersTests`, `ZipArchiveTests`, `DocumentMenuTests`
  (SchematicNSDocument, the File menu, UserLibraryPrompts' sheets and modal),
  `DatasheetWindowTests` (loader through a stub `URLProtocol`, the window),
  `DatasheetPageTests` (the web page, its messages, its window and print sheet),
  `VerifyDatasheetTests` (model, window, citations, Option-only items);
  `DatasheetFinderTests` extended. `LOGIC_COV_WORK=/tmp/logic-cov-Zc node
  scripts/coverage.js --app --tests SchematicNSDocumentTests,UserLibraryPromptsTests,UserLibraryUpdateTests,DatasheetFinderTests,MarkdownDatasheetTests,VerifyDatasheetLifetimeTests,CitationLocatorTests,BoardPreviewTests,DatasheetCatalogTests,WindowHelpersTests,DatasheetWindowTests,DocumentMenuTests,ZipArchiveTests,DatasheetPageTests,VerifyDatasheetTests,BoardRendererTests,PartLookTests,BoardPreviewWindowTests --only
  schematical/schematical/` → exit 0, no `path:line` in any of the nine files
  (0 of 212 … 431 lines each; 148 tests passed, 1 skipped:
  `SchematicNSDocumentTests.testClosingAKeyWindowFreesTheDocument`, its own
  skip). Run in the private copy (Corrections).
- [x] ratchet rows: parent (all nine files at 0).
  Ratchet rows: removed by the parent in this letter's commit (11 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: no uncovered line in these files.
  Check: `node scripts/coverage.js --app --tests <the classes the commit
  names> --only <file>` → no `path:line` for each file; `XB -only-testing`
  those classes → pass (est. 10 min: one scoped coverage build and run).
Commit: d686bdd4

### Phase 2 — `BoardPreviewWindow.swift`, `BoardRenderer.swift`, `PartLook.swift`

- [x] As Phase 1, for these files. New classes `BoardRendererTests`,
  `PartLookTests` (every look the catalog gives, 63 of them, drawn top-down,
  angled and on the bottom), `BoardPreviewWindowTests` (the window, toolbar,
  Save Image… and its panel and alert answered from their modal loop). The
  same scoped run → no `path:line` in `BoardPreviewWindow.swift` (0/87),
  `BoardRenderer.swift` (0/353), `PartLook.swift` (0/619). `node
  scripts/source-size.js --check` → exit 0 (new files staged; largest new test
  file `VerifyDatasheetTests.swift`, 377 code lines).
- [x] ratchet rows: parent (all three files at 0).
  Ratchet rows: removed by the parent in this letter's commit (11 rows). Re-measured in the
  shared tree by the parent with this letter's classes: no `path:line` for any of its files,
  no failed test.

Acceptance: as Phase 1.
  Check: as Phase 1; `SZ` → exit 0 (est. 10 min).
Commit: d686bdd4

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

- (2026-11-26) `BoardRenderer.drawPlane` and its call deleted: unreachable. The
  render draws only the side's outer layer (Top or Bottom), and an outer layer
  is always a component layer — `layerTypeProblem` refuses a Plane there and
  the file reader throws on one (`SchematicFile+Board.swift`, "allowed:
  [.component]"). An inner Plane is under the core and is not seen.
- (2026-11-26) Also restructured as unreachable: `CitationLocator.labelRange`'s
  empty-needle guard (a reference with a letter always folds to one; `find`
  refuses an empty needle anyway); `VerifyDatasheetModel.PDFPhase.none` (the
  PDF pane shows only once a part is picked, and picking starts the load:
  the phase starts at `.loading`); PartLook's three no-facts fallbacks for a
  character LCD, a 7-segment and an LCD display folded into optional defaults
  (`part.characterLCD`, `part.display`), same looks; `MarkdownDatasheetWindowView`
  makes its page in an `init`.
- (2026-11-26) Test-run incident: a first `DocumentMenuTests` draft subclassed
  `NSDocumentController` as a recorder, but `NSDocumentController()` hands back
  the shared controller, so two scoped runs sent File › New, Open…, Close All
  and Clear Menu to the test host's real controller. Clear Menu clears the Open
  Recent list of the app's bundle id (the owner's real recents). The recorder is
  gone: Open Recent's items are a view (`RecentDocumentsMenu`) given closures,
  Close All runs for real only with no edited document open.
- (2026-11-26) Measured in a private copy of the worktree (`/tmp/p68z/tree`)
  while other letters' test files did not compile in the shared tree; only
  those non-compiling test files of other letters were left out of the copy.
- (2026-11-26) Seams added for tests, behaviour unchanged: `DatasheetLoader.load`
  takes a `URLSession` (tests use a stub `URLProtocol`); the datasheet window's
  phase moved into `DatasheetWindowModel` with an injected fetch;
  `VerifyDatasheetModel` takes an injected fetch and now holds the search
  field's text and results-list state (`query`, `showsResults`, `queryChanged`,
  `submit`, `clearQuery`, `hideResults`, `choose`, `detail(of:)`,
  `editedText`, `subtitle`), moved out of `VerifyDatasheetWindowView`;
  `DatasheetPage` takes the bundle its viewer is in and an `openExternal`
  link opener; Board Preview's side, view, size and Save Image… moved into
  `BoardPreviewModel` (`chooseURL`, `showError`, `pngData`, `savePanel`);
  `DocumentMenuCommands` acts on `documents` (the shared controller, or a
  test's recorder through `controller`), with `send`, `openRecent`,
  `closeAll`, and Open Recent's items in `RecentDocumentsMenu`;
  `SchematicNSDocument.autosaves` (false in the test host) in place of the
  inline test-host check; `WindowReader`'s view is `WindowReaderView`.
- (parent) **One commit.** The letter's phases landed together, so its Commit lines
  name the same commit.

## Summary

Test writing and logic extraction, file by file, with scoped coverage runs
as the proof.
