# plan-44-data

Data and scripts of plan-44 (crystals and ceramic resonators from the top
three makers at DigiKey: Abracon, ECS, Suntsu). Written at planning time,
2026-11-16.

| Path | What |
|---|---|
| `census/count.js` | one DigiKey call: in-stock Active / Last Time Buy count of a category (171 Crystals, 174 Resonators) and maker, split by the Type, Operating Mode, Features, Ratings and Mounting Type facets; with no maker, the 15 biggest makers |
| `census/list.js`, `census/list.sh` | the planning listing: every in-stock Active / Last Time Buy part of the five (category, maker) pieces, series by series, most-stocked first, up to DigiKey's 300-result cap per series (`raw/`) |
| `raw/*.jsonl.gz`, `raw/*.err` | the listing: one JSON line per part; the per-series summary (`series <name>: <listed> of <count> (cut at 300: <stock of the 300th>)`) and the `list.js:` totals |
| `census/common.py` | shared code: reading the listing, the scope rules, de-duplication, the maker-site datasheet URL rules |
| `census/urls.py`, `urls-needed.tsv` | the distinct maker-site datasheet URLs of the in-scope rows (`via` = `direct` or `archive`) |
| `census/urlcheck.js`, `url-check.tsv` | which of them answer with a PDF (`direct`: a fetch; `archive` (Suntsu, 403 to every automated fetch): the Internet Archive has a snapshot) |
| `census/select.py` | `candidates.tsv` from the listing (and `--urls`, later `--stock`): scope, duplicates, datasheet, quota |
| `candidates.tsv` | one row per listed part: facts, the maker-site URL, `state` (`candidate` 600, `reserve` 144, `out` the rest), `reason`, `rank`; the last four columns (`pdf`, `hit`, `dead`, `stock_checked`) are filled by plan-44-A |
| `census/footprint-lines.py`, `footprints-needed.txt` | one line per (maker, DigiKey Package / Case, Supplier Device Package, Size) of the download set (`candidate` + `reserve`), the letter that draws it (C, D), the existing footprints with the same body size code and the footprint label (`XTAL-3225-4`: an id without its maker suffix) |

The download set is every `candidate` and `reserve` row (744 at planning
time). Reproduce (no DigiKey call is made by the last three):

```sh
cd planning/plan-44-data
census/list.sh raw                       # 283 calls; skip: raw/ is in the tree
python3 -I census/urls.py raw > urls-needed.tsv
node census/urlcheck.js urls-needed.tsv > url-check.tsv     # ~4 min (the archive is slow)
python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --report
python3 -I census/footprint-lines.py candidates.tsv > footprints-needed.txt
```

`list.sh` needs `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET`
(`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`). The
listing was gzipped after the run (`gzip -9 raw/*.jsonl`); `common.py` reads
both forms.
