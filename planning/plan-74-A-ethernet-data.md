# plan-74-A: Ethernet PHYs, controllers and MagJacks: listing, stock check, datasheet downloads, verdicts

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-74 is A → B → C → D → E → F → G → H → I → J → K
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-74 adds Ethernet to the parts library and the app. There are three families, each from the makers
with the most in-stock parts at DigiKey (§ 2, planning census of 2026-11-25):

| Family | Makers (rank order) | In scope |
|---|---|---|
| PHY | **Microchip**, **Texas Instruments**, **Analog Devices** (with Maxim) | single-port twisted-pair PHYs (10BASE-T, 100BASE-TX, 1000BASE-T) with an MII / RMII / RGMII / GMII / SGMII host side |
| controller | **Microchip**, **WIZnet** (named by the owner), **Renesas** and **onsemi** (tied for third, 2 parts each) | single-port Ethernet controllers with an integrated PHY and a host bus (SPI, parallel, or both), WIZnet's hardwired TCP/IP chips included |
| MagJack | **Pulse Electronics**, **Bel Fuse**, **TE Connectivity** | single-port RJ45 jacks with integrated magnetics, through hole, right angle or vertical, 10/100 or 10/100/1000 |

Each maker is capped at its 200 most-stocked in-scope parts per family. The Ethernet ICs are facts
only: a new kind `ethernet` whose devices all say `"simulated": false` (owner decision), so a
schematic with one placed does not run, as with an `mcu` without a model. The MagJacks extend the
existing `connector` kind with an `"rj45"` mode: passive terminals, like the USB connectors.

The single behavioral outcome of plan-74: loading the part files gives the baseline parts plus one
part per `add` row of `planning/plan-74-data/candidates.tsv`; the app lists the ICs under
**Interface → Ethernet → PHY / Controller** and the jacks under **Connector → RJ45**; a placed
`W5500` shows kind name "Ethernet controller" and turns Run off; a placed MagJack has the pins of
its drawing and a footprint whose pads match the drawing's suggested PCB layout; `ctest` passes T0 + 1
programs (`ethernet_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing, scope verdicts, per-part stock check, datasheet downloads, ordering-table verdicts, `specs-ic.tsv` and `specs-rj45.tsv` skeletons | large |
| B | IC specs, Microchip: pinouts and facts per (datasheet, package) | large |
| C | IC specs, TI, ADI, WIZnet, Renesas; the final `footprints-needed.txt` | large |
| D | IC footprints: every package of `footprints-needed.txt` | large |
| E | backend: the `ethernet` kind, the connector's `"rj45"` mode, `ethernet_test`, `connector_test` cases, docs | medium |
| F | IC parts: `gen-ethernet.py`, `Parts/ethernet.json`, `datasheets.json`, goldens | medium |
| G | MagJacks, Bel Fuse: pinouts, footprints, `gen-rj45.py`, parts, goldens | large |
| H | MagJacks, Pulse: pinouts, footprints, parts, goldens | large |
| I | MagJacks, TE: pinouts, footprints, parts, goldens | large |
| J | app: details, facts, kind names, drawer, Part Editor kind, harness | large |
| K | landing tier | small |

Outcome of A:

- `candidates.tsv` lists every part DigiKey shows for the three families and the plan's makers,
  with a scope verdict.
- `stock-check.tsv` checks every in-scope row: DigiKey's own `In Stock` line above 0, and status
  Active or Last Time Buy.
- `verdicts.tsv` gives each row `add`, `pending` (a MagJack whose drawing is an image; G–I decide it)
  or `skip` / `out` with a reason.
- `datasheets/ethernet/<maker>/` and `datasheets/connector-rj45/<maker>/` hold every PDF an `add` or
  `pending` row cites.
- `specs-ic.tsv` and `specs-rj45.tsv` exist with their pinout and drawing columns empty.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints,
  § `connector`, § `generic`); `docs/backend.md` (§ Adding a kind, § goldens).
- `planning/plan-69-data/census/README.md`: the planning census of plans 69–74 and its tools
  (`url.js`, `facets.sh`, `show.py`, `kw.js`). A calls them and does not change them.
- `planning/plan-74-data/README.md`: the files of this plan.
- Precedents: `planning/completed/plan-58-A-slide-switch-data.md` (a Safari listing, the
  `In Stock`-line rule, `check.py`/`list.sh` retries, `pending` verdicts);
  `planning/completed/plan-48-A-timer-opamp-data.md` (per-maker 200 cap, reserve rows, text scan,
  `specs.tsv` skeleton); `planning/completed/plan-45-A-terminal-jst-data.md` (connector families).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p74 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p74 -j8 && ctest --test-dir /tmp/logic-p74 -j8` → `100% tests passed` | MET (measured by the author at planning, 2026-11-25 at 63e2f7d2: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | at planning (63e2f7d2): **P0 = R0 = 40832**, **F0 = 3993**. E, F, G, H and I re-measure at their start and use their own |
| Connector baseline | `grep -c '\|connector\|' backend/tests/golden/parts.txt`; `grep -c '"id"' schematical/schematical/Parts/connector.json` | at planning: **C0 = 1541** connector parts, 567 device ids |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p74-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-25 at 63e2f7d2: `** BUILD SUCCEEDED **`, run by the author: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO build`) |
| Nothing of plan-74 exists yet | `grep -c 'PART_ETHERNET' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/ethernet.json` → no such file; `grep -c '"rj45"' backend/src/models/connector.c` → 0; `python3 -I -c "import json;print(sum(f['id'].startswith('RJ45-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0; `ls datasheets/ethernet datasheets/connector-rj45` → no such directory | MET (2026-11-25: 0; no file; 0; 0; no directory. `datasheets/modular-ethernet-connector-rj45-jacks-wi/` exists, empty and untracked: `git ls-files datasheets/modular-ethernet-connector-rj45-jacks-wi \| wc -l` → 0. plan-74 does not use it) |
| No other open plan adds these kinds, keys or folders | `grep -l -i 'PART_ETHERNET\|Parts/ethernet\.json\|"rj45"\|datasheets/ethernet\|connector-rj45' planning/plan-*.md \| grep -v plan-74 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 69 (`tvs`), 70 (`isolator`), 71 (`transceiver`), 72 (`ioexpander`, `i2cbus`) and 73 (serial bridges) are written at the same time. The lines several plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, `CMakeLists.txt`) are added only if absent and after whatever is last; each such step greps first (E Phase 1, J Phase 1) |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | a rule, applied per footprint in D, G, H, I (`fp-merge.py` skips an id already present) |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/365" 10 'document.title'` → `Jacks With Magnetics \| Connectors, Interconnects \| DigiKey` (the title starts `Jacks With Magnetics`) | MET at planning (2026-11-25: the census's pages loaded after the owner ticked "Verify you are human" twice; each time, a run of page loads brought the challenge back). When the command prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| `lz-string` for the census URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls /tmp/lz/node_modules/lz-string` → a directory | MET at planning (installed in the author's scratchpad, 2026-11-25) |
| The maker sites serve their PDFs to Python | `python3 -I planning/plan-74-data/census/fetch.py --probe` (A Phase 2 writes it from the planning probe) → 8 lines `200 … %PDF-` | MET as a probe at planning (2026-11-25: Python `urllib`, Safari user agent plus `Accept` and `Accept-Language`, got a PDF from each of: `www.belfuse.com` drawing `dr-mag-0826-1x1t-23-f.pdf` 544,569 B; `yageogroup.com/download/datasheet/J0011D01NL` 820,303 B; `www.te.com` DDEController `114-94447` 1,708,021 B, customer drawing `DocNm=5-2301995-2&DocType=Customer%20Drawing&DocLang=English&DocFormat=pdf&PartCntxt=5-2301995-2` 216,706 B and data sheet `1-1773921-5_RJ45_Integrated_Magnetics` 985,887 B; `ww1.microchip.com/downloads/en/DeviceDoc/00002414A.pdf` 1,317,441 B; `docs.wiznet.io/img/products/w5500/W5500_ds_v110e.pdf` 1,893,745 B; `www.ti.com/lit/ds/symlink/dp83825i.pdf` 2,487,023 B; `www.analog.com/…/ADIN1300.pdf` 1,861,324 B; `www.renesas.com/us/en/document/dst/1893c-datasheet` 869,781 B) |
| Tools | `pdftotext -v`, `pdfinfo -v`, `pdftoppm -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25: all used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `planning/plan-74-data/candidates.tsv` has one row per part of the A listing (§ 3.1): the three
  families, the plan's makers, DigiKey In Stock + Active. Each row has `scope` = `in` or `out` with
  a reason (§ 3.2). No in-scope maker has more than 200 `in` rows per family: the 200 most-stocked
  stay `in`, and the rest are `out` with reason `cap`.
- `stock-check.tsv` has one row per `in` row: DigiKey's own `In Stock` quantity (> 0) and status.
- `verdicts.tsv` has one row per `in` row: `add`, `pending` (G–I) or `skip` with a reason (§ 3.4).
- `datasheets/ethernet/{microchip,ti,adi,wiznet,renesas}/` and
  `datasheets/connector-rj45/{bel,pulse,te}/` hold each PDF an `add` or `pending` row cites, with one
  `datasheets.tsv` per family folder (file, URL, bytes, SHA-256).
- `datasheets/skipped-parts.md` has a section "Ethernet (plan-74)" listing every `skip` row.
  `datasheets/obsolete-not-included.md` gets a row only for a PDF all of whose parts are obsolete.
- `specs-ic.tsv` has one row per (maker, datasheet, package) of the IC `add` rows. `specs-rj45.tsv`
  has one row per (maker, drawing) of the jack `add` and `pending` rows. Their pinout, facts and
  drawing columns are empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `scripts/digikey.js` and the shared census tools of `planning/plan-69-data/census/` are not
  changed. AGENTS.md: stock is looked up in Safari.
- Out of scope, and never re-opened in later letters:
  - Ethernet switch ICs, USB-to-Ethernet bridges and EtherCAT slave controllers (LAN9252, LAN9253,
    LAN9254: two or three ports).
  - Single-pair PHYs and controllers (10BASE-T1S, 10BASE-T1L, 100BASE-T1, 1000BASE-T1), fiber-only
    PHYs, and multi-port PHYs.
  - PoE controllers.
  - Multi-port, stacked and RJ45-plus-USB jacks; surface-mount, panel-mount and board-edge jacks;
    2.5G, 5G and 10G jacks; plain RJ45 jacks without magnetics (DigiKey category 366).
- DigiKey's legacy labels (National Semiconductor, SMSC, Intersil, Rochester Electronics) are not
  makers of this plan.

## 2. Current State

There is no Ethernet IC and no RJ45 jack in the library (`grep -c -i 'ethernet\|rj45'
backend/tests/golden/parts.txt` → 0, 2026-11-25). The `connector` kind
(`backend/src/models/connector.c`, 278 lines) has five modes: barrel jack, `"usb"`, `"card"`,
`"testPoint"`, and `"terminal"` / `"jst"`. Each is a passive terminal block of `inout` pins
(`docs/parts-format.md` § `connector`). The `generic` kind (`backend/src/models/generic.c`) is the
`"simulated": false` shape: a passive engine model, and `circuit_add_part` refuses the part.
`footprints.json` (3993 footprints) has no `RJ45-` footprint, but it has the IC outlines the PHYs
and controllers mostly use:

```
python3 -I -c "import json,re;ids=[f['id'] for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']];print(sum(bool(re.match(r'^(V)?QFN-(24|28|32|36|48|56|64)',i)) for i in ids), sum(bool(re.match(r'^(L|T)QFP-(48|64|80|100|128)',i)) for i in ids))"
```

That gives `40 13`: e.g. `QFN-24-4X4`, `QFN-32-5X5`, `QFN-48-7X7`, `LQFP-48-PT`, `LQFP-64-10X10`,
`LQFP-100-14X14`, `SSOP-28`, `SOIC-28W`.

### Measured populations

All DigiKey numbers are from 2026-11-25, In Stock + Active, the facet counts of the planning census
(`planning/plan-69-data/census/README.md`; raw pages in `planning/plan-74-data/raw/`; each
printed by `python3 -I planning/plan-69-data/census/show.py planning/plan-74-data/raw/<file>
Manufacturer`).

| What | Count | Source |
|---|---|---|
| Ethernet transceivers in 710 (13 Ethernet / MII / RMII protocol ids) | 267: Microchip 151, TI 57, Infineon 15, ADI 24 + Maxim 1 = 25, NXP 8, MaxLinear 6, … | `raw/eth710.json` |
| Ethernet ICs in Controllers (753) | 314: Function Switch 156, Controller 109, USB to Ethernet 24, MAC Controller 18, PHY 7 | `raw/eth753.json` (`show.py … Function`) |
| …single-port with a host bus (Interface Parallel, SPI, SPI+Parallel, I2C+SPI, Bus) | 74: Microchip 64, WIZnet 4, onsemi 2, Renesas 2, Cirrus 1, NetBurner 1 | `raw/ethc2.json` (every row saved) |
| …in scope (§ 3.2) | **59**: Microchip 53, WIZnet 4, Renesas 2 (out: EtherCAT 11, 10BASE-T1S 2, other 2) | `python3 -I planning/plan-74-data/census/ctl-scope.py planning/plan-74-data/raw/ethc2.json` → `rows 74 in 59 {'wiznet': 4, 'microchip': 53, 'renesas': 2} out {'EtherCAT (2-3 ports)': 11, 'single pair (10BASE-T1S)': 2, 'not a maker of the pick / no DigiKey stock': 2}` |
| Jacks with magnetics (365) | 1,112: Pulse 222, Bel Fuse 185, TE 120 (TE Connectivity AMP Connectors 119 + TE Connectivity Erni 1), Abracon 113, Taoglas 91, Würth 91, Phoenix 54 | `raw/c365.json` |
| …category-wide facets that the scope filters use | Connector Type RJ45 1,037; Ports 1 875; Through Hole 1,013; Right angle 985 / Vertical 106; Applications 10/100… and 1G ids (§ 3.2) | `show.py raw/c365.json 'Connector Type' 'Number of Ports' 'Mounting Type' Orientation Applications` |
| Upper bounds on A's `in` rows | PHY ≤ 233 (151 + 57 + 25); controller 59; MagJack ≤ 505 (200 of Pulse's 222 + 185 + 120): **≤ 797** | the rows above. The scope filters of § 3.2 only lower the PHY and jack counts; A records the exact counts |
| Datasheet hosts of the first census pages | jacks: Bel `www.belfuse.com` drawings (30 of the first 100 rows), Pulse `yageogroup.com/download/datasheet/<pn>` (12), TE DDEController (1, the application spec 114-94447); ICs: `ww1.microchip.com` (72 of eth710's first 100, 62 of 74 controllers), `www.ti.com` (17), `www.analog.com` (6), `docs.wiznet.io` (2), `www.renesas.com` (2), DigiKey copies `mm.digikey.com` (5) | the planning probe over the census pages' `datasheetUrl` |
| Bel Fuse drawings carry no text | `pdftotext dr-mag-0826-1x1t-23-f.pdf - \| wc -w` → 8 (an image: the schematic page shows TRD1+ … TRD4-, TRCT1 … TRCT4 on pins 1–12, LEDs on 13–16, SHIELD) | planning probe, 2026-11-25 |
| TE: one customer drawing per part number | `5-2301995-2`'s drawing: 3 pages, with a "SUGGESTED PCB LAYOUT" and pins P1 … P8. The data sheet `1-1773921-5_RJ45_Integrated_Magnetics` lists 5-2301995-1 … -4 (1G / 10/100, LED / no LED, vertical) | planning probe, 2026-11-25; URLs read off `https://www.te.com/en/product-5-2301995-2.html` |

### Verified properties

- **DigiKey's In Stock facet also counts Marketplace and Factory stock.** In `raw/ethc2.json`,
  Cirrus `CS8920A-CQ` is listed with only `Marketplace 3,798`, and onsemi `NCN26010XMNTBG` with
  `In Stock 944, Factory 2,000`. Only DigiKey's own `In Stock` line, above 0, counts (AGENTS.md;
  plan-58-A).
- **A census page holds at most 100 products** (`facets.sh` saves the page's own `__NEXT_DATA__`).
  A family and maker with more than 100 is listed in pieces split by a facet (§ 3.1).
- **Image-only Bel drawings**: A's text scan cannot match a Bel part number inside its drawing. The
  drawing's file name, with `x` as the wildcard of the option letters, is DigiKey's link and covers
  the series (`0826-1C1T-23-F` → `dr-mag-0826-1x1t-23-f.pdf`). Such rows are `pending`, and G
  reads the title block.

## 3. Design

### 3.1 The listing (`census/list.sh`, `census/split.py`, `census/build.py`)

One page per (family, maker, piece), saved to `planning/plan-74-data/raw-A/<family>_<maker>_<piece>.json`
by `planning/plan-69-data/census/facets.sh` with these extra filters:

- **PHY**: category `x/710`; Protocol `578` = the 13 ids of `planning/plan-74-data/README.md`;
  Manufacturer `-1` = 150 (Microchip), 296 (TI), 505 and 175 (ADI, two labels). Microchip (151) is
  listed in two pieces by Protocol: [345107] and the other 12 ids.
- **controller**: category `x/753`; Protocol 345107; Function `110` = [333534, 370852, 386629];
  Interface `154` = [383917, 406106, 406137, 752314, 323795]; Manufacturer 150, 1278 (WIZnet),
  20 (Renesas), 488 (onsemi). One page in all: 74 rows.
- **MagJack**: category `x/365`; Connector Type `28` = [397074]; Number of Ports `2171` = [1];
  Mounting Type `69` = [411897]; Orientation `208` = [298923, 419539]; Manufacturer 553 (Pulse),
  507 (Bel Fuse), 17 and 2684 (TE, two labels). `split.py` reads a page's `productCount`. A page over 100 is split by
  Orientation, then by Applications `405` (the 23 in-scope ids of § 3.2), until each piece is under
  100.

`list.sh` retries a page up to 5 times when `check.py` finds it empty, or with rows of another
maker (plan-58-A's precedent: Safari sometimes returns another tab). `build.py` writes
`candidates.tsv`, one row per distinct (maker, part number):

`family  maker  part  stock  status  digikey_id  description  package  supplier_package  speed
host  orientation  leds  poe  datasheet_url  scope  reason  verdict  verdict_reason  pdf  stock_checked`

`family` is `phy`, `controller` or `magjack`; `maker` is the slug `microchip`, `ti`, `adi`, `wiznet`,
`renesas`, `onsemi`, `pulse`, `bel` or `te` (the datasheet folder names). `stock` is the `In Stock`
line only (0 when absent). `speed`, `host`, `orientation`, `leds` and
`poe` are DigiKey's parameters (Protocol / Interface / Applications / Orientation / LED Color).

### 3.2 Scope (`build.py`)

`out` reasons, the first that applies:

1. `no DigiKey stock`: `stock` = 0.
2. `status`: not Active or Last Time Buy.
3. PHY and controller: `switch` (Function Switch, or "switch" in the description);
   `EtherCAT` (LAN9252, LAN9253, LAN9254); `single pair` (10BASE-T1S / T1L, 100BASE-T1,
   1000BASE-T1 in the description or protocol: TJA11xx, DP83TC/TD/TG, LAN86xx/LAN87xx-T1,
   ADIN1100/1110, NCN26010); `multi-port` (more than one port in the description);
   `fiber only` (100BASE-FX with no copper mode).
4. MagJack: `speed` (Applications not one of the 23 ids 64321, 64324, 64325, 66761, 66766, 66767,
   66768, 66770, 66774, 66775, 66776, 66777, 66787, 66789, 66790, 66791, 66793, 66794, 66797, 66798,
   120120, 120121, 120122: 10, 10/100, 10/100/1000 and 1G Base-T/TX, with or without AutoMDIX and
   PoE).
5. `cap`: beyond the maker's 200 most-stocked `in` rows of that family.

A jack's `speed` column maps to the part's `"rj45"` value: `"10/100"` for 10 and 10/100 Base-T/TX,
`"1000"` for 10/100/1000 and 1G. `poe` is 1 where Applications names Power over Ethernet.

### 3.3 Stock check (`stock-check.tsv`)

`build.py`'s `stock` is the listing's own `In Stock` line, read part by part from the saved page's
product cell. The check is that line, not the facet. Columns `maker part in_stock status checked`
(the date). A row whose cell shows no `In Stock` line, or shows 0, is `out` (`no DigiKey stock`).

### 3.4 Datasheets and verdicts (`census/fetch.py`, `census/textscan.py`)

- **Download.** `fetch.py` uses Python `urllib` with a Safari user agent, `Accept` and
  `Accept-Language` (the planning probe). A `mm.digikey.com` link goes through
  `scripts/safari-digikey-pdf.sh`. TI links (`suppproductinfo.tsp?…gotoUrl=…/lit/gpn/<part>`) are
  rewritten to `https://www.ti.com/lit/ds/symlink/<part>.pdf`. A TE row's PDF is its customer
  drawing, `https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=<pn>&DocType=Customer%20Drawing&DocLang=English&DocFormat=pdf&PartCntxt=<pn>`
  (DigiKey links the application spec instead). TE's application spec
  `114-94447` (`…DocNm=114-94447&DocType=SS&DocLang=EN`) is downloaded once as well, to
  `datasheets/connector-rj45/te/114-94447.pdf`: plan-74-I reads its PCB layout figures. A Pulse row's PDF is
  `https://yageogroup.com/download/datasheet/<pn>`. A Bel row's PDF is DigiKey's link, with its
  query dropped. Files are named by the maker's file name in lower case (TE: `<pn>-cd.pdf`; Pulse:
  `<series>.pdf` when several part numbers serve byte-identical PDFs, by SHA-256).
- **Verdicts** (`textscan.py`, `pdftotext -layout`):
  - `add`: the part number, less DigiKey's packaging suffix (`-TR`, `T` reel prefixes of
    Microchip, `-R`, `-REEL`, `/TR`), appears verbatim in its PDF's text (ordering table, part
    number table, or title block), and the PDF prints no obsolete / not-recommended status for it.
  - `pending`: a Bel drawing (image only) whose file name pattern matches the part number.
  - `skip` (reasons): `no PDF`, `not in PDF`, `PDF obsolete`.
- **Reserves.** A `skip` frees a capped maker's slot. The next `cap` row of that maker and family
  becomes `in`, and gets its stock check and verdict, until 200 are `add` / `pending` or no row is left.

### 3.5 Spec skeletons

- `specs-ic.tsv`: `maker  datasheet  package  footprint  parts  type  speed  host  tcpip  vcc  pinout  note`.
  One row per (maker, PDF, DigiKey package) of the IC `add` rows. `parts` is the comma list of
  part numbers. `type` is `phy` or `controller`, from the family. `speed` (10, 100 or 1000) and
  `host` come from DigiKey's parameters. B and C check them against the datasheet and fill the rest.
- `specs-rj45.tsv`: `maker  drawing  parts  rj45  poe  orientation  leds  pins  footprint  note`.
  One row per (maker, drawing PDF) of the jack `add` / `pending` rows.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Listing and scope

- [ ] `planning/plan-74-data/census/list.sh`, `split.py`, `check.py`, `build.py` per § 3.1–3.3.
- [ ] Run the listing into `planning/plan-74-data/raw-A/`; write `candidates.tsv` and `stock-check.tsv`.
- [ ] Record in this section the counts by family, maker and `scope` / `reason`
      (`awk -F'\t' 'NR>1{print $1, $2, $16, $17}' planning/plan-74-data/candidates.tsv | sort | uniq -c`).

Acceptance: every page of the listing has fewer than 100 rows and matches its request. No in-scope
maker has more than 200 `in` rows per family. The controller family's `in` count is recorded next to the planning count, 59 (Microchip 53,
WIZnet 4, Renesas 2).
  Check: `python3 -I planning/plan-74-data/census/build.py --verify planning/plan-74-data/raw-A planning/plan-74-data/candidates.tsv` → `pages ok, cap ok` and the counts above (est. 25 min: the Safari pages of § 3.1 and the build).
  Result:
Commit:

### Phase 2 — Downloads, verdicts, skeletons

- [ ] `census/fetch.py` (with `--probe`: the eight planning URLs of § Prerequisites) and
      `census/textscan.py` per § 3.4; download into `datasheets/ethernet/<maker>/` and
      `datasheets/connector-rj45/<maker>/`, with the `datasheets.tsv` files.
- [ ] `verdicts.tsv`; reserves moved up per § 3.4; `candidates.tsv`'s `verdict` columns filled.
- [ ] `datasheets/skipped-parts.md` § "Ethernet (plan-74)"; `obsolete-not-included.md` rows if any.
- [ ] `specs-ic.tsv` and `specs-rj45.tsv` skeletons per § 3.5.

Acceptance: every `add` / `pending` row has a local PDF, and every `skip` row is listed in
`skipped-parts.md`.
  Check: `python3 -I planning/plan-74-data/census/textscan.py --verify` → `<a> add, <p> pending, <s> skip; 0 add/pending without a PDF; 0 skip missing from skipped-parts.md`; `find datasheets/ethernet datasheets/connector-rj45 -name '*.pdf' | wc -l` equals the distinct `pdf` values of the `add` and `pending` rows (est. 40 min: the downloads).
  Result:
Commit:

## Validation Plan

- A's checks are its two acceptance commands. Nothing in `backend/` or the app changes.
- Final gate: plan-74-K.

## Decisions

- Owner, 2026-11-25: **backend letters wait for plan-68.** The data, datasheet, spec and footprint letters run now; the first letter that changes backend or app code (E) lists plan-68 complete as a prerequisite, and every code letter meets plan-68's size and coverage checks (`node scripts/source-size.js --check`, `node scripts/coverage.js --check`), which the landing letter runs once over all tiers.
- Owner, 2026-11-25: the pick is the top 3 makers per family by DigiKey in-stock count, Active
  only, each capped at its 200 most-stocked; named parts are always in. WIZnet is named. Renesas and
  onsemi tie for third among controllers (2 parts each), so both are in.
- Owner, 2026-11-25: Ethernet covers PHYs, single-port controllers with an SPI or parallel host bus
  plus WIZnet's parts, and RJ45 jacks with magnetics. Switch ICs and USB-to-Ethernet bridges are out.
- Owner, 2026-11-25: Ethernet ICs are facts only, `"simulated": false`.
- Author: **single-pair, fiber-only, multi-port PHYs and EtherCAT controllers are out.** The
  plan's jacks are 8-contact RJ45 twisted-pair jacks, so only twisted-pair single-port parts pair with
  them. EtherCAT slave controllers are two- or three-port parts.
- Author: **jacks are single-port, through hole, right angle or vertical, up to 1G.** Multi-port,
  surface-mount and 2.5G–10G jacks need footprints and pin schemes no in-scope PHY uses.
- Author: **TE's datasheet is the customer drawing, not DigiKey's link.** DigiKey links TE's
  application spec 114-94447, which says the board-side (P) pin functions are on each product's
  drawing.
- Author: **Pulse's PDFs come from yageogroup.com.** That is DigiKey's link, and the site of Pulse's
  parent, YAGEO, which publishes its datasheets.
- Author: **Bel drawings are `pending`, decided in G**, since they carry no text.

## Corrections

## Summary

A lists the three families through Safari, keeps the in-scope parts within each maker's cap, checks
each one's own DigiKey stock line, downloads the datasheets and drawings, and gives every row a
verdict. The skeletons of the IC and jack specs are the hand-off to B–I.
