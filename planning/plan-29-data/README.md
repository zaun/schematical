# plan-29 data (diodes)

Census re-run by plan-29-A on 2026-11-09 (`/follow-plan 29`); the planning-time
tables of 2026-11-08 are replaced.

| File | What |
|---|---|
| `census/list.sh` | step 1: DigiKey listing, 5 categories × 3 makers, raw pages to `/tmp/p29/raw` |
| `census/recheck.sh` | step 1b: keyword re-listing of a piece the 300-result cap (or a DigiKey error) cut short (plan-29-A § 5) |
| `census/build.py` | step 2: raw pages → `all.tsv`, one row per part |
| `census/select.py` | step 3: DigiKey-data decisions (`candidate`, `check`, `out`), `leaf`, `flags` |
| `census/fetch.py` | step 4: download every datasheet URL to `/tmp/p29/pdf` |
| `census/textscan.py` | step 5: `pdf`, `hit` (part number printed, token boundaries), `dead` |
| `census/footprint-lines.py` | step 6: `footprints-needed.txt` |
| `census/sibling.py` | step 7: count of rows with no maker-site PDF that another same-maker PDF prints |
| `census/place.py` | step 8: copy the maker-site PDFs to `datasheets/diode/<maker>/`, `datasheets.tsv`; `--verify`, `--prune` |
| `census/dead-scan.py` | step 9a: datasheet-status verdicts (onsemi DISCONTINUED sections, revision history) → `census/dead-verdicts.tsv` |
| `census/decide.py` | step 9: rules 1-5 → `add` / `pending` / `skip` / `out`; `--skipped`, `--verify-skipped` (`datasheets/skipped-parts.md`) |
| `census/specs-skeleton.py` | step 10: `specs.tsv`, one row per (maker, category, datasheet, supplier package) of the add / pending rows |
| `census/scheme-check.py` | B-G: settle `pending` rows with `specs.tsv`'s `mpn_regex` |
| `census/specs-check.py` | B-G: every `add` row has a complete spec row and a matching footprint |
| `census/fp-load.sh`, `census/fp-merge.py` | B-G: load ready footprint objects through `footprint_check` |
| `candidates.tsv` | every listed part (13249): build.py columns + `leaf flags pdf hit dead`; `decision` / `reason` final for A |
| `footprints-needed.txt` | one line per (maker, supplier package, mounting) of the rows with a PDF (313) |
| `specs.tsv` | `maker category datasheet supplier_package mpn_regex package pinout pins footprint vr_io_checked note` (3198 rows; B-G fill the drawing columns and the regexes) |

Written later: `footprints-[B-G].json` (plan-29-B–G); `gen-parts.py`
(plan-29-I).
