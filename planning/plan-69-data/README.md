# plan-69 data: TVS diodes and ESD steering arrays

Working files of plan-69 (`planning/plan-69-A-tvs-data.md` … `plan-69-I-tvs-landing.md`).

## Written at planning (2026-11-25)

- `census/`: shared planning-census tools for plans 69–74 (`url.js`, `facets.sh`,
  `show.py`, `kw.js`) and their `README.md`, which holds every count, the owner's
  decisions and the download probes. **Do not edit them.** plan-69's own scripts go
  beside them under names no other plan uses (`tvs-*.py`, `tvs-*.sh`, `tvs-*.js`).
- `raw/c144.json`: category TVS Diodes (144), In Stock and Active, with every facet's counts.
- `raw/tvsz.json`: the same, Type Zener (`183` = 423536).
- `raw/tvss.json`: the same, Type Steering (Rail to Rail) (`183` = 408274).

Each raw file is `{"productCount", "filters": [{"id", "name", "o": [[text, value, count], …]}], "firstPage": [[mpn, maker, [[label, quantity], …]], …]}`.
Print one facet with `python3 -I census/show.py raw/tvsz.json Manufacturer`.

## Written by plan-69-A

| File | Content |
|---|---|
| `tvs-urls.tsv` | one listing URL per piece: `piece`, `type`, `maker`, `filters` (JSON), `url` |
| `listing/<piece>.json` | the saved page of each piece (`facets.sh` output: `n`, `f`, `p`) |
| `listing/pieces.tsv` | `piece`, `parent`, `n`, `rows`, `split_by` (the facet a piece over 100 was split by), `status` (`leaf`, `split`, `failed`) |
| `candidates.tsv` | one row per distinct maker part number: `mpn`, `type` (`zener` / `steering`), `company` (`littelfuse`, `vishay`, `yageo`, `semtech`, `diodes`), `maker` (DigiKey's label), `status`, `stock` (the row's own `In Stock` quantity), `grade`, `qualification`, `package`, `supplier_package`, `mounting`, `uni`, `bi` (channel counts), `vrwm`, `vbr`, `vc`, `ipp`, `ppk`, `cj` (DigiKey's text), `url` (DigiKey's datasheet link), `final` (the URL that answered a PDF), `pdf`, `rank` (stock order within type and company), `decision` (`candidate`, `reserve`, `add`, `skip`, `out`), `reason` |
| `url-check.tsv` | `url`, `final`, `bytes`, `ok` (1 = answered a PDF), `sha256` |
| `textscan.tsv` | `mpn`, `hit`, `scheme`, `dead`, `context` |
| `verdicts.tsv`, `verdict-overrides.tsv` | `mpn`, `verdict`, `reason`, `pdf` (`why` in the overrides) |
| (column numbers) | `candidates.tsv`: 1 `mpn`, 2 `type`, 3 `company`, 6 `stock`, 9 `package`, 12 `uni`, 13 `bi`, 14 `vrwm`, 22 `pdf`, 24 `decision`, 25 `reason`; `specs.tsv`: 1 `type`, 2 `company`, 3 `pdf`, 4 `package`, 5 `supplier_package`, 6 `uni`, 7 `bi`, 8 `rows` |
| `specs.tsv` | one row per (type, company, pdf, package, supplier_package, uni, bi) of the `add` rows: `rows`, `example`, then the empty `scheme`, `page`, `pinout`, `pins`, `pads`, `facts` columns that B fills |
| `footprints-needed.txt` | one line per (letter, company, package, supplier_package, pins) of the `add` rows, columns 1 `letter` (`C` two-terminal, `D` array), 2 `company`, 3 `package`, 4 `supplier_package`, 5 `pins`, 6 `pdfs`, 7 `parts`, 8 `existing` (a footprint id of the same family and pin count, or empty) |

## Written by B–D

- `specs.tsv` filled (B); `skip`/`out` rows of B in `verdicts.tsv`.
- `footprints-C.json`, `footprints-D.json` (`{"footprints": […]}`), `footprint-map.tsv` (C, D).
