# plan-51-F: IC sockets — the parts, the datasheets, the drawer

Last updated: 2026-11-21
Effort: large (3h–1d)
Depends on: plan-51-E

F ships the sockets: `Parts/socket.json` (every `add` row of `candidates.tsv`
plus 15 generic sockets), the socket footprints merged into `footprints.json`,
the datasheet entries, the regenerated goldens, and the small app change that lists
them in the drawer under **Connector → IC Socket** with correct facts. After F the
sockets are catalog parts you can browse and place; the socket mode of a chip is H
and I.

Outcome of F: `ctest` passes 61 programs with the new goldens; the app's drawer lists
`Connector → IC Socket → DIP` and `→ PLCC`, a placed socket is a block of its pins
with the footprint of its series, its sidebar shows Family, Pins, Row spacing,
Mounting and Finish; Run still works with a socket on the sheet.

References:

- `AGENTS.md` (§ Datasheets: in stock, obsolete, footprints, golden regeneration);
  `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints, § socket from plan-51-A);
  `docs/backend.md` § Tests (regenerating goldens).
- `scripts/gen-headers.js` (the generator precedent), plan-51-D § 3.1 (`gen-sockets.js`).
- `planning/plan-51-data/candidates.tsv`, `series.tsv`, `fits.tsv`,
  `footprints-D.json`, `footprints-E.json`.
- App: `schematical/schematical/Backend/PartCatalog.swift` (`CatalogPart.Details`, the
  `init` from a backend `Part`, `symbol(for:)`, drawer grouping), `Backend/PartFacts.swift`,
  `Editor/BundledPartFile.swift`, `Preferences/ReferencePrefixes.swift`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-E complete | `ls planning/completed/plan-51-E-*.md` → exists | MET (2026-11-24: `planning/completed/plan-51-E-socket-footprints-plcc.md`, commit 02e9b252) |
| Side files exist | `ls planning/plan-51-data/footprints-D.json planning/plan-51-data/footprints-E.json` | MET (2026-11-24: both exist, 146 and 8 footprints) |
| Every candidate row is `add` or `out` | `awk -F'\t' 'NR>1 && $12!="add" && $12!="out"' planning/plan-51-data/candidates.tsv \| wc -l` → 0 | MET (2026-11-24: 0) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p51-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-24 at 02e9b252: `** BUILD SUCCEEDED **`, the build with the `PART_SOCKET` kind and no app change) |
| Baselines at F's start | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` (P0 / R0 / F0 = 36515 / 36515 / 2775 at 2865af91; re-measure after A's landing, which adds none) | MET (2026-11-21) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- `ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed` (61 programs).
- `parts.txt` and `traces.txt` grow by exactly `P1` lines (the `add` rows plus 15);
  `footprints.txt` by exactly the footprints the sockets name; no other line changes
  (`git diff` of the three goldens shows only added lines).
- The app's drawer lists the sockets; `DrawerPerformanceTests` passes.

### Non-goals (explicit constraints)

- No socket mode on a chip, no dialog (H, I). No Part Editor support (G).
- No existing part, footprint, kind or file format changes.
- A socket is never `"simulated": false` (plan-51-A decision).

## 2. Current State

- `gen-headers.js` writes `header.json` and the `PINHDR-` footprints; the same shape serves.
- `Parts/*.json`: each file's devices hold pinouts and parts; a pin is
  `{"number": 1, "name": "1", "role": "inout"}` (`header.json`).
- The app loads every `Parts/*.json` in the bundle (`BackendCatalog.load`, `PartCatalog.swift:1005–1020`),
  so `socket.json` is read with no registration. `symbol(for:)` returns `.block` for
  a kind it does not list (`PartCatalog.swift:1217–1250`, `default`);
  `PartFacts` and `BundledPartFile.name(of:)` have `default` arms (`PartFacts.swift:262`,
  `BundledPartFile.swift:122`).
- `ReferenceCategory.of` gives `.integratedCircuit` (prefix `U`) to an unlisted kind
  (`ReferencePrefixes.swift:73–88`); `ReferencePrefixTests` enumerates `ReferenceCategory.allCases` (`:46`).

### Measured populations

| What | Count | Command |
|---|---|---|
| `add` rows at planning time (upper bound; B and C change it) | 569 before B and C | `python3 -I planning/plan-51-data/census/tally.py` → `three makers: 569` |
| Generic sockets | 15 (11 DIP + 4 PLCC) | `awk 'END{print NR-1}' planning/plan-51-data/fits.tsv` → 15 |
| Datasheet ids | one per distinct PDF B downloaded | `ls datasheets/ic-socket/*/*.pdf \| wc -l` |
| Baseline parts, footprints, datasheets | 36515 / 2775 / 9356 (2026-11-24 at 2865af91) | `wc -l backend/tests/golden/parts.txt`; `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']),len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"` |

## 3. Design

### 3.1 `gen-sockets.js --write`

Writes `Parts/socket.json`, replaces every `SKT-` object of `footprints.json` by the
objects the sockets name, and appends the datasheet entries to `datasheets.json`
(ids by the local PDF name, lower case; `url` from the folder's `datasheets.tsv`).
Idempotent; `--check` exits 1 on a difference (`gen-headers.js` precedent).

Devices: one per `(family, pins, spacing)` of `fits.tsv`: id `dip-14-300`, `plcc-44`;
`function` `14-pin, 0.3 in` / `44-pin`; `description` `14-pin 0.3 in DIP socket` /
`44-pin PLCC socket`; `drawer` `["Connector","IC Socket","DIP"]` / `["Connector","IC
Socket","PLCC"]`; `family`, `rowSpacing` (DIP); one pinout `p14` with pins 1 … N
`inout` named by number.

Parts: each `add` row `{"name": <MPN>, "manufacturer": <maker's DigiKey name>,
"datasheet": <id>, "package": "DIP socket"|"PLCC socket", "pinout": "p14",
"footprint": <id>, "mounting": "through-hole"|"surface-mount", "finish": <from the
row's DigiKey `Contact Finish - Mating`, lower case>, "fits": [<chip footprints of
fits.tsv>]}`; no `flags`. Generic sockets: `GENERIC-SOCKET-DIP-14-300`,
`GENERIC-SOCKET-PLCC-44`, manufacturer `Generic`, no datasheet, no finish, mounting
`through-hole` for a DIP and `surface-mount` for a PLCC (the through-hole PLCC groups are
`out`, plan-51-C § Corrections), the generic footprint of `series.tsv`'s `generic` cell.

### 3.2 The app

- `CatalogPart.Details.socket(family:, rowUM:, mounting:, finish:)` and
  `CatalogPart.fits: [Footprint]`, built where `footprintChoices` is (`PartCatalog.swift:1051–1058`).
- `PartFacts`: kind name `IC socket`; rows Family, Pins, Row spacing (DIP, in inches:
  0.3 in), Mounting, Finish (when given). `BundledPartFile.name(of:)`: `PART_SOCKET` →
  `"socket"`.
- `ReferenceCategory.socket` (title `IC Socket`, standard prefix `XS`), returned by
  `ReferenceCategory.of` for `PART_SOCKET`; `ReferencePrefixTests` updated.
- The drawer lists a socket like any part: groups by drawer path then function; the
  package filter shows `DIP socket` / `PLCC socket`. No new drawer code.

### Risk

Mass data and the three goldens. **Byte-identity is not the gate**: the goldens
change by exactly the added lines; the check is `git diff` showing no removed or
changed line.

## 4. Phases

### Phase 1 — generate and merge

- [x] `scripts/gen-sockets.js --write` per § 3.1; `scripts/gen-sockets.test.js` gains the device and part cases (a generic, a maker row, the datasheet id rule, idempotence). — `--write`, `--check`, `--root`; 6 new test cases (23 in all: datasheet id rule, device and part contents for a generic and a maker row, SKT- replacement and datasheet entries, idempotence and `--check`, a missing series row, missing generic coverage).
- [x] Run `node scripts/gen-sockets.js --write`, then `--check` → exit 0. — `socket.json: 575 parts in 15 devices; footprints.json: 154 SKT- footprints; datasheets.json: 8 entries (3 files changed)`; the second `--write` → `(0 files changed)`; `--check` → `socket.json (575 parts), footprints.json and datasheets.json are up to date`, exit 0.
- [x] `docs/parts-format.md`: the table's `socket.json`; the shipped-part count and the generic part names in § socket; the intro's kind list. — § socket gained the shipped-part paragraph (575 parts, 15 generic, the footprint and datasheet naming, no through-hole PLCC); the table's `socket.json` and the intro were A's.

Acceptance: the loader accepts everything.
  Check: `node scripts/gen-sockets.js --write && node scripts/gen-sockets.js --check && cmake --build /tmp/logic-p51 -j8 && /tmp/logic-p51/library_check` → exit 0 (est. 3 min). Measured 2026-11-24 (build dir `/tmp/logic-p51-wt`): `--write` and `--check` exit 0, `library_check: all passed`. VERIFIED.
Commit: 4f96af83

### Phase 2 — goldens

- [x] `/tmp/logic-p51/listing_check --print > backend/tests/golden/parts.txt`; `trace_check --print > …/traces.txt`; `footprint_check --print > …/footprints.txt`. — `listing_check --print` (37090 lines), `trace_check --print` (37090), `footprint_check --print` (2929).
- [x] Review: `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (only added lines); `git diff --stat` shows the three files. — `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `git diff --stat`: footprints.txt +154, parts.txt +575, traces.txt +575.

Acceptance: 61 of 61.
  Check: `ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed, 0 tests failed out of 61` (est. 1 min). Measured 2026-11-24: before the goldens `95% tests passed, 3 tests failed out of 61` (listing_check, trace_check, footprint_check only); after: `100% tests passed, 0 tests failed out of 61`. VERIFIED.
Commit: 4f96af83

### Phase 3 — the app

- [x] `PartCatalog.swift`: `Details.socket`, `fits`, the `init` arm for `PART_SOCKET`; `PartFacts.swift`; `BundledPartFile.swift`; `ReferencePrefixes.swift` (`.socket`) and `ReferencePrefixTests`. — `Details.socket(family:rowUM:mounting:finish:)`, `CatalogPart.fits`, the init arm and `BackendCatalog.catalogPart` reading `part.fits`, the drawer's `detailLabels` arm (`through hole`, finish); `PartFacts` (`IC socket`, Family, Row spacing, Mounting, Finish); `BundledPartFile.name(of:)` → `socket`; `ReferenceCategory.socket` (`IC Socket`, `XS`).
- [x] `schematicalTests/SocketPartsTests.swift`: the catalog has the 15 generics; `GENERIC-SOCKET-DIP-14-300` has 14 pins, a footprint with 14 pads, `fits` naming `PDIP-14` and `CDIP-14`, kind `PART_SOCKET`, facts rows Family `DIP`, Pins `14`, Row spacing `0.3 in`, Mounting `through-hole`; `ReferenceCategory.of` gives `.socket`; a document with a placed generic socket starts a run (`Simulation` builds) and the socket's pins join nothing. — 7 cases (the 15 generics: 11 DIP, 4 PLCC; every socket `>= 575`, drawer `Connector › IC Socket`, a footprint and every `fits` footprint with its pin count of pads; the generic 14-pin DIP socket and its facts; a generic PLCC is surface mount with no Row spacing; a maker socket `A 14-LC-TR`: manufacturer, datasheet URL, footprint, finish; `XS1`, `XS2` and `U1`; Run starts with a socket placed). "The socket's pins join nothing" is the backend `socket_test` circuit case; the app test checks that Run starts.

Acceptance: the tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p51-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/SocketPartsTests -only-testing:schematicalTests/ReferencePrefixTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartRegistrationTests` → `** TEST SUCCEEDED **` (est. 12 min: app build and four classes). Measured 2026-11-24: `** TEST SUCCEEDED **`, 32 tests, 0 failures (SocketPartsTests 7, ReferencePrefixTests 5, DrawerPerformanceTests 7, PartRegistrationTests 13). VERIFIED.
Commit: 5843288a

## Validation Plan

- Tests: `library_check`, `listing_check`, `trace_check`, `footprint_check` (new goldens), `SocketPartsTests`, `ReferencePrefixTests`, `DrawerPerformanceTests`.
- Coverage check: `listing_check` lists every part, so a dropped socket changes the golden; `footprint_check` fails on an unused footprint, so the merge is exact.
- Runtime proof: `SocketPartsTests` builds a circuit with a placed socket through the app's `Simulation`.
- Doc sync: `docs/parts-format.md` (file table, § socket counts).
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions. Author decisions:

- **The generic socket's part name is `GENERIC-SOCKET-<DIP|PLCC>-<pins>[-<300|600>]`** (the `GENERIC-HEADER-1X08` precedent).
- **Standalone sockets get the prefix `XS`**; a chip's socket line in the Bill of Materials is `X` + the chip's reference (plan-51-H).
- **Last Time Buy parts are added unflagged**: the `obsolete` flag is for an ordering table that lists Obsolete.
- **The package is `DIP socket` / `PLCC socket`**, so the drawer's package filter keeps sockets apart from chips.

## Corrections

- **2026-11-24 (plan-51-C), the numbers behind F changed.** `add` rows: 560 (`awk -F'\t' '$12=="add"' planning/plan-51-data/candidates.tsv | wc -l`; Mill-Max 318, Preci-Dip 204, Assmann 38), not "569 before B and C"; 15 candidates (through-hole PLCC) are `out` and in `datasheets/skipped-parts.md`; maker PDFs under `datasheets/ic-socket/`: 8 (Mill-Max 1, Preci-Dip 2, Assmann 5). The 15 generic sockets stay (11 DIP, through hole, from Mill-Max 110; 4 PLCC, **surface mount**, from the `generic` cells of `series.tsv`). Some mounting values come from `candidates.tsv` after C's correction of six Mill-Max `210-47-…` rows to through hole, not from DigiKey's field. Assmann's series are `ASS_<n>`; ids and series text with `_` become `-` (the footprint id rule).

## Summary

F's risk is the volume of generated data and the goldens. Untouched: the engine, existing parts and kinds, the file format.
