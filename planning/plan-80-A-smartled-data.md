# plan-80-A: Addressable LEDs: listing, stock check, verdicts, datasheets, specs

Last updated: 2026-11-25
Overall Effort: x-large (1d–3d). plan-80 is A → B → C → D → E
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-80 adds WS2812-class addressable LEDs: an RGB (or RGBW, or single-colour) LED with a driver IC
inside, fed by one data wire. Each LED takes the first frame it receives on DIN, passes the rest on
from DOUT, and shows its frame when the line stays low for the reset time. The parts come from the
four makers the owner chose (2026-11-25), from DigiKey category **LED Addressable, Specialty (126)**,
Configuration Discrete:

| Maker | In-stock discrete rows (census) | Cap |
|---|---|---|
| **Inolux** | 28 | 300 |
| **Dialight** | 10 | 300 |
| **Würth Elektronik** | 6 | 300 |
| **Lite-On** | 5 | 300 |

No maker reaches the 300 cap. The parts are **simulated** (owner decision): a new kind `smartled`
decodes the one-wire signal with each device's datasheet timing, forwards the rest of the stream,
and reports each LED's colour to the app, which paints it.

The single behavioral outcome of plan-80:

- Loading the part files gives the baseline parts plus one part per `add` row of
  `planning/plan-80-data/verdicts.tsv`.
- The drawer lists them under **Displays & Indicators → Addressable LED**.
- In play mode an ATmega328P at 16 MHz running `ws2812.hex` lights a chain of three
  `IN-PI55TATPRPGPB` red, green and blue, and the canvas paints each body in its colour.
- `ctest` passes T0 + 1 programs (`smartled_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing, scope, per-part stock check, verdicts, datasheet downloads, `specs.tsv`, `footprints-needed.txt` | large |
| B | backend: the `smartled` kind and model, the `colors` hook and `circuit_part_colors`, `smartled_test`, the AVR chain test, docs | large |
| C | footprints from the drawings, `gen-smartled.py`, `Parts/smartled.json`, `datasheets.json`, goldens | large |
| D | app: colours in the snapshot, the symbol, facts, drawer, Part Editor kind, test classes, harness | large |
| E | landing tier | small |

Outcome of A:

- `candidates.tsv` lists every row DigiKey shows for category 126, Configuration Discrete, the four
  makers, In Stock + Active, Marketplace excluded, with `scope` = `in` or `out` and a reason.
- `stock-check.tsv` has DigiKey's own `In Stock` line (> 0) and the status of every `in` row.
- `verdicts.tsv` gives every `in` row `add` or `skip` with a reason (§ 3.3).
- `datasheets/smartled/{inolux,dialight,wurth,liteon}/` hold every PDF an `add` row cites, each
  folder with a `datasheets.tsv` (file, URL, bytes, SHA-256).
- `specs.tsv` has one filled row per (maker, datasheet, package) of the `add` rows (§ 3.4), and
  `footprints-needed.txt` one line per distinct package drawing.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints,
  § `led`); `docs/backend.md` (§ Adding a kind, § goldens).
- `planning/plan-76-data/README.md` § Plan 80: the planning census of plans 76–82, and its raw page
  `planning/plan-76-data/raw/d126.json` (100 product rows kept).
- `planning/plan-69-data/census/` (`url.js`, `facets.sh`, `show.py`, `kw.js`): A calls them and does
  not change them.
- `planning/plan-80-data/README.md`: the files of this plan and the seven datasheets read at planning.
- Precedents: `planning/plan-74-A-ethernet-data.md` (listing, stock check, verdicts, specs
  skeleton); `planning/completed/plan-58-A-slide-switch-data.md` (the `In Stock`-line rule, retries).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p80 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p80 -j8 && ctest --test-dir /tmp/logic-p80 -j8` → `100% tests passed` | MET (measured at planning, 2026-11-25 at 686408f4: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | at planning (686408f4): **P0 = R0 = 40832**, **F0 = 3993**. B, C and D re-measure at their start and use their own |
| Nothing of plan-80 exists yet | `grep -c 'PART_SMARTLED' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/smartled.json` → no such file; `ls datasheets/smartled` → no such directory; `grep -c 'circuit_part_colors' backend/include/circuit.h` → 0 | MET (2026-11-25: 0; no file; no directory; 0) |
| No other open plan adds this kind, hook or folder | `grep -l -i 'PART_SMARTLED\|Parts/smartled\.json\|datasheets/smartled\|circuit_part_colors' planning/plan-*.md \| grep -v plan-80 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 76–82 are written at the same time; plan-81 adds an OLED state hook to the same `SimComponentType` and `SimulationSnapshot`. The shared lines (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the hook list, the editor's kind lists, `CMakeLists.txt`) are added only if absent and after whatever is last; each such step greps first (B Phase 1, D Phase 1) |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | a rule, applied per footprint in C |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/126" 10 'document.title'` → a title starting `LED Addressable, Specialty` | MET at planning (2026-11-25: the census pages of category 126 loaded). When the command prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| `lz-string` for the census URLs | `ls /tmp/lz/node_modules/lz-string` → a directory (else `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string`) | MET at planning (2026-11-25) |
| The maker sites serve PDFs | `curl -sL -A "Mozilla/5.0" -o /tmp/p80.pdf "https://s3-us-west-2.amazonaws.com/catsy.557/Dialight_datasheet_587-2056-147F_Addressable_RGB.pdf" && head -c5 /tmp/p80.pdf` → `%PDF-` | MET at planning (2026-11-25: Inolux `www.inolux-corp.com` 1,594,147 B; Dialight S3 2,197,399 B; Lite-On `optoelectronics.liteon.com` 801,881 B, each `%PDF-`). `www.we-online.com` answers curl with a 226-byte `Access Denied` page: Würth PDFs come through `scripts/safari-digikey-pdf.sh <url> <out>` |
| Tools | `pdftotext -v`, `pdfinfo -v`, `pdftoppm -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25: all used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `planning/plan-80-data/candidates.tsv`: one row per (maker, part) of the listing (§ 3.1), with
  `scope` and `reason`.
- `stock-check.tsv`: one row per `in` row, `In Stock` > 0, status Active or Last Time Buy.
- `verdicts.tsv`: one row per `in` row, `add` or `skip`.
- `datasheets/smartled/<maker>/`: every PDF an `add` row cites, plus `datasheets.tsv`.
- `datasheets/skipped-parts.md` § "Addressable LEDs (plan-80)" lists every `skip` row.
  `datasheets/obsolete-not-included.md` gets a row only for a PDF all of whose parts are obsolete.
- `specs.tsv`: one filled row per (maker, datasheet, package) of the `add` rows; `footprints-needed.txt`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `scripts/digikey.js` and `planning/plan-69-data/census/` are not changed. Stock is looked up in
  Safari (AGENTS.md).
- Out of scope, never re-opened in later letters: Adafruit, SparkFun and other packs of LEDs (owner,
  2026-11-25); XINGLIGHT (Marketplace only); ams OSRAM OSIRE (its own OSP protocol, not WS2812's);
  strips, rings, matrices, modules and filaments (Configuration other than Discrete); parts with a
  clock input (APA102-style two-wire) or a backup data input (WS2813-style BIN).

## 2. Current State

No addressable LED exists in the library (`grep -c -i 'addressable\|ws2812' backend/tests/golden/parts.txt`
→ 0, 2026-11-25). The `led` kind (`backend/src/models/led.c`, 144 lines) is one die: device keys
`colors`, `anode`, `cathode` only (`led.c:13`), lit while anode reads 1 and cathode 0, one light
(`led_lights`). The colour of a placed LED is chosen on the instance (`Document/SchematicFile.swift`
`color`), drawn by `Canvas/DiodeSymbolView.swift` (`lightColor(named:)`), which uses only the first
light. `footprints.json` has 11 `LED-` footprints, all two-pad but `LED-DIP4`; none of the 5050,
3535, 2020, 1515 or 0603 addressable packages.

### Measured populations

| What | Count | Source |
|---|---|---|
| Category 126, In Stock + Active, no Marketplace | 292 | `python3 -I planning/plan-69-data/census/show.py planning/plan-76-data/raw/m126.json Manufacturer` → `TOTAL 292` |
| …Configuration Discrete | 146: Adafruit 75, Inolux 28, Dialight 10, Würth 6, ams OSRAM 5, Lite-On 5, SparkFun 5, American Bright 3, Bivar 3, Feztek 3, Lumex 2, Kingbright 1 | `show.py planning/plan-76-data/raw/d126.json Manufacturer` |
| …of the four makers, on the saved page | 49 (all of them: 28 + 10 + 6 + 5) | the saved rows of `d126.json`, counted by maker |
| Interface facet of the 49 rows | `-` 27, Serial (Shift Register) 10, PWM 7, Serial 5 | the same rows' Interface column (`python3 -I planning/plan-80-data/census/rows.py planning/plan-76-data/raw/d126.json 154` over the four makers, `uniq -c`) |
| `-` and `PWM` rows are still addressable | `IN-PI554FCH` (PWM): pins VSS, DIN, VDD, DO, "Display data cascaded input (800k bps)"; `IN-PIS63BTPW` (PWM): DIN/DOUT, 24-bit frame, white die on the middle byte | the planning datasheets (`planning/plan-80-data/README.md`) |
| Distinct sizes of the 49 rows | 23 sizes: 5.00×5.00 mm 11, 2.00×2.00 7, 1.60×0.80 4, 3.20×2.80 3, `-` 3, 5.36×5.00 / 3.70×3.50 / 1.60×1.60 / 1.30×1.30 2 each, 14 sizes with 1 row | the Size / Dimension column (`python3 -I planning/plan-80-data/census/rows.py planning/plan-76-data/raw/d126.json 46`, `uniq -c`) |

### Verified properties

- **The Interface facet does not decide scope.** Both `PWM` and `-` rows are one-wire addressable
  LEDs in their datasheets (§ 2 table). The verdict reads the datasheet's pin table and protocol.
- **Timing comes in three forms**: typical values only (Inolux: T0H 0.3 µs, T1H 0.6 µs), typical ±
  tolerance (Dialight ±0.15 µs; Lite-On ±80 ns), or min/max columns. Reset low time differs: 50, 80,
  200 and 250 µs in the seven datasheets read.
- **Colour order differs by part**: G R B (Inolux IN-PI55, Dialight 587-1024), R G B (Dialight
  587-2056), one die on the middle byte (Inolux IN-PIS63 white, IN-PIS85 red).
- **`Number of LEDs` 3** on `IN-PI4818QAS5R5G5BPW`, `IN-PI32TAR5R5G5B`, `IN-PI32TARPRPGPB` (census
  rows): A reads from each datasheet how many frames the package takes, the `leds` column.

## 3. Design

### 3.1 The listing (`census/list.sh`, `census/build.py`)

One page per maker: `planning/plan-69-data/census/facets.sh planning/plan-80-data/raw-A/<maker>.json
x/126 '{"-9":[1],"207":[339432],"-1":[<id>]}'` with ids Inolux 1830, Dialight 350, Würth 732,
Lite-On 160 (ids from `d126.json`'s Manufacturer facet). Each page is under 100 rows, so it holds the
maker's every row. `list.sh` re-runs a page up to 5 times when it is empty, or when one of its rows
names another maker (Safari sometimes returns another tab: plan-58-A). `build.py` writes
`candidates.tsv`, columns:

`maker  part  stock  status  digikey_id  description  interface  size  number_of_leds  color  datasheet_url  scope  reason`

### 3.2 Scope

Every row of the listing is `in`, except `out` with reason `cap` past a maker's 300th most-stocked
row (none expected: 28 is the largest maker).

### 3.3 Stock check and verdicts

`stock-check.tsv` reads each `in` row's own `qtyAvailable` entry labelled `In Stock`; a row with
no such entry or 0 is `skip` / `no DigiKey stock` (AGENTS.md). A row whose status is not Active or
Last Time Buy is `skip` / `status`.

Then the datasheet (downloaded in Phase 2) decides:

- `add`: the pin table has a data input (DIN / DI) and a data output (DOUT / DO), one supply (VDD,
  one or more pins), ground, and NC pads only; the protocol section is a one-wire NRZ code with a
  T0H, a T1H and a reset low time.
- `skip` / `two-wire clocked`: a clock input (CI, CKI, SCK).
- `skip` / `backup data input`: a second data input (BIN, FDI).
- `skip` / `no protocol table`: the PDF gives no T0H / T1H / reset figures in text or in a drawn
  table A can read off the page image.
- `skip` / `datasheet`: no PDF reachable by curl, by `scripts/safari-digikey-pdf.sh`, or from
  DigiKey's `mm.digikey.com` copy.

A PDF all of whose parts are obsolete is deleted and recorded in `obsolete-not-included.md` (AGENTS.md;
not expected: every row is Active).

### 3.4 Specs (`specs.tsv`)

One row per (maker, datasheet, package) of the `add` rows. Columns, all read from the datasheet and
written with its page number:

| Column | Content |
|---|---|
| `pins` | `number:name:role` for every pad, roles `input` (DIN), `output` (DOUT), `power` (VDD), `ground`, `nc` |
| `order` | the bytes of one frame in send order, each `R`, `G`, `B`, `W` or `-` (a byte the part ignores), e.g. `G R B`, `R G B`, `G R B W`, `- W -` |
| `leds` | frames one package takes (1, or 3 for a package with three addressable dies) |
| `t0h t1h t0l t1l` | ns, the typical values |
| `tolerance` | ns: the datasheet's ±; for min/max columns, the smaller of (typ − min, max − typ) of T0H and T1H; for typical-only datasheets 150 (the ± that Dialight and the WS2812B convention print; recorded as `typ-only` in a `notes` column) |
| `reset` | ns, the minimum reset low time |
| `delay` | ns, DIN → DOUT delay: the typical figure, else the maximum, else 0 with `notes` = `no delay figure` |
| `vcc` | `[min, max]` V, the recommended operating supply |
| `drawing` | the page of the package outline and of the recommended land pattern |
| `footprint` | the id C gives the drawing (§ 3.5) |

### 3.5 Footprint ids

One footprint per distinct package drawing, id `LED-ADDR-<body>-<pads>-<maker>`, e.g.
`LED-ADDR-5050-4-INOLUX`, `LED-ADDR-5050-6-DIALIGHT`, `LED-ADDR-2020-4-DIALIGHT`. Two makers' drawings
with the same pads and land pattern share the first maker's id; `footprints-needed.txt` says which.

### 3.6 Rejected alternatives

- **Using the Interface facet to pick parts**: it labels 7 one-wire parts `PWM` and 27 `-` (§ 2).
- **Counting the In Stock facet alone**: it includes Marketplace and Factory stock (plan-74-A § 2);
  each row's own line decides.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill each `Commit:` line with the hash when the phase lands.

### Phase 1: listing and stock check

- [ ] Write `planning/plan-80-data/census/list.sh` and `build.py` (§ 3.1).
- [ ] Run them; write `candidates.tsv` and `stock-check.tsv` (§ 3.2, § 3.3).

Acceptance: `awk -F'\t' 'NR>1' planning/plan-80-data/candidates.tsv | cut -f1 | sort | uniq -c` →
four makers, each count equal to its Manufacturer facet count on its own `raw-A` page
(`show.py raw-A/<maker>.json Manufacturer`); `awk -F'\t' 'NR>1 && $3+0<=0' stock-check.tsv | wc -l`
equals the `no DigiKey stock` rows of `verdicts.tsv`. (≈ 2 min.)

Commit:

### Phase 2: datasheets and verdicts

- [ ] Download each distinct datasheet URL of the `in` rows into `datasheets/smartled/<maker>/`
      (Würth through `scripts/safari-digikey-pdf.sh`); write each folder's `datasheets.tsv`.
- [ ] Read each PDF's pin table and protocol section; write `verdicts.tsv` (§ 3.3).
- [ ] Add `datasheets/skipped-parts.md` § "Addressable LEDs (plan-80)" with every `skip` row.

Acceptance: `for f in datasheets/smartled/*/*.pdf; do head -c5 "$f"; echo; done | sort | uniq -c` →
only `%PDF-`; every `add` row's datasheet file exists (`python3 -I` one-liner over `verdicts.tsv` and
`datasheets.tsv` → `missing 0`). (≈ 1 min.)

Commit:

### Phase 3: specs

- [ ] Fill `specs.tsv`, one row per (maker, datasheet, package) of the `add` rows (§ 3.4).
- [ ] Write `footprints-needed.txt` (§ 3.5).

Acceptance: `python3 -I -c` over `specs.tsv` → every row has a non-empty `pins`, `order`, `leds`,
`t0h`, `t1h`, `tolerance`, `reset`, `vcc`, `drawing`, `footprint`; every `order` has 3 or 4 bytes
and at least one of `R G B W`; every `pins` has exactly one `input` and one `output` → `bad 0`. (≈ 1 min.)

Commit:

## Validation Plan

A's checks are the three phase acceptances. No build runs: A changes no code.

## Corrections
