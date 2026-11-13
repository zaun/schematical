# plan-36 data (microcontrollers)

Written at planning time, 2026-11-12, at 7ca1070. Each letter adds its own
files and updates this map.

| File | What | Written by |
|---|---|---|
| `raw/<label>.jsonl.gz`, `raw/<label>.err` | the planning listing: one JSON line per DigiKey product (in stock, Active / Last Time Buy) and the piece summary per run. Labels: the 35 AVR series of `census/list-avr.sh`, `espressif`, `rp2040` | planning |
| `candidates.tsv` | one row per listed part (1545; 1521 in scope). Columns: `mpn maker family device scope reason atdf_device atdf_pinout dk_package dk_supplier_package stock status grade dk_vcc dk_speed dk_flash dk_ram dk_eeprom datasheet_url pdf hit dead package_code verdict pinout_src footprint`. A fills `pdf`…`verdict`, B fills `pinout_src`, C–E fill `footprint` | planning; A, B, C–E |
| `datasheet-heads.tsv` | `curl -sIL` of each distinct datasheet URL: status, type, length, URL (193; 189 PDFs, 939.48 MB) | planning |
| `packs.tsv` | the 12 Microchip device packs (URL, SHA-256, bytes) whose ATDF files give the AVR pinouts. Apache-2.0; not committed, re-downloaded by A Phase 1 | planning |
| `footprints-needed.txt` | planning proxy: parts per (maker, DigiKey package, supplier package); 67 lines (22 dual-row, 31 quad/grid, 1 RP2040, 13 modules). C–E work from `package-codes.tsv` instead | planning |
| `census/list.js` | DigiKey KeywordSearch lister: one (category, maker[, series]) piece, split by Package / Case above 300 | planning |
| `census/run.sh`, `census/list-avr.sh` | wrappers (credentials from `~/.zshrc`; skip pieces with a `.done`) | planning |
| `census/atdfmatch.py` | ATDF loading and the package-geometry match (`geom`, `count`, `match`) | planning |
| `census/candidates.py` | builds `candidates.tsv` from `raw/` and the ATDF directory | planning |
| `census/head.sh` | one `curl -sIL` line per URL | planning |
| `stock-check.tsv`, `census/stock-check.sh`, `census/stock-parse.py` | the part-by-part DigiKey check of the in-scope rows with no stock figure (A § 3 rule 1) | A Phase 1 |
| `census/fetch.py`, `extra-pdfs.tsv` | the datasheet downloads (bare `curl -sL`, ww1 retry); one extra maker PDF no row links | A Phase 2 |
| `census/textscan.py`, `census/sibling.py`, `census/pkgcheck.py`, `package-overrides.tsv` | verbatim hits, package codes from the ordering tables, wrong-PDF detection, sibling PDFs, the DigiKey cross-check; two datasheet typos | A Phase 2 |
| `specs.tsv`, `specs-hand.tsv`, `census/specs-draft.py`, `census/specs-build.py`, `census/verdicts.py` | per-datasheet ordering-code rules (format in `verdicts.py`) and the verdicts | A Phase 3 |
| `dead-verdicts.tsv`, `census/dead.py`, `census/docs.py`, `census/place.py` | the obsolete / NRND reading, the two datasheets/*.md blocks, placement into `datasheets/mcu/` | A Phase 4 |
| `package-codes.tsv`, `census/package-codes.py` | one row per (package code, pins): geometry, pins, the datasheet's name; C-E fill drawing and footprint | B Phase 1 |
| `census/roles.py`, `census/pinout-lint.py` | the pad-name -> role table (B § 3.2), the pinout lint | B Phase 1 |
| `pinouts.tsv`, `pinout-map.tsv`, `atdf-exclude.tsv`, `census/pinouts.py` | every pinout an add row uses (`device pinout designator name role src`) and the (device, code, pins) -> pinout map; ATDF pinouts found wrong | B Phase 1-2 |
| `transcribe-units.tsv`, `transcribed*.tsv`, `esp-pads.tsv`, `census/units.py`, `transcribe.py`, `transcribe-hand.py`, `esp-pins.py`, `transcribed-merge.py` | the pinouts the ATDF cannot give: datasheet figures read from the PDF word boxes, hand readings, the ESP32 module pin tables, the RP2040 | B Phase 2 |
| `pinout-checks.tsv`, `full-checks.tsv`, `eye-checks.tsv`, `census/pinout-check.py`, `full-check.py`, `eye-checks.py` | the name check of every ATDF pinout (text, OCR, by eye) and the ten full checks | B Phase 3 |
| `devices.tsv`, `facts-overrides*.tsv`, `census/devices.py`, `facts-check.py` | one row per device: core, clock, flash, RAM, EEPROM, supply range; checked against each datasheet | B Phase 3 |
| `drawings.json`, `drawing-numbers.tsv`, `fp-pages.tsv`, `fp-pages-hand.tsv`, `census/drawings.py`, `drawing-numbers.py`, `fp-codes.py` | each package code's drawing page (hand pages win) | C, D |
| `footprints-C.json`, `census/fp-specs-C.py` | the dual-row footprints (SOIC, TSSOP, PDIP, SOT-23, DFN) | C |
| `footprints-D.json`, `footprint-device.tsv`, `census/fp-specs-D.py` | the quad, dual-row QFN, PLCC, grid and RP2040 footprints; per-device footprints of a code with two bodies (QFN32) | D |
| `footprints-E.json`, `footprint-mpn.tsv`, `census/fp-specs-E.py` | the Espressif module footprints, read from the land-pattern drawings' vectors (needs PyMuPDF in a venv); the U variants' footprints by MPN | E |
| `census/fp-load.sh`, `fp-merge.py`, `fp-pads-vs-pins.py` | load footprint files through `footprint_check` on a copy; merge into `footprints.json`; every pinout designator has a pad | C, D, E |
| `census/gen-parts.py` | `Parts/mcu.json` (303 devices, 1424 parts) and the 151 `datasheets.json` entries from candidates / devices / pinouts / pinout-map / package-codes / footprint-device / footprint-mpn; `--check-esp` checks the ESP32 flash / PSRAM decoding against the ordering tables | G |

## Verdicts (plan-36-A, listing of 2026-11-13 UTC)

| Family | add | skip | out (datasheet) | out (stock check) | out (scope) |
|---|---|---|---|---|---|
| AVR AT90 | 23 | 15 | 0 | 3 | 0 |
| AVR ATmega | 367 | 38 | 0 | 7 | 0 |
| AVR ATtiny | 265 | 4 | 0 | 1 | 0 |
| AVR Dx | 267 | 0 | 0 | 0 | 0 |
| AVR Ex | 72 | 0 | 0 | 0 | 0 |
| AVR Lx/Sx | 14 | 0 | 0 | 0 | 0 |
| AVR XMEGA | 113 | 1 | 24 | 0 | 0 |
| AVR megaAVR 0 | 58 | 0 | 0 | 0 | 0 |
| AVR tinyAVR 0/1/2 | 195 | 3 | 0 | 0 | 0 |
| ESP32 | 48 | 0 | 3 | 0 | 16 |
| RP2040 | 2 | 0 | 0 | 0 | 0 |
| **all** | 1424 | 61 | 27 | 11 | 16 |

1424 `add` rows on 303 devices, 151 PDFs in `datasheets/mcu/` (`datasheets/mcu/datasheets.tsv`;
`place.py --verify` → `0 missing, 0 unused`). Since A: ATTINY1616-MZ-VAO became `skip` (B),
ATXMEGA8E5-MUR's code is 32Z (D), and ESP32-WROOM-DA is `out` for its datasheet's "End of
Life (EOL)" banner (E). The 8 RP2350 / RP2354 rows are `out (scope)` too and are not in the table.

Re-running G (after any change above): `fp-merge.py ../../schematical/schematical/Parts/footprints.json footprints-C.json footprints-D.json footprints-E.json` on a footprints.json without them, `gen-parts.py --write` on a datasheets.json without the mcu entries, then the goldens (`docs/backend.md` § Tests).

Re-running B: `package-codes.py`, `pinouts.py candidates.tsv /tmp/p36/packs/atdf`, `units.py`, `transcribe.py`, `transcribe-hand.py`, `esp-pins.py`, `transcribed-merge.py`, `pinouts.py ... --transcribed transcribed.tsv`, `pinout-lint.py`, `pinout-check.py --full full-checks.tsv`, `devices.py`, `facts-check.py`.

Re-running A (after the listing): `textscan.py cand.tsv /tmp/p36/pdf /tmp/p36/txt`, `sibling.py`, `specs-draft.py` → `specs-build.py`, `verdicts.py cand.tsv specs.tsv`, `dead.py cand.tsv dead-verdicts.tsv`, `place.py cand.tsv /tmp/p36/pdf`, `docs.py`.

Re-running the planning census (about 62 DigiKey calls):

```
census/list-avr.sh /tmp/p36/raw
census/run.sh /tmp/p36/raw 872 1904 - espressif
census/run.sh /tmp/p36/raw 685 1690 - rp2040
python3 -I census/candidates.py /tmp/p36/raw /tmp/p36/packs/atdf candidates.tsv
```
