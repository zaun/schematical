# plan-78-A: Potentiometers: listing, scope, per-part stock check

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-78 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-78 adds potentiometers to the parts library: rotary pots, trimmers, slide pots and thumbwheel
pots from the four makers with the most in-stock, active parts at DigiKey across those four
categories (planning census of 2026-11-25, `planning/plan-76-data/README.md` § Plan 78):

| Maker | Rotary (84) | Trimmer (80) | Slide (78) | Thumbwheel (77) | All four |
|---|---|---|---|---|---|
| **Bourns** | 1,014 | 2,073 | 108 | 57 | **3,252** |
| **Amphenol Piher** | 211 | 633 | — | — | **844** |
| **Vishay Sfernice** | 436 | 369 | — | — | **805** |
| **Same Sky** | 221 | 222 | 14 | — | **457** |
| TT Electronics/BI (5th, out) | 138 | 261 | 9 | 12 | 420 |

Each maker keeps its **300 most-stocked in-scope parts** across the four categories: at most
1,200 parts. A pot is a passive part like a fixed resistor (owner decision: no knob in the
simulation). It joins the existing `resistor` kind through a new device key, `"wipers"`, whose
two halves load as the weak links a resistor already is (plan-78-E). The app draws it with a new
potentiometer symbol, reference prefix `RV` (plan-78-F).

The single behavioral outcome of plan-78: loading the part files gives the baseline parts plus one
part per `add` row of `planning/plan-78-data/verdicts.tsv`; the drawer lists them under
**Discrete → Potentiometers → Rotary / Trimmer / Slide / Thumbwheel**; a placed Bourns
`3296W-1-103LF` draws as a resistor with a wiper arrow, numbers as `RV1`, shows "10 kΩ, linear,
25 turns", and a schematic that wires its wiper to an LED through VCC on pin 1 still runs
(Simulation › Run stays enabled); every part has a footprint whose pads match its drawing's PCB
layout; `ctest` passes with no new program (the new cases go into `resistor_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | listing, scope verdicts, cap, per-part stock check | large |
| B | datasheet downloads, text-scan verdicts, reserves, `specs.tsv`, `footprints-needed.txt` | large |
| C | Bourns footprints (`footprints-bourns.json`) | large |
| D | Amphenol Piher, Vishay Sfernice, Same Sky footprints (`footprints-other.json`) | large |
| E | backend: the `resistor` kind's `"wipers"`, `"pot"`, `"taper"`, `"turns"`; `resistor_test`; docs | medium |
| F | app: potentiometer symbol, facts, `RV` prefix, Part Editor fields, exports, tests | large |
| G | parts: `gen-pot.py`, Bourns devices and parts, footprints merged, datasheets, goldens | large |
| H | parts: Amphenol Piher, Vishay Sfernice, Same Sky | large |
| I | landing tier; `planning/todo.md` ticks | small |

Outcome of A:

- `candidates.tsv` lists every part DigiKey shows for the four categories and the four makers,
  In Stock + Active + no Marketplace, each row `in` or `out` with a reason (§ 3.2).
- No maker has more than 300 `in` rows across the four categories (§ 3.2 `cap`).
- `stock-check.tsv` holds DigiKey's own `In Stock` line, above 0, for every `in` row.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints,
  § `resistor`).
- `planning/plan-76-data/README.md` (the census) and `planning/plan-69-data/census/` (`url.js`,
  `facets.sh`, `show.py`, `kw.js`: used as they are).
- `planning/plan-78-data/README.md`: the files of this plan and the facet ids.
- Precedents: `planning/plan-74-A-ethernet-data.md` (listing in pieces under 100 rows, the
  `In Stock`-line rule, the cap and reserves); `planning/completed/plan-58-A-slide-switch-data.md`
  (`list.sh` / `check.py` retries); `planning/completed/plan-34-*` (the resistor library).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p78 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p78 -j8 && ctest --test-dir /tmp/logic-p78 -j8` → `100% tests passed` | MET (author, 2026-11-25 at 686408f4: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | at planning (686408f4): **P0 = R0 = 40832**, **F0 = 3993**. C, D, E, G and H re-measure at their start |
| Resistor baseline | `python3 -I -c "import json;d=json.load(open('schematical/schematical/Parts/resistor.json'));print(len(d['devices']), sum(len(x['parts']) for x in d['devices']))"` | at planning: **56 devices, 1975 parts** |
| Nothing of plan-78 exists yet | `grep -c -i 'wiper\|potentiometer\|trimmer' backend/src/models/resistor.c docs/parts-format.md`; `python3 -I -c "import json;print(sum(f['id'].startswith('POT-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"`; `ls datasheets/potentiometer` | MET (2026-11-25: 0 and 0; 0; no such directory). `datasheets/trimmer-potentiometers/` exists, empty and untracked (`git ls-files datasheets/trimmer-potentiometers \| wc -l` → 0); plan-78 does not use it |
| No other open plan adds pot keys, footprints or folders | `grep -l -i '"wipers"\|POT-\|datasheets/potentiometer\|PotentiometerSymbolView' planning/plan-*.md \| grep -v plan-78 \| wc -l` → 0 | MET (2026-11-25: 0). Plans 76, 77, 79–82 are written at the same time; none touches `resistor.c` |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/80" 10 'document.title'` → a title starting `Trimmer Potentiometers` | MET at planning (2026-11-25: the census pages of `planning/plan-76-data/raw/` loaded). When the command prints `Just a moment...`, the owner ticks the box once in the DigiKey tab |
| `lz-string` for the census URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls /tmp/lz/node_modules/lz-string` → a directory | MET at planning (2026-11-25) |
| Tools | `pdftotext -v`, `pdfinfo -v`, `pdftoppm -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25: used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `planning/plan-78-data/candidates.tsv` has one row per part of the A listing (§ 3.1): the four
  categories, the four makers, In Stock + Active + no Marketplace. Each row has `scope` = `in` or
  `out` with a reason (§ 3.2). No maker has more than 300 `in` rows.
- `stock-check.tsv` has one row per `in` row: DigiKey's own `In Stock` quantity (> 0) and status.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `scripts/digikey.js` and the shared census tools of `planning/plan-69-data/census/` are not
  changed. AGENTS.md: stock is looked up in Safari.
- Out of scope, and never re-opened in later letters:
  - pots with an integrated switch, motorized pots, illuminated-shaft pots, and rheostats with two
    terminals (no wiper of their own);
  - panel-mount pots with solder lugs, solder turrets, solder hooks, wire leads or screw terminals
    (no PCB land pattern);
  - value-display pots (DigiKey 81), joysticks, encoders, and digital potentiometers (ICs; a
    different category);
  - TT Electronics/BI, Nidec and every maker outside the four.

## 2. Current State

There is no potentiometer in the library: `grep -c -i 'potentiometer\|trimmer'
backend/tests/golden/parts.txt` → 0 (2026-11-25). The `resistor` kind
(`backend/src/models/resistor.c`, 285 lines) holds fixed resistors and networks: device key
`"resistors"` (pairs `{"a","b"}`), part keys `ohms`, `tolerance`, `power`
(`docs/parts-format.md` § `resistor`). `footprints.json` (3993) has no pot footprint (no id
starting `POT-`; `RES-` ids are chip and axial resistors only).

### Measured populations

All DigiKey numbers: 2026-11-25, In Stock + Active + Exclude Marketplace, facet counts of the
planning census, printed by `python3 -I planning/plan-69-data/census/show.py
planning/plan-76-data/raw/<file> Manufacturer`.

| What | Count | Source |
|---|---|---|
| Rotary (84), all makers | 2,744 | `m84.json` |
| …Termination Style: Solder Lug 1,199; PC Pins 908; PC Pins, Board Locks 453; Solder Turret 113; Wire Leads 30; Solder Hook 11; Screw 3 | category-wide | `show.py m84.json 'Termination Style'` |
| …Gangs 1 2,441 / 2 272 / 4 8; Taper Linear 2,282 / Logarithmic 397 / Reverse Log 39; Turns 1: 2,446, 10: 214 | category-wide | `show.py m84.json 'Number of Gangs' Taper 'Number of Turns'` |
| Trimmer (80), all makers | 4,053 | `m80.json` |
| …Termination Style: PC Pins 3,062; J Lead 539; Gull Wing 405; Surface Mount 16; PC Pin 9; Wire Leads 8; Solder Hook 6; Z-Bend 1 | category-wide | `show.py m80.json 'Termination Style'` |
| …Turns: 1: 2,340, 25: 429, 12: 378, 15: 149, 11: 142, 20: 115 (17 values) | category-wide | `show.py m80.json 'Number of Turns'` |
| Slide (78) | 136: Bourns 108, Same Sky 14; Mounting Through Hole 134, Chassis 2 | `m78.json` |
| Thumbwheel (77) | 74: Bourns 57; Mounting Through Hole 72, Surface Mount 2 | `m77.json` |
| Upper bound on A's `in` rows | 4 × 300 = **1,200** (each maker's all-four total is above 300: 3,252, 844, 805, 457) | the table above. The scope filters only lower a maker's pool; A records the exact counts |

### Verified properties

- **DigiKey's In Stock facet counts Marketplace rows unless `-9` = 1 is set**; with it, the rows
  of a page show DigiKey's own `In Stock` line (`planning/plan-76-data/README.md`, checked on
  `m105.json`: 98 of 100 rows `In Stock` only, 2 with an extra `Alternate Stock` or `Factory`
  line). Only the `In Stock` line, above 0, counts (AGENTS.md; plan-58-A).
- **A census page holds at most 100 products** (`facets.sh` saves the page's own `__NEXT_DATA__`).
  A (category, maker) over 100 is listed in pieces (§ 3.1).
- The four makers' pools are above 300 even before scope (table above), so every maker's cap is
  reached unless scope removes more than its excess.

## 3. Design

### 3.1 The listing (`census/list.sh`, `census/check.py`)

One page per (category, maker, piece), saved to
`planning/plan-78-data/raw-A/<category>_<maker>_<piece>.json` by
`planning/plan-69-data/census/facets.sh <out> x/<category> '<extra>'`, `<extra>` holding
`"-9":[1]`, the category's scope filter and the maker (`planning/plan-78-data/README.md`). A
page's `productCount` over 100 is split by **Series** (`-4`): `list.sh` first loads the
(category, maker) page, reads the Series facet's options and their counts, and loads one page per
Series group, packing Series options greedily into groups under 100. A single Series over 100 is
split again by Resistance (`2085` in 80, 77, 78; `1` in 84).

`check.py` passes a page when its rows are all of the requested maker and its row count equals
`min(productCount, 100)`; `list.sh` retries a failing page up to 5 times (plan-58-A's precedent:
Safari sometimes returns another tab's page).

### 3.2 Scope and cap (`census/build.py`)

`build.py` writes `candidates.tsv`, one row per distinct (maker, part number), columns:

`category  maker  part  stock  status  digikey_id  description  series  ohms  tolerance  power
taper  gangs  turns  adjust  termination  mounting  size  datasheet_url  scope  reason  verdict
verdict_reason  pdf  stock_checked`

`category` is `rotary`, `trimmer`, `slide` or `thumbwheel`; `maker` the slug `bourns`, `piher`,
`sfernice`, `samesky` (the datasheet folder names). `stock` is the `In Stock` line only (0 when
absent). `ohms`, `tolerance`, `power`, `taper`, `gangs`, `turns`, `adjust`, `termination`,
`mounting`, `size` are DigiKey's parameters as text.

`out` reasons, the first that applies:

1. `no DigiKey stock`: `stock` = 0.
2. `status`: not Active.
3. `switch`: the description contains `SW`, `SWITCH` or `SPST`/`DPST`, or Features names a switch.
4. `motorized` / `illuminated`: Features `Motorized`, `Illuminated Shaft…`.
5. `gangs`: more than 2 gangs (4-gang rotary pots).
6. `cap`: beyond the maker's 300 highest-`stock` `in` rows across the four categories (ties broken
   by part number, ascending).

The scope filters of § 3.1 (termination, mounting) are applied by the page request, so A never sees
a solder-lug pot.

### 3.3 Stock check (`stock-check.tsv`)

The check is each row's own `In Stock` line, read from its saved page's product cell, not the
facet. Columns `maker part in_stock status checked` (the date). A row whose cell shows no
`In Stock` line, or 0, is `out` (`no DigiKey stock`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Listing

- [ ] `planning/plan-78-data/census/list.sh` and `check.py` per § 3.1.
- [ ] Run the listing into `planning/plan-78-data/raw-A/`.

Acceptance: every (category, maker) is covered by pages whose row counts add up to its
`productCount`, and every page matches its request.
  Check: `python3 -I planning/plan-78-data/census/check.py --all planning/plan-78-data/raw-A` → `pages ok` and one line per (category, maker) `rows == productCount` (est. 40 min: about one Safari page per 100 rows, at most 1 + 33 + 9 + 9 + 5 for Bourns, fewer for the others).
  Result:
Commit:

### Phase 2 — Scope, cap, stock check

- [ ] `census/build.py` per § 3.2–3.3; write `candidates.tsv` and `stock-check.tsv`.
- [ ] Record in this section the counts by maker, category and `scope` / `reason`
      (`awk -F'\t' 'NR>1{print $2, $1, $20, $21}' planning/plan-78-data/candidates.tsv | sort | uniq -c`).

Acceptance: no maker has more than 300 `in` rows, and every `in` row has an `In Stock` line above 0.
  Check: `python3 -I planning/plan-78-data/census/build.py --verify planning/plan-78-data/raw-A planning/plan-78-data/candidates.tsv` → `cap ok, stock ok` with the four `in` counts, each ≤ 300 (est. 1 min: local files only).
  Result:
Commit:

## Validation Plan

- A's checks are its two acceptance commands. Nothing in `backend/` or the app changes.
- Final gate: plan-78-I.

## Decisions

- Owner, 2026-11-25: **the four makers with the most in-stock parts, 300 most-stocked each.** The
  ranking is over the four pot categories together, before A's scope filters (Bourns 3,252, Amphenol
  Piher 844, Vishay Sfernice 805, Same Sky 457; TT Electronics/BI 420 is fifth).
- Owner, 2026-11-25: **no simulation knob.** Pots carry facts only; they do not model a wiper
  position (§ E).
- Author: **board mount only: PC pins, J-lead, gull-wing, SMD or Z-bend terminations.** A
  schematic part needs a land pattern; solder lugs and wire leads have none. Panel-mount rotary pots
  with PC pins are in: their terminals go into the board.
- Author: **switch, motorized and illuminated pots are out**: each is a pot plus another part
  (a switch, a motor, an LED) the resistor kind does not model.
- Author: **at most 2 gangs.** The 8 four-gang rotary pots (category-wide) have 12+ terminals that
  no symbol of plan-78-F draws; 1- and 2-gang pots cover the hobby uses.
- Author: **Marketplace rows are excluded in the request** (`-9` = 1), as in the census.

## Corrections

## Summary

A lists the four pot categories of the four makers through Safari, keeps the board-mount single and
dual pots within each maker's 300 cap, and checks each one's own DigiKey stock line. B downloads
their datasheets and gives every row a verdict.
