# plan-46-A: Part Wizard — inline datasheets in a single part file

Last updated: 2026-11-16
Overall Effort: huge (> 3d) — plan-46 is A → B → C → D → E → F → G → H → I → J → K → L → M → N
Effort: medium (1h–2h)
Depends on: nothing

## plan-46 at a glance

plan-46 turns File › New Part… into a **Part Wizard**: a guided walk
through making a part, for someone who has a datasheet and no idea what a
`KindSchema` is. Every screen says what it wants, where a datasheet usually
prints it (the common section titles and their other names) and what a
typical value looks like. The full form stays one click away
(**Guided | All fields**), so nothing is ever a dead end.

The behavioral outcome of the whole plan: a user who has never seen the
part format picks File › New Part…, answers the wizard's screens for a
74HC00-style gate (kind, part number, datasheet URL, a pasted pin table,
the gates, a footprint) and ends on a Review screen whose block and
footprint previews show the part; Save Part… writes a `.schpart` that
File › Load Part… loads, and its Open Datasheet button opens the URL they
pasted. A new kind added to the backend gets a working wizard with no
wizard code: its generated schema is enough.

The wizard's screens, in order:

1. **What are you making?** — the kind, in plain words, grouped, with
   example parts; or "Just a symbol and footprint (no simulation)" (the
   `generic` kind, B). A catalog search offers "Start from a similar part".
2. **Part number** — part number, manufacturer, package. Device id,
   drawer path and function are derived (and editable under More options).
3. **Datasheet** — an `https://` URL; the PDF opens in a pane beside the
   wizard and stays there for the rest of the steps, with **Find in
   datasheet** buttons per field.
4. **Pins** — paste the datasheet's pin table, map its columns, review the
   guessed roles and active-low flags.
5. **Kind steps** — the kind's own screens: a "Which chip does yours behave
   like?" model picker for kinds with variants, then Essentials, then one
   optional (skippable) screen per remaining group of keys. Curated kinds
   (L, M) have hand-written screens; every other kind's screens are derived
   from its schema (D).
6. **Footprint** — thumbnails of matching bundled footprints, or a
   parametric footprint (dual / quad / grid / header) from a few
   numbers with a labeled diagram, or the existing pad table.
7. **Review** — block and footprint previews, a pin ↔ pad table, the
   backend's verdict pointed at the step that has the problem; Save Part…
   and Add to Library.

Sub-plans (letter order is implementation order):

| Letter | What | Effort |
|---|---|---|
| A | Backend + draft: inline `"datasheets"` in a `.schpart` | medium |
| B | Backend + app: the `generic` kind (facts only, no simulation) | medium |
| C | Editor schemas for resistor, capacitor, transistor, mcu, generic | medium |
| D | `KindInfo` registry and the wizard step model (derived steps, conditions, pin renames) | medium |
| E | Wizard window shell, Guided \| All fields, section views shared with the form | large |
| F | Steps 1–2: kind picker, similar-part start, part number | medium |
| G | Step 3: datasheet URL, PDF pane, Find in datasheet | medium |
| H | Step 4: pin-table paste, column mapping, role and active-low guesses | medium |
| I | Step 5: model picker, derived kind pages, bus-style conditions | medium |
| J | Step 6: footprint thumbnails, parametric footprints | large |
| K | Step 7: review, pin ↔ pad table, verdict → step | medium |
| L | Guidance framework, shared guidance, guide drift test | medium |
| M | Curated guides for 13 simple kinds, blurbs for 60 models | large |
| N | Landing: help, docs, runtime proof, final gate | medium |

## This sub-plan

A user-pasted datasheet URL has nowhere to go today: a part's
`"datasheet"` must be the id of an entry in the bundled `datasheets.json`
(`backend/src/library/library.c:669–675`, `unknown datasheet "X"`). A adds
an optional top-level `"datasheets"` array to the single part file, exactly
like its inline `"footprints"` (plan-32-D): each entry is a
`datasheets.json` entry `{"id", "url"}`, its id new to the library, and the
part's `"datasheet"` may name it. The draft keeps the array as a typed
field. No UI yet (G).

Behavioral outcome: a `.schpart` with `"datasheets": [{"id":
"CUSTOM-6F1C2E8A", "url": "https://…/x.pdf"}]` and `"datasheet":
"CUSTOM-6F1C2E8A"` loads, `part->datasheet->url` is that URL, and the
Part Editor's validator can check the same draft any number of times in a
row (the inline entry is taken back with the part).

References:

- `docs/parts-format.md` § Single part files (the `.schpart` key table),
  § Datasheets (the entry rules), § Custom footprints in a part file (the
  precedent).
- `backend/src/library/library.c`: `PART_FILE_KEYS` (:1317),
  `read_part_file` (:1344), `load_part_file` (:1424),
  `part_library_unload_last_part_file` (:1476), `commit_parts` (:1036),
  `read_datasheets` (:1137, the entry checks), `DATASHEET_KEYS` (:1134),
  `part_library_find_datasheet` (:228), `part_library_find_footprint`
  (:198, the staged-lookup precedent), the part's datasheet check (:644–675).
- `backend/tests/part_file_test.c` (the `.schpart` cases).
- `schematical/schematical/Editor/PartDraft.swift` (`footprints`,
  `fileExtras` :241–243, `encoded` :409–411),
  `schematical/schematical/Editor/PartValidator.swift` (one scratch library
  validates every draft: load, copy out, unload).

## Prerequisites

These are a precondition on the whole of plan-46; every other letter points
here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p46 && cmake --build /tmp/logic-p46 -j8 && ctest --test-dir /tmp/logic-p46 -j8` → `100% tests passed` | MET (2026-11-16 at 4218bb74, re-run at plan start: `100% tests passed, 0 tests failed out of 36`; T0 = 36, `grep -c '^logic_test' backend/CMakeLists.txt` → 36) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p46-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-16 at 4218bb74, re-run at plan start: `** BUILD SUCCEEDED **`) |
| No inline datasheets yet | `grep -c '"datasheets"' backend/src/library/library.c` → 3 (`DATASHEET_FILE_KEYS` :1133, `read_datasheets`' member :1160, `load_datasheets`' source name :1235; none in `PART_FILE_KEYS`) | MET (2026-11-16 at 4218bb74, re-run at plan start: 3) |
| No `generic` kind yet | `grep -c 'PART_GENERIC' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/generic.json` → no such file | MET (2026-11-16 at 4218bb74, re-run at plan start: 0, `No such file or directory`) |
| The editor's save/open tests are green (added while executing, § Corrections 1) | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartFileSaveOpenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` | MET (2026-11-16: NOT MET at 4218bb74 — `testAddToLibrary` aborted the host; fixed in this plan, § Corrections 1; then `Executed 6 tests, with 0 failures`, `** TEST SUCCEEDED **`) |
| No wizard yet | `ls schematical/schematical/Editor/ \| grep -ci wizard` → 0 | MET (2026-11-16 at 4218bb74, re-run at plan start: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of every row.

## 1. Goal

- `load_part_file` accepts an optional top-level `"datasheets"` array; the
  part's `"datasheet"` resolves against it as well as against the library.
- After `part_library_unload_last_part_file`, the inline entries are gone:
  the same text loads again with no `duplicate datasheet id`.
- `PartDraft` reads and writes the array as `datasheets: [DatasheetDraft]`
  (not as a `fileExtras` key), and `setDatasheetURL(_:)` makes the one inline
  entry `CUSTOM-<first 8 of uuid>` and points the part at it.

### Non-goals (explicit constraints)

- **`datasheets.json` and every part file are unchanged.** Goldens
  (`parts.txt` 32897, `traces.txt` 32897, `footprints.txt` 1646 lines at
  5bccd0d7) stay byte-identical.
- **No `"md"` on an inline entry**: an `md` path names a file bundled with
  the app; a custom part has none.
- **No UI** in A (the field is G's).
- **No change to `version`** of the `.schpart` format: the key is optional;
  every existing file loads as before.

## 2. Current State

- `PART_FILE_KEYS` = format, version, uuid, kind, footprints, devices
  (`library.c:1317–1319`); any other top-level key is rejected by
  `kind_check_keys`.
- Inline footprints are read by `footprint_read_entries` into a
  `FootprintList` (`read_part_file`), **staged** on the library while the
  device loads (`library->staged_footprints`, `library.c:1410–1412`) so
  `part_library_find_footprint` sees them (`:205–213`), then adopted by
  `commit_parts` (`footprint_table_build`/`footprint_table_adopt`).
  `part_library_unload_last_part_file` restores `footprint_count` to
  `last_footprint_count` and rebuilds `footprint_ids` (`:1476–1516`).
- Datasheets: the library holds `datasheets`, `datasheet_count`,
  `datasheet_ids` (`library.c:123–125`); `read_datasheets` checks each entry
  (keys id/url/md, non-empty id, `https://` URL, relative `.md`, duplicate
  ids; `:1137–1215`). `part_library_find_datasheet` looks only in
  `datasheet_ids` (`:228–232`).
- The part's `"datasheet"` needs a `"manufacturer"` and is refused on a
  `"Generic"` part (`:660–668`).
- The app copies the URL out at load (`PartCatalog.swift:762`,
  `datasheetURL = part.datasheet.flatMap { URL(string: …url) }`), so an
  inline entry's URL survives as long as the part's arena does (the arena is
  adopted by `commit_parts`, `adopt_arena`).
- `PartDraft` keeps unknown top-level keys in `fileExtras`
  (`PartDraft.swift:242`), so a `"datasheets"` key would round-trip
  untyped today; the validator would then fail it at `kind_check_keys`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `.schpart` top-level keys | 6 | `sed -n 1317,1319p backend/src/library/library.c` → format, version, uuid, kind, footprints, devices |
| Bundled datasheet entries (ids an inline id must not take) | 7993 | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"` → 7993 |
| Bundled ids starting `CUSTOM-` | 0 | `grep -c '"id": "CUSTOM-' schematical/schematical/Parts/datasheets.json` → 0 |

### Verified properties

- **The validator reuses one library for every draft.** Read
  `PartValidator.validate(text:)`: `load_part_file`, copy the part out,
  `defer { part_library_unload_last_part_file }`. So an inline datasheet that
  the unload does not take back makes the second check of the same draft
  fail with `duplicate datasheet id` — this is A's correctness risk (§ 3).
- **An empty `"devices"` array is not A's concern**; A touches only the
  top-level keys and the datasheet lookup (read `read_part_file`).

## 3. Design

- `PART_FILE_KEYS` gains `"datasheets"`.
- `read_datasheets`' per-entry loop body moves into
  `static int read_datasheet_entry(KindContext *, const cJSON *item,
  const PartLibrary *, NameSet *ids, int allow_md, PartDatasheet **out)`;
  `read_datasheets` calls it with `allow_md = 1`, unchanged behavior.
- `read_part_file`: when `"datasheets"` is present it must be an array
  (`"datasheets" must be an array`); each entry goes through
  `read_datasheet_entry(…, allow_md = 0, …)` (an `md` key fails
  `datasheets[0]: an inline datasheet has no "md"`) into a new
  `DatasheetList` (`items`, `count`). Ids must be new to the library and to
  the file (the existing `duplicate datasheet id "X"`).
- Staging: `PartLibrary` gains `const DatasheetList *staged_datasheets`;
  `part_library_find_datasheet` falls back to it exactly as
  `part_library_find_footprint` falls back to `staged_footprints`. Set
  around `load_device` next to `staged_footprints`, cleared on every exit.
- Unused entries are refused: after the device loads, an inline entry the
  part does not name fails `datasheet "X" is not used` (the bundled
  `footprint_check` keeps the same rule for footprints; a file never carries
  dead entries).
- `commit_parts` takes the `DatasheetList` and appends its entries to
  `library->datasheets` / `datasheet_ids` (grow the array like `parts`).
  `load_part_file` records `library->last_datasheet_count` beside
  `last_footprint_count`; `part_library_unload_last_part_file` restores
  `datasheet_count` and rebuilds `datasheet_ids` from what stays, exactly as
  it does for footprints.
- Draft: `struct DatasheetDraft: Equatable, Sendable { var id: String; var
  url: String }`; `PartDraft.datasheets: [DatasheetDraft]?` read from and
  written to the top-level key (removed from `fileExtras`'s filter list at
  `PartDraft.swift:242`). `mutating func setDatasheetURL(_ url: String?)`:
  nil or empty removes the inline entry and clears `datasheet` if it named
  it; otherwise `datasheets = [DatasheetDraft(id: "CUSTOM-" +
  uuid.prefix(8), url: url)]` and `datasheet = that id`. `var
  datasheetURL: String?` reads the inline entry the part names.

Correctness risk: the unload (validator reuse, Add to Library's
replace-on-reregister). It is covered by the "load, unload, load again"
case in Phase 1.

Byte-identity is a gate here only for the goldens (no part file changes);
the feature itself is gated by `part_file_test`.

### Rejected alternatives

- **A `datasheetUrl` part key** — a second way to name a datasheet that the
  inspector, the listing and the BOM would each learn; the inline array
  reuses the one lookup everything already uses (owner decision, § Decisions).
- **Keeping inline entries after unload** — breaks the validator's reuse
  (§ 2 Verified properties).
- **Allowing unused inline entries** — the wizard always names its entry;
  allowing dead ones only lets hand-edited files rot.

## Compatibility / Format Impact

`.schpart` gains one optional top-level key; `version` stays 1. Every
existing `.schpart` and every schematic that embeds one loads unchanged
(the key is optional). `datasheets.json`'s format is unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work; `- [~]` for partial with one line on what
> remains; moot tasks are struck through with evidence, never deleted; fill
> each `Commit:` line when the phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Backend: read, stage, commit, unload

- [x] `backend/src/library/library.c`: `read_datasheet_entry` split out of
      `read_datasheets` (behavior unchanged: `library_check` still passes);
      `"datasheets"` in `PART_FILE_KEYS`; the `DatasheetList` read in
      `read_part_file`; `staged_datasheets` on `PartLibrary` and in
      `part_library_find_datasheet`; the unused-entry check;
      `commit_parts` adopts the list; `last_datasheet_count` recorded and
      restored by `part_library_unload_last_part_file`.
      (`ctest -R library_check` passed; the inline reader is
      `read_inline_datasheets`.)
- [x] `backend/tests/part_file_test.c` cases: (a) a gate part with one
      inline datasheet loads and `part->datasheet->url` is the URL; (b) the
      same text: load, `part_library_unload_last_part_file`, load again →
      0 both times, and `part_library_datasheet_count` is back to the bundled
      count after the unload; (c) an inline id equal to a bundled id →
      `duplicate datasheet id "sn74hc00"`; (d) an `http://` URL → `"url"
      must start with https://`; (e) an `md` key → `an inline datasheet has
      no "md"`; (f) an entry the part does not name → `datasheet
      "CUSTOM-X" is not used`; (g) `"datasheets": {}` → `"datasheets" must
      be an array`; (h) a `"Generic"` manufacturer naming the inline entry →
      the existing `a Generic part has no datasheet`.
      (Labelled `(ds-a)`…`(ds-h)` in the test, since `(a)`…`(j)` were taken:
      `/tmp/logic-p46/part_file_test | grep ds-` → 8 `ok (ds-…)` lines with
      the messages above; `ctest -R 'part_file_test|library_check|listing_check|footprint_check'`
      → `100% tests passed, 0 tests failed out of 4`; `git diff --stat backend/tests/golden` → empty.)

Acceptance: the eight cases pass and the bundled library is unchanged.
  Check: `cmake --build /tmp/logic-p46 -j8 && ctest --test-dir /tmp/logic-p46 -R 'part_file_test|library_check|listing_check|footprint_check'` → `100% tests passed, 0 tests failed out of 4`; `git diff --stat backend/tests/golden` → empty (est. 4 min).
Commit: c7984f75

### Phase 2 — The draft keeps the array

- [x] `schematical/schematical/Editor/PartDraft.swift`: `DatasheetDraft`,
      `datasheets`, reading and writing the key, `"datasheets"` added to the
      top-level keys excluded from `fileExtras`, `setDatasheetURL(_:)`,
      `datasheetURL`.
- [x] `schematical/schematicalTests/PartFileSaveOpenTests.swift`:
      `testInlineDatasheetRoundTrips` (a draft with `setDatasheetURL`
      encodes the key, reads back equal, `extraKeys` empty) and
      `testInlineDatasheetValidatesTwice` (`PartValidator.shared.validate`
      the same draft twice → `.success` both times; the second result's
      `datasheetURL` is the URL).
      (`xcodebuild … test -only-testing:schematicalTests/PartFileSaveOpenTests`
      → `Executed 6 tests, with 0 failures`, `** TEST SUCCEEDED **`; both new
      tests `passed`.)

Acceptance: the two tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartFileSaveOpenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 12 min: the first app test build; the draft is app code).
Commit: c9db9714 (the prerequisite fix of § Corrections 1: 7482e9dc)

### Phase 3 — Docs

- [x] `docs/parts-format.md` § Single part files: the example gains a
      `"datasheets"` line; the key table gains the `"datasheets"` row
      (optional array of `{"id", "url"}`, ids new to the library, no `md`,
      every entry named by the part); § Datasheets gains one sentence
      pointing at it.
      (`grep -c '"datasheets"' docs/parts-format.md` → 5. The example's part
      is now a Texas Instruments part naming the entry: a `"Generic"` part
      has no datasheet.)

Acceptance: the doc names the key and its rules.
  Check: `grep -c '"datasheets"' docs/parts-format.md` → at least 3 (1 today, the `datasheets.json` example; plus the `.schpart` example line and the table row) (est. 1 min).
Commit: 1ccd9f90

## Validation Plan

- Tests: `part_file_test` (8 new cases, error text included),
  `PartFileSaveOpenTests` (2 new).
- Coverage: case (b) runs the unload path with an inline entry — the path
  the validator takes on every edit.
- Runtime proof: none in A (no UI); G's runtime check opens the PDF from a
  saved part's Open Datasheet button.
- Doc sync: `docs/parts-format.md` (Phase 3).
- Final gate: plan-46-N runs it once for the whole plan.

## Decisions

- **Inline `"datasheets"` array in `.schpart`** (mirrors inline
  `"footprints"`), not a new part key and not "URL only in the editor" —
  decided by the owner, 2026-11-16. Custom parts get a real Open Datasheet.
- **`generic` kind for symbol-and-footprint-only parts** — owner,
  2026-11-16 (sub-plan B).
- **The wizard is for New Part only**; Open Part… and Copy to Editor… open
  the full form; every window has Guided | All fields — owner, 2026-11-16
  (sub-plan E).
- **Curated guides for the simple kinds first** (13 kinds: gate, buffer,
  dff, jkff, latch, mux, decoder, led, diode, switch, oscillator, header,
  7seg) plus shared guidance and model blurbs for every variant kind;
  complex kinds use derived steps in this plan — owner, 2026-11-16 (L, M).
- **The four formless kinds (resistor, capacitor, transistor, mcu) get
  editor schemas in this plan** — owner, 2026-11-16 (sub-plan C).
- **plan-46 lands before plan-44 and plan-42**, and plan-46's commit edits
  them (their kind-list steps move to the `KindInfo` registry and a
  generated schema; their new `PartKind` goes after `PART_GENERIC`) —
  owner, 2026-11-16 (for plan-44; plan-42 has the same two touch points and
  is edited the same way).

## Corrections

1. **A prerequisite no row covered: `PartFileSaveOpenTests` was not green at
   the base (4218bb74).** Phase 2's check ran `testAddToLibrary` into
   `malloc: *** error for object 0x292bd8b30: pointer being freed was not
   allocated` and `Test crashed with signal abrt` — with this plan's
   `library.c` and with the base's (`git show 4218bb74:backend/src/library/library.c`
   put in place for one run: the same abort), so not A's change. A debugger
   attached to the test host (`process attach --name schematical --waitfor`,
   a breakpoint on `malloc_error_break`) gave the stack:
   `PartCatalogStore.register` → `PartCatalogStore.Derived.__deallocating_deinit`
   → `swift_task_deinitOnExecutorImpl` → `TaskLocal::StopLookupScope::~StopLookupScope`
   → `___BUG_IN_CLIENT_OF_LIBMALLOC_POINTER_BEING_FREED_WAS_NOT_ALLOCATED`.
   `Derived` (the drawer's fact cache, a180f7c3) is a class nested in the
   main-actor `PartCatalogStore`, so it had an isolated deinit, and
   `register` frees it (`derivedCache = nil`) inside the test: the Swift
   runtime abort `TestSupport.newDocument()` already documents for
   documents. Fix: `Derived` is `nonisolated` (it is plain data). Added as a
   Prerequisites row with its check; landed as its own commit before
   Phase 2. Every later letter's app check registers parts, so they all
   rested on it.
2. Phase 1's test cases are labelled `(ds-a)`…`(ds-h)`: `part_file_test`
   already had cases `(a)`…`(j)`.
3. § 3's `read_part_file` bullet is implemented as a helper,
   `read_inline_datasheets`, called from `read_part_file` (the same
   behavior and messages).

## Summary

The risk is the unload: an inline entry that outlives its part breaks the
validator's one-library reuse. Everything bundled is untouched.
