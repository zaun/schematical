# plan-21 data

Research outputs that plan-21 (multiplexers / data selectors, the
multiplexers with storage and the '670 register file; kind `mux`)
transcribes into `mux.json`. Census 2026-11-07; datasheets read 2026-11-07.

| File | Contents |
|---|---|
| `candidates.tsv` | The census plan-21 was sized from: every in-stock TI part (DigiKey `--in-stock --manufacturer 296`, categories 743 and 703) whose function is in scope, 634 rows. |
| `census/` | The scripts that built `candidates.tsv` (`parse.py` DigiKey JSON → TSV, `fn.py` part number → family and function, `scope.py` function → plan, `fp.py` package → footprint). |
| `footprints-needed.txt` | The census's packages with no footprint (planning estimate; `parts.tsv` is the result: no add row needs a new footprint). |
| `parts.tsv` | One row per candidate (634): `name device family package code pins footprint flags status pdf decision reason`. `status` is DigiKey's on the re-check day; `code`/`pins` come from the part's row in the PDF's current packaging table; `pdf` is relative to `datasheets/`. `decision` is `add` (in stock, Active or Last Time Buy, and in the ordering table) or `skip` with the `reason`. |
| `specs.md` | One section per added device (20): PDFs and pages, pin names, pinouts per package code, function table, the model keys, timing and supply per family, and where the model is not exact. |

How `parts.tsv` was made (scripts under `/tmp/p21/`, not committed; the
counts in plan-21-A Phase 1 check their output):

1. Stock re-check (2026-11-08 00:01 UTC): one `node scripts/digikey.js
   --in-stock --manufacturer 296 --json "<base>"` per DigiKey base number
   (114), then one exact-name search per candidate DigiKey gives no base
   number (119); all 634 candidates were found in stock with their census
   status.
2. Datasheets: `https://www.ti.com/lit/ds/symlink/<name>.pdf` for the names
   the part numbers suggest (SN54 → SN74 sheet, CD54 → CD74, CD4xxx →
   cd4xxxb, CY74FCT → cy74fct…t, -Q1/-EP variants), then the TI literature
   numbers DigiKey links (sdls098, sdas219b, sdas137a); byte-identical files
   kept once. 102 are used. TI has no product page and no PDF for the '352,
   '353, '850, '851, LS354, LS355, LS604, 74ACT11151/11157/11251,
   ABT162460 and AHC2G157 (DigiKey links Rochester Electronics or
   DigiKey-hosted copies): those parts are `skip` "no TI datasheet".
3. Each candidate's row is looked up in every sheet's packaging table; the
   sheet named after the part's own series wins. Package code + pin count
   map to the existing footprints (`DCT` 8 → `SSOP-8`, `DCU` 8 →
   `VSSOP-8`, `YZP` 8 → `DSBGA-8-YZP`, `DCK` 6 → `SC70-6`, `DBQ` 16 →
   `QSOP-16`, the rest as plan-10 and plan-19).
4. Families: CD4 → `C`, standard TTL → `TTL`, CY74FCT → `FCT`, CLVC → `LVC`,
   CAHCT257 → `AHCT`, CAHCT157 → `AHC` (it is in the SN74AHC157-Q1
   ordering table, marked AT157Q, with AHC levels).
5. Flags: `military` (SN54/SNJ54/CD54 names, F3A), `automotive` (…Q1),
   `enhanced` (…EP), `ceramic` (J, W, FK packages). No part is `obsolete`:
   only Active and Last Time Buy parts are added (plan-21-A Decisions).
