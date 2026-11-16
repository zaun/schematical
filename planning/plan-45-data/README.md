# plan-45-data

Data and scripts of plan-45 (screw terminals, spring / push-in terminals, pluggable
terminal headers, and JST XH / PH / EH / ZH / SH / GH / VH board headers). Written at
planning time, 2026-11-16.

| Path | What |
|---|---|
| `census/count.js` | one DigiKey call: the in-stock Active / Last Time Buy count of a category (371 Terminal Blocks - Wire to Board, 370 Terminal Blocks - Headers, Plugs and Sockets, 368, 369, 314 Headers, Male Pins), a maker, a scope (`t` terminal filter, `jst` the seven JST series); the 12 biggest makers; the Number of Levels split; `SERIES=<n>` the biggest series; `SERIESF=<id>` one series |
| `census/sample.js` | one call: the datasheet-link hosts of a maker's 50 most-stocked parts in a category |
| `census/list.js`, `census/list.sh` | the planning listing: every in-stock Active / Last Time Buy part of the seven (category, maker, scope) pieces, split by facets below DigiKey's 300-result cap, most-stocked first (`raw/`) |
| `raw/*.jsonl.gz`, `raw/*.err` | the listing: one JSON line per part; per-piece summary lines `piece <split>: <listed> of <count>` and the `list.js:` totals |
| `census/common.py` | shared code: reading the listing, the scope rules, de-duplication, the variant (footprint family) key, the maker-site URL rules |
| `census/select.py` | `candidates.tsv` from the listing (and `--urls`, later `--stock`): scope, duplicates, datasheet, variant choice |
| `census/urls.py`, `urls-needed.tsv` | the distinct (maker URL, download source) pairs of the candidate rows |
| `census/urlcheck.py`, `url-check.tsv` | which of them download as a PDF (`Mozilla/5.0`; jst-mfg.com / jst.com need a Safari User-Agent) |
| `candidates.tsv` | one row per listed part: facts, URL, `variant`, `state` (`candidate` 521, `out` the rest), `reason`; the last four columns (`pdf`, `hit`, `dead`, `stock_checked`) are filled by plan-45-A |
| `census/footprint-lines.py`, `footprints-needed.txt` | one line per variant of the candidates: the letter that draws it (C fixed terminal blocks, D pluggable headers, E JST), maker, series, pitch, orientation, mounting, the position counts, parts, the existing footprints with the same pitch (none), the id stem |

Reproduce (no DigiKey call is made by the last four):

```sh
cd planning/plan-45-data
census/list.sh raw                                   # 145 calls; skip: raw/ is in the tree
python3 -I census/select.py raw /tmp/p45-c0.tsv      # the first pass, no URL judgement yet
python3 -I census/urls.py /tmp/p45-c0.tsv > urls-needed.tsv
python3 -I census/urlcheck.py urls-needed.tsv > url-check.tsv     # 1 min
python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --report
python3 -I census/footprint-lines.py candidates.tsv > footprints-needed.txt
```

The listing was gzipped after the run (`gzip -9 raw/*.jsonl`); `common.py` reads both forms.

`list.sh` needs `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET`
(`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`).
