# plan-54-data

Data and scripts of plan-54 (card slots: memory and SIM card sockets, DIMM / SO-DIMM
memory-module sockets and card-edge sockets, from the top three makers of each family,
Active parts only). Written at planning time, 2026-11-22 (HEAD 7b449718).

| Path | What |
|---|---|
| `census/makers.js` | one call per category: every maker of a DigiKey category (in stock, status Active) with its part count (`414` PC Card Sockets, `413` Inline Module Sockets, `303` Edgeboard Connectors) |
| `census/stocked.js` | one call per `<category>:<maker id>`: how many of its first 50 listed parts have a `QuantityAvailable` above 0 |
| `census/list.js`, `census/list.sh` | the planning listing: every in-stock Active part of each (category, maker) piece, split by facet below DigiKey's 300-result cap, most-stocked first (`raw/`) |
| `raw/*.jsonl.gz`, `raw/*.err` | the listing: one JSON line per part; per-piece summary lines `piece <split>: <listed> of <count>` and the `list.js:` totals |
| `census/common.py` | shared code: reading the listing, the family and maker tables, the datasheet source rules (`src_for`) |
| `census/select.py` | `candidates.tsv` from the listing (and `--urls`, later `--stock`): maker, stock, facts, packing duplicates, datasheet |
| `census/urls.py`, `urls-needed.tsv` | the distinct (maker URL, download source) pairs of the candidate rows |
| `census/urlcheck.py`, `url-check.tsv` | which download sources answer a PDF (`Mozilla/5.0`, one request a second per host) |
| `census/report.py` | prints the numbers plan-54 quotes (stocked parts per maker, resolvable links, candidates, variants, footprint keys) |
| `candidates.tsv` | one row per listed part: facts, URLs, `variant`, `state` (`candidate` 1270, `out` the rest), `reason`; the last four columns (`pdf`, `hit`, `dead`, `stock_checked`) are filled by plan-54-A |

Reproduce (no DigiKey call is made after the first step):

```sh
cd planning/plan-54-data
census/list.sh raw                                   # 164 calls; skip: raw/ is in the tree
python3 -I census/select.py raw /tmp/p54-c0.tsv      # the first pass, no URL judgement yet
python3 -I census/urls.py /tmp/p54-c0.tsv > urls-needed.tsv
python3 -I census/urlcheck.py urls-needed.tsv > url-check.tsv     # 15 min; urls-needed.tsv has 517 rows, 509 distinct sources (498 serve a candidate)
python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --report
python3 -I census/report.py raw candidates.tsv
```

`list.sh` needs `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET`
(`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`). The listing was gzipped
after the run (`gzip -9 raw/*.jsonl`); `common.py` reads both forms.
