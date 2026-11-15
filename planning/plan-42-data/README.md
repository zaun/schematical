# plan-42-data

Data and scripts of plan-42 (voltage regulators — fixed and adjustable
positive linear regulators, including dual / multi-output ones, and
switching regulators with an internal switch — from Texas Instruments,
Analog Devices and Microchip Technology). Written at planning time,
2026-11-15; plan-42-A re-runs and extends it.

| Path | What |
|---|---|
| `census/top.js` | DigiKey listing of one (category, maker id) piece: in stock, Active or Last Time Buy, Output Configuration = Positive, sorted by stock, split once by Output Type; pieces of 301–600 parts listed from both ends. Optional arguments: split facet ids (`41,16`), keywords (`named.sh`), a minimum stock (`recheck.sh`) |
| `census/list.sh <raw>` | `top.js` for the 14 (category, maker id) pieces; skips a piece whose `.done` exists |
| `census/named.sh <raw>` | `top.js` with keywords for the 13 owner-named bases (`planning/todo.md`: 78xx, LM317, LM1117 / TLV1117, LM2596 / LM2576, MCP1700 / MCP1702), pieces `<cat>-<maker>-named-<base>` |
| `census/recheck.sh <raw>` | the deeper re-listings (`<cat>-<maker>-re`), each with `MinimumQuantityAvailable` = the pick's floor of the time |
| `census/select.py <raw> <out.tsv> [--stock <tsv>] [--report]` | the pick: writes `candidates.tsv`; `--report` prints every count the plan quotes and a verdict per cut piece |
| `census/download-set.py <candidates.tsv> [--report]` | the download / stock-check set: per (family, maker) the picked rows, then the next rows by stock, up to 1.5 × the share |
| `census/footprint-lines.py <candidates.tsv>` | `footprints-needed.txt` |
| `raw/*.jsonl.gz`, `raw/*.err` | the planning-time listing (one file pair per piece) and `top.js`'s summary lines (`piece <label>: <listed> of <count> (cut at 300: <stock of the 300th>)`) |
| `candidates.tsv` | every listed part, columns of `select.py` `COLUMNS`; `reason` `candidate` = picked; `cut` = 1 when a piece cut at 300 listed it; `stock_checked`, `pdf`, `hit`, `dead` are filled by plan-42-A and B |
| `download-set.tsv` | `download-set.py`'s output at planning time (900 rows) |
| `footprints-needed.txt` | one line per (maker, DigiKey Package / Case, Supplier Device Package) of the picked parts, with the letter that draws it (D leaded, E leadless TI and Microchip, F leadless Analog Devices; plan-42-B regenerates it from the final pick) and the existing footprints with the same outline |

Restore and reproduce the planning pick:

```sh
mkdir -p /tmp/p42/raw && for f in planning/plan-42-data/raw/*.jsonl.gz; do
  gunzip -c $f > /tmp/p42/raw/$(basename ${f%.gz}); done
cp planning/plan-42-data/raw/*.err /tmp/p42/raw/
python3 -I planning/plan-42-data/census/select.py /tmp/p42/raw /tmp/p42/cand.tsv --report
cmp /tmp/p42/cand.tsv planning/plan-42-data/candidates.tsv && echo reproducible
```

DigiKey categories: 699 Voltage Regulators - Linear, Low Drop Out (LDO)
Regulators; 739 Voltage Regulators - DC DC Switching Regulators. Maker ids:
296 Texas Instruments, 14 National Semiconductor and 600062 Burr Brown (TI);
505 Analog Devices Inc., 175 Analog Devices Inc./Maxim Integrated (ADI); 150
Microchip Technology, 576 Micrel Inc. (Microchip). Parameter ids: 255 Output
Configuration (389019 Positive), 41 Output Type, 16 Package / Case.
