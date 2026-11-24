# plan-55-data

Data and scripts of plan-55 (power switches: integrated ideal diodes, load switches, current-limited and
USB power switches, from the top three makers of each family, Active parts only, no generic part).
Planning census written 2026-11-22 (HEAD 7c7b4d31); plan-55-B's listing, candidates and datasheet scripts 2026-11-23.

DigiKey's API answers scripts with a Cloudflare challenge since 2026-11-23; the scripts below reach it through
`scripts/digikey.js` (its login and request path, with the Safari fallback: Safari > Develop > Developer
Settings > "Allow JavaScript from Apple Events"). `--opt2` uses the second API key
(`DIGIKEY_CLIENT_ID2` / `DIGIKEY_CLIENT_SECRET2`, its own daily quota).

| Path | What |
|---|---|
| `census/families.js` | the planning census: the three families as DigiKey category + parametric filters (names, not ids), their makers, and per picked maker (a company's DigiKey labels counted together) the part count and the number of distinct `Package / Case` and `Supplier Device Package` values. 14 calls (2 category facet calls, 3 family calls, 9 maker-and-family calls) |
| `families.out` | its output, 2026-11-22: DigiKey in stock, status Active only |
| `census/common.js` | what `families.js` and `list.js` share: `FAMILIES` (the filters), `PIECES`, the request path |
| `census/list.js`, `census/list.sh` | B Phase 1: every in-stock Active part of the 14 (family, maker label) pieces, most-stocked first, to `raw/<piece>.jsonl` (`list.sh <dir> [--opt2]`, a piece with a `.done` is skipped) |
| `raw/` | the listing, 2026-11-23: `<piece>.jsonl.gz`, its `.err` (the pieces read, the calls) and `facets-<category>.json` (the filters' ids) |
| `raw-recheck/` | the second listing of Phase 2 (the stock re-check), same layout |
| `census/common.py` | shared code: reading the listing, the company of a maker label, the maker-URL rules (`alternatives`) |
| `census/select.py` | `candidates.tsv` from the listing (`--carry`, `--urls`, `--stock`, `--report`) |
| `candidates.tsv` | one row per listed part: the columns of plan-55-B § 3.2 |
| `census/urls.py`, `urls-needed.tsv` | the distinct maker-URL chains of the candidate rows (a TI part has up to three names to try) |
| `census/urlcheck.py`, `url-check.tsv` | which URLs answer a PDF (`url`, `host`, `http`, `ok`) |
| `census/stock.py`, `census/stock-check.sh`, `stock-check.tsv` | the re-check: `stock-check.sh <recheck dir> candidates.tsv stock-check.tsv --opt2` lists again and `stock.py` compares |
| `census/dl-urls.py`, `census/fetch.js` | B Phase 3: the URLs to download and the downloader (`fetch.js <urls.tsv> <pdf dir>`: 4 at a time, a `%PDF-` check and `pdfinfo`) |
| `census/textscan.py` | `pdftotext` each PDF: `pdf`, `hit` per candidate row; `--sibling`: a same-company PDF that prints the part number; rows with none are `out`, `no datasheet` |
| `census/place.py` | copies the PDFs to `datasheets/powerswitch/<company>/` and writes `datasheets/powerswitch/datasheets.tsv` (`--verify`) |
| `census/specs-skeleton.py`, `specs.tsv` | one row per (company, series, PDF, package, supplier package) of the candidate rows, the columns of plan-55-C § 3.1, the pinout columns empty |

Reproduce (the first listing; the second is the same on a later call with `raw-recheck`):

```sh
zsh planning/plan-55-data/census/list.sh planning/plan-55-data/raw --opt2
gzip -9 planning/plan-55-data/raw/*.jsonl
python3 -I planning/plan-55-data/census/select.py planning/plan-55-data/raw planning/plan-55-data/candidates.tsv --report
```

The census itself: `eval "$(grep -E '^export DIGIKEY_' ~/.zshrc)"; node planning/plan-55-data/census/families.js --opt2`.
