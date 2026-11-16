# plan-48-data

Data and scripts of plan-48 (555-type timers and common op-amps from the top
three makers of each type at DigiKey). Written at planning time, 2026-11-16.

| Path | What |
|---|---|
| `census/dk.js` | the DigiKey calls `count.js` and `list.js` share: login (the token `scripts/digikey.js` caches), one keyword search with category, maker and parametric filters, in stock, status Active or Last Time Buy |
| `census/count.js` | one call: the in-stock count of a category, optionally for some makers and parametric values, with the maker ranking and the facet splits (value ids included) |
| `census/list.js`, `census/list.sh` | the planning listing: the 300 most-stocked parts of each piece, most-stocked first (`raw/`); `LIST_CAP=50` lists one page |
| `raw/count-timer.txt`, `raw/count-opamp.txt` | the two `count.js` runs of plan-48-A § 2 (maker ranking, facets) |
| `raw/*.jsonl.gz`, `raw/*.err`, `raw/*.done` | the listing: one JSON line per part; `piece <name>: <listed> of <count> (cut at 300: <stock of the 300th>)` and the `list.js:` totals |
| `census/common.py` | shared code: reading the listing, the company of a DigiKey maker name, the scope rules, the maker-site datasheet URL rules |
| `census/urlcheck.py`, `url-check.tsv` | the distinct maker-site datasheet URLs of the in-scope rows and which answer with a PDF through Python's `urllib` (`final` = the URL that answered); the PDFs stay in the cache directory given on the command line (`<md5 of url>.pdf`) |
| `census/select.py` | `candidates.tsv` from the listing (`--urls`, `--pdf`; later `--stock` and `--verdicts`): scope, datasheet, the 200-per-maker quota, the literal part-number scan of each PDF |
| `candidates.tsv` | one row per listed part (1032): `state` `candidate` 662 (timer 62, op-amp 600), `reserve` 294, `out` 76 with its `reason`; `rank` inside its (type, company) |
| `census/to-check.py`, `census/stock-check.sh` | plan-48-A phase 1: the part-by-part stock check with `node scripts/digikey.js` (one call per part), into `stock-check.tsv` |
| `census/footprint-lines.py`, `footprints-needed.txt` | one line per distinct package of the download set (`--add`: of the final pick), the plan-48 letter that draws it (C leaded, D leadless and ball) and the existing footprints of the same family and pin count |

`candidates.tsv` columns, in order (the plans' `awk` commands use these
numbers): 1 `mpn`, 2 `company` (`ti`, `adi`, `st`, `renesas`), 3 `type`
(`timer`, `opamp`), 4 `class` (timer: `single`, `dual`; op-amp: `gp`,
`jfet`, `cmos`), 5 `circuits`, 6 `status`, 7 `stock`, 8 `package`, 9
`supplier_package`, 10 `mounting`, 11 `grade` (`AEC-Q100` or empty), 12
`supply_min`, 13 `supply_max`, 14 `output`, 15 `piece`, 16 `cut` (1 = its
piece hit the 300-result cap), 17 `maker` (DigiKey's name), 18
`description`, 19 `via` (`link` or `guess`), 20 `url`, 21 `final` (the PDF's
plain URL), 22 `state`, 23 `reason`, 24 `rank`, 25 `pdf` (cache file), 26
`hit` (1 = the part number stands in the PDF's text), 27 `stock_checked`.

`footprints-needed.txt` columns: 1 `letter`, 2 `company`, 3 `mounting`, 4
`package`, 5 `supplier_package`, 6 `pins`, 7 `types`, 8 `candidates`, 9
`parts`, 10 `example`, 11 `datasheets`, 12 `existing`.

The download set is every `candidate` and `reserve` row (956 at planning
time, 422 distinct URLs, 418 distinct PDFs). Reproduce (no DigiKey call is
made by the last three; `urlcheck.py` downloads about 650 MB):

```sh
cd planning/plan-48-data
census/list.sh raw                       # 21 calls; skip: raw/ is in the tree
python3 -I census/urlcheck.py raw /tmp/p48/pdf > url-check.tsv        # about 10 min cold
python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --pdf /tmp/p48/pdf --report
python3 -I census/footprint-lines.py candidates.tsv ../../schematical/schematical/Parts/footprints.json > footprints-needed.txt
```

`list.sh`, `count.js` and `stock-check.sh` need `DIGIKEY_CLIENT_ID` /
`DIGIKEY_CLIENT_SECRET` (`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`).
The listing was gzipped after the run (`gzip -9 raw/*.jsonl`); `common.py`
reads both forms.
