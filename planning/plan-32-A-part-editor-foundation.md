# plan-32-A: Part editor foundation: single-part files and Load Part

Last updated: 2026-11-11
Overall Effort: huge (split into plan-32-A … plan-32-J)
Effort: large (3h–1d)
Depends on: nothing

This sub-plan builds the layer every later sub-plan writes against: a single-part
file (`.schpart`) the backend can load, a way to load one into the running app's
part library and drawer, and the File › Load Part… menu item that does it. The
behavioral outcome: choosing File › Load Part… on a valid `.schpart` file makes
its part appear in the drawer's picker under its drawer path, and the backend's
`part_library_find` returns it by name, for the rest of the session.

References:

- `docs/parts-format.md` — the shipped part-file format this extends (§ The file,
  § Devices, § Parts, § Footprints, § Loading rules).
- `docs/backend.md` — the C API section ("Using it") and the test layout.
- `docs/file-format.md` — the `.schematical` format (sub-plan B changes it).
- `backend/include/parts.h` — `struct Part`, `load_parts`, `load_footprints`.
- `backend/src/library/library.c` — `load_parts` (line 1058), the file, device and
  part key tables (lines 207–215), `load_part_footprint` (line 433).
- `backend/src/library/footprints.c` — `load_footprints` (line 855), the footprint
  parser `load_footprint` (line 833), the duplicate-id check (line 860).
- `schematical/schematical/Backend/PartCatalog.swift` — `PartLibraryHandle`
  (line 8), `PartCatalogStore.load(into:)` (line 708), `part(named:)` (line 1000),
  the private `entries` and `partsByName` (lines 910–911), `CatalogPart.init` (line 564).
- `schematical/schematical/Document/SchematicFileController.swift` —
  `SchematicFileCommands` (the File menu, the `CommandGroup(after: .newItem)` block).
- `schematical/Info.plist` — the app's declared document types.
- `AGENTS.md` — git rules (one commit per new plan; commit only own files).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| No uncommitted edits to the files this plan touches | `git status --short backend schematical/schematical/Backend schematical/schematical/Document schematical/schematicalTests docs` → no output | MET (re-run 2026-11-11 in the P-32 worktree, empty) |
| The backend builds from a clean configure | `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build` → exit 0 | MET (run 2026-11-11 in the P-32 worktree as `/tmp/p32-build`, exit 0; `/tmp/logic-build` is the main checkout's cache, see Corrections) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before starting, and again before deciding to stop.
> If you stop, report the current status of all prerequisites.

## 1. Goal

- A `.schpart` file (format `schematical-part`, version 1) that holds exactly one
  device with exactly one orderable part, and optionally the custom footprints
  that part uses, loads into the backend with `load_part_file`, and the loaded
  part is retrievable by name (`part_library_find`) and by UUID.
- The Swift app registers a loaded part into its catalog at runtime: the part is
  listed in the drawer under its `drawer` path, `part(named:)` returns it, and
  it can be placed on a schematic like a bundled part, for the rest of the session.
- File › Load Part… opens an open panel for `.schpart` files, registers the file,
  and reports a backend error message (naming the place in the file) on failure.

### Non-goals (explicit constraints)

- **Shipped data does not change.** No file under `schematical/schematical/Parts/`
  and no `footprints.json` / `datasheets.json` entry is edited. Golden files under
  `backend/tests/golden/` do not change (verify: `git diff --stat backend/tests/golden`
  → no output at the end of this sub-plan).
- **Existing loaders keep their behavior.** `load_parts`, `load_footprints` and
  `load_datasheets` accept and reject exactly what they accept and reject today.
  The new entry point is additive.
- **Session only.** Registered parts are not written into the app bundle (read-only)
  and are not copied anywhere; they are gone at the next launch. Persistence is a
  decided non-goal (§ Decisions).
- **No schematic file changes in this sub-plan.** Documents do not yet embed custom
  parts (plan-32-B).
- **No editor window.** The editor is plan-32-C.

## 2. Current State

- The backend loads every part file with `load_parts(library, text, source, &error)`
  (`backend/src/library/library.c:1058`). The top-level keys are fixed by
  `FILE_KEYS` (`library.c:207`): `format`, `version`, `kind`, `devices`. A part's
  footprint is resolved only from the library's footprint table:
  `load_part_footprint` calls `part_library_find_footprint(library, id)` (`library.c:439`)
  and rejects an unknown id with `unknown footprint "<id>"`. A part file therefore
  cannot carry its own footprint today.
- Footprints are loaded from `footprints.json` by `load_footprints`
  (`footprints.c:855`). Its per-footprint parser `load_footprint` is `static`
  (`footprints.c:833`). A footprint id that already exists in the library is
  rejected: `duplicate footprint id "<id>"` (`footprints.c:860`).
- `struct Part` (`backend/include/parts.h`) has no identity beyond `name`.
- The Swift catalog is built once at launch: `PartCatalogStore.load()`
  (`PartCatalog.swift:1131`) reads every bundled JSON, `footprints.json` and
  `datasheets.json` first (`PartCatalog.swift:708`), then `part(named:)` (line 1000)
  reads `partsByName`. `entries` and `partsByName` are `private var` (lines 910–911):
  nothing adds to them after launch.
- The File menu is `SchematicFileCommands` (`Document/SchematicFileController.swift`),
  built with `CommandGroup(after: .newItem)`; its Open…, Save and Save As… buttons
  are the precedent for a new menu item. `schematical/Info.plist` declares the
  document types.
- Counts (measured 2026-11-11):
  - Shipped part files: 26 kinds, one file each: `ls schematical/schematical/Parts/*.json`
    minus `footprints.json` and `datasheets.json` → 26 files.
  - Shipped orderable parts: `rg -o '"manufacturer"' schematical/schematical/Parts/*.json | wc -l` → 26729.
  - Shipped footprints: `rg -c '"id"' schematical/schematical/Parts/footprints.json` → 1212.
  - Existing backend tests use the `*_test` / `*_check` pattern in `backend/tests/`
    (`docs/backend.md`, § Layout).

### Verified properties

- **The loader's key tables and error texts are as cited.** Verified by reading
  `library.c:207–215` and `library.c:433–460`.
- **A footprint id added to a library that already holds footprints is checked
  against them.** The duplicate check (`footprints.c:860`) runs for every entry
  against `part_library_find_footprint`, so inline ids are checked against bundled
  ids. Verified by reading `footprints.c:855–870`.
- **Parts are added to one library across calls.** `load_parts` runs once per bundled
  file on the same handle (`PartCatalog.swift:720`), and a duplicate name across files
  is rejected (`docs/parts-format.md`, § Loading rules). A later `load_part_file` on
  the same handle therefore sees every bundled name. Verified by reading
  `PartCatalog.swift:716–722`.

## 3. Design Overview

Four pieces, each independently testable:

1. **Backend: `load_part_file`** (Phase 1). A new entry point beside `load_parts`.
   It takes the single-part file text, reads an optional `footprints` array with the
   same entry format as `footprints.json` (parsed by a `footprints.c` helper that
   reuses `load_footprint`), resolves the part's footprints against the library
   (inline ones now present, bundled ones already present), then runs the same
   device and part loader as `load_parts`. It enforces the single-part shape and
   requires `uuid`. `struct Part` gains `const char *uuid` (NULL for every bundled
   part), set from the file.
2. **Swift: runtime registration** (Phase 3). `PartCatalogStore` gets one method,
   `register(fileText:source:)`. It (a) reads `uuid` and applies the clash rules
   (§ Decisions), (b) rewrites the name to its suffixed form in the text before
   calling `load_part_file`, (c) adds the new `CatalogPart` to `entries` and
   `partsByName`, and (d) returns it. The drawer's category and package lists are
   computed properties over `entries`, so they update with no extra code.
3. **Swift: File › Load Part…** (Phase 4). A menu command in `SchematicFileCommands`
   opens an `NSOpenPanel` for the `.schpart` type (declared in `Info.plist`), reads
   the file, and calls `register`. A backend error shows an alert with the backend's
   message verbatim.
4. **Format documentation** (Phase 5). `docs/parts-format.md` gets a new section for
   single-part files and `docs/backend.md` the new entry point.

Where the correctness risk concentrates: **the library's name and footprint tables
are shared by every later sub-plan.** A registration that partially fails (for
example a footprint loads but the part is rejected) must leave the library as it
was. Phase 1 therefore validates the whole file into temporary storage and commits
footprints and the part only after every check passes. Schedule: Phase 1 first, when
the blast radius is one new function; the Swift-side registration (Phase 3) comes
after it has its own tests.

### Rejected alternatives

- **Encoding a custom footprint in `footprints.json` at run time.** Rejected: the
  bundle is read-only in the shipped app, and it would make a custom part's
  footprint visible to every later part load. Custom footprints live in the part file
  (owner decision).
- **Making `load_parts` accept the new keys.** Rejected: it would give the bundled
  file loader a `uuid` and a `footprints` key with a different meaning per file.
  A separate entry point keeps the shipped loader's rules unchanged.
- **Re-parsing the whole bundle to register one part.** Rejected: the bundle holds
  26729 shipped parts (`rg -o '"manufacturer"' schematical/schematical/Parts/*.json | wc -l`
  → 26729); a single-part file goes into the library directly.
- **Storing the UUID only in Swift.** Rejected: the document (plan-32-B) and the
  library must agree on one identity, and the backend loader is the only place that
  reads the file.

## 4. Detailed Design

### 4.1 File format (`schematical-part`, version 1)

```json
{
  "format": "schematical-part",
  "version": 1,
  "uuid": "6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C10",
  "kind": "gate",
  "footprints": [ { "id": "CUSTOM-6F1C2E8A", "description": "...", "style": "explicit", "pins": 14, "body": { "width": 8.0, "height": 19.0 }, "pads": [ ... ] } ],
  "devices": [ { "id": "...", "function": "...", "description": "...", "drawer": ["Logic", "Gates"], "pinouts": { ... }, "...kind keys...": "...", "parts": [ { ...one part... } ] } ]
}
```

| Key | Rule |
|---|---|
| `format` | exactly `"schematical-part"` |
| `version` | exactly `1` |
| `uuid` | required; 36 characters, hexadecimal digits and hyphens in the 8-4-4-4-12 layout, uppercase (`UUID.uuidString` form) |
| `kind` | one of the 26 kinds; the same names `load_parts` accepts |
| `footprints` | optional array; each entry follows the `footprints.json` entry format (`footprints.c`, `load_footprint`). Its ids must be unique in the library after the bundled footprints are loaded (`footprints.c:860`) |
| `devices` | exactly one device; that device has exactly one part |

No other top-level key is allowed (same rule as `FILE_KEYS`). A part's `footprint`
may name a bundled footprint id or an id listed in `footprints`.

### 4.2 Backend changes

- `backend/include/parts.h`: `struct Part` gains `const char *uuid;` (NULL for bundled
  parts). Declare `int load_part_file(PartLibrary *library, const char *json, const char *source, char **error);` beside `load_parts` (`parts.h:217`).
- `backend/src/library/footprints.c`: move the body of `load_footprints`'s per-entry
  loop into `static int load_footprint_array(…)` and expose
  `int load_footprint_entries(PartLibrary *library, const cJSON *array, const char *source, char **error);` (declared in
  `backend/src/library/library_internal.h`, the header both files already include).
  `load_footprints` calls it, so bundled behavior is unchanged.
- `backend/src/library/library.c`: `load_part_file` parses the text, checks the
  top-level keys against `PART_FILE_KEYS = {format, version, uuid, kind, footprints, devices}`,
  checks `format`, `version`, `uuid` and the one-device, one-part shape, loads
  `footprints` through `load_footprint_entries` into a **staging** list, then runs
  the same `load_device` code `load_parts` uses, with the staged footprints visible
  to `part_library_find_footprint`. The commit step (adding staged footprints and the
  part to the library) runs only if every check passed; on any error the library is
  unchanged.
- The error text follows the `load_parts` form: `<source>: devices[0] (<id>): parts[0] (<name>): <message>`.

### 4.3 Swift changes

- `PartCatalogStore.register(fileText: String, source: String) throws -> CatalogPart`
  (in `PartCatalog.swift`, beside `load(into:)`):
  1. Parse `uuid` and `name` from the text (`JSONSerialization`).
  2. If a part with that `uuid` is registered: the stored text and the new text
     are compared after canonical re-encoding (sorted keys); equal → return the
     registered part; different → throw `PartRegistrationError.uuidInUse(uuid, name)`.
  3. Otherwise, if `name` is a registered part name, the new name is `name` plus
     `.N` for the lowest `N ≥ 2` such that the result is not registered (`SN74HC00N.2`,
     `SN74HC00N.3`). The text's `name` is rewritten to the suffixed name.
  4. Call `load_part_file` on the store's own `library` handle. The backend's staging
     (§ 4.2) does the validation; a failure throws `PartRegistrationError.backend(message)`
     and leaves the library unchanged.
  5. Build `CatalogPart` with `CatalogPart.init(_:footprint:connectorFootprint:footprintChoices:)`
     (`PartCatalog.swift:564`), using `part_library_find_footprint` for each footprint
     id, and insert into `entries` and `partsByName`.
- `Part.uuid` is read into `CatalogPart.uuid: String?` (new stored property; nil for
  bundled parts). `CatalogPart` is `Hashable`; the new property joins the hash.

### 4.4 File › Load Part…

- `SchematicFileCommands` gets `Button("Load Part…")` in the `CommandGroup(after: .newItem)`
  block, after the Open… item. Keyboard shortcut: none in this sub-plan (plan-32-C
  adds ⇧⌘N for New Part…).
- The action opens `NSOpenPanel` with `allowedContentTypes = [UTType("com.schematical.part")]`,
  reads the file (UTF-8), and calls `register`. A `PartRegistrationError` shows an
  `NSAlert` with the message. A successful load selects nothing and changes no document.
- `schematical/Info.plist`: an exported type `com.schematical.part`, conforming to
  `public.json`, with the `schpart` extension, in the same form as the `.schematical`
  type already declared there.

## Phases

### Phase 1 — Backend: `load_part_file`

One line: a new loader entry point with no caller; nothing in the app changes until
Phase 3, so this lands alone.

- [x] `backend/include/parts.h`: add `const char *uuid;` to `struct Part` (after `name`); declare `load_part_file`. Also declares `part_library_find_uuid` (§ 1 Goal: "retrievable … by UUID"; no task named it).
- [x] `backend/src/library/footprints.c`: extract the per-entry body of `load_footprints` into `load_footprint_entries` (declared in `backend/src/library/library_internal.h`); `load_footprints` calls it. Done as `footprint_read_entries`, declared in `footprint.h` (Corrections): `footprint_read_file` calls it.
- [x] `backend/src/library/library.c`: implement `load_part_file` with `PART_FILE_KEYS`, the format/version/uuid checks, the one-device one-part shape check, staged footprints, and the all-or-nothing commit (§ 4.2). Staging: `PartLibrary.staged_footprints`, read by `part_library_find_footprint` only during the load; the commit is `commit_parts` (shared with `load_parts`), which builds every grown table on the side before swapping.
- [x] `backend/src/library/library.c`: set `Part.uuid` from the file; bundled parts stay NULL. (`part_file_test` (a) checks `SN74AHC1G00DBVR`'s uuid is NULL.)
- [x] Tests: new `backend/tests/part_file_test.c`, registered in `backend/CMakeLists.txt` beside the other `*_test` programs. Cases: (a) a valid gate file with one inline footprint loads, `part_library_find` returns it, `uuid` matches; (b) bundled footprint id used with no inline footprints loads; (c) missing `uuid` → error naming `uuid`; (d) two devices → error; (e) an inline footprint id equal to a bundled id → `duplicate footprint id`; (f) a bad inline footprint pad count → the footprint error text; (g) a rejected file leaves `part_library_count` and `part_library_footprint_count` unchanged; (h) an unknown top-level key → error. `/tmp/p32-build/part_file_test` → `PASS: load_part_file (plan-32-A cases a-h)`; (g) is a part rejected (duplicate name) after its inline footprint checked, and also checks the footprint id and uuid are not found afterwards.
- [x] `docs/backend.md`: one paragraph under "Using it" for `load_part_file` (Phase 5 finishes the docs). Also a `part_file_test` entry under § Tests.

Acceptance: `part_file_test` passes all cases (a)–(h), and bundled loading is unchanged.
  Check: `cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build -R 'part_file|listing|footprint'` → 100% passed (est. 3 min; the scoped run includes the existing `listing_check` and footprint tests, which would catch a change to bundled loading).
  Result (2026-11-11, `/tmp/p32-build`): `100% tests passed, 0 tests failed out of 3`.
Commit: (see next commit)

### Phase 2 — Golden and fixture check for the shared loader

One line: prove the refactor of `load_footprints` changed nothing for bundled data.

- [x] Run the existing footprint and listing checks against the goldens: `backend/tests/golden/footprints.txt` and `backend/tests/golden/parts.txt` are regenerated only into `/tmp` and diffed, never written.
- [x] No file under `backend/tests/golden/` is changed. (`git diff --stat backend/tests/golden` → empty.)

Acceptance: the regenerated `footprints.txt` and `parts.txt` are byte-identical to the goldens.
  Check: `/tmp/logic-build/footprint_check --print > /tmp/fp.txt && cmp /tmp/fp.txt backend/tests/golden/footprints.txt` → exit 0 (est. 1 min); `/tmp/logic-build/listing_check --print > /tmp/parts.txt && cmp /tmp/parts.txt backend/tests/golden/parts.txt` → exit 0 (est. 1 min). (The `--print` form is the one `docs/backend.md` gives.)
  Result (2026-11-11, `/tmp/p32-build`, files in `/tmp/p32/`): `cmp` footprints → exit 0; `cmp` parts → exit 0.
Commit: (same commit as Phase 1)

### Phase 3 — Swift: runtime registration

One line: the catalog accepts a part at run time, with the clash rules, and the drawer lists it.

- [ ] `schematical/schematical/Backend/PartCatalog.swift`: add `PartRegistrationError` (`uuidInUse`, `backend(String)`, `notAPartFile`) with `errorDescription` texts.
- [ ] `PartCatalog.swift`: add `CatalogPart.uuid: String?` (nil for bundled parts; included in `Hashable`).
- [ ] `PartCatalog.swift`: implement `PartCatalogStore.register(fileText:source:) throws -> CatalogPart` per § 4.3 (steps 1–5). The name rewrite (step 3) edits the JSON object and re-serializes it with sorted keys.
- [ ] `PartCatalog.swift`: `register` inserts into `entries` and `partsByName`; because `categories`, `packages` and `kinds(inCategory:)` read `entries`, the drawer reflects the new part with no other change.
- [ ] Tests: new `schematical/schematicalTests/PartRegistrationTests.swift`: (a) a valid file registers and `part(named:)` returns it, with its drawer path in `categories`; (b) registering the same file twice returns the existing part (no duplicate in `entries`); (c) a different file with the same uuid throws `uuidInUse`; (d) a new uuid with a bundled name registers as `SN74HC00N.2`, then a third as `SN74HC00N.3`; (e) a file the backend rejects throws `backend(...)` and `entries.count` is unchanged; (f) a registered part's `kind` and `pins` equal the backend's.

Acceptance: all six `PartRegistrationTests` cases pass.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartRegistrationTests` → `Test Suite 'PartRegistrationTests' passed` (est. 4 min, the first build dominates; it is the smallest scope that runs the registration path against the real backend).
Commit: —

### Phase 4 — File › Load Part…

One line: the menu item and its open panel, reusing Phase 3's `register`.

- [ ] `schematical/schematical/Document/SchematicFileController.swift`: add `Button("Load Part…")` to `SchematicFileCommands` after Open…; the action runs an `NSOpenPanel` (`allowedContentTypes` for `com.schematical.part`) and calls `PartCatalogStore.shared.register(fileText:source:)`.
- [ ] Error path: `PartRegistrationError` → `NSAlert` with `errorDescription`; no state changes.
- [ ] `schematical/Info.plist`: exported type `com.schematical.part`, extension `schpart`, conforming to `public.json`.
- [ ] Tests: `PartRegistrationTests` gains (g): the open-panel handler's file-reading helper (factored out as `PartFileReader.read(url:) throws -> String`) returns the file's text and throws `notAPartFile` for a non-UTF-8 file.

Acceptance: the Load Part… menu item exists in the File menu, and a fixture `.schpart` file opened from it appears in the drawer (runtime proof, § Validation).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartRegistrationTests` → passed (est. 4 min); then the run skill's app launch with `File › Load Part…` on a fixture, observed in the drawer's search for the part's name (est. 10 min; this is the only check that needs the real app, because the menu wiring cannot be unit-tested).
Commit: —

### Phase 5 — Format documentation

One line: the shipped format doc and the backend doc describe the new file.

- [ ] `docs/parts-format.md`: new section "Single part files" with the § 4.1 table, the one-device rule, the uuid rule, the suffix rule of § 4.3, and an example file.
- [ ] `docs/backend.md`: finish the `load_part_file` paragraph (Phase 1's stub), and the `Part.uuid` field.
- [ ] `docs/parts-format.md` § Loading rules: one bullet that a single part file's name clashes are resolved by the app (suffix), not by the loader (the loader still rejects a duplicate name, per `load_part_file`).

Acceptance: the new section and the bullet exist, and every key in § 4.1 appears in the section's table.
  Check: `rg -c '"uuid"|"footprints"|"schematical-part"' docs/parts-format.md` → at least 3 (est. 1 min).
Commit: —

## Validation Plan

- Tests: `backend/tests/part_file_test.c` (Phase 1, cases a–h); `schematicalTests/PartRegistrationTests.swift` (Phases 3–4, cases a–g). Negative and error cases: missing uuid, two devices, duplicate footprint id, unknown key, non-UTF-8 file, uuid clash with different content, a backend rejection leaving the library unchanged.
- Coverage check: the backend CMake target compiles `library.c` and `footprints.c` into the test binaries (the new function is in the denominator): `rg -n 'load_part_file' backend/tests/part_file_test.c backend/src/library/library.c` shows the call site and the definition.
- Runtime proof: launch the app (run skill), File › Load Part… on a fixture `.schpart` with a custom footprint, then the drawer's search finds the fixture's name. The fixture is created by hand in `/tmp` with the § 4.1 shape.
- Doc sync: `docs/parts-format.md` (new section), `docs/backend.md` (entry point, `Part.uuid`).
- Final gate (run once, after Phase 5): `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build` in the background per the project's test tiers (est. 15–25 min), plus the Phase 3–4 Swift scheme run (est. 4 min). Per-phase checks stay scoped.

## Decisions

- **Part kinds in v1: all 26 kinds** — decided by the owner, 2026-11-11. This sub-plan covers the file format and loading for every kind; the form for each kind is plan-32-C (gate) and plan-32-F through J (the rest).
- **Loaded parts last until quit** — decided by the owner, 2026-11-11. Registered parts are in memory only. Settles Phase 3's scope: no copy into Application Support.
- **A name clash keeps both parts** — decided by the owner, 2026-11-11. The new copy is renamed with a numeric suffix: `SN74HC00N.2`, `.3`, the lowest free number from 2. The separator `.` is the author's choice; the owner asked for a numeric suffix and did not name a separator. Settles § 4.3 step 3.
- **Every custom part has a UUID** — decided by the owner, 2026-11-11. The UUID is the part's identity; the name is for display and the BOM. Bundled parts have no UUID (`Part.uuid` NULL). Settles § 4.1.
- **A custom footprint lives in the part file only** — decided by the owner, 2026-11-11. `footprints.json` is not written. Settles § 4.2 and the rejected alternative in § 3.
- **Extension `.schpart`; File › New Part… (⇧⌘N) is the editor's entry point** — decided by the owner, 2026-11-11. This sub-plan declares the type; plan-32-C adds New Part….
- **Same UUID with the same content reuses the registered part; with different content, it is refused** — the author's choice (no user-visible alternative was offered): a UUID names one definition, and silently replacing one would change pins under placed parts. Settles § 4.3 step 2.

## Corrections

- **Build directory** (2026-11-11). `/tmp/logic-build` is configured from the main
  checkout (`cmake -S backend -B /tmp/logic-build` in the worktree → "does not match the
  source … used to generate cache"). Every check of this plan that names
  `/tmp/logic-build` runs in `/tmp/p32-build`, configured from the P-32 worktree; same
  commands otherwise.
- **The footprint loader's shape differs from § 2 / § 4.2** (read 2026-11-11).
  `load_footprints` is in `library.c` (not `footprints.c:855`); `footprints.c` has
  `footprint_read_file`, which already stages a file's footprints into a `FootprintList`
  before `load_footprints` commits them, and the shared header is `footprint.h`
  (`library_internal.h` holds only the arena context). So the extracted helper is
  `footprint_read_entries(KindContext *, const cJSON *array, const PartLibrary *, FootprintList *)`,
  declared in `footprint.h`, called by `footprint_read_file` and `load_part_file`. The
  design (staged footprints, all-or-nothing commit) is unchanged.
- **A copied device would have been rejected as a duplicate device** (found 2026-11-11,
  not in the plan). `load_device` checks `"<kind>/<device id>"` against every loaded
  device, and a copy of a bundled part (plan-32-C § 4.5) keeps the bundled device id
  (`1G00`), as do several custom parts of one device. A single part file therefore skips
  the library device check and adds no device id (`FileParts.single`); the part name and
  uuid stay unique. `part_file_test` (a) and (b) both load a `1G00` device copy.
- **The backend also keeps the uuid** (2026-11-11). § 1 asks for the part to be
  retrievable "by UUID"; no task named the function. Added
  `part_library_find_uuid`, and a second file with a loaded uuid is rejected
  (`uuid "…" is already loaded`); the Swift side (§ 4.3 step 2) answers a same-uuid
  file before calling the backend, so this is the backstop.

- **A 27th kind is planned** (noted 2026-11-11 by plan-33's planning
  commit) — plan-33 (capacitors) adds the kind `capacitor` and the file
  `Parts/capacitor.json` (plan-33-E). Once it lands there are 27 kind files;
  plan-32-C's test (b), which compares `PartDraft.kindNames` with the
  `Parts/*.json` file names, then sees 27. plan-33 adds no part-editor form
  for it (plan-33-A § Decisions).
- **A 28th kind is planned** (noted 2026-11-11 by plan-34's planning
  commit) — plan-34 (resistors) adds the kind `resistor` and the file
  `Parts/resistor.json` (plan-34-E, F). Once both plans land there are 28
  kind files, and plan-32-C's test (b) sees 28. plan-34 adds no part-editor
  form for it (plan-34-G § Decisions).

## Summary

The real risk is the backend's all-or-nothing commit in `load_part_file`: a partial
registration would leave the library's name and footprint tables inconsistent for
every later load. Everything else is additive. Left untouched: every shipped part file,
`footprints.json`, `datasheets.json`, and the goldens.
