# plan-46-G: Part Wizard — step 3 (datasheet), the PDF pane, Find in datasheet

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-F

G makes the datasheet the user's companion for the rest of the wizard.
Step 3 takes an `https://` URL, saves it in the part as an inline
datasheet (A), and opens the PDF in the wizard window's right-hand pane,
which gains a **Datasheet** tab beside Block and Footprint and stays open
on every later step. Every step (and, from L, every field) has **Find in
datasheet** buttons that search the PDF for the section titles that step's
information usually sits under, jumping to the first hit and on to the
next with each press. A site that refuses the download can be worked
around with **Open a downloaded PDF…**: the local file is only viewed; the
part keeps the URL.

Behavioral outcome: pasting `https://www.ti.com/lit/ds/symlink/sn74hc00.pdf`
on step 3 shows the PDF in the pane; on the Pins step, Find in datasheet
jumps to the page holding "Pin Configuration and Functions"; the saved
`.schpart` carries the inline entry, and the placed part's Open Datasheet
button opens that URL.

References:

- `schematical/schematical/Datasheet/DatasheetWindow.swift` (the download
  with a browser user agent, `PDFDocument(data:)`, the failure messages,
  `PDFDocumentView`), `OpenDatasheetButton`.
- `PartDraft.setDatasheetURL(_:)` / `datasheetURL` (A),
  `docs/parts-format.md` § Datasheets (https only, manufacturer's site),
  `library.c:660–668` (a datasheet needs a manufacturer; Generic has none).
- `Editor/PartEditorWindow.swift` `PartPreviewPane` (the Block | Footprint
  segment), `PartEditorModel.PreviewPane`.
- App sandbox: `ENABLE_OUTGOING_NETWORK_CONNECTIONS = YES`,
  `ENABLE_USER_SELECTED_FILES = readwrite`
  (`schematical.xcodeproj/project.pbxproj`, both configurations).
- PDFKit: `PDFDocument.findString(_:withOptions:)`, `PDFView.go(to:)`,
  `PDFView.setCurrentSelection(_:animate:)`.

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-F complete | `ls planning/completed/plan-46-F-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-F-wizard-kind-and-identity-steps.md`) |
| Inline datasheets load (A) | `ctest --test-dir /tmp/logic-p46 -R part_file_test` → passed | MET (2026-11-16 at d0aefc6b: `part_file_test … Passed`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Datasheet/DatasheetLoader.swift`: the download and checks moved out of
  `DatasheetWindowView` (`func load(_ url: URL) async -> Result<PDFDocument,
  DatasheetLoader.Failure>`, the same user agent and messages); the window
  uses it; `PDFDocumentView` becomes internal and takes a `DatasheetFinder`.
- `Datasheet/DatasheetFinder.swift` (`@Observable`): holds the document and
  a weak `PDFView`; `find(_ terms: [String])` tries each term in order
  (case-insensitive), keeps the hits of the first term that has any, and
  moves to the next hit on each call with the same terms (wrapping);
  returns the page index (1-based) or nil; `lastResult` text for the UI
  ("Page 3 of 18 — “Pin Configuration and Functions”" / "Not found:
  Pin Configuration, Pin Functions, …").
- Step 3 page `DatasheetStepPage`: URL field; `http://` is offered as
  `https://`; anything else not `https://` shows "The address must start
  with https://"; a Generic manufacturer shows "A datasheet needs a
  manufacturer" with a Go to Part number button; Load fetches into the
  pane; Open a downloaded PDF… (`NSOpenPanel`, PDF only) views a local
  copy; a bundled datasheet (a copied part) shows its id and URL with
  Replace.
- The preview pane in Guided mode: `Datasheet | Block | Footprint`; it
  switches to Datasheet when a PDF loads.
- `WizardStep.searchTerms` for the generic steps (data in
  `Editor/Wizard/StepSearchTerms.swift`):
  identity — "Ordering Information", "Package Option Addendum",
  "Orderable Part Number", "Device Information";
  pinout — "Pin Configuration and Functions", "Pin Configuration",
  "Pin Functions", "Pin Description", "Pin Assignments",
  "Terminal Functions", "Pinout";
  supply — "Recommended Operating Conditions", "Operating Conditions",
  "Absolute Maximum Ratings";
  footprint — "Land Pattern", "Recommended Land Pattern",
  "Example Board Layout", "Recommended Footprint", "PCB Layout",
  "Mechanical Data", "Package Outline", "Package Dimensions".
  Kind steps' terms come from guides (L); a derived step has none and
  shows no button.

### Non-goals (explicit constraints)

- **The PDF is never stored** in the part, the schematic or the app's
  container; only the URL is.
- **No PDF text extraction into fields** (no automatic filling).
- **The inspector's Open Datasheet button is unchanged**; it works for
  custom parts because A resolves the inline entry.

## 2. Current State

- `DatasheetWindowView.load()` downloads with `URLSession.shared` and a
  `Mozilla/5.0 (Macintosh) Schematical` user agent, refuses non-2xx and
  non-PDF bodies with specific messages, and has Try Again / Open in
  Browser (`DatasheetWindow.swift:38–96`).
- `PDFDocumentView` is `private` to that file (`:99`).
- Some manufacturers refuse automated fetches even with a browser agent
  (Silicon Labs answers 403; memory note `datasheet-download-blocks`), so
  the local-file fallback is needed, not optional.

## 3. Design notes

- One `DatasheetFinder` per editor window (`PartEditorModel.datasheet`),
  so the pane and every step's button share the loaded document.
- Find runs `findString` on the main actor; the PDFs are tens of pages and
  `findString` over a document is a single call per term.
- The step buttons sit in the step header: "Find in datasheet" (the step's
  terms); L adds a small magnifier per field.

Rejected: a web view of the URL — PDFKit's search and paging are what
Find needs, and the existing window already proves PDFKit works here.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Loader and finder

- [x] `DatasheetLoader.swift`, `DatasheetFinder.swift`;
      `DatasheetWindowView` uses the loader (behavior unchanged; the
      runtime walk opened a datasheet window through it).
- [x] `schematicalTests/DatasheetFinderTests.swift`: a 4-page PDF made in
      the test (each page draws one heading with `NSAttributedString`
      into a `CGContext` PDF: "Features", "Pin Configuration and
      Functions", "Pin Functions", "Pin Functions (continued)") →
      `find(["Pin Configuration", "Pin Functions"])` returns 2, and 2 again
      on the next call (the first term with hits wins, and it has one);
      `find(["Pin Functions"])` returns 3, then 4, then wraps to 3;
      `find(["Thermal"])` returns nil with the "Not found" text.
- [x] URL rule tests in the same file: `http://x/y.pdf` → offered
      `https://x/y.pdf`; `ftp://…` → the message; Generic manufacturer →
      the message. (`xcodebuild … test
      -only-testing:schematicalTests/DatasheetFinderTests` → `Executed 8
      tests, with 0 failures`, `** TEST SUCCEEDED **`; § Corrections 1, 2.)

Acceptance: the finder walks hits as § 1 says.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/DatasheetFinderTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: d23ebe95

### Phase 2 — The step and the pane

- [x] `Editor/Wizard/DatasheetStepPage.swift`; the Guided pane's three
      tabs; `StepSearchTerms.swift`; the step header's Find button.
- [x] Runtime check (run skill): New Part… → Gate → part number
      `MY-HC00`, manufacturer Texas Instruments → step 3: paste the TI URL
      above → the PDF shows → Next to Pins → Find in datasheet → the pane
      shows the pin-configuration page → finish with a bundled footprint →
      Add to Library → place it → Open Datasheet opens the same URL.
      Pointer to (0, 0).
      (2026-11-16, this plan's Debug build: `MY-HC00`, Texas Instruments →
      Datasheet step, address
      `https://www.ti.com/lit/ds/symlink/sn74hc00.pdf` → Load → `Showing
      www.ti.com`, the pane switched to Datasheet and drew the 39-page PDF
      (screenshot checked). Pins → Find in datasheet → `Page 2 of 39 — “Pin
      Configuration and Functions”` (the contents page; § Corrections 1),
      again → `Page 3 of 39`, the pin section. Five pins, a NAND gate, HC
      timing, `SOT23-5` → Review `The part loads: 5 pins` → Add to Library →
      `Added MY-HC00 to the parts drawer` → dragged from Custom › Logic
      gates and flip-flops › Logic Gate onto the canvas and selected → the
      sidebar's Open Datasheet → window `MY-HC00 Datasheet – www.ti.com`
      with 39 pages. Pointer left at (0, 0).)

Acceptance: the datasheet follows the user through the wizard and onto the placed part.
  Check: the walk above (est. 15 min; network and PDFKit rendering are only visible in the app).
Commit: —

## Validation Plan

- Tests: `DatasheetFinderTests`; the existing datasheet window keeps its
  behavior (manual: open any bundled part's datasheet once in Phase 2).
- Runtime proof: Phase 2's walk.
- Doc sync: N (help).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions (inline datasheets).

## Corrections

1. **Hits on a table-of-contents page go last.** The runtime walk's first
   Find on the Pins step landed on `Page 2 of 39 — “Pin Configuration and
   Functions”`: the TI datasheet's contents page, which lists every
   section title; the section itself (page 3) took a second press. § 1's
   "keeps the hits of the first term that has any" still holds, but the
   hits are ordered with those on a page whose text holds "Table of
   Contents" after the others (`DatasheetFinder.contentsLast`), so the
   first press reaches the section. `testATableOfContentsPageComesLast`
   covers it; a title found only on the contents page is still found.
2. **`DatasheetFinder` must outlive a test.** It is a main-actor class, so
   freeing one inside a test aborts the host (the Swift runtime issue of
   plan-46-A § Corrections 1); the tests keep theirs in a static list, as
   the model tests do. In the app the finder lives as long as its window's
   model.
3. **All fields keeps its two-tab preview.** § 1's `Datasheet | Block |
   Footprint` is the Guided pane (`PartEditorModel.GuidedPane`,
   `GuidedPreviewPane`); `PreviewPane` (Block, Footprint) and its test are
   unchanged.

## Summary

The new code is a loader move and a small search wrapper; the risk is
network variance, covered by the local-PDF fallback and the walk.
