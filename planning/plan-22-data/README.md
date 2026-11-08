# plan-22 data

Research outputs that plan-22 (comparators, adders, ALUs, parity
generators/checkers and Wallace trees; kind `arith`) transcribes into
`arith.json`. Census 2026-11-07; datasheets read 2026-11-07.

| File | Contents |
|---|---|
| `candidates.tsv` | The census plan-22 was sized from: every in-stock TI part (DigiKey `--in-stock --manufacturer 296`, categories 773 Comparators, 703 Specialty Logic, 720 Parity Generators and Checkers, and 743 for the '261/'384) whose function is in scope, 225 rows. |
| `census/` | The scripts that built `candidates.tsv` (`parse.py` DigiKey JSON → TSV, `fn.py` part number → family and function, `scope.py` function → plan, `fp.py` package → footprint, `probe.sh` TI symlink probe). |
| `footprints-needed.txt` | The census's packages with no footprint (planning estimate; `parts.tsv` is the result: no add row needs a new footprint). |
| `parts.tsv` | One row per candidate (225): `name device family package code pins footprint flags status pdf decision reason`. `status` is DigiKey's on the re-check day; `code`/`pins` come from the part's row in the PDF's current packaging table (the '275: from its scanned sheet's package list); `pdf` is relative to `datasheets/`. `decision` is `add` (in stock, Active or Last Time Buy, and in the ordering table) or `skip` with the `reason`. |
| `specs.md` | One section per added device (16): PDFs and pages, pin names, pinouts per package code, function table, the model keys, timing and supply per family, and where the model is not exact. |

How `parts.tsv` was made (scripts under `/tmp/p22/scripts/`, not committed;
the counts in plan-22-A Phase 1 check their output):

1. Stock re-check (2026-11-07 16:30 HST): one `node scripts/digikey.js
   --in-stock --manufacturer 296 --json "<base>"` per DigiKey base number
   (88), then one exact-name search per candidate DigiKey gives no base
   number (25); 113 searches, 499 in-stock TI parts parsed. 222 of the 225
   candidates were found in stock with their census status;
   SN74ALS677ADW, SN74ALS679DW and SN74ALS679N were not (`skip` "not in
   stock on 2026-11-07").
2. Datasheets: `https://www.ti.com/lit/ds/symlink/<name>.pdf` for the names
   the part numbers suggest (SN54 → SN74 sheet, CD54 → CD74, CD4xxx →
   cd4xxxb), the TI literature numbers DigiKey links (sdls008, sdls071,
   sdls095a, sdls136, sdls152, sdls206, sdas038c) and the TI literature
   numbers printed on the Rochester copies DigiKey links (sdls123, schs091b,
   schs136e, schs196c, sdas224b, sdas228a, sdas236a, sdfs069a, scas069b,
   sccs025b); byte-identical files kept once (29 distinct sheets, 26 used).
   The Rochester copies' other literature numbers (sdls168 '381/'382,
   sdls169 '384, sdas235 '882, sdas183a '866, sdas209b AS181, sdas012c
   '677, sdas003c '679, sdas051a '527/'528, scas022a 74AC11521) return 404
   at ti.com, and TI has no product page for those parts: `skip` "no TI
   datasheet". The '275 is the exception the owner chose: its only sheet is
   a TI scan DigiKey hosts (`SNx4(L)S27x.PDF`), kept as
   `arith/snx4ls27x.pdf` (plan-22-A §2).
3. Each candidate's row is looked up in every sheet's packaging table; the
   sheet named after the part's own series wins. Package code + pin count
   map to the existing footprints (`N` 14/16/20 → `PDIP-14/16/20`, `D` →
   `SOIC-14/16`, `DW` 20/24 → `SOIC-20W/24W`, `NS` → `SOP-14/16/20`, `PW` →
   `TSSOP-16/20`, `J` 14/16/20 → `CDIP-14/16/20`, `J` 24 → `CDIP-24-600`,
   `JT` 24 → `CDIP-24`, `W` → `CFP-14/20`, `FK` 20 → `LCCC-20`).
4. Military SMD numbers take the device and family of the SN54/CD54 part
   their sheet's marking column (8601301EA = CD54HC85F3A, 5962-8685701RA =
   CD54HCT688F3A, 8415301RA = SNJ54LS688J) or DigiKey's description
   (JM38510/08201 = SN54S85, /31101 = 54LS85, /32901 = 54LS280, /34701 =
   54F521) names.
5. Families: CD4 → `C`, standard TTL → `TTL`. Flags: `military` (SN54/SNJ54/
   CD54 names, SMD numbers, F3A), `ceramic` (J, JT, W, FK packages). No part
   is `obsolete`: only Active and Last Time Buy parts are added (plan-22-A
   Decisions).
