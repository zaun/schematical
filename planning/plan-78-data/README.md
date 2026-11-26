# plan-78 data: potentiometers

Files of plan-78 (A → B → C → D → E → F → G → H → I). The planning census is shared with
plans 76–82: `planning/plan-76-data/README.md` § Plan 78, raw pages
`planning/plan-76-data/raw/m84.json`, `m80.json`, `m78.json`, `m77.json`.

| File | Written by | Content |
|---|---|---|
| `census/list.sh`, `census/check.py`, `census/build.py` | A | the listing (Safari pages per category × maker × piece), the page check, `candidates.tsv` |
| `raw-A/<category>_<maker>_<piece>.json` | A | one saved DigiKey page per listing piece (`planning/plan-69-data/census/facets.sh`) |
| `candidates.tsv` | A | one row per (maker, part): DigiKey facts, `scope` / `reason`, `verdict` |
| `stock-check.tsv` | A | `maker part in_stock status checked` for every `in` row |
| `census/fetch.py`, `census/textscan.py` | B | downloads (with `--probe`), verdicts by text scan |
| `verdicts.tsv` | B | `add` / `skip` per `in` row, with a reason |
| `specs.tsv` | B | one row per (maker, series drawing): type, gangs, pin names, turns, taper, footprint id |
| `footprints-needed.txt` | B | one footprint id per line, sorted, unique |
| `footprints-bourns.json` | C | Bourns land patterns |
| `footprints-other.json` | D | Amphenol Piher, Vishay Sfernice and Same Sky land patterns |
| `census/fp-load.sh`, `census/fp-merge.py` | C | copied from `planning/plan-58-data/census/`, scratch dir `/tmp/p78-fp.XXXXXX` |
| `census/gen-pot.py` | G | `specs.tsv` + `verdicts.tsv` → devices in `Parts/resistor.json` (`--check`) |

Facet ids the listing uses (all from the raw pages above):

| Category | id | Scope filter | Manufacturer ids |
|---|---|---|---|
| Rotary Potentiometers, Rheostats | 84 | Termination Style `258` ∈ {PC Pins 384858, PC Pins Board Locks 384860} | Bourns 118, Amphenol Piher 1993, Vishay Sfernice 716, Same Sky 2223 |
| Trimmer Potentiometers | 80 | Termination Style `258` ∈ {PC Pins 384858, PC Pin 384842, J Lead 361793, Gull Wing 352076, Surface Mount 409393, Z-Bend 423508} | same |
| Slide Potentiometers | 78 | Mounting Type `69` = Through Hole 411897 | same |
| Thumbwheel Potentiometers | 77 | Mounting Type `69` ∈ {Through Hole 411897, Surface Mount 409393} | same |

Every page also carries In Stock (`-2` = 5), Active (`1989` = 0) and Exclude Marketplace
(`-9` = 1), added by `url.js` and the `-9` extra filter.
