# plan-31-D: Edit › Undo and Redo

Last updated: 2026-11-09
Effort: medium (1h–2h)
Depends on: plan-31-C

The Edit menu's Undo and Redo items act on the page the focused window
shows: "Undo Move" / "Redo Add Wire", ⌘Z / ⇧⌘Z. While a text field or the
note editor is being edited, ⌘Z is that field's own typing undo, as in any
Mac app. Undo is off while the simulation runs. This finishes plan-31 and
closes both undo items in `planning/todo.md`.

References:

- `planning/plan-31-A-undo-history.md` — engine and Decisions (menu only, no
  toolbar buttons; the forwarding probe)
- `schematical/schematical/Document/SchematicFileController.swift` —
  `SchematicFileCommands`, `FocusedValues.schematicFile` (the precedent for
  window-scoped menu commands)
- `schematical/schematical/schematicalApp.swift` — `.commands { … }`
- `schematical/schematical/ContentView.swift` — `page`,
  `.focusedSceneValue(\.schematicFile, file)`

## Prerequisites

See plan-31-A § Prerequisites. Also:

| Must be true | Command | Status |
|---|---|---|
| plan-31-C is complete | `ls planning/plan-31-C-*` → no match (archived) | MET (2026-11-09: `ls planning/plan-31-C-*` → no match; archived at 8f427dc) |

## 1. Goal

- In a schematic window, Edit › Undo undoes the shown page's last step and is
  titled "Undo <step name>" ("Undo" when the page has none); Redo likewise.
  Switching pages switches which list the items act on.
- With an editable text view as first responder, or in a window with no
  schematic (memory, CPU, registers, terminal, BOM, datasheet), ⌘Z / ⇧⌘Z go
  to the responder chain as the default Edit menu does today.

### Non-goals (explicit constraints)

- No toolbar buttons (A § Decisions).
- No change to File menu commands or to any other menu.

## 2. Current State

- The app has no Edit menu customisation:
  `rg -n 'CommandGroup' schematical/schematical` → `SchematicFileController.swift:333`
  (`after: .newItem`) and `BillOfMaterialsWindow.swift:114`
  (`before: .windowList`). SwiftUI's default Edit › Undo/Redo send `undo:`
  / `redo:` to the first responder, which only text views use today.
- `SchematicFileCommands` reads `@FocusedValue(\.schematicFile)`, set by
  `ContentView` with `.focusedSceneValue(\.schematicFile, file)`. A focused
  scene value is set for as long as that window is key, including while a
  sidebar field inside it has focus.
- `ContentView` owns `@State private var page = EditorPage.schematic` and
  `simulation` (`isRunning`).
- `SchematicDocument` is `@Observable`; A's `history` is a stored property
  (not `@ObservationIgnored`), so a menu title that reads
  `undoName(on:)` updates when the lists change.
- Probe (A § Verified properties): `NSApp.sendAction(Selector(("undo:")), to:
  nil, from: nil)` with an `NSTextField` editing undoes its typing; `redo:`
  redoes it.

## 3. Design

New file `schematical/schematical/Document/UndoCommands.swift`:

```swift
/// The focused schematic window's document and the page it shows.
struct UndoTarget {
    let document: SchematicDocument
    let page: EditorPage
}

extension FocusedValues {
    @Entry var schematicUndo: UndoTarget?
}

/// What ⌘Z / ⇧⌘Z do.
enum UndoRoute: Equatable {
    case responderChain   // text editing, or no schematic window
    case page(EditorPage)
}

enum UndoRouting {
    static func route(textIsEditing: Bool, target: UndoTarget?) -> UndoRoute
    /// "Undo Move", or "Undo" with nothing to undo or while playing.
    static func undoTitle(_ target: UndoTarget?) -> String
    static func redoTitle(_ target: UndoTarget?) -> String
}

struct UndoCommands: Commands {
    @FocusedValue(\.schematicUndo) private var target
    var body: some Commands {
        CommandGroup(replacing: .undoRedo) {
            Button(UndoRouting.undoTitle(target)) { perform(undo: true) }
                .keyboardShortcut("z", modifiers: .command)
            Button(UndoRouting.redoTitle(target)) { perform(undo: false) }
                .keyboardShortcut("z", modifiers: [.command, .shift])
        }
    }
}
```

`perform(undo:)`: `textIsEditing = (NSApp.keyWindow?.firstResponder as?
NSTextView)?.isEditable == true`. `.responderChain` →
`NSApp.sendAction(Selector(undo ? "undo:" : "redo:"), to: nil, from: nil)`.
`.page(p)` → `target.document.undo(on: p)` / `redo(on: p)`; when it returns
false (nothing to do, or playing) → `NSSound.beep()`.

The items are never disabled. The first responder is AppKit state that
SwiftUI does not observe, so an enabled state computed from it would go
stale, and a disabled item would also block a text field's own undo. Titles
come from the page list, which is observed.

`ContentView`: `.focusedSceneValue(\.schematicUndo, UndoTarget(document:
document, page: page))`. `schematicalApp`: add `UndoCommands()` to
`.commands`.

`undo(on:)` already returns false while `isPlaying` (A), so a run needs no
extra check here.

Rejected: disabling items from `undoName == nil` (blocks text undo when the
page list is empty); a toolbar button (owner said menu only).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; moot as `- [x] ~~text~~ — moot:
> <evidence>`; fill `Commit:` when it lands. **An unticked box means NOT DONE.**

### Phase 1 — Menu commands

- [x] `Document/UndoCommands.swift` per §3.
- [x] `ContentView.swift`: the `schematicUndo` focused scene value.
- [x] `schematicalApp.swift`: `UndoCommands()` in `.commands`.
- [x] Tests: `schematical/schematicalTests/UndoCommandsTests.swift`:
      `route(textIsEditing: true, target: x)` → `.responderChain`;
      `route(false, nil)` → `.responderChain`; `route(false, target on .pcb)`
      → `.page(.pcb)`; `undoTitle` is "Undo" for an empty page, "Undo Flip"
      after a recorded `flipChip`, "Undo" while `isPlaying`, and follows the
      target's page (a Schematic step does not title the PCB target unless
      it is a two-page step); `redoTitle` after an undo is "Redo Flip".
      6 tests. Check run → `Executed 6 tests, with 0 failures`,
      `** TEST SUCCEEDED **`.

Acceptance: the routing and title cases pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/UndoCommandsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: 03c4ef5

### Phase 2 — In the app, and the to-do list

- [x] Drive the built app (the `run` skill; AGENTS.md: pointer back to
      (0, 0) when done), with screenshots of the Edit menu:
      1. Schematic: drag a part from the drawer, move it, flip it → Edit
         shows "Undo Flip"; ⌘Z three times → canvas empty; ⇧⌘Z → part back.
      2. Erase a part with a wire on a pin, ⌘Z → wire back on the pin
         (plan-30 note).
      3. PCB page: move a footprint → "Undo Move"; switch to Schematic →
         Edit shows the Schematic list's step, not the PCB move; back to PCB,
         ⌘Z → footprint back.
      4. Select a board text, type in its text field, ⌘Z while the field is
         focused → the typing is undone, not a page step.
      5. ⌘R to run, ⌘Z → beep, nothing changes; stop.
      Done 2026-11-09 with the Debug build from the worktree (`open -a
      /tmp/p31dd/Build/Products/Debug/schematical.app`, ⌘N for a window,
      `/tmp/p25drive/drive` for mouse and keys, screenshots in
      `/tmp/p31shots/`): 1. Edit showed "Undo Flip"; three ⌘Z left the
      canvas empty; ⇧⌘Z put the part back (s6, s7, s8). 2. Erase of U2 with
      a wire to its 1A, ⌘Z → U2 back, wire on 1A, no loose end (s10, s11).
      3. PCB: dragged U2's footprint → "Undo Move"; on Schematic, Edit showed
      "Undo Add Wire"; back on PCB ⌘Z put the footprint back (s13, s14,
      s15). 4. Board text field: typed "ABC" ("TextABC"), ⌘Z with the field
      focused → "Text", the board text stayed (s17, s18). 5. ⌘R ran, ⌘Z
      changed nothing and Edit read plain "Undo"; after stopping, "Undo Add
      Wire" again (s19, s20, s21). The beep itself is not visible in a
      screenshot; `undo(on:)` returning false while playing is covered by
      `UndoHistoryTests`. Pointer moved to (0, 0); the app instance closed.
      Final gate (full app target) → `Executed 329 tests, with 0 failures`,
      `** TEST SUCCEEDED **`.
- [x] `planning/todo.md`: tick both undo items ("Add full undo/redo … on the
      schematic", "… on the PCB"), with "(plan-31)". Done.

Acceptance: all five drive steps behave as listed (screenshots recorded in
this plan's Corrections only if something differed).
  Check: the five drive steps above → as listed (est. 10 min; a person-level
  check of menu wiring that unit tests cannot see — first responder and
  focused scene values only exist in a running app).
Commit: this commit (D is archived in it; hash in the merge report)

## Validation Plan

- Tests: `UndoCommandsTests`; with A's `UndoHistoryTests`, B's
  `SchematicUndoTests`, C's `PCBUndoTests`.
- Coverage: the routing and titles are pure functions, tested; the AppKit
  calls in `perform` are covered by the drive.
- Runtime proof: Phase 2's drive.
- Doc sync: `planning/todo.md` (Phase 2). No `docs/` file covers editing
  (`ls docs` → `backend.md export.md file-format.md parts-format.md`), and the
  file format does not change.
- Final gate (once, after Phase 2): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min; the landing tier for the whole of plan-31, since A–D all touched shared model and window code).

## Decisions

See plan-31-A § Decisions. Author's calls here: the items are never disabled
(§3); ⌘Z with nothing to undo beeps.

## Corrections

1. Drive step 3 expected the Schematic page to show "the Schematic list's
   step"; it did ("Undo Add Wire"). Its Redo read plain "Redo": the PCB
   move cleared the shared "Delete" redo step from both pages, as plan-31-A
   §4.1 specifies for a shared step. As designed; noted so the next reader
   does not take it for a bug.

## Summary

Low risk: a menu group, a focused value and two pure functions. The one
behavior to watch is text-field undo, which the probe and drive step 4
cover.
