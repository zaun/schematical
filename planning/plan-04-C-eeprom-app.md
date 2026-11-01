# plan-04-C: EEPROMs in the app — load file on start

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-04-B (`circuit_memory_load`, `memory_image_parse`)

plan-04 adds Microchip parallel EEPROMs. A and B made them a backend part
kind with a load API. This sub-plan puts them in the app: they appear in
the drawer under Memory → EEPROM, open a memory window on double-click
like SRAM, and each placed EEPROM has a **Load file on start** setting in
its memory window. The setting can be changed only while the simulation is
stopped; while stopped, the window previews the contents the chip will
start with. Starting the simulation loads the file (binary or Intel HEX)
into the chip; with no file the chip starts erased (all FF). Writes made
during a run are gone after Stop: every start loads the file again. The
chosen file is saved in the `.schematical` file as a path relative to the
schematic plus an app-scoped security bookmark, so it still works after the
app restarts.

Outcome of C: on a schematic with an `AT28C256-15PU`, choosing
`boot.bin` (stopped) shows its bytes in the memory window; Run shows the
same bytes live and the chip's data pins read them; Save → quit → relaunch
→ Open → Run loads `boot.bin` again, with the app sandboxed.

References:

- `docs/file-format.md` (the `.schematical` format, § Changing the format),
  `docs/backend.md` (`circuit_memory_load`, `memory_image.h`).
- `schematical/schematical/Memory/MemoryWindow.swift` (`MemoryInspector`),
  `Simulation/Simulation.swift` (`SimulationController.start`, `build`),
  `Document/SchematicFile.swift`, `Document/SchematicFileController.swift`,
  `Canvas/SchematicModel.swift` (`PlacedChip`, `SchematicDocument`),
  `Canvas/SchematicCanvasView.swift` (double-click), `Backend/PartCatalog.swift`
  (`CatalogPart.Details`).
- plan-04-A §Decisions (all owner decisions for plan-04).
- Headless harness approach: `planning/completed/plan-03-A-footprint-format.md`
  § Corrections and `planning/completed/plan-03-D-pcb-tab.md` § Runtime proof.

## Prerequisites

See plan-04-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-04-B complete | `ls planning/completed/plan-04-B-memory-load.md` → exists | MET (2026-11-01: archived after B's final gate, 41f140c) |
| Load API present | `grep -n '^int circuit_memory_load\|^int memory_image_parse' backend/include/*.h` → one line each (anchored: the header comments mention both too) | MET (2026-11-01: `circuit.h:65`, `memory_image.h:29`) |
| App builds signed and sandboxed | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p04-signed-dd build` → `** BUILD SUCCEEDED **`; `codesign -d --entitlements - /tmp/p04-signed-dd/Build/Products/Debug/schematical.app` lists `com.apple.security.app-sandbox` and `com.apple.security.files.user-selected.read-write` | MET (2026-11-01 at start of work: `** BUILD SUCCEEDED **`; entitlements list app-sandbox and files.user-selected.read-write) |
| The desktop session is unlocked (sandbox proof, Validation step 6, needs a real window and open panel) | `ioreg -n Root -d1 \| grep -o '"CGSSessionScreenIsLocked"=[A-Za-z]*'` → `"CGSSessionScreenIsLocked"=No` (or no output) | MET at plan start (2026-11-01: no output); NOT MET when C began and again at the final gate (2026-11-01: `"CGSSessionScreenIsLocked"=Yes`). Needed only for Validation 6 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all*
> prerequisites.

## 1. Goal

- Drawer: Memory → EEPROM lists the plan-04-A EEPROM parts with
  `"<n> ns"` labels, like SRAM.
- Double-clicking a placed EEPROM opens its memory window, as SRAM.
- EEPROM memory window, stopped: a "Load file on start" row (file name or
  "None — starts erased (FF)", Choose…, Clear) and a grid with the contents
  the chip will start with. Running: the row is disabled (help: "Stop the
  simulation to change the load file") and the grid shows live contents.
- Run with a file loads it (`circuit_memory_load` before `circuit_start`);
  an unreadable or invalid file stops Run with
  `"U3: <message>"` (the §4.4 error text, e.g. `U3: boot.bin: line 3: bad checksum 0x1F (expected 0x2E)`
  or `U3: can't read boot.bin: …`) and the simulation does not start.
- Save writes `"memoryFile": {"path", "bookmark"}` on the part; Open
  restores it; a moved schematic + file pair still resolves by the relative
  path.
- SRAM windows and every existing file open and behave exactly as before.

### Non-goals (explicit constraints)

- `version` stays 2: `memoryFile` is an optional field (§ Changing the
  format). Older builds open new files and ignore it.
- No setting for SRAM; SRAM still starts all X.
- No writing back: the app never modifies the load file; run-time writes are
  lost at Stop (owner decision).
- No document-scoped bookmarks and no new entitlement (owner decision:
  app-scoped + relative path).
- No change to the PCB page; EEPROMs show their footprints there through
  plan-03 code unchanged.

## 2. Current State

- **Memory window.** `MemoryInspector` (`MemoryWindow.swift`) reads
  `simulation.memorySize(of:)`; when it is nil (stopped) the window shows
  "Simulation Not Running" with an SRAM-specific description. It reloads on
  `simulation.stateVersion` and `isRunning`. It has the `SchematicDocument`
  and `SimulationController` through `SchematicSessions`.
- **Opening it.** `SchematicCanvasView.swift` double-click:
  `chip.part.kind == PART_SRAM` → `onOpenMemory(chip.id)` (the only
  `PART_SRAM` in the app: `grep -rn PART_SRAM schematical/schematical | wc -l` → 1).
- **Catalog.** `CatalogPart.Details` has one case per kind; `.sram` is built
  from `part.sram?.pointee` (`PartCatalog.swift:269`) and labelled
  `"\(accessNs) ns"` (`:193`). `detailLabels` is the only exhaustive switch
  over `Details` (`grep -rn 'case .supply' schematical/schematical` → 1).
  The drawer groups by the part's `drawer` path, so `["Memory","EEPROM"]`
  needs no drawer code. `symbol(for:)` defaults to `.block`.
- **Model.** `PlacedChip` (`SchematicModel.swift:383`) holds per-part state
  (`isFlipped`, `switchesOn`, `board`); `SchematicDocument` mutates it by id
  (`toggleSwitch`, `moveFootprint`). No copy/paste exists
  (`grep -n -i 'paste\|duplicate' Canvas/SchematicModel.swift` → none).
- **File.** `SchematicFile(document:)` has 4 call sites, all in
  `SchematicFileController.swift` (`grep -rn 'SchematicFile(document:' schematical/schematical | wc -l` → 4);
  `apply(to:catalog:)` has 1 (`:114`). `hasUnsavedChanges` compares a
  fresh `SchematicFile(document:)` with `savedSnapshot`.
- **Simulation.** `SimulationController.start` builds the netlist, then
  `build(_:from:)` adds parts, nets, connections, switches and calls
  `circuit_start` last. An error sets `errorMessage` and leaves
  `isRunning` false.
- **Sandbox.** `ENABLE_APP_SANDBOX = YES`,
  `ENABLE_USER_SELECTED_FILES = readwrite` (`project.pbxproj`); no
  entitlements file. App-scoped security bookmarks need no further
  entitlement (`com.apple.security.files.bookmarks.app-scope` has no effect
  on current macOS, per the Apple Developer Forums thread 798402).
- **Tests.** No Swift test target. Plans 02 and 03 proved the app with a
  headless harness: the app's Swift sources (all but `schematicalApp.swift`)
  plus the backend compiled into a CLI inside a copy of the Debug `.app`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `SchematicFile(document:)` call sites | 4 | above |
| `apply(to:` call sites | 1 | `grep -rn '\.apply(to:' schematical/schematical \| wc -l` |
| Exhaustive `Details` switches | 1 | above |
| `PART_SRAM` uses in the app | 1 | above |

## 3. Design Overview

Five pieces, landed in three phases:

1. **Catalog + canvas**: `.eeprom` details, `isMemory`, double-click.
2. **Document + file**: `PlacedChip.memoryFile`, `SchematicFile.Part.memoryFile`,
   relative path + bookmark encode/resolve.
3. **File access** (`Memory/MemoryFileAccess.swift`, new): choose, read
   under security scope, parse through `memory_image_parse`.
4. **Simulation**: load every EEPROM's file before `circuit_start`.
5. **Memory window**: the setting row and the stopped preview.

Correctness risk concentrates in (2): an encoding that differs between the
saved snapshot and a fresh one would leave every schematic "unsaved", and a
path computed against the wrong folder breaks Open after Save As. It is
pinned by the round-trip proof (§Validation 4). The sandbox behavior of (3)
can only be shown in the signed build by a person choosing a file
(§Validation 6).

Rejected alternatives:

- **Document-scoped bookmarks** — owner chose app-scoped (reports of
  "Item URL disallowed by security policy"; needs an entitlement).
- **Embedding the file in the schematic** — owner chose path + bookmark.
- **Keeping the file open (security scope) for the window's life** — access
  is started and stopped around each read, so nothing leaks when a part or
  window goes away.
- **Choosing the file from the canvas inspector** — the owner asked for the
  setting in the memory window.

## 4. Detailed Design

### 4.1 Catalog and canvas

- `PartCatalog.swift`: `case eeprom(words: Int, bits: Int, accessNs: Int, grade: String, pageBytes: Int, writeCycleUs: Int)`
  from `part.eeprom?.pointee` (`PartEeprom`, plan-04-A §4.1:
  `words, bits, access_ps, grade, page_bytes, write_cycle_ps`);
  `detailLabels` → `["\(accessNs) ns"]`.
- `CatalogPart.isMemory: Bool` — `kind == PART_SRAM || kind == PART_EEPROM`;
  `CatalogPart.memorySize: MemorySize?` — from `.sram`/`.eeprom` details.
- `SchematicCanvasView.swift`: the double-click test becomes
  `chip.part.isMemory`.

### 4.2 Document (`SchematicModel.swift`)

```swift
/// An EEPROM's load file: where it is, and the app-scoped security bookmark
/// that lets the sandboxed app read it after a restart.
nonisolated struct MemoryFile: Hashable, Sendable {
    var url: URL
    var bookmark: Data?
    var name: String { url.lastPathComponent }
}
```

- `PlacedChip.memoryFile: MemoryFile?` (init parameter, default nil; kept
  only when `part.kind == PART_EEPROM`, else nil).
- `SchematicDocument.setMemoryFile(_ id: UUID, _ file: MemoryFile?)` — no-op
  for non-EEPROM chips. The window calls it only while stopped.

### 4.3 File (`SchematicFile.swift`, `SchematicFileController.swift`, `docs/file-format.md`)

```json
{ "id": "...", "reference": "U4", "partNumber": "AT28C256-15PU",
  "x": 300, "y": 120, "flipped": false,
  "pcb": { "x": 20.32, "y": 0, "rotation": 0 },
  "memoryFile": { "path": "roms/boot.bin", "bookmark": "Ym9va21hcms..." } }
```

- `SchematicFile.Part.memoryFile: MemoryFileEntry? = nil`,
  `struct MemoryFileEntry: Codable, Equatable, Sendable { var path: String; var bookmark: Data? }`
  (`Data` encodes as base64, `JSONEncoder` default).
- `init(document:fileURL:)` (`fileURL: URL?`): `path` is the file's path
  relative to `fileURL`'s folder, by path components with `..` as needed
  (`/a/b/s.schematical` + `/a/roms/x.bin` → `../roms/x.bin`); with
  `fileURL == nil` (never saved) it is the absolute path. All 4 call sites
  pass the controller's current `fileURL`; `write(to: url)` passes `url`.
- `apply(to:catalog:fileURL:)`: for an EEPROM part with `memoryFile`, the
  URL is the resolved bookmark
  (`URL(resolvingBookmarkData:options: .withSecurityScope, relativeTo: nil, bookmarkDataIsStale:)`);
  a stale bookmark is recreated from the resolved URL. If there is no
  bookmark or it fails to resolve, the URL is `path` resolved against
  `fileURL`'s folder (absolute paths as they are) and `bookmark` is nil.
  `memoryFile` on any other part is ignored. A missing or unreadable file
  never fails Open; it shows in the memory window and at Run.
- Opening calls `apply(…, fileURL: url)`; `savedSnapshot` is taken after
  apply, so a refreshed bookmark does not mark the document unsaved.
- `docs/file-format.md`: the `memoryFile` example and table row (EEPROM
  only, optional; `path` relative to the schematic's folder, or absolute
  for a schematic never saved; `bookmark` base64, app-scoped, used first on
  this Mac); `partNumber` row mentions EEPROM; § Validation on open: a
  missing load file does not fail Open.

### 4.4 File access (`Memory/MemoryFileAccess.swift`, new)

```swift
enum MemoryFileAccess {
    /// Open panel (any file); returns the file with an app-scoped bookmark,
    /// made while the panel's access is live. nil when cancelled.
    @MainActor static func choose(attachedTo window: NSWindow?) async -> MemoryFile?
    /// The file's bytes, read under its security scope.
    static func read(_ file: MemoryFile) throws -> Data
    /// The chip's start contents: the parsed file, or all 0xFF without one.
    static func image(_ file: MemoryFile?, size: MemorySize) throws -> [UInt32]
}
```

- `read`: resolve the bookmark if present (as §4.3), then
  `startAccessingSecurityScopedResource()` → `Data(contentsOf:)` →
  `stopAccessing…` (deferred). Errors become
  `MemoryFileError.unreadable(name:reason:)` →
  `"can't read \(name): \(reason)"`.
- `image`: `memory_image_parse(bytes, size, memory_image_format_for(name), words, bits, &values, &error)`;
  a parse error becomes `MemoryFileError.invalid(name:message:)` →
  `"\(name): \(message)"` with the C message (plan-04-B §4.3).
- `choose` validates nothing; the window previews the result at once, which
  shows any error.

### 4.5 Simulation (`Simulation.swift`)

- `start(document:)`: before `build`, for each chip with
  `part.kind == PART_EEPROM` and a `memoryFile`, `MemoryFileAccess.image`
  with `part.memorySize`; an error sets
  `errorMessage = "\(chip.reference): \(error.localizedDescription)"`,
  destroys the circuit, and returns (not running).
- `build(_:from:loads:)`: after switches and before `circuit_start`, for
  each `(part, values)`: `circuit_memory_load(circuit, part, 0, values, values.count)`;
  -1 fails the build like any other building call.

### 4.6 Memory window (`MemoryWindow.swift`)

- `MemoryInspector` for an EEPROM chip shows `LoadFileBar` above the page
  bar: `Text("Load file on start:")`, the file name (or
  `"None — starts erased (FF)"`), `Button("Choose…")`, `Button("Clear")`
  (Clear disabled without a file). Both buttons are disabled while
  `simulation.isRunning`, with `.help("Stop the simulation to change the load file")`.
  Choose calls `MemoryFileAccess.choose` (sheet on the memory window) and
  `document.setMemoryFile`.
- Stopped, EEPROM: `size` = `chip.part.memorySize`; `words` = the page of
  `MemoryFileAccess.image(chip.memoryFile, size:)`, computed when the window
  appears, when `chip.memoryFile` changes, and when `isRunning` turns false;
  no highlights. An error replaces the grid with
  `ContentUnavailableView("Can't Load File", systemImage: "exclamationmark.triangle", description: Text(message + " Choose the file again."))`.
  Footer: `"Contents at start · \(size.words) words × \(size.bits) bits (\(function))"`.
- Running: unchanged live path (`simulation.memorySize`, `readMemory`).
- SRAM: unchanged, including "Simulation Not Running".

## Compatibility / Format Impact

- `.schematical`: optional `memoryFile` on EEPROM parts; `version` 2. Old
  files open unchanged; new files open in older builds (field ignored,
  EEPROM parts unknown to a build before plan-04-A fail Open as unknown
  parts, as for any new part).
- No backend change in this letter.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Catalog and canvas

EEPROMs in the drawer and openable; the window shows "Simulation Not
Running" for them until Phase 3. Safe alone.

- [x] `Backend/PartCatalog.swift`: `.eeprom` details, `detailLabels`,
      `isMemory`, `memorySize` (§4.1).
- [x] `Canvas/SchematicCanvasView.swift`: double-click on `isMemory`.
- [x] Harness (`/tmp/p04/harness/build.sh`, the plan-03 approach): a
      `catalog` command that prints the EEPROM drawer group and labels.
      (Built against `/tmp/p04c-dd`; `run` = `/tmp/p04/harness/run`.)

Acceptance: the drawer lists every EEPROM part under Memory → EEPROM with
its access time. **Met:** `xcodebuild` → `** BUILD SUCCEEDED **`;
`run catalog` → `footer: 3989 parts from backend`,
`Memory/EEPROM: 174 parts; functions 8 KB (24), 32 KB (108), 128 KB (42)`,
`AT28C256-15PU: 150 ns isMemory true size 32768 x 8`; SRAM still
`Memory/SRAM: 172 parts`, `AS6C62256-55PCN: 55 ns`.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p04c-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 1 min).
  Check: `/tmp/p04/harness/run catalog` → `Memory/EEPROM: 174 parts`, and `AT28C256-15PU: 150 ns` (est. seconds).
Commit: e1dd565

### Phase 2 — Document, file and simulation load

The setting exists in the model and the file and is loaded on Run; no UI
sets it yet (the harness does). Safe alone: files without `memoryFile` are
unchanged.

- [x] `Canvas/SchematicModel.swift`: `MemoryFile`, `PlacedChip.memoryFile`,
      `setMemoryFile` (§4.2).
- [x] `Document/SchematicFile.swift`: `MemoryFileEntry`,
      `init(document:fileURL:)`, `apply(to:catalog:fileURL:)` (§4.3).
- [x] `Document/SchematicFileController.swift`: pass `fileURL` at the 4
      snapshot sites and to `apply`.
- [x] `schematical-Bridging-Header.h`: `#include "memory_image.h"` beside
      `parts.h` and `circuit.h` (today it includes only those two; the
      header search paths already contain `backend/include`).
- [x] `Memory/MemoryFileAccess.swift` (§4.4).
- [x] `Simulation/Simulation.swift`: load before start, error message (§4.5).
- [x] `docs/file-format.md` (§4.3).
- [x] Harness commands `load` and `roundtrip` (Validation 3–5).

Acceptance: a file set on an EEPROM loads on Run, round-trips through
Save/Open, and resolves by relative path when the bookmark is gone.
**Met (2026-11-01):** `xcodebuild` → `** BUILD SUCCEEDED **`.
`run load`: no file → `readMemory(0,16)` all `FF`, data nets `11111111`;
`boot.bin` → `00 01 … 0F`, `0x1F0` → `F0 … FF`, `0x200` → `FF`, data nets
`00000000`; `boot.hex` → the same; `bad.hex` → `running false error U3:
bad.hex: line 3: bad checksum 0x69 (expected 0x68)`; a missing file →
`U3: can't read missing.bin: The file “missing.bin” couldn’t be opened
because there is no such file.` `run roundtrip`: `jq … .memoryFile.path` →
`"boot.bin"`, other parts have no `memoryFile`; reopen →
`/private/tmp/p04-proof/boot.bin` with its bookmark (see Corrections),
fresh snapshot == saved file (not unsaved); path from `sub/deeper` →
`../../boot.bin`, never saved → absolute. `run moved` (bookmark deleted
with `jq`) → `/tmp/p04-proof/moved/boot.bin`, Run reads `00 01 …`.
  Check: the Phase 1 `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
  Check: `/tmp/p04/harness/run load` and `/tmp/p04/harness/run roundtrip` → Validation 3–5 outputs (est. seconds).
Commit: ce757fa

### Phase 3 — Memory window

The setting row and the stopped preview.

- [x] `Memory/MemoryWindow.swift`: `LoadFileBar`, stopped preview, error
      view, footer, disabled while running (§4.6).
- [x] Harness command `window` rendering `MemoryWindowView` with
      `ImageRenderer` stopped (no file, `boot.bin`, a bad HEX file) and
      running (Validation 2). (Rendered off-screen through `NSHostingView`
      instead; see Corrections. Also `4-sram-stopped.png`.)

Acceptance: the window shows and guards the setting as §4.6 says.
**Met (2026-11-01):** `xcodebuild` → `** BUILD SUCCEEDED **`; `run window`
wrote `/tmp/p04-proof/0-none.png` (row "None — starts erased (FF)", Clear
disabled, grid all `FF`, footer "Contents at start · 32,768 words × 8 bits
(32 KB)"), `1-boot.png` (row "boot.bin", row 0 `00 01 … 0F`, row F0
`F0 … FF`), `2-bad.png` ("Can't Load File", "bad.hex: line 3: bad checksum
0x69 (expected 0x68) Choose the file again."), `3-running.png` (Choose… and
Clear disabled, live grid `00 01 …`, footer without "Contents at start"),
`4-sram-stopped.png` (SRAM unchanged: "Simulation Not Running").
  Check: the Phase 1 `xcodebuild` → `** BUILD SUCCEEDED **` (est. 1 min).
  Check: `/tmp/p04/harness/run window` → the four PNGs of Validation 2 (est. seconds).
- [ ] Validation 6 (sandbox): in the signed build `/tmp/p04-signed-dd`,
      on the desktop: open `eeprom.schematical`, double-click U3, Choose…
      `boot.bin`, Run → live grid `00 01 …`; Stop; Save; quit; relaunch;
      Open the saved file; Run → the same bytes; screenshots to
      `/tmp/p04-proof/sandbox-*.png`. Blocked 2026-11-01: the desktop session
      is locked (Prerequisites row 4: `CGSSessionScreenIsLocked=Yes`); the
      open panel's sandbox grant needs a person at an unlocked desktop.

Commit: 08e165b

## Validation Plan

- Tests: none automated in Swift (no test target). The parser and load are
  covered by plan-04-B's CTests; the app paths by the runtime proof.
- Coverage check: each harness step drives one new path — catalog,
  choose (bookmark), load at start, load error, round trip, relative-path
  fallback, window stopped/running/error.
- Runtime proof. Fixtures in `/tmp/p04-proof/`: `boot.bin` (bytes
  `00 01 … FF` repeated to 512 bytes), `boot.hex` (the same 512 bytes as
  Intel HEX, 16-byte records, plus an `04` record), `bad.hex` (`boot.hex`
  with one checksum changed on line 3), `eeprom.schematical` (VCC, GND, one
  `AT28C256-15PU` with CE# and OE# tied to GND through wires, WE# to VCC,
  A0–A14 to GND).
  1. Catalog: Phase 1 check.
  2. Window (`window`): `0-none.png` row "None — starts erased (FF)", grid
     all `FF`; `1-boot.png` grid row 0 `00 01 … 0F`, footer
     "Contents at start · 32768 words × 8 bits (32 KB)"; `2-bad.png`
     "Can't Load File", `bad.hex: line 3: bad checksum …`; `3-running.png`
     buttons disabled, grid live.
  3. Load (`load`): `boot.bin` set, Run, advance 1 µs: `readMemory(0, 16)`
     → `00 … 0F` all known; the I/O0–I/O7 nets read `00` (address 0, read
     enabled); with `bad.hex` Run fails with
     `U3: bad.hex: line 3: bad checksum …` and
     `isRunning` false. Same with `boot.hex` → same bytes as `boot.bin`.
  4. Round trip (`roundtrip`): set `boot.bin`, save to
     `/tmp/p04-proof/eeprom-saved.schematical` →
     `jq '.parts[] | select(.partNumber=="AT28C256-15PU") | .memoryFile.path'` → `"boot.bin"`;
     reopen → `memoryFile.url.path` = `/tmp/p04-proof/boot.bin`,
     `hasUnsavedChanges` false.
  5. Relative fallback: copy the saved schematic and `boot.bin` to
     `/tmp/p04-proof/moved/`, delete the `bookmark` key with `jq`, open →
     `memoryFile.url.path` = `/tmp/p04-proof/moved/boot.bin`; Run loads it.
  6. Sandbox (signed build, `/tmp/p04-signed-dd`, driven with the `run`
     skill on the desktop): open `eeprom.schematical`, double-click U3,
     Choose… `boot.bin`, Run → live grid `00 01 …`; Stop; Save; quit;
     relaunch; Open the saved file; Run → the same bytes. Screenshots to
     `/tmp/p04-proof/sandbox-*.png`.
- Doc sync: `docs/file-format.md`.
- Final gate (plan-04 overall, once, after Phase 3):
  `rm -rf /tmp/logic-land-p04 && cmake -S backend -B /tmp/logic-land-p04 && cmake --build /tmp/logic-land-p04 -j8 && ctest --test-dir /tmp/logic-land-p04`
  → `100% tests passed, 0 tests failed out of 11` (est. 1 min); the
  `xcodebuild` (est. 1 min); the runtime proof (est. 20 min).
  **Ran (2026-11-01, after Phase 3):** `100% tests passed, 0 tests failed
  out of 11` (2.9 s); goldens unchanged since plan-04-A; signed
  `xcodebuild` → `** BUILD SUCCEEDED **`, entitlements app-sandbox and
  files.user-selected.read-write; runtime proof steps 1–5 re-run on the
  final code with the same results as the phases (`Memory/EEPROM: 174
  parts`; `boot.bin`/`boot.hex` → `00 01 … 0F`, nets `00000000`; `bad.hex`
  → `U3: bad.hex: line 3: bad checksum 0x69 (expected 0x68)`; round trip
  not unsaved; moved copy → `/tmp/p04-proof/moved/boot.bin`). Step 6 not
  run: desktop locked (Phase 3's last task).

## Decisions

Owner decisions for all of plan-04 are in plan-04-A §Decisions. Author's
choices for this letter:

- The setting lives on the placed part (`PlacedChip.memoryFile`), so two
  EEPROMs can load different files.
- A file that can't be read or parsed stops Run with a message naming the
  part and file, rather than starting erased silently.
- Opening never fails over a load file; problems show in the window and at
  Run.
- Security scope is held only around each read (§3).
- Absolute path for a schematic that was never saved; relative from the
  first save on.

## Corrections

- 2026-11-01 (Phase 2): Validation 4 expected the reopened file at
  `/tmp/p04-proof/boot.bin`; the bookmark resolves to the real path,
  `/private/tmp/p04-proof/boot.bin` (`/tmp` is a symlink to `/private/tmp`),
  the same file. Path arithmetic resolves symlinks on both sides, so the
  saved `path` is still `"boot.bin"` and the reopened document is not
  "unsaved". The relative fallback (Validation 5) gives `/tmp/…/moved/…`
  because it joins the stored path to the schematic's folder as given.
- 2026-11-01 (Phase 2): Validation 3 reads the data pins through nets, but
  §Validation's `eeprom.schematical` left I/O0–I/O7 unwired (no net). The
  fixture wires each to its own junction, so each has a net; the harness
  reads them through `SimulationController.wireStyle` (0 → `.normal`,
  1 → green, X → orange, Z → `.floating`). The run lasts the harness's
  300 ms sleep at speed 1 (about 0.3 s of circuit time), not 1 µs; the
  read window settles in 150 ns either way.
- 2026-11-01 (Phase 3): `ImageRenderer` does not run `onAppear` (the window
  loads its page there) and draws AppKit-backed buttons and fields as
  placeholders, so `run window` hosts `MemoryWindowView` in an off-screen
  `NSWindow` and renders the layer tree (`CALayer.render(in:)`;
  `cacheDisplay` skipped SwiftUI's text layers). Same view code, real
  controls.
- 2026-11-01 (Phase 3): the footer prints the word count with the locale's
  grouping, "32,768 words × 8 bits (32 KB)", as the SRAM footer already
  does (`Text` interpolation of an `Int`); §Validation 2's "32768" was the
  same text without the separator.
- 2026-11-01: the ctest count after plan-04 is 11 (plan-05's `engine_test`
  came first), not 10; the final gate below says 11.

## Summary

The risk is the save/open path handling (relative paths, stale bookmarks,
the unsaved-changes snapshot), pinned by the round-trip and fallback
proofs, and the sandbox, which only a signed build driven by a person can
show (Validation 6). SRAM, the PCB page and every existing file are
unchanged.
