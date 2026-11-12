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

Re-running the planning census (about 62 DigiKey calls):

```
census/list-avr.sh /tmp/p36/raw
census/run.sh /tmp/p36/raw 872 1904 - espressif
census/run.sh /tmp/p36/raw 685 1690 - rp2040
python3 -I census/candidates.py /tmp/p36/raw /tmp/p36/packs/atdf candidates.tsv
```
