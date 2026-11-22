# plan-53-B: The User Library on disk, and its categories

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-53-A

The catalog gets a persistent **User Library**: custom parts kept as `.schpart`
files in a folder in the app's own Application Support, registered into the
catalog at every launch. Every registered custom part is now listed in the
parts picker in one of two categories: **User Library** (kept on disk) or
**File Library** (this session only: from a file that embedded it, or loaded
"once"). This sub-plan builds the model and the disk side, with no new windows
or prompts (those are plan-53-C and plan-53-D).

References:

- plan-53-A (identity: no two different parts share a uuid)
- `schematical/schematical/Backend/PartCatalog.swift` — `CatalogPart.drawer`,
  `PartCatalogStore` (`entries`, `partsByName`, `partsByUUID`,
  `registeredTexts`, `register`, `groups(matching:)`, `shared`)
- `docs/parts-format.md` § Single part files
- `schematical/schematical/Document/SchematicFileController.swift` —
  `PartFileLoading` (the part file type and `PartFileReader`)

## Prerequisites

Stated in plan-53-A; this sub-plan additionally needs plan-53-A complete.

| Must be true | Command | Status |
|---|---|---|
| plan-53-A is complete and archived | `ls planning/completed/plan-53-A-*.md` → one file | NOT MET at writing (the plan is unbuilt) |
| `PartCatalogStore.register` no longer throws for a uuid clash | `grep -c 'uuidInUse' schematical/schematical/Backend/PartCatalog.swift` → `0` | NOT MET at writing |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue, and report all of them if you stop.

## 1. Goal

- A part added to the User Library survives a relaunch: it is registered at
  launch and listed under **User Library**, with its function sub-group
  (Gates, Memory, …) kept below it.
- Every other registered custom part is listed under **File Library**.
- Removing a part from the User Library deletes its file and takes it out of
  the picker at once.
- A library file that cannot be loaded is reported, not fatal.

### Non-goals (explicit constraints)

- **No UI** in this sub-plan beyond what the picker already does with a new
  category name: no alert, window, menu item or context-menu item.
- **A part file's text is never changed by being listed.** The category is a
  property of the catalog's copy, not of the file.
- **Bundled parts, their order and their categories are unchanged.**
- **Only `PartCatalogStore.shared` reads the folder.** A catalog made with
  `PartCatalogStore()` has no library until one is attached (tests).
- **The backend does not change.** It cannot unload an arbitrary part
  (`part_library_unload_last_part_file` takes back only the latest), so a
  removed part stays registered in the session and is only hidden.

## 2. Current State

- `CatalogPart.drawer` is `let drawer: [String]` (`PartCatalog.swift:501`). The
  Part Editor's parts have `["Custom", <group>]` (`PartEditorModel.swift:144`).
- `PartCatalogStore.register` (`PartCatalog.swift:1725`) loads the file, then
  `partsByName[name]`, `partsByUUID[uuid]`, `registeredTexts[uuid]`, and appends
  to `entries` when `drawer` is not empty, dropping `derivedCache`.
- The picker groups by `drawer[0]`, sorted by `localizedStandardCompare`
  (`PartCatalog.swift:1836`); `categories`, `kinds(inCategory:)`, the filter menus
  and the search text all come from `entries` (`Derived`, `PartCatalog.swift:1500–1545`).
- `PartCatalogStore.shared` is created once at launch (`schematicalApp.swift:39`):
  `PartCatalogStore()` then `load()` (`PartCatalog.swift:1411`).
- The app is sandboxed (`ENABLE_APP_SANDBOX = YES`, `schematical.xcodeproj/project.pbxproj:341`),
  so the Application Support folder is inside the app's container.
- No code uses Application Support today
  (`grep -rn 'applicationSupportDirectory' schematical/schematical --include='*.swift' | wc -l` → 0).
- Tests build their own `PartCatalogStore()` and call `load()`
  (`TestSupport.catalog`, `PartRegistrationTests`, `SchematicFileV5Tests`); 6 test
  files use `PartCatalogStore.shared`
  (`grep -rl 'PartCatalogStore.shared' schematical/schematicalTests | wc -l` → 6;
  `AutomationTests.testActiveLowIsReported` registers custom parts into it, `:512`),
  and the test host is the app itself, so `schematicalApp.init` creates `shared`
  in every test run.

### Measured populations

| What | Count | Command |
|---|---|---|
| Source files that read `.drawer` outside `PartCatalog.swift` | 2 | `grep -rln '\.drawer\b' schematical/schematical --include='*.swift' \| grep -v Backend/PartCatalog.swift` → `Automation/AutomationRead.swift`, `Editor/PartDraft.swift` (the latter its own `drawer`, not `CatalogPart`'s) |
| Source files that use `PartCatalogStore.shared` | 9 | `grep -rl 'PartCatalogStore.shared' schematical/schematical --include='*.swift' \| wc -l` → 9 |
| Tests that assert a registered part's drawer path | 1 | `PartRegistrationTests.testValidFileRegisters` (read: it registers with `drawer: ["Custom A", "Gates"]` and checks it is listed) |

### Verified properties

- **A struct copy can carry a different drawer.** `CatalogPart` is a
  `nonisolated struct` (`PartCatalog.swift:399`) compared and hashed by name and
  uuid only (`:537–541`), so a copy with another `drawer` is the same part for
  every `Set`/`==` use.
- **Category menus follow the category automatically.** Read `Derived` and
  `kinds(inCategory:)` (`PartCatalog.swift:1500–1590`): the family, count and
  color menus are built from the parts' drawer path, so a `User Library`
  category gets its own menus with no drawer-view change.

## 3. Design Overview

Two units:

- **`UserPartLibrary`** (new, `Backend/UserPartLibrary.swift`): the folder. It
  lists, writes and deletes `.schpart` files and nothing else.
- **`PartCatalogStore`**: owns each custom part's *listing* (`user`, `file`,
  `removed`) and applies it to the catalog's copy of the part.

**Listing.** `enum PartListing { case user, file, removed }`, kept per uuid in
`listings: [String: PartListing]`. The catalog's copy of a custom part (in
`entries`, `partsByName`, `partsByUUID`) has its first drawer level replaced:
`user` → `"User Library"`, `file` → `"File Library"`; the deeper levels are the
file's own, so `["Custom", "Gates"]` lists as `["User Library", "Gates"]` and a
one-level `["Custom"]` as `["User Library"]`. `removed` parts are in
`partsByName` and `partsByUUID` (documents that hold them still resolve) but not
in `entries`.

**On disk.** One file per part, `<uuid>.schpart`, whose text is the part's
`registeredText` (sorted keys, its name as registered, any suffix included). The
file's `creationDate` is the "added" date. The folder is
`<Application Support>/UserLibrary`; under XCTest (`XCTestConfigurationFilePath`
in the environment) it is a fresh folder in `NSTemporaryDirectory()`, so test
runs never read or write the owner's library.

**Startup.** `shared` creates the catalog, `load()`s the bundled parts, then
attaches the folder: every `*.schpart` in file-name order is read; a file whose
uuid an earlier library file already claimed, or which the loader refuses, is
recorded as a problem (file name, reason) and skipped; the rest are registered
with listing `user`.

**Correctness risk** concentrates in keeping the three lookups and `entries` in
step when a part changes listing; one private method (`setListing`) does it,
and every public change goes through it.

**Gate class.** Behavior is new, so tests are the gate, not byte-identity.
Bundled part order and the golden catalog are expected not to change.

Rejected alternatives:

- **One JSON file for the whole library** — a bad entry would take the rest with
  it, and removal would rewrite everything.
- **Keep `Custom` as the category and add a flag** — the owner asked for a
  `User Library` category; deriving it from the listing keeps files untouched.
- **Unload a removed part from the backend** — not possible for an arbitrary
  part (`docs/backend.md`, `part_library_unload_last_part_file`).

## 4. Detailed Design

### 4.1 `CatalogPart`

`drawer` becomes `private(set) var drawer`. Add
`func listed(in category: String) -> CatalogPart`: a copy whose first drawer
level is `category` (unchanged when `drawer` is empty).

### 4.2 `UserPartLibrary`

```swift
struct UserPartLibraryFile { let url: URL; let uuid: String; let text: String; let added: Date }
struct UserPartLibraryProblem { let url: URL; let reason: String }
final class UserPartLibrary {
    let directory: URL
    init(directory: URL)
    static var defaultDirectory: URL        // § 3
    func files() -> (files: [UserPartLibraryFile], problems: [UserPartLibraryProblem])
    func write(uuid: String, text: String) throws   // creates the folder; atomic
    func remove(uuid: String) throws                // missing file is not an error
    func removeFile(at url: URL) throws             // for a problem entry
}
```

`files()` reads the folder in file-name order, only `.schpart` files; an
unreadable folder returns nothing; a file that is not UTF-8 or has no `uuid`
is a problem.

### 4.3 `PartCatalogStore`

- `private(set) var userLibrary: UserPartLibrary?`,
  `private(set) var userLibraryProblems: [UserPartLibraryProblem]`.
- `func attachUserLibrary(_ library: UserPartLibrary)`: stores it, registers its
  files (§ 3 *Startup*) with listing `user`, fills the problems.
- `register(fileText:source:listing: PartListing = .file)`: sets the new part's
  listing. If the uuid was already registered with the same content it returns
  that part and, when its listing is `removed`, makes it `file`; a `user` or
  `file` listing is kept.
- `func addToUserLibrary(fileText:source:) throws -> CatalogPart`: `register(…, listing: .user)`
  (an already-registered `file` part becomes `user`), then writes
  `registeredText` to the library. A failed write leaves the part listed
  `file` and throws `PartRegistrationError.libraryWrite(reason)` (new case).
  With no library attached it throws `PartRegistrationError.noUserLibrary`.
- `func addToUserLibrary(_ part: CatalogPart) throws`: the same for a part that
  is already registered.
- `func removeFromUserLibrary(uuid: String) throws`: deletes the file, listing
  `removed`.
- `func listing(of part: CatalogPart) -> PartListing?`: nil for a bundled part.
- `var userLibraryItems: [UserLibraryItem]` (`part`, `added`, `url`), in name
  order, for plan-53-D's window.
- `private func setListing(_:for:)`: replaces the part's copy in `entries`,
  `partsByName` and `partsByUUID` (for `removed`: drops it from `entries` only),
  and sets `derivedCache = nil`.
- `shared`: after `load()`, `attachUserLibrary(UserPartLibrary(directory: UserPartLibrary.defaultDirectory))`.

### 4.4 Names

A library file holds the name as registered, so a launch registers a name that
is already unique. If a newer bundled part takes that name, the usual suffix
rule applies and the file is left as it is.

## Compatibility / Format Impact

- **Part files:** unchanged. A new folder of them is created on first add.
- **`.schematical`:** unchanged.
- **Picker:** the `Custom` category no longer appears for a custom part; it
  lists under `User Library` or `File Library`.
- **Automation:** `search_parts` / `get_part` print the new category path
  (`AutomationRead.swift:347–369`); nothing else changes.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` partial; moot tasks stay, struck through with evidence;
> fill each `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — The folder: `UserPartLibrary`

Safe alone: a new type nothing calls yet.

- [ ] `Backend/UserPartLibrary.swift`: the type of § 4.2, `defaultDirectory` per § 3.
- [ ] `schematicalTests/UserPartLibraryTests.swift` (new files under `schematical/` and `schematicalTests/` need no project edit: the project uses file-system-synchronized groups, `grep -c PBXFileSystemSynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj` → 5, and `PartCatalog.swift` has no line of its own in it): `UserPartLibrary.defaultDirectory` is under `NSTemporaryDirectory()` when run under XCTest; then, in a temp folder — `write` then `files()` returns the file with the text and a date; `remove` deletes it and a second `remove` does not throw; a non-`.schpart` file is ignored; a file with no uuid and a non-UTF-8 file are problems; a missing folder gives no files and no problems; `write` creates a missing folder.

Acceptance: the folder type behaves as listed.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserPartLibraryTests` → `** TEST SUCCEEDED **` (est. 4–8 min).
Commit: —

### Phase 2 — Listings and categories in the catalog

- [ ] `PartCatalog.swift`: `drawer` becomes `private(set) var`; `listed(in:)` (§ 4.1).
- [ ] `PartCatalog.swift`: `PartListing`, `listings`, `setListing`, `register(…listing:)`, `listing(of:)`, `PartRegistrationError.libraryWrite` and `.noUserLibrary` with descriptions (§ 4.3). Every custom part registered without an argument is listed `file`.
- [ ] `schematicalTests/PartRegistrationTests.swift`: `testValidFileRegisters` expects the category `File Library` and the sub-group from the file (`["File Library", "Gates"]`); add: a one-level drawer lists directly under `File Library`; a `removed` part is not in `groups(matching:)` but `part(named:)` and `part(uuid:)` still return it; registering the same content again relists a `removed` part as `file`.

Acceptance: a registered custom part lists under `File Library` with its sub-group, and `removed` hides it from the picker only.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PartRegistrationTests` → `** TEST SUCCEEDED **` (est. 5–10 min).
Commit: —

### Phase 3 — Adding, removing, and loading at launch

- [ ] `PartCatalog.swift`: `userLibrary`, `userLibraryProblems`, `attachUserLibrary`, both `addToUserLibrary`, `removeFromUserLibrary`, `userLibraryItems` (§ 4.3); `shared` attaches `UserPartLibrary.defaultDirectory`.
- [ ] `schematicalTests/UserLibraryCatalogTests.swift` (new): with a temp folder and a fresh `PartCatalogStore()` + `load()` + `attachUserLibrary` — (a) `addToUserLibrary` lists the part under `User Library` with its sub-group, writes `<uuid>.schpart` holding `registeredText`; (b) a second store attached to the same folder lists the same part (relaunch), with the same name and uuid; (c) `removeFromUserLibrary` deletes the file and hides the part, and a third store attached to the folder lacks it; (d) a `file` part passed to `addToUserLibrary(_:)` becomes `user`; (e) a name clash with a bundled part is suffixed once and the suffixed name is what the file holds; (f) a corrupt file and a duplicate-uuid file are in `userLibraryProblems` and do not stop the others; (g) with no library attached, `addToUserLibrary` throws `noUserLibrary`; (h) a failed write (a library folder that is a regular file) throws `libraryWrite` and the part stays `file`; (i) the `User Library` category has its kinds in `kinds(inCategory:)`.
- [ ] Docs: `docs/parts-format.md` § Single part files: a paragraph "The User Library" (the folder, one file per part named by uuid, the text as registered, the two picker categories, removal hides until the next launch). `docs/backend.md` needs no change.

Acceptance: a part added to the library is back after a relaunch of the catalog, and removing it deletes it.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/UserLibraryCatalogTests -only-testing:schematicalTests/PartRegistrationTests -only-testing:schematicalTests/UserPartLibraryTests` → `** TEST SUCCEEDED **` (est. 6–12 min).
Commit: —

## Validation Plan

- Tests: as in the phases, with the negative cases (corrupt file, duplicate
  uuid, failed write, no library).
- Coverage check: `shared` is the only caller of the folder at launch; the XCTest
  folder choice is exercised because every test run creates `shared`
  (`schematicalApp.init`); `UserPartLibraryTests` asserts
  `UserPartLibrary.defaultDirectory` is under `NSTemporaryDirectory()` when run
  under XCTest, so a run can never touch the owner's library.
- Runtime proof: none in this sub-plan (no UI to add a part); plan-53-D runs
  the app.
- Doc sync: `docs/parts-format.md` (Phase 3).
- Final gate (run ONCE, after Phase 3): the three test files above plus
  `-only-testing:schematicalTests/DrawerPerformanceTests` (the catalog's
  `derived` build changed) and `-only-testing:schematicalTests/AutomationTests`
  → `** TEST SUCCEEDED **` (est. 10–15 min).

## Decisions

- A User Library category and a File Library category in the picker — decided by the owner, 2026-11-21.
- File-only parts are listed under File Library, in the one catalog shared by every window — decided by the owner, 2026-11-21 (it accepted the global list).
- Library parts keep their function sub-groups below `User Library` — accepted by the owner, 2026-11-21.
- One `.schpart` file per part, named by uuid, in the app's Application Support — author's call: reuses the single-part format and makes removal a file delete.
- Removal hides the part for the rest of the session and the file is gone at the next launch — author's call, forced by the backend's single-level unload.

## Corrections

(Filled in during execution.)

## Summary

The risk is catalog state that falls out of step: one `setListing` keeps
`entries` and the two lookups together. Untouched: the backend, part files,
the `.schematical` format, bundled parts and their order.
