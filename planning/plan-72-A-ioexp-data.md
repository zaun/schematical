# plan-72-A: IO expanders and I2C buffers/muxes — Safari listing, part-by-part stock check, datasheet downloads, verdicts

Last updated: 2026-11-25
Overall Effort: huge (> 3d) — plan-72 is A → B → C → D → E → F → G → H → I → J
Effort: large (3h–1d of work, about 40 Safari page loads)
Depends on: nothing (§ Prerequisites)

plan-72 adds **I/O expanders** and **I2C bus buffers, repeaters, accelerators,
muxes and switches** to the parts library and the app (`planning/todo.md:108`
"Add IO Extenders"; owner request of 2026-11-25, decisions in
`planning/plan-69-data/census/README.md`). Makers, by DigiKey in-stock Active
count at planning (§ 2):

| Family | Makers (rank order) | Kind |
|---|---|---|
| I/O expander | **Texas Instruments**, **Analog Devices** (with Maxim), **NXP**, **Microchip** (owner: 4th maker, for MCP23017/MCP23S17) | `ioexpander` |
| I2C buffer / repeater / accelerator / hot-swap / bus extender | **Analog Devices** (with Maxim), **NXP**, **Texas Instruments** | `i2cbus` |
| I2C mux / switch | **Texas Instruments** (TCA954x), **Diodes Inc.** (PI4MSD5V954x), **NXP** (PCA984x) | `i2cbus` |

In play mode an expander is an I2C or SPI target on the converter engine's
bus front ends: a host's writes set its output latches, direction, polarity
and pull registers, its port pins drive (or release) the nets, a read returns
the pins' levels, and its INT pin goes active on an input change and clears
the way its datasheet says. An I2C mux closes and opens real engine contacts
between its upstream SDA/SCL and the downstream channel pairs as its control
register says; a buffer joins its two sides while it is enabled; a P82B96-style
buffer splits a bidirectional line into a TX/RX pair. Address translators and
isolated buffers are facts only (`"simulated": false`, plan-72-E § 3).

The single behavioral outcome of plan-72: loading the part files gives the
baseline parts (P0, § Prerequisites) plus one part per `add` row of
`planning/plan-72-data/candidates.tsv`; the app lists them under
**Interface → I/O Expanders → I2C / SPI** and **Interface → I2C Bus → Buffers /
Muxes & Switches**; in play mode a test host that writes `0x00` to a TCA9555's
Configuration Port 0 and `0x01` to its Output Port 0 lights an LED on P00, and a
TCA9548A with control byte `0x04` passes a downstream TCA9555's ACK on SD2/SC2
while an identical TCA9555 on SD0/SC0 stays silent; `ctest` passes T0 + 2 programs (`ioexpander_test`,
`i2cbus_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | Safari listing of the pick, part-by-part stock check, datasheet downloads, text scan, verdicts, `specs.tsv` skeleton | large |
| B | specs: pinouts per (datasheet, package), the register template per device, address tables, `footprints-needed.txt` | large |
| C | footprints for every package `footprints-needed.txt` names that `footprints.json` lacks | large |
| D | backend: the converter engine's `io` section (ports, polarity, pulls, change interrupts), register `next` / `alias`; the `ioexpander` kind; `ioexpander_test`; docs | large |
| E | backend: `io.links` (conditional contacts), `io.splits`, bus style `none` for `i2cbus`; the `i2cbus` kind; `i2cbus_test`; docs | large |
| F | parts: TI and NXP I/O expanders (`gen-parts.py` and the templates), datasheets, goldens | large |
| G | parts: ADI and Microchip I/O expanders, datasheets, goldens | large |
| H | parts: every `i2cbus` part, datasheets, goldens | large |
| I | app: drawer, symbol, details, register window, Part Editor drafts and guides, harness | large |
| J | landing tier | small |

Outcome of A: `planning/plan-72-data/candidates.tsv` lists every in-scope
part of the ten (family, maker) pieces of § 2 with its DigiKey row; every row
is `add`, `skip` or `out` with a reason; `datasheets/ioexpander/<maker>/` and
`datasheets/i2cbus/<maker>/` hold every maker-site PDF an `add` row cites,
with one `datasheets.tsv` per family folder; `specs.tsv` has one row per
(family, maker, PDF, package) of the `add` rows, its pinout and template
columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts,
  § Datasheets, § Footprints, § `converter`, § `leddriver` `ports`);
  `docs/backend.md` (§ Adding a kind).
- `planning/plan-69-data/census/` (`README.md`, `url.js`, `facets.sh`,
  `show.py`, `kw.js`: the planning census of plans 69–74, read-only for this
  plan) and `planning/plan-72-data/raw/` (`c749.json`, `c756.json`,
  `i2cbuf.json`, `c743.json`).
- Precedent, read before starting: `planning/completed/plan-58-A-slide-switch-data.md`
  and `planning/plan-58-data/census/` (`make-urls.js`, `list.sh`, `check.py`,
  `build.py`, `fetch.py`, `textscan.py`, `decide.py`, `place.py`,
  `specs-skeleton.py`: the Safari census this letter copies and adapts; do not
  call them); `planning/completed/plan-48-A-timer-opamp-data.md` (stock check,
  reserve rows, the 200 cap, verdict reasons).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`
  (formats); `datasheets/reference-boards/{driver,bus-switch}/` (plan-43's
  copies of PCF8574ADWR, TCA9548APWR, PCA9617ADPJ, TCA4307DRGR, LTC4317:
  the same parts land here, § 2).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p72 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p72 -j8 && ctest --test-dir /tmp/logic-p72 -j8` → `100% tests passed` | MET at planning by the author (2026-11-25 at 63e2f7d2: `100% tests passed, 0 tests failed out of 77`; **T0 = 77**, `grep -c '^logic_test' backend/CMakeLists.txt` → 77). plan-68 adds tests: A records the new T0 |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt` | at planning (63e2f7d2): **P0 = 40832**, **R0 = 40832**, **F0 = 3993**; D, F, G and H re-measure at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p72-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-25 at 63e2f7d2: `** BUILD SUCCEEDED **`, run by the author: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO build`) |
| Neither kind exists | `grep -c 'PART_IOEXPANDER\|PART_I2CBUS' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/{ioexpander,i2cbus}.json` → no such file; `ls datasheets/ioexpander datasheets/i2cbus` → no such directory | MET (2026-11-25: 0; no file; no directory. `datasheets/i-o-expanders/` exists, empty and untracked; plan-72 does not use it) |
| No other open plan adds these kinds or parts | `grep -l -i -E 'ioexpander\|i2cbus\|PART_IOEXPANDER\|PART_I2CBUS\|TCA9548\|PCF8574\|MCP23017\|PCA9555\|TCA9555\|PCA9515\|P82B96' planning/plan-*.md \| grep -v plan-72 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 69 (`tvs`), 70 (`isolator`), 71 (`transceiver`), 73 (serial-kind USB bridges) and 74 (`ethernet`) are written the same day and add kinds: the lines several plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, the drawer table in `docs/parts-format.md`) are added only if absent and after whatever is last; each such step greps first (D Phase 1, E Phase 1, I Phase 1) |
| No plan-43 leftover is being worked in parallel | `grep -n 'driver: 8 parts\|bus-switch: 17 parts' planning/todo.md` → 2 lines, both `- [ ]` | MET (2026-11-25: `planning/todo.md` lines 218 (bus-switch) and 226 (driver) of the "Reference-board parts still missing" list; plan-72-J rewrites both counts) |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | rule, applied per footprint in C |
| Safari has a usable DigiKey tab | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/749" 10 'document.title'` → `I/O Expanders \| Integrated Circuits (ICs) \| DigiKey` | MET (2026-11-25, during planning; a "Just a moment..." title means the owner ticks Cloudflare's box once in the DigiKey tab) |
| `node`, `lz-string`, `pdftotext`, `pdfinfo` | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && which node pdftotext pdfinfo` → three paths | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-72-data/candidates.tsv`: one row per part of the ten pieces
  of § 2, each with `decision` `add`, `skip` or `out` and a reason; the 200
  cap of § 4.2 applied per (family, maker).
- Every `add` row has DigiKey's own `In Stock` line above zero (Marketplace and
  Factory-only stock do not count) and status Active or Last Time Buy.
- `datasheets/ioexpander/{ti,adi,nxp,microchip}/` and
  `datasheets/i2cbus/{adi,nxp,ti,diodes}/` hold each maker-site PDF at least
  one `add` row cites; `datasheets/ioexpander/datasheets.tsv` and
  `datasheets/i2cbus/datasheets.tsv` map each file to its URL, size and SHA-256.
- `planning/plan-72-data/specs.tsv` exists with one row per (family, maker,
  PDF, package) of the `add` rows.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No new census tool for DigiKey beyond copies of plan-58's; `scripts/digikey.js`
  is not used (AGENTS.md: stock goes through Safari).
- Out of scope, every row `out`: voltage level translators (PCA9306, TXS0102
  and the like: DigiKey's Translators category, not 749/756/743), LED drivers
  and PWM controllers (PCA9685, PCA963x: category 745), keypad-only scanners
  with no GPIO register, military / hi-rel grades, bare die and wafers.
- The legacy labels of § 2 (Rochester stock) are not makers of this plan.

## 2. Current State

The library has no I/O expander or I2C bus part: `grep -c -E '^(PCF857|PCA95|TCA95|TCA64|TCAL|PCAL|MCP230|MCP23S|MAX73|PCA9515|PCA9517|PCA9617|P82B|TCA954|PCA984|PI4MSD|LTC43)' backend/tests/golden/parts.txt` → 6, all six `LTC4358…` ideal-diode power switches (`powerswitch` kind), none of this plan's parts. The closest existing pieces are the converter engine's I2C/SPI targets (`backend/src/models/serial_target.c`), its declarative registers and protocol (`converter.c`, `converter_load.c`), and the `ports` output driver of `leddriver` (`display_out.c`, `docs/parts-format.md` § `leddriver` `ports`): plan-72-D and -E build on them.

plan-43 left five of this plan's parts in the reference-board backlog (`awk -F'\t' 'NR==1{for(i=1;i<=NF;i++)h[$i]=i;next} $h["family"]~/bus-switch|driver/' planning/plan-43-data/candidates.tsv | cut -f3` → PCF8574ADWR (driver), TCA9548APWR, PCA9617ADPJ, TCA4307DRGR, LTC4317IDHC#PBF (bus-switch); 25 rows in the two families). A treats them like any other row; plan-72-J updates the two backlog counts.

### Measured populations

All counts are DigiKey facet counts through Safari, 2026-11-25, In Stock and Active (`planning/plan-69-data/census/README.md`; raw pages `planning/plan-72-data/raw/`). Reproduce: `LZ=/tmp/lz/node_modules planning/plan-69-data/census/facets.sh /tmp/p72/c749.json x/749 && python3 -I planning/plan-69-data/census/show.py /tmp/p72/c749.json Manufacturer`.

| Piece | Category, filter | Count | raw |
|---|---|---|---|
| I/O expanders, all makers | I/O Expanders (749) | 453 | `c749.json` |
| — TI / ADI (175 + 505) / NXP / Microchip | 749, Manufacturer | 101 / 78 (75 + 3) / 65 / 62 | `c749.json` |
| — next makers (not in scope) | 749 | Diodes 23, Nexperia 22, MaxLinear 19, Renesas 16 | `c749.json` |
| I2C buffers etc., all makers | Signal Buffers, Repeaters, Splitters (756), Applications I2C-Hotswap, I2C, HDMI+I2C, I2C+USB, I2C+PCIe+SAS | 148 | `i2cbuf.json` |
| — ADI (505 + 175) / NXP / TI | 756 + that filter, Manufacturer | 92 (91 + 1) / 23 / 13 | `i2cbuf.json` |
| — next maker (not in scope) | 756 | Diodes 10 | `i2cbuf.json` |
| I2C muxes / switches | Signal Switches, Multiplexers, Decoders (743), keyword probes `kw.js` on In Stock + Active | `TCA954` 11 (TI), `PI4MSD` 10 (Diodes), `PCA984` 4 (NXP); `PCA9548`, `PCA9544`, `PCA9543`, `PCA9540`, `PCA9541` 0 each; `LTC4306` 0; `MAX735` 0 | README rows; `c743.json` |

Upper bound of the pick before the stock check and verdicts: 101 + 78 + 65 + 62 + 92 + 23 + 13 + 11 + 10 + 4 = **459** rows (no (family, maker) piece reaches the 200 cap). 743 has no I2C facet (`python3 -I planning/plan-69-data/census/show.py planning/plan-72-data/raw/c743.json` lists Type, Circuit, Independent Circuits and no Interface/Applications facet), so the mux piece is listed by series keyword (§ 4.1).

Packages at stake (the `Supplier Device Package` facet): `python3 -I planning/plan-69-data/census/show.py planning/plan-72-data/raw/c749.json 'Supplier Device Package'` → 82 values, led by 24-TSSOP 66, 16-TSSOP 43, 16-SOIC 19, 24-TQFN (4x4) 17, 24-HWQFN 16; `… i2cbuf.json 'Supplier Device Package'` → 33 values, led by 8-MSOP 45, 8-DFN (3x3) 16, 8-TSSOP 15. The common outlines exist already (`TSSOP-16`, `TSSOP-24`, `SOIC-16`, `SSOP-24`, `QSOP-16`, `VSSOP-8`, `PDIP-16`, `PLCC-44` in `footprints.json`, checked with a regex scan of the ids on 2026-11-25); B writes the exact list.

### Verified properties

- The converter engine's ports driver already drives a pin from a data field, releases it while a direction field says input, makes it an open-drain sink or a pulled-up input by a field value, and reports the pin's level in a read field (`docs/parts-format.md` § `leddriver` `ports`, `display_out.c`).
- The I2C target front end decodes up to 4 address straps with 4 ties each (`SERIAL_I2C_STRAPS = 4`, `SERIAL_I2C_TABLE = 64` in `backend/src/models/serial_target.h`), enough for every expander's A0–A2 or AD0–AD2.
- The engine has model-controlled contacts: `sim_set_contact(sim, instance, pin_a, pin_b, closed)` (`backend/src/engine/sim.h`), used by `relay.c`; a closed contact makes the two nets one, both ways.
- TI's TCA9555 and TCA6424A datasheets state the pair / triple pointer behaviour (`pdftotext -layout tca9555.pdf`, line 1494: "if Input Port 1 is read, the next byte read is Input Port 0"; `tca6424a.pdf` line 1563: "If AI is 1, the 2 least significant bits are automatically incremented"). The PCF8574 datasheet (`pcf8574.pdf`, lines 690–696) states that INT resets when the port returns to the original data or is read from or written to. The TCA9548A has "a single 8-bit control register" (`tca9548a.pdf` line 676). All fetched 2026-11-25 from `https://www.ti.com/lit/ds/symlink/<name>.pdf` with `curl -sL -A "Mozilla/5.0"`.

## 3. Design Overview

A is data work only, the plan-58-A shape: list the pick through Safari, check
each row's stock, download and scan the datasheets, decide each row, leave a
skeleton for B. Ten pieces: four expander makers, three buffer makers, three
mux series groups. The whole population is `candidates.tsv`; later letters
never re-run the listing.

The risks are DigiKey's, not the code's: a page that silently comes from
another tab (`check.py` catches it), "In Stock" counting Marketplace stock
(only the product's own `In Stock` line counts), and Cloudflare re-challenging
after about 100 loads an hour (the listing is about 40 loads).

Rejected: `scripts/digikey.js` (AGENTS.md); listing 743 by manufacturer
(TI's 743 rows are mostly bus switches and decoders, which are out of scope;
the series keywords pick exactly the I2C muxes); one listing page for 749 as
a whole (453 rows over a 100-row page: the listing is per maker, and TI's 101
is split by Interface facet value).

## 4. Detailed Design

### 4.1 The listing pages (`census/make-urls.js` → `census/urls.tsv`)

One page per piece, each under 100 products, In Stock + Active + the filter:

| Piece | Page(s) |
|---|---|
| expander TI | 749, Manufacturer 296, Interface `I2C, SMBus` (752412); 749, Manufacturer 296, every other Interface value |
| expander ADI | 749, Manufacturer 175 + 505 |
| expander NXP | 749, Manufacturer 568 |
| expander Microchip | 749, Manufacturer 150 |
| buffer ADI / NXP / TI | 756 + the I2C Applications filter of § 2, Manufacturer 505 + 175 / 568 / 296 |
| mux TI / Diodes / NXP | 743 with `keywords=TCA954` / `PI4MSD` / `PCA984`, and one page each for `PCA954`, `LTC4306`, `MAX735`, `PCA9641`, `TCA9543`, `TCA9545` (series that a keyword prefix could miss) |

`census/list.sh` (from plan-58's) loads each URL, saves the `__NEXT_DATA__`
envelope, retries a page whose `check.py` fails up to 5 times.

### 4.2 `candidates.tsv`

Columns (written by `census/build.py`): `family maker mpn series description
interface io package supplier_package mounting stock_in stock_marketplace status
datasheet detail rank decision reason` (`family` `ioexpander` or `i2cbus`;
`maker` the folder slug `ti`, `adi`, `nxp`, `microchip`, `diodes`); A Phase 2
appends `pdf hit dead` (columns 19–21). `stock_in` is the product's own
`In Stock` quantity; `rank` is the row's place in its (family, maker) by
`stock_in`, descending. `build.py` rules, in this order:

1. `stock_in` = 0 → `out`, reason `no DigiKey stock` (Marketplace / Factory only).
2. Military, bare die, wafer (description or grade) → `out`, reason `military` / `die`.
3. Not an I/O expander or I2C bus part by its description (a translator, an
   LED/PWM driver, a keypad scanner without GPIO, a bus switch with no I2C
   control) → `out`, reason `not in scope: <what it is>`.
4. `rank` > 200 → `out`, reason `cap 200` (no piece reaches it at planning; the rule stays).
5. Otherwise `add`.

### 4.3 Verdicts (`census/textscan.py`, `census/decide.py`)

As plan-58-A § 4.1: `pdf` (1 when the PDF downloaded and opens with `pdfinfo`),
`hit` (the part number, or its base number plus the package/reel suffix as the
ordering table prints it, stands in the PDF text), `dead` ("obsolete",
"discontinued", "NRND", "not recommended for new designs", "end of life" within
300 characters of the number). For `add` rows: `pdf` 0 → `skip`, `no datasheet`;
`hit` 0 → `skip`, `not in the ordering table`; `dead` 1 → stays `add`, flagged
obsolete by the parts letter. A PDF whose every row is `dead` 1 is deleted and
recorded in `datasheets/obsolete-not-included.md`, its rows `out`, `obsolete`.
Every `skip` goes to the "I/O expanders and I2C bus parts (plan-72)" section of
`datasheets/skipped-parts.md`.

### 4.4 Where files go

- `datasheets/ioexpander/<maker>/<original file name>.pdf`,
  `datasheets/i2cbus/<maker>/…`; `maker` = `ti`, `adi`, `nxp`, `microchip`,
  `diodes`. URL rule (plan-27-A): the maker's site when the same document is
  there, else DigiKey's. TI: `https://www.ti.com/lit/ds/symlink/<name>.pdf`
  (curl with a Mozilla user agent works, § 2); ADI: Python `urllib` with a
  Safari user agent (memory: analog.com hangs curl); NXP: `nxp.com/docs/en/data-sheet/<name>.pdf`;
  Microchip: `ww1.microchip.com/downloads/…`; Diodes: `diodes.com/assets/Datasheets/<name>.pdf`;
  DigiKey-hosted copies through `scripts/safari-digikey-pdf.sh`.
- `planning/plan-72-data/census/`: `make-urls.js`, `list.sh`, `check.py`,
  `build.py`, `fetch.py`, `textscan.py`, `decide.py`, `place.py`,
  `specs-skeleton.py`, each with `--help` and `--selftest` or `--verify`.

### 4.5 `specs.tsv`

One row per (family, maker, PDF, package) of the `add` rows. Columns: `family
maker pdf package parts template io bus address_pins footprint pinout note`.
A writes the first five (`parts` = the part numbers, comma-separated); B fills
the rest.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` for partially done; `- [x] ~~text~~ —
> moot: <evidence>` for a moot task; fill the phase's `Commit:` line when it
> lands. An unticked box means NOT DONE.

### Phase 1 — list the pick

- [ ] Re-run every Prerequisites command; update the Status column.
- [ ] `planning/plan-72-data/census/make-urls.js` → `urls.tsv` (§ 4.1);
      `census/list.sh /tmp/p72/raw` → one `<piece> count N rows N` line per page, no `FAILED`.
- [ ] `python3 -I census/build.py /tmp/p72/raw planning/plan-72-data/candidates.tsv`
      (§ 4.2) → prints `<rows> rows, <add> add, <out> out`.
- [ ] Record the populations in § 2's table next to the planning ones; a
      difference goes to Corrections.

Acceptance: each page's count equals its row count, and every row's
`decision` is `add` or `out`.
  Check: `awk -F'\t' 'NR>1 && $17!="add" && $17!="out"' planning/plan-72-data/candidates.tsv | wc -l` → 0, and `python3 -I planning/plan-72-data/census/check.py --all /tmp/p72/raw` → `0 pages short` (est. 25 min, Safari).
Commit:

### Phase 2 — download and scan the datasheets

- [ ] `census/fetch.py planning/plan-72-data/candidates.tsv /tmp/p72/pdf` → `<n> urls; <n> are PDFs; 0 missing`, exit 0 (§ 4.4 headers per host).
- [ ] `census/textscan.py planning/plan-72-data/candidates.tsv /tmp/p72/pdf` → adds `pdf`, `hit`, `dead`.
- [ ] `fetch.py --selftest`, `textscan.py --selftest` → `ok`.

Acceptance: every `add` row has `pdf`, `hit` and `dead` filled.
  Check: `awk -F'\t' 'NR>1 && $17=="add" && $19==""' planning/plan-72-data/candidates.tsv | wc -l` → 0 (est. 20 min).
Commit:

### Phase 3 — decide, place, skeleton

- [ ] `census/decide.py` (§ 4.3) → `<n> add, <n> skip, <n> out`; `--skipped` writes the plan-72 section of `datasheets/skipped-parts.md`; `--verify-skipped` exit 0.
- [ ] Obsolete check (AGENTS.md): every all-`dead` PDF deleted and listed in `datasheets/obsolete-not-included.md` (family `ioexpander` or `i2cbus`, category Interface).
- [ ] `census/place.py` → copies the PDFs, writes both `datasheets.tsv`; `--verify` → `<n> URLs, <n> PDFs, 0 problems`.
- [ ] `census/specs-skeleton.py planning/plan-72-data/candidates.tsv planning/plan-72-data/specs.tsv` → `<n> rows`.
- [ ] `planning/plan-72-data/README.md`: the scripts and the final counts.

Acceptance: `place.py --verify` and `decide.py --verify-skipped` exit 0;
`specs.tsv` has one row per distinct (family, maker, pdf, package) of the `add` rows.
  Check: `python3 -I planning/plan-72-data/census/place.py --verify && python3 -I planning/plan-72-data/census/decide.py --verify-skipped && python3 -I planning/plan-72-data/census/specs-skeleton.py --verify` → three lines, each `0 problems`, exit 0 (est. 3 min).
Commit:

## Validation Plan

- Tests: the scripts' `--selftest` / `--verify` modes (above); no backend or app change.
- Coverage: the scripts live under `planning/` (plan-68's coverage tool covers `scripts/`, `backend/`, the app; `planning/` scripts are not in its scope — the plan-58-A precedent).
- Runtime proof: not applicable (data only).
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`, `planning/plan-72-data/README.md`.
- Final gate (run once, after Phase 3): `python3 -I planning/plan-72-data/census/place.py --verify` and `git status --short` lists only `planning/plan-72-*`, `planning/plan-72-data/`, `datasheets/` paths (est. 1 min).

## Decisions

- Owner, 2026-11-25: **backend letters wait for plan-68.** The data, datasheet, spec and footprint letters run now; the first letter that changes backend or app code (D) lists plan-68 complete as a prerequisite, and every code letter meets plan-68's size and coverage checks (`node scripts/source-size.js --check`, `node scripts/coverage.js --check`), which the landing letter runs once over all tiers.
- Pick rule: the top three makers per family by DigiKey in-stock Active count, a family's 200 most-stocked per maker when it has more — owner, 2026-11-25 (`planning/plan-69-data/census/README.md` decision 1).
- Microchip is a fourth I/O-expander maker (MCP23017 / MCP23S17) — owner, 2026-11-25 (decision 5).
- I2C buffers and muxes are in scope — owner, 2026-11-25 (decision 2).
- Simulation: expanders on the existing I2C/SPI engines; muxes and buffers as engine contacts — owner, 2026-11-25 (decision 3: "logic where it's cheap"); the split into kinds is the author's (plan-72-D § 3, -E § 3).
- ADI and Maxim are one maker; Rochester's legacy labels are not makers — plan-48 precedent.
- "In stock" is DigiKey's own `In Stock` line above zero — plan-58-A's reading of AGENTS.md.
- The I2C-mux piece is listed by series keyword in category 743, because 743 has no I2C facet — the author's call (§ 2).

## Corrections

(none yet)
