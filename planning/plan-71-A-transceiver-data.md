# plan-71-A: Line transceivers — the Safari listing, the top-200 pick, part-by-part stock, datasheets, verdicts

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-71 runs A → B → C → D → E → F → G → H → I → J.
Effort: large (3h–1d, plus Cloudflare ticks by the owner)
Depends on: nothing (§ Prerequisites)

plan-71 adds **CAN, RS-485 / RS-422, RS-232 and LIN line transceivers** to
the parts library and the app as a new kind, `transceiver`. It takes the top
three makers of each protocol at DigiKey by in-stock count, each capped at its
200 most-stocked parts (owner decisions 1 and 2,
`planning/plan-69-data/census/README.md`):

| Protocol | Makers (rank order) | In scope |
|---|---|---|
| CAN | **Texas Instruments**, **NXP**, **Microchip** | DigiKey Protocol "CANbus": CAN and CAN FD transceivers, with or without standby / silent pins, a VIO pin, or an isolation barrier |
| RS-485 | **Analog Devices** (with Maxim), **Texas Instruments**, **MaxLinear** | Protocols "RS422, RS485" and "RS485": half- and full-duplex transceivers, RS-422 drivers and receivers, isolated parts |
| RS-232 | **Analog Devices** (with Maxim), **Texas Instruments**, **Renesas** | Protocol "RS232": charge-pump line drivers / receivers (MAX232 / MAX3232 type) and drivers or receivers only |
| LIN | **Texas Instruments**, **NXP**, **onsemi** | Protocols "LIN" and "LINbus": LIN transceivers, with or without an integrated regulator |

In play mode a transceiver is logic. A driver turns its TXD / DI / T_IN
input into the line level and a receiver turns the line back. CAN is a
dominant / recessive wired-AND, RS-485 a driver-enabled differential pair,
RS-232 an inverting driver and receiver, and LIN a single-wire bus pulled up
inside the part and pulled low by any driver. So two USB-UART bridges
(`MCP2200`, the `usbuart` model) joined through two CAN transceivers pass
bytes from one to the other. The line pins are analog pins for the voltage
check (G § 3.4).

The single behavioral outcome of plan-71: loading the part files gives the
baseline parts (§ Prerequisites, P0), plus four generics (`GENERIC-CAN`,
`GENERIC-RS485`, `GENERIC-RS232`, `GENERIC-LIN`), plus one part per `add`
row of `planning/plan-71-data/candidates.tsv`. At planning time that is up
to 1,506 picked rows, before verdicts. The app lists them under
**Interface → Transceiver → CAN / LIN / RS-232 / RS-485**. In play mode a
byte typed into one `MCP2200` arrives at a second `MCP2200` across two
`GENERIC-CAN` parts, and the same holds for RS-485, RS-232 and LIN. `ctest`
passes T0 + 1 programs (`transceiver_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | Safari listing of the 12 (maker, protocol) pieces, the top-200 pick, the part-by-part In Stock check, datasheet text scan and verdicts, datasheet placement, `specs.tsv` skeleton | large + Cloudflare ticks |
| B | specs: pinouts and facts per (PDF, package), CAN and LIN; the shared spec tools | large |
| C | specs: RS-485 / RS-422 | large |
| D | specs: RS-232; the final pick, skipped parts, obsolete PDFs, the final `footprints-needed.txt` | large |
| E | footprints: leaded packages (SOIC, DIP, SSOP, TSSOP, MSOP, SOT-23) | large |
| F | footprints: leadless packages (DFN, SON, QFN, WQFN, CSP) | large |
| G | backend: the `transceiver` kind and model (on plan-75's shared supply-domain loader), `transceiver_test`, docs | large |
| H | parts: the four generics, `transceiver.json`, datasheets, footprints, goldens, the Part Editor's kind lists | large |
| I | app: details, facts, kind names, the drawer, the `transceiver-app` harness | large |
| J | landing tier | small |

Outcome of A:

- `planning/plan-71-data/listing/` holds every in-stock Active part of the 12
  pieces, one saved page per (maker, protocol, split), each under DigiKey's
  100-row page.
- `candidates.tsv` ranks each piece by the product's own In Stock quantity.
  It marks the first 200 (or all) `candidate` and the rest `reserve`.
- `stock-check.tsv` records each candidate's In Stock line, which must be above 0.
- `verdicts.tsv` gives each candidate `add`, or `skip` / `out` with a § 4
  reason from its datasheet text.
- `datasheets/drivers-receivers-transceivers/<maker>/` holds every PDF an
  `add` row cites, with a `datasheets.tsv` per folder.
- `specs.tsv` has one row per (protocol, maker, PDF, package) of the `add`
  rows, with its pinout and fact columns left empty.

References:

- `AGENTS.md` (§ Datasheets: stock in Safari, in-stock only, obsolete PDFs);
  `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints);
  `docs/backend.md` (§ Adding a kind).
- `planning/plan-69-data/census/README.md`: the shared census, the owner's
  decisions, the tools `url.js`, `facets.sh`, `show.py`, `kw.js`.
- `planning/plan-71-data/README.md` and `raw/`: this plan's planning counts.
- `planning/completed/plan-58-A-slide-switch-data.md` and
  `planning/plan-58-data/census/` (`make-urls.js`, `list.sh`, `check.py`,
  `build.py`, `textscan.py`, `place.py`, `specs-skeleton.py`): the Safari
  listing method A copies.
- `planning/completed/plan-48-A-timer-opamp-data.md` (§ 4 verdict rules, the
  reserve-row rule), `planning/plan-48-data/census/textscan.py`, `place.py`.
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p71 && cmake --build /tmp/logic-p71 -j8 && ctest --test-dir /tmp/logic-p71 -j8` → `100% tests passed` | MET (2026-11-25 at 63e2f7d2, measured by the author at planning: `100% tests passed, 0 tests failed out of 77`; **T0 = 77**, `grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | at planning (63e2f7d2): **P0 = 40832**, **R0 = 40832**, **F0 = 3993**, **E0 = 40832**. G and H re-measure them at their start and use their own numbers. |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p71-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-25 at 63e2f7d2: `** BUILD SUCCEEDED **`, run by the author: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO build`) |
| The kind does not exist | `grep -c 'PART_TRANSCEIVER' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/transceiver.json` → no such file; `ls -A datasheets/drivers-receivers-transceivers \| wc -l` → 0 | MET (2026-11-25: 0; no such file; 0. The folder exists, empty and untracked: `git ls-files datasheets/drivers-receivers-transceivers \| wc -l` → 0) |
| No other open plan adds the `transceiver` kind or these parts | `grep -l -E 'PART_TRANSCEIVER\|Parts/transceiver\.json\|"kind": "transceiver"\|datasheets/drivers-receivers-transceivers' planning/plan-*.md \| grep -v plan-71 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 69 (`tvs`), 70 (`isolator`), 72 (`ioexpander`, `i2cbus`), 73 (serial-kind USB bridges) and 74 (`ethernet`) are being written at the same time. They touch the same shared lines (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, the drawer table in `docs/parts-format.md`). Each such step here adds its line only if absent, after whatever is last, and greps first (G Phase 1, H Phase 1, I Phase 1). |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | a rule, applied per footprint in E and F |
| Safari can reach DigiKey | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/710" 10 'document.title'` → `Drivers, Receivers, Transceivers \| Integrated Circuits (ICs) \| DigiKey` | MET (2026-11-25, census). Cloudflare re-challenges after a few dozen loads ("Just a moment..."). The owner then ticks the box in a DigiKey tab brought to the front: `osascript -e 'tell application "Safari" to activate'`, then open `https://www.digikey.com/en/products/filter/x/710` in a new tab of window 1. |
| lz-string for the filter URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls node_modules/lz-string` → `README.md …` | MET at planning (the census used it) |
| The maker sites answer Python's `urllib` | one PDF per maker by `planning/plan-71-data/census/fetch.py` (Phase 4) | MET (2026-11-25: TI, ADI, MaxLinear, NXP, Microchip, onsemi and Renesas each served a `%PDF-` file; `planning/plan-71-data/README.md` § Datasheet hosts) |
| `pdftotext`, `pdfinfo` | `which pdftotext pdfinfo` → two paths | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-71-data/listing/` has, for each of the 12 (maker, protocol)
  pieces of § 2, saved pages whose rows add up to the piece's facet count.
  Each page is a URL whose `productCount` is at most 100.
  `python3 -I planning/plan-71-data/census/check-pieces.py` prints
  `12 pieces, 0 short`.
- `candidates.tsv` has one row per listed part: protocol, maker, part number,
  In Stock quantity (the product's own `In Stock` line), datasheet URL,
  package, DigiKey's Data Rate and Grade parameters, rank, and `state`.
  `state` is `candidate` for the first 200 of a piece by In Stock (all of
  them under 200) and `reserve` for the rest. At planning time that is 1,506
  candidates (§ 2).
- `stock-check.tsv` has one row per candidate with `found` 1 and stock above
  0. A row with In Stock 0 (only Marketplace or Factory stock) is `out`
  (reason `stock check`), and the next `reserve` row of its piece moves up.
- `verdicts.tsv` has one row per candidate: `add`, or `skip` / `out` with a
  § 4 reason. A `skip` or `out` moves the next reserve row up, which gets its
  stock check and verdict in turn, until the piece has its 200 `add` rows (or
  every row it has) or no reserve row is left.
- `datasheets/drivers-receivers-transceivers/{ti,nxp,microchip,adi,maxlinear,renesas,onsemi}/`
  holds each PDF at least one `add` row cites. Each folder's `datasheets.tsv`
  maps each file to its URL, size and SHA-256.
- `specs.tsv` exists with one row per (protocol, maker, PDF, package) of the
  `add` rows, its `pinout` and fact columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No row is final in this letter: B–D read the pinouts, and a row whose
  package has no pinout in its PDF is skipped there.
- No DigiKey API (`scripts/digikey.js`): stock is looked up in Safari
  (AGENTS.md).
- Out of scope, recorded as `out` with their reason (§ 4):
  - multiprotocol parts (DigiKey "RS232, RS422, RS485", "RS232, RS485",
    "RS422, RS423"), which are not listed;
  - UARTs with a built-in transceiver (MAX3140, already in `serial`);
  - CAN or LIN system basis chips (an SPI port, a watchdog or a supervisor
    beside the transceiver);
  - CAN controllers with a transceiver (MCP25625);
  - bus extenders built on a CAN physical layer (LT3960, I2C over CAN);
  - military and hi-rel parts (Grade Military, MIL-STD-883, ceramic, flatpack).
- The legacy labels DigiKey keeps for Rochester stock (Harris, Intersil,
  National Semiconductor, Sipex, Fairchild, Elantec) are not makers.

## 2. Current State

The library has no line transceiver. `buffer` holds the 74x245 bus
transceivers (`backend/src/models/buffer.c:1-14`), and their datasheets are
in `datasheets/transceiver/`. That is why this plan's PDFs go to
`datasheets/drivers-receivers-transceivers/`, DigiKey's slug for category
710. The MAX3140 (a UART with an RS-485 transceiver) is a `serial` part
(`backend/src/models/serial_max3100.c:33-41`), and its receiver rules are the
reading G's RS-485 model copies. plan-43 found five CAN, LIN and RS-232
reference-board parts (`MCP2562-E/SN`, `TJA1028TK/5V0/10/Z`, `TJA1052IT/5Y`,
`ST3232BDR`, `SN65HVD232QDRG4Q1`; `planning/plan-43-data/candidates.tsv`,
family `transceiver`). The `planning/todo.md` item "transceiver: 12 parts"
stays open: the ones this pick covers are added here, and ST3232BDR's maker
(STMicroelectronics) is not in it.

### Measured populations

All DigiKey counts are from 2026-11-25, In Stock and Active, Safari facet
counts (`planning/plan-69-data/census/README.md`; raw pages in
`planning/plan-71-data/raw/`). Command per row:
`python3 -I planning/plan-69-data/census/show.py planning/plan-71-data/raw/<file> Manufacturer`.

| Piece | File | Facet count | Picked (≤ 200) | Pages at 100 rows (lower bound) |
|---|---|---|---|---|
| CAN · TI (296) | `can.json` | 204 | 200 | 3 |
| CAN · NXP (568) | `can.json` | 108 | 108 | 2 |
| CAN · Microchip (150) | `can.json` | 95 | 95 | 1 |
| RS-485 · ADI (505 + 175) | `rs485.json` | 415 + 717 = 1,132 | 200 | 5 + 8 = 13 |
| RS-485 · TI | `rs485.json` | 334 | 200 | 4 |
| RS-485 · MaxLinear (1016) | `rs485.json` | 111 | 111 | 2 |
| RS-232 · ADI (505 + 175) | `rs232.json` | 229 + 505 = 734 | 200 | 3 + 6 = 9 |
| RS-232 · TI | `rs232.json` | 251 | 200 | 3 |
| RS-232 · Renesas (20) | `rs232.json` | 101 | 101 | 2 |
| LIN · TI | `lin.json` | 39 | 39 | 1 |
| LIN · NXP | `lin.json` | 34 | 34 | 1 |
| LIN · onsemi (488) | `lin.json` | 18 | 18 | 1 |
| **total** | | **3,161** listed | **1,506** | **42** |

A piece over 100 rows is split by the Package / Case facet (key `16`,
`"pt": "String"`, the plan-58 method). A package value still over 100 is
split again by Series (`-4`). DigiKey's script-driven pagination does not
work in Safari's temporary tab (`planning/completed/plan-58-A-slide-switch-data.md:127`),
so every page is its own filtered URL. Cloudflare challenged the census
after 25 to 35 loads on 2026-11-25. The 42 or more loads therefore take two
or three owner ticks.

## 3. Method

Scripts in `planning/plan-71-data/census/`, copied from plan-58's and
plan-48's:

- `make-urls.js` writes `urls.tsv`, one URL per (protocol, maker, split),
  from the facet ids of `raw/*.json` (Manufacturer `-1`, Protocol `578`,
  Package / Case `16`, Series `-4`). First pass: one URL per piece. A piece
  whose page reports more than 100 rows gets one URL per Package / Case
  value, from that page's own facet.
- `list.sh` loads each URL in Safari (`scripts/safari-digikey.sh`) and saves
  the `__NEXT_DATA__` envelope to `listing/<protocol>_<maker>_<split>.json`,
  with plan-58's `check.py` retry (another tab's page, an empty page, or a
  Cloudflare page means retry).
- `check-pieces.py` checks that each piece's saved rows add up to its
  unsplit facet count.
- `build.py` writes `candidates.tsv`. For each row it records the product's
  own `In Stock` quantity line (Marketplace and Factory lines are not stock;
  the plan-58 rule), the datasheet URL, Package / Case, Supplier Device
  Package, Data Rate, Grade and Qualification. It ranks each piece by In
  Stock (descending, then part number) and marks candidates and reserves.
- `stock-check.tsv` comes from the same rows. The listing *is* the
  part-by-part check, because every row carries its own In Stock line.
- `fetch.py` downloads each distinct candidate PDF with urllib (the probe's
  headers), rewriting `mm.digikey.com` MaxLinear links to
  `https://www.maxlinear.com/ds/<series lower>.pdf`. A URL that serves HTML
  falls back to `scripts/safari-digikey-pdf.sh`. `urlcheck.tsv` records
  each URL's result.
- `textscan.py` (plan-48's) extracts each PDF's ordering table and status
  words; `verdicts.py` applies § 4; `place.py` moves PDFs into
  `datasheets/drivers-receivers-transceivers/<maker>/` and writes
  `datasheets.tsv`; `specs-skeleton.py` writes `specs.tsv`.

## 4. Verdict rules

A candidate is `add` unless one of these holds, checked in order:

| State | Reason | Test |
|---|---|---|
| `out` | `stock check` | In Stock line 0 or absent |
| `out` | `military` | Grade Military, or the part number or datasheet names MIL-STD-883, ceramic, flatpack |
| `out` | `system basis chip` | the datasheet's pin table has SPI pins (SCK, SDI, SDO, CSN) or a watchdog (WDI, WD) beside the transceiver |
| `out` | `controller` | the part is a CAN controller with an SPI port (MCP25625) |
| `out` | `not a line transceiver` | a CAN-PHY bus extender (LT3960), or a part whose function (datasheet p.1) is not a driver and/or receiver of its protocol |
| `out` | `multiprotocol` | the datasheet's p.1 names RS-232 and RS-485 / RS-422 modes of one part |
| `out` | `no datasheet` | no URL of the row serves a PDF after the § 3 fallbacks |
| `skip` | `not in ordering table` | the full part number (with its package and reel suffixes) is not in the PDF's ordering or package-option table |
| `skip` | `no pinout` | (B–D) the PDF prints no pinout for the row's package |

Each `skip` and `out` gets a line in `datasheets/skipped-parts.md` § Line
transceivers (plan-71) (D writes it). A PDF whose every listed part is
marked obsolete or "NRND" (not for new designs) is deleted and recorded
in `datasheets/obsolete-not-included.md` (family Interface, category
Transceiver, group = the protocol). A PDF where only some parts are obsolete
keeps all its parts, and the obsolete ones get `"flags": ["obsolete"]` (H).
AEC-Q100 parts are kept and flagged `"automotive"` (`docs/parts-format.md`
§ Parts).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The listing

- [ ] `planning/plan-71-data/census/`: `make-urls.js`, `list.sh`, `check.py`,
      `check-pieces.py` (copied from plan-58 and adapted per § 3).
- [ ] First pass: 12 URLs; save each page to `listing/`; for each piece over
      100 rows, write its Package / Case split URLs and list them.
- [ ] `check-pieces.py` → `12 pieces, 0 short`.

Acceptance: the listing is complete for every piece.
  Check: `python3 -I planning/plan-71-data/census/check-pieces.py` → `12 pieces, 0 short` (est. < 1 min; the listing itself is 42 or more Safari loads, about 15 s each, plus the owner's Cloudflare ticks).
  Result:
Commit:

### Phase 2 — Candidates and the stock check

- [ ] `build.py` → `candidates.tsv`, `stock-check.tsv`.

Acceptance: each piece has min(200, its row count) candidates, each with In
Stock above 0, or a recorded `out` and a reserve moved up.
  Check: `awk -F'\t' 'NR>1 && $NF=="candidate"' planning/plan-71-data/candidates.tsv | cut -f1,2 | sort | uniq -c` → 12 lines matching § 2's Picked column (the moved-up reserves keep the counts); `awk -F'\t' 'NR>1 && $4<=0' planning/plan-71-data/stock-check.tsv | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

### Phase 3 — Datasheets

- [ ] `fetch.py` → every distinct candidate URL; `urlcheck.tsv`.
- [ ] Each URL that fails gets the § 3 fallbacks; still failing → `out: no
      datasheet`, and a reserve row moves up.

Acceptance: every candidate's PDF is on disk.
  Check: `awk -F'\t' 'NR>1 && $4!=1' planning/plan-71-data/urlcheck.tsv | wc -l` → the number of `no datasheet` rows in `verdicts.tsv` (est. < 1 min).
  Result:
Commit:

### Phase 4 — Verdicts and placement

- [ ] `textscan.py`, `verdicts.py` → `verdicts.tsv` per § 4; reserve rows
      moved up and run through Phases 2–4 until no piece changes.
- [ ] `place.py` → `datasheets/drivers-receivers-transceivers/<maker>/` and
      each folder's `datasheets.tsv`.
- [ ] `specs-skeleton.py` → `specs.tsv`.

Acceptance: every candidate has a verdict, and every `add` row's PDF is placed.
  Check: `python3 -I planning/plan-71-data/census/verdicts.py --check` → `<n> candidates, 0 without a verdict, 0 add rows without a placed PDF`; `head -1 planning/plan-71-data/specs.tsv` → `protocol company pdf package supplier_package rows example scheme label pinout facts` (est. < 1 min).
  Result:
Commit:

## Decisions

- Owner, 2026-11-25: **the top three makers per protocol at DigiKey by in-stock count, Active and Last Time Buy, the 200 most-stocked per maker** (census README decision 1).
- Owner, 2026-11-25: **RS-232 and LIN are in** besides CAN and RS-485 (decision 2), and **a transceiver is logic in play mode** (decision 3).
- Author: **ADI is one maker** (its two DigiKey ids), as in plan-48.
- Owner, 2026-11-25: **backend letters wait for plan-68.** The data, datasheet, spec and footprint letters run now; the first letter that changes backend or app code (G) lists plan-68 complete as a prerequisite, and every code letter meets plan-68's size and coverage checks (`node scripts/source-size.js --check`, `node scripts/coverage.js --check`), which the landing letter runs once over all tiers.

## Corrections

(none yet)
