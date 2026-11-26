# plan-82-A: Indicator LEDs: listing, scope, stock check, verdicts

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-82 is A → B → C → D → E → F → G → H → I → J
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-82 adds real indicator LEDs to the parts library. Today the `led` kind has one device and one
part, `GENERIC-LED`. plan-82 adds the 300 most-stocked in-scope parts of each of the four makers
with the most in-stock parts in DigiKey's **LED Indication - Discrete** category (105), Marketplace
excluded (planning census, `planning/plan-76-data/README.md` § Plan 82):

| Rank | Maker | In stock + Active, no Marketplace | Cap |
|---|---|---|---|
| 1 | **Kingbright** | 959 | 300 |
| 2 | **Broadcom** | 768 | 300 |
| 3 | **SunLED** | 562 | 300 |
| 4 | **QT Brightek (QTB)** | 497 | 300 |

Every configuration is in scope: single colour, bi-colour, RGB (common anode, common cathode),
independent dies and bidirectional (anti-parallel) pairs. Every mounting with a PCB footprint is in
scope: surface mount (top, right angle, bottom entry / reverse mount) and through hole (radial,
right angle). A multi-die package needs the `led` kind's new `"dies"` key (F); the app draws and
lights each die (G).

The single behavioral outcome of plan-82: loading the part files gives the baseline parts plus one
part per `add` row of `planning/plan-82-data/verdicts.tsv`; the drawer lists them under
**Displays & Indicators → LED → Single color / Bi-color / RGB**; a placed red Kingbright
`APT1608EC` lights red when its anode reads 1 and its cathode 0, with no colour picker; a placed RGB
LED lights each die in its own colour from its own pins; `ctest` passes T0 + 1 programs (`led_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing, scope verdicts, the 300 cap, per-part stock check, `verdicts.tsv` | large |
| B | datasheet downloads, obsolete check, `specs.tsv` skeleton | large |
| C | Kingbright specs (pin maps, dies, facts) and footprints | large |
| D | Broadcom specs and footprints | large |
| E | SunLED and QTB specs and footprints | large |
| F | backend: `led` `"dies"`, the fact keys, `led_test`, docs | medium |
| G | app: multi-die symbol, per-die lights, colours, facts, Part Editor, harness | large |
| H | `gen-led.py`; Kingbright and Broadcom parts; goldens | medium |
| I | SunLED and QTB parts; goldens; `skipped-parts.md` | medium |
| J | landing tier | small |

A, B, C, D and E change no code and no shipped part file: they run now. F waits for plan-68 (§ F
Prerequisites), and G, H, I and J follow F.

Outcome of A:

- `candidates.tsv` lists every row DigiKey shows in category 105 for the four makers (In Stock,
  Active, no Marketplace), each with `scope` = `in` or `out` and a reason (§ 3.2). No maker has
  more than 300 `in` rows: its 300 most-stocked stay `in`, the rest are `out` with reason `cap`.
- `stock-check.tsv` has one row per `in` row: DigiKey's own `In Stock` line (> 0) and the status.
- `verdicts.tsv` has one row per `in` row: `add`, or `skip` with a reason.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` § `led`, § Parts, § Footprints,
  § The drawer's categories.
- `planning/plan-76-data/README.md` (the census of plans 76–82); `planning/plan-69-data/census/`
  (`url.js`, `facets.sh`, `show.py`, `kw.js`: used, not changed).
- `planning/plan-82-data/README.md` (the files of this plan).
- Precedents: `planning/plan-74-A-ethernet-data.md` (the listing in pieces, `check.py` retries,
  `stock-check.tsv`, `verdicts.tsv`); `planning/completed/plan-58-A-slide-switch-data.md` (the
  `In Stock`-line rule); `planning/completed/plan-48-A-timer-opamp-data.md` (a per-maker cap with
  reserve rows).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p82 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p82 -j8 && ctest --test-dir /tmp/logic-p82 -j8` → `100% tests passed` | MET (2026-11-25 at 686408f4, build dir `/tmp/logic-p76`: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | at planning (686408f4): **P0 = R0 = 40832**, **F0 = 3993**. F, H and I re-measure at their start and use their own |
| LED baseline | `grep -c '\|led\|' backend/tests/golden/parts.txt`; `python3 -I -c "import json;d=json.load(open('schematical/schematical/Parts/led.json'));print(len(d['devices']),sum(len(x['parts']) for x in d['devices']))"` | at planning: **L0 = 1** LED part line; `1 1` (device `led-generic`, part `GENERIC-LED`) |
| Nothing of plan-82 exists yet | `grep -c '"dies"' backend/src/models/led.c` → 0; `ls datasheets/led` → no such directory; `python3 -I -c "import json,re;print(sum(bool(re.match(r'^LED-(KB|AV|SL|QTB)-',f['id'])) for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-25: 0; no directory; 0) |
| No other open plan touches the `led` kind or `datasheets/led` | `grep -l '"dies"\|datasheets/led/\|Parts/led\.json' planning/plan-*.md \| grep -v plan-82 \| wc -l` → 0 | MET (2026-11-25: 0). plan-80 (`smartled`) is a separate kind and file |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` → 0 | a rule, applied per footprint in C, D, E (`fp-merge.py` skips an id already present) |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/105" 10 'document.title'` → a title starting `LED Indication - Discrete` | MET at planning (2026-11-25: the census pages loaded; one challenge came back after a run of loads and cleared on its own). When the command prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| `lz-string` for the census URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls /tmp/lz/node_modules/lz-string` → a directory | MET at planning (2026-11-25) |
| Tools | `pdftotext -v`, `pdfinfo -v`, `pdftoppm -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25: all used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `planning/plan-82-data/candidates.tsv` has one row per distinct (maker, part number) of the A
  listing (§ 3.1). Each row has `scope` `in` or `out` with a reason (§ 3.2). Each maker has at most
  300 `in` rows: the 300 with the most DigiKey `In Stock`.
- `stock-check.tsv` has one row per `in` row, every quantity above 0, every status `Active` or
  `Last Time Buy`.
- `verdicts.tsv` has one row per `in` row: `add` or `skip` with a reason.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `scripts/digikey.js` and `planning/plan-69-data/census/` are not changed. AGENTS.md: stock is
  looked up in Safari.
- Out of scope, and never re-opened in later letters:
  - LED Lighting (Color 125, White 124), Infrared/UV emitters (94), panel indicators (108), light
    pipes, LED bar graphs and displays (92), addressable LEDs (126: plan-80).
  - Rows whose Mounting Type is `User Defined`, `-` or chassis / panel: no PCB footprint.
  - Packages of more than 4 dies (§ 3.2).
- todo.md:56 ("Flipped LED doesn't look correct, the light arrows look wrong") is a separate bug of
  the existing symbol. plan-82 does not fix it; G's symbol work keeps today's flip behaviour.

## 2. Current State

- `backend/src/models/led.c` (144 lines): one die per device. Device keys `colors`, `anode`,
  `cathode` (`DEVICE_KEYS`, line 13); each pin name binds exactly one pin (`kind_pin`). One light
  (`led_lights` → `lights_read(…, 1, …)`). No part keys.
- `schematical/schematical/Parts/led.json`: device `led-generic`, part `GENERIC-LED`, eleven
  generic footprint choices (`LED-0402` … `LED-RADIAL-5MM`, `LED-DIP4`). The footprints' `note` is
  `generic; plan-03-A §4.5`: no maker drawing.
- The app: `Canvas/DiodeSymbolView.swift` draws the LED; `lightColor(named:)` knows Red, Green,
  Blue, Amber, Yellow, Orange, White, Warm white, Blue-green. `SchematicCanvasView.swift:1269`
  passes `simulation.lights[chip.id]?.first`: only the first light. The colour picker
  (`Drawer/PartOptionsSection.swift:114`) shows when `part.colors.count > 1`.

### Measured populations

All from `planning/plan-76-data/raw/m105.json` (2026-11-25, category 105, In Stock + Active +
Exclude Marketplace), `python3 -I planning/plan-69-data/census/show.py planning/plan-76-data/raw/m105.json <facet>`:

| What | Count |
|---|---|
| category total | 6,479 |
| the four makers | Kingbright 959, Broadcom 768, SunLED 562, QTB 497 = 2,786 rows to list |
| Configuration (all makers) | Standard 5,241; `-` 1,459; Independent 646; Common Anode 385; Common Cathode 153; Bidirectional 75 |
| Mounting Type (all makers) | Surface Mount 4,757; Through Hole 2,071; SMD Right Angle 828; SMD Bottom Entry 242; `-` 65; TH Axial 18; TH Right Angle 7; User Defined 7; TH and Snap On 1 |
| Color (all makers, top) | Red 1,388; Green 1,253; Yellow 786; Blue 616; Orange 383; White 377; RGB 324; Green-Red 286; Amber 256; Yellow-Green 141; White Cool 119; Green-Yellow 107; RGBW 17 |
| upper bound on `in` rows | 4 × 300 = **1,200** |

### Verified properties

- **The Marketplace exclusion holds per row.** Of the first 100 rows of the census page, 98 show
  only `In Stock`, 1 `In Stock` + `Alternate Stock`, 1 `In Stock` + `Factory`, none Marketplace
  (`planning/plan-76-data/README.md`). A still reads each row's own `In Stock` line.
- **A census page holds at most 100 rows**; every maker here has more, so each is listed in pieces
  (§ 3.1).
- **Datasheet hosts answer plain HTTP.** `curl -sL -A "Mozilla/5.0"` fetched Kingbright
  `https://www.KingbrightUSA.com/images/catalog/SPEC/APT1608EC.pdf` (200, 455,813 B, `%PDF-`,
  4 pages) and SunLED `https://www.sunledusa.com/products/spec/XZMG53W-1.pdf` (200, 258,126 B,
  `%PDF-`, 4 pages), 2026-11-25. Kingbright has one PDF per part number.

## 3. Design

### 3.1 The listing

One page per (maker, piece), saved to `planning/plan-82-data/raw-A/<maker>_<piece>.json` by
`planning/plan-69-data/census/facets.sh` with extra filters `{"-9":[1], "-1":[<maker id>], …}`
(maker ids: Kingbright 754, Broadcom 516, SunLED 1497, QTB 1516). `split.py` reads a page's
`productCount`; a page over 100 is split by Mounting Type `69`, then by Color `37`, until each
piece is under 100 (each split lists every option of the facet, so the pieces add up to the
maker's count). `list.sh` retries a page up to 5 times when `check.py` finds it empty or with rows
of another maker. `build.py` writes `candidates.tsv`, one row per distinct (maker, part number),
from each row's own fields: `In Stock` line, status, mount, configuration, colour, package,
supplier device package, lens, datasheet URL.

### 3.2 Scope (`census/scope.py`)

A row is `in` unless one of these, checked in this order, applies (`reason` as written):

1. `mount`: Mounting Type is `-`, `User Defined`, chassis or panel.
2. `stock`: no `In Stock` line, or its quantity is 0 (Marketplace / Factory only).
3. `status`: status other than `Active` or `Last Time Buy`.
4. `dies>4`: the colour field lists more than 4 colours, or the description says an array of more
   than 4 LEDs.
5. `invisible`: a colour is Infrared or Ultraviolet (those are emitters, category 94's job; the
   app's colour list has no glow for them).
6. `cap`: the row is not among the maker's 300 with the highest `In Stock` (ties broken by part
   number, ascending).

The cap is applied last, so a maker's 300 are 300 usable rows.

### 3.3 Stock check

`stock-check.tsv`: one row per `in` row, read from the row's own census entry (part, `In Stock`,
status, page file). A row whose quantity or status disagrees with the page's facets is re-read by
loading its maker piece again; the second reading stands.

### 3.4 Verdicts

`verdicts.tsv`: every `in` row is `add` unless its datasheet URL is empty (`skip`, reason
`no datasheet`). B, C, D and E may turn an `add` into `skip` only for: a PDF that does not download
(`datasheet 404`), a datasheet with no package drawing (`no drawing`), or every part of the PDF
obsolete (`obsolete`, also recorded in `datasheets/obsolete-not-included.md`).

### Rejected alternatives

- **Cap after scope by family (per colour or package).** The owner asked for 300 per maker; a cap
  per colour would change the count.
- **Including LED Lighting (125/124).** Owner decision at planning: indicator LEDs only.

## 4. Phases

### Phase 1: the tools

- [ ] `planning/plan-82-data/census/list.sh`, `split.py`, `check.py`, `build.py`, `scope.py`
      (`planning/plan-74-data/census/` and `planning/completed/plan-58-A-*` as the pattern).

Acceptance: `planning/plan-82-data/census/list.sh kingbright` lists Kingbright; `python3 -I
planning/plan-82-data/census/build.py` then prints the Kingbright row count, equal to the sum of its pieces' `productCount` and to the maker page's
(959 at planning) — ~10 min.

Commit:

### Phase 2: the four makers

- [ ] List Broadcom, SunLED and QTB the same way.
- [ ] Run `scope.py`; write `candidates.tsv`.

Acceptance: `python3 -I planning/plan-82-data/census/scope.py --summary` prints one line per
maker: rows, `in` (≤ 300), and the count per out reason; each maker's rows equal its page's
`productCount` — ~25 min (at least 32 page loads: 2,786 rows in pieces under 100, plus the 4 maker pages).

Commit:

### Phase 3: stock check and verdicts

- [ ] Write `stock-check.tsv` (§ 3.3) and `verdicts.tsv` (§ 3.4).

Acceptance: `awk -F'\t' 'NR>1 && ($2<=0)' planning/plan-82-data/stock-check.tsv | wc -l` → 0;
`cut -f3 planning/plan-82-data/verdicts.tsv | sort | uniq -c` shows only `add` and `skip` — 1 min.

Commit:

## 5. Validation Plan

A ships no code: its gate is the Phase 3 acceptance plus `git status --short planning/plan-82-data`
showing only plan-82 files. The landing tier is J's.

## Decisions (owner, 2026-11-25)

- Indicator LEDs only (category 105), top 4 makers by in-stock count, 300 per maker.
- Marketplace stock does not count (AGENTS.md: DigiKey's own quantity above zero).
- Every configuration and every PCB mounting is in scope; multi-die packages get the `led` kind's
  `"dies"` (F). LEDs simulate as today: lit while anode reads 1 and cathode 0.
- Code letters wait for plan-68 complete (the rule plans 69–75 follow).
