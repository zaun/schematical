# plan-19 data

Research outputs that plan-19 (decoders, encoders, BCD and 7-segment
decoders; kind `decoder`) transcribes into `decoder.json`. Census
2026-11-07; stock re-checked and datasheets read 2026-11-07.

| File | Contents |
|---|---|
| `candidates.tsv` | The census plan-19 was sized from: every in-stock TI part (DigiKey `--in-stock --manufacturer 296`, categories 743 and 703) whose function is in scope, 505 rows. |
| `census/` | The scripts that built `candidates.tsv` (`parse.py` DigiKey JSON → TSV, `fn.py` part number → family and function, `scope.py` function → plan, `fp.py` package → footprint). |
| `footprints-needed.txt` | The census's packages with no footprint (planning estimate; `parts.tsv` is the result). |
| `parts.tsv` | One row per candidate (505): `name device family package code pins footprint flags status pdf decision reason`. `status` is DigiKey's on the re-check day; `code`/`pins` come from the part's row in the PDF's current packaging table; `pdf` is relative to `datasheets/`. `decision` is `add` (in stock, Active or Last Time Buy, and in the ordering table) or `skip` with the `reason`. |
| `specs.md` | One section per added device (32): PDFs and pages, pin names, pinouts per package code, function table, the model keys, timing and supply per family, and where the model is not exact. |

How `parts.tsv` was made (scripts under `/tmp/p19/`, not committed; the
counts in plan-19-A Phase 1 check their output):

1. Stock re-check: one `node scripts/digikey.js --in-stock --manufacturer 296
   --json "<base>"` per DigiKey base number (82), then one exact-name search
   per candidate DigiKey gives no base number (30); 503 of 505 candidates were
   found in stock with an unchanged status (the two '278 parts were not; they
   are skipped anyway).
2. Datasheets: `https://www.ti.com/lit/ds/symlink/<name>.pdf` for the names
   the part numbers suggest (SN54 → SN74 sheet, CD54 → CD74, CD4xxx →
   cd4xxxb), retried once; byte-identical files kept once. 89 are used.
3. Each candidate's row is looked up in every sheet's packaging table; the
   sheet named after the part's own series wins. Package code + pin count
   map to the footprint (`DCT` 8 → `SSOP-8`, `DBQ` 16 → `QSOP-16`, `DYY` 16
   → `SOT23-THIN-16`, `RSV` 16 → `UQFN-16-RSV`, the rest as plan-10).
4. Flags: `military` (SN54/SNJ54/CD54 names, F3A), `automotive` (…Q1),
   `enhanced` (…EP), `ceramic` (J, W, FK packages). No part is `obsolete`:
   only Active and Last Time Buy parts are added (plan-19-A Decisions).
