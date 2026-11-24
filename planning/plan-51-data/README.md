# plan-51-data

Data and scripts of plan-51 (IC sockets for DIP and PLCC chips, and the socket mode of a
placed chip). Written at planning time, 2026-11-21.

| Path | What |
|---|---|
| `census/list.js` | every in-stock Active / Last Time Buy part of DigiKey category 409 (IC Sockets) of one maker, most-stocked first, split by facet below DigiKey's 300-result cap (derived from `planning/plan-45-data/census/list.js`) |
| `census/list.sh` | `list.js` for the twelve makers of category 409 that have a DIP or PLCC socket in stock; a maker whose `.done` exists is skipped, so a run cut by the daily quota goes on the next day |
| `census/common.py` | reading both listing formats, and the scope rule (`scope()`): the part is a candidate |
| `census/tally.py` | the counts plan-51 cites; `--candidates candidates.tsv` writes the candidate rows of the three chosen makers |
| `census/chips.py` | the chips a socket can hold (`FITS`: socket shape → chip footprints of `footprints.json`), parts per footprint, parts with an absent pin; `fits.tsv` is its output |
| `raw/54.jsonl.gz` | Mill-Max, complete (550 parts, 24 DigiKey calls, `raw/54.err`) |
| `raw/<maker>.jsonl.gz`, `.err`, `.done` | plan-51-B (2026-11-24): `census/list.sh raw` listed all twelve makers on the second DigiKey key (Preci-Dip, `1212`: 1007 parts, 32 calls, DIP, PLCC and the rest of its types; the other ten makers 1 or 2 calls each, 14 in all; Mill-Max `54` is the planning-time listing of 2026-11-21) |
| `raw/digikey-json/<maker>.jsonl.gz` | the planning-time listing of seven makers, as `node scripts/digikey.js --in-stock --category 409 --manufacturer <id> --limit 300 --pages 6 --json` printed them. `common.rows` prefers `raw/<maker>.jsonl.gz`, so these are the fallback only |
| `candidates.tsv` | one row per candidate of Mill-Max, Preci-Dip (DIP and PLCC) and Assmann; columns 11–14 (`pdf`, `state`, `reason`, `stock_checked`) are B's: filled in by hand and script, so never re-run `tally.py --candidates` over this file |
| `fits.tsv` | the socket shapes and the chip footprints each holds |

Reproduce (no DigiKey call is made):

```sh
cd planning/plan-51-data
python3 -I census/tally.py --candidates /tmp/p51-candidates.tsv
python3 -I census/chips.py /tmp/p51-fits.tsv
```

`list.sh` needs the DigiKey credentials (`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)2?=' ~/.zshrc)"`
is inside it). `list.js` uses the second key (`DIGIKEY_CLIENT_ID2` / `DIGIKEY_CLIENT_SECRET2`) unless
`DK_OPT2=0`, and sends its requests through `scripts/digikey.js`, whose Safari fallback gets past
Cloudflare's challenge on `api.digikey.com` (needs Safari > Develop > Developer Settings > Allow
JavaScript from Apple Events).
