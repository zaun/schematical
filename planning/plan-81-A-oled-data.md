# plan-81-A: OLED displays: listing, stock check, datasheets, verdicts, specs

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-81 is A → B → C → D → E → F → G
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-81 adds OLED display modules to the parts library and the app: the in-stock graphic and
character OLED modules of the four makers with the most such parts at DigiKey (planning census
2026-11-25, `planning/plan-76-data/README.md` § Plan 81):

| Maker | Graphic (DigiKey 107) | Character (DigiKey 99) | Total |
|---|---|---|---|
| **Newhaven Display** | 28 | 19 | 47 |
| **Display Visions** | 17 | 7 | 24 |
| **Orient Display** | 13 | 0 | 13 |
| **DLC Display** | 11 | 0 | 11 |

Each maker is capped at its 300 most-stocked parts (owner rule for plans 76–82); no maker comes near
it, so the cap removes nothing. Where each module goes:

| Target | Which modules | Simulation |
|---|---|---|
| `oled`, simulated | graphic modules whose controller is of the **SSD1306 class**: 1-bit monochrome, page-addressed GDDRAM (SSD1306, SSD1306B/BZ, SSD1305, SSD1309, SSD1315, SH1106, SH1107, CH1116) | the controller's command set on the interface its straps select (C, D) |
| `oled`, facts only | graphic modules with any other controller (greyscale or colour: SSD1322, SSD1327, SSD1331, SSD1333, SSD1351, SSD1353, SSD1357, SSD1317, SSD1311, SH8601, …), modules with their own processor (serial-command modules), and character modules whose only interface is I2C or SPI, or whose controller is the US2066 | none: `"simulated": false`, Run off, as an `mcu` without a model |
| `charlcd` | character modules whose shipped interface is the HD44780-style 6800 parallel bus (Newhaven `NHD-0xxx[KDS]ZW-…`, Display Visions `EA W…`) | the existing HD44780 model (plan-11) with a new emissive `panel` colour (B) |

The single behavioral outcome of plan-81:

- Loading the part files gives the baseline parts plus one part per `add` row of
  `planning/plan-81-data/verdicts.tsv`.
- The drawer lists them under **Displays & Indicators → OLED → Graphic / Character**.
- In play mode an AVR that runs the SSD1306 I2C init sequence and writes a test pattern to a
  placed `NHD-2.23-12832UCB3` (SSD1305, 128 × 32) shows that pattern, pixel for pixel, on the
  symbol, in the panel colour; an unwritten GDDRAM shows as X.
- A placed SSD1322 module shows kind name "OLED display" and turns Run off.
- A placed `NHD-0420DZW-AY5` runs as a character display in yellow.
- `ctest` passes T0 + 1 programs (`oled_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing, scope, stock check, datasheets, verdicts, `specs.tsv`, `footprints-needed.txt` | large |
| B | backend: the `oled` kind (facts and keys), `charlcd`'s OLED panels, `oled_test` (load cases), docs | medium |
| C | the SSD1306-class model: GDDRAM, command set, profiles, I2C and 4-wire SPI, reset, `OledState`, `circuit_oled_state` | large |
| D | the 3-wire SPI, 8080 and 6800 interfaces, parallel reads, timing checks | large |
| E | footprints, `gen-oled.py`, `Parts/oled.json`, new `charlcd.json` devices, `datasheets.json`, goldens | large |
| F | app: state polling, `OLEDSymbolView`, facts, kind name, drawer, Part Editor kind, test classes, harness | large |
| G | landing tier | small |

Outcome of A:

- `planning/plan-81-data/candidates.tsv` lists the 95 rows of `census-rows.tsv` with a scope verdict.
- `stock-check.tsv` holds each `in` row's own `In Stock` line (> 0) and status.
- `datasheets/display-oled/{newhaven,displayvisions,orient,dlc,controllers}/` holds every PDF an `add`
  row cites, plus the controller datasheets C and D are written from; `datasheets.tsv` lists them.
- `verdicts.tsv` gives each `in` row `add` (with target) or `skip` (reason).
- `specs.tsv` has one row per (maker, datasheet, pinout) of the `add` rows, filled in (pins, bus,
  straps, glass, colour, supply, drawing). `footprints-needed.txt` lists the footprints E draws.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints,
  § `charlcd`, § `mcu` (the `"simulated": false` shape)); `docs/backend.md` (§ Adding a kind, § goldens).
- `planning/plan-76-data/README.md` (the census of plans 76–82) and `planning/plan-69-data/census/`
  (the census tools; A uses them unchanged).
- `planning/plan-81-data/README.md` (this plan's files and the planning measurements).
- Precedents: `planning/plan-74-A-ethernet-data.md` (listing, stock check, verdicts, specs skeleton),
  `planning/completed/plan-11-*` (the character LCD modules: module drawings, `connectorFootprint`).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p81 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p81 -j8 && ctest --test-dir /tmp/logic-p81 -j8` → `100% tests passed` | MET (2026-11-25 at 686408f4, by the planner: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `… traces.txt`; `… footprints.txt` | at planning (686408f4): **P0 = R0 = 40832**, **F0 = 3993**. B, C, D, E re-measure at their start |
| charlcd baseline | `python3 -I -c "import json;d=json.load(open('schematical/schematical/Parts/charlcd.json'));print(len(d['devices']),sum(len(x['parts']) for x in d['devices']))"` | at planning: **72 devices, 72 parts** |
| Nothing of plan-81 exists yet | `grep -c 'PART_OLED' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/oled.json backend/src/models/oled.c` → no such file; `grep -c 'oled' backend/src/models/charlcd.c` → 0; `ls datasheets/display-oled` → no such directory | MET (2026-11-25: 0; no files; 0; no directory). `datasheets/lcd-oled-graphic/` exists, empty and untracked (`git ls-files datasheets/lcd-oled-graphic \| wc -l` → 0); plan-81 does not use it |
| No other open plan adds this kind or folder | `grep -l -i 'PART_OLED\|Parts/oled\.json\|datasheets/display-oled\|oled_state' planning/plan-*.md \| grep -v plan-81 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 69–80 and 82 add other kinds at the same time; the shared lines (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, listing arrays, the editor's kind lists, `CMakeLists.txt`) are added only if absent and after whatever is last (B Phase 1, F Phase 1) |
| Safari has a DigiKey page open (`safari-digikey-pdf.sh` opens its tabs in that window) | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/107" 10 'document.title'` → a title starting `Display Modules - LCD, OLED, Graphic` | MET at planning (2026-11-25). When it prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| Orient PDFs through Safari | `scripts/safari-digikey-pdf.sh https://www.orientdisplay.com/wp-content/uploads/2021/02/AOM12864A0-0.96WW-ANO.pdf /tmp/o.pdf && head -c5 /tmp/o.pdf` → `%PDF-` | MET at planning (477,365 B, 18 pages); curl gets 403 |
| DLC's slider check passed in Safari | `scripts/safari-digikey-pdf.sh "https://www.dlcdisplay.com/nportal/fwebapi/cms/lowcode/file/getFileById?id=1760131013944299520" /tmp/d.pdf && head -c5 /tmp/d.pdf` → `%PDF-` | NOT MET at planning (2026-11-25: HTTP 430, "Network Security Verification — drag slider to verify"). The owner opens `https://www.dlcdisplay.com/viewfilebizce/1760131013944299520/DLC0096DNOG-W-5.pdf` in the DigiKey window of Safari and drags the slider once; then the command is re-run |
| Tools | `pdftotext -v`, `pdfinfo -v`, `pdftoppm -v`, `python3 --version`, `node --version` exit 0 | MET (used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

The files of "Outcome of A" exist, with every `in` row checked in stock, every `add` row's PDF on
disk, and every `add` row's pinout, bus and glass written in `specs.tsv`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app (B–F do those).
- `scripts/digikey.js` and `planning/plan-69-data/census/` are not changed.
- Out of scope, never re-opened in later letters: LCD, TFT, e-paper and VFD modules; bare OLED
  glass without a controller; makers other than the four (Adafruit, DFRobot, 4D Systems, Seeed,
  SparkFun, Knitter, Lumex, Vishay, American Bright and Parallax rows of the census); touch panels
  as an input (a touch controller on a module is a fact only).

## 2. Current State

- No OLED part exists: `grep -c -i 'oled' backend/tests/golden/parts.txt` → 0 (2026-11-25).
- `charlcd` (`backend/src/models/charlcd.c`, 1248 lines) models HD44780-compatible character
  modules on a 4/8-bit parallel bus with E-edge strobes; its `panel` key accepts only
  `yellow-green`, `blue-negative`, `gray`, `white` (`charlcd.c:208-210`); the app's panel enum is
  `Backend/PartCatalog.swift:465-495`. Shipped: 72 devices, 72 parts, all Newhaven (25), Focus (32),
  Midas (13), Displaytech, Orient (1 each). Parts carry `footprint` (module outline + header, e.g.
  `LCM-NHD-0216BZ`) and `connectorFootprint` (the header alone, e.g. `HDR-1X16-P2.54`).
- No pixel display model exists; the only display-state hook is `lcd_state`
  (`backend/src/engine/sim.h:157-159`; `circuit_lcd_state`, `backend/src/circuit.c:779`), which C
  copies for `oled_state`.
- The SPI and I2C target front ends are `backend/src/models/serial_target.h` (`SerialSpi`, modes
  0–3, 8-bit bytes; `SerialI2c`, 7-bit address, open-drain SDA, address straps).

### Measured populations

| What | Count | Source |
|---|---|---|
| In-scope-maker rows | 95: Newhaven 47, Display Visions 24, Orient 13, DLC 11 | `cut -f2 planning/plan-81-data/census-rows.tsv \| sort \| uniq -c` |
| …graphic / character | 69 / 26 | `cut -f1 … \| sort \| uniq -c` → 69 `107`, 26 `99` |
| …DigiKey names an SSD1306-class controller | 20 (SSD1306 4, SSD1305 4, SSD1309 2, "SPD0301, SSD1305, SSD1309" 2, SH1107 2, SSD1315 1, SSD1306BZ 1, "SSD1306, SSD1315" 1, SH1106G 1, "SH1106, SSD1306" 1, CH1116 1) | `cut -f5 … \| sort \| uniq -c` |
| …DigiKey names another controller | 40 (SSD1322 15, US2066 7, SSD1351 7, SSD1333 3, SSD1353 2, SSD1331 2, SSD1357Z 1, SSD1317 1, SSD1311 1, SH8601 1) | same |
| …no controller in DigiKey's data (`-`) | 35 | same; A reads them from the PDFs (§ 3.3) |
| Datasheet hosts | newhavendisplay.com 47; lcd-module.de/.com/shop 24; orientdisplay.com 13; dlcdisplay.com 10; mm.digikey.com 1 | `planning/plan-81-data/README.md` § Planning measurements |

## 3. Design

### 3.1 Listing and stock (Phase 1)

`candidates.tsv` = `census-rows.tsv` (the planning census; no new listing pages are loaded: the two
raw pages hold all 123 OLED rows of both categories, and the 95 of the four makers) plus `scope` and
`reason`. Every row is `in` except: a part whose DigiKey description or datasheet makes it a bare
glass without controller (`out`, "glass only"), and a part that is a kit or an adapter board
(`out`, "accessory"). Stock (plan-74-A § 3.3's rule): the check is each row's own `In Stock` line
as the saved census page shows it in the product cell (`census-rows.tsv` column `in_stock`, read
2026-11-25), not the facet count. `stock-check.tsv` has columns `maker part in_stock status
checked`; every row of `census-rows.tsv` shows a line above 0 (`awk -F'\t' 'NR>1 && $4==0'
planning/plan-81-data/census-rows.tsv | wc -l` → 0 at planning), and every row is Active (the
census filtered `1989` = 0).

### 3.2 Datasheets (Phase 2)

Folders: `datasheets/display-oled/newhaven/`, `displayvisions/`, `orient/`, `dlc/`, and
`controllers/` (SSD1306, SSD1305, SSD1309, SH1106, SH1107, US2066, from the URLs of
`planning/plan-81-data/README.md`). Fetch rules, per host (measured at planning): Newhaven and
Display Visions with curl (Safari user agent); Orient with `scripts/safari-digikey-pdf.sh`; DLC
with `scripts/safari-digikey-pdf.sh` on the `getFileById?id=<id>` URL, the `<id>` being the
number in the census row's `viewfilebizce/<id>/` URL; the one `mm.digikey.com` copy with
`scripts/safari-digikey-pdf.sh`. Every file is checked to start `%PDF-`, and `datasheets.tsv`
records file, URL, bytes, SHA-256 and method. One PDF serves every part it covers (Display
Visions' `-6e` sheets cover several colour suffixes). Obsolete check (AGENTS.md): a PDF every one
of whose parts is obsolete is deleted and recorded in `datasheets/obsolete-not-included.md`
(family "Displays & Indicators / OLED").

### 3.3 Verdicts (Phase 3)

Per `in` row, from its datasheet. Columns of `verdicts.tsv` (E and G read them by number):
`maker  part  category  controller  verdict  target  reason`.


1. **Controller.** The controller the datasheet names (cover page, "Controller" line, or the
   command table's heading). The 35 rows with DigiKey `-` get it this way; a datasheet that names
   none and prints an HD44780-style instruction table (Newhaven `NHD-0xxxKZW/DZW/SZW`, as checked
   on `NHD-0420DZW-AY5`) is recorded as controller `WS0010-compatible`.
2. **Target**:
   - `oled-sim`: graphic, controller ∈ {SSD1306, SSD1306B, SSD1306BZ, SSD1305, SSD1309, SSD1315,
     SH1106, SH1106G, SH1107, CH1116}. A row DigiKey lists with two or three controllers
     ("SSD1306, SSD1315"; "SPD0301, SSD1305, SSD1309") takes the one its datasheet's command
     table is written for; a sheet that says the module ships with either takes the first
     SSD1306-class name it lists, and the part's `note` names the others.
   - `oled-facts`: graphic with any other controller; a module whose host interface is a command
     processor (Display Visions `EA PLUG…`, which DigiKey lists with "I2C, RS-232, SPI, USB");
     character modules whose controller is US2066 or SSD1311, or whose pinout carries no parallel
     data bus.
   - `charlcd`: character, HD44780-compatible instruction set, and a 6800 parallel bus (RS, R/W,
     E, DB0–DB7) on the pinout as shipped (Newhaven's default jumper J68).
3. **Verdict** `add`, or `skip` with a reason (no stock; no datasheet obtainable; obsolete;
   duplicate of another row's part number).

### 3.4 Specs (Phase 4)

One `specs.tsv` row per (maker, datasheet, pinout), read from the datasheet's pin table and
drawing:

`maker  datasheet  parts  target  controller  pins  bus  straps  glass  colour  supply  interfaces  drawing_page  footprint  connector_footprint`

- `pins`: `number:name:role` per pin, roles as `docs/parts-format.md` § Pinouts (supply pins
  `power` / `ground`; NC `nc`).
- `bus` (`oled-sim` rows): which module pins carry the controller's CS#, D/C#, RES#, R/W# (WR#),
  E (RD#) and D0–D7; `straps`: each BS pin either a module pin name or a fixed level the module
  ties it to (`BS0=0 BS1=1 BS2=0` for an I2C-only module), read from the module's interface
  table or schematic. `charlcd` rows fill the charlcd `bus`, `contrast` (`-` for an OLED, which
  has none) and `timing`/`exec` columns from the instruction and bus-timing tables.
- `glass`: width × height in pixels (or columns × rows of characters), the column offset into the
  controller's RAM (SH1106 on a 128-wide glass: 2), and the COM pins configuration the datasheet's
  initialization sequence sets (`DAh` value: `0x02` sequential, `0x12` alternative). The model
  renders a firmware value that differs from it by the controller's Table 10-3 rules (C § 3.4).
- `colour`: the emitted colour the datasheet gives (white, yellow, green, blue, red; a two-zone
  glass as `yellow:0-15,blue:16-63`).
- `supply`: logic VDD range (the `vcc`), and the panel VCC as a fact.
- `drawing_page`: the page of the mechanical drawing E draws the footprint from. `footprint` /
  `connector_footprint`: ids by the rule of E § 3.1; `footprints-needed.txt` lists every id not
  already in `footprints.json` (`grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` → 0).

### Rejected alternatives

- **Reloading DigiKey listing or product pages.** The two saved census pages already hold every
  OLED row of both categories with its own `In Stock` line (`o107` 96 rows, `o99` 27).
- **One target for all character OLEDs.** The US2066 adds an extended instruction set (RE/IS
  bits, the `0x2A`/`0x71`/`0x08` init sequence) that the HD44780 model would read as other
  instructions; those modules are facts only instead of a wrong simulation.

## 4. Phases

### Phase 1 — listing and stock

- [ ] Write `planning/plan-81-data/candidates.tsv` from `census-rows.tsv` (§ 3.1), scope and reason per row.
- [ ] Write `planning/plan-81-data/stock-check.tsv` from `census-rows.tsv` (§ 3.1).
- [ ] Write `planning/plan-81-data/check-specs.py` with its `--phase 1` checks.

**Acceptance:** `python3 -I planning/plan-81-data/check-specs.py --phase 1` (the check script,
started in this phase and extended in Phase 4) → `ok 95 rows, <n> in, <n> stock rows`: 95
candidates, one stock row per `in` row, every `in_stock` > 0. (≈ 1 s.)

Commit:

### Phase 2 — datasheets

- [ ] Fetch every PDF an `in` row cites into `datasheets/display-oled/<maker>/` by § 3.2's host rules.
- [ ] Fetch the six controller datasheets into `datasheets/display-oled/controllers/`.
- [ ] Write `planning/plan-81-data/datasheets.tsv`; run the obsolete check.

**Acceptance:** `for f in datasheets/display-oled/*/*.pdf; do head -c5 "$f" | grep -q '%PDF-' || echo BAD $f; done` → no output, and every `in` row's datasheet file is in `datasheets.tsv` (`python3 -I` one-liner joining the two files → 0 missing). (≈ 1 min.)

Commit:

### Phase 3 — verdicts

- [ ] Write `planning/plan-81-data/verdicts.tsv` (§ 3.3) for every `in` row.
- [ ] Add a section "OLED displays (plan-81)" to `datasheets/skipped-parts.md` listing every `skip` row.

**Acceptance:** every `in` row has exactly one verdict, every `add` row a target in {`oled-sim`,
`oled-facts`, `charlcd`}, and `awk -F'\t' '$5=="add" && $6=="oled-sim"' verdicts.tsv` lists
`NHD-2.23-12832UCB3` (the Goal's example). (≈ 1 min.)

Commit:

### Phase 4 — specs and footprints list

- [ ] Fill `planning/plan-81-data/specs.tsv` (§ 3.4) for every `add` row.
- [ ] Extend `check-specs.py` with the `--phase 4` checks.
- [ ] Write `planning/plan-81-data/footprints-needed.txt`.

**Acceptance:** `python3 -I planning/plan-81-data/check-specs.py --phase 4` prints `ok`: every `add` part is in exactly one specs row; every `oled-sim` row has a `bus`
naming D0 and D1 (SCLK, SDIN), RES# or a fixed reset, and every BS strap; every row has a
`drawing_page` and a `footprint`. (≈ 10 s.)

Commit:

## 5. Validation Plan

A changes no code; its gate is the four acceptance checks above, re-run once at the end:
`python3 -I planning/plan-81-data/check-specs.py --phase 4` → `ok`, plus the Phase 2 `%PDF-` loop → no
output. The landing tier of plan-81 is G.

## Decisions

- Owner (2026-11-25): **OLED displays are their own plan**, one of seven hobby part types (plans 76–82).
- Owner (2026-11-25): **top 4 makers, at most 300 parts each**, by DigiKey in-stock count. For OLED
  modules across graphic (107) and character (99): Newhaven, Display Visions, Orient, DLC.
- Owner (2026-11-25): **simulate the SSD1306 class** (monochrome page-addressed controllers);
  every other OLED is facts only.
- Owner rule (AGENTS.md): only DigiKey's own In Stock line counts; Marketplace rows are excluded
  (the census filter `-9` = 1).
- Author: **one `oled` kind for simulated and facts devices**, as `mcu` holds both (B § Rejected).
- Author: **character OLEDs with the 6800 parallel bus go to `charlcd`** with an `oled-*` panel;
  US2066 / SSD1311 and serial-only character modules are `oled` facts (§ 3.3, § Rejected).
- Author: **the module as shipped defines its pinout** (Newhaven's default jumper J68; solder
  jumpers that change the interface are a fact in the part's `note`).

## Corrections

## Summary

A turns the planning census into checked candidates, PDFs, verdicts and specs; B–F build on them.
