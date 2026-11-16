# plan-47-data

Data and scripts of plan-47 (inductors, fuses, PTC resettable fuses, relays
and buzzers from the top three makers of each type at DigiKey). Written at
planning time, 2026-11-16.

| Path | What |
|---|---|
| `census/dk.js` | the DigiKey calls `count.js` and `list.js` share: login (the token `scripts/digikey.js` caches), one keyword search with category, maker and parametric filters, in stock, status Active or Last Time Buy |
| `census/count.js` | one call: the in-stock count of a category, optionally for some makers and parametric values, with the maker ranking and the facet splits (value ids included) |
| `census/list.js`, `census/list.sh` | the planning listing: the 300 most-stocked parts of each of 36 pieces (type × maker × mounting, or × relay category), most-stocked first (`raw/`) |
| `raw/*.jsonl.gz`, `raw/*.err`, `raw/*.done` | the listing: one JSON line per part; `piece <name>: <listed> of <count> (cut at 300: <stock of the 300th>)` and the `list.js:` totals |
| `clips/clips.jsonl`, `clips/clips.err` | the in-stock PCB fuse clips of the three fuse makers, most-stocked first (plan-47-A § 6) |
| `census/common.py` | shared code: reading the listing, the company of a DigiKey maker name, the scope and class rules, de-duplication, the quotas, the maker-site datasheet URL rules |
| `census/urls.py`, `urls-needed.tsv` | the distinct maker-site datasheet URLs of the in-scope rows (`via` = `link`, `sibling` or `guess`) |
| `census/urlcheck.py`, `url-check.tsv` | which of them answer with a PDF through Python's `urllib` (`final` = the URL that answered: a Panasonic file is tried under five directories); the PDFs stay in the cache directory given on the command line |
| `census/select.py` | `candidates.tsv` from the listing (and `--urls`, later `--stock` and `--keep`): scope, duplicates, datasheet, quota |
| `candidates.tsv` | one row per listed part (8075): `state` `candidate` 2953, `reserve` 569, `out` the rest with its `reason`; `rank` inside its (type, company, class); the last four columns (`pdf`, `hit`, `dead`, `stock_checked`) are filled by plan-47-A |
| `census/footprint-lines.py`, `footprints-needed.txt` | one line per distinct package of the download set (`candidate` + `reserve`; `--add`: of the final pick), the letter that draws it (D inductors, E fuses, F PTCs, G relays, H buzzers) and the existing footprints of the same size or package code |

`candidates.tsv` columns, in order (the plans' `awk` commands use these
numbers): 1 `mpn`, 2 `company`, 3 `type`, 4 `class`, 5 `series`, 6 `status`,
7 `stock`, 8 `value` (inductance, rated current, hold current, coil or input
voltage, frequency), 9 `form` (a relay's contact form or circuit, a fuse's
response time, a buzzer's technology), 10 `package`, 11 `supplier_package`,
12 `size`, 13 `mounting`, 14 `grade` (`AEC-Q200`, `Military` or empty), 15
`detail` (a relay's coil type and termination, a PTC's lead spacing, a
buzzer's termination, an inductor's shielding, a fuse's type), 16
`category`, 17 `piece`, 18 `cut` (1 = its piece hit the 300-result cap), 19
`maker` (DigiKey's name), 20 `description`, 21 `via`, 22 `url`, 23 `state`,
24 `reason`, 25 `rank`, 26 `pdf`, 27 `hit`, 28 `dead`, 29 `stock_checked`.

`footprints-needed.txt` columns: 1 `letter`, 2 `type`, 3 `company`, 4
`series` (relays only), 5 `mounting`, 6 `package`, 7 `supplier_package`, 8
`size`, 9 `detail`, 10 `parts`, 11 `example`, 12 `datasheets`, 13 `existing`.

The download set is every `candidate` and `reserve` row (3522 at planning
time, 1461 distinct URLs). Reproduce (no DigiKey call is made by the last
four; `urlcheck.py` downloads about 1 GB):

```sh
cd planning/plan-47-data
census/list.sh raw                       # 171 calls; skip: raw/ is in the tree
python3 -I census/urls.py raw > urls-needed.tsv
python3 -I census/urlcheck.py urls-needed.tsv /tmp/p47/pdf > url-check.tsv   # about 40 min cold
python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --report
python3 -I census/footprint-lines.py candidates.tsv ../../schematical/schematical/Parts/footprints.json > footprints-needed.txt
```

`list.sh` and `count.js` need `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET`
(`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`). The
listing was gzipped after the run (`gzip -9 raw/*.jsonl`); `common.py` reads
both forms. The maker rankings and category counts of plan-47-A § 2 are
`count.js` runs; each command is in that table.
