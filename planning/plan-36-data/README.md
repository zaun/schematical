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
| AVR tinyAVR 0/1/2 | 196 | 2 | 0 | 0 | 0 |
| ESP32 | 49 | 1 | 1 | 0 | 16 |
| RP2040 | 2 | 0 | 0 | 0 | 0 |
| **all** | 1426 | 61 | 25 | 11 | 16 |

1426 `add` rows on 304 devices, 152 PDFs in `datasheets/mcu/` (`datasheets/mcu/datasheets.tsv`). The 8 RP2350 / RP2354 rows are `out (scope)` too and are not in the table.

Re-running A (after the listing): `textscan.py cand.tsv /tmp/p36/pdf /tmp/p36/txt`, `sibling.py`, `specs-draft.py` → `specs-build.py`, `verdicts.py cand.tsv specs.tsv`, `dead.py cand.tsv dead-verdicts.tsv`, `place.py cand.tsv /tmp/p36/pdf`, `docs.py`.

Re-running the planning census (about 62 DigiKey calls):

```
census/list-avr.sh /tmp/p36/raw
census/run.sh /tmp/p36/raw 872 1904 - espressif
census/run.sh /tmp/p36/raw 685 1690 - rp2040
python3 -I census/candidates.py /tmp/p36/raw /tmp/p36/packs/atdf candidates.tsv
```
