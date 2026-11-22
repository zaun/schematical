# plan-53-D: Replace a loaded part, live

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-53-C

Editing a User Library part updates it in place, **immediately**: the picker,
the library file and every placed chip in every open schematic switch to the
new version without a relaunch (decided by the owner, 2026-11-21). This
sub-plan builds the engine for that, with no editor UI (plan-53-E):

- the backend can retire a loaded part, so its name, uuid and inline ids are
  free to load again;
- `PartCatalogStore.updateUserLibraryPart(uuid:fileText:)` swaps the part under
  the same uuid, writes the library file, and reports the new part;
- every open schematic that places the part has its chips switched to the new
  version, wires on pins that no longer exist left as loose ends.

References:

- plan-53-A, plan-53-B, plan-53-C
- `backend/src/library/library.c` — `struct PartLibrary` (`:120`),
  `part_library_unload_last_part_file` (`:1594`), `commit_parts` (`:1052`),
  `load_part_file` (`:1538`); `backend/include/parts.h`; `docs/backend.md` Notes
- `schematical/schematical/Canvas/SchematicModel.swift` — `PlacedChip.resized(to:)`
  (`:897`), `setHeaderSize` (`:2113`), `loosen` (`:5121`), the contents loader that
  calls `history.clear()` (`:1834`)
- `schematical/schematical/Document/SchematicFileController.swift` — `all`
  (`:47`), `willReplaceDocument` (`:65`)
- `schematical/schematical/ContentView.swift:104` — where a window's
  `SimulationController` and `SchematicFileController` are made

## Prerequisites

Stated in plan-53-A; this sub-plan additionally needs plan-53-C complete.

| Must be true | Command | Status |
|---|---|---|
| plan-53-C is complete and archived | `ls planning/completed/plan-53-C-*.md` → one file | NOT MET at writing (unbuilt) |
| The catalog has `removeFromUserLibrary` and `userLibrary` | `grep -c 'func removeFromUserLibrary' schematical/schematical/Backend/PartCatalog.swift` → `1` | NOT MET at writing |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue, and report all of them if you stop.

## 1. Goal

- `part_library_retire_part` removes a single-part-file part, its name, uuid and
  the inline footprint and datasheet ids the caller names; loading a new file
  with the same uuid, name and ids then succeeds.
- `updateUserLibraryPart(uuid:fileText:)` replaces a User Library part's content
  under the same uuid, on disk and in the session.
- After it, every chip of that part in every open schematic is the new version;
  a chip's wire on a pin the new version lacks is a loose end, and a schematic
  that was running is stopped first.

### Non-goals (explicit constraints)

- **The uuid never changes** in an update. A different uuid is Save As… (plan-53-E).
- **The kind cannot change**: an update whose `kind` differs from the part's is
  refused.
- **Remove is unchanged** (plan-53-B): a removed part is hidden, not retired,
  because a schematic that places it is simulated by name and would no longer
  find it.
- **No other backend behavior changes.** Bundled parts have no uuid and cannot
  be retired; the Part Editor's scratch-library validation is unchanged.
- **The update is not undoable** and clears the undo history of each schematic
  it changes (§ 3).
- **Pin meaning is not preserved across versions.** Wires follow the pin's
  position (its index); a version that reorders or renames pins rewires them
  without notice. The owner chose immediate update knowing this
  (2026-11-21).

## 2. Current State

- The backend library only grows: `commit_parts` rebuilds the name, uuid,
  footprint and datasheet tables with the new file added; the one removal,
  `part_library_unload_last_part_file` (`library.c:1594`), takes back the latest
  `load_part_file` while nothing else was loaded since. Arena memory is freed
  only there and in `part_library_destroy` (`library.c:160`), so a `Part`
  pointer stays valid until then.
- A single part file's part is in `library->parts[]`, `library->names`,
  `library->uuids`; its inline footprints in `library->footprints[]` and
  `footprint_ids`; its inline datasheets in `library->datasheets[]` and
  `datasheet_ids` (`struct PartLibrary`, `library.c:120–143`). `devices` is not
  touched by a single part file (`file->single`, `library.c:1088`).
- Simulation and the voltage check find a chip's part **by name**: the netlist
  carries `partNames: chips.map(\.partNumber)` (`Simulation.swift:231`) and calls
  `circuit_add_part(circuit, name, …)` (`Simulation.swift:409`); `VoltageCheck`
  calls `part_library_find(library.pointer, chip.partNumber)`
  (`VoltageCheck.swift:67`).
- A placed chip holds its `CatalogPart` by value (`PlacedChip.part`,
  `SchematicModel.swift:779`, a `let`). `PlacedChip.resized(to:)` builds the same
  chip on another part, keeping id, reference, symbol, origin, flip, board
  placement and options. `setHeaderSize` (`:2113`) is the precedent for
  changing a chip's pin count: it `loosen`s the wire ends on pins at index ≥
  the new pin count (`loosen` turns each into a loose junction at the same
  point, `:5121`), swaps the chip, re-anchors the footprint on pad 1 and
  `settleWires`.
- A wire end names a pin by chip id and index (`.pin(ref)`, `ref.index`), not by
  designator (`SchematicModel.swift:2125`). A trace is geometry only
  (`BoardTrace.swift:9–17`: layer, width, points); nothing in it names a pad.
- Undo steps are diffs of chip, wire and other values
  (`UndoHistory.swift:104–172`); loading a file's contents ends with
  `history.clear()` (`SchematicModel.swift:1834`).
- Every window has a `SchematicFileController`, registered in `init`
  (`SchematicFileController.swift:76`) in the weak table `all` (`:47`), and a
  `willReplaceDocument` closure that stops its simulation
  (`ContentView.swift:104–116`).
- `PartCatalogStore.library` (`PartLibraryHandle`) is the one C library every
  document and simulation of the app uses (`ContentView.swift:98–104`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Swift places that look a part up in the C library by name | 2 files | `grep -rln 'part_library_find(' schematical/schematical --include='*.swift'` → `Backend/PartCatalog.swift`, `Simulation/VoltageCheck.swift` |
| Test places that build a `SchematicFileController` | 2 files | `grep -rln 'SchematicFileController(' schematical/schematicalTests` → `KiCadExportTests.swift`, `FileDesignRulesTests.swift` |

### Verified properties

- **Old `Part` memory stays valid after a retire.** Read
  `part_library_unload_last_part_file`: it frees only chunks newer than
  `last_arenas`; `part_library_destroy` frees all. A retire that frees no arena
  chunk leaves every `Part`, `PartFootprint` and `PartDatasheet` pointer valid.
- **A custom part's tables are the three listed.** Read `commit_parts`
  (`library.c:1052–1125`): it merges names, uuids, footprint ids and datasheet
  ids; devices only for a multi-part file.

## 3. Design Overview

Three pieces in order:

1. **Backend `part_library_retire_part`** (new, `parts.h`): removes one
   single-file part. The caller names the inline footprint and datasheet ids to
   drop, because the library does not record which belong to which file (the
   catalog knows: they are in the part's registered text).
2. **Catalog `updateUserLibraryPart`**: write the new file, retire the old
   part, load the new text, swap the catalog's records.
3. **Documents**: every open document's chips of that uuid become the new
   `CatalogPart`.

**Update order** (all-or-nothing as far as possible):

1. Check: the uuid is listed `user`; the new text's `uuid` equals it and its
   `kind` equals the old text's.
2. Write `<uuid>.schpart` with the new text (atomic). A failure throws
   `libraryWrite` with nothing else changed.
3. Retire the old part (§ 4.1) and `load_part_file` the new text, its name made
   unique against every *other* part by the suffix rule. If the load fails,
   load the old text back, rewrite the old file text, and throw the backend's
   message.
4. Replace the part in `entries`, `partsByName` (dropping the old name key when
   the name changed), `partsByUUID` and `registeredTexts`; listing stays `user`.

**Documents** (`SchematicFileController.updateOpenDocuments(uuid:to:)`, static,
over `all`): for each controller whose document places the uuid, call its
`willReplaceDocument()` (stops a running simulation), then
`document.updatePart(uuid:to:)`:

- one pass over the chips with `part.uuid == uuid`;
- wire ends on those chips' pins at index ≥ the new pin count are `loosen`ed
  (as `setHeaderSize` does);
- each chip becomes `old.resized(to: newPart)`; its board reference offset is
  reset and its footprint is re-anchored on pad 1 as `setHeaderSize` does (the
  new footprint may have other pads), then `settleWires`;
- the swap runs outside `recordEdit` and ends with `history.clear()`: the
  older undo steps hold chips of the old version, and a step back would put an
  old `CatalogPart` under a name the library now resolves to the new one, so
  the simulation and the voltage check would disagree with the canvas.

**Correctness risk** concentrates in the backend retire (table rebuilds, no
freed memory) and in the document swap (wires, footprint anchor, history). Each
has its own test with an old and a new version that differ in pin count.

**Gate class.** Behavior is new; tests are the gate. Bundled-library counts and
every existing backend test result are expected not to change.

Rejected alternatives:

- **Free the old part's arena memory** — circuits and the Swift side may still
  hold its pointers; the leak is the size of one part per update, until the
  process ends.
- **Record which inline ids a part file owns inside the library** — the
  catalog already has them in the registered text; an extra table is more
  backend for no gain.
- **Record the update as one undo step** — stepping back would put an old
  `CatalogPart` under a name the library now resolves to the new version
  (above).
- **Delete wires on pins that vanish** — `loosen` keeps the wire's point and is
  the precedent.

## 4. Detailed Design

### 4.1 Backend

```c
/* parts.h. Takes one single-part-file part out of the library: its name, uuid
   and the listed inline footprint and datasheet ids (the library keeps the
   memory until part_library_destroy, so Part pointers already handed out stay
   valid). Returns 0; -1 when no part has this uuid (nothing changes). */
int part_library_retire_part(PartLibrary *library, const char *uuid,
                             const char *const *footprint_ids, size_t footprint_count,
                             const char *const *datasheet_ids, size_t datasheet_count);
```

In `library.c`, built on the side like `part_library_unload_last_part_file`:
find the part by uuid; make `parts[]` without it; rebuild `names` from the
remaining parts' names and `uuids` from their uuids; make `footprints[]` and
`datasheets[]` without the listed ids and rebuild `footprint_ids` and
`datasheet_ids` from what stays; on any allocation failure free the new tables
and return -1 with the library as it was. On success `count` and the two other
counts shrink, and `last_part_file = 0` (the latest-file undo no longer
applies). `devices` is not touched.

### 4.2 Catalog

```swift
func updateUserLibraryPart(uuid: String, fileText: String) throws -> CatalogPart
```

Per § 3. New errors in `PartRegistrationError`: `.notInUserLibrary(name:)`
(the uuid is not listed `user`), `.identityChanged(detail:)` (uuid or kind
differs). The ids to retire come from the old registered text's
`footprints[].id` and `datasheets[].id`. After the swap the catalog calls
`SchematicFileController.updateOpenDocuments(uuid:to:)`.

### 4.3 Documents

`SchematicDocument.updatePart(uuid: String, to newPart: CatalogPart)` and
`SchematicFileController.updateOpenDocuments(uuid:to:)` per § 3. The footprint
re-anchor and the wire loosening are lifted out of `setHeaderSize` into one
private helper both use, so the header's behavior (its tests) does not change.

## Compatibility / Format Impact

- **Backend:** one new public function. No existing behavior changes.
- **Part files and `.schematical`:** unchanged. A schematic saved after an
  update embeds the new version under the same uuid.
- **A schematic saved before an update**, opened later, embeds the old version
  under the same uuid: plan-53-A's rule gives that copy a fresh uuid, and the
  open offer (plan-53-C) lists it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot tasks stay, struck through with evidence; fill
> each `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — Backend `part_library_retire_part`

Safe alone: a new function nothing calls yet.

- [ ] `backend/include/parts.h` and `backend/src/library/library.c`: § 4.1.
- [ ] `backend/tests/part_file_test.c`, cases after the last lettered one: load a part with an inline footprint and datasheet; retire it with its ids: `part_library_find` and `_find_uuid` return NULL, `part_library_find_footprint` NULL, counts back to the bundled counts, bundled parts unchanged; the old `Part*` read before the retire still has its name afterwards; load a changed version with the same uuid, name and ids: it loads and `_find_uuid` returns it; retiring an unknown uuid returns -1 and changes no count; retire then `part_library_unload_last_part_file` returns -1 (no latest file to take back).
- [ ] `docs/backend.md` Notes, "Single part files": `part_library_retire_part` — what it removes, that memory stays, that it is how an app replaces a loaded part.

Acceptance: a retired part's name, uuid and ids load again and old pointers stay valid.
  Check: `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build --target part_file_test && ctest --test-dir /tmp/logic-build -R part_file_test --output-on-failure` → `100% tests passed` (est. 2–4 min).
Commit: —

### Phase 2 — The catalog update

- [ ] The C function reaches Swift through the bridging header like `load_part_file` (`PartLibraryHandle` wrapper in `Backend/`); a Swift wrapper `PartLibraryHandle.retire(uuid:footprintIDs:datasheetIDs:)`.
- [ ] `PartCatalog.swift`: `updateUserLibraryPart`, the two new errors (§ 4.2); `SchematicFileController.updateOpenDocuments` is called through a closure the store holds (`var partUpdated: (String, CatalogPart) -> Void`, set by `SchematicFileController` at first use) so the catalog does not depend on the controller in tests.
- [ ] `schematicalTests/UserLibraryUpdateTests.swift` (new, temp-folder library): update a library part (a gate with an inline footprint) with a changed pin name and description: `part(uuid:)` and `part(named:)` return the new one, `registeredText` and the library file hold the new text, the picker lists it once under `User Library`, `groups(matching:)` finds it by its new description; a renamed update frees the old name (`part(named: old)` nil) and holds the new; a rename to a name another part has gets the suffix; the old `CatalogPart` value is still readable; an update with a different `kind` or `uuid` throws `identityChanged` and nothing changes; an update of a `file` part throws `notInUserLibrary`; a text the backend rejects throws its message, the old version is still the library's part and the file still holds the old text; an unwritable library folder throws `libraryWrite` and changes nothing; a relaunch (a second store on the same folder) lists the new version.

Acceptance: an update takes effect in the catalog and on disk at once, and every failure leaves the old version.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryUpdateTests` → `** TEST SUCCEEDED **` (est. 5–10 min; the app and backend build first).
Commit: —

### Phase 3 — Open schematics follow

- [ ] `SchematicModel.swift`: the shared helper lifted out of `setHeaderSize`; `updatePart(uuid:to:)` (§ 3). `setHeaderSize` calls the helper and is otherwise unchanged.
- [ ] `SchematicFileController.swift`: `static func updateOpenDocuments(uuid:to:)` (§ 3), and the store's `partUpdated` closure set to it.
- [ ] `schematicalTests/UserLibraryUpdateTests.swift`: with two documents and their controllers (as `KiCadExportTests` builds one), each placing the part, one with a wire on pin 1 and a wire on pin 3: after an update to a version with 2 pins both documents' chips are the new `CatalogPart`, the pin-1 wire is still on the chip, the pin-3 wire's end is a loose junction at its old point, `canUndo` is false in both, and the `willReplaceDocument` closure ran once for each; a third document without the part is untouched (same chips, undo history kept); the chip's id, reference, origin and board placement are kept; after an update to a version whose footprint has other pads the chip's pad 1 sits where it did; `setHeaderSize` behavior is unchanged (the three header test classes above).

Acceptance: open documents show the new version at once, wires on vanished pins are loose, and unrelated documents are untouched.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryUpdateTests -only-testing:schematicalTests/LooseEndTests -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/ReferencePositionTests` → `** TEST SUCCEEDED **` (est. 8–15 min; the last three are the tests that call `setHeaderSize` — `grep -rln setHeaderSize schematical/schematicalTests` → `ReferencePositionTests`, `SchematicUndoTests`, `LooseEndTests` — and cover the helper it now shares).
Commit: —

## Validation Plan

- Tests: per phase, with the failure cases (bad text, wrong uuid or kind, not in
  the library, unwritable folder) and the three documents case.
- Coverage check: `part_file_test`, `UserLibraryUpdateTests` and the header
  tests cover the retire, the update and the shared helper; the live effect on a
  running simulation is covered by the real-app run in plan-53-F.
- Runtime proof: plan-53-F's run edits a placed library part and sees the
  canvas change with no relaunch.
- Doc sync: `docs/backend.md` (Phase 1), `docs/parts-format.md` § Single part
  files, "The User Library" paragraph: an edit updates the part everywhere at
  once (done in plan-53-E with the editor).
- Final gate (run ONCE, after Phase 3):
  `ctest --test-dir /tmp/logic-build -R 'part_file_test|library_check'` and
  `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryUpdateTests -only-testing:schematicalTests/UserLibraryCatalogTests -only-testing:schematicalTests/PartRegistrationTests -only-testing:schematicalTests/SchematicFileV5Tests`
  → all pass (est. 10–15 min).

## Decisions

- An edit to a library part takes effect immediately in the picker and in open schematics — decided by the owner, 2026-11-21, after being told it needs a backend call, that open schematics switch to the new version, and that a version with other pins can break placed wires.
- Wires on pins the new version lacks become loose ends — author's call: the `setHeaderSize` precedent; nothing is deleted.
- The update clears the undo history of the schematics it changes — author's call, forced by by-name simulation (§ 3).
- Remove still hides rather than retires — author's call (see Non-goals).

## Corrections

(Filled in during execution.)

## Summary

The risk is the pair of mechanisms that make "immediately" real: the
backend's table rebuild without freeing memory, and the document swap with its
wires, footprint and history. Untouched: part files, the `.schematical` format,
bundled parts, the Part Editor's validation, Remove.
