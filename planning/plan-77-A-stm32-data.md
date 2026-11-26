# plan-77-A: STM32: listing, stock check, the 300 pick, pin data, datasheet downloads

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-77 is A → B → C → D → E → F → G → H
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-77 adds ST's STM32 microcontrollers to the existing `mcu` kind: the **300 most-stocked
active STM32 parts** at DigiKey, across every STM32 line DigiKey stocks (owner decision,
2026-11-25). They are **facts only**: each device says `"simulated": false`, as the ESP32 and RP2040
devices do (owner decision: no Cortex-M simulator). Pinouts come from ST's own pin database,
STM32_open_pin_data (BSD-3-Clause), checked against each datasheet's pin table.

The single behavioral outcome of plan-77: loading the part files gives the baseline parts plus
300 STM32 parts; the drawer lists them under **Microcontroller → STM32 → F0 / F1 / … / U5**; a
placed STM32 part shows its core (e.g. "Cortex-M3"), clock, flash and RAM, has the pins its
datasheet names, a footprint whose pads match ST's recommended footprint for its package, and
turns Simulation › Run off; `ctest` passes with as many programs as before (the new cases go into
`mcu_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing of all 1,421 in-stock STM32 rows, stock check, the 300 pick, pin-data match, verdicts, datasheet downloads | large |
| B | `gen-stm32.py`: pinouts, devices and facts from the XML; datasheet pin-table check; `footprints-needed.txt` | large |
| C | footprints: leaded and lead-frame packages (LQFP, TSSOP, SO, UFQFPN, VFQFPN) | large |
| D | footprints: ball packages (UFBGA, TFBGA, WLCSP) | large |
| E | backend: the Cortex-M `core` values, the `core2` key, `mcu_test` cases, docs | medium |
| F | parts: the devices in `Parts/mcu.json`, `datasheets.json`, goldens | medium |
| G | app: core names, the Part Editor's `core` / `core2` fields, drawer, tests | medium |
| H | landing tier | small |

Outcome of A:

- `planning/plan-77-data/candidates.tsv` lists every STM32 row DigiKey shows in category 685 for
  maker ST, In Stock + Active + no Marketplace: 1,421 rows at planning.
- `stock-check.tsv` gives each row's own `In Stock` quantity and status.
- `opd.tsv` gives each row its STM32_open_pin_data RefName, or `none`.
- `verdicts.tsv`: exactly 300 `add` rows (§ 3.3); every other row `out` with a reason.
- `datasheets/mcu/st/` holds every datasheet an `add` row cites, listed in `datasheets.tsv`.
- `planning/plan-77-data/opd/` holds the XML files of the `add` rows, `LICENSE` and `COMMIT`.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` § `mcu` (line 2889), § Datasheets,
  § Footprints; `docs/backend.md` § goldens.
- `planning/plan-76-data/README.md` § Plan 77 (the planning census) and
  `planning/plan-69-data/census/` (`url.js`, `facets.sh`, `show.py`, `kw.js`: used as they are).
- `planning/plan-77-data/README.md` (the files of this plan; the measured facts of the pin data).
- Precedents: `planning/completed/plan-36-A-mcu-data.md` (the mcu listing, stock check,
  datasheet heads), `planning/completed/plan-36-B-mcu-pinouts.md` (ATDF pinouts checked against
  the datasheet's text), `planning/plan-74-A-ethernet-data.md` (the split listing, the `In Stock`
  line rule, `list.sh` retries).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p77 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p77 -j8 && ctest --test-dir /tmp/logic-p77 -j8` → `100% tests passed` | MET (2026-11-25 at 686408f4, run by the author: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | at planning (686408f4): **P0 = R0 = 40832**, **F0 = 3993**. E and F re-measure at their start and use their own |
| mcu baseline | `grep -c '\|mcu\|' backend/tests/golden/parts.txt`; `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/mcu.json'))['devices']))"` | at planning: **M0 = 1424** mcu parts, 299 devices |
| No STM32 exists yet | `grep -c 'STM32' schematical/schematical/Parts/mcu.json`; `grep -c 'cortex-m3\|cortex-m4' backend/src/models/mcu.c`; `ls datasheets/mcu/st` | MET (2026-11-25: 0; 0; no such directory) |
| No other open plan adds STM32 parts or Cortex-M cores | `grep -l -i 'STM32\|cortex-m3\|cortex-m4\|core2' planning/plan-*.md \| grep -v plan-77 \| wc -l` | MET (2026-11-25: 0. Plans 76 and 78–82 are written at the same time and add other kinds) |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/685" 10 'document.title'` → a title starting `Microcontrollers` | MET at planning (2026-11-25: the census pages of `planning/plan-76-data/raw/` loaded). When the command prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| `lz-string` for the census URLs | `ls /tmp/lz/node_modules/lz-string` → a directory (`mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string`) | MET at planning |
| st.com serves datasheets to Python | `python3 -I -c "import urllib.request as u;r=u.Request('https://www.st.com/resource/en/datasheet/stm32f103c8.pdf',headers={'User-Agent':'Mozilla/5.0 (Macintosh; Intel Mac OS X 14_0) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15','Accept':'application/pdf,*/*','Accept-Language':'en-US,en;q=0.9'});b=u.urlopen(r,timeout=30).read();print(len(b),b[:5])"` → a size and `b'%PDF-'` | MET (2026-11-25: `1965230 b'%PDF-'`). `curl` fails on st.com's HTTP/2 (`curl: (92) HTTP/2 stream 1 was not closed cleanly`); the fetcher uses Python's HTTP/1.1 |
| GitHub reachable | `git ls-remote https://github.com/STMicroelectronics/STM32_open_pin_data HEAD` → one line | MET (2026-11-25: the author cloned it, `--depth 1`) |
| Tools | `pdftotext -v`, `pdfinfo -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

`verdicts.tsv` has exactly 300 `add` rows: the most-stocked STM32 part numbers with DigiKey's own
`In Stock` above 0, status Active, and a RefName in STM32_open_pin_data. Each `add` row's datasheet
is in `datasheets/mcu/st/`, and its XML file is in `planning/plan-77-data/opd/`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `scripts/digikey.js` and `planning/plan-69-data/census/` are not changed. Stock is looked up in
  Safari (AGENTS.md).
- Out of scope, never re-opened in later letters: STM8, SPC5 and any other ST series not starting
  `STM32` (202 rows at planning: STM8 132, SPC5 35, `-` 35); STM32MP microprocessors (not in 685);
  STM32 wireless lines DigiKey files outside 685 (WB, WL, WBA); evaluation boards; parts beyond
  the 300.
- No simulation model of any Cortex core.

## 2. Current State

The `mcu` kind (`backend/src/models/mcu.c`) holds 299 devices and 1,424 parts: AVR (Microchip),
ESP32 modules (Espressif) and the RP2040 (Raspberry Pi). No STM32 (`grep -c STM32
schematical/schematical/Parts/mcu.json` → 0). The allowed cores are `mcu.c:24-25`:
`avr avrrc avrxt avrxm cortex-m0plus xtensa-lx6 xtensa-lx7 riscv`. `datasheets/mcu/` has
`espressif/`, `microchip/`, `raspberry-pi/` and six AVR PDFs, with a `datasheets.tsv`.

### Measured populations

All DigiKey numbers from 2026-11-25, category 685, maker ST (497), In Stock + Active + Exclude
Marketplace (`planning/plan-76-data/raw/st.json`; `python3 -I planning/plan-69-data/census/show.py
planning/plan-76-data/raw/st.json Series`):

| What | Count |
|---|---|
| ST MCUs | 1,623 |
| …Series starting `STM32` | **1,421**: F4 157, F0 146, F1 144, L4 143, L0 117, U5 107, G0 92, F3 86, C0 76, H7 64, F7 60, F2 56, L1 45, U3 32, U0 28, G4 24, L5 17, H5 14, C5 8, N6 5 |
| The planning page's first 100 rows | 99 STM32 rows; In Stock from 91,711 down to 118; 0 with no stock; datasheet hosts `www.st.com` 98, `mm.digikey.com` 2; 58 distinct datasheet URLs |
| Their pin-data match | 99 of 99 match a RefName; 4 match two (`planning/plan-77-data/README.md`) |
| STM32_open_pin_data | 2,282 XML files; Families present: C0 F0 F1 F2 F3 F4 F7 G0 G4 H5 H7 L0 L1 L4 L4+ L5 N6 U0 U3 U5 WB WB0 WBA WL WL3 WL4 MP1 MP2. **No STM32C5**: a C5 row matches nothing and is `out` (§ 3.3) |

### Verified properties

- DigiKey's In Stock facet with Marketplace excluded still counts Factory and Alternate stock
  (plan-76-data README: 1 + 1 of 100 rows on `m105`). Only the product's own `In Stock` line, above
  0, counts.
- A census page holds at most 100 products; six series are over 100 (F4, F0, F1, L4, L0, U5) and
  are split (§ 3.1).
- Python `urllib` fetches st.com PDFs; `curl` does not (§ Prerequisites).

## 3. Design

### 3.1 The listing (`census/list.sh`, `census/split.py`, `census/build.py`)

One page per (series, piece), saved to `planning/plan-77-data/raw-A/<series>_<piece>.json` by
`planning/plan-69-data/census/facets.sh` with `{"-9":[1],"-1":[497],"-4":[<series id>]}` (series
ids from `st.json`'s Series facet). `split.py` reads a page's `productCount`; a page over 100 is
split by Package / Case (`16`, the ids of `st.json`'s facet) until each piece is 100 or fewer.
`list.sh` retries a page up to 5 times when it is empty or holds rows of another series (plan-58-A:
Safari sometimes returns another tab's page). `build.py` writes `candidates.tsv`, one row per
distinct part number. The sum of the rows must equal the Series facet's STM32 total of the same
day (1,421 at planning); a difference is a missing piece, re-listed before going on.

### 3.2 Stock check and pin-data match

- `stock-check.tsv`: the `In Stock` line of each row as saved on its page (`qtyAvailable`, label
  `In Stock`), and its status. A row with no `In Stock` line, or 0, is `out` `no DigiKey stock`.
- `opd.tsv` (`census/opd-match.py`): each part number against every RefName of a pinned clone of
  STM32_open_pin_data: `(a-b)` → `[ab]`, lowercase `x` → one character, a prefix match; the
  longest matching RefName wins. The clone is `git clone --depth 1`; its commit goes in
  `planning/plan-77-data/opd/COMMIT`.

### 3.3 The pick (`census/pick.py`)

Rows sorted by `In Stock`, highest first (ties: part number, ascending). Walking down, a row is
`add` while fewer than 300 rows are `add` and it has a RefName; a row with no RefName is `out`
`no open pin data`; every row after the 300th `add` is `out` `cap`. Tape-and-reel and other
packaging variants with their own part number (`…TR`) are distinct parts and count as rows.
`verdicts.tsv` columns: `part  verdict  reason  stock  refname`, rows in the walk's order (so the
first `add` row is the most-stocked part; G and H use it as their check part).

### 3.4 Datasheets (`census/fetch.py`)

The datasheet of a part is the URL on its DigiKey row. A `mm.digikey.com` URL is replaced by ST's
own `https://www.st.com/resource/en/datasheet/<root>.pdf`, where `<root>` is the first `add` part
number of that line in lower case, cut after its flash letter (`STM32F103C8T6` → `stm32f103c8`),
which is the form ST's datasheet URLs take (58 of the planning page's 58 distinct URLs are st.com
URLs of this form or the older `DM000…` form). Each URL is fetched once with Python
`urllib` (the prerequisite's headers) into `datasheets/mcu/st/<root>.pdf`, checked with `pdfinfo`
(a Title naming the line), and listed in `datasheets.tsv`: file, URL, bytes, SHA-256, the `add`
parts it covers. The `DM000…` URLs are saved under the `<root>` name of their first part.

Obsolete check (AGENTS.md): every `add` row is Active by construction, so no PDF of A covers only
obsolete parts and `datasheets/obsolete-not-included.md` gets no row.

### 3.5 The pin data copy

`census/opd-copy.py` copies the XML file of each distinct `add` RefName, plus `LICENSE`, into
`planning/plan-77-data/opd/`, keeping ST's names. B reads only this copy, so the pinouts do not
move when ST updates the repository.

### Rejected alternatives

- **Ranking by line, not by part** (e.g. 300 spread evenly over the lines): the owner asked for
  the most-stocked; a line with no stock-leading part gets none.
- **Reading pinouts from the datasheets' tables only** (plan-36's text transcription): ST's XML is
  the CubeMX database, machine-readable and licensed for reuse; B still checks it against each
  datasheet's table.
- **Counting `TR` variants as one part**: each is a separate orderable part number with its own
  stock; the library lists orderable parts (plan-74-A precedent).

## 4. Phases

### Phase 1 — Listing

- [ ] `census/split.py`, `census/list.sh`, `census/build.py` (§ 3.1) in `planning/plan-77-data/census/`.
- [ ] List every series; `candidates.tsv`.

Acceptance: `python3 -I planning/plan-77-data/census/build.py --check` → `rows N = series total N`
with N equal to the Series facet's STM32 sum of the same day (1,421 at planning).

Commit:

### Phase 2 — Stock, pin data, pick

- [ ] `stock-check.tsv` (§ 3.2).
- [ ] Clone STM32_open_pin_data into the scratchpad; `census/opd-match.py` → `opd.tsv`.
- [ ] `census/pick.py` → `verdicts.tsv` (§ 3.3).

Acceptance: `python3 -I planning/plan-77-data/census/pick.py --check` → `add 300`, with every
`add` row's stock > 0 and a RefName, and no `out` `cap` row with more stock than the last `add`.

Commit:

### Phase 3 — Pin data copy and datasheets

- [ ] `census/opd-copy.py` → `planning/plan-77-data/opd/` (§ 3.5) with `LICENSE` and `COMMIT`.
- [ ] `census/fetch.py --probe` (one PDF) then `census/fetch.py` (§ 3.4) → `datasheets/mcu/st/`,
      `datasheets.tsv`.

Acceptance: `python3 -I planning/plan-77-data/census/fetch.py --check` → every `add` row names a
file in `datasheets.tsv`, every file starts `%PDF-` and its SHA-256 matches;
`ls planning/plan-77-data/opd/*.xml | wc -l` equals the distinct RefNames of the `add` rows
(`cut -f2 planning/plan-77-data/opd.tsv` joined to the `add` rows, `sort -u | wc -l`).

Commit:

## 5. Validation Plan

A's checks are the three acceptance commands (each under a minute; they read only the saved
pages and files). No backend or app check: A changes neither. The landing tier is H's.
