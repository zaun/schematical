# plan-40-data

Data and scripts of plan-40 (BJTs and MOSFETs from the top three makers of
each family). Written at planning time, 2026-11-14; plan-40-A re-runs and
extends it.

| Path | What |
|---|---|
| `census/top.js` | DigiKey listing of one (category, maker id) piece: in stock, Active or Last Time Buy, sorted by stock, split once by the type facet, pieces of 301–600 parts listed from both ends; the optional 4th argument lists split facet ids (`74,16`) |
| `census/list.sh <raw>` | `top.js` for the 30 (category, maker id) pieces; skips a piece whose `.done` exists |
| `census/recheck.sh <raw>` | the three deeper re-listings plan-40-A Phase 1 runs (`<cat>-<maker>-re`) |
| `census/select.py <raw> <out.tsv> [--report]` | the pick: writes `candidates.tsv`; `--report` prints every count the plan quotes and a verdict per cut piece |
| `census/footprint-lines.py <candidates.tsv>` | `footprints-needed.txt` |
| `raw/*.jsonl.gz`, `raw/*.err` | the planning-time listing (one file pair per piece) and `top.js`'s summary lines (`piece <label>: <listed> of <count> (cut at 300: <stock of the 300th>)`) |
| `candidates.tsv` | every listed part, columns of `select.py` `COLUMNS`; `reason` `candidate` = picked; `cut` = 1 when a piece cut at 300 listed it; `stock_checked`, `pdf`, `hit`, `dead` are filled by plan-40-A and B; the last two columns, `vgs_v` (Vgs max, V) and `hfe_min`, are DigiKey facts for G |
| `footprints-needed.txt` | one line per (maker, DigiKey Package / Case) of the picked parts, with its letter (C through-hole, D small-signal surface mount, E power / leadless surface mount) and the existing footprints with the same outline |

Restore and reproduce the planning pick:

```sh
mkdir -p /tmp/p40/raw && for f in planning/plan-40-data/raw/*.jsonl.gz; do
  gunzip -c $f > /tmp/p40/raw/$(basename ${f%.gz}); done
cp planning/plan-40-data/raw/*.err /tmp/p40/raw/
python3 -I planning/plan-40-data/census/select.py /tmp/p40/raw /tmp/p40/cand.tsv --report
cmp /tmp/p40/cand.tsv planning/plan-40-data/candidates.tsv && echo reproducible
```

DigiKey categories: 276 Single Bipolar Transistors, 277 Bipolar Transistor
Arrays, 292 Single Pre-Biased Bipolar Transistors, 293 Bipolar Transistor
Arrays Pre-Biased, 278 Single FETs / MOSFETs, 289 FET / MOSFET Arrays. Maker
ids: 488 onsemi and 600039 Fairchild Semiconductor (onsemi), 1727 Nexperia,
31 Diodes Incorporated, 448 Infineon Technologies, 21 Infineon Technologies
Americas and 600047 International Rectifier (Infineon), 742 Vishay Siliconix
and 600092 Siliconix (Vishay Siliconix).
