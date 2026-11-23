# plan-55-data

Data and scripts of plan-55 (power switches: integrated ideal diodes, load switches, current-limited and
USB power switches, from the top three makers of each family, Active parts only, no generic part).
Written at planning time, 2026-11-22 (HEAD 7c7b4d31).

| Path | What |
|---|---|
| `census/families.js` | the planning census: the three families as DigiKey category + parametric filters (names, not ids), their makers, and per picked maker (a company's DigiKey labels counted together) the part count and the number of distinct `Package / Case` and `Supplier Device Package` values. 14 calls (2 category facet calls, 3 family calls, 9 maker-and-family calls) |
| `families.out` | its output, 2026-11-22: DigiKey in stock, status Active only |

Reproduce:

```sh
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
node scripts/digikey.js --limit 1 "LM66100DCKT" > /dev/null    # one call: refreshes the cached login token
node planning/plan-55-data/census/families.js > /tmp/families.out
```

plan-55-B adds the listing scripts (`list.js`, `list.sh`, `select.py`, …) and `candidates.tsv` here.
