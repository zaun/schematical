# plan-69-A: TVS diodes and ESD arrays — Safari listing, the pick, datasheet downloads, verdicts

Last updated: 2026-11-25
Overall Effort: huge (> 3d) — plan-69 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-69 adds **TVS diodes** and **ESD steering arrays** to the parts library and
the app as a new kind, `tvs`, from the top three makers of each type at DigiKey by
in-stock count (`planning/plan-69-data/census/README.md`, rows for plan 69):

| Type (DigiKey TVS Diodes, 144, facet Type) | Makers (rank order) | Pick |
|---|---|---|
| **Zener** (`183` = 423536): single and multi-channel TVS / ESD diodes, uni- and bidirectional | **Littelfuse** 3,252, **Vishay General Semiconductor** 2,508, **YAGEO** 1,365 | the 200 most-stocked in-scope rows per maker |
| **Steering (Rail to Rail)** (`183` = 408274): diode arrays that steer each line to a rail and to ground | **Semtech** 88, **Littelfuse** 34, **Diodes Incorporated** 33 | every in-scope row (each maker has fewer than 200) |

A TVS part is facts only in play mode: it is in the circuit, drives nothing and
does not turn Run off. The voltage check knows its standoff voltage: a net above
the part's working standoff voltage on any of its pins (other than a ground pin)
is a voltage problem, as a capacitor's or fuse's rating is (plan-69-E).

The single behavioral outcome of plan-69: loading the part files gives the baseline
parts (§ Prerequisites, P0 = 40832) plus one `tvs` part per `add` row of
`planning/plan-69-data/candidates.tsv` (at most 755: 600 Zener-type, 155
steering); the app lists them under **Discrete → TVS diodes → Unidirectional /
Bidirectional / ESD arrays**; a placed `SMBJ5.0A`-type unidirectional TVS draws as
the TVS diode symbol with reference `D1`; wiring its cathode to a 12 V supply is
refused with "… is rated 5 V"; wiring it to 5 V is accepted and Run stays on;
`ctest` passes T0 + 1 programs (`tvs_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | Safari listing (pieces of at most 100 rows), the pick, datasheet URL check and downloads, text scan, verdicts, placement, `specs.tsv` skeleton, `footprints-needed.txt` | large |
| B | specs: ordering schemes, pinouts and the fact check per (datasheet, package); skipped and obsolete lists | large |
| C | footprints: two-terminal packages (SMA/SMB/SMC, SOD, DO-15/DO-201/P600 axial, DO-218, chip 0201/0402, 2-pin DFN) | large |
| D | footprints: array packages (SOT-23, SC-70/SOT-363, SOT-563, SOIC, MSOP, DFN/UFDFN/XFDFN) | large |
| E | backend: the `tvs` kind, `VOLTAGE_TVS_OVER`, ERC passive pins, `tvs_test`, docs | large |
| F | parts: Zener-type TVS diodes (Littelfuse, Vishay, YAGEO); the editor's kind lists; goldens | large |
| G | parts: steering arrays (Semtech, Littelfuse, Diodes Inc.); goldens | medium |
| H | app: catalog details, facts, kind names, the TVS symbol, reference `D`, drawer, exports, harness | large |
| I | landing tier | small |

Outcome of A: `planning/plan-69-data/candidates.tsv` lists every in-scope row of
the six (type, maker) pieces from a fresh Safari listing, each with its own `In
Stock` quantity; every picked row has a `verdict` (`add`, or `skip` / `out` with a
reason from its datasheet's text); `datasheets/tvs/<company>/` holds every
maker-site PDF an `add` row cites, with one `datasheets.tsv` per folder;
`specs.tsv` and `footprints-needed.txt` exist for B, C and D.

References:

- `AGENTS.md` (§ Datasheets: in stock at DigiKey, stock looked up in Safari,
  obsolete PDFs); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints).
- `planning/plan-69-data/census/README.md` (the planning counts and the owner's
  decisions), `planning/plan-69-data/README.md` (this plan's files and columns).
- `planning/completed/plan-58-A-slide-switch-data.md` and
  `planning/plan-58-data/census/` (`list.sh`, `check.py`, `build.py`: the Safari
  listing A copies); `planning/completed/plan-48-A-timer-opamp-data.md` and
  `planning/plan-48-data/census/` (`common.py`, `urlcheck.py`, `select.py`,
  `textscan.py`, `verdicts.py`, `place.py`, `specs-skeleton.py`,
  `footprint-lines.py`: the selection and verdict scripts A copies and adapts).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p69 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p69 -j8 && ctest --test-dir /tmp/logic-p69 -j8` → `100% tests passed` | MET (2026-11-25 at 63e2f7d2: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | at planning (63e2f7d2): **P0 = 40832**, **R0 = 40832**, **F0 = 3993**, **E0 = 40832**; E, F and G re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-25 at 63e2f7d2: `** BUILD SUCCEEDED **`) |
| The kind does not exist | `grep -c 'PART_TVS' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/tvs.json datasheets/tvs` → no such file, no such directory | MET (2026-11-25: 0; neither exists) |
| No other open plan adds the `tvs` kind | `grep -l 'PART_TVS\|Parts/tvs\.json\|datasheets/tvs\|"kind": *"tvs"' planning/plan-*.md \| grep -v '/plan-69-' \| wc -l` → 0 | MET (2026-11-25: 0). Plans 70–74 (written the same day) add `isolator`, `transceiver`, `ioexpander` / `i2cbus`, `serial` parts and `ethernet`: the lines several plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, `kind_is_passive`, `VoltageProblemKind`, the editor's kind lists, footprint ids) are added only if absent and after whatever is last; each such step greps first (E Phase 1, F Phase 1, H Phase 1) |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | rule, applied per footprint in C and D |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/tvs-diodes/144" 10 'document.title'` → `TVS Diodes \| Circuit Protection \| DigiKey` | MET (2026-11-25: `TVS Diodes \| Circuit Protection \| DigiKey`). About 100 page loads in a row bring Cloudflare's check back (`Just a moment...`); then the owner ticks "Verify you are human" once in the DigiKey tab and the command is re-run |
| lz-string for the filter URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls /tmp/lz/node_modules/lz-string` | MET (2026-11-25, the planning census used it) |
| Semtech's datasheets come out of Safari | `scripts/safari-digikey.sh "https://semtech.my.salesforce.com/sfc/p/E0000000JelG/a/44000000MCQl/KlPeDyssnjD4rqRBbMXrL70pAtgk9WdMSWIRW.T3mAE" 25 'document.documentElement.outerHTML.match(/renditionDownload\?rendition=JPGZ&amp;versionId=(\w+)&amp;operationContext=DELIVERY&amp;contentId=(\w+)/).slice(1).join(" ")'` → `06844000005gkLl 05T4400000FEiJt` (versionId, contentId); `curl -sL -A "Mozilla/5.0" -o /tmp/p69-rclamp0504s.pdf "https://semtech.my.salesforce.com/sfc/dist/version/renditionDownload?rendition=ORIGINAL_Pdf&versionId=06844000005gkLl&operationContext=DELIVERY&contentId=05T4400000FEiJt&page=0&d=/a/44000000MCQl/KlPeDyssnjD4rqRBbMXrL70pAtgk9WdMSWIRW.T3mAE&oid=00DE0000000JelG&dpt=null&viewId="` → a PDF | MET (2026-11-25, by the author: an 8-page PDF, title `RClamp0504S.pmd`, page 1 "RClamp0504S RailClamp Low Capacitance TVS Array". The page holds no `/sfc/dist/version/download` link and the `download/?oid=…&ids=<versionId>` form returns an empty file; `rendition=PDF` is empty too, and only `ORIGINAL_Pdf` gives the file) |
| `pdftotext`, `pdfinfo`, `python3`, `node` | `which pdftotext pdfinfo python3 node` → four paths | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `listing/pieces.tsv` has no `failed` piece, and its `leaf` pieces' `rows` add up
  to the `n` of the six roots: every in-scope DigiKey row of the six (type, maker)
  pieces is in `candidates.tsv`.
- Each picked row (`decision` `candidate`) has an `In Stock` quantity above 0 on
  its own listing row and status Active or Last Time Buy.
- `verdicts.tsv` has one row per picked row: `add`, or `skip` / `out` with one of
  the reasons of § 4. A `skip` or `out` moves the next `reserve` row of its
  (type, company) up; that row gets its scan and verdict in turn, until each
  Zener-type maker has 200 `add` rows or no reserve row is left.
- `datasheets/tvs/<company>/` holds each PDF an `add` row cites; each folder's
  `datasheets.tsv` maps the file to its URL, size and SHA-256.
- `specs.tsv` has one row per (type, company, pdf, package, supplier_package, uni,
  bi) of the `add` rows, its B columns empty; `footprints-needed.txt` has one line
  per package line of the `add` rows.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No row is final here: B reads the ordering schemes, pinouts and facts, and
  skips a row whose datasheet does not bear them out.
- No listing through `scripts/digikey.js` (AGENTS.md: stock is looked up in
  Safari); `scripts/digikey.js` is not changed.
- Military parts (DigiKey Grade Military, any MIL-PRF qualification) are out.
  Automotive (AEC-Q101 / AEC-Q100) parts are in and get `"flags": ["automotive"]`
  in F and G (`docs/parts-format.md:255`), as earlier kinds do.
- TVS thyristors, varistors, gas discharge tubes, mixed-technology and surge
  protection devices (DigiKey categories 147, 141, 142, 149, 992) are out: the kind
  covers DigiKey's TVS Diodes category only.
- ST's `USBLC6` and TI's `TPD4E…` arrays are not in scope: neither maker is in a
  type's top three (Steering: TI 24, fifth; ST below). The kind's format covers
  their shape (E's fixture is a six-pin, four-line steering array), so a later plan
  adds them as data only. The reference-board todo item "protection: 18 parts"
  (`planning/todo.md`) stays its own item.

## 2. Current State

The library has no TVS kind and no TVS part: `grep -c 'PART_TVS' backend/include/parts.h`
→ 0; the `diode` kind (`backend/src/models/diode.c`, 29 devices,
`grep -c '|diode|' backend/tests/golden/parts.txt` → 11166 parts) covers
rectifier, fast, switching, Schottky, Zener and bridge diodes; its two `ESDLW` /
`ESDLWH` parts (Taiwan Semiconductor) are listed as Zener-type diodes and stay
there. `footprints.json` has 3993 footprints, among them the two-terminal diode
outlines `DIODE-SMA`, `DIODE-SMB`, `DIODE-SMC`, `DIODE-SMF`, `DIODE-SOD-123F`,
`DIODE-SOD-123FL`, `DIODE-SOD-123W`, `DIODE-SOD-323`, `DIODE-SOD-523`,
`DIODE-SOD-923`, `DIODE-DO-15`, `DIODE-DO-41`, `DIODE-DO-201AD`,
`DIODE-DO-218AC`, `DIODE-DFN1006`, `DIODE-CLP1006` and the small-outline
`SOT23-3`, `SOT23-6`, `SC70-6`, `SOT363-6`, `SOT563-6` families
(`python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 3993).
Diode parts number pin 1 the cathode and pin 2 the anode on two-terminal packages
(`diode.json` pinout `two-pin`: `1 K`, `2 A`), and F keeps that.

### Measured populations

All DigiKey numbers are from 2026-11-25, In Stock and Active, read from the saved
planning pages (`S` = `python3 -I planning/plan-69-data/census/show.py`).

| What | Count | Command |
|---|---|---|
| TVS Diodes (144), any type | **24,278** | `S planning/plan-69-data/raw/c144.json Manufacturer` → `TOTAL` |
| …Type Zener / Steering / `-` | **22,407 / 426 / 216** | `S raw/c144.json Type` |
| **Makers, Zener type** | Littelfuse **3,252**, Vishay General Semiconductor – Diodes Division **2,508**, YAGEO **1,365**; then MDE Semiconductor 1,222, Bourns 1,193, Panjit 984 | `S raw/tvsz.json Manufacturer` |
| **Makers, Steering** | Semtech **88**, Littelfuse **34**, Diodes Incorporated **33**; then onsemi 26, TI 24, Bourns 21 | `S raw/tvss.json Manufacturer` |
| The pick | Zener 200 + 200 + 200 = 600, Steering 88 + 34 + 33 = 155: **755** candidate rows at most | min(200, count) per (type, maker) of the two rows above |
| Zener type by package (top) | DO-214AB/SMC 5,135; DO-214AA/SMB 4,051; DO-214AC/SMA 2,909; P600 axial 1,359; SOD-123F 1,068; DO-15 axial 915; SOT-23-3 586; DO-201AA 443; SOD-323 425; 0201 355; 0402 (1006) 323; DO-218AB 256; SOD-523 233; 179 Package / Case values in all | `S raw/tvsz.json 'Package / Case'` |
| Steering by package (top) | SOT-23-6 55; 10-UFDFN 53; 8-SOIC 28; SOT-363 24; 6-UFDFN EP 17; TO-253-4 16; SOT-563/666 15; 6-UFDFN 14; 10-MSOP 14; 9-UDFN 13; SOD-323 13; 62 values in all | `S raw/tvss.json 'Package / Case'` |
| Steering lines (unidirectional channels) | 4: 224, 2: 84, 8: 31, 3: 28, 1: 22, 6: 16, 7: 3, 5: 1 | `S raw/tvss.json 'Unidirectional Channels'` |
| Zener type, unidirectional / bidirectional channel 1 | 11,376 / 9,787 (multi-channel rows: unidirectional 2–24 and bidirectional 2–8 channels, 1,173 and 599 rows) | `S raw/tvsz.json 'Unidirectional Channels' 'Bidirectional Channels'` |
| Grades, Zener type | Automotive 4,822; Military 107 (out) | `S raw/tvsz.json Grade` |
| Grades, Steering | Automotive 60; no Military | `S raw/tvss.json Grade` |
| Listing pages needed, lower bound | **76** leaf pages (⌈3252/100⌉ 33 + ⌈2508/100⌉ 26 + ⌈1365/100⌉ 14 + 3 steering pieces), plus one page per split node | arithmetic on the maker counts above; a page holds at most 100 products (`python3 -c "import json;print(len(json.load(open('planning/plan-69-data/raw/tvsz.json'))['firstPage']))"` → 100) |

### Verified properties

- **A category page carries every parameter A needs.** Each product of
  `__NEXT_DATA__` has the maker part number, the datasheet link, its own `In
  Stock` quantity, status, Type (`183`), Unidirectional / Bidirectional Channels
  (`1729` / `1730`), Voltage - Reverse Standoff (`761`, e.g. `5V (Max)`),
  Breakdown (Min) (`1731`), Clamping (Max) @ Ipp (`1732`), Current - Peak Pulse
  (`714`, e.g. `6A (8/20µs)`), Power - Peak Pulse (`1733`), Capacitance @ Frequency
  (`566`), Grade (`1977`), Qualification (`2700`), Mounting Type (`69`), Package /
  Case (`16`) and Supplier Device Package (`1291`) (read from the planning page
  `D5V0L1B2WS-7`, 2026-11-25).
- **A page shows at most 100 products and pagination does not work from a script**
  (plan-58-A: page links and filter clicks do nothing in Safari's temporary tab);
  every page's facets carry the counts inside its own filter, so a piece over 100
  rows is split by URL into children of a single-valued facet.
- **The default order is not by stock** (the planning page's first rows are
  819,543, 407,209, 69,844, 277,005 …), so the 200 most-stocked rows of a maker
  are known only after its whole listing.
- **Maker hosts** (Python `urllib`, Safari user agent, 2026-11-25):
  `www.littelfuse.com/media?resourcetype=datasheets&…` answers `403` with the user
  agent alone and `200 application/pdf` (779,384 bytes, the SMAJ datasheet) with
  `Accept` and `Accept-Language` added; `www.vishay.com/docs/81796/vesd05a1.pdf`
  and `www.diodes.com/assets/Datasheets/DT1240-04LP.pdf` answer a PDF with the user
  agent alone. Semtech's DigiKey links are `semtech.my.salesforce.com/sfc/p/…`
  distribution pages, the same links `semtech.com`'s product pages give: `curl`
  gets an auto-submitting form, and its POST answer is a Lightning viewer that
  builds the download link in JavaScript (no `/sfc/dist/version/download` link in
  the 53,962-byte HTML). Loaded in Safari, the page names the file's `versionId` and
  `contentId`, and `curl` of its `rendition=ORIGINAL_Pdf` URL gets the PDF (§ 3.4,
  checked in § Prerequisites).

## 3. Method

### 3.1 Scope (`census/tvs-common.py`, from plan-48's `common.py`)

A row is in scope when: its maker label is one of the type's three (Littelfuse
`18`; Vishay General Semiconductor – Diodes Division `112`; YAGEO `13`; Semtech
`600`; Diodes Incorporated `31`); it has an `In Stock` line with a quantity above
0 (the `In Stock` line only; a Marketplace or Factory line does not count:
plan-58-A); status Active or Last Time Buy; Grade not Military and no MIL-PRF
qualification; Mounting Type Surface Mount, Surface Mount Wettable Flank or Through
Hole. Everything else is `out` with its reason.

### 3.2 The listing (`census/tvs-list.sh`, `census/tvs-split.py`)

- Six roots: (Zener, each of its three makers) and (Steering, each of its three),
  each filtered In Stock (`-2` 5), status Active and Last Time Buy (`1989` 0 and 4)
  and its maker (`-1`), on category 144 with Type `183`. `tvs-urls.js` (from
  `census/url.js`, with the status pair) writes each piece's URL.
- `tvs-list.sh` loads a piece (`census/facets.sh`'s JavaScript, saved to
  `listing/<piece>.json`). A piece with `n` ≤ 100 is a `leaf` and its products are
  the rows. A piece above 100 is `split`: `tvs-split.py` picks, among the
  single-valued facets `Package / Case` (`16`), `Voltage - Reverse Standoff`
  (`761`) and `Series` (`-4`), in that order, the first whose options' counts add
  up to the piece's `n` and whose largest option is under the piece's `n`; each
  option becomes a child piece with that filter added (`"pt": "String"` for a
  numeric key). A child above 100 is split again with the next facet.
- A load that returns another tab's page, an empty file or `Just a moment...` is
  retried up to five times (plan-58's `check.py` rule); a piece that still fails is
  `failed`, and A does not continue past Phase 1 with a failed piece.
- `tvs-build.py` merges the leaves into `candidates.tsv` (one row per distinct
  maker part number; a part on two leaves is one row) and checks `sum(rows of
  leaves) == n of the root` per root.

### 3.3 The pick (`census/tvs-select.py`, from plan-48's `select.py`)

Within each (type, company), the in-scope rows by falling `stock`: Zener type, the
first 200 are `candidate` and the rest `reserve`; Steering, every in-scope row is
`candidate`. `rank` is the row's place in that order.

### 3.4 Datasheet URLs (`census/tvs-urlcheck.py`, from plan-48's `urlcheck.py`)

- Littelfuse, Vishay, YAGEO, Diodes: DigiKey's link when it is on the maker's host;
  fetched with Python `urllib`, a Safari user agent and `Accept` /
  `Accept-Language` headers (§ 2). A DigiKey-hosted link (`mm.digikey.com`) is
  replaced by the maker's URL for the same file name when the maker's host answers
  a PDF for `https://www.vishay.com/docs/<id>/<name>.pdf` (Vishay: the id and name
  from another row of the same series) or `https://www.diodes.com/assets/Datasheets/<base>.pdf`
  (Diodes: the part number without the packing suffix `-7`, `-7B`, `-13`, `-13-F`, `-F`).
- Semtech: `census/tvs-semtech.sh` loads each distinct distribution URL in Safari
  (`scripts/safari-digikey.sh <url> 25 '<the JavaScript of § Prerequisites>'`), which
  prints the file's `versionId` and `contentId`. It then downloads
  `…/sfc/dist/version/renditionDownload?rendition=ORIGINAL_Pdf&versionId=<v>&operationContext=DELIVERY&contentId=<c>&page=0&d=/a/<a>/<token>&oid=00DE0000000JelG&dpt=null&viewId=`
  with `curl -sL -A "Mozilla/5.0"`, where `<a>/<token>` are the distribution URL's last
  two path parts. The datasheet's `url` in `datasheets.json` is the distribution URL
  (Semtech's own link, as `semtech.com` gives it).
- A row with no URL that answers a PDF is `no datasheet` (`out`), and the next
  reserve row moves up.

## 4. Verdict rules (`census/tvs-verdicts.py`, Phase 3)

Per picked row, in this order; the first that applies decides:

1. Scope or stock (§ 3.1) failed → `out`, with the scope reason.
2. **Ordering table.** `add` needs the part number in the PDF's text
   (`pdftotext -layout`, spaces removed, upper case): verbatim (`hit` 1), or by the
   maker's printed packing rule (`scheme`): Littelfuse — the base number before a
   packing suffix the datasheet's Part Numbering / Ordering section prints (`-T`,
   `-TP`, `-B`, `-13`, `TR`, `TG`, `UTG`, `ATG`, `JTG`, `XTG`, `FTG`, `ETG`); Vishay —
   `<base>-E3/<code>`, `<base>-M3/<code>`, `<base>HE3/<code>`, `<base>-HM3/<code>`
   and `<base>-G3-08` / `-GS08` / `-GS18` when the datasheet's Ordering Information
   table prints that pattern with the code; YAGEO — the base number before a
   packing suffix its Ordering Information prints; Semtech — `.TCT`, `.TNT`, `.TZT`,
   `.TBT` read as the datasheet's `RClamp<n>` ordering lines; Diodes — the base
   number before `-7`, `-7B`, `-13`, `-13-F`, `-F`, `Q-7`, `Q-13`. A part whose
   number the PDF prints only as a series range (`SMBJ5.0A – SMBJ440CA`) is `add`
   when its exact number appears in the electrical table. Otherwise `skip`, `not in
   the datasheet's ordering table`.
3. **Status words.** `OBSOLETE`, `DISCONTINUED`, `NRND`, `NOT RECOMMENDED FOR NEW
   DESIGNS`, `END OF LIFE`, `LAST TIME BUY` or `LIFEBUY` within 300 characters after
   the part number → `out`, `datasheet status`. DigiKey's Last Time Buy status
   alone keeps a row in.
4. Otherwise `add`.

Every `skip` row goes to `datasheets/skipped-parts.md` § TVS diodes (plan-69), with
its reason (B writes it). A PDF whose every listed row is `out` by rule 3 is not
placed and goes to `datasheets/obsolete-not-included.md` (B), with the family
`tvs`, the category `Discrete` and the group `TVS diodes`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The Safari listing

- [ ] `planning/plan-69-data/census/tvs-urls.js`, `tvs-list.sh`, `tvs-split.py`,
      `tvs-check.py` (from plan-58's `make-urls.js`, `list.sh`, `check.py`) per § 3.2;
      `tvs-urls.tsv` holds the six roots.
- [ ] Run `census/tvs-list.sh listing` until every piece is `leaf` or `split`;
      record the page loads and Cloudflare stops in § Corrections.
- [ ] `census/tvs-build.py listing candidates.tsv` per § 3.2 and § 3.1.

Acceptance: the listing is whole.
  Check: `python3 -I planning/plan-69-data/census/tvs-build.py --verify planning/plan-69-data/listing` → `6 roots, 0 failed, leaves add up` and per root `<type> <maker> n=<n> rows=<n>`; the six `n` go in § Corrections beside the planning counts 3,252 / 2,508 / 1,365 / 88 / 34 / 33 (est. 1–2 h: at least 82 page loads, § 2, with a Cloudflare tick after each run of about 100 loads as plan-58-A recorded; nothing smaller lists a maker whose rows exceed one page).
Commit:

### Phase 2 — The pick, URLs, the text scan

- [ ] `census/tvs-select.py candidates.tsv --report` per § 3.3; record the per-(type,
      company) counts of `candidate` and `reserve` here.
- [ ] `census/tvs-urlcheck.py candidates.tsv /tmp/p69/pdf > url-check.tsv` per § 3.4
      for every `candidate` and `reserve` row; `census/tvs-semtech.sh` for Semtech's
      distribution URLs into the same cache.
- [ ] `census/tvs-textscan.py candidates.tsv /tmp/p69/pdf textscan.tsv` (from plan-48's
      `textscan.py`, with § 4's packing rules and status words); read the `context`
      of every `scheme` and every `dead` row; a wrong match goes to
      `verdict-overrides.tsv` (`mpn`, `verdict`, `reason`, `why`).

Acceptance: every picked row has a scan result.
  Check: `awk -F'\t' 'NR>1' planning/plan-69-data/textscan.tsv | wc -l` equals `awk -F'\t' 'NR>1 && $24=="candidate"' planning/plan-69-data/candidates.tsv | wc -l` (est. 20 min: downloads are cached in `/tmp/p69/pdf`).
Commit:

### Phase 3 — Verdicts, refill, placement, skeletons

- [ ] `census/tvs-verdicts.py candidates.tsv textscan.tsv verdicts.tsv` per § 4,
      `verdict-overrides.tsv` applied last; `census/tvs-refill.sh` (select →
      urlcheck → textscan → verdicts) until every picked row is `add` or no reserve
      row is left in its (type, company); the counts go in § Corrections.
- [ ] `census/tvs-place.py candidates.tsv /tmp/p69/pdf` (from plan-48's `place.py`):
      each distinct PDF (by SHA-256) an `add` row cites goes to
      `datasheets/tvs/<company>/<name>.pdf` (`<name>`: the URL's last path segment
      in lower case; Littelfuse: the `filename` query value; Semtech: the part
      family, `rclamp0524p`; `.pdf` appended when missing), each folder gets its
      `datasheets.tsv` (`file`, `url`, `bytes`, `sha256`), and `pdf` is set.
- [ ] `census/tvs-specs-skeleton.py` (from plan-48's `specs-skeleton.py`) writes
      `specs.tsv`; `census/tvs-footprint-lines.py` (from plan-48's
      `footprint-lines.py`) writes `footprints-needed.txt` (letter `C` for a part
      with two terminals or a two-pad chip package, `D` for every other package).
- [ ] Update `planning/plan-69-data/README.md` with anything the scripts changed.

Acceptance: every picked row is `add`, its PDF is in the tree, and each has a spec
row and a package line.
  Check: `python3 -I planning/plan-69-data/census/tvs-place.py --verify planning/plan-69-data/candidates.tsv` → `0 missing, 0 unused`; `awk -F'\t' 'NR>1 && $2=="add"' planning/plan-69-data/verdicts.tsv | wc -l` equals the `candidate` count; `awk -F'\t' 'NR>1{s+=$8} END{print s}' planning/plan-69-data/specs.tsv` equals it too (est. 10 min).
Commit:

## Validation Plan

- Tests: none (no product code changes); the checks are the three acceptance
  commands.
- Coverage check: `git status --short backend schematical` → empty after A.
- Runtime proof: `tvs-select.py --report` prints the final per-(type, company)
  counts; they go in § Corrections.
- Doc sync: `planning/plan-69-data/README.md`.
- Final gate: plan-69-I.

## Decisions

- Owner, 2026-11-25: **backend letters wait for plan-68.** The data, datasheet, spec and footprint letters run now; the first letter that changes backend or app code (E) lists plan-68 complete as a prerequisite, and every code letter meets plan-68's size and coverage checks (`node scripts/source-size.js --check`, `node scripts/coverage.js --check`), which the landing letter runs once over all tiers.
- Owner, 2026-11-25: **the top three makers per type by DigiKey in-stock count,
  Active and Last Time Buy, at most the 200 most-stocked per maker** (census
  README, decision 1).
- Owner, 2026-11-25: **TVS / ESD parts are facts only in play mode** (decision 3).
- Author: **the types are DigiKey's Type facet** (Zener, Steering), ranked
  separately, as plan-48 ranked timers and op-amps; a part with Type `-` (216
  rows) is in neither and out.
- Author: **the listing is split by URL, not paged** (§ 2): plan-58's method; each
  row's own `In Stock` line is its stock check, so no separate lookup follows.
- Author: **Semtech PDFs come through Safari** (§ 3.4); the `datasheets.json` URL is
  Semtech's own distribution link, the one its site gives.
- Author: **`datasheets/tvs/<company>/`**, as plan-48 named folders by kind.

## Corrections

## Summary

The work is the listing: at least 82 Safari page loads, split by facet, before any
pick can be made, because DigiKey's page order is not by stock. The risk then sits
in § 4 rule 2 (series datasheets print part numbers as families and packing
schemes), so every `scheme` match is read in context.
