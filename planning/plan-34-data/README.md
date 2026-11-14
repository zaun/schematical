# plan-34 data (resistors)

Written at planning time, 2026-11-11; updated by plan-34-A Phase 4 (2026-11-13).

| Path | What |
|---|---|
| `census/top.js` | DigiKey KeywordSearch sorted by stock (QuantityAvailable, descending), statuses Active + Last Time Buy, in stock; slices ±1% / ±5% / Jumper; splits a piece above 300 by Package / Case, Series, Features (never listing Automotive / Current Sense values), Composition, Power, TCR. One JSON line per product. |
| `census/list.sh <raw>` | `top.js` for categories 52, 53, 50 × Viking Tech (2577), YAGEO (13), Vishay Dale (541). |
| `census/recheck.sh <raw>` | re-lists the pieces `select.py` reports as `NEEDS DEEPER LISTING` (plan-34-A § 5). |
| `census/select.py <raw> <out> [--verdicts] [--report]` | scope, at most two per (category, ohms, tolerance, size), quotas chip 1500 / through-hole 400 / arrays 100 (plan-34-A § 3); the cap check. |
| `census/footprint-lines.py <candidates.tsv>` | one line per land pattern the picked rows need. |
| `census/recheck.sh` (cont.) | `MINQ=<n> run …` lists a piece only down to stock n (`top.js` `TOP_MIN_QTY`), from both ends when it holds 301–600 parts. `top.js` resumes from `<stem>.resume` and writes `piece …: failed: HTTP 400` for a piece DigiKey refuses. |
| `census/cut-picked.py <raw> <candidates.tsv> [<stock-check.tsv>]` | the picked rows whose piece (or the base piece it was split from) was cut at 300; `<maker id> TAB <mpn>`. |
| `census/stock-check.sh <raw> <candidates.tsv> <stock-check.tsv>` | plan-34-A § 5 rule 2: one DigiKey call per such row; appends `mpn found stock status date`; skips rows already checked; stops with 20 calls left. |
| `census/common.py` | what the Phase 3 scripts share: eligibility, makers' own hosts, the YAGEO URL rewrite, the PDF cache. |
| `census/fetch.py <candidates.tsv> <pdf-dir>` | downloads every eligible row's datasheet URL into `<pdf-dir>/<md5>.pdf`, `index.tsv`. |
| `census/textscan.py`, `census/sibling.py` | fill `pdf` (the row's maker-site PDF), `hit` (part number printed verbatim), `dead` (an obsolete word near it). |
| `census/place.py` | copies the needed PDFs to `datasheets/resistor/<maker>/`, writes `datasheets/resistor/datasheets.tsv`, sets `pdf` to the path; `--verify`, `--prune`. |
| `census/specs-skeleton.py` | `specs.tsv`: one row per (maker, series, PDF) of the eligible rows; merges into an existing file. |
| `census/scheme.py`, `verdicts.py`, `scheme-check.py` | plan-34-B: the ordering-code field check, the verdicts, one spec row's matches. |
| `census/gen-parts.py` | plan-34-F: `Parts/resistor.json`. |
| `census/fp-load.sh`, `fp-merge.py` | plan-34-C: load ready footprint objects through `footprint_check` on a copy. |
| `stock-check.tsv` | plan-34-A Phase 2's part-by-part stock check. |
| `specs.tsv` | one row per (maker, series, PDF); B fills `mpn_regex`, `fields_checked`, C and D `package`, `pinout`, `footprint`. |
| `raw/*.jsonl.gz`, `raw/*.err` | the listing, complete for the pick (plan-34-A Phase 1, 2026-11-13): 24770 parts, the nine base pieces and six re-listings. |
| `candidates.tsv` | `select.py`'s output with `pdf`, `hit`, `dead` (plan-34-A Phase 3): 24770 rows, 1994 `candidate` (picked), the rest `out` with a reason. |
| `footprints-needed.txt` | the land-pattern lines of the picked rows (83 on 2026-11-13: chip 34, through-hole 30, arrays 19). |

To reproduce the planning pick: gunzip `raw/*.jsonl.gz` into a folder with
the `.err` files, then `python3 -I census/select.py <folder> /tmp/cand.tsv
--report` → `14882 parts listed`, 1994 picked.

DigiKey credentials: `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET` from
`~/.zshrc` (`list.sh` and `recheck.sh` load them). Calls are rationed (about
1000 a day, shared with other sessions).
