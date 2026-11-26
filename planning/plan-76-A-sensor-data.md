# plan-76-A: Sensors: listing, scope, per-part stock check

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-76 is A → B → C → D → E → F → G → H → I → J → K → L → M → N → O
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-76 adds board-mount sensor ICs to the parts library and the app: seven families, the four
makers with the most in-stock parts in each at DigiKey (planning census of 2026-11-25,
`planning/plan-76-data/README.md` § Plan 76), each maker capped at its 300 most-stocked in-scope
parts per family.

| Family | DigiKey categories | Makers (rank order) | Upper bound |
|---|---|---|---|
| temperature | 518 | **ADI** (Maxim 215 + ADI 109 = 324 → 300), **TI** 258, **Microchip** 181, **NXP** 27 | 766 |
| humidity | 529 | **Sensirion** 49, **Amphenol Telaire** 21, **TE Measurement Specialties** 13, **Innovative Sensor Technology** 9 | 92 |
| pressure | 512 | **Amphenol All Sensors** 411 → 300, **Honeywell** 406 → 300, **ST** 104, **Amphenol NovaSensor** 93 | 797 |
| motion | 515 + 555 + 567 | **ADI** 90, **ST** 28, **TE Measurement Specialties** 22, **Memsic** 10 | 150 |
| magnetic | 554 + 565 | **Allegro** 291, **TI** 282, **Diodes** 225, **Infineon** 127 | 925 |
| light | 536 | **ams OSRAM** 26, **Vishay** 22, **Broadcom** 12, **TI** 11 | 71 |
| current | 525 | **Allegro** 236, **LEM** 166, **TI** 108, **Melexis** 69 | 579 |
| **all** | | | **3,380** |

The upper bounds are the census facet counts (board mount, In Stock, Active, Marketplace
excluded), capped at 300. A's scope rules and stock check only lower them.

The sensors are facts only: a new kind `sensor` whose devices all say `"simulated": false` (owner
decision), so a schematic with one placed does not run, as with an `mcu` without a model.

The single behavioral outcome of plan-76: loading the part files gives the baseline parts plus one
part per `add` row of `planning/plan-76-data/candidates.tsv`; the app lists them under
**Sensors → Temperature / Humidity / Pressure / Motion / Magnetic / Light / Current**; a placed
temperature part shows kind name "Temperature sensor", its facts (measures, output, range, accuracy,
supply), and turns Run off; every part has a footprint whose pads match its datasheet's land
pattern; `ctest` passes T0 + 1 programs (`sensor_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing, scope verdicts, cap, per-part stock check | large |
| B | datasheet downloads, text verdicts, obsolete checks, `specs-<family>.tsv` skeletons | large |
| C | the specs rules and `specs-check76.py`; temperature specs, ADI and TI | large |
| D | temperature specs, Microchip and NXP; the temperature footprints | large |
| E | humidity and light: specs and footprints | large |
| F | pressure specs and footprints, Amphenol All Sensors and Honeywell | large |
| G | pressure specs and footprints, ST and NovaSensor | large |
| H | motion: specs and footprints | large |
| I | magnetic specs and footprints, Allegro and TI | large |
| J | magnetic specs and footprints, Diodes and Infineon | large |
| K | current: specs and footprints | large |
| L | backend: the `sensor` kind, `sensor_test`, docs (first code letter: waits for plan-68) | medium |
| M | parts: `gen-sensor.py`, `Parts/sensor.json` for all seven families, `datasheets.json`, goldens | large |
| N | app: details, facts, kind names, drawer, Part Editor kind, tests, harness | large |
| O | landing tier | small |

Outcome of A:

- `candidates.tsv` lists every part DigiKey shows for the seven families and the plan's makers,
  with a scope verdict.
- `stock-check.tsv` checks every in-scope row: DigiKey's own `In Stock` line above 0, and status
  Active or Last Time Buy.
- No (family, maker) has more than 300 `in` rows.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints,
  § `mcu` — the `"simulated": false` shape); `docs/backend.md` (§ Adding a kind, § goldens).
- `planning/plan-76-data/README.md` (the census of plans 76–82) and `planning/plan-76-data/plan-76.md`
  (this plan's files, the download probe).
- `planning/plan-69-data/census/` (`url.js`, `facets.sh`, `show.py`, `kw.js`): A calls them and
  does not change them.
- Precedents: `planning/plan-74-A-ethernet-data.md` (the listing, scope, cap and stock-check
  shape this letter copies); `planning/completed/plan-58-A-slide-switch-data.md` (the
  `In Stock`-line rule, retries); `planning/completed/plan-48-A-timer-opamp-data.md` (per-maker
  cap, reserve rows).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p76 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p76 -j8 && ctest --test-dir /tmp/logic-p76 -j8` → `100% tests passed` | MET (measured by the author at planning, 2026-11-25 at 686408f4: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | at planning (686408f4): **P0 = R0 = 40832**, **F0 = 3993**. D–K (footprints), L and M re-measure at their start and use their own |
| Nothing of plan-76 exists yet | `grep -c 'PART_SENSOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/sensor.json` → no such file; `ls datasheets/sensor` → no such directory | MET (2026-11-25: 0; no file; no directory) |
| No other open plan adds this kind or folder | `grep -l -i 'PART_SENSOR\|Parts/sensor\.json\|datasheets/sensor/' planning/plan-*.md \| grep -v plan-76 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 69–75 and 77–82 add other kinds at the same time: the lines several plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, `CMakeLists.txt`) are added only if absent and after whatever is last; each such step greps first (L Phase 1, N Phase 1) |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | a rule, applied per footprint in D–K (`fp-merge76.py` skips an id already present) |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/518" 10 'document.title'` → a title starting `Temperature Sensors - Analog and Digital Output` | MET at planning (2026-11-25: the census's 60 pages loaded; once a run of keyword loads brought the challenge back and it cleared within minutes). When the command prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| `lz-string` for the census URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls /tmp/lz/node_modules/lz-string` → a directory | MET at planning (2026-11-25) |
| The census excludes Marketplace stock | `python3 -I planning/plan-69-data/census/show.py planning/plan-76-data/raw/b518.json Exclude` → an `-9 Exclude` facet | MET (2026-11-25: every saved page was requested with `"-9":[1]`; § 2 Verified properties) |
| Tools | `python3 --version`, `node --version`, `pdftotext -v`, `pdfinfo -v` each exit 0 | MET (2026-11-25: all used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `planning/plan-76-data/candidates.tsv` has one row per part of the A listing (§ 3.1): the seven
  families, the plan's makers, DigiKey In Stock + Active + board mount, Marketplace excluded. Each
  row has `scope` = `in` or `out` with a reason (§ 3.2). No (family, maker) has more than 300 `in`
  rows: the 300 most-stocked stay `in`, and the rest are `out` with reason `cap`.
- `stock-check.tsv` has one row per `in` row: DigiKey's own `In Stock` quantity (> 0) and status.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No download (B).
- `scripts/digikey.js` and `planning/plan-69-data/census/` are not changed. AGENTS.md: stock is
  looked up in Safari.
- Out of scope, and never re-opened in later letters:
  - DigiKey categories other than the eight of the table (gas, color, proximity, flow, force,
    level, position/angle, photodiodes, thermistors, the industrial categories).
  - Chassis-mount, panel-mount, free-hanging (wire-lead) and housed industrial sensors: the
    board-mount filter of the census (`planning/plan-76-data/README.md`, "Board mount").
  - Makers outside the table, including Bosch Sensortec and TDK InvenSense (motion ranks 5th and
    7th: 8 and 7 board-mount rows).
  - Development boards and breakout modules.
- DigiKey's legacy labels (National Semiconductor, Fairchild, Intersil, Rochester Electronics) are
  not makers of this plan. "Analog Devices Inc." and "Analog Devices Inc./Maxim Integrated" are one
  maker, ADI (plan-48, plan-74).

## 2. Current State

There is no sensor kind and no sensor part in the library (`grep -c -i 'sensor'
backend/tests/golden/parts.txt` → 0, 2026-11-25; the word appears only in two USB-A connector
descriptions in `connector.json`, "sensor contact"). `planning/todo.md:150-152` asks for an I2C
sensor breakout and says it "Needs a sensor kind (none today)"; `planning/todo.md:224` lists 10
reference-board sensor parts, of which `TCN75AVOA713` and `VEML7700-TT` fall in plan-76's
families (`planning/plan-76-data/plan-76.md`).

Empty, untracked datasheet folders from older downloads exist and are not used by plan-76:
`datasheets/ambient-light-ir-uv-sensors`, `analog-and-digital-output`,
`angle-linear-position-measuring`, `gas-sensors` (`ls … | wc -l` → 0 each; `git ls-files … | wc -l`
→ 0 each, 2026-11-25). plan-76's PDFs go to `datasheets/sensor/<family>/<maker>/`.

### Measured populations

All DigiKey numbers are from 2026-11-25, In Stock + Active + board mount, Marketplace excluded
(`planning/plan-76-data/raw/`; each printed by
`python3 -I planning/plan-69-data/census/show.py planning/plan-76-data/raw/<file>.json Manufacturer`,
and the multi-category families summed by maker label).

| Family | Rows | Pages over 100 rows for one maker (A splits them, § 3.1) |
|---|---|---|
| temperature (`b518`) | 907; ADI 324 (Maxim label 215, ADI label 109), TI 258, Microchip 181, NXP 27 | ADI-Maxim 215, ADI 109, TI 258, Microchip 181 |
| humidity (`b529`) | 104; Sensirion 49, Telaire 21, TE 13, IST 9 | none |
| pressure (`b512`) | 1,230; All Sensors 411, Honeywell 406, ST 104, NovaSensor 93 | All Sensors, Honeywell, ST |
| motion (`b515`, `m555`, `b567`) | 133 + 18 + 42; ADI 68 + 13 + 9, ST 15 + 2 + 11, TE 22 + 0 + 0, Memsic 10 + 0 + 0 | none |
| magnetic (`b554`, `b565`) | 372 + 1,012; Allegro 83 + 208, TI 121 + 161, Diodes 30 + 195, Infineon 37 + 90 | Allegro 565, TI 554, TI 565, Diodes 565 |
| light (`b536`) | 94; OSRAM 26, Vishay 22, Broadcom 12, TI 11 | none |
| current (`b525`) | 901; Allegro 236, LEM 166, TI 108, Melexis 69 | Allegro, LEM, TI |

Motion sum check: `python3 -I` over the three files' Manufacturer facets → `ADI 90, ST 28, TE 22,
Memsic 10, Bosch 8, Murata 8, TDK InvenSense 7` (2026-11-25). Magnetic: `Allegro 291, TI 282,
Diodes 225, Infineon 127, Melexis 112, ABLIC 112` (Melexis and ABLIC tie for 5th, out).

Tie at 4th in temperature: NXP 27 and Melexis 27. Summed `In Stock` over each maker's 27 rows
(`raw/t568.json`, `raw/t413.json`, product rows kept): NXP 244,014, Melexis 30,016. NXP is in.

Package mix (top 6 `Package / Case` per family, `show.py … "Package / Case"`):
temperature 8-TSSOP/MSOP 141, 8-SOIC 121, TO-92-3 59, SC-70-5 34, SOT-23-3 34, 10-MSOP 33;
pressure 6-SMD top port 121, 4-SIP module 95, 8-SMD J-lead dual ports 77, 8-DIP top port 76;
motion (515) 14-VFLGA 16, 12-VFLGA 13, 14-CLCC 12; magnetic (565) SOT-23-3 392, SOT-23W 79,
TO-92-3 short 79, 3-SIP 67; current 8-SOIC 232, Module 151, 16-SOIC wide 116.

### Verified properties

- **The Marketplace exclusion removes Marketplace-only rows.** `raw/m105.json`'s first 100 rows
  show 98 with only an `In Stock` line, 1 `In Stock` + `Alternate Stock`, 1 `In Stock` +
  `Factory`. A still reads every row's own `In Stock` line (§ 3.3).
- **A census page holds at most 100 products** (`facets.sh` saves the page's own `__NEXT_DATA__`).
- **Gyroscopes (555) has no Mounting Type facet**: the board-mount request returns 0 rows
  (`b555` empty); its 18 rows are all ICs (`show.py raw/m555.json "Package / Case"`: 32-BFCBGA 10,
  16-TFLGA 2, 16-WFLGA module 2, 20-BFLGA 2, 16-SOIC 1, 24-SMD module 1). A lists 555 without the
  Mounting Type filter.
- **ST's pressure rows include NXP's former MEMS pressure sensors** (36 of the first 42 rows of
  `b512` for ST link `www.nxp.com` datasheets, e.g. MPX/MPL parts). DigiKey's maker label is ST;
  so is the plan's.

## 3. Design

### 3.1 The listing (`census/list76.sh`, `census/split76.py`, `census/build76.py`)

One page per (family, category, maker, piece), saved to
`planning/plan-76-data/raw-A/<family>_<cat>_<maker>_<piece>.json` by
`planning/plan-69-data/census/facets.sh` with the extra filters `{"-9":[1], "69":[409393,411897,409430],
"-1":[<maker ids>]}` (no `69` for 555). Maker ids (the census facets): ADI 505 and 175, TI 296,
Microchip 150, NXP 568, Sensirion 1649, Telaire 600370, TE MEAS 223, IST 2952, All Sensors 442,
Honeywell 480, ST 497, NovaSensor 600369, Memsic 1267, Allegro 620, Diodes 31, Infineon 448,
ams OSRAM 475, Vishay 751, Broadcom 516, LEM 398, Melexis 413.

`split76.py` reads a page's `productCount`. A page over 100 is split by `Package / Case` (`16`),
each option its own piece; a piece still over 100 is split again by `Series` (`-4`). Pieces are
listed until each is under 100. `check76.py` checks a saved page: rows present, every row of the
requested maker, row count equal to `productCount`. `list76.sh` retries a page up to 5 times when
the check fails (plan-58-A's precedent: Safari sometimes returns another tab).

`build76.py` writes `candidates.tsv`, one row per distinct (family, maker, part number):

`family  maker  part  stock  status  digikey_id  category  description  package  supplier_package
output  sensor_type  datasheet_url  scope  reason  verdict  verdict_reason  pdf  stock_checked`

`family` is `temperature`, `humidity`, `pressure`, `motion`, `magnetic`, `light` or `current`;
`maker` is the slug `adi`, `ti`, `microchip`, `nxp`, `sensirion`, `telaire`, `te`, `ist`,
`allsensors`, `honeywell`, `st`, `novasensor`, `memsic`, `allegro`, `diodes`, `infineon`, `osram`,
`vishay`, `broadcom`, `lem` or `melexis` (the datasheet folder names). `stock` is the `In Stock`
line only (0 when absent). `output` is DigiKey's Output Type (`41`); `sensor_type` is Sensor Type
(`345`) or Type (`183`), whichever the category has.

### 3.2 Scope (`build76.py`)

`out` reasons, the first that applies:

1. `no DigiKey stock`: `stock` = 0.
2. `status`: not Active or Last Time Buy.
3. `not a part`: a kit, an evaluation board, a cable or an accessory (description `KIT`, `EVAL`,
   `BOARD`, `CABLE`, `ACCESSORY`, or DigiKey's category of the row not one of the eight).
4. `cap`: beyond the (family, maker)'s 300 most-stocked `in` rows (by `stock`, ties by part
   number). Only ADI temperature (324), All Sensors pressure (411) and Honeywell pressure (406) can
   reach it at planning counts.

### 3.3 Stock check (`stock-check.tsv`)

`build76.py`'s `stock` is the listing's own `In Stock` line, read part by part from the saved
page's `qtyAvailable` cell. The check is that line, not the facet. Columns `family maker part
in_stock status checked` (the date). A row whose cell shows no `In Stock` line, or shows 0, is
`out` (`no DigiKey stock`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Listing tools

- [ ] `planning/plan-76-data/census/list76.sh`, `split76.py`, `check76.py`, `build76.py` per
      § 3.1–3.3.
- [ ] Dry run on one family: humidity (4 makers, every page under 100).

Acceptance: the humidity listing builds and verifies.
  Check: `planning/plan-76-data/census/list76.sh humidity && python3 -I planning/plan-76-data/census/build76.py --verify planning/plan-76-data/raw-A planning/plan-76-data/candidates.tsv humidity` → `pages ok, cap ok` and `humidity in` ≤ 92 (est. 3 min: 4 Safari pages).
  Result:
Commit:

### Phase 2 — The full listing

- [ ] Run `list76.sh` for the other six families into `planning/plan-76-data/raw-A/`; write
      `candidates.tsv` and `stock-check.tsv` for all seven.
- [ ] Record in this section the counts by family, maker and `scope` / `reason`
      (`awk -F'\t' 'NR>1{print $1, $2, $14, $15}' planning/plan-76-data/candidates.tsv | sort | uniq -c`).

Acceptance: every page of the listing has fewer than 100 rows and matches its request; no
(family, maker) has more than 300 `in` rows; the `in` total is at most 3,380.
  Check: `python3 -I planning/plan-76-data/census/build76.py --verify planning/plan-76-data/raw-A planning/plan-76-data/candidates.tsv` → `pages ok, cap ok`, and per-family `in` counts no higher than § 2's upper bounds (est. 40 min: about 70 Safari pages, one Cloudflare tick expected).
  Result:
Commit:

## Validation Plan

- A's checks are its two acceptance commands. Nothing in `backend/` or the app changes.
- Final gate: plan-76-O.

## Decisions

- Owner, 2026-11-25: **board-mount sensor ICs in seven families**, the top 4 makers per family by
  DigiKey in-stock count, 300 most-stocked each, one plan with a letter (or two) per family.
- Owner, 2026-11-25: **sensors are facts only** (`"simulated": false`); no register model, no
  value set in the app.
- Owner, 2026-11-25 (AGENTS.md): only parts DigiKey itself has in stock; Marketplace and
  Factory-only stock does not count.
- **backend and app letters wait for plan-68** (owner, 2026-11-25, for plans 69–75; the same rule
  holds here): A–K (data, datasheets, specs, footprints) run now; L (the first code
  letter) lists plan-68 complete as a prerequisite, and every code letter meets plan-68's size and
  coverage checks.
- Author: **the families are DigiKey's categories**, grouped where one hobby function spans
  several (motion = accelerometers + gyroscopes + IMUs; magnetic = linear/compass ICs + solid-state
  switches). A part's family is its category's, so no part is in two families.
- Author: **NXP is temperature's 4th maker**: tied with Melexis at 27 rows, NXP has 8× the
  summed stock (244,014 vs 30,016).
- Author: **Amphenol's divisions are separate makers** (All Sensors, NovaSensor, Telaire), as
  DigiKey lists them: each has its own datasheets, part numbering and site.
- Author: **gyroscopes are listed without the board-mount filter**: 555 has no Mounting Type
  facet, and all 18 of its rows are ICs.

## Corrections

## Summary

A lists the seven sensor families through Safari, keeps each maker's 300 most-stocked in-scope
parts, and checks each one's own DigiKey stock line. B takes `candidates.tsv` from here.
