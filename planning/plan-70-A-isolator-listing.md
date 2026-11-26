# plan-70-A: Optocouplers and digital isolators: the Safari listing, the per-part In Stock line, the top-200 pick

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-70 runs A → B → C → D → E → F → G → H → I → J.
Effort: large (3h–1d, plus the owner's Cloudflare ticks)
Depends on: nothing (§ Prerequisites)

plan-70 adds **optocouplers** (transistor, Darlington, triac and logic
outputs) and **digital isolators** to the parts library and the app. They go
into a new kind, `isolator`. It takes the top three makers of each type at
DigiKey by in-stock count, each capped at its 200 most-stocked parts (owner
decision 1, `planning/plan-69-data/census/README.md`). Digital isolators are
in scope by owner decision 2.

| Type | DigiKey category | Makers (rank order, census counts) | In scope |
|---|---|---|---|
| `transistor` | Optoisolators – Transistor, Photovoltaic Output (903) | **Isocom** 930, **Vishay** 671, **onsemi** 505 | Output Type Transistor, Transistor with Base, Darlington, Darlington with Base; Input Type DC or AC/DC; surface mount or through hole |
| `logic` | Optoisolators – Logic Output (902) | **Broadcom** 376, **Toshiba** 159, **Renesas** 118 | Output Type Push-Pull/Totem Pole, Open Collector, Open Collector Schottky Clamped, Open Drain, Tri-State, CMOS, Push-Pull; any input type |
| `triac` | Optoisolators – Triac, SCR Output (904) | **onsemi** 197, **Vishay** 127, **Würth Elektronik** 102 | Output Type Triac, Triac Power |
| `digital` | Digital Isolators (901) | **Analog Devices** 944 (774 + Maxim 170), **Texas Instruments** 330, **Skyworks** 321 | Channel Type Unidirectional; Isolated Power No; every Type except USB, LVDS, Digital Input Module and `-` |

In play mode an isolator is logic (owner decision 3). An LED input is
*driven* while its anode reads 1 and its cathode 0, as a relay coil is. A
driven LED closes the output transistor's or triac's contact, or sets a
logic output. A digital isolator copies each channel's input to its output
across the barrier while both sides are powered, and drives its default
level when the input side is not powered (G § 3).

The single behavioral outcome of plan-70: loading the part files gives the
baseline parts (P0, § Prerequisites), plus two generics (`GENERIC-OPTO`,
`GENERIC-ISOLATOR`), plus one part per `add` row of
`planning/plan-70-data/candidates.tsv`. That is at most **2,103** picked rows
before verdicts: transistor 600, logic 477, triac 426, digital 600 (§ 2).
The app lists them under **Interface → Optocoupler → Transistor / Darlington
/ Triac / Logic** and **Interface → Digital isolator**. In play mode a
switch on 5 V, through a `GENERIC-RESISTOR`, into the LED of a placed
`GENERIC-OPTO` lights an LED wired to the opto's collector. A
`GENERIC-ISOLATOR` passes a switch's level from side 1 to side 2. With
side 1's supply removed, the isolator drives its default level instead.
`ctest` passes T0 + 1 programs (`isolator_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | Safari listing of the 12 (type, maker) pieces, split under 100 rows; each part's own In Stock line; the top-200 pick | large + Cloudflare ticks |
| B | datasheet URLs, downloads, text scan, verdicts, placement, `specs.tsv` skeleton | large |
| C | specs: transistor- and triac-output optocouplers (pinouts, facts, ordering-key decisions); the shared spec tools | large |
| D | specs: logic-output optocouplers and digital isolators; the final pick, skipped parts, obsolete PDFs, the final `footprints-needed.txt` | large |
| E | footprints: optocoupler packages (DIP, gull-wing SMD, wide-lead, SOP, SO, LSOP, stretched SO) | large |
| F | footprints: digital-isolator packages (SOIC narrow and wide, SSOP, QSOP, leadless) | large |
| G | backend: the `isolator` kind and model (on plan-75's shared supply-domain loader), ERC pin types, `isolator_test`, docs | large |
| H | parts: the two generics, `isolator.json`, datasheets, footprints, goldens, the Part Editor's kind lists | large |
| I | app: details, facts, kind names, the drawer, the `isolator-app` harness | large |
| J | landing tier | small |

Outcome of A:

- `planning/plan-70-data/listing/` holds every scoped in-stock Active part
  of the 12 pieces. It has one saved page per (type, maker, split), each
  with fewer than 100 rows.
- `candidates.tsv` ranks each piece by the product's own `In Stock`
  quantity. It marks the first 200 (or all) rows `candidate` and the rest
  `reserve`. Every `candidate` and `reserve` row has its own `In Stock`
  quantity above 0. A listed part without one is `out`.

References:

- `AGENTS.md` (§ Datasheets: in stock, Safari); `planning/plan-69-data/census/README.md`
  (the census and the owner's decisions); `planning/plan-70-data/README.md`.
- `planning/completed/plan-58-A-slide-switch-data.md` and
  `planning/plan-58-data/census/` (`make-urls.js`, `list.sh`, `check.py`,
  `build.py`). These are the Safari listing scripts A copies and adapts; do
  not call them.
- `planning/completed/plan-48-A-timer-opamp-data.md` (the pick, reserve and
  refill rules, and the Rochester legacy-label rule).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p70 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p70 -j8 && ctest --test-dir /tmp/logic-p70 -j8` → `100% tests passed` | MET (2026-11-25 at 63e2f7d2: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | at planning (63e2f7d2): **P0 = 40832**, **R0 = 40832**, **F0 = 3993**, **E0 = 40832**; G, H re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p70-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-25 at 63e2f7d2: `** BUILD SUCCEEDED **`, run by the author: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO build`) |
| The kind does not exist | `grep -c 'PART_ISOLATOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/isolator.json datasheets/isolator` → no such file or directory | MET (2026-11-25: 0; both "No such file or directory") |
| No other open plan adds the `isolator` kind or these parts | `grep -l -E 'PART_ISOLATOR\|Parts/isolator\.json\|"kind": "isolator"\|datasheets/isolator' planning/plan-*.md \| grep -v plan-70 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 69 (`tvs`), 71 (`transceiver`), 72 (`ioexpander`, `i2cbus`), 73 (serial-kind USB bridges) and 74 (`ethernet`) are written the same day and add kinds. They touch the same shared lines (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, the drawer table in `docs/parts-format.md`, `erc_pins.c`). Each such step here adds its line only if absent, after whatever is last, and greps first (G Phase 1, H Phase 1, I Phase 1). The supply-domain loader is shared through plan-75 (a prerequisite of G), not moved by this plan |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | rule, applied per footprint in E and F |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/901" 10 'document.title'` → `Digital Isolators \| Integrated Circuits (ICs) \| DigiKey` | MET during the planning census (2026-11-25). About 100 page loads in an hour make Cloudflare challenge the window again (`Just a moment...`); then the owner ticks "Verify you are human" in the DigiKey tab once, and the command is re-run |
| lz-string for the filter URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls node_modules/lz-string` → the package | rule (installed per session; planning used a scratch copy) |
| Tools | `pdftotext -v`, `pdfinfo -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-70-data/listing/` has a saved page for every piece and
  split of § 3.2. Each page passes `check.py`: its rows match the request,
  and its `productCount` equals its row count, below 100.
- `candidates.tsv` has one row per listed part, `pick` = `candidate` for the
  200 most-stocked rows of each (type, maker) (all of them below 200), and
  `reserve` for the rest, in stock order.
- Every `candidate` and `reserve` row has `stock` > 0 and `status` Active
  (or Last Time Buy), read from its own product cell on the saved page.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No datasheet work: B downloads and reads the PDFs.
- The makers and the scope of § 1's table are not re-opened. A does not
  re-rank makers inside the scoped filter: the owner took the census's top
  three per category.
- `scripts/digikey.js` is not used (AGENTS.md: stock lookups go through Safari).
- Photovoltaic, MOSFET-output and logic-output parts of category 903,
  bidirectional and isolated-power digital isolators, and USB, LVDS and
  Digital Input Module isolators are out of scope (§ Decisions).

## 2. Current State

The library has no optocoupler or digital isolator
(`grep -c -E '^(4N25|4N35|PC817|LTV-817|6N137|HCPL|MOC30|ISO77|ADUM|SI86)' backend/tests/golden/parts.txt`
→ 0). The closest kind is `relay` (`backend/src/models/relay.c`), whose
`solid-state` type has input LEDs and contacts. Its 7 solid-state devices are
Panasonic PhotoMOS relays (`grep -c '"type": "solid-state"'
schematical/schematical/Parts/relay.json` → 7). The `buffer` kind's
`translator` model has the only two-supply `domains`
(`backend/src/models/buffer.c:load_domains`). The reference-board todo item
"optocoupler: 2 parts" (`planning/todo.md:232`) names `VO617A-9X017T`
(Vishay) and `TCS40DPR,LF` (`planning/plan-43-data/candidates.tsv:168-169`).
The second of these is a Toshiba magnetic sensor, not an optocoupler.

### Measured populations

All DigiKey numbers come from 2026-11-25, In Stock + Active, read from the
saved raw pages:
`python3 -I planning/plan-69-data/census/show.py planning/plan-70-data/raw/<file> <facet>`.

| What | Count | Command (`show.py` on) |
|---|---|---|
| Category 903 (transistor / photovoltaic output), all | **3,695** | `raw/c903.json` → `TOTAL` |
| …by Output Type | Transistor 2,352; Transistor with Base 686; Darlington 284; Darlington with Base 150 (in scope: **3,472**); Photovoltaic 54, Photovoltaic Linearized 45, Logic 27, MOSFET 19, `-` 3 (out) | `raw/c903.json "Output Type"` |
| …by Input Type, Mounting | DC 3,182, AC/DC 436; surface mount 2,366, through hole 1,242, DIN rail 3, `-` 9 | `raw/c903.json "Input Type" "Mounting Type"` |
| …makers | **Isocom 930, Vishay 671, onsemi 505**; Broadcom 361, Toshiba 305, Renesas 192, Lite-On 175 | `raw/c903.json Manufacturer` |
| Category 902 (logic output), all | **924** | `raw/c902.json` → `TOTAL` |
| …by Output Type | Push-Pull/Totem Pole 371; Open Collector Schottky Clamped 229; Open Collector 201; Open Drain 58; Tri-State 34; CMOS 7; Push-Pull 3 (in scope: **903**); Photo IC 9, `-` 3 (out) | `raw/c902.json "Output Type"` |
| …by Input Type, channels (side 1/side 2) | DC 836, Logic 70, AC/DC 5, `-` 4; 1/0 719, 2/0 157, 1/1 15, 4/0 7, 2/1 3, 2/2 3, 3/0 2, `-` 9 | `raw/c902.json "Input Type" "Inputs - Side 1/Side 2"` |
| …makers | **Broadcom 376, Toshiba 159, Renesas 118**; onsemi 88, Vishay 53 | `raw/c902.json Manufacturer` |
| Category 904 (triac / SCR output), all | **731**; Triac 691, Triac Power 21; surface mount 400, through hole 311 | `raw/c904.json "Output Type" "Mounting Type"` |
| …makers | **onsemi 197, Vishay 127, Würth Elektronik 102**; Isocom 91 | `raw/c904.json Manufacturer` |
| Category 901 (digital isolators), all | **1,895** | `raw/c901.json` → `TOTAL` |
| …by Channel Type, Isolated Power | Unidirectional 1,781, Bidirectional 61, both 42; Isolated Power No 1,597, Yes 287 | `raw/c901.json "Channel Type" "Isolated Power"` |
| …by Type (the out ones) | USB 12, LVDS 23, Digital Input Module 1, `-` 4 | `raw/c901.json Type` |
| …by channels, side 1/side 2 | 4 753, 2 567, 6 198, 3 180, 1 100, 5 55, 7 12, 8 12, 9 6, 10 1; 1/1 321, 3/1 255, 2/2 249, 4/0 239, 2/0 195, 2/1 143, 1/0 85 (top seven) | `raw/c901.json "Number of Channels" "Inputs - Side 1/Side 2"` |
| …makers | **ADI 944** (Analog Devices 774 + Analog Devices/Maxim 170), **TI 330, Skyworks 321**; NVE 81, Toshiba 48 | `raw/c901.json Manufacturer` |
| **Picked rows, upper bound** | **2,103** = transistor 600 (200 + 200 + 200) + logic 477 (200 + 159 + 118) + triac 426 (197 + 127 + 102) + digital 600 (200 + 200 + 200): min(200, the maker's category count). The scoped count per maker is lower by the out-of-scope rows, which A measures | the maker rows above |
| **Pages, lower bound** | **54** saved pages: ⌈count / 99⌉ per piece, 10 + 7 + 6 (transistor) + 4 + 2 + 2 (logic) + 2 + 2 + 2 (triac) + 10 + 4 + 4 (digital); splits add pages | the maker rows above |
| DigiKey's own `In Stock` line, first pages | 398 of the 400 first-page rows of the four categories carry a positive `In Stock` line; 1 in 902 and 1 in 904 do not | `python3 -I` over `raw/c90{1,2,3,4}.json` `firstPage` (planning probe, 2026-11-25) |

### Verified properties

- **The listing page carries the parameters, the stock line and the
  datasheet URL.** A DigiKey category page's `__NEXT_DATA__` envelope has up
  to 100 products. Each is a list of cells with `productDetail`
  (`productNumber`, `manufacturer`, `datasheetUrl`, `detailUrl`),
  `qtyAvailable` (the `In Stock` line) and one cell per parameter. The
  planning census's keyword probe of `FT232RL` in category 753 (2026-11-25)
  printed such a cell set, with
  `"datasheetUrl":"https://ftdichip.com/wp-content/uploads/2020/08/DS_FT232R.pdf"`
  and `"qtyAvailable":[{"quantity":"0","label":"In Stock",…}]`. The saved raw
  pages keep only part number, maker and `In Stock` lines
  (`planning/plan-70-data/raw/c903.json` `firstPage`).
- **The page's `In Stock` line is the per-part stock check.** It is the
  quantity DigiKey shows for that part (AGENTS.md: "DigiKey shows a quantity
  available above zero"). plan-58-A used it as its stock check, with no
  product-page read.
- **A filtered page's facets count inside its filter.** The planning pages
  with an extra filter (`planning/plan-71-data/raw/can.json`: Protocol
  CANbus → `TOTAL 574`, makers TI 204, NXP 108, Microchip 95) show per-option
  counts of the filtered set. So a (type, maker) page names its own split
  options and their sizes, and A splits from the page itself (§ 3.2).
- **Parametric filters need `"pt":"String"`** (`planning/plan-58-data/census/make-urls.js`
  header; the census `url.js` sets it for every numeric key).
- **Clicking filters or pages from a script does not work** in Safari's
  temporary tab. A therefore asks for one filtered URL per piece
  (plan-58-A § 2).
- **Cloudflare** challenges the Safari window after about 100 page loads in
  an hour. The planning census hit it twice in about 60 loads. The owner's
  tick in the DigiKey tab clears it.

## 3. Design

### 3.1 Scope filters (in the URL)

Every piece adds In Stock (`-2` = 5) and Active (`1989` = 0), the maker
(`-1`), and the type's own filters, by value id from the raw pages:

| Type | Category | Filters |
|---|---|---|
| transistor | 903 | `41` Output Type ∈ {415695, 415701, 337184, 337186}; `69` Mounting ∈ {409393, 411897} |
| logic | 902 | `41` ∈ {390989, 382144, 382141, 382145, 416041, 331587, 390971} |
| triac | 904 | `41` ∈ {415917, 415920} |
| digital | 901 | `1762` Channel Type ∈ {417765}; `1761` Isolated Power ∈ {380240}; `183` Type ∈ every option of `raw/c901.json`'s Type facet except 418344 (USB), 369718 (LVDS), 338788 (Digital Input Module), 1 (`-`) |

Maker ids: Isocom 58, Vishay 751, onsemi 488, Broadcom 516, Toshiba 264,
Renesas 20, Würth 732, ADI 505 and 175 (two pieces, one company), TI 296,
Skyworks 863.

### 3.2 Pieces and splits (`census/make-urls.js`, `census/list.sh`)

- A piece is (type, maker id). `list.sh` loads the piece's URL. When
  `productCount` < 100, the page is the piece (`<type>_<maker>.json`).
- Otherwise it splits by the page's own `Package / Case` (`16`) facet: one
  URL per option, the options' counts summing to the piece's count. An
  option still at 100 or more splits again by `Series` (`-4`). One that is
  still too big after that splits by `Supplier Device Package` (`1291`).
  Every split is saved as `<type>_<maker>_<facet>-<value>[_…].json`.
- `check.py` rejects a page that is empty, that is another tab's page (a
  row's maker differs from the request, or `productCount` differs from the
  split's facet count), or that has 100 rows. `list.sh` retries it up to 5
  times, 3 s apart, and prints `FAILED <file>` after that.
- A page is never re-requested once saved (`list.sh` skips existing files),
  so a Cloudflare stop resumes where it left off.

### 3.3 `candidates.tsv` (`census/build.py`)

Columns: `type maker mpn series output input channels side1 side2 package
supplier_package mounting grade status stock rank pick datasheet_url detail`.

- `stock` is the number on the product's `In Stock` line, 0 when the line
  is missing (DigiKey's In Stock facet also counts Marketplace and Factory
  stock, plan-58-A § 2). A row with stock 0 is `out`, `no DigiKey stock`.
- ADI's two maker ids are one maker `adi`; maker slugs are those of
  `planning/plan-70-data/README.md`.
- A part number listed twice (once per split, because DigiKey lists a part
  under one package only, and once under each maker id for ADI) is kept once.
- `rank` is the order by `stock` (descending, then part number) inside
  (type, maker). `pick` = `candidate` for rank ≤ 200, `reserve` above.

### 3.4 Stock

Each row's `stock` is its own product cell's `In Stock` quantity on the
saved page, and `status` its own Product Status cell (the URL already
filters Active). These are what DigiKey shows for that part at listing time.
A product-page read per part would add about 2,100 Safari loads, about 21
Cloudflare windows at 100 loads an hour, and read the same number. A row
drops out later only when B or D finds its datasheet does not list it. The
next `reserve` row of its (type, maker) then moves up without a new read.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: The listing

- [ ] Re-run every Prerequisites command; update the Status column.
- [ ] `census/make-urls.js`, `list.sh`, `check.py` per § 3.1–3.2 (from
      plan-58's). `check.py --selftest` covers an empty page, a wrong-maker
      page, a count mismatch, and a full page, and prints `ok`.
- [ ] `LZ=/tmp/lz/node_modules census/list.sh planning/plan-70-data/listing`
      prints one line per saved page and no `FAILED` line.
- [ ] Record the per-piece counts and the page count in § Corrections.

Acceptance: every piece is complete. The pages of each piece sum to the
piece's own `productCount`.
  Check: `python3 -I planning/plan-70-data/census/check.py --verify planning/plan-70-data/listing`
  → `12 pieces, <n> pages, 0 incomplete`. That is about 60 min of Safari
  for at least 54 pages, and only a full listing shows a piece's split
  counts.
Commit:

### Phase 2: The pick

- [ ] `census/build.py` per § 3.3; `--selftest` (a duplicate across splits,
      the ADI merge, a missing `In Stock` line) prints `ok`.
- [ ] `python3 -I census/build.py planning/plan-70-data/listing planning/plan-70-data/candidates.tsv --report`
      prints, per (type, maker), `listed / in stock / candidate / reserve`.
      § 2's upper bound gets the real numbers in § Corrections.

Acceptance: each (type, maker) has min(200, in-stock rows) candidates.
  Check: `awk -F'\t' 'NR>1 && $17=="candidate"{n[$1" "$2]++} END{for(k in n)print k,n[k]}' planning/plan-70-data/candidates.tsv`
  → 12 lines, each at most 200, which match `build.py --report`. Takes
  about 1 min.
Commit:

### Phase 3: The stock gate

- [ ] `build.py --verify-stock planning/plan-70-data/candidates.tsv` reads
      every `candidate` and `reserve` row back from its saved page and
      compares `stock` and `status`. It prints
      `<n> rows, 0 without stock, 0 not Active/LTB, 0 differ from the page`.

Acceptance: every kept row has its own positive `In Stock` line.
  Check: `awk -F'\t' 'NR>1 && ($17=="candidate" || $17=="reserve") && $15<=0' planning/plan-70-data/candidates.tsv | wc -l`
  → 0, and the `--verify-stock` line above. Takes about 1 min.
Commit:

## Validation Plan

- Tests: the `--selftest` modes of `check.py` and `build.py`; the three
  acceptance checks.
- Coverage check: `git status --short backend schematical` → empty after A.
- Doc sync: `planning/plan-70-data/README.md` (the scripts and columns as
  written).
- Final gate: plan-70-J.

## Decisions

- Owner, 2026-11-25: **backend letters wait for plan-68.** The data, datasheet, spec and footprint letters run now; the first letter that changes backend or app code (G) lists plan-68 complete as a prerequisite, and every code letter meets plan-68's size and coverage checks (`node scripts/source-size.js --check`, `node scripts/coverage.js --check`), which the landing letter runs once over all tiers.
- Owner, 2026-11-25: **the top three makers per type at DigiKey, Active and
  Last Time Buy, the 200 most-stocked per maker** (census README decision 1).
- Owner, 2026-11-25: **digital isolators are in** (decision 2), and **an
  isolator is logic in play mode** (decision 3).
- Author: **ADI is one maker** (its two DigiKey ids), as in plan-48.
- Author: **photovoltaic and MOSFET outputs are out.** They are gate-drive
  sources and PhotoMOS relays, and the `relay` kind's `solid-state` type
  already holds PhotoMOS parts. **The 27 Logic-output rows of category 903
  are out** too: the logic type takes its parts from category 902.
- Author: **bidirectional and isolated-power digital isolators are out.**
  A bidirectional open-drain channel needs a wired-AND model across the
  barrier, and an isoPower part needs a regulator output on side 2. USB,
  LVDS and Digital Input Module parts are not single-ended logic channels.
- Author: **automotive (AEC-Q101/Q100) grades are in**, as in plan-48.
- Author: **DIN-rail modules are out** (3 rows); they are not board parts.
- Author: **Fairchild Semiconductor is a legacy label** (Rochester stock,
  plan-48 § 2) and not a maker of this plan.
- Author: **the stock check is each row's own `In Stock` line on its saved
  listing page** (§ 3.4), as in plan-58-A. A product-page read per part is
  not repeated.

## Corrections

## Summary

A is DigiKey work through Safari only. The risks are pages from the wrong
tab (`check.py`), DigiKey's In Stock facet counting third-party stock
(`build.py` counts only the `In Stock` line), and Cloudflare stops (`list.sh`
resumes).
