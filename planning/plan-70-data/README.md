# plan-70 data — optocouplers and digital isolators

Written at planning time (2026-11-25); each letter adds its files and updates this list.

## At planning time

- `raw/c903.json`, `raw/c902.json`, `raw/c904.json`, `raw/c901.json`: the four DigiKey
  category pages (transistor / photovoltaic-output optoisolators 903, logic-output
  optoisolators 902, triac / SCR-output optoisolators 904, digital isolators 901). Each was
  filtered In Stock + Active only and saved with `planning/plan-69-data/census/facets.sh`
  (2026-11-25). Each file is `{"productCount", "filters", "firstPage"}`: every facet with
  each option's `[text, value id, productCount]`, and the first 100 products as
  `[part number, maker label, [[In Stock label, quantity], …]]`. Print a facet with
  `python3 -I planning/plan-69-data/census/show.py planning/plan-70-data/raw/c903.json Manufacturer "Output Type"`.
- The census tools are shared and read-only for this plan: `planning/plan-69-data/census/`
  (`url.js`, `facets.sh`, `show.py`, `kw.js`, `README.md` with the owner's decisions).

## Written by the letters

| File | Letter | What |
|---|---|---|
| `census/make-urls.js` | A | the scoped filter URL of each (type, maker) piece and its splits |
| `census/list.sh`, `census/check.py` | A | the Safari listing (one saved page per piece, retried until it matches its request) |
| `census/build.py` | A | `candidates.tsv` from the saved pages, ranked per (type, maker) by DigiKey's own In Stock quantity |
| `listing/` | A | the saved pages (`<type>_<maker>[_<split>].json`) |
| `candidates.tsv` | A, B, D | one row per listed part: `type maker mpn series output input channels side1 side2 package supplier_package mounting grade status stock rank pick datasheet_url detail` (A; `stock` is the row's own `In Stock` line); `pdf hit dead decision reason` (B); final `decision` (D) |
| `census/fetch.py`, `textscan.py`, `decide.py`, `place.py`, `maker-urls.py`, `specs-skeleton.py` | B | downloads, text scan, verdicts, placement, the specs skeleton (adapted from `planning/plan-58-data/census/` and `planning/plan-48-data/census/`) |
| `url-check.tsv`, `textscan.tsv`, `maker-urls.tsv` | B | per URL / per row results |
| `specs.tsv` | B, C, D | one row per (type, maker, PDF, package, channels) of the kept rows; pinout and fact columns filled by C and D |
| `footprints-needed.txt` | D | one line per package line of the final pick, `letter` E or F |
| `footprints-E.json`, `footprints-F.json`, `footprint-map.tsv` | E, F | new footprint objects and the line → footprint map |
| `census/gen-parts.py` | H | writes `schematical/schematical/Parts/isolator.json` |

## Datasheet folders

`datasheets/isolator/<maker>/` with `maker` one of `isocom`, `vishay`, `onsemi`, `broadcom`,
`toshiba`, `renesas`, `wurth`, `adi`, `ti`, `skyworks`; one `datasheets.tsv` per folder
(`file`, `url`, `bytes`, `sha256`). The empty, DigiKey-named folders
`datasheets/digital-isolators/` and `datasheets/optoelectronics-accessories/` are not used.
