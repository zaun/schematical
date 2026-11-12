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

Written by plan-33-A (2026-11-11, at execution):

| File | What |
|---|---|
| `raw/parts-2026-11-11.jsonl.gz` | every bucket of `buckets.tsv` listed (Phase 1; replaces `raw/ceramic-2026-11-11.jsonl.gz`) |
| `census/fetch.py` | downloads each distinct `datasheet_url` of the rows with `rank` ≤ N (default 10) to `/tmp/p33/pdf/<md5>.pdf` (bare curl, then a `Mozilla/5.0` agent, then Safari headers); `--verify` → `N untried` |
| `census/textscan.py` | fills `pdf`, `hit`, `dead` in place for the scanned ranks |
| `census/place.py` | copies the maker-site PDFs to `datasheets/capacitor/<kemet\|calchip\|avx>/`, writes `datasheets/capacitor/datasheets.tsv`; `--verify`, `--prune` |
| `census/specs-skeleton.py` | `specs.tsv`: one row per placed PDF (`maker datasheet url`), scheme columns empty |
| `specs.tsv` | the part-number schemes (B fills them) |

Written by plan-33-B: `census/specs-fill.py` (the schemes, read from each
PDF's guide), `census/scheme-check.py` (column `check`),
`census/dead-scan.py` (`census/dead-pdfs.txt`), `census/dead-verdicts.tsv`,
`census/decide.py` (the walk; `--skipped`, `--verify-skipped`, `--empty`),
`buckets-empty.tsv`, `census/footprint-lines.py`, `fpgroups.tsv`,
`footprints-needed.txt`. plan-33-C: `census/fp-load.sh` and
`census/fp-merge.py` (from plan-29), `footprints-C.json`,
`footprints-generic.json`. plan-33-D: `footprints-D.json`. plan-33-F:
`census/gen-parts.py`.

The working files (PDF downloads and their text) live in `/tmp/p33/pdf/`
and are not kept; `fetch.py` and `textscan.py` rebuild them.
