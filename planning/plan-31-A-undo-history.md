# plan-31-A: Undo history engine

Last updated: 2026-11-09
Overall Effort: large (3h–1d) — the whole plan-31 feature (A–D)
Effort: medium (1h–2h)
Depends on: nothing

plan-31 adds full undo/redo to the app. The Schematic page and the PCB page
each have their own undo list and their own redo list. Every action that
changes what is saved in the `.schematical` file is one undo step. ⌘Z undoes
the last step on the page being shown, ⇧⌘Z redoes it. A step that changes
both pages (adding or deleting a part, renaming it, changing its footprint)
goes on both lists. Undoing it from either page undoes it on both.

This sub-plan builds the engine: capturing the document's saved state, the
difference between two states as per-item changes, applying those changes
backwards or forwards, the two pairs of stacks, gestures, coalescing and the
play-mode lock. Nothing calls it yet. B wires up the Schematic page, C the
PCB page, D the Edit menu.

When A is done, a test can wrap any existing document edit in
`recordEdit(.schematic, "Flip") { … }`, call `undo(on: .schematic)`, and get
back exactly the document it started with.

References:

- `schematical/schematical/Canvas/SchematicModel.swift` — `SchematicDocument`
  (state, every edit, `replaceContents`, `pruneSelection`)
- `schematical/schematical/Document/SchematicFile.swift` — what is saved
  (the line between undoable and not undoable)
- `schematical/schematical/Document/SchematicFileController.swift` —
  `hasUnsavedChanges` (compares a `SchematicFile` to the saved one)
- `planning/todo.md` — the two undo items, and plan-30's note
- `planning/completed/plan-30-B-loose-ends-auto-connect.md` — the
  auto-connect on drop that must undo with its move

## Prerequisites

Stated once here for all of plan-31; B, C and D point here.

| Must be true | Command | Status |
|---|---|---|
| Stopping the simulation puts switches back where they were at the start (flips made in play mode are not edits, so they never need undoing) | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/PlaySwitchResetTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` | MET (re-run 2026-11-09 at 0113188 in the P-31 worktree: passed inside the full run below, `PlaySwitchResetTests` 2 cases passed) |
| The app test target passes before any plan-31 change | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` | MET (re-run 2026-11-09 at 0113188: `Executed 214 tests, with 0 failures`, `** TEST SUCCEEDED **`) |

Everything below is written for the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- A pure engine (`UndoHistory.swift`) plus a document API
  (`recordEdit`, `beginGesture`/`endGesture`, `endCoalescing`,
  `undo(on:)`/`redo(on:)`, `undoName(on:)`/`redoName(on:)`, `isPlaying`)
  that B, C and D use. Proven by `UndoHistoryTests`, with no app caller.

### Non-goals (explicit constraints)

- No change to the file format (`docs/file-format.md`) or to
  `SchematicFile`. Undo history is never saved.
- No change to any edit's behavior. A recorded edit does exactly what it did
  before; the engine only watches the state before and after.
- Selection, hover, zoom/pan, the open tool, hidden PCB layers,
  `editingNoteID`, `editorMessage` and `pendingRemoval` are not saved, so
  they are never undo steps. Undo does not restore them (selection is pruned
  to what still exists, see §5).
- No use of `NSUndoManager`/`UndoManager` (rejected, §3).

## 2. Current State

There is no undo. `rg -n -i 'undo' schematical/schematical --glob '*.swift'`
finds only the line tool's "Esc: undo a corner" tooltip
(`SchematicModel.swift:SchematicTool.tooltip`, a route-draft key, not a
document edit) and the `setButton` doc comment ("Not undoable").

`SchematicDocument` (`Canvas/SchematicModel.swift`, `@MainActor @Observable
final class`) holds all saved state in these stored properties, each
`private(set)` except `board`:

| Property | Type | Page that shows it |
|---|---|---|
| `chips` | `[PlacedChip]` | both (body on Schematic; `board` placement on PCB; `reference` on both) |
| `wires` | `[Wire]` | Schematic |
| `junctions` | `[Junction]` | Schematic |
| `buses` | `[Bus]` | Schematic |
| `notes` | `[SchematicNote]` | Schematic |
| `nextReference` | `Int` | Schematic (next "U" number) |
| `board` | `BoardSettings` (`var`, public set) | PCB |
| `texts` | `[BoardText]` | PCB |
| `traces` | `[BoardTrace]` | PCB |

Every element type is `Hashable` (so `Equatable`) and `Identifiable` by
`UUID`: `PlacedChip` (`SchematicModel.swift`), `Wire`, `Junction`,
`Bus` (`BusModel.swift`), `SchematicNote` (`SchematicNote.swift`),
`BoardText`, `BoardSettings` (`BoardSettings.swift`), `BoardTrace`
(`BoardTrace.swift`), `BoardPlacement` (`BoardModel.swift`).
`CatalogPart.==` compares `name` only (`Backend/PartCatalog.swift`), so
comparing chips is cheap.

`private(set)` setters are usable only inside `SchematicModel.swift`
(the class and its same-file extensions), so code that writes state back
must live in that file. The pure diff/apply code lives in a new file and
works on a value copy.

`replaceContents(...)` (`SchematicModel.swift`) replaces everything (file
open; test setup) and clears selections. `pruneSelection()` drops selected
ids that no longer exist.

`SchematicFileController.hasUnsavedChanges` compares
`SchematicFile(document:)` with the snapshot taken at the last save/open, so
undoing back to the saved state clears "unsaved" with no extra work.

`EditorPage` (`ContentView.swift`) is `enum { case schematic, pcb }`; the
engine reuses it.

`SimulationController.start(document:)` / `stop()`
(`Simulation/Simulation.swift`) already keep `playDocument` (weak) for the
switch reset (commit 0bc135e). While running, the canvas only pans and flips
switches (`SchematicCanvasView.handleChanged`, `if isRunning`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Schematic edit functions B wraps (overloads counted) | 38 | per-name grep of plan-31-B § Measured populations → 38 |
| PCB edit functions C wraps (overloads counted) | 28 | per-name grep of plan-31-C § Measured populations → 28 |
| Direct writes to `document.board` outside the model | 2 | `rg -n 'document.board.color = \|\$document.board.units' schematical/schematical` → `PCBSidebarView.swift:37`, `:125` |
| Size of the largest sample file (`be-6502.schematical`) | 36 parts, 207 wires, 33 junctions, 2 buses, 0 notes | `python3 -c "import json;d=json.load(open('be-6502.schematical'));print({k:len(v) for k,v in d.items() if isinstance(v,list)})"` |
| App tests before plan-31 | 214 | full test run at 0bc135e → `Executed 214 tests, with 0 failures` |

### Verified properties

- **Text fields keep their own undo when ⌘Z is forwarded.** Probe
  `/tmp/undoprobe/main.swift` (AppKit window, `NSTextField`, typed "abc"
  then "def" through the field editor, then
  `NSApp.sendAction(Selector(("undo:")), to: nil, from: nil)`): output
  `first responder is NSTextView: true`, `undo handled: true after undo: `
  (empty), `redo handled: true after redo: abcdef`. D relies on this.
- **Unchanged arrays compare cheaply.** Edits like `moveSchematicGroup`
  assign only when a value differs (`if chips[index].origin != moved`), so an
  unchanged frame leaves array buffers shared, and Swift's `Array ==` returns
  early on a shared buffer.
- **Cross-page references.** Read `BoardTrace` and `BoardText`: they hold
  board points and layers, no part ids. Wires/junctions/buses reference parts
  (`WireEnd.pin(PinRef)`) and are Schematic-only. So the PCB list can never
  leave a wire pointing at a missing part.

## 3. Design Overview

**State and keys.** `DocumentState` is a value copy of the nine stored
properties (copy-on-write, so capturing it is O(1)). A difference between
two states is a set of `UndoKey`s, each with a before and an after value:

| Key | What it covers | Owner page(s) |
|---|---|---|
| `.part(id)` | a chip's existence and every field except `board` and `reference` (origin, flip, switches, color, gender, ohms, footprint choice, part, memory file) | Schematic |
| `.reference(id)` | the chip's `reference` | both |
| `.footprint(id)` | the chip's `board` placement, plus its `footprintID` and `part.name` (what the PCB draws) | PCB |
| `.wire(id)`, `.junction(id)`, `.bus(id)`, `.note(id)` | one item, including existence | Schematic |
| `.nextReference` | the counter | Schematic |
| `.text(id)`, `.trace(id)` | one item, including existence | PCB |
| `.board` | `BoardSettings` | PCB |

**Steps and lists.** An `UndoStep` has a name ("Move", "Delete", …), an
optional coalescing key, its changes, and its pages. Its pages are the page
the action was made on plus every page that owns a changed key. So deleting
a part (changes `.part` and `.footprint`) is on both lists; renaming (changes
`.reference`) is on both; moving a wire is on the Schematic list only.
`UndoHistory` keeps an undo stack and a redo stack per page. A two-page step
is the same object on both pages' stacks and is always in the same state on
both: undoing it from either page removes it from both undo stacks and
pushes it on both redo stacks.

**Applying.** Undo writes each key's before value, redo its after value,
into the current state. Keys are applied per item, so the two lists stay
independent: undoing a Schematic move touches only that move's parts and
wires, never a footprint the PCB moved since. A key whose part no longer
exists (`.reference`/`.footprint` of a deleted part) is skipped. A step whose
apply changes nothing is dropped from every stack and the next one is undone
in the same command.

**Recording.** `recordEdit(page, name, coalescing:) { body }` captures state,
runs the body, diffs, and pushes a step if anything changed. Calls nest: only
the outermost records. `beginGesture(page)` / `endGesture()` open and close an
outer recording around a whole mouse drag (B, C), so a drag with an
auto-connect on drop is one step. The step's name is the name of the first
inner `recordEdit` that changed state.

**Correctness risk** is in apply: re-inserting deleted items at the right
index, and applying a `.part` re-insert together with its `.reference` and
`.footprint` in the same step. It is all pure code on `DocumentState`, tested
here in A before anything calls it.

**Byte-identity is not this plan's gate.** Behavior is new; gates are tests.

Rejected alternatives:

- **`UndoManager` per page, returned through the window delegate
  (`windowWillReturnUndoManager`).** Text views (the note `TextEditor`, the
  sidebar fields) register their typing in the window's undo manager, so
  typing would mix into the page lists and leave dead text-view actions
  behind after the field closes. Our own stacks keep page steps and text
  typing apart. D forwards ⌘Z to a focused text view instead.
- **Whole-page snapshots** (undo = restore the page's slice of state). A
  rename on the PCB, or a footprint change made from the Schematic drawer,
  changes data both pages show. Restoring a whole slice would also revert the
  other page's later edits to the same part. Per-key changes do not.
- **Recording per stored-property `didSet`, closed at the end of the run
  loop turn.** Loses action names, and steps would depend on run-loop timing
  in tests.
- **Wrapping in the views instead of the model.** The 66 functions (62
  names) are called as `document.<name>(` from 67 places in 9 files
  (for each name, `grep -rnE "document\.<name>\(" schematical/schematical
  --include='*.swift'` outside `SchematicModel.swift`, summed → 67 calls,
  9 files), and a future view could forget. The model is the one place every
  edit passes.

## 4. Detailed Design

### 4.1 `Canvas/UndoHistory.swift` (new, pure, no AppKit)

```swift
/// The saved part of a document, as a value. See plan-31-A.
struct DocumentState: Equatable {
    var chips: [PlacedChip]; var wires: [Wire]; var junctions: [Junction]
    var buses: [Bus]; var notes: [SchematicNote]; var nextReference: Int
    var board: BoardSettings; var texts: [BoardText]; var traces: [BoardTrace]
}

enum UndoKey: Hashable {
    case part(UUID), reference(UUID), footprint(UUID)
    case wire(UUID), junction(UUID), bus(UUID), note(UUID), nextReference
    case text(UUID), trace(UUID), board
    var pages: Set<EditorPage>   // the Owner column of §3
}

/// One key's value on one side of a step. Items carry their array index so
/// a deleted item goes back where it was (clamped to the current count).
enum UndoValue: Equatable {
    case chip(PlacedChip, index: Int)      // for .part, .reference and .footprint
    case wire(Wire, index: Int), junction(Junction, index: Int)
    case bus(Bus, index: Int), note(SchematicNote, index: Int)
    case text(BoardText, index: Int), trace(BoardTrace, index: Int)
    case nextReference(Int), board(BoardSettings)
}

final class UndoStep {                        // a class: one object on two pages
    let name: String
    let coalescing: String?
    var changes: [UndoKey: (before: UndoValue?, after: UndoValue?)]
    var pages: Set<EditorPage>
}

enum UndoDiff {
    /// Every key whose value differs. Chips: `.part` compares the chip with
    /// `board` set to nil and `reference` set to ""; `.reference` compares
    /// `reference`; `.footprint` compares (`board`, `footprintID`, `part.name`).
    /// A chip that appears or disappears changes all three keys.
    static func changes(from: DocumentState, to: DocumentState) -> [UndoKey: (before: UndoValue?, after: UndoValue?)]

    /// Writes `before` (undo) or `after` (redo) of each change into `state`.
    /// Order: `.part` keys first (removals, then insertions by index, then
    /// field copies), then `.reference` and `.footprint` (skipped when the part
    /// is absent), then the other keys (removal when the value is nil, else
    /// replace in place or insert at the clamped index). Returns whether
    /// `state` changed.
    static func apply(_ changes: …, backwards: Bool, to state: inout DocumentState) -> Bool
}
```

`.part` apply on an existing chip copies every field except `board` and
`reference` from the stored chip. Inserting a missing chip inserts the stored
chip whole, which already has its `board` and `reference` from that moment.

`UndoHistory` (struct): `undo: [EditorPage: [UndoStep]]`,
`redo: [EditorPage: [UndoStep]]`, and:

- `push(_ step:)` — for each page in `step.pages`: append to undo; clear that
  page's redo (a two-page step removed this way is removed from the other
  page's redo too). Coalescing: if `step.coalescing` is non-nil and equals the
  top undo step's on its page, and `coalescingOpen` is true, merge instead
  (keys union; before from the older step for keys it has, after from the
  newer; drop keys whose before == after; the merged step's pages = union).
  Any push with a different key, `endCoalescing()`, undo or redo sets
  `coalescingOpen = false`; a coalescing push sets it true.
- Cap: 500 steps per page undo stack; the oldest is dropped from that page
  only (a two-page step dropped from one page stays on the other, still
  applicable because keys are per item).
- `popUndo(on:)` / `popRedo(on:)` remove the top step of that page from both
  of its pages' stacks of that kind and return it; `pushRedo` / `pushUndo`
  put it on top of both its pages' other stacks.
- `clear()`.

### 4.2 Document API (`Canvas/SchematicModel.swift`, new `// MARK: Undo`)

```swift
private var history = UndoHistory()
private var recordingDepth = 0
private var recordingStart: DocumentState?
private var recordingPage: EditorPage = .schematic
private var recordingName: String?
private var recordingCoalescing: String?
/// True while the simulation runs: edits are not recorded and undo/redo do
/// nothing. Set by SimulationController (plan-31-B).
var isPlaying = false
private(set) var isGestureOpen = false

var state: DocumentState { get }          // a value copy
private func setState(_ s: DocumentState) // writes all nine back, then
                                          // pruneSelection(), clears
                                          // editingNoteID if its note is gone

@discardableResult
func recordEdit<T>(_ page: EditorPage, _ name: String, coalescing: String? = nil,
                   _ body: () -> T) -> T
func beginGesture(_ page: EditorPage)     // closes an open gesture first
func endGesture()
func endCoalescing()
@discardableResult func undo(on page: EditorPage) -> Bool   // closes an open gesture first
@discardableResult func redo(on page: EditorPage) -> Bool
func undoName(on page: EditorPage) -> String?
func redoName(on page: EditorPage) -> String?
```

`recordEdit`: when `isPlaying`, just run the body. Depth 0: capture
`recordingStart = state`, set page, name nil. Every call: snapshot
`let before = state` (O(1)), run body, and if `recordingName == nil &&
state != before` set `recordingName = name` and `recordingCoalescing =
coalescing` (the outermost call's coalescing key wins when it gave one).
Back at depth 0: diff `recordingStart` → `state`; if non-empty push
`UndoStep(name: recordingName!, …, pages: [page] ∪ keys' pages)`.

A gesture is `recordingDepth += 1` without a body; `endGesture` drops the
depth and records like the end of an outer `recordEdit`.

`undo(on:)`: false when `isPlaying`. Loop: pop the page's top step (none →
false); `var s = state`; `UndoDiff.apply(…, backwards: true, to: &s)`; if it
changed, `setState(s)`, push to redo, return true; else drop it and loop.
`redo` mirrors it.

`replaceContents(...)` ends with `history.clear()` and resets the recording
fields (file open starts fresh lists; tests that set up through it start
empty).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial with what remains; moot tasks as
> `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` when a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Pure engine

Types and functions with no caller; safe alone.

- [x] `schematical/schematical/Canvas/UndoHistory.swift`: `DocumentState`,
      `UndoKey` (with `pages`), `UndoValue`, `UndoStep`, `UndoDiff.changes`,
      `UndoDiff.apply`, `UndoHistory` (push with coalescing and two-page
      steps, cap 500, pop/push for undo and redo, `endCoalescing`, `clear`),
      per §4.1. The test target picks the file up by itself (the project uses
      synchronized folders: `grep -c PBXFileSystemSynchronizedRootGroup
      schematical/schematical.xcodeproj/project.pbxproj` → 5). Done; see
      Corrections 1–2 (`UndoChange` struct, `nonisolated` `UndoStep`).
- [x] Tests: `schematical/schematicalTests/UndoHistoryTests.swift`, pure
      cases on hand-built `DocumentState`s:
      diff of a moved chip is `.part` only; of a moved footprint is
      `.footprint` only; of a renamed chip is `.reference` only; of an added
      chip is all three; apply backwards then forwards round-trips every key
      kind; a deleted wire/note/trace comes back at its old index (and at the
      end when the array shrank); `.footprint` of an absent part is skipped and
      apply returns false; a two-page step undone from one page leaves both
      undo stacks and lands on both redo stacks; a new Schematic step clears
      Schematic redo but not PCB redo, except a shared step, which leaves
      both; coalescing merges two pushes with one key and stops after
      `endCoalescing()` or another key; the 501st push drops the oldest.
      Added: deleting an earlier chip changes no later chip; `.part` apply
      keeps today's board and reference; coalescing back to the start drops
      the step; `clear`. Check run → `Executed 17 tests, with 0 failures`,
      `** TEST SUCCEEDED **`.

Acceptance: every listed case passes.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/UndoHistoryTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: —

### Phase 2 — Document API

The recording API on `SchematicDocument`, still with no app caller.

- [ ] `schematical/schematical/Canvas/SchematicModel.swift`: the `// MARK:
      Undo` section of §4.2 (`state`, `setState`, `recordEdit`,
      `beginGesture`, `endGesture`, `endCoalescing`, `undo(on:)`,
      `redo(on:)`, `undoName(on:)`, `redoName(on:)`, `isPlaying`,
      `isGestureOpen`); `replaceContents` clears the history.
- [ ] Tests (same file, `UndoHistoryTests`), on real documents through
      `TestSupport.newDocument()` / `TestSupport.place`:
      `recordEdit(.schematic, "Flip") { doc.flipChip(id) }` then `undo` →
      `doc.state` equals the state before, `redo` → equals after;
      `undoName(on: .schematic) == "Flip"`;
      nested `recordEdit` inside a gesture is one step named by the first
      inner edit that changed something (a no-op `moveSchematicGroup` with
      zero delta, then `toggleSwitch` → "Flip Switch");
      an edit with no change records nothing; `isPlaying` records nothing and
      `undo` returns false; `replaceContents` empties both lists;
      `SchematicFile(document:)` after undo equals the one before the edit
      (the `hasUnsavedChanges` comparison); a step undone after its selected
      part is gone leaves no selected id that does not exist.

Acceptance: the document cases pass, and the Phase 1 cases still pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/UndoHistoryTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `UndoHistoryTests` (Phases 1–2).
- Coverage: the new file is in the app target (synchronized folder); the
  tests call every public function of §4.1 and §4.2.
- Runtime proof: none in A (no caller). B, C and D prove it in the app.
- Doc sync: none (no user-visible change yet).
- Final gate (once, after Phase 2): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min; A adds code to `SchematicDocument`, which every canvas test goes through, so the whole app target runs once).

## Decisions

- Each page has its own undo and redo list; ⌘Z acts on the page shown —
  decided by Justin, 2026-11-09 (the request).
- An action that changes both pages goes on both lists — decided by
  Justin, 2026-11-09 ("removing an item (from PCB or Schematic) effects both
  so should be added to both undos"). Applied as the key-owner rule of §3:
  adding/deleting a part, renaming it, changing its footprint or header size.
- Switch flips made while editing are undo steps; flips made while the
  simulation runs are not, and Stop puts the switches back — decided by
  Justin, 2026-11-09. The reset landed first as a bug fix (0bc135e); plan-31
  only records nothing while `isPlaying`.
- Undo/Redo live in the Edit menu with ⌘Z / ⇧⌘Z; no toolbar buttons —
  decided by Justin, 2026-11-09 (D).
- Author's calls: what is saved is what is undoable (board units and colour
  included); selection is pruned, not restored; 500 steps per page; a step
  that changes nothing when undone is skipped; history survives Save and is
  cleared by opening a file; own stacks, not `UndoManager` (§3).

## Corrections

1. §4.1 writes a change as a tuple `(before: UndoValue?, after: UndoValue?)`.
   It is a struct `UndoChange { before, after }` (Equatable, so tests and the
   coalescing merge can compare it); `UndoChanges` is the dictionary alias.
   Same content.
2. `UndoStep` is `nonisolated final class`. As a main-actor class (the
   target's default isolation) its free aborted the test host:
   `malloc: *** error for object …: pointer being freed was not allocated`
   in every `UndoHistoryTests` case (`/tmp/p31-a1.log`, first run) — the
   isolated-deinit bug `TestSupport.keptDocuments` documents. Nonisolated, 17
   of 17 pass.
3. A chip whose array index moved only because an earlier chip was deleted
   is not a `.part` change (`testDeletingAnEarlierChipDoesNotChangeLaterOnes`);
   items of every kind differ by value only, and an item replaced in place
   keeps today's position.

## Summary

The risk is in `UndoDiff.apply` (re-inserting deleted items and the
part/reference/footprint split). It is pure code, tested here before B and C
route 66 edit functions through it. Nothing user-visible changes in A.
