# plan-35 data: 20x4 / 40x2 / 40x4 character LCD census

Made 2026-11-11 while writing plan-35 (sub-plan A uses it first). Every
number in plan-35 comes from these files and the commands below.

## Files

| File | What |
|---|---|
| `raw/cat99-instock.jsonl.gz` | DigiKey category 99 ("LCD, OLED Character and Numeric"), in stock, every maker: 810 parts, 39 pages, 39 API calls (`raw/cat99-instock.err`) |
| `raw/cat99-m{2632,757,3086}-all.jsonl.gz` | the same category for Focus LCDs (2632), Newhaven (757) and Midas (3086), any stock and status: 251 / 250 / 64 parts, 6 / 5 / 2 calls |
| `census/list.sh` | makes the four `raw/` listings (`list.sh <outdir>`; 52 calls) |
| `census/census.py` | `census.py raw/cat99-instock.jsonl.gz > all-target.tsv`: the 189 in-stock 20x4, 4x20, 40x2 and 40x4 parts (DigiKey "Display Format"), plus a per-maker count on stderr |
| `census/allstat.py` | status x stock of those formats in the per-maker listings |
| `census/dl.sh` | downloads each candidate's datasheet (`dl.sh <maker\tpart\turl tsv> <dir>`) |
| `census/font.py` | the font each PDF chooses (Midas "Font Set", Focus checkbox glyphs U+F0FE checked / U+F06F unchecked, Focus ROM code "ST7066U-0A") |
| `census/pages.sh` | `pages.sh <pdf> <regex>`: pages whose text matches |
| `census/outline.py`, `census/pins.py` | module outline and pin table from each PDF's text |
| `census/verdicts.tsv` | add / skip and why, for all 126 top-3 rows |
| `census/footprint-groups.tsv` | footprint id -> the parts that share it |
| `census/drawing-pages.tsv` | part -> PDF -> mechanical drawing page |
| `census/midas.sh`, `census/midas-urls.tsv` | each Midas part's own-site PDF (midasdisplays.com product page); 13 found, none for MC42008A6W-SPR |
| `census/candidates.py` | builds `candidates.tsv` from the above |
| `census/footprints.py` | builds `footprints-needed.txt` from `candidates.tsv` and `footprint-groups.tsv` |
| `census/recheck.sh` | plan-35-A: part-by-part stock and status re-check (67 calls) |
| `candidates.tsv` | 126 rows (67 `add`, 59 `skip`); the facts used by plan-35-B..F |
| `footprints-needed.txt` | the 34 module footprints (33 at planning + the NHD-0420E2Z-NSW-BBW split, plan-35-C Corrections), their parts, drawing page and header |
| `pdf-sha256.tsv` | sha256 / bytes / pages of the 67 module PDFs and `ST7066U.pdf` as downloaded 2026-11-11 |

## Counts (2026-11-11)

- In-stock 20x4/40x2/40x4 parts, every maker: 189 (`census.py` stderr).
  The category listing is 810 parts, over DigiKey's 300-result cap, so
  digikey.js split it into pieces; no piece was cut (no warning in
  `raw/cat99-instock.err`). The per-maker listings are under 300 each.
- Top three makers by those parts: Focus LCDs 75, Newhaven 34, Midas 17
  (next: Display Visions 11, Winstar 11). 126 rows.
- Per-maker, any stock (`allstat.py raw/cat99-m*-all.jsonl.gz`): Focus 75
  (all Active, in stock); Newhaven 52 (34 Active in stock, 2 Active no
  stock, 16 Obsolete no stock); Midas 20 (17 in stock, 3 Active no stock).
  The 21 parts with no stock were never candidates (stock rule).
- Owner scope (parallel LCD only) drops 8 OLEDs and 12 serial/IST3602 parts.
- Datasheet reading drops 32 Focus parts with no stated font, 1 Focus
  overview-only PDF, 4 English/European-font parts, 1 FPC-tail module and
  1 Midas part with no datasheet on midasdisplays.com (MC42008A6W-SPR).
- 67 parts are added: 20x4 41, 40x2 14, 40x4 12; Focus 32, Newhaven 22,
  Midas 13.
