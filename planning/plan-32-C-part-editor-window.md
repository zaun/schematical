# plan-32-C: Part editor window, form, and single-part file save/open (gate first)

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-32-B

The editor is a window the user opens from File › New Part… (⇧⌘N), or from a
library part's context menu as a copy. It holds the part's information in a form,
validates it with the backend's own loader, saves and opens single-part files
(`.schpart`), and adds the part to the library (plan-32-A). This sub-plan builds the
window, the generic form (identity, pinouts, footprint choice, supply range) and
the **gate** kind's keys, which proves the design end to end. Every other kind is
added to the same form by plan-32-F through plan-32-J.

Behavioral outcome: a user creates a gate part in the editor, saves it to a
`.schpart` file, and File › Load Part… loads that file back into the picker; the
part's pins, drawer path and footprint are what the form showed.

References:

- `docs/parts-format.md` — § Parts (the part keys), § Pinouts and pins, § Devices.
- `backend/src/models/gate.c` — the gate kind's keys: `DEVICE_KEYS` (line 28),
  `PART_KEYS` (line 29), `VOLTAGE_KEYS` (line 77), the timing keys `KEYS` (line 82)
  and `SPLIT_KEYS` (line 97), the gate-operation keys `KEYS` (line 134).
- `schematical/schematical/Backend/PartCatalog.swift` — `PartLibraryHandle` (line 8),
  `PartCatalogStore.register` (plan-32-A), `CatalogPart` (line 208).
- `schematical/schematical/Document/SchematicFileController.swift` — `WindowCloseGuard`
  (the unsaved-changes guard), `SchematicFileCommands` (the File menu).
- `schematical/schematicalApp.swift` — the `WindowGroup(…, for:)` scenes (the
  datasheet and memory windows are the precedent for a per-target window).
- `schematical/schematical/Parts/footprints.json` — the footprint choice list.
- plan-32-A § 4.1 — the single-part file shape.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-B complete: version 5 documents and `PartCatalogStore.register` exist | `xcodebuild test … -only-testing:schematicalTests/SchematicFileV5Tests` → `passed` | MET (run 2026-11-11 in the P-32 worktree: `SchematicFileV5Tests` passed, 10 cases) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- File › New Part… (⇧⌘N) opens a kind picker; choosing a kind opens a new editor
  window for a blank part of that kind. In this sub-plan only **gate** is enabled;
  the other 25 kinds are listed but disabled until their sub-plan lands.
- The editor's form fills every part key the gate kind accepts (§ 4.4) and the
  generic keys (§ 4.3). Save Part… writes a valid `.schpart` file; Open Part…
  reads one into the editor; Add to Library registers the part (plan-32-A).
- A validation message from the backend shows in the form, with its place in the file.
- A library gate part's drawer context menu gets **Copy to Editor…**, which opens the
  part as an editor copy with a new UUID.

### Non-goals (explicit constraints)

- **Bundled parts are never edited in place.** The editor always works on a copy with
  a new UUID; bundled files are read-only.
- **No kind other than gate in this sub-plan.** Other kinds: plan-32-F…J.
- **No footprint editing beyond choosing a bundled footprint.** Custom footprints and
  the footprint preview are plan-32-D; the block preview is plan-32-E.
- **Keys the form does not show are kept.** The editor's draft keeps every key of the
  file it does not have a control for (§ 4.2, `extras`), so an open-and-save never
  drops data. This is what lets F–J add kinds one at a time.
- **No change to the shipped gate files or to version 4 documents.**

## 2. Current State

- The window scene pattern: `schematicalApp.swift` declares one `WindowGroup` per
  target type with `.restorationBehavior(.disabled)` (memory, processor, registers,
  terminal, bill of materials, datasheet windows). The editor follows that pattern.
- The unsaved-changes guard: `WindowCloseGuard(shouldClose:)` in
  `Document/SchematicFileController.swift` answers `windowShouldClose`; the
  schematic window uses it to ask before closing a dirty document.
- The File menu: `SchematicFileCommands` (`SchematicFileController.swift`) uses
  `FocusedValues` (`@Entry var schematicFile`) to route Open/Save to the focused
  window. The editor adds its own focused value.
- Gate parts in the shipped files: `rg -o '"manufacturer"' schematical/schematical/Parts/gate.json | wc -l`
  → 3438 parts; `rg -o '"function"' schematical/schematical/Parts/gate.json | wc -l`
  → 126 devices (measured 2026-11-11).
- Gate keys (read from `gate.c`): device keys `gates`, `output`, `timing`, `units`
  (`gate.c:28`); part keys `family`, `vcc`, `inputMax` (`gate.c:29`); voltage keys
  `vcc`, `inputMax` (`gate.c:77`); timing entry keys `rise`, `fall`, `note` (`gate.c:82`)
  and the split-timing keys `otherLow`, `otherHigh`, `note`, `vcc`, `inputMax` (`gate.c:97`);
  gate operation keys `op`, `inputs`, `output` (`gate.c:134`).
- The shipped part keys (`library.c:211`): `name`, `package`, `pinout`, `flags`,
  `footprint`, `connectorFootprint`, `footprintChoices`, `manufacturer`, `datasheet`.
- The footprint list: `footprints.json`, 1212 entries (`rg -c '"id"' schematical/schematical/Parts/footprints.json` → 1212).

### Verified properties

- **The gate model reads its keys from the device object.** Read `gate.c:28–29`
  and the `DEVICE_KEYS` use in the loader; the device's `gates` array is what the
  gate form edits.
- **The kind table in `docs/parts-format.md` is incomplete.** Read the table: 21 rows
  (`rg -o '^\| `[a-z0-9]+\.json`' docs/parts-format.md | wc -l` → 21). The shipped files are
  26 (`Parts/*.json` minus the two non-kind files). The missing five are `mux.json`,
  `buffer.json`, `arith.json`, `header.json`, `pio.json`; Phase 5 adds them.

## 3. Design Overview

Layers, each testable alone:

1. **`PartDraft`** (Swift value type): the editor's model of one single-part file. It
   has typed fields for the generic keys, a `GateDraft` for the gate's keys, and an
   `extras: [String: JSONValue]` for every other key (in the device and the part).
   `PartDraft.init(file: JSONValue)` and `PartDraft.encoded() -> JSONValue` are inverses
   for every key the file has (§ 4.2).
2. **`PartValidator`**: builds a single-part file from a draft, loads it with
   `load_part_file` into a scratch `PartLibraryHandle` that holds the bundled
   `footprints.json` and `datasheets.json` (so footprint and datasheet references
   resolve), and returns the `CatalogPart` or the backend's message. Runs off the
   main actor, debounced.
3. **`PartEditorWindowView`**: the SwiftUI form; sections Identity, Pinouts,
   Footprint, Supply, and the kind section (the gate form in this sub-plan).
4. **Window and menus**: the scene, the File › New Part… kind picker, the editor's
   focused commands (Save Part…, Open Part…, Add to Library), and **Copy to Editor…**
   in the drawer's context menu.

Where correctness risk concentrates: the **round trip** of `PartDraft`. If a key is
lost or reordered in a way the backend reads differently, the user's saved part is
not the part they edited. Schedule: the draft's round trip is proven over every
shipped gate part before any UI is built on it (Phase 2 before Phase 3).

### Rejected alternatives

- **A typed model for every key of every kind, now.** Rejected: it would make this
  sub-plan the size of all of F–J. The `extras` keep the file lossless while the kinds
  land one at a time.
- **Editing the JSON text directly (a code editor).** Rejected: the owner asked for a
  form, and a text editor can produce files the backend rejects with no field to point at.
- **Validating only on Save.** Rejected: the form shows the backend's message at the
  field, so validation runs on each change (debounced).

## 4. Detailed Design

### 4.1 Targets and windows

- `PartEditorTarget: Hashable, Codable { let id: UUID; let source: Source }` with
  `enum Source: Hashable, Codable { case new(kind: String), copy(name: String), file(URL) }`.
  The UUID keeps each window distinct.
- `schematicalApp.swift`: `WindowGroup("Part Editor", id: "part-editor", for: PartEditorTarget.self)`,
  `.defaultSize(width: 760, height: 720)`, `.restorationBehavior(.disabled)`.
- File › New Part… (⇧⌘N) in `SchematicFileCommands`, after Open…: opens a sheet listing
  the 26 kinds (names from `docs/parts-format.md`'s table); enabled kinds are the ones
  listed in `PartDraft.enabledKinds` (only `gate` in this sub-plan); choosing one calls
  `openWindow(id: "part-editor", value: PartEditorTarget(id: UUID(), source: .new(kind:)))`.
- Drawer context menu (`Drawer/ChipDrawerView.swift`): **Copy to Editor…** on a library
  part of an enabled kind; opens `.copy(name:)`.
- Each editor window has `WindowCloseGuard` (unsaved changes ask before closing).

### 4.2 `PartDraft` and the round trip

- Fields: `uuid` (new for a new or copied part), `name`, `kind`, `manufacturer`,
  `package`, `description`, `function`, `drawer: [String]` (0–2), `flags: Set<String>`
  (the five names of `docs/parts-format.md` § Flags), `datasheet: String?`,
  `pinouts: [PinoutDraft]` (name, `style` numbered/ball, pins), `pinout: String`
  (the part's pinout), `footprint: String?` (a bundled id, § plan-32-D for custom),
  `vcc: [Double]?` (2 numbers), `inputMax: Double?`, `gate: GateDraft?`, `extras`.
- `extras` holds every key of the device object and the part object the draft has no
  field for, in the position it had (keys are written sorted anyway, so position is not
  kept; the key and value are).
- `PartDraft.init(file:)` and `encoded()` must satisfy: for every shipped gate part,
  `encoded(init(file: F)) == F` as JSON values, where `F` is the single-part file built by
  the copy path (§ 4.5) and `uuid` is the only key that differs (it is set by the copy).

### 4.3 Generic form

- **Identity**: name (required; unique against the library — a clash shows the suffix
  rule's result, plan-32-A), manufacturer (text, default `Generic`), package (text), description,
  function (text), drawer (two text fields), flags (five toggles), datasheet (a picker of
  the ids in `datasheets.json`, plus None; the backend rule: a datasheet needs a
  manufacturer, `Generic` has none).
- **Pinouts**: a list of named pinouts; each has a style (numbered or ball, fixed at
  creation) and a pin table (designator, name, role, group, side). Roles: the seven of
  `docs/parts-format.md` § Pinouts and pins. Side: auto, in, out.
- **Footprint**: a picker of the bundled footprints whose `pins` equal the pinout's
  pin count (`footprints.json` entry field `pins`), with its description.
- **Supply**: `vcc` min and max (volts) and `inputMax`, shown for kinds that take them
  (gate in this sub-plan).

### 4.4 Gate form

- Device: `gates` (an array of operations: `op` (one of the eight names in `gate.c`'s
  `OP_NAMES`), `inputs` (pin names), `output` (pin name)), `output` (`open-drain` or
  push-pull; `gate.c:28`), `timing` (the rise and fall pairs with optional split
  `otherLow`/`otherHigh`, `gate.c:82` and `gate.c:97`), `units` (`gate.c:28`).
- Part: `family` (the logic-family list in `docs/parts-format.md` § Values), `vcc`,
  `inputMax` (`gate.c:29`).
- Pin names in `gates` are validated by the backend (the rule that every name used must
  exist in every pinout).

### 4.5 The copy path (bundled part → draft)

- `BundledPartFile.copy(name:) -> JSONValue`: finds the device and the part in the bundled
  `Parts/<kind>.json` (parsed with `JSONSerialization`), returns the single-part file object
  (`format`, `version`, a new `uuid`, `kind`, the device with exactly that one part).
  Footprints are not inlined (they are bundled).

### 4.6 Save, open, add

- Save Part… (⌘S in the editor): `PartDraft.encoded()` → sorted-keys JSON →
  `NSSavePanel` with `.schpart`. Refused while the validator reports an error.
- Open Part… (⌘O in the editor): reads a `.schpart`, `PartDraft.init(file:)`, replaces the
  draft (asks first if the draft is dirty).
- Add to Library: the validator's text goes to `PartCatalogStore.register` (plan-32-A);
  the drawer then lists the part.

### 4.7 Validation

- `PartValidator.validate(draft) async -> Result<CatalogPart, String>`: encodes the draft,
  loads the bundled `footprints.json` and `datasheets.json` into a scratch
  `PartLibraryHandle` once (cached for the window's life), then `load_part_file`. The
  error string is the backend's message (`<source>: devices[0] (<id>): parts[0] (<name>): …`).
- Runs on each change, debounced; the form shows the message under the Identity section.

## Phases

### Phase 1 — Window, targets and New Part… (gate only)

One line: an empty editor window opens from File › New Part…, with the kind picker.

- [x] `schematical/schematical/Editor/PartEditorTarget.swift`: `PartEditorTarget` and its `Source` (§ 4.1).
- [x] `schematical/schematicalApp.swift`: the `part-editor` `WindowGroup` (§ 4.1).
- [x] `schematical/schematical/Editor/PartEditorWindow.swift`: `PartEditorWindowView` with an empty Identity section and the title "New Gate Part" (the kind's display name from `PartDraft.kindNames`).
- [x] `schematical/schematical/Editor/PartKindPicker.swift`: the sheet (a small `Window`, Corrections) listing the 26 kinds, enabled = `PartDraft.enabledKinds` (`["gate"]`); disabled rows show the kind's name and "Coming soon".
- [x] `SchematicFileController.swift`: `SchematicFileCommands` gets Button("New Part…") with ⇧⌘N, opening the picker.
- [x] Tests: `schematical/schematicalTests/PartEditorWindowTests.swift`: (a) `PartDraft.enabledKinds == ["gate"]`; (b) the 26 kind names in `PartDraft.kindNames` equal the sorted `Parts/*.json` file names minus `footprints.json` and `datasheets.json` (measured: 26; the kind table in `docs/parts-format.md` lists only 21 of them, `rg -o '^\| `[a-z0-9]+\.json`' docs/parts-format.md | wc -l` → 21, so Phase 5 adds the missing five rows).

Acceptance: the two `PartEditorWindowTests` cases pass, and the New Part… item opens the picker in a launched app.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartEditorWindowTests` → `passed` (est. 4 min); the picker's appearance is the run skill's check (est. 5 min).
  Result (2026-11-11): `PartEditorWindowTests` passed (a, b — 26 kind files read from `Parts/` — c, and a new-draft case). Real app: ⇧⌘N opened the "New Part" window listing all 26 kinds, Gate enabled, the rest "Coming soon"; choosing Gate opened "New Gate Part".
Commit: (this commit, with Phases 2–5)

### Phase 2 — `PartDraft` round trip over every shipped gate part

One line: the draft reproduces every shipped gate part's file exactly, and the backend accepts it.

- [x] `schematical/schematical/Editor/PartDraft.swift`: `PartDraft` with the § 4.2 fields, `init(file:)` and `encoded()`, with `extras` for every key without a field.
- [x] `schematical/schematical/Editor/GateDraft.swift`: `GateDraft` for `gates`, `output`, `timing`, `units` (§ 4.4).
- [x] `schematical/schematical/Editor/BundledPartFile.swift`: the copy path (§ 4.5).
- [x] Tests: `schematical/schematicalTests/PartDraftRoundTripTests.swift`: for every gate part in `Parts/gate.json` (3438 parts, measured above), `copy(name:)` → `PartDraft(file:)` → `encoded()` equals the copy as JSON values with `uuid` excluded; and `PartValidator`'s `load_part_file` accepts each encoded file.

Acceptance: over all 3438 gate parts, the encoded draft equals the copy and the backend accepts it; zero mismatches and zero rejections.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartDraftRoundTripTests` → `passed` with the test's own count line `checked 3438 parts, 0 mismatched, 0 rejected` (est. 6 min; the population is the whole gate kind, which is the smallest set that exercises every gate key).
  Result (2026-11-11): passed, `checked 3438 parts, 0 mismatched, 0 with extras, 0 rejected` (4.2 s); the line also counts parts with keys left in `extras` (plan-32-F's measure), and nested keys a field has no control for count as extras.
Commit: (this commit)

### Phase 3 — Validator and the window's form (Identity, Pinouts, Footprint, Supply, gate section)

One line: the window shows the form for a gate part and reports backend errors at the form.

- [x] `schematical/schematical/Editor/PartValidator.swift`: scratch library (cached per window), `validate(draft) async -> Result<CatalogPart, String>`, debounced by the window (§ 4.7).
- [x] `PartEditorWindow.swift`: Identity, Pinouts (add, remove, reorder rows; numbered or ball style), Footprint (picker of bundled footprints with equal pin count, from `footprints.json`), Supply (vcc min/max, inputMax), and the gate section (`GateDraft`: operations list, output, timing, units, family).
- [x] The validation message is shown under the form; Add to Library and Save are disabled while it is an error.
- [x] Tests: `PartValidatorTests.swift`: (a) a valid draft validates to a `CatalogPart` with the expected `pins.count`; (b) a draft whose gate names an unknown pin returns an error that contains the pin name; (c) a draft with a footprint whose pad count differs from the pinout returns the backend's footprint message.

Acceptance: the three validator cases pass; a launched app shows an error message for a deliberately wrong pin name in the form.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartValidatorTests` → `passed` (est. 4 min); the form error is the run skill's check (est. 10 min).
  Result (2026-11-11): `PartValidatorTests` passed (a–c, plus five checks in a row leaving the scratch library clean). Real app: Copy to Editor… on SN74HC00N, renamed to ZZEDIT-NAND, pin `1A` of pinout `lccc20` renamed `1AX` → the form showed `ZZEDIT-NAND.schpart: devices[0] (74_00): pinouts.lccc20: gates[0]: no pin named "1A"`, and Save Part…/Add to Library went disabled; renaming it back showed "The part loads: 14 pins".
Commit: (this commit)

### Phase 4 — Save, Open, Add to Library and Copy to Editor…

One line: a part leaves the editor as a `.schpart` file or as a library part, and comes back the same.

- [x] `PartEditorWindow.swift`: Save Part… (⌘S, focused value), Open Part… (⌘O, focused value), Add to Library (calls `PartCatalogStore.register`).
- [x] `schematical/schematical/Editor/PartEditorCommands.swift`: `FocusedValues` entry for the focused editor; the commands.
- [x] `schematical/schematical/Drawer/ChipDrawerView.swift`: the context-menu item Copy to Editor… on a part of an enabled kind (opens `.copy(name:)`).
- [x] Tests: `PartFileSaveOpenTests.swift`: (a) `PartDraft` saved, then opened, then saved gives identical bytes; (b) a file the backend rejects is not replaced in the draft (the open reports the message); (c) Add to Library of a validated draft makes `PartCatalogStore.shared.part(named:)` return it.

Acceptance: the three save/open cases pass, and in a launched app a part saved from the editor is loaded back with File › Load Part… (plan-32-A) and appears in the drawer.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartFileSaveOpenTests` → `passed` (est. 5 min); the app round trip is the run skill's check (est. 10 min).
  Result (2026-11-11): `PartFileSaveOpenTests` passed (a–c, plus a copy-has-new-uuid / bad-save-refused case). Real app: ⌘S in the editor saved `ZZEDIT-NAND.schpart` (9645 bytes); File › Load Part… on it raised the drawer count 26,636 → 26,637 and the search `ZZEDIT` shows Logic › Gates › NAND › `ZZEDIT-NAND  DIP · HC · 2–6 V`. The same session is plan-32-B's runtime proof (see there).
Commit: (this commit)

### Phase 5 — Help and keys

One line: the editor's keys are on the Schematic Help page.

- [x] `schematical/schematical/Help/KeyboardHelp.swift`: the Schematic Help page lists ⇧⌘N (New Part…), and the Part Editor's ⌘S, ⌘O and the Copy to Editor… item.
- [x] `docs/parts-format.md`: add the five missing rows to the kind table (`mux.json` → `mux`, `buffer.json` → `buffer`, `arith.json` → `arith`, `header.json` → `header`, `pio.json` → `pio`); a line in § Parts that a single part file is authored in the Part Editor (link to plan-32-A § 4.1 for the shape).

Acceptance: the Help page text contains "New Part" and "⇧⌘N" (test in `PartEditorWindowTests`, case c).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartEditorWindowTests` → `passed` (est. 4 min).
  Result (2026-11-11): passed, case (c) included; `rg -o '^\| `[a-z0-9]+\.json`' docs/parts-format.md | wc -l` → 26.
Commit: (this commit)

## Validation Plan

- Tests: `PartEditorWindowTests` (a–c), `PartDraftRoundTripTests` (all 3438 gate parts), `PartValidatorTests` (a–c), `PartFileSaveOpenTests` (a–c).
- Coverage check: `rg -n 'PartDraft|PartValidator' schematical/schematical --glob '*.swift' -l` lists the new files, and each is in the app target (`rg -n 'Editor/' schematical/schematical.xcodeproj/project.pbxproj | wc -l` shows them).
- Runtime proof: New Part… → gate → fill a 2-input NAND (pins, one `gates` entry, footprint `DIP-14`-equivalent bundled footprint with 14 pins) → Save → File › Load Part… on the saved file → the drawer's search finds it (run skill, est. 20 min).
- Doc sync: `docs/parts-format.md` (Phase 5), `KeyboardHelp.swift` (Phase 5).
- Final gate (once, after Phase 5): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers).

## Decisions

- **Gate first; the other 25 kinds in F–J** — the owner chose all 26 kinds in v1 (plan-32-A § Decisions). Ordering: the gate kind is the first because its keys are the largest single set that tests the whole form (§ 2). § Phases.
- **Bundled parts are copied, never edited** — the author's choice, forced by the read-only bundle (`docs/file-format.md`'s rule that bundled parts come from the library). § 4.5.
- **Unknown keys are kept (`extras`)** — the author's choice: it makes each later kind an additive change and keeps any file lossless (§ 3).
- **Validation runs through the backend loader** — the author's choice: one set of rules (the backend's), no duplicate Swift validation (§ 3).
- **Clash names follow plan-32-A's suffix rule** — decided by the owner (plan-32-A § Decisions).

## Corrections

- **The kind forms are schema-driven** (2026-11-11). § 3 describes a typed `GateDraft`
  struct; the implementation gives each kind a `KindSchema` (`Editor/KindSchema.swift`):
  a list of `Field`s, each a key with its control type (text, number with unit, choice,
  pin, pin list, volt range, nested object, list of objects, family-keyed map, …).
  `GateDraft.schema` (in `Editor/GateDraft.swift`) lists gate.c's keys; the form renders
  every field with its typed control (`Editor/FieldEditors.swift`), and the draft keeps
  each kind value as JSON (`PartDraft.kindDevice` / `kindPart`). This is not the rejected
  "generic key/value grid": every key has a typed, labelled control with the backend's
  rule as help. It is what makes F–J's 25 kinds (some with 20 key arrays) feasible with
  the same guarantees: `PartDraft.formKeys(kind)` is the schema's key set (nested keys
  included) for the drift tests, and `extraKeys` lists both top-level extras and nested
  keys a field has no control for, so "empty extras" is checked at every depth.
- **The validator needs to take a load back** (2026-11-11). § 4.7's scratch library
  would keep every draft it loads, so the second check of a draft fails as a duplicate
  name. New backend call `part_library_unload_last_part_file` (it removes the latest
  single part file's part, name, uuid and inline footprints and frees its arena, while
  nothing else was loaded since); `part_file_test` case (i) checks it, `docs/backend.md`
  describes it. The scratch library is one shared `PartValidator.shared` (loaded once,
  locked), not one per window.
- **The kind picker is a small window, not a sheet** (2026-11-11). File › New Part… must
  work with no window open; a sheet needs one. `Window("New Part", id: "new-part")`.
- **⌘O and ⌘S are one item each** (2026-11-11). § 4.6 gives the editor ⌘S and ⌘O, which
  the schematic's Open… and Save already use. `SchematicFileCommands` shows Open Part… /
  Save Part… in place of Open… / Save while a Part Editor window is focused
  (`FocusedValues.partEditor`), and adds New Part… (⇧⌘N), Save Part As… and Add to
  Library (`PartEditorCommands`).
- **Datasheet field** (2026-11-11). § 4.3 asks for a picker of the `datasheets.json` ids;
  there are thousands, so it is a text field with a menu of the matching ids (first 40).
- **Test hosts keep the editor models** (2026-11-11). Freeing a `PartEditorModel` inside
  a test aborts the host (`pointer being freed was not allocated`, the isolated-deinit
  problem `TestSupport.newDocument()` documents); the tests keep their models in a static
  array.
- **Phase 4 (c) uses `PartCatalogStore.shared`, as written**; PartRegistrationTests and
  SchematicFileV5Tests use stores of their own.

## Summary

The risk is the `PartDraft` round trip; it is proven over the whole gate kind before any
UI depends on it. Untouched: bundled files, version 4 documents, and every kind but gate.
