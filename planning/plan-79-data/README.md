# plan-79 data: battery holders, clips and contacts

Files of plan-79 (`planning/plan-79-*.md`). The planning census is shared with plans 76–82:
`planning/plan-76-data/README.md` § Plan 79, raw pages `planning/plan-76-data/raw/m86.json`
(all of category 86) and `p86.json` (PCB mount only). Census tools: `planning/plan-69-data/census/`
(used as they are).

| File | Written by | What |
|---|---|---|
| `census/list.sh`, `census/check.py`, `census/build.py` | A | the listing: one Safari page per (maker, piece), retried, built into `candidates.tsv` |
| `raw-A/<maker>_<piece>.json` | A | the listing's saved pages (product rows kept) |
| `candidates.tsv` | A | one row per (maker, part number) of the listing, with `scope` and `rank` |
| `stock-check.tsv` | A | DigiKey's own `In Stock` line and status per `in` row |
| `verdicts.tsv` | A, B, D–H | `add`, `pending` or `skip` per `in` row, with a reason |
| `specs.tsv` | A (skeleton), B, D–H | one row per (maker, drawing): cell, cells, function, pins, footprint id, drawing page |
| `fetch.py` | B | the PDF downloader (Safari user agent, retries, SHA-256) |
| `datasheets.tsv` (one per `datasheets/connector-battery/<maker>/`) | B | file, URL, bytes, SHA-256 |
| `drawings.tsv` | B | per PDF: pages, `pdftotext` word count, text / outlined / image, layout figure page |
| `footprints-<maker>.json` | D–H | the maker's new footprints before they are merged |
| `merge-fp.py` | D | merges a `footprints-<maker>.json` into `Parts/footprints.json`, skipping an id already present; `--check` |
| `gen-battery.py` | D | writes the `battery-` blocks of `Parts/connector.json` from `specs.tsv` + `verdicts.tsv`; idempotent; `--check` |

Pick rule (owner, 2026-11-25): the top 4 makers by DigiKey in-stock count (Marketplace excluded,
PCB mount), each capped at its 300 most-stocked parts: Keystone Electronics (361 → 300), MPD (249),
Adam Tech (74), Assmann WSW (53). At most 676 parts.
