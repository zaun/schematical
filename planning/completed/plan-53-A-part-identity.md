# plan-53-A: Part identity: a new part gets a new uuid

Last updated: 2026-11-21
Overall Effort: large (3h–1d) — plan-53 is the persistent User Library (A–F)
Effort: medium (1h–2h)
Depends on: nothing

plan-53 gives the app a persistent library of custom parts. Its first piece is
about identity. Today a custom part is known by its `uuid`, and two different
parts with one uuid are refused (`uuidInUse`; a schematic that embeds one fails
to open with `customPartConflict`). With a library that outlives the session
that refusal would hit often: the library holds one version of a part and a
schematic embeds another. After this sub-plan **no two different parts ever
share a uuid**, because whoever would collide is given a new one:

- the catalog registers a part whose uuid is taken by *different* content under
  a fresh uuid instead of refusing it;
- a schematic that embeds such a part opens, with its placed parts remapped to
  the fresh uuid;
- the Part Editor keeps a part's uuid when it is saved (Save updates the part),
  and gives a new uuid to a part it makes new: Save As… of a part that was
  opened or saved before, Copy to Editor…, and an Add to Library that would
  collide;
- each fresh uuid carries `originId`, the uuid the part started from, in the
  part file. Nothing reads `originId` yet; it is kept for later update logic.

References:

- `docs/parts-format.md` § Single part files (the file's keys, the uuid rules)
  and § Custom footprints in a part file (the `CUSTOM-<8 of uuid>` ids)
- `docs/file-format.md` § Custom parts (the open rules this changes)
- `docs/backend.md` — Notes, "Single part files"
- `schematical/schematical/Backend/PartCatalog.swift` — `PartFileObject`,
  `PartRegistrationError`, `PartCatalogStore.register(fileText:source:)`
- `schematical/schematical/Document/SchematicFile.swift` — `apply`
- `schematical/schematical/Editor/PartEditorModel.swift`
- `backend/src/library/library.c` — `PART_FILE_KEYS`, `read_part_file`
- plan-32-A, plan-32-B, plan-32-C (completed): what this changes

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
They are stated once here; plans B, C and D point here.

| Must be true | Command | Status |
|---|---|---|
| plan-32 (custom parts, Part Editor) is complete and archived | `ls planning/completed/plan-32-*.md \| wc -l` → `10` | MET (run 2026-11-21: `10`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `PartCatalogStore.register(fileText:source:)` never throws for a uuid clash:
  a file whose uuid is registered with different content is registered under a
  fresh uuid, with its inline `CUSTOM-<8 of uuid>` footprint and datasheet ids
  renamed to match, and its `originId` set.
- Opening a version 5 schematic whose embedded part collides this way succeeds,
  and its placed parts resolve to the fresh-uuid part.
- The Part Editor's Save keeps the uuid. Save As… of a part that was opened or
  saved before writes a new file under a new uuid with `originId` set; Copy to
  Editor… makes a draft under a new uuid with `originId` set; Add to Library
  freshens the draft when its uuid collides.
- The part file format accepts an optional `originId` (an uppercase UUID) and
  ignores it.

### Non-goals (explicit constraints)

- **No persistence here.** Nothing is written to disk by this sub-plan; the
  library is plan-53-B.
- **Same uuid with the same content is still the same part**: `register`
  returns the registered part, as now (`PartRegistrationTests.testSameFileTwiceReturnsTheRegisteredPart`).
- **The `.schematical` file format does not change.** Version 5, `customParts`
  and `partUUID` keep their meaning; only the open rules for a clash change.
- **Bundled parts have no uuid and no `originId`.**
- **`originId` is never read** by the app or the backend beyond checking its
  shape. No update, merge or lineage feature.
- **Save never changes a uuid.** Updating a part in place is the Part Editor's
  Save (a file) and plan-53-D/E (a library part).
- **The name-clash rule is unchanged**: a taken name gets the lowest free
  `.2`, `.3`, … suffix.

## 2. Current State

- `PartCatalogStore.register` (`PartCatalog.swift:1725`): if the file's uuid is
  in `partsByUUID` it compares the file, renamed to the registered name, with
  the registered text; equal returns the registered part, different throws
  `PartRegistrationError.uuidInUse` (`PartCatalog.swift:1161`). A taken name gets
  a suffix; then `load_part_file`.
- `SchematicFile.apply` (`SchematicFile.swift:964–972`) registers every
  `customParts` entry first and turns `uuidInUse` into
  `SchematicFileError.customPartConflict` (`SchematicFile.swift:672`). A placed
  part resolves with `part.partUUID.map { catalog.part(uuid: $0) }`
  (`SchematicFile.swift:980`).
- The backend accepts exactly the top-level keys in `PART_FILE_KEYS`
  (`library.c:1380`): `format version uuid kind footprints datasheets devices`.
- The Part Editor writes the part's one inline footprint with the id
  `CUSTOM-` + the first 8 characters of the uuid (`CustomFootprintDraft.id(forPart:)`,
  `CustomFootprintDraft.swift:135`) and its inline datasheet with the same id
  (`PartDraft.inlineDatasheetID`, `PartDraft.swift:626`).
- `PartEditorModel.copy(named:)` (`PartEditorModel.swift:409`) replaces only
  `draft.uuid` with a new one. The `.file` target (`PartEditorModel.swift:382`)
  and `load(text:from:)` (`PartEditorModel.swift:502`) keep the file's uuid.
  `addToLibrary()` (`PartEditorModel.swift:516`) registers the draft as it is.
- `PartDraft` keeps unknown top-level keys of a file in `fileExtras`
  (`PartDraft.swift:290`) and writes them back (`PartDraft.swift:455`).
- The ids a part file may use "must be new to the library" (`docs/parts-format.md`
  § Single part files, the `footprints` and `datasheets` rows), so two parts
  that keep one `CUSTOM-<8>` footprint id cannot both be registered.

### Measured populations

| What | Count | Command |
|---|---|---|
| App call sites of `register(fileText:` | 3 | `grep -rn 'register(fileText' schematical/schematical --include='*.swift' \| wc -l` → 3 (`SchematicFileController.swift:591`, `SchematicFile.swift:966`, `PartEditorModel.swift:520`) |
| Test call sites of `register(fileText:` | 11 | `grep -rn 'register(fileText' schematical/schematicalTests \| wc -l` → 11 |
| Places that name `uuidInUse` / `customPartConflict` | 9 lines | `grep -rn 'uuidInUse\|customPartConflict' schematical --include='*.swift' \| wc -l` → 9 (2 in tests: `PartRegistrationTests.swift:90`, `SchematicFileV5Tests.swift:216`) |
| Test cases this changes | 3 | `PartRegistrationTests.testSameUUIDOtherContentIsRefused`, `SchematicFileV5Tests.testConflictingUUIDLeavesTheDocument`, `PartFileSaveOpenTests.testCopyHasANewUUIDAndRefusesABadSave` (read: each asserts the old identity rule) |

### Verified properties

- **A copied custom part keeps its inline footprint id.** Read
  `PartEditorModel.copy(named:)` (`PartEditorModel.swift:409–420`): it parses the
  registered text and sets `draft.uuid` only. A copy of a part with an inline
  footprint therefore carries `CUSTOM-<old 8>` under a new uuid, and Add to
  Library would collide with the registered original's footprint id. This
  sub-plan's renaming fixes that too.
- **`fileExtras` carries an unknown top-level key through a round trip.** Read
  `PartDraft.init(file:)` (`PartDraft.swift:290`) and `encoded()`
  (`PartDraft.swift:455`): `root = fileExtras` first, then the typed keys.
  `originId` therefore round-trips with no typed field.
- **Tests build their own catalogs.** Read `PartRegistrationTests.swift:9–14`,
  `SchematicFileV5Tests.swift:10–14`, `TestSupport.catalog`
  (`TestSupport.swift:10`): each is `PartCatalogStore()` then `load()`.

## 3. Design Overview

Four pieces, in order:

1. **The format** accepts `originId` (backend).
2. **`PartFileObject.freshenIdentity()`** is the one place a part file gets a
   new identity. Everything else calls it.
3. **The catalog** calls it where a clash used to throw.
4. **The two callers** (schematic open, Part Editor) use the result.

`freshenIdentity()`:

1. `old` = the file's `uuid`; `new` = `UUID().uuidString`.
2. `originId` = the file's existing `originId` if it has one, else `old`. The
   *root* origin is kept, so a lineage of versions all point at the first.
3. `uuid` = `new`.
4. Every string **value** (not key), anywhere in the object, equal to
   `"CUSTOM-" + old.prefix(8)` becomes `"CUSTOM-" + new.prefix(8)`. These are
   exactly the inline footprint id (`footprints[].id`), the part's `footprint`,
   `connectorFootprint`, each `footprintChoices[].footprint`, the inline
   datasheet id and the part's `datasheet`. A hand-written inline id that does
   not follow the `CUSTOM-<8>` form is left alone; if it clashes the loader
   refuses it as any id clash.

**Correctness risk** concentrates in the schematic open: the remap must happen
before placed parts resolve, for every placed part, and a saved-again file must
embed the fresh part under the fresh uuid. It is tested end to end in
Phase 2.

**Gate class.** Behavior changes on purpose, so byte-identity is not the gate:
tests are. Files that do not collide and parts that are not edited are
unchanged; `SchematicFileV5Tests.testPlainSchematicIsVersion4AsBefore` and
`PartFileSaveOpenTests.testSaveOpenSaveIsStable` (unchanged) pin that.

Rejected alternatives:

- **Refuse as today and let the user choose** — the owner decided every version
  gets a new uuid so nothing collides.
- **Keep the old uuid and rename the older part** — the part a document already
  placed keeps its uuid in its file; renaming it would rewrite other files.
- **Rewrite the uuid inside the document's `partUUID`s on disk at open** — the
  document is only changed in memory; its next save writes the fresh uuid.

## 4. Detailed Design

### 4.1 Backend: `originId`

`backend/src/library/library.c`: add `"originId"` to `PART_FILE_KEYS`. In
`read_part_file`, right after the `uuid` checks, when the root has an
`originId` key read it with `kind_string` and require `valid_uuid`, with the
error `"originId" must be an uppercase UUID (XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), not "…"`
(the wording of the `uuid` error). The value is not stored on `Part`.

### 4.2 `PartFileObject.freshenIdentity()`

`PartCatalog.swift`, in `PartFileObject`: a mutating method as in § 3. It walks
`object` recursively (dictionaries and arrays of `Any`) replacing equal string
values.

### 4.3 `register`

When `partsByUUID[uuid]` exists and the content differs, replace today's
`throw` with: restore the incoming file's own name (the compare step set it to
the registered name), call `freshenIdentity()`, and continue into the
name-clash and `load_part_file` steps. The returned `CatalogPart.uuid` is the
fresh one. Add `func uuidConflicts(_ fileText: String) -> Bool` (true when the
uuid is registered with different content, by the same compare) for the editor.
Delete `PartRegistrationError.uuidInUse`.

### 4.4 Schematic open

`SchematicFile.apply`: while registering `customParts`, keep
`remapped: [String: String]`; for an entry whose registered part's uuid differs
from the entry's own `uuid`, record `old → new`. Resolve each placed part with
`catalog.part(uuid: remapped[$0] ?? $0)`. Delete the `uuidInUse` catch and
`SchematicFileError.customPartConflict`.

### 4.5 Part Editor

- `PartDraft.freshened() throws -> PartDraft`: `PartFileObject(text: fileText())`,
  `freshenIdentity()`, then `PartDraft(fileText:)` of the result.
- `copy(named:)`: for a registered custom part, parse its registered text as a
  `PartFileObject`, `freshenIdentity()`, build the draft; for a bundled part
  keep today's behaviour (new uuid, no `originId`: a bundled part has no
  identity to record).
- The `.file` target, `load(text:from:)` and `write(to:)` are unchanged: a part
  opened from a file keeps its uuid, and Save writes it back under that uuid.
- `PartEditorModel.saveAsNewVersion(to url: URL) throws` (new): when the draft
  has a `fileURL` (it was opened or saved before) it first does
  `draft = draft.freshened()`; then `write(to: url)`. `saveAs()` calls it after
  its panel returns a URL, so a cancelled panel changes nothing. A draft that
  was never saved keeps its uuid on its first Save As…. The old file is left as
  it was.
- `addToLibrary()`: when `catalog.uuidConflicts(draft text)`, set
  `draft = draft.freshened()` before registering.

## Compatibility / Format Impact

- **Part file:** one new optional top-level key, `originId`. A file without it
  is unchanged; a build without this key refuses a file that has it (unknown
  top-level key).
- **`.schematical`:** unchanged. A file that used to fail with
  `customPartConflict` now opens.
- **Error text:** the two error cases are removed; no other message changes.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` for partial; mark a moot task
> `- [x] ~~text~~ — moot: <evidence>`; fill each `Commit:` the moment it lands;
> add any task you discover you needed. An unticked box means NOT DONE.

### Phase 1 — Backend accepts `originId`

Safe alone: a new optional key nothing else uses yet.

- [x] `backend/src/library/library.c`: `"originId"` in `PART_FILE_KEYS`; read and validate it in `read_part_file` (§ 4.1).
- [x] `backend/tests/part_file_test.c`: add cases after the last lettered one: a file with a valid `originId` loads and `part_library_find_uuid` still returns the part; a file with `"originId":"not-a-uuid"` is refused with a message containing `originId`; a lowercase UUID is refused. (case (k), three checks)
- [x] `docs/parts-format.md` § Single part files: an `"originId"` row in the key table (optional; an uppercase UUID in the `uuid` layout; the uuid the part's first version had; ignored by the app and the loader beyond its shape) and the sentence "No other top-level key is allowed" still true. `docs/backend.md` Notes: one sentence.

Acceptance: the three new cases pass and the existing `part_file_test` cases still pass. **MET**
  Check: `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build --target part_file_test && ctest --test-dir /tmp/logic-build -R part_file_test --output-on-failure` → `100% tests passed, 0 tests failed out of 1` (run in the worktree against a build dir of its own, `/tmp/logic-build-p53`, so other sessions' `/tmp/logic-build` is not shared; est. 2–4 min).
Commit: 2d4e7d4d

### Phase 2 — `freshenIdentity()`, the catalog and the schematic open

One commit: removing `uuidInUse` breaks `SchematicFile.apply`, so the catalog and the open path change together.

- [x] `PartCatalog.swift`: `PartFileObject.freshenIdentity()` (§ 4.2).
- [x] `PartCatalog.swift`: `register` freshens on a uuid clash and `uuidConflicts(_:)` (§ 4.3); delete `PartRegistrationError.uuidInUse` and its description.
- [x] `SchematicFile.swift` `apply`: the remap and the resolve change (§ 4.4); delete `SchematicFileError.customPartConflict` (case at `:672`, description at `:716`).
- [x] `schematicalTests/PartRegistrationTests.swift`: replace `testSameUUIDOtherContentIsRefused` with a test that registers the same uuid with other content and gets a part whose `uuid` differs, whose name is the file's own (suffixed only if taken), whose `registeredText` holds `originId` equal to the first uuid, and, with an inline-footprint file (`inlineFootprint: true`), whose footprint id is `CUSTOM-` + the new uuid's first 8 characters; the first part is unchanged. Add: a lineage (v1, v2 freshened from v1, v3 freshened from v2) keeps `originId` = v1's uuid; `uuidConflicts` is false for the same content and true for different.
- [x] `schematicalTests/SchematicFileV5Tests.swift`: replace `testConflictingUUIDLeavesTheDocument` with: register uuid U with content X, open a v5 file whose embedded U has content Y; the open succeeds, the placed part is the fresh-uuid part (its `uuid` ≠ U, its `registeredText` has `originId` U and the file's pins), a second placed part with the same `partUUID` resolves to the same part, and saving then reopening in a fresh catalog gives one embedded part under the fresh uuid.
- [x] `docs/file-format.md` § Custom parts: the On-open bullet's "one whose uuid is loaded with other content fails the open" becomes "is loaded under a new uuid, and the placed parts that name the old uuid use it; the part's file records the old uuid as `originId`"; the failure list in § Validation on open (`docs/file-format.md:620` area) drops the uuid conflict.

Acceptance: the catalog registers a clashing part under a fresh uuid with renamed inline ids, and a schematic opens through a clash with its placed parts on the fresh part. **MET**
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartRegistrationTests -only-testing:schematicalTests/SchematicFileV5Tests` → `** TEST SUCCEEDED **` (est. 5–10 min; the app builds first).
Commit: ad8eaf29

### Phase 3 — The Part Editor: Save As…, copies and Add to Library

- [x] `PartDraft.swift`: `freshened()` (§ 4.5).
- [x] `PartEditorModel.swift`: `copy(named:)`, `saveAsNewVersion(to:)` (called by `saveAs()` after its panel) and `addToLibrary()` per § 4.5.
- [x] `schematicalTests/PartFileSaveOpenTests.swift`: `testSaveOpenSaveIsStable` is unchanged and still passes (Save keeps the uuid); `testCopyHasANewUUIDAndRefusesABadSave` also asserts a copy of a registered custom part with an inline footprint renames the footprint id and sets `originId`; add: `saveAsNewVersion` of an opened file writes a new file whose uuid differs, whose `originId` is the opened file's uuid, with the inline footprint id renamed, and leaves the opened file's bytes unchanged; `saveAsNewVersion` of a never-saved draft keeps its uuid and sets no `originId`; Add to Library of a draft whose uuid is registered with other content adopts a fresh uuid, registers, and a second Add of the unchanged draft returns the same part.
- [x] `docs/parts-format.md` § Single part files: the sentences on the uuid ("the app goes by the uuid") gain: a clash gets a new uuid and the old one becomes `originId`; the Part Editor keeps the uuid on Save and makes a new one for Save As… of a saved part and for a copy.

Acceptance: Save keeps a part's uuid; Save As…, a copy and a colliding Add to Library make a new one. **MET**
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartFileSaveOpenTests -only-testing:schematicalTests/PartDraftRoundTripTests` → `** TEST SUCCEEDED **` (est. 4–8 min).
Commit: ad8eaf29

## Validation Plan

- Tests: as in each phase, including the negative cases (bad `originId`, same
  content returns the same part).
- Coverage check: the changed code (`register`, `freshenIdentity`, `apply`,
  the editor paths) is exercised by the named test files; the Phase 2 and 3 checks
  run those files.
- Runtime proof: none beyond tests in this sub-plan; the end-to-end run is
  plan-53-F.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`, `docs/file-format.md`
  (done in Phases 1–3).
- Final gate (run ONCE, after Phase 3): `ctest --test-dir /tmp/logic-build -R 'part_file_test|library_check'` and
  `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartRegistrationTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PartFileSaveOpenTests -only-testing:schematicalTests/PartDraftRoundTripTests -only-testing:schematicalTests/PartEditorWindowTests`
  → all pass (est. 10–15 min).

## Decisions

- Every version of a part gets a new uuid so nothing collides — decided by the owner, 2026-11-21. Settles the clash rule (§ 3, § 4.3, § 4.4).
- Editing a part: Save updates the existing part (same uuid), Save As… makes a new one (new uuid) — decided by the owner, 2026-11-21. Settles § 4.5 and replaces the earlier assumption that every opened file got a new uuid.
- The original id is kept in the file as `originId` for later update logic, and unused for now — decided by the owner, 2026-11-21. Settles § 4.1 and the "never read" non-goal.
- The root origin is kept as `originId` across a chain of versions — author's call, no user-visible consequence. A lineage then shares one origin.

## Corrections

- **The name is not part of the same-content test.** `register` and `uuidConflicts` compare the file under the *registered* name (so a suffixed copy is the same part), so a draft whose only change is its name is not a new version: Add to Library returns the registered part. The plan's Phase 3 test changed the name; it now changes the description (`PartFileSaveOpenTests.testAddToLibraryOfANewVersionGetsANewUUID`).
- **A `PartCatalogStore` freed inside a test aborts the host** (`pointer being freed was not allocated`, as `TestSupport.newDocument()` notes). The first Phase 2 run (`/tmp/p53-xc2.log`, `** TEST FAILED **`) had a local store in the new V5 test; it is now a kept static (`SchematicFileV5Tests.otherStore`).
- **Found later (plan-53-F's end-to-end test): the same clashing file registered a new copy on every open.** § 4.3's "register under a fresh uuid" gave the same text a new random uuid each time, so opening one schematic twice added `NAME.3`, `NAME.4`, …. `register` now remembers the clashing text and its fresh uuid (`freshenedTexts`); the same text again is that part (`PartRegistrationTests.testTheSameClashingFileTwiceIsOnePart`). See plan-53-F's Corrections.
- **Final gate (run once after Phase 3), 2026-11-21:** `ctest --test-dir /tmp/logic-build-p53 -R 'part_file_test|library_check'` → `100% tests passed, 0 tests failed out of 2`; `xcodebuild test … PartRegistrationTests SchematicFileV5Tests PartFileSaveOpenTests PartDraftRoundTripTests PartEditorWindowTests` → `Executed 40 tests, with 0 failures`, `** TEST SUCCEEDED **` (`/tmp/p53-xc5.log`).
- **Check commands ran in the worktree with their own build directories** (`/tmp/logic-build-p53`, `-derivedDataPath /tmp/p53-dd`) so other sessions' builds are not shared. Phase 2 and 3 were run together: `xcodebuild test … -only-testing:…PartRegistrationTests -only-testing:…SchematicFileV5Tests -only-testing:…PartFileSaveOpenTests -only-testing:…PartDraftRoundTripTests` → `Executed 31 tests, with 0 failures`, `** TEST SUCCEEDED **` (`/tmp/p53-xc4.log`).

## Summary

The risk is the schematic open: a remap that misses one placed part leaves it
"unknown". Everything else is small and local. Untouched: the `.schematical`
format, bundled parts, the name-suffix rule, and any reading of `originId`.
