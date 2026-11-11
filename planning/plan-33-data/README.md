# plan-33 data (capacitors)

Planning-time census of 2026-11-11 (plan-33-A § 2). plan-33-A Phase 1 lists
the rest of the buckets and replaces the planning-time files.

DigiKey calls need `DIGIKEY_CLIENT_ID` / `_SECRET`, exported in `~/.zshrc`,
which tool shells do not load. Run the scripts from a small script file:

```zsh
#!/bin/zsh
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
node /Users/justinzaun/Development/logic/planning/plan-33-data/census/bucket-parts.mjs "$@"
```

| File | What |
|---|---|
| `census/queries.mjs` | the 73 bucket queries: label `<type>\|<package>\|<dielectric or voltage>`, DigiKey category, parametric filters (plan-33-A § 3 rule 1) |
| `census/buckets.mjs` | one call per query: its Capacitance facet (and every other facet) → `buckets.jsonl` / `facets.jsonl`; `[<label prefix>]` limits the queries; re-runs append only missing labels |
| `census/buckets.py` | E6 buckets per query (one capacitance may have several DigiKey value ids, joined by `;`) → `buckets.tsv`; prints the counts |
| `census/bucket-parts.mjs` | one call per bucket: its 50 most-stocked parts with their parameters → raw JSON lines; skips listed buckets, re-logs in on HTTP 401, stops on 429; `--only N` |
| `census/select.py` | pick order (`rank`, plan-33-A § 3 rule 2), `out` (military, wet tantalum), `flags` → `candidates.tsv` |
| `census/footprint-proxy.py` | planning-time `footprints-needed.txt` (measured lines for the listed buckets, upper bounds for the rest) |
| `buckets.jsonl` | 73 lines: label, `ProductsCount`, Capacitance facet (2026-11-11) |
| `facets.jsonl` | 59 lines (the non-ceramic queries): the same plus every facet, for the footprint bounds |
| `buckets.tsv` | 1048 buckets: `label`, `capacitance` (value ids) |
| `raw/ceramic-2026-11-11.jsonl.gz` | 273 ceramic buckets listed at planning time (Phase 1 merges it into `raw/parts-<date>.jsonl.gz`) |
| `candidates.tsv` | one row per (bucket, listed part): `bucket type mount key capacitance farads rank mpn maker status stock series voltage dielectric tolerance package mounting size height spacing ratings size_code datasheet_url flags decision reason pdf hit dead` (9653 rows at planning time, all `candidate`) |
| `footprints-needed.txt` | footprint groups: section 1 measured, section 2 upper bounds (plan-33-C § 2, D § 2) |

Written later: `census/fetch.py`, `textscan.py`, `place.py`,
`specs-skeleton.py`, `specs.tsv` (A); `scheme-check.py`, `dead-scan.py`,
`decide.py`, `bucket-more.mjs`, `footprint-lines.py`, `buckets-empty.tsv`,
`fpgroups.tsv` (B); `fp-load.sh`, `footprints-C.json`,
`footprints-generic.json` (C); `footprints-D.json` (D); `gen-parts.py` (F).
