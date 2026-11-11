# plan-34 data (resistors)

Written at planning time, 2026-11-11; plan-34-A Phase 4 updates the map.

| Path | What |
|---|---|
| `census/top.js` | DigiKey KeywordSearch sorted by stock (QuantityAvailable, descending), statuses Active + Last Time Buy, in stock; slices ±1% / ±5% / Jumper; splits a piece above 300 by Package / Case, Series, Features (never listing Automotive / Current Sense values), Composition, Power, TCR. One JSON line per product. |
| `census/list.sh <raw>` | `top.js` for categories 52, 53, 50 × Viking Tech (2577), YAGEO (13), Vishay Dale (541). |
| `census/recheck.sh <raw>` | re-lists the pieces `select.py` reports as `NEEDS DEEPER LISTING` (plan-34-A § 5). |
| `census/select.py <raw> <out> [--verdicts] [--report]` | scope, at most two per (category, ohms, tolerance, size), quotas chip 1500 / through-hole 400 / arrays 100 (plan-34-A § 3); the cap check. |
| `census/footprint-lines.py <candidates.tsv>` | one line per land pattern the picked rows need. |
| `raw/*.jsonl.gz`, `raw/*.err` | the planning listing (14882 parts). The four `-re-` runs were cut by the quota (`rate limited`); `52-dale-re-1` holds 76 pieces of Dale chip ±1%. |
| `candidates.tsv` | `select.py`'s output at planning time: 14882 rows, 1994 `candidate` (picked), the rest `out` with a reason. |
| `footprints-needed.txt` | 86 land-pattern lines of the picked rows (chip 34, through-hole 34, arrays 18). |

To reproduce the planning pick: gunzip `raw/*.jsonl.gz` into a folder with
the `.err` files, then `python3 -I census/select.py <folder> /tmp/cand.tsv
--report` → `14882 parts listed`, 1994 picked.

DigiKey credentials: `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET` from
`~/.zshrc` (`list.sh` and `recheck.sh` load them). Calls are rationed (about
1000 a day, shared with other sessions).
