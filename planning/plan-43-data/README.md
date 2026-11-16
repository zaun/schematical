# plan-43-data

Data and scripts of plan-43 (the reference boards' parts that fit a kind the
library already has). Written at planning time, 2026-11-15.

The source is the lookup of the parts the 209 reference boards use and the
library lacks (`routing-example/`, git-ignored): each missing part number was
searched on DigiKey and the datasheet of the most-stocked Active match
downloaded. `routing-example/datasheets-log.tsv` has one line per lookup.

| Path | What |
|---|---|
| `census/classify.py <log>` | writes `candidates.tsv` from the lookup log, `census/fit.tsv`, `census/skip.tsv` and `census/rename.tsv`; prints the counts plan-43-A quotes |
| `census/fit.tsv` | the 47 parts plan-43 adds: kind, part number, DigiKey maker name |
| `census/skip.tsv` | 5 parts that fit a kind but have no datasheet on their maker's site |
| `census/rename.tsv` | 1 lookup that took a sibling of the part the board names |
| `census/lookup.py` | one DigiKey keyword search per `fit.tsv` row; keeps the answer in `raw/` and writes `facts.tsv` |
| `candidates.tsv` | the 300 parts with a datasheet and stock: family, where (`plan-43`, `skipped`, `plan-42`, `later`), part number, maker, stock, status, the PDF's path at planning time, the values the boards give, DigiKey category |
| `facts.tsv` | DigiKey status, stock, package, datasheet URL and parameters of the 47 (2026-11-15) |
| `targets.tsv` | per part: the letter that adds it, the device it joins or starts, package label, pinout, the existing footprints to compare its drawing with, the maker-site datasheet URL, the `datasheets.json` id when one exists, the PDF's path at planning time |
| `raw/*.json` | the DigiKey answers behind `facts.tsv` |

Reproduce:

```sh
cd planning/plan-43-data
python3 -I census/classify.py ../../routing-example/datasheets-log.tsv   # 300 parts
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
python3 -I census/lookup.py                                              # 47 rows; 0 not found; 0 with no stock
```
