# plan-36-A: Microcontrollers — census refresh, stock check, datasheets, ordering-table verdicts

Last updated: 2026-11-12
Overall Effort: huge (> 3d) — plan-36 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d; one DigiKey quota day for Phase 1)
Depends on: nothing (§ Prerequisites)

plan-36 adds hobby microcontrollers to the parts library and the app as a
new part kind, `mcu`: **every in-stock 8-bit AVR** Microchip sells through
DigiKey (classic ATmega and ATtiny, AT90, tinyAVR 0/1/2, megaAVR 0, AVR
DA/DB/DD/DU, EA/EB, LA/SD and XMEGA), the **RP2040** (QFN-56), and the
Espressif **ESP32, ESP32-S2, -S3, -C3 and -C6 modules** (WROOM, WROVER,
MINI and SOLO). None of them is simulated. Every `mcu` device carries
`"simulated": false`, a new device key that any kind may use. The backend
refuses to build a circuit that holds an unsimulated part. In the app,
**Simulation › Run** (and ⇧⌘R) is disabled while any such part is on the
schematic, and the menu names it. The parts sit in the drawer under
**Microcontroller → <family> → <device>**.

The single behavioral outcome of plan-36: loading the part files gives the
baseline 27828 parts (P0, § Prerequisites) plus one `mcu` part per `add`
row of `planning/plan-36-data/candidates.tsv` (1521 in-scope rows at
planning time, before the datasheet verdicts). Each of those parts lists
with `|unsimulated`. `circuit_add_part` on any of them fails with `part
"ATMEGA328P-PU" is not simulated`. In the app, a schematic holding an
ATMEGA328P-PU has Simulation › Run disabled with the line `U1
(ATMEGA328P-PU) is not simulated` under it, and deleting the part enables
Run again. `ctest` passes T0 + 1 programs (`mcu_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | census refresh, part-by-part stock check of the 20 rows with no stock figure, datasheet downloads and placement, ordering-table verdicts (verbatim or per-datasheet regex), obsolete / NRND reading, `skipped-parts.md` / `obsolete-not-included.md` | large |
| B | pinouts: the ATDF extractor (Microchip device packs, Apache-2.0), the pad-name → role table, the sibling rule, the datasheet transcriptions for devices the packs do not cover, the ESP32 module and RP2040 pin tables, `pinouts.tsv` | large |
| C | footprints: Microchip dual-row and small packages (PDIP, SOIC, SSOP, TSSOP, DFN/UDFN, SOT-23-6) | large |
| D | footprints: Microchip quad and grid packages (TQFP, LQFP, QFN/VQFN, dual-row QFN, PLCC, BGA, WLCSP) and the RP2040 QFN-56 | large |
| E | footprints: the 13 Espressif module land patterns (`explicit`) | medium |
| F | backend: the `simulated` device key, the refusal in `circuit_add_part`, the `mcu` kind, `|unsimulated` in the listing, `mcu_test`, docs | large |
| G | parts: the generator, `Parts/mcu.json`, `datasheets.json` entries, goldens | large |
| H | app: `isSimulated`, Run disabled and its reason line, ⇧⌘R guard, facts, `kindName`, the `.microcontroller` reference category, Part Editor kind lists, help | large |
| I | landing tier | small |

Outcome of A: `candidates.tsv` has a verdict for each in-scope row:
`add`, `skip` with a reason, or `out` with a reason. Each `add` row names
its datasheet PDF, which is in the tree under `datasheets/mcu/<maker>/`,
and the package code its datasheet's ordering table gives it.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-36-data/` (written at planning time, 2026-11-12):
  `census/list.js`, `run.sh`, `list-avr.sh`, `atdfmatch.py`,
  `candidates.py`, `head.sh`; `raw/*.jsonl.gz` and `raw/*.err` (the
  listing); `candidates.tsv`; `datasheet-heads.tsv`; `packs.tsv`;
  `footprints-needed.txt`; `README.md`.
- `planning/plan-34-A-resistor-data.md` § 4 and `planning/completed/plan-29-A-diode-data.md`
  (the `fetch.py` / `textscan.py` / `sibling.py` / `place.py` pattern A reuses).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here and adds only its own rows.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p36 && cmake --build /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36 -j8` → `100% tests passed` | MET 2026-11-12 at 7ca1070: `100% tests passed, 0 tests failed out of 32`; **T0 = 32** (`grep -c '^logic_test' backend/CMakeLists.txt` → 32) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | MET: **P0 = 27828**, **R0 = 27828**, **F0 = 1386** (2026-11-12 at 7ca1070) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p36-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-12, working tree at 7ca1070 with the owner's uncommitted edits: `** BUILD SUCCEEDED **`) |
| No mcu kind yet | `grep -c 'PART_MCU' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/mcu.json` → no such file; `ls datasheets/mcu` → only the six spike PDFs (`atmega2560.pdf atmega328p.pdf atmega32u4.pdf atmega4809.pdf attiny13a.pdf attiny85.pdf`, fae501f) | MET (2026-11-12: 0, no file, six files) |
| No other open plan adds microcontrollers or `simulated` | `grep -l -i 'PART_MCU\|"simulated"\|Parts/mcu.json' planning/plan-*.md \| grep -v plan-36 \| wc -l` → 0 | MET (2026-11-12: 0) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc`. Phase 1 costs about 62 calls for the re-listing (the planning listing cost 59 for the AVR series, 2 for Espressif, 1 for Raspberry Pi: the `list.js:` lines of `raw/*.err`) plus 20 for the part-by-part check. The quota is shared with other sessions; plan-34's worktree session was planning on the same day | Start Phase 1 on a day whose first call reports ≥ 100 left. On 2026-11-12 planning ended at `78 left today` (the `rp2040.err` line), with credentials present |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `candidates.tsv` reflects a listing made on the day A ran. Every
  in-scope row (`scope` = `in`) has stock above 0 and status Active or Last
  Time Buy. The 20 rows with no top-level stock figure are confirmed part
  by part, or they are `out`, reason `stock check`.
- `datasheets/mcu/microchip/`, `datasheets/mcu/espressif/` and
  `datasheets/mcu/raspberry-pi/` hold every maker-site PDF an in-scope row
  needs. That was 193 distinct URLs at planning time: 189 answered a PDF
  (939 MB) and 4 answered 403 (§ 2). `datasheets/mcu/datasheets.tsv` maps
  each file to its URL, size and SHA-256.
- Every in-scope row has `verdict` `add`, `skip` or `out`, with a
  `reason` for each `skip` and `out`. Each `add` row has `pdf`, `hit` (`verbatim` or
  `regex`) and `package_code`, the package code its datasheet's ordering
  table prints for it, e.g. `32A`, `28P3`, `SS`, `STX`.
- `datasheets/skipped-parts.md` lists the `skip` rows. Any PDF whose every
  row is out for datasheet status is deleted and listed in
  `datasheets/obsolete-not-included.md`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No pinout work (B) and no footprint work (C–E). `package_code` is read
  here because the ordering table is where the verdict is made. C–E map
  codes to land patterns.
- No part outside the owner's scope (§ Decisions): no dev boards, no
  RP2350/RP2354, no bare ESP32 SoCs or PICO system-in-packages, no ESP32-C61
  / -H2 / ESP8684 modules, no 32-bit AVR (AVR32 UC3).
- `scripts/digikey.js` is not changed: the census needs series and
  parameter filters with full parameters per product, so `census/list.js`
  calls the API itself (the plan-34 `top.js` precedent).

## 2. Current State

The library has no microcontroller. `cpu.json` holds bus CPUs (6502,
65C02, 65C816, Z80); the W65C134S and W65C265S microcontrollers were
skipped by the owner (`datasheets/skipped-parts.md:15–16`). The folder
`datasheets/mcu/` holds six AVR datasheets for the datasheet-format spike
(fae501f, "no parts added to the catalog"), and they have no
`datasheets.json` entry (`grep -c 'atmega\|attiny' schematical/schematical/Parts/datasheets.json`
→ 0).

### Measured populations

All DigiKey numbers 2026-11-12: in stock (`MinimumQuantityAvailable` 1),
status Active or Last Time Buy (`StatusFilter` 0, 4), category 685
Microcontrollers (AVR and Raspberry Pi) or 872 RF Transceiver Modules and
Modems (Espressif). Maker ids: 150 Microchip Technology, 1904 Espressif
Systems, 1690 Raspberry Pi. Each listing command is a line of
`census/list-avr.sh` or `census/run.sh`. Its output is in
`planning/plan-36-data/raw/<label>.err`.

| What | Count | Command |
|---|---|---|
| Microchip microcontrollers in stock, all statuses | 8824; Core Processor `AVR` 1572 | `dk.sh --category 685 --manufacturer 150 --in-stock --pages 1 --json` → `ProductsCount`, `ParametricFilters` "Core Processor" |
| …of which 32-bit AVR (AVR32 UC3, out of scope) | 72 (series A0 6, A1 5, A3 10, A4 1, B 15, C 21, D 2, L 12) | same call, `Series` facet |
| 8-bit AVR series listed | 35 series ids (`census/list-avr.sh`); `AVR® ATA66 LIN-SBC` listed 0 Active/LTB | `raw/*.err` |
| 8-bit AVR parts, Active + LTB, in stock | **1468** (Active 1468, LTB 0) | `python3 -I census/candidates.py …` per-family lines summed; `atmega` alone 384, split by Package / Case into 17 pieces, none cut (`raw/atmega.err`) |
| …per family | ATmega 412, ATtiny 269, AVR Dx 268, tinyAVR 0/1/2 197, XMEGA 137, AVR Ex 72, megaAVR 0 58, AT90 41, AVR Lx/Sx 14 | `candidates.py` output |
| …devices | 302 (ATmega 88, ATtiny 42, Dx 44, XMEGA 44, tinyAVR 0/1/2 36, AT90 16, Ex 17, megaAVR 0 8, Lx/Sx 7) | `awk -F'\t' 'NR>1 && $5=="in"{print $3"\t"$4}' candidates.tsv \| sort -u \| cut -f1 \| uniq -c` |
| …automotive (Grade Automotive / AEC-Q100) | 6 | `awk -F'\t' 'NR>1 && $5=="in" && $13=="automotive"' candidates.tsv \| wc -l` |
| …with no top-level stock figure | 20 (`ATTINY44A-CCU`, `ATMEGA165PA-MU`, `AT90PWM81-16SN`, …) | `candidates.py` → `in-scope rows with no top-level stock` |
| …reel twins (base part number also listed) | 404 | an `R`/`T`/`TR`-suffix row whose base number is a row too; kept, the gate.json precedent (`SN74HC00D`, `…DR`, `…DT`) |
| Espressif modules in stock | 67: **51 in scope**, 16 out (6 bare SoCs, 4 PICO SiPs / PICO-MINI, 3 ESP8684, 2 ESP32-C61, 1 ESP32-H2) | `raw/espressif.err`; `candidates.py` |
| …in-scope module devices | 13: ESP32-WROOM-32 (4), -WROOM-DA (2), -WROVER (5), -MINI-1 (3), S2-SOLO (1), S2-SOLO-2 (4), S2-MINI-2 (4), S3-WROOM-1 (11), S3-MINI-1 (3), C3-WROOM-02 (2), C3-MINI-1 (4), C6-WROOM-1 (4), C6-MINI-1 (4) | `awk -F'\t' 'NR>1 && $5=="in" && $3=="ESP32"{print $4}' candidates.tsv \| sort \| uniq -c` |
| Raspberry Pi in category 685 | 10: **2 in scope** (`SC0914(13)` 121607, `SC0914(7)` 28568), 8 RP2350 / RP2354 out | `raw/rp2040.err`; `candidates.py` |
| **In-scope rows** | **1521** (AVR 1468, ESP32 51, RP2040 2); **316 devices** | `candidates.py` → `rows 1545 in 1521 devices in 316` |
| Distinct datasheet URLs, all listed rows | 193; 189 answer `200 application/pdf` (939.48 MB), 4 answer 403 | `census/head.sh` over the URLs → `datasheet-heads.tsv` |
| …the 4 that answer 403 | `AVR64DD-14-20-Prelim-DataSheet-DS40002444.pdf` (ww1), `ATtiny1614-16-17-DataSheet-DS40002204A.pdf` (www.microchip.com), and two product pages (`/en-us/product/ATMEGA1284P-AUR`, `ATTINY44A-MU`) | `awk -F'\t' '$1!=200' datasheet-heads.tsv` |
| Rows with no datasheet URL or a non-maker host | 12 none, 4 `mm.digikey.com`, 1 `cdn-shop.adafruit.com` (AVR); 1 none (`ESP32-S3-MINI-1-N4R2`), 1 `mm.digikey.com` (`ESP8684-MINI-1-H4X`, out) | host count of `datasheet_url` |
| DigiKey calls the planning census cost | 74 (152 → 78 left) | the first and last `left today` figures (`q1.err`, `rp2040.err`) |

### Verified properties

- **Ordering codes print verbatim in the classic datasheets.** `pdftotext
  -layout` of `ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf` (653
  pages, 33319446 bytes, bare `curl -sL` → `%PDF-`) contains each of the 12
  listed ATMEGA328P rows (`ATmega328P-AU`, `-AUR`, `-AN`, `-ANR`, `-MU`,
  `-MUR`, `-MN`, `-MNR`, `-MMH`, `-MMHR`, `-PU`, `-PN`), case aside. The
  newer datasheets (AVR Dx/Ex, tinyAVR 2) print a "Product Identification
  System" pattern (`AVR128DA28T-I/SS`) instead. Those get a regex per
  datasheet (§ 3 rule 5), as in plan-34-B.
- **Downloads.** ww1.microchip.com, espressif.com /
  documentation.espressif.com and datasheets.raspberrypi.com answer a bare
  `curl -sL` with a PDF (`datasheet-heads.tsv`; `rp2040-datasheet.pdf`
  5301205 bytes, `esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf`
  1230114 bytes, both `%PDF-`).
- **The RP2040 ordering codes are in its datasheet**: `rp2040-datasheet.pdf`
  prints `SC0914(7)` (7" reel of 500) and `SC0914(13)` (13" reel of 3,400)
  (`pdftotext -layout`, lines 47731 and 47734). The part names keep the
  parentheses. The loader only rejects empty and duplicate names
  (`backend/src/library/library.c:618, 625`).
- **Espressif pin tables are text.** `esp32-wroom-32e…datasheet_en.pdf`
  Table 3 "Pin Definitions" extracts as `Name  No.  Type  Function` rows
  (`GND 1 P`, `3V3 2 P`, `EN 3 I`, `IO32 8 I/O`, `NC 17 -`). Footnote marks
  run into names (`IO163` is IO16 with note 3). B reads them.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` only when all of these hold. Any other outcome is
`out` or `skip`, with the reason in `reason`.

1. **In stock and Active or Last Time Buy** at DigiKey on the day of
   Phase 1's listing. `list.js` asks for statuses 0 and 4 with
   `MinimumQuantityAvailable` 1. A row whose product has no top-level
   `QuantityAvailable` (20 at planning time) is looked up by its own part
   number (`dk.sh --manufacturer <id> --in-stock --limit 5 --json "<mpn>"`).
   It stays only when a product with exactly that
   `ManufacturerProductNumber` has `QuantityAvailable` above 0 or a
   `ProductVariations` entry with `QuantityAvailableforPackageType` above
   0; otherwise `out`, reason `stock check`.
2. **Scope** (`candidates.py`): 8-bit AVR series only (the 35 of
   `list-avr.sh`); Espressif ESP32 / S2 / S3 / C3 / C6 WROOM, WROVER, MINI
   and SOLO modules (`ESP_IN` in `candidates.py`); Raspberry Pi
   `SC0914(…)`. Everything else is `out` with the reason `candidates.py`
   writes. Automotive rows stay in and get the `automotive` flag (G).
3. **A maker datasheet** (Phase 3): the row's URL is on the maker's own
   site (`microchip.com`, `espressif.com`, `documentation.espressif.com`,
   `raspberrypi.com`) and downloads as a PDF. A Microchip URL that answers
   403 is retried on `ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/<file>`
   with the same file name. A product-page URL (`/en-us/product/<PART>`), a
   distributor copy, or no URL at all takes a downloaded same-maker PDF
   that prints the part's device name in its title and its part number per
   rule 5 (`sibling.py`). If none does, the row is `skip`, reason `no
   datasheet on the maker's site`.
4. **Not marked obsolete, discontinued, NRND or end of life** by the
   datasheet (`textscan.py` sets `dead` = 1 when the ordering-table page
   or the first page prints one of those words next to the part's device
   name; Phase 4 reads every `dead` = 1 row and decides it by hand).
5. **In the datasheet's ordering table**: the part number printed
   verbatim (case-insensitive, token boundary), or a full match of the
   datasheet's ordering-code regex, written in `specs.tsv` (one row per
   PDF: `pdf`, `mpn_regex`, `package_field`, `note`). The regex's named
   group `pkg` captures the package code, e.g. `-(?P<temp>[IEM])/(?P<pkg>SS|SO|SP|STX|PT|RXB|6LX)`
   for AVR Dx. A verbatim hit takes its package code from the ordering
   table's row: the table's "Package Type" column, which `textscan.py`
   reads off the same line. Otherwise the row is `skip`, reason `not in
   the datasheet's ordering table`.

## 4. Datasheet placement

`census/place.py` copies each needed PDF from `/tmp/p36/pdf/<md5 of the
URL>.pdf` to `datasheets/mcu/<maker>/<name>.pdf`, where `<maker>` is
`microchip`, `espressif` or `raspberry-pi`. `<name>` is the URL path's
last segment with the query dropped, `%20` → `-`, `.pdf` appended when
missing, and characters outside `A-Za-z0-9._-` → `-`. A name clash gets
`-<first 6 hex of the md5>`. Files with the same SHA-256 under two URLs
share one file. A PDF whose SHA-256 equals one of the six spike PDFs in
`datasheets/mcu/` is not copied again; its row points at the existing
file. `datasheets/mcu/datasheets.tsv` (`path url bytes sha256`) is written
beside them. The `datasheets.json` id of a file is its `<name>` in lower
case without `.pdf` (G writes the entries).

## 5. Scripts

- `/tmp/p36/dk.sh` (not in the tree): `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`,
  then `exec node /Users/justinzaun/Development/logic/scripts/digikey.js "$@"`.
- In the tree, from planning: `census/list.js`, `run.sh`, `list-avr.sh`,
  `atdfmatch.py`, `candidates.py`, `head.sh` (§ 2 commands). `run.sh` and
  `list-avr.sh` write to `/tmp/p36/raw` by default and skip a piece whose
  `.done` exists, so a run the quota cuts goes on the next day.
- Written in this letter, adapted from `planning/plan-29-data/census/`:
  `census/stock-check.sh` (§ 3 rule 1), `census/fetch.py` (bare `curl -sL`,
  § 3 rule 3's 403 retry), `census/textscan.py` (`pdf`, `hit`, `dead`,
  `package_code` for verbatim hits), `census/sibling.py`, `census/place.py`
  (§ 4), `census/verdicts.py` (applies `specs.tsv` and the rules, fills
  `verdict` and `reason`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Refresh the listing and check the 20 (one quota day)

- [ ] On a day with ≥ 100 calls left: `rm -rf /tmp/p36/raw && mkdir -p
      /tmp/p36/raw && planning/plan-36-data/census/list-avr.sh /tmp/p36/raw
      && planning/plan-36-data/census/run.sh /tmp/p36/raw 872 1904 -
      espressif && planning/plan-36-data/census/run.sh /tmp/p36/raw 685
      1690 - rp2040` (fresh stock; 62 calls at planning time).
- [ ] Download the device packs listed in `planning/plan-36-data/packs.tsv`
      to `/tmp/p36/packs/` and check each SHA-256. Unzip `atdf/*` into
      `/tmp/p36/packs/atdf/` (353 files at planning time). `candidates.py`
      needs them for the `atdf_*` columns. A pack whose SHA differs (a
      newer pack replaced it) is fetched at the version `packs.tsv` names.
      The URLs are versioned, and Microchip keeps old versions
      (`index.html` lists 2.0.2 onwards).
- [ ] `python3 -I planning/plan-36-data/census/candidates.py /tmp/p36/raw
      /tmp/p36/packs/atdf /tmp/p36/cand.tsv`. Compare its counts with § 2,
      and record a different count in § Corrections with the date.
- [ ] Write `census/stock-check.sh` (§ 3 rule 1). Run it on the in-scope
      rows with an empty `stock` (20 at planning time) and write
      `planning/plan-36-data/stock-check.tsv` (`mpn found stock date`).
      `candidates.py --stock <tsv>` marks a `found` = 0 row `out`, reason
      `stock check`.

Acceptance: a same-day listing, and every in-scope row has a stock figure above 0.
  Check: `awk -F'\t' 'NR>1 && $5=="in" && ($11=="" || $11==0)' /tmp/p36/cand.tsv | wc -l` → 0 after `--stock` (est. 1 quota day: about 82 calls).
Commit: —

### Phase 2 — Downloads

- [ ] `census/fetch.py /tmp/p36/cand.tsv /tmp/p36/pdf`: every distinct URL
      of the in-scope rows (193 at planning time, all rows), bare `curl
      -sL`, keep a file only if it starts `%PDF-` and `pdfinfo` reads it.
      Use § 3 rule 3's retry for a Microchip 403.
- [ ] `census/textscan.py`: `pdftotext -layout` each PDF once into
      `/tmp/p36/txt/`, then fill `pdf`, `hit`, `dead` and (verbatim hits)
      `package_code` per row.
- [ ] `census/sibling.py`: a row with no PDF, or a non-maker one, gets a
      downloaded same-maker PDF that prints its device name and its part
      number (§ 3 rule 3).

Acceptance: every in-scope row has a PDF or a recorded reason.
  Check: `awk -F'\t' 'NR>1 && $5=="in" && $20=="" && $24!="skip"' /tmp/p36/cand.tsv | wc -l` → 0 (`$20` is `pdf`, `$24` is `verdict`; `sibling.py` sets `skip`, reason `no datasheet on the maker's site`, for a row it cannot place) (est. 25 min for 939 MB, then 10 min of `pdftotext`).
Commit: —

### Phase 3 — Ordering-code regexes

- [ ] `planning/plan-36-data/specs.tsv`: one row per PDF that some
      in-scope row without a verbatim hit uses. Each row gets an
      `mpn_regex` from the datasheet's "Ordering Information" / "Product
      Identification System" page, with the named group `pkg` and, where
      the scheme has one, `temp`. The `note` column gives the page, e.g.
      `AVR128DA28-32-48-64-DataSheet-DS40002183B.pdf p.581`.
- [ ] `census/verdicts.py`: `hit` = `regex` for a full match, plus
      `package_code` from `pkg`. Rows still without a hit are `skip`, reason
      `not in the datasheet's ordering table`.

Acceptance: every in-scope row with a PDF has a hit or a skip reason.
  Check: `python3 -I planning/plan-36-data/census/verdicts.py /tmp/p36/cand.tsv planning/plan-36-data/specs.tsv --report` → `0 undecided` (est. 3 min after the regexes; nothing smaller checks every row).
Commit: —

### Phase 4 — Dead rows, docs, placement

- [ ] Read each `dead` = 1 row's page, and record `out` (reason `datasheet:
      obsolete` / `NRND` / `discontinued`) or keep it in `specs.tsv`'s
      `note`. A PDF whose every row is `out` for datasheet status: delete
      it from `/tmp/p36/pdf` and add a line to
      `datasheets/obsolete-not-included.md` (PDF name, family `mcu` /
      Microcontroller / <family>).
- [ ] `datasheets/skipped-parts.md`: one line per (reason, PDF) of the
      `skip` rows, in that file's table format.
- [ ] `census/place.py` (§ 4). `git add` only `datasheets/mcu/` and the two
      `.md` files.
- [ ] Copy `/tmp/p36/cand.tsv` to `planning/plan-36-data/candidates.tsv`.
      Update `planning/plan-36-data/README.md` with the verdict counts per
      family.

Acceptance: every `add` row's PDF is in the tree, and nothing extra is.
  Check: `python3 -I planning/plan-36-data/census/place.py --verify planning/plan-36-data/candidates.tsv` → `0 missing, 0 unused` (est. 1 min).
Commit: —

## Validation Plan

- Tests: none (data only). The scripts check themselves (`--report`,
  `--verify`).
- Coverage check: Phase 3's `0 undecided` covers every in-scope row.
- Runtime proof: not applicable; no part is loaded.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: plan-36-I.

## Decisions

- **Scope** — decided by the owner, 2026-11-12: every in-stock **8-bit**
  AVR (classic megaAVR/tinyAVR, AT90, tinyAVR 0/1/2, megaAVR 0, AVR
  DA/DB/DD/DU/EA/EB, plus the LA, SD and XMEGA series DigiKey lists as
  8-bit AVR); the RP2040 chip; Espressif modules. No dev boards, no
  RP2350, no bare ESP32 SoCs. The author added two details: XMEGA counts as
  8-bit AVR (an 8-bit AVR core; DigiKey's core size "8/16-Bit"), and
  AVR32 UC3 does not (a 32-bit core).
- **Which Espressif modules** — author, from the owner's "ESP32 WROOM/WROVER,
  S2, S3, C3, C6 WROOM/MINI": the WROOM, WROVER, MINI and SOLO modules of
  the five families (SOLO is the S2's WROOM-class module). The PICO
  system-in-packages are chips (QFN), and PICO-MINI-02U is a PICO module,
  so both are out. C61, H2 and ESP8684 are other families, so they are out
  too.
- **Plan first** — decided by the owner, 2026-11-12: this plan is written
  and committed before any implementation.
- **Every in-stock part, reel twins included** — author: the owner chose
  "all in-stock", and the library already lists reel and cut-tape numbers
  as their own parts (gate.json: `SN74HC00D`, `SN74HC00DR`, `SN74HC00DT`).
- **The listing is refreshed in Phase 1** — author: stock moves, and the
  inclusion rule is "in stock" on the day the parts are added. The
  planning listing stays in `raw/` as the record the counts came from.

## Corrections

## Summary

A is data work. Its risk is the size of the datasheet set (189 PDFs,
939 MB) and the newer Microchip datasheets that print a code pattern
rather than full codes, which need hand-written regexes. Nothing in the
code moves.
