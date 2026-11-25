# plan-59 probe catalogue

The shared definition of every probe the spike runs, the verdict rules and the
rule that picks the migration route. plan-59-A, -B and -C run probes from this
file; plan-59-D applies the selection rule to `findings.md`.

## Candidates

- **G** — SwiftUI `DocumentGroup` with a `ReferenceFileDocument` that wraps the
  existing `SchematicDocument` (`schematical/schematical/Canvas/SchematicModel.swift:1446`).
  Built in plan-59-A (core) and plan-59-B (app integration).
- **H** — an `NSDocument` subclass whose window controller hosts `ContentView`
  in an `NSHostingController`, with the SwiftUI `App` keeping only the
  auxiliary scenes, Settings and Commands. Built in plan-59-C.

Both are built in detached worktrees of base commit `2d10a029` (`.claude/` is
git-ignored: `git check-ignore -v .claude/worktrees/x` → `.gitignore:15:.claude/`).
Neither is ever merged. No branch is created (AGENTS.md § Git).

## Verdicts

Every probe gets one verdict per candidate, written to `findings.md`:

- `works` — public API, behaves as a system document app behaves.
- `workaround` — behaves correctly, using public API only. The evidence cell
  names the API used. **Not a workaround:** replacing or proxying a window's
  delegate (what `WindowCloseGuard` does today), method swizzling, private API
  or private selectors, posting synthesized events, observing window
  notifications to repair what the framework did (what `tabNextWindow` does
  today). Those get `blocked`. `NSViewRepresentable` and AppKit classes used
  to read or set window properties are public API and count as a workaround.
- `blocked` — no public-API way found. The evidence cell says what was tried.

Each verdict row cites evidence: a command and its output, or a screenshot
saved under `planning/plan-59-data/evidence/<probe>-<candidate>.png`.

## Selection rule (applied once, in plan-59-D)

1. A candidate is **viable** when every probe not marked `info` has `works` or
   `workaround`.
2. Both viable → G. G is the standard SwiftUI document route; H is only chosen
   when G is not viable.
3. Only one viable → that one.
4. Neither viable → the candidate with fewer `blocked` rows; every `blocked`
   row becomes a question to the owner, asked before the migration plan is
   written (plan-59-D phase 2).

Undo is settled before any probe runs: the owner decided (2026-11-24) that
undo is **per document, on one `NSUndoManager`**. The two per-page lists
(`UndoHistory.swift`) go away; a step is an undo registration on the
document's manager. U3 tests that decision; a `blocked` U3 follows rule 4.

## Driving the real app

- The spike app is the real `ContentView` in a signed build: `PRODUCT_NAME`
  `SchematicalSpike59`, `PRODUCT_BUNDLE_IDENTIFIER` `jgz.schematical.spike59`,
  so its process name, container (`~/Library/Containers/jgz.schematical.spike59`),
  recent-documents list and window restoration cannot collide with the user's
  installed app (which shares `jgz.schematical`).
- Ad-hoc signed (`CODE_SIGN_IDENTITY=-`) with the project's entitlements. A
  probe on 2026-11-24 showed an ad-hoc signed `app-sandbox` app runs sandboxed
  (its container appeared; `~/.zshrc` was unreadable). Nothing is registered
  with the developer account.
- Open a file the way Finder does with `open -a <spike.app> <file>` (both go
  through the same open-documents Apple event). The default-handler
  registration of `.schematical` is never changed.
- Drive menus and keys with `osascript` and System Events against process
  `SchematicalSpike59`. Run `lsappinfo front` before each keystroke batch and
  stop if the spike app is not frontmost: other sessions also drive the screen.
- Take pictures with `screencapture -l <window id>`; window ids come from
  `osascript`/`CGWindowListCopyWindowInfo` for the spike process.
- When a probe has moved the real mouse, move the pointer to (0, 0) before
  finishing (AGENTS.md § Testing in the app).
- Multi-line shell goes in a script file in the scratchpad directory and is
  run from there; worktree shells refuse loops and heredocs.

## Baselines (recorded in `findings.md` § Baselines before any probe)

- B1: median and p90 of `scripts/harness/edit-lag.swift` and
  `scripts/harness/drag-timing.swift` on `examples/6502.schematical`, base
  commit, Release harness build.
- B2: the relative `memoryFile.path` the unmodified app writes for Save As to a
  second folder (the value F2 must reproduce).
- B3: `SchematicFile(document:fileURL:)` main-thread time on
  `examples/6502.schematical`, median of 20.

## Probes

`info` probes feed the migration plan but do not decide viability.

### F — the file

| ID | Probe | Procedure | Pass |
|---|---|---|---|
| F1 | File URL while reading | Open `fixtures/eeprom-rel/design.schematical` (an EEPROM whose load file `rom.bin` sits beside it, path stored relative) in the sandboxed spike app | The chip's load file resolves to `rom.bin` through `MemoryFileAccess.file(path:bookmark:schematic:)` with the real URL |
| F2 | File URL while writing | Save As the same design into a second folder; then Duplicate; then let autosave write it | The stored `memoryFile.path` equals the B2 value for that destination, each time |
| F3 | Round trip | Open each of the 9 `examples/**/*.schematical` through the candidate's read path, write through its write path, decode both | `SchematicFile.decode(written) == SchematicFile.decode(original)` for 9 of 9 |
| F4 | Snapshot cost | Time the candidate's snapshot step (the main-thread part: `SchematicFile(document:fileURL:)`) on `examples/6502.schematical`, 20 runs; with autosave on, type 30 characters in a net label | Median ≤ 16.7 ms (one 60 Hz frame); no dropped typing |
| F5 | Catalog ready on cold open | Quit the app; `open -a <spike.app> examples/6502.schematical` | The window shows all parts of the file (chip count equals the file's `parts` length); no placeholder parts |
| F6 | Sandbox and other writers | Save a design with an EEPROM load file, relaunch, open it; then change the file from the shell and press Save | Load file bytes still read via the bookmark; the system's "modified by another application" prompt appears on Save |
| F7 `info` | Part files | Double-click routing for `.schpart` with the Info.plist document types added | `.schpart` does not open a schematic window |

### U — dirty state and undo

| ID | Probe | Procedure | Pass |
|---|---|---|---|
| U1 | Edit marks the document edited | Move one part | Close button shows the edited dot; the title menu offers Save |
| U2 | Undo back to clean | Edit, undo; edit, save, edit, undo; redo | After each step the edited flag equals "differs from the saved file" (today: `SchematicFileController.hasUnsavedChanges`, a snapshot compare) |
| U3 | One `NSUndoManager` per document | Each `UndoStep` (`UndoHistory.swift`: a per-item before/after diff) registers one undo on the document's manager; its handler applies the diff and registers the redo; `setActionName(step.name)`; one undo group per user action (a drag is one step, `UndoStep.coalescing`); do a Schematic edit, a PCB edit, a Schematic edit, then undo three times and redo three times | The system Edit › Undo and Redo (no `UndoCommands` replacement) read "Undo Move" etc. and restore each step's state in strict reverse order; drag and typing coalesce as `UndoHistory` does today; U1 and U2 hold |
| U4 | Text field undo | Edit a net-label name, press ⌘Z while the field is editing, then again with the field closed | First ⌘Z undoes typing; second undoes the document step |
| U5 | Simulation lock | Start the simulation, press ⌘Z | Nothing is undone and no edit is registered (`isPlaying` guards, `SchematicModel.swift:2023`) |

### A — the app around the document

| ID | Probe | Procedure | Pass |
|---|---|---|---|
| A1 | File menu | List the File menu | New, Open…, Open Recent, Close, Save, Duplicate or Save As…, Rename…, Move To…, Revert To, Export ▸ (Image, PDF, Gerber, KiCad, EasyEDA Pro), Print…, New Part…, Open Part…, Add Part to Library…; every key equivalent unique; each item acts on the frontmost document |
| A2 | Commands reach the focused document | With two documents open, run Export ▸ Image from each | Each acts on its own document |
| A3 | Exports from the Export menu | Export KiCad (three files beside the chosen URL) and EasyEDA Pro from the menu inside the sandbox | Files written; the save panel is the system's, with no Format accessory |
| A4 | Print | ⌘P | The system print flow runs and produces `PrintExport`'s pages (`Document/PrintExport.swift`) |
| A5 | Child windows | From one document open Memory, Processor, Registers, Terminal, Bill of Materials, Design Rule Check and Board Preview; with a second document open, repeat | Each child shows its own document's data; each closes with its document; none is restored at launch |
| A6 | Lifetime | Run the simulation, close the window; Revert To Last Saved with the simulation running | Simulation stopped; the document object freed (weak reference nil within one run loop turn); Revert reloads the canvas |
| A7 | Title bar | Keep `.windowToolbarStyle(.unifiedCompact(showsTitle: false))` | Window title menu, proxy icon and edited state work; toolbar unchanged (before/after screenshot) |
| A8 | Tabs | ⌘N, ⌘O, open the same file twice | New and opened documents become tabs per the system tab setting with no `tabbingMode` or `didBecomeKey` code; an untouched Untitled window is replaced by an opened file; the second open of an open file focuses it |
| A9 | Opening paths | Cold `open -a`; warm `open -a`; two files in one `open`; Open Recent after relaunch; drop on the Dock icon | All four open; recents persist |
| A10 | Restoration | Two documents open, one edited and unsaved; quit; relaunch | Both return, with the edited content; the page (Schematic or PCB) is restored; child windows are not |
| A11 | Close and quit prompts | ⌘W on an edited document; ⌘Q with two edited documents; Cancel on the second; again with the simulation running | System Save/Don't Save/Cancel sheets; Cancel aborts the quit; no `WindowCloseGuard` and no `confirmQuit` |
| A12 | Versions | Edit and save twice; Revert To ▸ Browse All Versions; Lock; Duplicate | Each works on a sandboxed file |
| A13 | Automation | Implement `list_windows`, `new_window`, `open`, `save`, `focus_window` and `resolveWindow` (`Automation/AutomationSession.swift`, `AutomationTools.swift:236`) against the candidate | Each returns the same result as on base, driven through public API |
| A14 | Performance | Re-run B1 on the spike tree | Medians within 10% of B1 |
| A15 `info` | Other scenes | Settings, User Library, Verify Datasheet, Part Editor and Help windows | They open as on base |
| A16 `info` | Test seams | For each of the 9 constructor sites in 8 test files, write down the replacement seam | A table in `findings.md` |
