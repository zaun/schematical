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
| `raw/1212.jsonl.gz` | Preci-Dip, **partial**: the BGA and DIP types were read before the daily quota ran out (`raw/1212.err`, HTTP 429); the PLCC and later types are not listed. plan-51-B deletes this file and runs `census/list.sh raw` again |
| `raw/digikey-json/<maker>.jsonl.gz` | the other seven makers, complete, as `node scripts/digikey.js --in-stock --category 409 --manufacturer <id> --limit 300 --pages 6 --json` printed them (planning time only: these makers are not in scope) |
| `candidates.tsv` | one row per candidate of Mill-Max, Preci-Dip (DIP only, until B) and Assmann |
| `fits.tsv` | the socket shapes and the chip footprints each holds |

Reproduce (no DigiKey call is made):

```sh
cd planning/plan-51-data
python3 -I census/tally.py --candidates /tmp/p51-candidates.tsv
python3 -I census/chips.py /tmp/p51-fits.tsv
```

`list.sh` needs `DIGIKEY_CLIENT_ID` / `DIGIKEY_CLIENT_SECRET`
(`eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`).
