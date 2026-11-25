# plan-64-D: Document Info — exports, agent tools, docs, landing

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-64-C

The title block leaves the canvas: it is drawn in the image, SVG, PDF and print exports (they share the
canvas's scene), and written into the KiCad and EasyEDA Pro schematics. KiCad's own title block (title, date,
revision, company, comment) also carries the Document Info, shown or not. The agent-control server can read and set
the information and address the block (`titleblock`). The docs are brought in step and the feature lands.

References:

- plan-64-A/B/C (`DocumentInfo`, `TitleBlockLayout`, `SchematicScene.titleBlock`, `SchematicItem.titleBlock`).
- `docs/export.md` (§ KiCad :244-256, § EasyEDA Pro :334-341), `docs/agent-control.md` (:60-125),
  `schematical/schematical/Automation/AutomationSchematic.swift`, `AutomationRead.swift`, `AutomationTools.swift`.
- `schematical/schematical/Document/KiCadSchematicWriter.swift` (`notes()` :657, `extent(of:)` :50, the
  `(title_block` at :117), `EasyEDAProSchematicWriter.swift` (:352).

## Prerequisites

Stated once in plan-64-A; re-run its commands. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-64-C is complete | `grep -c "^Commit: —" planning/plan-64-C-title-block-canvas.md planning/completed/plan-64-C-title-block-canvas.md 2>/dev/null` → `0` for whichever file exists | MET (2026-11-25: 0 unticked boxes, 0 pending Commit lines) |

## 1. Goal

- Every export that draws or writes the schematic includes a shown title block, laid out as on the canvas (the
  KiCad and EasyEDA Pro files as lines and text, like a text block). KiCad's `(title_block …)` carries the title,
  the updated date, the revision, the author and the description. `get_design` shows the information and the
  block; `set_document_info` changes it; `titleblock` works as an item for `select`, `move` and `delete`.

### Non-goals (explicit constraints)

- No PCB export carries it (KiCad board, Gerber, pick-and-place, drill, board image).
- No change to the image export's options or the print/PDF page logic: the block is part of the scene.
- A schematic with no title block exports byte for byte as before, except KiCad's `(title_block …)`, which gains the
  Document Info's fields (empty ones are left out; with no info it is the one `(title "<base>")` line as today).

## 2. Current State

- Image, SVG, PDF and print all build a `SchematicScene(document)` and paint it (`ImageExport.swift:174,257`;
  `PrintExport.swift:164,256,521`); the scene is where C adds the block, so those four need no code.
- The KiCad writer measures the sheet from `extent(of:)` (`KiCadSchematicWriter.swift:50-66`, including
  `for note in document.notes { add(note.rect) }`), draws each note as one `text(…)` plus a `polyline(…)` frame
  (`notes()`, :657-672), and writes `(title_block (title <base name>))` (:117-119). `KiCad.q` quotes a string.
- The EasyEDA Pro writer draws a note as a `poly` frame and a `TEXT` line (:352-361).
- Tool registry: each tool is an `AutomationTool(name:summary:properties:required:edits:run:)`
  (`AutomationSchematic.swift:86-94` `set_note`); `item(_:_:forDelete:)` parses `kind:id` specs, with a no-colon spec
  meaning a part reference (:845); `position(_:_:)` gives `move`'s x/y target (:399); `selectionText` names items
  (`AutomationRead.swift:398-410`); `get_design` has a text form (sections like `NOTES`, :171) and a JSON form
  (`"notes": …`, :279).
- `docs/agent-control.md:74` lists the schematic tools; the MCP `instructions` text is `AutomationGuide.text`
  (`AutomationTools.swift:340-360`) and the doc says to keep the two in step (`docs/agent-control.md` § Conventions).

### Measured populations

| What | Count | Command |
|---|---|---|
| Schematic exporters that draw notes themselves (not through `SchematicScene`) | 2 | `grep -ln "document.notes" schematical/schematical/Document/*Writer.swift` → `EasyEDAProSchematicWriter.swift`, `KiCadSchematicWriter.swift` |
| Tool-test calls to `add_note` / `set_note` in `AutomationTests.swift` (the model for new tests) | 4 | `grep -c '"add_note"\|"set_note"' schematical/schematicalTests/AutomationTests.swift` → 4 |
| Docs naming the notes tools | 1 | `grep -ln "set_note" docs/*.md` → `docs/agent-control.md` |

### Verified properties

- `AutomationSchematic.delete` removes schematic items by selecting them and calling `deleteSchematicSelection()`
  (:912), so C's "titleBlock in the selection hides it" makes `delete titleblock` work with no tool change beyond `item(_:)`.
- `moveItems` moves through `moveGroup(document, items, by:)` (:429), which C's `schematicGroupBase` handles.
- `batch` wraps any tools in one step, so `set_document_info` need only be one `recordEdit` itself.

## 3. Design Overview

- **KiCad / EasyEDA geometry** comes from `TitleBlockLayout` (C): the writers walk `cells` for text (each wrapped
  line, at its `textOrigin`, with the cell's size) and `separators` plus the frame for lines. No second layout.
- **KiCad `(title_block)`:** `title` is the Document Info's title when not empty, else the base name (as now);
  `(date "<updated as yyyy-MM-dd>")` when `updated` is set; `(rev "<revision>")`, `(company "<author>")` and
  `(comment 1 "<description with line breaks as spaces>")` when not empty. Written whether or not the block is shown on the schematic.
- **Agent tools:** `set_document_info` — optional `title`, `author`, `revision`, `description`, `created`, `updated`
  (ISO 8601, e.g. `2026-11-25T14:07:00Z`), `automatic_dates` (bool), `title_block` (bool: show or hide). One
  `recordEdit` "Edit Document Info". `created`/`updated` with `automatic_dates` on (after this call's own change) is
  an error: *"dates are set automatically on save; pass automatic_dates=false to set them"*. `get_design` text form
  gets a `DOCUMENT` section (always printed) — `title`, `author`, `revision`, `created`, `updated`, `automatic_dates`,
  `description`, and `title block: at (x,y) bounds=…` or `title block: off`; the JSON form gets `"document": {…}`
  with `"title_block": null | {"x","y","bounds"}`. `item("titleblock")` → `.titleBlock` (error *"the title block is
  not on the schematic: set_document_info title_block=true"* when off).
- **Risk:** low. The KiCad writer is the one place a malformed file could break a real consumer: the check runs the
  real `kicad-cli` on it (memory: *Export checks (KiCad/Gerber)*).

Rejected alternatives:

- Drawing the title block with KiCad's native title block (it needs a drawing sheet and a page frame the exporter
  does not write): the block is written as lines and text, like the text blocks beside it.
- A tool per field: `set_document_info` mirrors `set_note`/`set_bus`, one call for several fields.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. Use `- [~]` for
> partly done. Mark a moot task `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line when the phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Exports

- [x] `KiCadSchematicWriter.swift`: `extent(of:)` adds `document.titleBlockRect` when shown; a `titleBlock()`
      beside `notes()` (:657) returns the frame (`polyline`, keys `titleblock/frame`), each separator
      (`titleblock/sep<i>`) and each cell line (`text(…, key: "titleblock/<cell>/<line>", justify: "left top")`);
      add it to `drawing` after `notes()` (:107); the `(title_block` (:117) gets the fields in § 3. — done
- [x] `EasyEDAProSchematicWriter.swift`: after the notes loop (:352), the frame and separators as `poly` (open,
      `wireStyle`) and each cell line as `TEXT` with `noteStyle`, from `TitleBlockLayout`. — done
- [x] `SchematicScene` already paints it (C); no change to `ImageExport`/`PrintExport`. — done: no change to ImageExport/PrintExport; SVG test and PrintExportTests pass
- [x] Tests: `KiCadExportTests` — a document with a title block and info → the `.kicad_sch` text contains the title
      and the author as `text` entries, `(title_block` has `(rev "B")` `(company "…")` and `(date "…")`, parens still
      balance (`testFilesAreWellFormed`'s check), two exports of one document are the same bytes (as
      `testSameDocumentGivesSameBytes`), and a document with no title block and default info exports exactly as
      before (`(title "<base>")` only); `EasyEDAProExportTests` — the `.epro` text contains the title; 
      `SchematicSVGTests` — the SVG of a document with a title block contains the title text; `SchematicSceneTests` —
      a scene with only a title block is not `isEmpty` and its ink bounds equal the block's rect. — done: KiCadExportTests.testDocumentInfoAndTitleBlock + testHiddenTitleBlockIsNotDrawnButTheInfoIsKept, EasyEDAProExportTests.testTitleBlockIsOnTheSheet, SchematicSVGTests.testTitleBlockIsInTheSVG; xcodebuild test KiCadExportTests EasyEDAProExportTests SchematicSVGTests SchematicSceneTests PrintExportTests -> ** TEST SUCCEEDED **, 73 tests 0 failures
- [x] Real-tool check: add a `titleblock` sample to `KiCadExportTests.testPrintSamplesWhenAsked` (a document with two
      parts, a filled-in info and the block shown) printing `KICAD_FILE titleblock/<name>.kicad_sch`; run it with
      `TEST_RUNNER_KICAD_OUT=1`, decode the file, run `~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli sch export
      pdf` on it, the exit code must be 0 and the PDF's text must contain the title and author (`pdftotext <pdf> - | grep -c '<title>\|<author>'` → 2). — done: TEST_RUNNER_KICAD_OUT=1 test run printed titleblock.kicad_sch (9732 bytes); kicad-cli sch export pdf exit 0; pdftotext shows Title: Clock board, J. Smith, Revision, the description (grep -c -> 4 lines; the plan said 2: the title and author appear in both the drawn block and KiCads own title block)

Acceptance: the exports carry the block; KiCad opens the file and prints the title.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p64-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/SchematicSceneTests -only-testing:schematicalTests/PrintExportTests` → `** TEST SUCCEEDED **` (est. 12 min: five classes that exercise every changed exporter); then the `kicad-cli` run above → exit 0 and the title in the PDF text (est. 5 min).
Commit: —

### Phase 2 — Agent tools

- [x] `AutomationSchematic.swift`: `AutomationTool(name: "set_document_info", …)` next to `set_note` (:86) with the
      properties in § 3, `edits: true`; its `run` is one `document.recordEdit(.schematic, "Edit Document Info")` that
      applies the given fields through A's setters and `setTitleBlockShown`, refusing dates under automatic dates;
      `item(_:)` (:845): `spec.lowercased() == "titleblock"` (before the no-colon part lookup) → `.schematic(.titleBlock)`
      or the error in § 3; the unknown-item message (:862) lists `titleblock`. — done (DocumentInfo.isoText/isoDate are shared with the file writer)
- [x] `AutomationRead.swift`: the `DOCUMENT` section in the text form (:171 area) and `"document"` in the JSON form
      (:279 area). — done
- [x] `AutomationTools.swift`: one sentence in the `instructions` text (:348): the title block is addressed as
      `titleblock`, and `set_document_info` edits the information. — done
- [x] Tests (`AutomationTests`, following the `add_note` tests at :309-366): `set_document_info` with title, author,
      revision, description and `title_block: true` → `get_design` shows them and the block's bounds; the same call
      is one undo step (`undo` restores the defaults); `created` with automatic dates on is an error;
      `automatic_dates: false` + `created` stores it; `move` with `titleblock` and `dx`/`dy` moves it; `delete`
      with `titleblock` hides it and leaves the information; `select titleblock` with the block off is the error;
      `get_design` JSON has `"title_block": null` when off. — done: AutomationTests.testDocumentInfoAndTheTitleBlock; xcodebuild test AutomationTests AutomationWindowToolsTests DocumentInfoTests -> ** TEST SUCCEEDED **, 54 tests 0 failures

Acceptance: an agent can read and set everything the window can, and address the block.
  Check: `xcodebuild … test -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/AutomationWindowToolsTests` → `** TEST SUCCEEDED **` (est. 9 min).
Commit: —

### Phase 3 — Docs and landing

- [x] `docs/export.md`: in § KiCad (:250-252) add the title block to the list ("the Document Info's title block,
      when shown, as lines and text; KiCad's own title block carries the title, date, revision, author and
      description"); in § EasyEDA Pro (:340) "the title block is lines and `TEXT`"; in § Image and § PDF and print one
      sentence that a shown title block is drawn with the schematic. — done
- [x] `docs/agent-control.md`: `set_document_info` in the schematic tools list (:74); `titleblock` in the item
      conventions (§ Conventions, near `Wires, labels, notes…`, :117); one line on get_design's `DOCUMENT` section. — done
- [x] `docs/file-format.md`: bring A's `## Document info` section to the shape that shipped (the `titleBlock` position,
      the defaults, the dates rule) and record any difference from the plan under Corrections. — done: the shipped file shape (ISO 8601 UTC dates, automaticDates only as false, titleBlock snapped) is what A described; no difference
- [ ] Landing gate — see Validation Plan. Archive plan-64-A…D to `planning/completed/` (`git mv`) in the landing commit.

Acceptance: the docs describe the shipped behavior; the landing gate passes.
  Check: `grep -n "titleblock\|title block" docs/export.md docs/agent-control.md docs/file-format.md | wc -l` → at least 8 lines naming it across the three docs (est. 1 min); the Validation Plan's final gate.
Commit: —

## Validation Plan

- Tests: as each phase says (`KiCadExportTests`, `EasyEDAProExportTests`, `SchematicSVGTests`, `SchematicSceneTests`, `AutomationTests`).
- Coverage check: the xcodebuild output names each new test as `passed`; the KiCad sample test only prints when
  `TEST_RUNNER_KICAD_OUT=1` is set, so the real-tool check is run with it and its `KICAD_FILE titleblock/…` line must appear.
- Runtime proof: the real-window passes in B (Phase 2) and C (Phase 3), and the `kicad-cli` run in Phase 1.
- Doc sync: `docs/file-format.md` (A), `docs/export.md`, `docs/agent-control.md`, the in-app Keyboard Help (B, C).
- Final gate (run ONCE, after the last phase), as one xcodebuild: `xcodebuild -project schematical/schematical.xcodeproj
  -scheme schematical -derivedDataPath /tmp/p64-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/DocumentInfoTests
  -only-testing:schematicalTests/TitleBlockTests -only-testing:schematicalTests/UndoHistoryTests
  -only-testing:schematicalTests/DocumentUndoTests -only-testing:schematicalTests/SchematicUndoTests
  -only-testing:schematicalTests/SchematicNoteTests -only-testing:schematicalTests/SchematicNSDocumentTests
  -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/SelectionModelTests
  -only-testing:schematicalTests/SelectionSidebarTests -only-testing:schematicalTests/SchematicClipboardTests
  -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/KiCadExportTests
  -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/SchematicSceneTests
  -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/PrintExportTests
  -only-testing:schematicalTests/AutomationTests` → `** TEST SUCCEEDED **` (est. 25 min: eighteen classes, none a
  router fixture class; the whole unit bundle took 18 min when last measured (memory *Export checks*) and runs in the background per the
  project's policy, not here). Backend: no backend file changed, so no `ctest` run.

## Decisions

- Exports carry the title block as the canvas draws it; KiCad's title block carries the information whether or not the
  block is shown — author's call (the information is document metadata; KiCad has a place for it).
- `set_document_info` refuses dates while dates are automatic — author's call (they would be overwritten on the next save).
- The block is not in any PCB output — author's call (the request is a schematic title block).

## Corrections

- Phase 1: the real-tool check's grep (`pdftotext … | grep -c '<title>\|<author>'` → 2) counts matching lines, and the
  PDF has four: the title and author are both in the drawn block and in KiCad's own title block (`Title: Clock board`,
  the author as the company). The check's intent (title and author reach KiCad's output) is met; the expected
  count was wrong.

## Summary

The risk is a KiCad file the real tool rejects; the `kicad-cli` check catches it. Everything else reuses the canvas's
layout and the existing tool and writer patterns; PCB outputs and every schematic without a title block are unchanged.
