# plan-32-B: Part editor: schematic documents carry their custom parts

Last updated: 2026-11-11
Effort: medium (1h–2h)
Depends on: plan-32-A

A `.schematical` document that places a custom part stores that part's full
definition in the document, so the document opens correctly on a machine where the
part was never loaded. The behavioral outcome: a document with a custom part,
saved, closed, relaunched and opened, shows the same part with the same pins and
footprint, without any File › Load Part… step.

References:

- `docs/file-format.md` — the `.schematical` format this sub-plan extends to version 5.
- `schematical/schematical/Document/SchematicFile.swift` — `SchematicFile`
  (`currentVersion = 4`, line 16), the part entry (`partNumber`, `footprint`,
  `pcb`), `SchematicFileError.unknownParts` (line 397), and the open path
  `apply(to:catalog:fileURL:)`, which resolves each part with
  `catalog.part(named:)` and refuses an unknown name.
- plan-32-A — `PartCatalogStore.register(fileText:source:)` and the clash rules.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-A is complete: the register path exists and its tests pass | `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartRegistrationTests` → `passed` | NOT MET at writing (plan-32-A not started); the sub-plan starts only after it passes |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- A document whose parts include a custom part is written as version 5, with a
  `customParts` array holding each custom part's single-part file object (as
  registered, with its suffixed name and UUID), and each such placed part carries
  its `partUUID`.
- Opening a version 5 document registers its custom parts (plan-32-A's rules)
  before resolving the placed parts. A placed part resolves by `partUUID` when it
  has one, otherwise by `partNumber`, as now.
- A document with no custom parts is written exactly as version 4 is today.

### Non-goals (explicit constraints)

- **Version 4 files open and save unchanged.** A document with no custom part is
  written with `version` 4 and no `customParts` key; its bytes do not change.
- **Bundled parts are not embedded.** A placed bundled part keeps `partNumber` only
  (the design rule in `docs/file-format.md`: "Parts are stored by part number only").
- **Older builds refuse version 5 files** through the existing `newerVersion` check
  (`SchematicFile.swift:588`). Nothing new is needed for that; a test pins it.
- **No change to `footprint`, `pcb`, `color`, `ohms`, `gender`, `switches` keys.**

## 2. Current State

- `SchematicFile.currentVersion = 4` (`SchematicFile.swift:16`). A file newer than
  that is refused: `guard version <= currentVersion` (`SchematicFile.swift:588`).
- Each placed part is `SchematicFile.Part` with `partNumber` (`SchematicFile.swift:66`).
  The open path resolves it with `catalog.part(named: name)`; a missing name is
  collected into `unknown` and thrown as `SchematicFileError.unknownParts`
  (`SchematicFile.swift:634–714`).
- Keys are written sorted (`encoded()`, `SchematicFile.swift`: `.sortedKeys`), so the
  bytes are a function of the content.
- Counts (measured 2026-11-11): `rg -n 'partNumber' schematical/schematical/Document/SchematicFile.swift | wc -l`
  → 5 lines (the stored key, the legacy name map and the open path); `rg -l 'partNumber' schematical/schematical --glob '*.swift' | wc -l`
  → 18 Swift files use the name, so the open and save changes stay in `SchematicFile.swift` and the catalog.

### Verified properties

- **The open path resolves each part by name only.** Read `SchematicFile.swift:apply`
  (the loop at line 634): the lookup is `catalog.part(named: name)` and nothing else.
- **`encoded()` sorts keys.** Read `SchematicFile.swift` (`encoder.outputFormatting`
  includes `.sortedKeys`).
- **The version gate is the only older-build protection.** Read `SchematicFile.swift:578–590`.

## 3. Design Overview

Two pieces:

1. **The file shape.** `customParts: [JSONValue]` (each element the single-part file
   object, plan-32-A § 4.1) at the top level, written only when non-empty; `version`
   is 5 exactly when `customParts` is written. Each `Part` entry gains
   `partUUID: String?`, written only for a custom part. `JSONValue` is a small
   recursive `Codable` enum (object, array, string, number, bool, null), new in
   `SchematicFile.swift`, so the custom-part objects round-trip without a typed model.
2. **Open and save.** On open, before the placed-part loop, each `customParts` entry
   goes through `PartCatalogStore.register(fileText:source:)` (plan-32-A). A
   uuid clash with different content makes the open fail with
   `SchematicFileError.customPartConflict(uuid:name:)`; the document is not touched
   (the apply's existing "throws without touching the document" rule). On save, the
   custom parts used by placed parts are collected by `partUUID`, deduplicated,
   and their registered file text is re-encoded with sorted keys.

Correctness risk concentrates in the open path: a placed part must resolve to the
registered part even when the registration renamed it (`SN74HC00N` → `SN74HC00N.2`).
The rule is that `partUUID` is the key, and the stored `partNumber` is what the
document showed at save time, used only when there is no `partUUID`. Schedule: the
file-shape change first (no behavior change for version 4), then the open path, then
the save path.

### Rejected alternatives

- **Embedding the custom part inside each placed part.** Rejected: two placements of
  one custom part would store its definition twice, and the two copies could drift.
  One definition per document, keyed by UUID.
- **Storing the registered text as a string.** Rejected: an escaped JSON string inside
  JSON is unreadable in a diff and breaks the sorted-keys guarantee.
- **Writing version 5 for every save.** Rejected: every saved document would become
  unreadable by the previous build. Version 5 only when a custom part is present.

## 4. Detailed Design

### 4.1 File shape

```json
{
  "format": "schematical",
  "version": 5,
  "customParts": [
    { "format": "schematical-part", "version": 1, "uuid": "6F1C…", "kind": "gate",
      "footprints": [ … ], "devices": [ … ] }
  ],
  "parts": [
    { "id": "…", "reference": "U1", "partNumber": "SN74HC00N.2", "partUUID": "6F1C…",
      "x": 200, "y": 120, "flipped": false }
  ]
}
```

- `customParts` (array, optional): one element per distinct custom part used by a
  placed part, in order of first placement (`parts` order). Missing or empty means none.
- `partUUID` (string, optional, parts entries): the UUID of a custom part; present
  exactly for a placed custom part.
- `partNumber` for a custom part is its registered (possibly suffixed) name, so a
  reader that ignores `customParts` still sees the name.

### 4.2 Open

- `SchematicFile.apply(to:catalog:fileURL:)`: before the placed-part loop, for each
  `customParts` element: re-encode it (sorted keys) to text and call
  `catalog.register(fileText:source:)`. Failures:
  - `PartRegistrationError.uuidInUse` → throw `SchematicFileError.customPartConflict(uuid:name:)`.
  - `PartRegistrationError.backend(message)` → throw `SchematicFileError.badCustomPart(message)`.
  A thrown error leaves the document untouched (the existing rule at the top of `apply`).
- In the placed-part loop, resolve with `partUUID` first: the registered part with that
  UUID (a new store lookup `part(uuid:)`); if there is no `partUUID`, `part(named:)` as now.
  A `partUUID` that resolves to nothing (a `customParts` entry missing) is an unknown part,
  collected into `unknown` as today.

### 4.3 Save

- `SchematicDocument` → `SchematicFile` encoding: collect each placed part whose
  `PlacedChip.part.uuid != nil`; deduplicate by UUID in placement order; for each, look up
  the registered file text (the store keeps it: `register` stores the re-encoded text in a
  `[uuid: String]` map, a new private property) and add the JSON object to `customParts`.
- `version` is 5 when `customParts` is non-empty, else 4.

## Phases

### Phase 1 — Shape: the JSON value type and the optional keys (no behavior change)

One line: the file can hold the new keys and reads them back, with no open or save behavior yet.

- [ ] `schematical/schematical/Document/SchematicFile.swift`: add `JSONValue` (recursive `Codable`).
- [ ] `SchematicFile`: add `customParts: [JSONValue]?` (written only when non-empty, sorted keys), and `Part.partUUID: String?` (written only when non-nil).
- [ ] `SchematicFile.currentVersion` → 5; `version` written as 5 only when `customParts` is non-empty, else 4 (§ 4.3). A version 4 document with no custom parts encodes byte-identically to the build at HEAD.
- [ ] Tests: `schematical/schematicalTests/SchematicFileV5Tests.swift`: (a) a version 4 document with no custom parts encodes with `"version" : 4` and no `customParts` key; (b) a version 5 document with a `customParts` element decodes and re-encodes to the same JSON value (sorted keys); (c) a version 6 document is refused with `newerVersion(6)`.

Acceptance: the three `SchematicFileV5Tests` cases pass, and the version 4 encoding in (a) equals the HEAD encoding of the same document.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicFileV5Tests` → `passed`; the HEAD comparison is the test's own golden string, written from `git show HEAD:schematical/schematical/Document/SchematicFile.swift` by running the HEAD build's encoder once (est. 6 min, one build).
Commit: —

### Phase 2 — Open: register the document's custom parts and resolve by UUID

One line: a version 5 document opens with its custom parts registered, and its placed parts resolve by UUID.

- [ ] `schematical/schematical/Backend/PartCatalog.swift`: `PartCatalogStore.part(uuid:)` (a `[String: CatalogPart]` map kept beside `partsByName`, filled by `register`).
- [ ] `PartCatalog.swift`: `register` stores the re-encoded file text in a `[uuid: String]` private map (used by Phase 3).
- [ ] `SchematicFile.swift`: `SchematicFileError.customPartConflict(uuid:name:)` and `badCustomPart(String)` with their `errorDescription` texts.
- [ ] `SchematicFile.apply(to:catalog:fileURL:)`: register each `customParts` element before the placed-part loop (§ 4.2); resolve each placed part by `partUUID` first.
- [ ] Tests (added to `SchematicFileV5Tests.swift`): (d) a version 5 file whose custom part is not in the catalog opens and the placed part's `chip.part.uuid` equals the stored UUID; (e) the same file opened twice does not duplicate the part in `entries`; (f) a file whose `customParts` UUID is already registered with different content throws `customPartConflict` and the document's `chips` is unchanged; (g) a placed `partUUID` with no `customParts` entry is reported in `unknownParts`.

Acceptance: cases (d)–(g) pass.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicFileV5Tests` → `passed` (est. 4 min).
Commit: —

### Phase 3 — Save: write the custom parts a document uses

One line: saving a document with custom parts writes version 5 with exactly the parts it uses.

- [ ] `SchematicDocument` save path: collect custom parts by `partUUID` in placement order, deduplicate, and read their text from the store's `[uuid: String]` map (Phase 2) into `customParts`.
- [ ] Set `version` to 5 when `customParts` is non-empty, else 4 (§ 4.3).
- [ ] `partNumber` written for a custom part is its registered name (suffix included).
- [ ] Tests (added to `SchematicFileV5Tests.swift`): (h) a document with two placements of one custom part writes one `customParts` element; (i) a document with one custom and one bundled part writes `partUUID` on the custom placement only and no `partUUID` on the bundled one; (j) save → open → save produces identical bytes.

Acceptance: cases (h)–(j) pass.
  Check: `xcodebuild test … -only-testing:schematicalTests/SchematicFileV5Tests` → `passed` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `SchematicFileV5Tests` (cases a–j), negative cases (c, f, g).
- Coverage check: `rg -n 'customPartConflict|partUUID' schematical/schematical/Document/SchematicFile.swift` shows the new error and the key; the test target compiles that file (it is in the app target the tests use).
- Runtime proof: save a schematic with one custom part from the app, quit, relaunch with the part not loaded, open the file: the placed part shows its name and pins (run skill, est. 15 min). This needs plan-32-C's editor to create the custom part; until then the proof uses a `.schpart` loaded by File › Load Part… (plan-32-A) and placed from the drawer.
- Doc sync: `docs/file-format.md` gets a version 5 section (`customParts`, `partUUID`, the version rule, the older-build refusal) and its header's "This document describes version 4" becomes 5.
- Final gate (once, after Phase 3): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (full Swift scheme, est. 15 min, background per the project's tiers) and `rg -c 'version' docs/file-format.md` shows the version 5 section.

## Decisions

- **Version 5 only when a custom part is saved** — the author's choice (no user-visible alternative was offered): an older build refuses version 5 files, so a document with no custom part stays readable by it. § 3.
- **`partUUID` is the key; `partNumber` is display only for custom parts** — the author's choice: a rename by a clash (§ plan-32-A) must not break resolution. § 3.
- **Custom parts are embedded whole, once per document, keyed by UUID** — the author's choice (rejected alternatives in § 3).
- **The owner's clash rules and the session-only rule** apply here as in plan-32-A (§ Decisions there).

## Corrections

(Filled in during execution.)

## Summary

The risk is the open path: a renamed custom part must still resolve to the placed
part, which is why resolution is by UUID. Untouched: version 4 files and the bundled
part design.
