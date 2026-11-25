# plan-61-C: What the user sees — title, tabs, launch, export, print, part files

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-61-B (prerequisites are stated in plan-61-A § Prerequisites)

61-B made the schematic a system document. This sub-plan carries the owner's
answers about what that looks like: the title in the title bar, a tab setting
that defaults to tabs, an Untitled document at a plain launch, exports under
File › Export with the KiCad project written into a chosen folder, printing
through the document, `.schpart` files opening in a Part Editor, and the page
restored with the window. The single outcome: each of those behaviors exists
and has a test or a recorded check.

References:

- `planning/plan-59-data/findings.md` § Owner answers and rows A3, A4, A7,
  A8, A9, A10, F7.
- `planning/plan-59-data/evidence/plan61-probes-H.txt` and
  `plan61-title-shown-H.png`: the four behaviors probed on the spike's H build
  before this plan was written (`scripts/patch-H-plan61-probes.py`).
- `schematical/schematical/Document/SchematicExportCommands.swift`,
  `Document/SaveFormatPicker.swift`, `Document/PrintExport.swift`,
  `Preferences/PreferencesView.swift`, `Editor/PartEditorTarget.swift`.
- `docs/export.md`.

## Prerequisites

Stated in plan-61-A. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-61-B is complete | `rg -c '^- \[ \]' planning/plan-61-B-nsdocument-and-hosted-window.md` → no output | NOT MET until B lands |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run before you continue and before you stop; report all prerequisites if
> you stop.

## 1. Goal

- The document window shows its title: the file name, the Edited mark, the
  title menu and the proxy icon.
- Settings has **Open documents in**: System Setting, Tabs, Windows; default
  Tabs. New and opened documents follow it.
- A plain launch with nothing restored opens an Untitled document; opening a
  file replaces an untouched Untitled one.
- File › Export has Image…, PDF…, Gerber PCB…, KiCad Project…, EasyEDA Pro
  Project…; the save panel has no Format accessory; KiCad asks for a folder.
- File › Print… and the system's print action both print `PrintExport`'s pages.
- A `.schpart` file sent to the app opens in a Part Editor.
- A restored window shows the page (Schematic or PCB) it showed at quit.

### Non-goals (explicit constraints)

- The export formats' contents do not change (`KiCadExport`, `EasyEDAProExport`,
  `GerberExport`, `ImageExport`, `PrintExport` are called as today).
- Restoration itself is the system's and follows the system's Close windows
  when quitting setting; this plan adds only the page.
- The Part Editor does not become a document type with autosave; it only
  accepts a file from Finder.

## 2. Current State

After 61-B: the window is `SchematicNSDocument`'s, built in
`makeWindowControllers()` with `toolbarStyle = .unifiedCompact` and no title
(as `.windowToolbarStyle(.unifiedCompact(showsTitle: false))` did); tabs follow
the system setting; a plain launch shows no window; exports are today's:
Image…, PDF… and Gerber PCB… in `SchematicExportMenu`, and KiCad and EasyEDA
Pro only through the Save As panel's Format accessory (`SaveFormatPicker`),
which 61-B's deletion of `saveAs()` left unreachable. Print… is the app's own
command (`SchematicFileController.printPages`). `Info.plist` exports the part
type `jgz.schematical.part` and nothing claims it. The page is `@State` in
`ContentView` (`:36`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Export menu items today | 3 | `rg -c 'Button\("' schematical/schematical/Document/SchematicExportCommands.swift` → 3 |
| Uses of `SaveFormatPicker(` | 1 | `rg -c 'SaveFormatPicker\(' schematical/schematical` → 1 file, 1 line (in `saveAs()`, deleted by 61-B) |
| Settings pages | 5 | `rg -n 'case partList' schematical/schematical/Preferences/PreferencesView.swift` → `case partList, defaults, references, designRules, automation` |
| Docs that describe exports or the Save panel's Format menu | 1 | `rg -l 'Format' docs/export.md` → `docs/export.md` |

### Verified properties

| Claim | How verified |
|---|---|
| `titleVisibility = .visible` on the hosted document window shows the file name and `Edited` beside the toolbar, with the toolbar's items intact | `evidence/plan61-title-shown-H.png`; `titlebar` → `titleVisibility=visible` |
| `tabbingMode = .preferred` makes New and opened documents tabs of one window; `.disallowed` makes them separate windows; `.automatic` follows the system setting | `evidence/plan61-probes-H.txt`: `tabGroupWindows=2 tabbingMode=1`; `tabGroupWindows=0 tabbingMode=2`; plan-59 A8 for automatic |
| A plain launch delivers the open-application Apple event (`currentAppleEvent.eventID == kAEOpenApplication`) and a launch with a file does not; with no document after the next main-queue turn, `newDocument(nil)` gives the Untitled window | the same file: `didFinishLaunching: event=1868656752 plain=true documents=0` → `[0] SchematicNSDocument name=Untitled`; with a file `event=nil plain=false documents=1` |
| Closing a blank Untitled document from `makeWindowControllers()` of a document with a file replaces it | plan-59 A8 (H); the same file: after opening 6502 the list holds only `6502.schematical` |
| The sandbox refuses the second and third KiCad files beside a save panel's URL; a folder from `NSOpenPanel` takes all three | plan-59 A3: `You don’t have permission to save the file “6502.kicad_sch”`; then `6502.kicad_pcb 6502.kicad_pro 6502.kicad_sch` |
| `NSDocument.printOperation(withSettings:)` returning `PrintExport`'s operation makes `printDocument:` show the system print panel | plan-59 A4 (H): `'6502.schematical' sheet=NSPanel 'Print'` |
| A `.schpart` open event reaches a SwiftUI scene that still handles external events, as `onOpenURL`, and that scene opens a window | `plan61-probes-H.txt`: `Part Editor scene onOpenURL test.schpart`, window `'Part Editor'` |
| `encodeRestorableState(with:)` / `restoreState(with:)` on the document carry the page | plan-59 A10 (H): `H restored page pcb`, `page selected=1` |

## 3. Design Overview

Six small, independent pieces; each is a phase. None depends on another
except that all need 61-B's document.

- **Title.** `window.titleVisibility = .visible` in `makeWindowControllers()`.
- **Tab setting.** `PreferenceKey.documentTabs` with a `DocumentTabs` enum
  (`system`, `tabs`, `windows`; default `tabs`), a picker on the General
  Defaults page, applied as `tabbingMode` when a document window is made and,
  on change, to the open document windows.
- **Launch.** `AppDelegate.applicationDidFinishLaunching` reads the launch
  Apple event; a plain launch with no document after one main-queue turn calls
  `newDocument(nil)`. `makeWindowControllers()` of a document with a file
  closes blank Untitled documents (no file, not edited, no parts, no autosave
  file).
- **Export.** `SchematicExportMenu` gains KiCad Project… (folder chooser,
  `canChooseDirectories`, prompt Export) and EasyEDA Pro Project… (save
  panel). `SaveFormatPicker` (the class) is deleted; `SaveFormat` stays for
  its content types.
- **Print.** `SchematicNSDocument.printOperation(withSettings:)` builds the
  PDF with the saved `PrintOptions`. File › Print… keeps the pages sheet and
  then runs that operation.
- **Part files.** `CFBundleDocumentTypes` gets the part type (role Editor,
  no document class). The Part Editor scene drops
  `.handlesExternalEvents(matching: [])`; its root view takes the URL from
  `.onOpenURL` and shows the editor on the file source
  (`PartEditorTarget.Source`, `Editor/PartEditorTarget.swift:19`).
- **Page.** `SchematicEditorModel.page` is what `ContentView` shows; the
  document encodes it in its restorable state and, restoring, sets it before
  the window is built.

Where the risk concentrates: nothing here is unprobed; the widest change is
export, which moves a user path (Save As ▸ Format) to File › Export.

Byte-identity is not a gate.

Rejected alternatives: a zip for the KiCad project (the owner chose the
folder); always tabbing with no setting, or following only the system
setting (the owner asked for the three-way setting, default Tabs: "more of a
VSCode than a Pages style app"); the Open panel at launch (the owner chose
Untitled).

## Compatibility / Format Impact

No file format changes. User-visible: the title appears in the toolbar row;
KiCad and EasyEDA Pro exports are under File › Export; the Save panel has no
Format menu; a new Settings item; Finder offers Schematical for `.schpart`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot tasks keep their line with the
> evidence. Fill `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — Title and tab setting

- [ ] `Document/SchematicNSDocument.swift`: `titleVisibility = .visible`; `tabbingMode` from `DocumentTabs.current`.
- [ ] `Preferences/PreferencesView.swift`: `PreferenceKey.documentTabs`, `enum DocumentTabs`, the picker **Open documents in** on the General Defaults page; on change, set `tabbingMode` on every open document window.
- [ ] `schematicalTests/SchematicNSDocumentTests.swift`: with the key unset a new document's window has `tabbingMode == .preferred`; set to `windows`, `.disallowed`; set to `system`, `.automatic`; `titleVisibility == .visible`.

Acceptance: the test passes.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/SchematicNSDocumentTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 2 — Launch and the blank Untitled

- [ ] `schematicalApp.swift` `AppDelegate`: the plain-launch rule of § 3. `SchematicNSDocument.makeWindowControllers()`: close blank Untitled documents when this one has a file.
- [ ] Test: two documents, the first blank and untitled, the second read from a file and given its window → `NSDocumentController.shared.documents` holds only the second; the same with one part placed in the first → both stay.

Acceptance: the test passes, and the built app started with `open -a` and no file shows one window named Untitled.
  Check: the test as in phase 1 (est. 5 min); `open -a <Debug app>; osascript -e 'tell application "System Events" to tell process "schematical" to get name of every window'` → `Untitled` (est. 1 min; with the owner's installed app closed).
Commit: —

### Phase 3 — Export

- [ ] `Document/SchematicExportCommands.swift`: KiCad Project… and EasyEDA Pro Project…; the exporter gains `exportKiCadProject()` (folder chooser, security scope around the write) and `exportEasyEDAProProject()` (from `spike-H.patch`).
- [ ] Delete `SaveFormatPicker` (the class) from `Document/SaveFormatPicker.swift`.
- [ ] `KiCadExportTests.swift`, `EasyEDAProExportTests.swift`: the tests that went through the Format picker call the new functions' write halves (`writeKiCad(to:)`, `writeEasyEDAPro(to:)` are unchanged).
- [ ] `docs/export.md`: KiCad and EasyEDA Pro are under File › Export; KiCad writes three files into the chosen folder.

Acceptance: the two export suites pass and the doc names the new path.
  Check: `xcodebuild test … -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests 2>&1 | tail -3` → `** TEST SUCCEEDED **`; `rg -c 'File › Export' docs/export.md` → at least `1` (est. 5 min).
Commit: —

### Phase 4 — Print and part files

- [ ] `SchematicNSDocument.printOperation(withSettings:)`; `printPages()` runs the pages sheet and then `runModalPrintOperation` with it.
- [ ] `Info.plist`: the part document type. `schematicalApp.swift`: the Part Editor scene handles external events; its root view opens the file from `.onOpenURL`. `Editor/PartEditorModel.swift`: `open()` splits into the panel and `open(_ url: URL)`.
- [ ] Tests: `printOperation(withSettings:)` on a document with one part returns an operation whose `jobTitle` is the document's name; `PartEditorModel.open(_:)` on a part file written to a temporary folder loads its name.

Acceptance: both tests pass.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicNSDocumentTests -only-testing:schematicalTests/PartFileSaveOpenTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 5 — The restored page

- [ ] `ContentView`'s page is initialized from `model.page` and writes back to it in its existing `onChange(of: page)`; the document's `encodeRestorableState(with:)` and `restoreState(with:)` carry it and `invalidateRestorableState()` is called on change.
- [ ] Test: a document with `model.page = .pcb` encoded with an `NSKeyedArchiver` and restored into a second document gives `.pcb`.

Acceptance: the test passes.
  Check: as phase 1 (est. 5 min).
Commit: —

## Validation Plan

- Tests: `SchematicNSDocumentTests` additions; `KiCadExportTests`, `EasyEDAProExportTests`, `PartFileSaveOpenTests`.
- Coverage check: each line of § 1 has a phase and each phase a test or a recorded check.
- Runtime proof: phase 2's `osascript` check; `evidence/plan61-probes-H.txt` is the reference for the rest.
- Doc sync: `docs/export.md`.
- Final gate (once): the four classes above in one `xcodebuild test` → `** TEST SUCCEEDED **` (est. 7 min).

## Decisions

- Show the document's title in the title bar — decided by the owner, 2026-11-24.
- A tab setting with System Setting, Tabs, Windows, default Tabs — decided by the owner, 2026-11-24.
- A plain launch shows an Untitled document, replaced by the first opened file — decided by the owner, 2026-11-24.
- KiCad export asks for a folder — decided by the owner, 2026-11-24.
- A `.schpart` file opens in a Part Editor — decided by the owner, 2026-11-24.
- The setting lives on the General Defaults page — decided by the author, 2026-11-24.

## Corrections

Filled in during execution.

## Summary

Six probed behaviors, one phase each; the widest user-visible move is KiCad
and EasyEDA Pro export from the Save panel to File › Export.
