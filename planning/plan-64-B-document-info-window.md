# plan-64-B: Document Info — window and ⌘I

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-64-A

The Window menu gets **Document Info** (⌘I). It opens a window beside the document's own, as Bill of Materials
does, with the form for title, author, revision, description, the *Set dates automatically* checkbox, and the
created and updated dates. Edits are undoable steps on the document (A). There is no *Include on Schematic*
toggle yet: it needs the title block, so C adds it to the same form.

References:

- plan-64-A (`DocumentInfo`, the `setInfo…` setters, `setAutomaticDates`).
- `schematical/schematical/BillOfMaterials/BillOfMaterialsWindow.swift` — the precedent: `…Target`, `…Commands`,
  `…MenuItem`, `…WindowView` (the "Schematic Closed" fallback, `SchematicEditorModel.session`).
- `schematical/schematical/Document/ChildWindows.swift` (`ChildWindowOpener`), `Document/EditorMenus.swift`.
- `schematical/schematical/Help/KeyboardHelp.swift`.

## Prerequisites

Stated once in plan-64-A; re-run its commands. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-64-A is complete | `grep -c "^Commit: —" planning/plan-64-A-document-info-model.md planning/completed/plan-64-A-document-info-model.md 2>/dev/null` → `0` for whichever file exists | MET (2026-11-25: 0 unticked boxes, 0 pending Commit lines) |

## 1. Goal

- ⌘I, or Window › Document Info, opens (or brings to the front) one Document Info window per document, on either
  page. Typing in a field changes the document's `DocumentInfo` and shows in Edit › Undo. With *Set dates
  automatically* on, the two date rows are read-only and show the stored dates (or what they will say, if empty);
  off, each is editable with a date-and-time picker.

### Non-goals (explicit constraints)

- No title block, no *Include on Schematic* toggle, no sidebar panel (C).
- No change to the other child windows or to how they close.
- The window edits nothing while the simulation runs: every field is disabled then (the document's undo is locked
  while it runs, `SchematicModel.swift:2154`, so an edit would apply without a step).

## 2. Current State

- Child windows are AppKit windows hosting SwiftUI (`ChildWindowCenter.open`, `ChildWindows.swift:15-45`),
  opened through `ChildWindowOpener.callAsFunction(id:value:)` (:97-132), which switches on the target's type;
  the document closes them all in `close()` (`SchematicNSDocument.swift:236`).
- The menu item is a `Commands` group with `CommandGroup(before: .windowList)` and a view that reads
  `editor?.menus.openBillOfMaterials` so it re-renders when the editor installs its closure
  (`BillOfMaterialsWindow.swift:189-212`); the closure is installed in `ContentView.installMenus()`
  (`ContentView.swift:346-349`), cleared by `EditorMenus.removeAll()` (:40-45), and the window is dismissed in
  `.onDisappear` (`ContentView.swift:808-811`) with an `isBillOfMaterialsOpen` flag (:78). The command groups are
  registered in `schematicalApp.swift:90-97`.
- ⌘I is unused: `grep -rn 'keyboardShortcut("i"' schematical/schematical` → no output (2026-11-25).
- The window view reads the live document with `SchematicEditorModel.session(target.documentID)`
  (`Document/SchematicEditorModel.swift:80`) and shows `ContentUnavailableView("Schematic Closed", …)` when the
  document is gone (`BillOfMaterialsWindow.swift:215-227`).
- The Keyboard Help sheet lists ⇧⌘B as `entry("⇧⌘B", "Bill of Materials")` (`KeyboardHelp.swift:60`), in the
  section both pages show.

### Measured populations

| What | Count | Command |
|---|---|---|
| Child-window targets handled by `ChildWindowOpener` | 7 | `grep -c "case let target as" schematical/schematical/Document/ChildWindows.swift` → 7 |
| Test files that open a child window through the opener | 1 | `grep -rln "ChildWindowOpener" schematical/schematicalTests` → `SchematicNSDocumentTests.swift` |
| Tests that touch `EditorMenus` closures | 0 | `grep -rn "menus\." schematical/schematicalTests` → no output |

### Verified properties

- A fixed `NSWindow` size and `center()` are what the other windows use; no per-document frame is kept
  (`ChildWindows.swift:27-34`): the Document Info window follows that.
- `ChildWindowCenter.open` with a target already open brings the window forward instead of making a second
  (`windows[id]?[AnyHashable(target)]` found → `makeKeyAndOrderFront`, :20-23), so ⌘I twice is one window.

## 3. Design Overview

- **One shared form.** `DocumentInfoForm(document:isLocked:)` is a SwiftUI `Form` section group so C can embed
  the same fields in the sidebar. The window view wraps it in `Form { … }.formStyle(.grouped)`.
- **Bindings call the setters from A directly** (`Binding(get: { document.documentInfo.title }, set:
  { document.setInfoTitle($0) })`). Typing coalesces into one step per field (A).
- **Dates.** `automaticDates` on: each row is a `LabeledContent` with `DocumentInfo.dateText(date)`, or *"Set when
  the file is saved"* when nil. Off: a `DatePicker(selection:displayedComponents: [.date, .hourAndMinute])`
  bound to the date; when the date is nil, a *Set to Now* button in its place that calls `setInfoCreated(Date())`
  (or `setInfoUpdated`). Seconds are not shown or edited; the stored value keeps whatever it has.
- **Description** is a `TextEditor` with a fixed 96 pt height and monospaced font, as the Text tool's sidebar
  field is (`SchematicSidebarView.swift` `NotePanel`).
- **Risk:** low; it is UI over A's setters. The one trap is the menu item's enabled state (a focused value
  delivered once, see `EditorMenus` header comment): it must read the closure through `editor?.menus`, as BOM does.

Rejected alternatives:

- A sheet or a popover instead of a window: the request is a window; a window can stay open while editing the
  schematic.
- A SwiftUI `WindowGroup` scene: the editor is hosted in an `NSDocument` window and has no scene to open it from
  (`ChildWindows.swift:3-8`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. Use `- [~]` for
> partly done. Mark a moot task `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line when the phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The window

- [x] New folder `schematical/schematical/DocumentInfo/`, file `DocumentInfoWindow.swift`:
      `nonisolated struct DocumentInfoTarget: Codable, Hashable, Sendable { let documentID: UUID }`;
      `struct DocumentInfoForm: View` (fields and rules in § 3; takes `document: SchematicDocument` and
      `isLocked: Bool`, applies `.disabled(isLocked)`); `struct DocumentInfoWindowView: View` (session lookup,
      `isLocked: session.simulation.isRunning`, fallback `ContentUnavailableView("Schematic Closed",
      systemImage: "info.circle", description: Text("The schematic this information belongs to is no longer open."))`,
      `.frame(minWidth: 380, minHeight: 440)`). — done
- [x] `ChildWindows.swift`: a `case let target as DocumentInfoTarget:` in `ChildWindowOpener` →
      `center.open(target, document: documentID, title: "Document Info", size: NSSize(width: 420, height: 520))`
      hosting `DocumentInfoWindowView`; update the header comment's list (:3-5) and `ChildWindowOpener`'s "seven"
      (:93) to eight. — done
- [x] Tests: in `schematicalTests/SchematicNSDocumentTests.swift`, beside `testChildWindowsBelongToTheirDocument`
      (:213), `testDocumentInfoWindow` using that file's `openDocument()`: `ChildWindowOpener(documentID:)(id:
      "documentInfo", value: DocumentInfoTarget(documentID:))` makes one window titled "Document Info" in
      `ChildWindowCenter.shared.windows(of:)`, not restorable; a second open keeps the count at 1; `a.close()` closes it. — done: testDocumentInfoWindow passed (xcodebuild test SchematicNSDocumentTests KeyboardHelpTests -> ** TEST SUCCEEDED **, 27 tests 0 failures)

Acceptance: the opener makes, re-uses and closes the window.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p64-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SchematicNSDocumentTests` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: ab34dfd0

### Phase 2 — ⌘I, the menu, the help line

- [x] `EditorMenus.swift`: `var openDocumentInfo: (() -> Void)?` (doc comment `/// Window › Document Info.`); set
      it to nil in `removeAll()`. — done
- [x] `ContentView.swift`: `@State private var isDocumentInfoOpen = false` (:78); in `installMenus()` (:346)
      `menus.openDocumentInfo = { isDocumentInfoOpen = true; openWindow(id: "documentInfo", value:
      DocumentInfoTarget(documentID: documentID)) }`; in `.onDisappear` (:808) dismiss it like BOM. — done
- [x] `DocumentInfoWindow.swift`: `struct DocumentInfoCommands: Commands` (`@FocusedValue(\.schematicEditor)`,
      `CommandGroup(before: .windowList)`, a `DocumentInfoMenuItem` view reading `editor?.menus.openDocumentInfo`,
      `Button("Document Info") { open?() }.keyboardShortcut("i", modifiers: .command).disabled(open == nil)`);
      register `DocumentInfoCommands()` in `schematicalApp.swift` after `BillOfMaterialsCommands()` (:92). — done
- [x] `KeyboardHelp.swift`: after the ⇧⌘B entry (:60) add `entry("⌘I", "Document Info: title, author, revision, dates and description")`. — done
- [x] Tests: `KeyboardHelpTests.swift` — both pages list `⌘I` (add it to `testSchematicKeys`' list and a
      `testPCBListsDocumentInfo` assertion for `.pcb`); `SchematicNSDocumentTests.swift` — `model.menus.
      openDocumentInfo` assigned a closure is nil again after `model.menus.removeAll()`. (No test renders the
      editor view, so `installMenus` itself is proven by the real-window pass below.) — done: testPCBListsDocumentInfo and testDocumentInfoMenuClosureIsRemoved passed
- [ ] Real-window pass (the owner's rule: the GUI is the proof for a menu): build, launch, open a schematic,
      press ⌘I → one "Document Info" window; type a title → Edit menu shows "Undo Edit Title"; tick/untick *Set
      dates automatically* → the date rows lock/unlock; ⌘S → *Updated* shows the save time; ⌘I again → still one
      window. Quit the app afterwards and move the pointer to (0, 0) (AGENTS.md; see memory *GUI probe etiquette*:
      no modal left up, kill the app after each command).

Acceptance: ⌘I opens the one window, on both pages, and the menu item is disabled with no editor.
  Check: `xcodebuild … test -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/SchematicNSDocumentTests` → `** TEST SUCCEEDED **` (est. 10 min), plus the real-window pass above, whose observed results are written under this phase when done.
Commit: —

## Validation Plan

- Tests: `SchematicNSDocumentTests` (the new `testDocumentInfoWindow`; child windows still close with the document), `KeyboardHelpTests`.
- Coverage check: the xcodebuild output names `Test Case '-[schematicalTests.SchematicNSDocumentTests testDocumentInfoWindow]' passed` (grep for it; a green run that does not name the test proves nothing).
- Runtime proof: the real-window pass in Phase 2.
- Doc sync: `Help/KeyboardHelp.swift` (the in-app help is the keyboard doc); the file-format doc is A's.
- Final gate: runs once in plan-64-D.

## Decisions

- The command is **Window › Document Info (⌘I)**, beside Bill of Materials — author's call: the request names a
  window, and BOM is the precedent for a per-document window with a Window-menu command.
- The fields are disabled while the simulation runs — author's call (the document's undo is locked then).
- *Include on Schematic* lives in the form from C, not here — author's call (it needs the title block).

## Corrections

(Filled in during execution.)

## Summary

The risk is the menu item's enabled state, which depends on a focused value delivered once; it is avoided by
reading the closure through `EditorMenus` as Bill of Materials does. Nothing else in the app changes.
