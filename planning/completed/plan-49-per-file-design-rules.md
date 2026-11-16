# Per-File Design Rules and the Two-Step Design Rule Check Plan

Last updated: 2026-11-16
Effort: medium (1h–2h)

Design rules (min trace width, clearance, mask expansion, … 13 lengths) move
from app preferences into the `.schematical` file. Today every reader calls
`DesignRules.current()`, so the same file exports different Gerbers and gets
different check results on another Mac. After this plan a board's rules are
part of its file: every reader uses the document's rules, a rule change is an
undoable edit, and Window › Design Rule Check is a two-step wizard — step 1
shows and edits the file's rules, step 2 runs the check and shows the results.

The single outcome: a file saved by this build carries its rules, and
opening it on a Mac with different Settings ▸ DRC Defaults gives the same
check results, Gerbers, Default net class and routing.

References:

- `docs/file-format.md` (§ `board`, § Net classes, § Design rules)
- `docs/routing.md` (lines 51 and 88 name Settings ▸ DRC Defaults)
- `schematical/schematical/Preferences/DesignRulePreferences.swift`,
  `DesignRuleManufacturers.swift`
- `schematical/schematical/DesignRuleCheck/DesignRuleCheckWindow.swift`
- `schematical/schematical/Document/SchematicFile.swift`

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan number 49 is free | `ls planning planning/completed \| grep -c plan-49` → 0 | MET |

Everything below is written against that.

`$DD` in the checks is a scratch derived-data folder (any empty directory).

## 1. Goal

- `BoardSettings` holds the board's `DesignRules` and the manufacturer preset
  they came from; both are saved in the file's `board.designRules` object and
  read back.
- A new document copies Settings ▸ DRC Defaults when it is created. A file
  with no `designRules` gets the app defaults when it is opened, and they are
  written on its next save.
- No code outside Settings and those two copy points reads the app defaults:
  the check, Gerber export, the router, the Default net class, the PCB canvas,
  the PCB and Nets sidebars, the board preview and the automation tools all
  use the document's rules.
- Changing the rules is one undo step ("Change Design Rules") and marks the
  file changed.
- Window › Design Rule Check (⇧⌘D) opens on step 1 (the file's rules,
  editable; **Run Check** is the default button; **Reset to App Defaults**),
  and step 2 is today's live results table with a **Rules** back button.

### Non-goals (explicit constraints)

- The file version stays 4 (5 with custom parts). `designRules` is an
  optional key; files without it open.
- Settings ▸ DRC Defaults keeps its look and its stored keys
  (`drcManufacturer`, `drcCustomRules`). It becomes the template for new
  files only.
- The check itself (`DesignRuleCheck.swift`), the manufacturers' tables and
  the rule list do not change.
- Existing traces, vias and net classes are not resized when rules change.

## 2. Current State

- `DesignRules` (13 `Double`s, mm) and `DesignRules.current(defaults:)`:
  `Preferences/DesignRulePreferences.swift:5,75`. `current()` returns the
  chosen manufacturer's fixed table, or the stored Custom rules.
- `DRCManufacturer` (Custom + 12 makers, `fixedRules`):
  `Preferences/DesignRuleManufacturers.swift:11`.
- The check window, `DesignRuleCheckTable`
  (`DesignRuleCheck/DesignRuleCheckWindow.swift:71`), reads the two
  `@AppStorage` keys, reruns on any change of board or rules after 150 ms,
  and has a toolbar `SettingsLink` "Design Rules…".
- `BoardSettings` (`Canvas/BoardSettings.swift:271`) is the board's saved
  settings; `SchematicDocument.board` (`Canvas/SchematicModel.swift:1573`)
  is changed only inside `recordEdit`, which snapshots document state, so
  anything in `BoardSettings` is undoable. Precedent to mirror: net classes
  (`addNetClass`, `SchematicModel.swift:3448`; file entry
  `SchematicFile.Board.NetClassEntry`; reader `SchematicFile.netClasses`).
- File writing builds `Board(…)` at `Document/SchematicFile.swift:717`;
  reading is `boardSettings()` at `:1169`, which starts from
  `BoardSettings()`.

### Measured populations

| What | Count | Command |
|---|---|---|
| App call sites of `.current()` to switch | 19, in 13 files | `grep -rn "\.current()" --include='*.swift' schematical/schematical \| grep -v "DesignRuleCheck.swift:7" \| wc -l` → 19; `grep -rln …` → 13 |
| Test call sites of `current(` | 12, in 4 files | `grep -rn "current(" --include='*.swift' schematical/schematicalTests \| wc -l` → 12 |

### Verified properties

- **An older build opens a file with `designRules`.** `SchematicFile.Board`
  uses synthesized `Codable` (the file's only two `init(from:)` are at lines
  371 and 493, neither `Board`'s), and synthesized decoding ignores unknown
  keys. So the version does not need to rise; the older build opens the file
  with its own app rules and drops the key on save.
- **Undo covers `board`.** `recordEdit` compares and stores `state`, and the
  net-class edits change `board.netClasses` inside it with no other
  bookkeeping (`SchematicModel.swift:3448–3530`).
- **`DesignRules` and `DRCManufacturer` are `nonisolated`, `Hashable`,
  `Sendable`**, so they can be stored in the `nonisolated` `BoardSettings`.

## 3. Design Overview

Three layers, landed in this order:

1. **Storage.** `BoardSettings` gains `rules: DesignRules = .customDefault`
   and `rulesManufacturer: DRCManufacturer = .custom`. The struct default is
   a constant so `BoardSettings()` stays pure; the two copy points set the
   real values: `SchematicDocument.init` (new document) and
   `SchematicFile.boardSettings()` (file with no `designRules`).
2. **Readers.** `DesignRules.current()` is renamed `appDefaults()` and
   `DRCManufacturer.appDefault()` is added; every other caller takes
   `document.board.rules`. Functions whose `rules:` parameter defaulted to
   `.current()` lose the default where they have the document
   (they read it), or take `rules: DesignRules? = nil` meaning the
   document's.
3. **Wizard.** The check window gets a `step` state. The rules form is
   shared with Settings.

Correctness risk is in layer 2: a missed reader keeps the old per-Mac
behaviour silently. The gate is a grep that finds no `appDefaults()` caller
outside the allowed three files, plus a test that a document with different
rules from the app's gets a different Default net class and DRC result.

This plan changes behaviour (rules now follow the file), so tests, not
byte-identity, are the gate.

Rejected:

- *Lazy stamping* (rules written only once edited): leaves unedited files
  dependent on the Mac. The owner chose copy-up-front.
- *Storing only the manufacturer name*: a later table update would change
  old files' behaviour. Values are stored and are authoritative.
- *A separate top-level `designRules` key*: rules are board settings and
  `BoardSettings` already gives undo and file plumbing.
- *Raising the file version*: not needed (see Verified properties), and it
  would lock every saved file out of older builds.

## 4. Storage and file format

`board.designRules`, always written:

```json
"designRules": { "manufacturer": "jlcpcb", "minTraceWidth": 0.127, "minClearance": 0.127,
                 "minDrill": 0.3, "minViaDrill": 0.3, "minViaDiameter": 0.6,
                 "minAnnularRing": 0.13, "minHoleToHole": 0.5, "minCopperToEdge": 0.3,
                 "minSilkscreenWidth": 0.15, "minSilkscreenHeight": 1, "minSilkscreenToPad": 0.15,
                 "minSolderMaskDam": 0.1, "solderMaskExpansion": 0.05 }
```

- Every key is optional in the entry struct. A missing rule takes the app
  default for that rule; a missing or unknown `manufacturer` is `custom`
  (the values are authoritative, the name is a label).
- A rule that is not finite or is negative is a `badPCB` error
  ("design rule minClearance -1 must be 0 or more"), like other board values.
- A missing `designRules` object: rules and manufacturer are the app's
  defaults (`DesignRules.appDefaults()`, `DRCManufacturer.appDefault()`).

The label: `BoardSettings.rulesTitle` is the manufacturer's title, plus
" (modified)" when the manufacturer has fixed rules and `rules` differs from
them.

Document edit: `SchematicDocument.setDesignRules(_ rules: DesignRules,
manufacturer: DRCManufacturer)` inside `recordEdit(.pcb, "Change Design
Rules", coalescing: "designRules")`; values are clamped to 0 or more.

## 5. The wizard

`DesignRuleCheckTable` becomes `DesignRuleCheckWizard` with
`@State step: Step = .rules` (the window opens on step 1 every time).

- **Step 1 — Rules.** A grouped form: a Manufacturer picker (choosing a
  maker copies its table into the file; choosing Custom keeps the values),
  then the 13 fields, always editable (editing under a maker shows
  "JLCPCB (modified)" in the footer). Bottom bar: **Reset to App Defaults**
  (disabled when the file's rules and manufacturer equal the app's) and
  **Run Check** (`.keyboardShortcut(.defaultAction)`), which goes to step 2.
  Footer text: "These rules are saved with this board. New boards start
  from Settings ▸ DRC Defaults."
- **Step 2 — Results.** Today's table, empty state and copy command, still
  rechecking live as the board changes. Toolbar: **Rules** (back to step 1)
  in place of the `SettingsLink`. The subtitle and the empty state use
  `board.rulesTitle`.

The 13-field form is one shared view, `DesignRuleFields(rules:editable:set:)`,
in `DesignRulePreferences.swift`, used by Settings (editable only under
Custom, as now) and by step 1.

## Compatibility / Format Impact

- New optional `board.designRules` object; version unchanged. Older builds
  open such a file and ignore the key.
- A file saved before this plan opens with the app defaults of the Mac
  opening it and carries them from its next save.
- Automation's rule output reads the document's rules.

## Phases

### Phase 1 — Rules in the board settings and the file

- [x] `Preferences/DesignRulePreferences.swift`: rename `current(defaults:)`
      to `appDefaults(defaults:)` (and every call site, app and tests);
      `DesignRuleManufacturers.swift`: add
      `DRCManufacturer.appDefault(defaults:)`.
- [x] `Canvas/BoardSettings.swift`: add `rules`, `rulesManufacturer`,
      `rulesTitle`.
- [x] `Canvas/SchematicModel.swift`: `init` copies the app defaults into
      `board`; add `setDesignRules(_:manufacturer:)`.
- [x] `Document/SchematicFile.swift`: `Board.DesignRulesEntry`, written
      always, read in `boardSettings()` with the rules of § 4.
- [x] Tests: new `schematicalTests/FileDesignRulesTests.swift` — a document's
      rules survive save and open; a file with no `designRules` opens with
      the app defaults; a missing rule key takes the default; a negative
      rule is refused; an unknown manufacturer is Custom; `rulesTitle`
      says "(modified)"; `setDesignRules` is one undo step.

Acceptance: the new tests pass and the file tests still pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/FileDesignRulesTests -only-testing:schematicalTests/DesignRuleSettingsTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/NetClassTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: f9a8daf8

### Phase 2 — Every reader uses the document's rules

- [x] Switch the 19 call sites: `Document/GerberExport.swift` (2),
      `Canvas/SchematicModel.swift` (2), `Canvas/NetsSidebar.swift`,
      `Canvas/PCBSidebarView.swift`, `Canvas/PCBCanvasView.swift` (2),
      `BoardPreview/BoardRenderer.swift`, `Automation/AutomationRead.swift`
      (2), `Routing/AutorouteModel.swift` (2), `Routing/AutorouteSheet.swift`
      (2), `Routing/RoutingSession.swift`,
      `Routing/RoutingProblemBuilder.swift`, `Routing/AssistRouter.swift` (2).
- [x] Tests: in `FileDesignRulesTests`, a document whose rules differ from
      the app's gets its own Default net class (`addNetClass` width) and its
      own routing problem minimums; update the 5 test call sites of
      `NetRules.defaults(for: .current())` to the document's rules.

Acceptance: only Settings and the two copy points read the app defaults.
  Check: `grep -rln "appDefaults()\|appDefault()" --include='*.swift' schematical/schematical` → exactly `Preferences/DesignRulePreferences.swift`, `Preferences/DesignRuleManufacturers.swift`, `Canvas/SchematicModel.swift`, `Document/SchematicFile.swift`, `DesignRuleCheck/DesignRuleCheckWindow.swift`; and `xcodebuild … test -only-testing:schematicalTests/FileDesignRulesTests -only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/ViaTests -only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/RoutingProblemTests -only-testing:schematicalTests/GerberExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: f9a8daf8

### Phase 3 — The two-step wizard and the docs

- [x] `Preferences/DesignRulePreferences.swift`: extract `DesignRuleFields`;
      Settings uses it; the Settings footer says the rules are what new
      boards start from.
- [x] `DesignRuleCheck/DesignRuleCheckWindow.swift`: the wizard of § 5;
      header comment updated.
- [x] `DesignRuleCheck/DesignRuleCheck.swift:7` comment; `docs/file-format.md`
      (`designRules` in § board with the rules of § 4, the Default class
      paragraph, a "rules are in the file" bullet); `docs/routing.md` lines
      51 and 88.
- [x] Run the app: open a board, ⇧⌘D shows step 1; change min clearance,
      Run Check, results use it; ⌘Z restores it; save, reopen, the rule is
      kept. — done as a hosted-window test, not by hand (see Corrections):
      `xcodebuild … test -only-testing:schematicalTests/DesignRuleCheckWizardTests`
      → `** TEST SUCCEEDED **`; snapshots of both steps looked at. Save and
      reopen: `FileDesignRulesTests.testRulesSurviveSaveAndOpen`.
- [x] Tests: new `schematicalTests/DesignRuleCheckWizardTests.swift` (added
      task) — the wizard in a window: step 1 lists the 13 rules, typing
      0.5 into Min clearance changes the board's rule as one undo step, Run
      Check shows the results under it, undo changes the list, Rules goes
      back, Reset to App Defaults copies Settings.

Acceptance: the wizard works in the running app as the last task describes,
and the docs name the file as the home of the rules.
  Check: `grep -c "designRules" docs/file-format.md` → at least 2; the app
  walk above, observed (est. 10 min).
Commit: f9a8daf8

## Validation Plan

- Tests: `FileDesignRulesTests` (storage, file, undo, readers), plus the
  existing settings, net class, trace, via, routing and Gerber tests.
- Coverage check: `FileDesignRulesTests` sets rules that differ from every
  default, so a reader still on the app defaults fails it.
- Runtime proof: the Phase 3 app walk.
- Doc sync: `docs/file-format.md`, `docs/routing.md`.
- Final gate (once): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test CODE_SIGNING_ALLOWED=NO` for the suites named above plus `DesignRuleCheckTests`, `DesignRuleGoldenTests`, `PCBUndoTests`, `AssistRouterTests`, `RoutingVerifierTests`, `UndoHistoryTests` → `** TEST SUCCEEDED **` (est. 10 min).

## Decisions

- Rules are per file, edited in the Design Rule Check, with the app's
  Settings as the starting values — decided by the owner, 2026-11-16.
- Copy-up-front: a new file copies the app defaults at creation; an old file
  takes them when opened and keeps them from its next save — decided by the
  owner, 2026-11-16.
- The Design Rule Check is a two-step wizard: rules, then results — decided
  by the owner, 2026-11-16.
- Values are stored, with the manufacturer as a label shown "(modified)" —
  proposed to the owner 2026-11-16 and not objected to.

## Corrections

- **No worktree or branch.** The follow-plan skill forks a `worktree-P-49`
  branch; `AGENTS.md` forbids creating a branch the owner did not name, so
  the plan was worked on `main` in the main tree, committing only its own
  files.
- **`setDesignRules` does not coalesce** (§ 4 said `coalescing:
  "designRules"`). A coalescing key merges edits until `endCoalescing()`,
  which nothing here would call, so every rule change in a session would
  have become one undo step. Each change is its own step.
- **Phase 2's grep finds two files, not five.** `grep -rln
  "appDefaults()\|appDefault()"` → `Document/SchematicFile.swift` and
  `Canvas/SchematicModel.swift`: the two copy points. The definitions take
  `defaults:` so the pattern skips them, and the wizard reads the two
  `@AppStorage` keys for Reset to App Defaults (so the button follows
  Settings live) rather than calling `appDefaults()`.
- **Three byte-golden tests** (`NetClassTests`, `NetRulesTests`,
  `SchematicFileV5Tests`) compare a saved file with a golden from before
  this plan. They now strip the `designRules` object first
  (`TestSupport.withoutDesignRules`, which fails when it is absent). Not in
  the plan's task list.
- **Tests must use `TestSupport.newDocument()`**: freeing a
  `SchematicDocument` in a test aborts the host on macOS 15.8.
- **Phases 1–3's code landed as one commit**: the rename (phase 1), the
  readers (phase 2) and the shared form (phase 3) touch the same files.
- **The app walk was done as a test, not with the mouse.** The screen was
  locked for the whole session, and then neither real events nor the
  accessibility API reach another process's windows (`AXWindows` of the
  launched app returned the application itself). The real app was
  launched and showed its Untitled window and a Window ▸ Design Rule Check
  item (disabled on the Schematic page, as designed). The steps themselves
  are driven in the app's test host by `DesignRuleCheckWizardTests`, which
  hosts the wizard's real view in a window, types into the real field and
  presses the real buttons; PNG snapshots of step 1 and step 2 were
  inspected. Not exercised by hand: ⇧⌘D opening the window from the PCB
  page, and File ▸ Save / Open through the panels.
- Final gate, observed: 16 suites (the 15 named plus
  `DesignRuleCheckWizardTests`) → `Executed 202 tests, with 0 failures`,
  `** TEST SUCCEEDED **`.
- Phases 1 and 2 check, observed: the 15 suites of the final gate →
  `Executed 201 tests, with 0 failures`, `** TEST SUCCEEDED **`.

## Summary

The risk is a reader left on the app defaults; the grep gate and a test with
non-default rules close it. The check itself, the manufacturers' tables, the
file version and Settings' stored keys are untouched.
