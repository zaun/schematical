# plan-79-A: Battery holders: listing, stock check, scope and verdicts

Last updated: 2026-11-25
Overall Effort: huge (> 3d). plan-79 is A → B → C → D → E → F → G → H → I → J
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-79 adds PCB-mount battery holders, retainers, clips and contacts to the parts library, from the
four makers with the most in-stock PCB-mount parts at DigiKey (planning census of 2026-11-25,
`planning/plan-76-data/README.md` § Plan 79):

| Maker | DigiKey id | In-stock PCB-mount parts | In plan-79 |
|---|---|---|---|
| **Keystone Electronics** | 36 | 361 | its 300 most-stocked |
| **MPD (Memory Protection Devices)** | 61 | 249 | all |
| **Adam Tech** | 2057 | 74 | all |
| **Assmann WSW Components** | 123 | 53 | all |

At most 676 parts. They join the existing `connector` kind as a new mode, `"battery"` (C): passive
terminals like a USB connector. A battery holder does not supply a voltage in play mode; the
`supply` kind is the power symbol for that.

The single behavioral outcome of plan-79: loading the part files gives the baseline parts plus one
part per `add` row of `planning/plan-79-data/verdicts.tsv`; the app lists them under
**Connector → Battery → Coin / Cylindrical / 9 V**; a placed Keystone coin cell holder draws as a
battery symbol with `+` and `−` pins, takes reference `BT1`, shows its cell size and cell count in
its details, and has a footprint whose pads match the drawing's PCB layout.

| Letter | Content | Effort |
|---|---|---|
| **A** | the listing, scope, rank and 300 cap, per-part stock check, `verdicts.tsv` and `specs.tsv` skeletons | large |
| B | datasheet and drawing downloads, the obsolete check, `drawings.tsv` (how each drawing reads) | large |
| C | backend: the connector's `"battery"` mode, `connector_test` cases, docs | medium |
| D | Keystone coin cell holders, retainers and contacts: specs, footprints, `merge-fp.py`, `gen-battery.py`, parts, goldens | large |
| E | Keystone cylindrical and 9 V holders, clips and contacts: specs, footprints, parts, goldens | large |
| F | MPD coin cell holders, retainers and contacts | large |
| G | MPD cylindrical and 9 V holders, clips and contacts | large |
| H | Adam Tech and Assmann WSW | large |
| I | app: battery symbol, details, drawer, reference prefix, Part Editor fields, BOM, tests, harness | large |
| J | landing tier | small |

Outcome of A:

- `candidates.tsv` lists every part DigiKey shows in category 86 for the four makers, PCB mount,
  In Stock + Active + no Marketplace, with a scope verdict and a stock rank per maker.
- `stock-check.tsv` checks every `in` row: DigiKey's own `In Stock` line above 0, status Active.
- `verdicts.tsv` gives each `in` row `add`, `pending` (B–H read its drawing) or `skip` with a reason.
- `specs.tsv` exists with one row per (maker, datasheet URL) of the `add` and `pending` rows, its
  pin, footprint and drawing columns empty.

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § Datasheets, § Footprints,
  § `connector`); `docs/backend.md` (§ goldens).
- `planning/plan-76-data/README.md`: the planning census of plans 76–82. Its pages were filtered
  In Stock (`-2` = 5), Active (`1989` = 0) and Exclude Marketplace Products (`-9` = 1).
- `planning/plan-69-data/census/` (`url.js`, `facets.sh`, `show.py`, `kw.js`): A calls them and
  does not change them.
- `planning/plan-79-data/README.md`: the files of this plan.
- Precedents: `planning/plan-74-A-ethernet-data.md` (the same listing on another category, the
  `In Stock`-line rule); `planning/completed/plan-58-A-slide-switch-data.md` (`list.sh`/`check.py`
  retries, `pending` verdicts); `planning/completed/plan-48-A-timer-opamp-data.md` (the per-maker
  cap with reserve rows); `planning/completed/plan-45-A-terminal-jst-data.md` (connector families).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate. Every other letter
points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p79 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p79 -j8 && ctest --test-dir /tmp/logic-p79 -j8` → `100% tests passed` | MET (measured by the author at planning, 2026-11-25 at 686408f4: `100% tests passed, 0 tests failed out of 77`); **T0 = 77** (`grep -c '^logic_test' backend/CMakeLists.txt` → 77) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `erc_pins.txt` / `footprints.txt` | at planning (686408f4): **P0 = R0 = E0 = 40832**, **F0 = 3993**. C–H re-measure at their start and use their own |
| Connector baseline | `grep -c '|connector|' backend/tests/golden/parts.txt`; `grep -c '"id"' schematical/schematical/Parts/connector.json` | at planning: **C0 = 1541** connector parts, 567 device ids |
| Nothing of plan-79 exists yet | `grep -c '"battery"' backend/src/models/connector.c` → 0; `python3 -I -c "import json;print(sum(f['id'].startswith('BAT-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0; `ls datasheets/connector-battery` → no such directory; `ls schematical/schematical/Canvas/BatterySymbolView.swift` → no such file; `grep -c 'mfr=Keystone\|mfr=MPD\|mfr=Adam Tech' backend/tests/golden/parts.txt` → 0 | MET (2026-11-25: 0; 0; no directory; no file; 0. Assmann already has 38 parts in other kinds: `grep -c 'mfr=Assmann' backend/tests/golden/parts.txt` → 38) |
| No other open plan adds this mode, its keys or folders | `grep -l -i 'connector-battery\|"battery"\|BatterySymbolView' planning/plan-*.md \| grep -v plan-79 \| wc -l` → 0 | MET (2026-11-25: 0) |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/86" 10 'document.title'` → a title that starts `Battery Holders, Clips, Contacts` | MET at planning (2026-11-25: the census loaded `x/86` twice, `m86.json` and `p86.json`). When the command prints `Just a moment...`, the owner ticks "Verify you are human" once in the DigiKey tab |
| `lz-string` for the census URLs | `mkdir -p /tmp/lz && cd /tmp/lz && npm i lz-string && ls /tmp/lz/node_modules/lz-string` → a directory | MET at planning (2026-11-25) |
| Tools | `pdftotext -v`, `pdfinfo -v`, `pdftoppm -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-25: all used at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to
> stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `planning/plan-79-data/candidates.tsv` has one row per part of the listing (§ 3.1), each with
  `scope` = `in`, `reserve` or `out` (with a reason) and `rank` (1 = most stocked, per maker).
- No maker has more than 300 `in` rows. Keystone's rows past its 300th `in` row are `reserve`, in
  rank order (§ 3.3).
- `stock-check.tsv` has one row per `in` and `reserve` row: DigiKey's own `In Stock` quantity and
  status.
- `verdicts.tsv` has one row per `in` row: `add`, `pending` or `skip` with a reason.
- `datasheets/skipped-parts.md` has a section "Battery holders (plan-79)" listing every `skip` row.
- `specs.tsv` has one row per (maker, datasheet URL) of the `add` and `pending` rows.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `scripts/digikey.js` and the census tools of `planning/plan-69-data/census/` are not changed.
  AGENTS.md: stock is looked up in Safari.
- Out of scope, and never re-opened in later letters:
  - chassis-, panel-mount, free-hanging (wire-lead) and custom-mount holders: no footprint;
  - battery packs, chargers and battery-product accessories (categories 85, 87, 89);
  - mechanical-only clips that touch neither end of the cell (no electrical terminal);
  - every maker but the four.

## 2. Current State

There is no battery holder in the library (`grep -c 'mfr=Keystone\|mfr=MPD\|mfr=Adam Tech'
backend/tests/golden/parts.txt` → 0, 2026-11-25), no `BAT-` footprint, and no battery code. The
app already has a `.battery` reference category with prefix `BT`
(`schematical/schematical/Preferences/ReferencePrefixes.swift:7,18,50`), but
`ReferenceCategory.of` maps no kind to it (line 75: `PART_CONNECTOR` → `.testPoint` or
`.femaleConnector`).

The `connector` kind (`backend/src/models/connector.c`) has the modes barrel (default), `"usb"`,
`"card"`, `"testPoint"`, `"terminal"` and `"jst"`; plan-74-E adds `"rj45"`. Every mode is a block of
passive `inout` terminals. C adds `"battery"` the same way.

### Measured populations

All DigiKey numbers: 2026-11-25, In Stock + Active + Exclude Marketplace, facet counts
(`python3 -I planning/plan-69-data/census/show.py planning/plan-76-data/raw/p86.json Manufacturer
"Battery Type, Function"`).

| What | Count | Source |
|---|---|---|
| category 86, all mounts | 1,360: Keystone 622, MPD 425, Adam Tech 74, Assmann 60, Bulgin 28 | `raw/m86.json` |
| …PCB mount (Mounting Type `69` ∈ 385069 PCB SMT, 385070 PCB TH, 409393 Surface Mount) | **780**: Keystone 361, MPD 249, Adam Tech 74, Assmann 53, TE Linx 15, Würth 8, TE AMP 7, Bulgin 6, Harwin 5, Jauch 1, Laird 1 | `raw/p86.json` |
| …by function (all makers) | coin holder 255, coin retainer 222, cylindrical holder 176, cylindrical contact (single) 41, coin contact 36, cylindrical clip 31, 9 V holder 8, 9 V snap contact (single) 6, 9 V snap connector 2, cylindrical contact (dual) 2 | `raw/p86.json` |
| Upper bound on `in` rows | 300 + 249 + 74 + 53 = **676** | the rows above |
| Datasheet hosts (first 100 rows of category 86, all mounts) | Keystone `www.keyelco.com/userAssets/file/…` 83 rows on 20 distinct PDFs; MPD `www.memoryprotectiondevices.com/datasheets/<series>/<series>-datasheet.pdf` 14, DigiKey copy `mm.digikey.com` 1; TE `www.te.com` DDEController 2 | the census page's `datasheetUrl` fields, read at planning |

### Verified properties

- **DigiKey's In Stock facet also counts Marketplace and Factory stock** (plan-58-A, plan-74-A § 2).
  The census excludes Marketplace with `-9` = 1; only DigiKey's own `In Stock` line, above 0,
  counts for a part (AGENTS.md).
- **A census page holds at most 100 products.** Keystone (361) and MPD (249) are listed in pieces
  (§ 3.1).
- **Both big makers serve PDFs to Python** (2026-11-25, `urllib` with a Safari user agent):
  `https://www.keyelco.com/userAssets/file/K75p16.pdf` → 632,180 B, `%PDF-`, one catalog page
  (title "KEYSTONE 75TH ANNIVERSARY CATALOG K75 P16", 376 words of text, which says "Refer to page
  41 for Battery Contact Layout": Keystone's land patterns are on other catalog pages, B fetches
  them); `https://www.memoryprotectiondevices.com/datasheets/BCAA/BCAA-datasheet.pdf` → 141,724 B,
  `%PDF-`, one page, 42 words of text: the drawing's dimensions are vector outlines, not text, so
  its numbers are read from a `pdftoppm` render (B's `drawings.tsv` records which PDFs are so).

## 3. Design

### 3.1 The listing (`census/list.sh`, `census/check.py`, `census/build.py`)

One page per (maker, piece), saved to `planning/plan-79-data/raw-A/<maker>_<piece>.json` by
`planning/plan-69-data/census/facets.sh` with the extra filters
`{"-9":[1],"69":[385069,385070,409393],"-1":[<maker id>]}` plus the piece's filter:

- Keystone (361): split by Battery Type, Function `1445`: coin holder 332100, coin retainer 332102,
  coin contact 332099, cylindrical holder 336683, every other function id of `p86.json`'s `1445`
  facet. A piece still over 100 is split again by Mounting Type (385069 / 385070).
- MPD (249): the same function split.
- Adam Tech (74), Assmann (53): one page each.

`check.py <page> <maker id>` exits 1 when the page has no rows, when a row's maker is another maker
(Safari sometimes answers with another tab's page; plan-58-A), or when the row count differs from
the page's `productCount` and that count is at most 100. `list.sh` retries a page up to 5 times
with a 10 s pause. `build.py` writes `candidates.tsv`, one row per distinct (maker, part number):

`maker  part  stock  status  digikey_id  description  function  cell_size  cells  mounting  termination  datasheet_url  scope  rank  reason`

`build.py --expect <maker>` (used by E–H) prints the number of parts that maker must end with:
min(300, its `in` + `reserve` rows whose verdict is not `skip`).

`stock` is DigiKey's own `In Stock` quantity of the row (0 when only Marketplace or Factory). A
product listed with two packagings is one row, with the larger `In Stock`.

### 3.2 Scope

A row is `out` with a reason when:

- `stock` is 0 (`no DigiKey stock`), or status is not Active (`status <s>`);
- its function is a mechanical-only clip: description or drawing says "retainer clip" with no
  contact to a cell end (`mechanical only`). A coin cell **retainer** (DigiKey's "Coin Cell,
  Retainer") is in: its spring touches the cell's top (`+`) and the PCB pad under the cell is `−`;
- it is an accessory (a cover, an insulator, a pull tab): `accessory`.

Every other row is `in` (before the cap).

### 3.3 Rank, cap and reserve

`rank` orders a maker's `in` rows by `stock`, largest first, ties by part number. Rows ranked 1–300
stay `in`; rows ranked 301 and up are `reserve` (only Keystone has any). When a later step turns an
`in` row into `skip` (no drawing, a drawing with no PCB layout and no tab dimensions, an obsolete
PDF), the best-ranked `reserve` row becomes `in` and gets its own verdict, so a maker ends with
min(300, its `in` + `reserve` rows that are not `skip`) parts. Each promotion is a line in
`verdicts.tsv` (`promoted for <part>`).

### 3.4 Stock check and verdicts

`stock-check.tsv`: `maker part in_stock status checked` (the date). Its numbers come from the
listing's own rows (each row carries its `In Stock` line); a row whose page is older than 7 days at
the time of a later letter is re-checked there.

`verdicts.tsv`: `maker part verdict reason datasheet_url`. A's verdicts:

- `add` when the row has a datasheet URL on the maker's own host (§ 2);
- `pending` when its only datasheet URL is DigiKey's copy (`mm.digikey.com`): `datasheets.json`
  takes only the maker's own URL (`docs/parts-format.md` § Datasheets), so B tries the maker's
  pattern (MPD: `https://www.memoryprotectiondevices.com/datasheets/<series>/<series>-datasheet.pdf`)
  and turns the row `add` or `skip` (`no maker datasheet`);
- `skip` with `no datasheet` when DigiKey lists none;
- `pending` for every MPD row (its drawings are outlined vectors: B records them, F and G read the
  render) and for every Keystone row whose catalog page refers to another page for the layout.

### 3.5 `specs.tsv` skeleton

One row per (maker, datasheet URL) of the `add` and `pending` rows:

`maker  datasheet  parts  function  cell  cells  pins  pin_names  footprint  layout_page  drawing_kind  note`

A fills `maker`, `datasheet`, `parts` (the part numbers, comma-separated), `function`, `cell` and
`cells` (from DigiKey's Battery Cell Size and Number of Cells facets, mapped per § 3.2 of plan-79-C).
The other columns are empty; B fills `layout_page` and `drawing_kind`, D–H the rest.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The listing

- [ ] `planning/plan-79-data/census/list.sh`, `check.py`, `build.py` per § 3.1.
- [ ] Run `list.sh`; every page passes `check.py`; save `raw-A/`.
- [ ] `build.py` → `candidates.tsv`.

Acceptance: the listing covers every PCB-mount part of the four makers.
  Check: `python3 -I planning/plan-79-data/census/build.py --counts` → one line per maker whose row
  count equals that maker's `productCount` at listing time (planning: Keystone 361, MPD 249, Adam
  Tech 74, Assmann 53; a later count may differ, the line prints both) (est. 30 min: about 16 pages
  through Safari).
  Result:
Commit:

### Phase 2 — Scope, rank, cap, stock

- [ ] § 3.2 and § 3.3 in `build.py`: `scope`, `rank`, `reason`.
- [ ] `stock-check.tsv` per § 3.4.

Acceptance: no maker has more than 300 `in` rows, and every `in` row has stock above 0.
  Check: `python3 -I planning/plan-79-data/census/build.py --check` → `ok: in ≤ 300 per maker, every in row In Stock > 0 and Active` (est. < 1 min).
  Result:
Commit:

### Phase 3 — Verdicts and the specs skeleton

- [ ] `verdicts.tsv` per § 3.4; `datasheets/skipped-parts.md` § "Battery holders (plan-79)".
- [ ] `specs.tsv` per § 3.5.

Acceptance: every `in` row has a verdict and every `add` / `pending` row is in one `specs.tsv` row.
  Check: `python3 -I planning/plan-79-data/census/build.py --verdicts` → `ok: <n> in rows, <n> verdicts, every add/pending part in specs.tsv` (est. < 1 min).
  Result:
Commit:

## Validation Plan

- Phase checks above. No code changes, so no backend or app tier in A.
- Final gate: plan-79-J.

## Decisions

- Owner, 2026-11-25: top 4 makers by DigiKey in-stock count, 300 most-stocked parts per maker.
- Owner, 2026-11-25 (plans 76–82): the census excludes Marketplace stock; only DigiKey's own
  `In Stock` line counts (AGENTS.md).
- Author: **PCB mount only.** A wire-lead or chassis holder has no land pattern, so it would be a
  part with no footprint; such a holder is wired through a connector the library already has.
- Author: **reserve rows fill the cap** (plan-48-A's precedent), so a skipped part does not leave
  Keystone short of 300 when stocked parts remain.

## Corrections

## Summary

The Safari listing of the four makers' PCB-mount battery holders, capped at 300 per maker, with a
stock check and verdicts, the skeleton every later letter fills.
