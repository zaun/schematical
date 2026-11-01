# plan-04-A: Microchip parallel EEPROMs — datasheets, `eeprom` kind, parts

Last updated: 2026-11-01
Overall Effort: x-large (1d–3d) — plan-04 is A → B → C
Effort: large (3h–1d)
Depends on: plan-03 (footprints; every non-supply part must name one)

plan-04 adds parallel EEPROMs to the part library and the app. They are
"basically the same as SRAM" — the same address/data/CE#/OE#/WE# bus, a
memory window with live contents — but they hold their contents across
power: each placed EEPROM can load a file when the simulation starts.

This sub-plan adds the datasheets and the backend part kind:

- the 12 current Microchip parallel-EEPROM datasheets in
  `datasheets/eeprom/`, and the 4 end-of-life ones recorded in
  `datasheets/obsolete-not-included.md`;
- a new `eeprom` kind (`backend/src/models/eeprom.c`): reads like the SRAM
  model, and writes the way the datasheets describe — byte and page loads
  (tBLC window), an internal write cycle (tWC, up to 10 ms) during which
  reads return DATA# polling on I/O7 and the toggle bit on I/O6, and
  software data protection (SDP). The chip powers up erased (every byte FF);
- `schematical/schematical/Parts/eeprom.json`: 174 parts from 12 devices,
  each naming a footprint (3 reused, 10 new in `footprints.json`).

Outcome of A: `eeprom_test` passes every case of §5.4; `listing_check`,
`trace_check` and `footprint_check` match regenerated goldens that differ
from today's only by the 174 new part lines and the 10 new footprint lines;
the app builds and its drawer lists the new parts (with plain details until
plan-04-C).

References:

- `AGENTS.md` (datasheet and part-file rules), `docs/parts-format.md`
  (part files, § Footprints, § `sram`), `docs/backend.md` (adding a kind,
  tests, goldens).
- `backend/src/models/sram.c` (the model this one mirrors for reads),
  `backend/src/library/kind.h` (`PartKindOps`, `KindContext` helpers),
  `backend/src/library/kinds.c`, `backend/include/parts.h`.
- `planning/completed/plan-03-B-logic-footprints.md` §3 (pad rules R1–R6)
  and `planning/completed/plan-03-C-other-footprints.md` §4.1 (SRAM
  footprints reused here).
- Datasheets (after Phase 1): `datasheets/eeprom/*.pdf` (§4.1).

## Prerequisites

This table is the gate for all of plan-04; B and C point here and add their
own rows.

| Must be true | Command | Status |
|---|---|---|
| plan-03 complete | `ls planning/plan-03-* 2>/dev/null` → no output; `ls planning/completed/plan-03-D-pcb-tab.md` → exists | MET (2026-11-01, 4e0613b) |
| Footprint key required | `grep -n 'missing \\"footprint\\"' backend/src/library/library.c` → one match | MET (2026-11-01) |
| Backend tests green | `cmake -S backend -B /tmp/logic-plan04 && cmake --build /tmp/logic-plan04 -j8 && ctest --test-dir /tmp/logic-plan04` → `100% tests passed, 0 tests failed out of 8` | MET (2026-11-01, 20.3 s) |
| Datasheet server reachable | `curl -s -o /dev/null -w '%{http_code} %{content_type}' -A "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15" -L https://ww1.microchip.com/downloads/en/DeviceDoc/doc4259.pdf` → `200 application/pdf` | MET (2026-11-01) |
| No `eeprom` kind yet | `ls schematical/schematical/Parts/eeprom.json backend/src/models/eeprom.c 2>/dev/null` → no output | MET (2026-11-01) |
| plan-05 complete (owner, 2026-11-01: plan-05 lands before plan-04) | `ls planning/plan-05-* 2>/dev/null` → no output; `ls planning/completed/plan-05-C-app-realtime.md` → exists | NOT MET (2026-11-01) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `ls datasheets/eeprom/*.pdf | wc -l` → 12, each matching the SHA-256 of
  §4.1; `datasheets/obsolete-not-included.md` has the 4 rows of §4.1.
- `grep -c '"name"' schematical/schematical/Parts/eeprom.json` → 174.
- `wc -l < backend/tests/golden/parts.txt` → 3991 (3817 + 174),
  `traces.txt` → 3991, `footprints.txt` → 136 (126 + 10); the diff against
  today's goldens is only added lines.
- `ctest` → `100% tests passed, 0 tests failed out of 9` (8 today +
  `eeprom_test`).

### Non-goals (explicit constraints)

- No change to any existing part, model, footprint or golden line:
  `git diff backend/tests/golden` shows only added lines.
- `sram.c` is not touched (no shared code is factored out of it: its 172
  parts are pinned by `traces.txt`).
- Not modeled: the 12 V chip erase and 12 V device-ID mode (a logic
  simulator has no 12 V level), the VCC-sense power-on write inhibit
  (5–10 ms), the 15 ns write-pulse noise filter (a short pulse is a tWP
  violation instead, §5.3), tCEPH, endurance. No datasheet in scope gives a
  software chip-erase sequence (they cite app note AN0544 only), so there
  is none.
- No load API, no file parsing (plan-04-B); no app change beyond what the
  new enum value forces (plan-04-C adds the EEPROM details, the window
  setting and the file field). Until C lands the drawer shows EEPROMs with
  no detail label (`CatalogPart.init` falls through to `.supply`), which is
  harmless and replaced in C Phase 1.

## 2. Current State

- Kinds: `KINDS[]` in `backend/src/library/kinds.c` (9 kinds); adding one
  follows `docs/backend.md` § Adding a kind (enum value + info struct in
  `parts.h`, `models/<kind>.c`, one `KINDS[]` line, the part file and its
  `parts-format.md` section).
- `PartKind` ends `… PART_OSCILLATOR, PART_SUPPLY`; the app and tests use
  the names, never literal numbers (`grep -rn -E 'PartKind\(rawValue|kind == [0-9]|\.rawValue' schematical/schematical backend/src backend/tests backend/include`
  → 7 hits, all `PART_<NAME>.rawValue` or `\.kind.rawValue` in the drawer's
  kind sets), so appending `PART_EEPROM` renumbers nothing. The drawer's
  category menus key on gate/switch/LED/7seg kinds only; Memory gets none,
  as for SRAM.
- SRAM model (`sram.c`): read output window (Z → X → data, tOH hold,
  high-Z delay), write timing checks storing X, `corrupt()` for unknown
  addresses, `memory_size`/`memory_read` hooks. The EEPROM read side reuses
  its rules with tCLZ = tOLZ = 0 (no EEPROM datasheet gives a low-Z time).
- Footprints (`Parts/footprints.json`, 126): EEPROM packages that match an
  existing land pattern — `PDIP-28-600`, `STSOP1-28-8X13.4`,
  `TSOP1-32-8X20` (SRAM, plan-03-C §4.1).
- Test harness: `load_parts_dir` loads `footprints.json` first, then every
  `*.json` (`backend/tests/support/read_file.c`); `listing_check`,
  `trace_check`, `footprint_check` take `--print` to regenerate goldens.
- Listing: `backend/tests/support/listing.c` names each kind and prints its
  details (`case PART_SRAM: words=… bits=… access_ps=…`).

### Measured populations

| What | Count | Command / source |
|---|---|---|
| Current Microchip parallel-EEPROM datasheets | 12 | census 2026-11-01 (§4.1); every device's Microchip product page reads "In Production" |
| End-of-life datasheets (all parts EOL) | 4 | AT28C040 (doc0542), 28C16A, 28C17A, 28C64A product pages: "End of Life" |
| Orderable parts | 174 | Ordering Information tables of the 12 sheets, die/wafer (W, DWF) codes excluded; full list in Appendix A |
| … of them military / space (`"flags": ["military"]`) | 98 | AT28C256-mil 52 + AT28HC256-mil 24 + AT28C010-mil 19 + AT28C010-12DK 3 |
| Parts per footprint | PLCC-32 38, SOIC-28W 25, LCCC-32 23, CDIP-28-600 19, CFP-28 19, CPGA-28 19, TSOP1-32-8X20 10, CDIP-32-600 6, CFP-32 5, CPGA-30 4, CFP-32-435 3, PDIP-28-600 2, STSOP1-28-8X13.4 1 | Appendix A |
| New footprints | 10 | §4.3 |
| Golden lines today | parts 3817, traces 3817, footprints 126 | `wc -l backend/tests/golden/*` |
| CTests today | 8 | `ctest` above |

### Verified properties

- **Status.** microchip.com product pages answer 403 to scripts; status
  came from their search snippets ("In Production" for AT28C64B, AT28HC64B,
  AT28HC64BF, AT28BV64B, AT28C256, AT28HC256, AT28BV256, AT28C010,
  AT28C010-12DK, AT28LV010; "End of Life" for AT28C040, 28C16A, 28C17A,
  28C64A). Atmel notice SC083106A (2008) ended the PDIP versions that the
  current ordering tables already omit; no current ordering code is marked
  obsolete anywhere, so no part gets `"obsolete"`.
- **Downloads.** All 16 URLs of §4.1 returned `200 application/pdf` with
  the listed size and SHA-256 (curl with a Safari user agent, 2026-11-01).
- **Pinouts.** Every 28-pin package (PDIP, SOIC, CERDIP, flatpack, PGA)
  uses the JEDEC byte-wide order (1 = A14, NC on the 64K parts … 28 = VCC);
  PLCC-32 and LCC-32 share one map per density; the TSOP maps and the
  AT28C010-12DK flatpack differ. All are written out in Appendix B, read
  from each sheet's pin-configuration table.
- **PGA grids** (top view, row 1 at the top, columns left→right), from the
  sheets' pin drawings and cross-checked with the 28U/30U "PIN NUMBERING"
  bottom views:
  `CPGA-28` rows: `4 3 1 27 26 / 5 2 28 24 25 / 7 6 - 22 23 / 9 8 - 20 21 / 11 10 14 16 19 / 12 13 15 17 18`;
  `CPGA-30`: the same with 29 and 30 in the two centre holes (row 3 and
  row 4, column 3).
- **Behavior facts** (every sheet): page load with tBLC between bytes;
  address latched on the later falling edge of CE#/WE#, data on the first
  rising edge; OE# low inhibits writes; DATA# polling on I/O7 and toggle
  bit on I/O6 during tWC; no RDY/BUSY# pin; SDP enable `A/AA, B/55, A/A0`
  and disable `A/AA, B/55, A/80, A/AA, B/55, A/20` with A, B = `5555`,
  `2AAA` (`1555`, `0AAA` on the 64K parts); SDP ships off, except AT28BV64B,
  AT28BV256 and AT28LV010, where it is always on and has no disable; with
  SDP on, a load not preceded by the 3-byte prefix starts the write timer
  but writes nothing; the page address (A6+ or A7+) must not change during
  a page load (what happens if it does is not stated).

## 3. Design Overview

Three phases, lowest risk first:

1. **Datasheets** — files only.
2. **Kind + model** — `PART_EEPROM`, `PartEeprom`, `eeprom.c` (loader and
   model), listing support, loader cases in `library_check`, and
   `eeprom_test` on an inline fixture part file (so the model is proven
   before any real part exists).
3. **Parts** — the 10 footprints, `eeprom.json` (174 parts), golden
   regeneration, and `eeprom_test` cases on real parts.

Correctness risk concentrates in the write state machine of §5.3 (load
window, write cycle, SDP, polling), which is new code with no golden to
compare against; `eeprom_test` checks each rule at its datasheet times.
The footprint numbers are the second risk (hand-checked against the
drawings, as plan-03 did). Golden regeneration is behavior-changing by
design: only added lines are expected, and any changed existing line is a
bug to fix.

Rejected alternatives:

- **Extend the `sram` kind with an "eeprom" flag.** The write path shares
  nothing with SRAM's, and touching `sram.c` risks its 172 golden traces.
- **Factor a shared read core out of `sram.c`.** Same risk, for about 150
  lines of read logic; the EEPROM file has its own copy, written to the
  same rules.
- **Instant SRAM-style writes, or ROM behavior.** Owner chose the datasheet
  write cycle.
- **Defaulting a missing read table (AT28C010-12DK) to zeros.** It is the
  space-grade AT28C010 die; it takes the AT28C010 military `-12` values
  (§4.2).

## 4. Data

### 4.1 Datasheets (`datasheets/eeprom/`)

Download each with
`curl -sfL -A "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15" -o datasheets/eeprom/<file> '<url>'`
and check `shasum -a 256`.

| File | Device(s) | URL | Bytes | SHA-256 |
|---|---|---|---|---|
| `AT28C64B.pdf` | AT28C64B (DS20006432B) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/AT28C64B-64-Kbit-8Kx8-Parallel-EEPROM-with-Page-Write-and-Software-Data-Protection-DS20006432.pdf | 990136 | ddbbd18a10a13a0144c2d6a3f75e85e5bfcb87cb07029e1fb1c141a0bd878fcc |
| `AT28HC64B.pdf` | AT28HC64B, AT28HC64BF (DS20006473B) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/AT28HC64B-AT28HC64BF-64-Kbit-Industrial-High-Speed-Parallel-EEPROM-Page-Write-Software-Data-Protection-DS20006473.pdf | 1008648 | 88cfac2fc130e14dbdc3a837b1d7cee00681920bd92cb209a4491f8230f066c0 |
| `AT28BV64B.pdf` | AT28BV64B (Atmel 0299K) | https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-0299-PEEPROM-AT28BV64B-Datasheet.pdf | 402561 | d0489c27bca44b69e506c3d271370ff97af81363ccccfedc47b479f54316e3ce |
| `AT28C256.pdf` | AT28C256 industrial (DS20006386B) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/AT28C256-Industrial-Grade-256-Kbit-Paged-Parallel-EEPROM-Data-Sheet-DS20006386.pdf | 749812 | 2a586001cf95d3b5f845971ee4e60854e925f4ac349c400d1ce78d470b451de6 |
| `AT28C256-military.pdf` | AT28C256 military (DS20006344A) | https://ww1.microchip.com/downloads/en/DeviceDoc/AT28C256-%E2%80%93-Military-Grade-256-Kbit-(32,768-x-8)-Paged-Parallel-EEPROM.pdf | 521281 | 9e40a446ca17fdf7e4d836ff52e369ee77425aba341b3c6956b18ef5e735388d |
| `AT28HC256.pdf` | AT28HC256 industrial (DS20006428B) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/AT28HC256-256-Kbit-32,768x8-High-Speed-Parallel-EEPROM-DS20006428.pdf | 983616 | 4f2ba5c5b76453e96467dbea40338abece3ad1ba6b6819d237782f3a9a63ceff |
| `AT28HC256-military.pdf` | AT28HC256 military (DS20006352A) | https://ww1.microchip.com/downloads/en/DeviceDoc/Military-Grade-256-Kbit-High-Speed-Paged-Parallel-EEPROM-Data-Sheet-20006352A.pdf | 522442 | 09a660f8ac73df9dfac385d7f7df081b6587043dcaafd47adfd9342e4b7d4991 |
| `AT28BV256.pdf` | AT28BV256 (DS20006409B) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/AT28BV256-256-Kbit-Industrial-Grade-Battery-Voltage-Paged-Parallel-EEPROM-DS20006409.pdf | 997389 | 6e4fbe715413f106ee99d3f25cd4986f281b9d7224cb5bb55c40f4115b00ff67 |
| `AT28C010.pdf` | AT28C010 industrial (DS20006331A) | http://ww1.microchip.com/downloads/en/DeviceDoc/20006331A.pdf | 479462 | 26eda5e4a49099ee150b78b177d82a14031dcd9ab9fa37e823d547223fc09fb7 |
| `AT28C010-military.pdf` | AT28C010 military (DS20006311A) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/20006311A.pdf | 516390 | 7055766b3f6f20b7793e76831330de1f014e23af084c9291730a6bcffd24e96c |
| `AT28C010-12DK.pdf` | AT28C010-12DK space grade (4259E) | https://ww1.microchip.com/downloads/en/DeviceDoc/doc4259.pdf | 364039 | 2d3b24cb74e882db8fca3496cc9e549e585aeff8fa9f9fcb9720a87e75d20113 |
| `AT28LV010.pdf` | AT28LV010 (DS20006405A) | https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/AT28LV010_1-Mbit_(128Kx8)_Industrial_Grade_Low-Voltage_Paged_Parallel_EEPROM_20006405A.pdf | 425898 | 8ad946dee1bebfe71f6748bd55b5a7a29f65a4329dea8cdd1a0856c0a863c7b1 |

`datasheets/obsolete-not-included.md` gains (PDFs not committed; family
`eeprom`, group Memory → EEPROM):

| PDF | Family | Category / group | Notes |
|---|---|---|---|
| `doc0542.pdf` (AT28C040, 0542F) | eeprom | Memory → EEPROM | 512K × 8; End of Life (Microchip product page); https://ww1.microchip.com/downloads/en/DeviceDoc/doc0542.pdf |
| `11125J.pdf` (28C16A, DS11125J) | eeprom | Memory → EEPROM | 2K × 8; End of Life; https://ww1.microchip.com/downloads/en/devicedoc/11125J.pdf |
| `11127J.pdf` (28C17A, DS11127J) | eeprom | Memory → EEPROM | 2K × 8 with RDY/BSY#; End of Life; https://ww1.microchip.com/downloads/en/devicedoc/11127J.pdf |
| `11109K.pdf` (28C64A, DS11109K) | eeprom | Memory → EEPROM | 8K × 8; End of Life; https://ww1.microchip.com/downloads/en/DeviceDoc/11109K.pdf |

### 4.2 Devices and grades

12 devices, one per datasheet. Grade key = the speed code, plus `F` for the
fast-write option (E, the high-endurance option, uses the base grade).
Times in ns; `tblc` and `twc` are maxima, the rest minima (write) or maxima
(read). Every grade also has `toh 0, tas 0, tcs 0, tch 0, toes 0`.

| Device id | Grades | tacc = tce | toe | tdf | tah | twp | tds | tdh | toeh | twph | tblc | twc |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `AT28C64B` | 15 | 150 | 70 | 50 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 |
| `AT28HC64B` | 70 / 90 / 12 / 12F | 70 / 90 / 120 / 120 | 35 / 40 / 50 / 50 | 35 / 40 / 50 / 50 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 (12F: 2000000) |
| `AT28BV64B` | 20 | 200 | 80 | 55 | 100 | 200 | 100 | 0 | 0 | 100 | 100000 | 10000000 |
| `AT28C256` | 15 / 15F | 150 | 70 | 50 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 (15F: 3000000) |
| `AT28C256-MIL` | 15 / 20 / 25 / 15F | 150 / 200 / 250 / 150 | 70 / 80 / 100 / 70 | 50 / 55 / 60 / 50 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 (15F: 3000000) |
| `AT28HC256` | 70 / 90 / 12 / 90F | 70 / 90 / 120 / 90 | 35 / 40 / 50 / 40 | 35 / 40 / 50 / 40 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 (90F: 3000000) |
| `AT28HC256-MIL` | 90 / 12 / 90F / 12F | 90 / 120 / 90 / 120 | 40 / 50 / 40 / 50 | 40 / 50 / 40 / 50 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 (F: 3000000) |
| `AT28BV256` | 20 | 200 | 80 | 55 | 50 | 200 | 50 | 0 | 0 | 100 | 150000 | 10000000 |
| `AT28C010` | 12 / 15 | 120 / 150 | 50 / 55 | 50 / 55 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 |
| `AT28C010-MIL` | 12 / 15 / 20 / 25 | 120 / 150 / 200 / 250 | 50 / 55 / 55 / 55 | 50 / 55 / 55 / 55 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 |
| `AT28C010-12DK` | 12 | 120 | 50 | 50 | 50 | 100 | 50 | 0 | 0 | 50 | 150000 | 10000000 |
| `AT28LV010` | 20 | 200 | 80 | 55 | 100 | 200 | 100 | 10 | 10 | 100 | 150000 | 10000000 |

Device keys (§5.1) per device:

| Device id | Org (address bits) | `function` | `pageSize` | `commandAddresses` | `protection` |
|---|---|---|---|---|---|
| AT28C64B, AT28HC64B | 8K × 8 (13) | `8 KB` | 64 | `["1555","0AAA"]` | `off` |
| AT28BV64B | 8K × 8 (13) | `8 KB` | 64 | `["1555","0AAA"]` | `always` |
| AT28C256, -MIL, AT28HC256, -MIL | 32K × 8 (15) | `32 KB` | 64 | `["5555","2AAA"]` | `off` |
| AT28BV256 | 32K × 8 (15) | `32 KB` | 64 | `["5555","2AAA"]` | `always` |
| AT28C010, -MIL, -12DK | 128K × 8 (17) | `128 KB` | 128 | `["5555","2AAA"]` | `off` |
| AT28LV010 | 128K × 8 (17) | `128 KB` | 128 | `["5555","2AAA"]` | `always` |

`drawer` `["Memory","EEPROM"]`; `description` e.g. `"32K x 8 paged parallel EEPROM"`
(`"… low-voltage …"` for BV/LV, `"… high-speed …"` for HC); `note` names
the PDF and pages as `sram.json` does. AT28C010-12DK's note records that
its datasheet has no AC read table and the AT28C010 military `-12` values
are used, and that its printed tBLC "150 ms" is read as 150 µs.

### 4.3 Footprints (`Parts/footprints.json`, 10 new)

Rules from plan-03-B §3; body = nominal (mean of min and max, rounded half
up to 0.01). Where a drawing does not print the lead tip-to-tip size
(flatpacks), E(max) = body width (max) + 2 × lead length (max). Where two
views of one drawing disagree (28U), the larger (bottom-view) outline is
used. Pin-1 at the top of the left column (dual) or top centre (quad).

| id | Style | Values | Drawing (file p.) |
|---|---|---|---|
| `SOIC-28W` | dual, R1 | pins 28, pitch 1.27, span 9.40, pad 2.00 × 0.60 rect, body 7.50 × 17.90 | Microchip C04-052C outline + C04-2052A land pattern (`AT28C256.pdf` p.24–26) |
| `PLCC-32` | quad, R1 | sides [9,7,9,7], pitch 1.27, spanX 10.80, spanY 13.31, pad 2.54 × 0.66 rect, body 11.43 × 13.97, `slots` pin p → p−4 for p = 5…32, pins 1–4 → 29–32 | C04-023 + C04-2023 land pattern (`AT28C64B.pdf` p.22–24) |
| `CDIP-28-600` | dual, R5 | pins 28, pitch 2.54, span 15.24, drill 0.95 (lead 0.66 max), pad 1.65 round, `pin1Shape` rect, body 14.22 × 37.22 | 28D6 (`AT28C256-military.pdf` p.22) |
| `CDIP-32-600` | dual, R5 | pins 32, pitch 2.54, span 15.24, drill 0.85 (lead 0.58 max), pad 1.55 round, `pin1Shape` rect, body 14.25 × 42.20 | 32D6 (`AT28C010-military.pdf` p.22) |
| `CFP-28` | dual, R2 | pins 28, pitch 1.27, pad 10.20 × 0.61 rect (lead 9.40 max, b 0.56 max), E(max) 29.37 → span 20.17, body 10.16 × 18.29 | 28F (`AT28C256-military.pdf` p.23) |
| `CFP-32` | dual, R2 | pins 32, pitch 1.27, pad 10.20 × 0.56 rect (lead 9.40 max, b 0.51 max), E(max) 31.20 → span 22.00, body 12.20 × 20.84 | 32F (`AT28C010-military.pdf` p.24) |
| `CFP-32-435` | dual, R2 | pins 32, pitch 1.27, pad 9.00 × 0.53 rect (lead 8.20 max, b 0.48 max), E(max) 27.58 → span 19.58, body 11.05 × 20.83 | FP32.435 (`AT28C010-12DK.pdf` p.17) |
| `LCCC-32` | quad, R4 | sides [9,7,9,7], pitch 1.27, terminal 1.27 × 0.635 nominal → pad 1.57 × 0.685 rect, spanX 10.46, spanY 13.00, body 11.43 × 13.97, `slots` as PLCC-32 | 32L / 32L1 (`AT28C256-military.pdf` p.24, `AT28C010-military.pdf` p.23) |
| `CPGA-28` | explicit, R5 | 28 pads on the 5 × 6, 2.54 mm grid of §2 (centre column holes in rows 3–4 empty), x = (col − 3) × 2.54, y = (row − 3.5) × 2.54, drill 0.85 (pin 0.58 max), pad 1.55 round, body 13.97 × 16.51 | 28U (`AT28C256-military.pdf` p.25) |
| `CPGA-30` | explicit, R5 | 30 pads, full 5 × 6 grid of §2, same pad, drill, body | 30U (`AT28C010-military.pdf` p.25) |

`explicit` gives no `pin1Shape`; pin 1's pad is round like the rest (the
format allows none other there). Reused unchanged: `PDIP-28-600` (28P6 /
C04-079B, row 15.24), `STSOP1-28-8X13.4` (28T), `TSOP1-32-8X20` (32T).

## 5. Detailed Design — the `eeprom` kind

### 5.1 Part file keys (`docs/parts-format.md` § `eeprom`)

```json
"address": ["A0", "A1", "…", "A14"],
"data": ["I/O0", "I/O1", "I/O2", "I/O3", "I/O4", "I/O5", "I/O6", "I/O7"],
"chipEnable": "CE#", "outputEnable": "OE#", "writeEnable": "WE#",
"pageSize": 64,
"commandAddresses": ["5555", "2AAA"],
"protection": "off",
"grades": {"15": {"tacc": 150, "tce": 150, "toe": 70, "tdf": 50, "tah": 50, "twp": 100,
                  "tds": 50, "twph": 50, "tblc": 150000, "twc": 10000000, "note": "…"}}
```

| Key | Rule |
|---|---|
| `address` | 1–24 pin names; `address[i]` is bit i |
| `data` | exactly 8 pin names |
| `chipEnable`, `outputEnable`, `writeEnable` | one pin name each (active low) |
| `pageSize` | power of two, 1 ≤ pageSize ≤ 2^len(address) |
| `commandAddresses` | two hex strings (`[0-9A-F]+`), each < 2^len(address): the SDP addresses A and B |
| `protection` | `"off"` (SDP starts disabled, can be enabled and disabled) or `"always"` (always enabled, no disable) |
| `grades` | non-empty; timing names `tacc tce toe tdf toh tas tah tcs tch twp twph tds tdh toes toeh tblc twc` (ns, missing = 0) and optional `note`; `tblc` and `twc` must be > 0 |

Part key `"grade"` (a key of `grades`). Errors use the kind helpers' style,
e.g. `eeprom.json: devices[0] (AT28C256): "pageSize" must be a power of two`.

### 5.2 Public facts (`parts.h`)

```c
PART_EEPROM            /* appended after PART_SUPPLY */
typedef struct { uint32_t words; unsigned bits; uint64_t access_ps; const char *grade;
                 unsigned page_bytes; uint64_t write_cycle_ps; } PartEeprom;
const PartEeprom *eeprom;   /* on Part, after supply */
```

`words` = 2^len(address), `bits` = 8, `access_ps` = `tacc`, `page_bytes` =
`pageSize`, `write_cycle_ps` = `twc`. `listing.c`: kind name `eeprom`,
details `words=%u bits=%u access_ps=%llu page=%u twc_ps=%llu`.

### 5.3 Model (`backend/src/models/eeprom.c`)

State per instance: `value[words]`, `known[words]` (uint8), created erased
(`0xFF`/`0xFF`); the page buffer (up to 128 loaded offsets with value,
known, loaded flag); SDP state; mode (idle, loading, busy).

**Controls.** A control reads 1 (active), 0 or −1 (X) as in `sram.c`
(Z = inactive). `read_on = CE ∧ OE ∧ ¬WE`; `write_on = CE ∧ WE ∧ ¬OE`
(three-valued AND).

**Reads (idle or loading).** Exactly SRAM's window with tCLZ = tOLZ = 0:
outputs leave Z when `read_on` turns 1 (X until valid), are valid at
max(address + tacc, CE# fall + tce, OE# fall + toe), hold old data tOH
after an address change, and return to Z tdf after `read_on` turns 0 (X in
between). An X `read_on` drives X.

**Loads.** A load starts when `write_on` turns 1: the address is latched
then. It ends when `write_on` turns 0: the data is latched then. The byte
is stored in the page buffer as X (all bits unknown) when any of these
fails: pulse ≥ twp; address stable ≥ tas before the start and unchanged
for tah after it; data stable ≥ tds before the end; OE# inactive ≥ toes
before the start; and, for the second and later loads of a window, the
gap since the previous end ≥ twph. A data or OE# change within tdh / toeh
after the end turns that byte X. X data bits are stored unknown. The first
load opens a window; each later load must start within tblc of the
previous end. A timer at (last end + tblc) with no load started since
begins the write cycle.

**Page.** The window's page is the first load's address with the low
log2(pageSize) bits cleared. A load whose page differs marks the window
"mixed": the cycle then writes X to every loaded offset in the first
page (the datasheets leave this undefined; X is the conservative result).
Loading the same offset twice keeps the later byte.

**SDP.** The window's first loads are compared with the command sequences
(addresses A, B from `commandAddresses`, data hex):
- enable prefix `A/AA, B/55, A/A0`: the rest of the window is data;
  protection turns on at the end of the cycle;
- disable `A/AA, B/55, A/80, A/AA, B/55, A/20` (only when `protection` is
  `off`): the rest is data; protection turns off at the end of the cycle.
Command bytes are never data. While protection is on, a window that does
not start with the enable prefix writes nothing, but its cycle still runs
(busy for twc). When a window's first loads match only part of a sequence
and then diverge, those loads are data. Protection starts on for
`always`, off for `off`, at every power-up (each `circuit_start`), as the
sheets ship it.

**Write cycle (busy, twc).** Starts at the tblc timeout. Loads are
ignored. A read (`read_on` going to 1, i.e. OE# or CE# falling with the
other low and WE# high) drives I/O7 = complement of bit 7 of the last
loaded byte (X if that bit is X), I/O6 = the toggle bit, I/O0–I/O5 = X,
with the normal read timing. The toggle bit is 0 on the first read of the
cycle and flips on each later read. At twc the loaded bytes are committed
to `value`/`known` (or X per "mixed"), SDP changes apply, the chip goes
idle, and outputs refresh to true data.

**Unknown controls.** `write_on` = X while idle or loading adds the
current address (or, with unknown address bits, every word matching the
known ones, SRAM's `corrupt()` rule) to the window as X, opening one if
needed. An X while busy changes nothing.

**Hooks.** `memory_size` (words, 8), `memory_read` (committed array, not
the page buffer). No `set_switch`/`lights`.

### 5.4 Tests

- `library_check`: a valid one-device fixture (`PartEeprom` fields), and
  one rejection per §5.1 rule: data count 7, `pageSize` 48, `pageSize` 2×
  words, command address `"8000"` on a 15-bit part, `"55G5"`, `protection`
  `"sometimes"`, `twc` 0, unknown timing name, part grade not in grades.
- `eeprom_test` (new CTest) — Phase 2 on an inline fixture (a copy of the
  `AT28C256` device with grades 15 and 15F, one PDIP pinout, footprint
  `PDIP-28-600`), Phase 3 adds real parts:
  1. power-up: `memory_read` all FF known; a read at 0 drives Z before
     CE#/OE#, X, then `FF` at max(tacc, tce, toe); Z again tdf after OE#.
  2. byte write `0x5A` at `0x1234`: memory unchanged until tblc + twc after
     the load ends; reads during the cycle show I/O7 = 1, I/O6 0 then 1 then
     0 on three reads, I/O0–5 X; after the cycle `0x5A` known and a read
     returns it.
  3. page write: 64 bytes in one page within tblc of each other → one cycle,
     all 64 committed at the same time; bytes outside the page unchanged.
  4. gap > tblc: the second byte starts after the cycle began → ignored.
  5. load during busy → ignored.
  6. twp − 1 ns pulse → that byte X after the cycle; tds violated → X.
  7. mixed page → every loaded offset of the first page X.
  8. SDP: enable prefix + one byte → written, protection on; then an
     unprotected byte → cycle runs (busy for twc, polling seen) and nothing
     written; protected byte → written; disable sequence → unprotected
     bytes write again.
  9. X on WE# with a known address → that byte X after the cycle.
  10. (Phase 3) `AT28C256F-15JU`: commit at tblc + 3 ms, not 10 ms.
  11. (Phase 3) `AT28BV256-20JU`: unprotected write from power-up writes
      nothing; the disable sequence leaves it protected.
  12. (Phase 3) `AT28C64B-15PU`: the enable prefix at `1555`/`0AAA` turns
      protection on (a following unprotected byte is not written).
  13. memory hooks: `AT28C010-12JU` size 131072 × 8.

## Compatibility / Format Impact

- Part file format: a new kind (`eeprom`) and file; no existing key changes.
- `parts.h`: one enum value (appended), one struct, one `Part` field.
  `PartKind` numbers of existing kinds are unchanged.
- Goldens: 174 lines added to `parts.txt` and `traces.txt`, 10 to
  `footprints.txt`; no existing line changes.
- App: compiles unchanged (the new enum value hits existing defaults).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>`; never delete it.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Datasheets

Files only; nothing loads them.

- [ ] Download the 12 PDFs of §4.1 into `datasheets/eeprom/` with the given
      names; check each SHA-256.
- [ ] `datasheets/obsolete-not-included.md`: the 4 rows of §4.1 (their PDFs
      are not committed).

Acceptance: the 12 files are the census's files; the 4 EOL sheets are
recorded.
  Check: `cd datasheets/eeprom && shasum -a 256 *.pdf` → the 12 hashes of §4.1 (est. seconds).
  Check: `grep -c 'Memory → EEPROM' datasheets/obsolete-not-included.md` → 4 (est. seconds).
Commit: —

### Phase 2 — `eeprom` kind and model

The kind loads and simulates; no part file uses it yet, so every golden is
unchanged.

- [ ] `backend/include/parts.h`: `PART_EEPROM`, `PartEeprom`, `Part.eeprom` (§5.2).
- [ ] `backend/src/models/eeprom.c`: `PartKindOps EEPROM_KIND` (device keys
      §5.1, part key `grade`, load/bind/part) and `EEPROM_MODEL` (§5.3).
- [ ] `backend/src/library/kinds.c`: `&EEPROM_KIND`;
      `backend/CMakeLists.txt`: `src/models/eeprom.c`, `logic_test(eeprom_test)`.
- [ ] `backend/tests/support/listing.c`: `eeprom` name and details (§5.2).
- [ ] `backend/tests/library_check.c`: the §5.4 cases.
- [ ] `backend/tests/eeprom_test.c`: cases 1–9 on the inline fixture.
- [ ] `docs/parts-format.md`: `eeprom.json` in the file table, the
      `### eeprom` section (§5.1, the behavior of §5.3 in prose like `sram`);
      `docs/backend.md`: `eeprom.c` in Layout, `eeprom_test` in Tests.

Acceptance: every model rule of §5.3 holds at its datasheet time on the
fixture; nothing else moves.
  Check: `cmake -S backend -B /tmp/logic-plan04 && cmake --build /tmp/logic-plan04 -j8 && ctest --test-dir /tmp/logic-plan04 -R 'eeprom_test|library_check' --output-on-failure` → 2 of 2 pass (est. 20 s).
  Check: `ctest --test-dir /tmp/logic-plan04 -R 'listing_check|trace_check|footprint_check'` → 3 of 3 pass, and `git diff --exit-code backend/tests/golden` (est. 15 s).
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p04a-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 1 min; the app compiles `backend/src` and imports `parts.h`).
Commit: —

### Phase 3 — Footprints, parts, goldens (largest blast radius last)

- [ ] `footprints.json`: the 10 footprints of §4.3, each with a `note`
      naming drawing, file and page.
- [ ] `schematical/schematical/Parts/eeprom.json`: the 12 devices of §4.2,
      pinouts of Appendix B, the 174 parts of Appendix A (package name,
      pinout, footprint, grade, `"flags": ["military"]` on the 98 military
      / space parts).
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt` with
      `--print`; hand-check every new `footprints.txt` line against its
      drawing (pitch, span, pad size, pin 1 position, PGA pin positions).
- [ ] `eeprom_test`: cases 10–13 on real parts.

Acceptance: 174 parts load with footprints; goldens grow only by the new
lines.
  Check: `grep -c '"name"' schematical/schematical/Parts/eeprom.json` → 174; `grep -c 'military' schematical/schematical/Parts/eeprom.json` → 98 (est. seconds).
  Check: `wc -l < backend/tests/golden/parts.txt` → 3991, `traces.txt` 3991, `footprints.txt` 136; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. seconds).
  Check: `cmake --build /tmp/logic-plan04 -j8 && ctest --test-dir /tmp/logic-plan04 --output-on-failure` → `100% tests passed, 0 tests failed out of 9` (est. 1 min; the whole backend suite, because every golden and loader test reads `Parts/`).
Commit: —

## Validation Plan

- Tests: `library_check` (loader rules), `eeprom_test` (§5.4, fixture and
  real parts), `listing_check`/`trace_check`/`footprint_check` (goldens
  with the new lines).
- Coverage check: `eeprom.c` is in `add_library(logic …)`; every §5.3 rule
  has an `eeprom_test` case (the list in §5.4 maps one to one).
- Runtime proof: `/tmp/logic-plan04/listing_check --print | grep -c ' eeprom '`
  → 174; the app build (Phase 2 check) loads `eeprom.json` through the
  bundle (`BackendCatalog.load` scans `Parts/*.json`). The full app proof
  is plan-04-C's.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`,
  `datasheets/obsolete-not-included.md`.
- Final gate (once, after Phase 3): `rm -rf /tmp/logic-land-p04a && cmake -S backend -B /tmp/logic-land-p04a && cmake --build /tmp/logic-land-p04a -j8 && ctest --test-dir /tmp/logic-land-p04a` → `100% tests passed, 0 tests failed out of 9` (est. 1 min); the Phase 2 `xcodebuild` (est. 1 min).

## Decisions

Owner decisions for all of plan-04 (Justin Zaun, 2026-11-01):

- **Maker: Microchip AT28 parallel EEPROMs.** Alliance Memory (the SRAM
  maker) makes no EEPROMs (its catalog: DRAM, SRAM, flash, eMMC). (§4.1)
- **Writes follow the datasheet write cycle** (page loads, tWC busy, DATA#
  polling, toggle bit), not instant writes or ROM. A 10 ms cycle is about
  16 s of real time at the app's 10 µs-per-tick speed. (§5.3)
- **All 174 parts, military and space included**, with the 10 footprints
  that needs. (§4.3, Appendix A)
- **Power-up contents: erased (FF) with no load file; the file is loaded
  again at every start; writes made during a run are lost at Stop.** (§5.3,
  plan-04-C)
- **Load file formats: raw binary and Intel HEX.** (plan-04-B §4.3)
- **The schematic stores the load file as a path relative to the schematic
  plus an app-scoped security bookmark** (not document-scoped: that needs an
  extra entitlement and has reported failures on recent macOS). (plan-04-C §4.3)
- **The memory window, when stopped, shows the "Load file on start"
  setting and a preview of the start contents**; the setting is locked
  while running. (plan-04-C §4.6)
- **plan-04 comes after plan-03** and adds its own footprints.

Author's choices: one device per datasheet; grade `F` for the fast-write
option; AT28C010-12DK takes the AT28C010 military `-12` read values; X for
a mixed-page load; command bytes never stored; protection resets to the
shipped state at every power-up; the toggle bit starts at 0; reels (`-T`)
are parts (as SRAM's `TR` codes and TI's `R` codes are), die and wafer
codes are not; no reel code is invented where a sheet prints none
(AT28C256 industrial, AT28HC256E-70SU tube).

## Corrections

- 2026-11-01 (before start): plan-05 now lands first (owner). It adds one
  ctest (`engine_test`, plan-05-B), so every ctest count in plan-04 A/B/C
  is one higher (A: 10, B: 11, C: 11), and plan-05-B regenerates
  `traces.txt` (2479 changed lines) before plan-04-A adds its lines.
  plan-05-C moves the circuit loop in `Simulation.swift` to
  `SimulationEngine` (background thread); `SimulationController.build` and
  the build-then-`circuit_start` order that plan-04-C hooks into are kept,
  with `start(document:)` now calling `start(netlist:)`.

## Summary

The risk is the new write state machine (§5.3), proven rule by rule in
`eeprom_test`, and the 10 hand-derived footprints, hand-checked against
their drawings. Existing parts, models and golden lines are untouched; the
goldens only grow.

## Appendix A — the 174 parts

Package names in the part file: `PLCC`, `SOIC`, `PDIP`, `TSOP`, `CERDIP`,
`Flatpack`, `LCC`, `PGA`. Pinout keys: `p28` (28-pin byte-wide order),
`plcc32` (PLCC-32 / LCC-32 map), `tsop28`, `tsop32`, `p32` (32-pin
CERDIP/flatpack, JEDEC), `fp32dk` (AT28C010-12DK flatpack), `pga28`,
`pga30`. Footprint follows from the package (Appendix B).

| Device | Parts (grade in brackets; `/883` codes are military) |
|---|---|
| AT28C64B (5) | -15JU, -15JU-T (PLCC); -15PU (PDIP); -15SU, -15SU-T (SOIC) [15] |
| AT28HC64B (16) | AT28HC64B-{70,90,12}{JU,JU-T} (PLCC), AT28HC64B-{70,90,12}{SU,SU-T} (SOIC) [70/90/12]; AT28HC64BF-12{JU,JU-T} (PLCC), AT28HC64BF-12{SU,SU-T} (SOIC) [12F] |
| AT28BV64B (3) | -20JU (PLCC), -20SU (SOIC), -20TU (TSOP 28) [20] |
| AT28C256 (7) | AT28C256-15{JU,PU,SU}; AT28C256E-15{JU,SU} [15]; AT28C256F-15{JU,SU} [15F] |
| AT28C256-MIL (52) | AT28C256-{15,20,25}{DM,FM,LM,UM}/883 and each with `-815` (24) [15/20/25]; AT28C256E-{15,25}{DM,FM,LM,UM}/883 and each with `-815` (16), AT28C256E-20{DM,FM,LM,UM}/883 with no `-815` (4) [15/20/25]; AT28C256F-15{DM,FM,LM,UM}/883 and each with `-815` (8) [15F]. D = CERDIP, F = Flatpack, L = LCC, U = PGA |
| AT28HC256 (21) | AT28HC256-70{JU,JU-T}, -90{JU,JU-T,SU,SU-T}, -12{JU,JU-T,SU,SU-T} [70/90/12]; AT28HC256E-70{JU,JU-T}, AT28HC256E-70SU-T, AT28HC256E-90{JU,JU-T,SU,SU-T} [70/90]; AT28HC256F-90{JU,JU-T,SU,SU-T} [90F] |
| AT28HC256-MIL (24) | AT28HC256{,E}-{90,12}{DM,FM,LM,UM}/883 [90/12]; AT28HC256F-{90,12}{DM,FM,LM,UM}/883 [90F/12F] |
| AT28BV256 (4) | -20{JU,JU-T,SU,SU-T} [20] |
| AT28C010 (16) | AT28C010{,E}-{12,15}{JU,JU-T} (PLCC), AT28C010{,E}-{12,15}{TU,TU-T} (TSOP 32) [12/15] |
| AT28C010-MIL (19) | AT28C010-{12,15,20}{DM,EM,FM,UM}/883 (12), AT28C010-25{DM,FM}/883 (2) [12/15/20/25]; AT28C010E-12{DM,EM,FM,UM}/883 (4), AT28C010E-15DM/883 (1) [12/15]. D = CERDIP, E = LCC, F = Flatpack, U = PGA |
| AT28C010-12DK (3) | AT28C010-12DK-E, -12DK-MQ, -12DK-SV (Flatpack) [12] |
| AT28LV010 (4) | -20{JU,JU-T} (PLCC), -20{TU,TU-T} (TSOP 32) [20] |

Military flag: every `/883` part and the three `-12DK` parts (98).

## Appendix B — pinouts and footprints

| Pinout | Packages → footprint | Pin map (number = name) |
|---|---|---|
| `p28` (256K) | PDIP → PDIP-28-600; SOIC → SOIC-28W; CERDIP → CDIP-28-600; Flatpack → CFP-28 | 1 A14, 2 A12, 3 A7, 4 A6, 5 A5, 6 A4, 7 A3, 8 A2, 9 A1, 10 A0, 11 I/O0, 12 I/O1, 13 I/O2, 14 GND, 15 I/O3, 16 I/O4, 17 I/O5, 18 I/O6, 19 I/O7, 20 CE#, 21 A10, 22 OE#, 23 A11, 24 A9, 25 A8, 26 A13, 27 WE#, 28 VCC |
| `p28` (64K: AT28C64B, AT28HC64B, AT28BV64B) | PDIP → PDIP-28-600; SOIC → SOIC-28W | the 256K map with 1 NC (not A14) and 26 NC (not A13) |
| `plcc32` (64K) | PLCC → PLCC-32 | the 256K `plcc32` map with 2 NC (not A14) and 30 NC (not A13) |
| `tsop28` (AT28BV64B) | TSOP → STSOP1-28-8X13.4 | 1 OE#, 2 A11, 3 A9, 4 A8, 5 NC, 6 WE#, 7 VCC, 8 NC, 9 A12, 10 A7, 11 A6, 12 A5, 13 A4, 14 A3, 15 A2, 16 A1, 17 A0, 18 I/O0, 19 I/O1, 20 I/O2, 21 GND, 22 I/O3, 23 I/O4, 24 I/O5, 25 I/O6, 26 I/O7, 27 CE#, 28 A10 |
| `plcc32` (256K) | PLCC → PLCC-32; LCC → LCCC-32 | 1 DC, 2 A14, 3 A12, 4 A7, 5 A6, 6 A5, 7 A4, 8 A3, 9 A2, 10 A1, 11 A0, 12 NC, 13 I/O0, 14 I/O1, 15 I/O2, 16 GND, 17 DC, 18 I/O3, 19 I/O4, 20 I/O5, 21 I/O6, 22 I/O7, 23 CE#, 24 A10, 25 OE#, 26 NC, 27 A11, 28 A9, 29 A8, 30 A13, 31 WE#, 32 VCC |
| `plcc32` (1M) | PLCC → PLCC-32; LCC → LCCC-32 | 1 DC (NC on military), 2 A16, 3 A15, 4 A12, 5 A7, 6 A6, 7 A5, 8 A4, 9 A3, 10 A2, 11 A1, 12 A0, 13 I/O0, 14 I/O1, 15 I/O2, 16 GND, 17 I/O3, 18 I/O4, 19 I/O5, 20 I/O6, 21 I/O7, 22 CE#, 23 A10, 24 OE#, 25 A11, 26 A9, 27 A8, 28 A13, 29 A14, 30 NC, 31 WE#, 32 VCC |
| `p32` (1M) | CERDIP → CDIP-32-600; Flatpack → CFP-32 | same names as `plcc32` (1M), pins 1–32 |
| `tsop32` (1M) | TSOP → TSOP1-32-8X20 | 1 A11, 2 A9, 3 A8, 4 A13, 5 A14, 6 NC, 7 WE#, 8 VCC, 9 NC, 10 A16, 11 A15, 12 A12, 13 A7, 14 A6, 15 A5, 16 A4, 17 A3, 18 A2, 19 A1, 20 A0, 21 I/O0, 22 I/O1, 23 I/O2, 24 GND, 25 I/O3, 26 I/O4, 27 I/O5, 28 I/O6, 29 I/O7, 30 CE#, 31 A10, 32 OE# |
| `fp32dk` | Flatpack → CFP-32-435 | 1 A16, 2 A15, 3 A12, 4 A7, 5 A6, 6 NC, 7 A5, 8 A4, 9 A3, 10 A2, 11 A1, 12 A0, 13 I/O0, 14 I/O1, 15 I/O2, 16 GND, 17 I/O3, 18 I/O4, 19 I/O5, 20 I/O6, 21 I/O7, 22 CE#, 23 A10, 24 OE#, 25 A11, 26 A9, 27 A8, 28 A13, 29 A14, 30 WE#, 31 NC, 32 VCC |
| `pga28` | PGA → CPGA-28 | `p28` names; positions §2 |
| `pga30` | PGA → CPGA-30 | `p28` names plus 29 A15, 30 A16; positions §2 |

Roles: A*, CE#, OE#, WE# `input` (address pins in group `address`), I/O*
`inout`, VCC `power`, GND `ground`, NC and DC `nc`. Where a sheet's table
and drawing disagree, the table wins (AT28C010 military flatpack pins
17/18: table I/O3/I/O4).
