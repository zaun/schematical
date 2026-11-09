# plan-29 data (diodes)

Written at planning time, 2026-11-08; plan-29-A re-runs the census and
rewrites the tables.

| File | What |
|---|---|
| `census/list.sh` | step 1: DigiKey listing, 5 categories × 3 makers, raw pages to `/tmp/p29/raw` |
| `census/build.py` | step 2: raw pages → `all.tsv`, one row per part |
| `census/select.py` | step 3: DigiKey-data decisions (`candidate`, `check`, `out`), `leaf`, `flags` |
| `census/fetch.py` | step 4: download every datasheet URL to `/tmp/p29/pdf` |
| `census/textscan.py` | step 5: `pdf`, `hit` (part number printed, token boundaries), `dead` |
| `census/footprint-lines.py` | step 6: `footprints-needed.txt` |
| `census/sibling.py` | step 7: rows with no maker-site PDF that another same-maker PDF prints |
| `candidates.tsv` | the planning-time table: single diodes only (Vishay and Taiwan Semi complete, onsemi 462 of 1113: the 300-result cap); columns as `build.py` + `leaf flags pdf hit dead` |
| `footprints-needed.txt` | one line per (maker, supplier package, mounting) of the single-diode rows with a PDF |

Written later: `census/recheck.sh`, `place.py`, `decide.py`,
`scheme-check.py`, `dead-verdicts.tsv`, `specs.tsv` (plan-29-A);
`fp-load.sh`, `specs-check.py`, `footprints-[B-G].json` (plan-29-B–G);
`gen-parts.py` (plan-29-I).
