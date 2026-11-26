# plan-77 data: STM32 microcontrollers

Files of plan-77 (A → H). Planning census: `planning/plan-76-data/README.md` § Plan 77 (shared by
plans 76–82), raw page `planning/plan-76-data/raw/st.json`.

| File | Letter | What |
|---|---|---|
| `raw-A/<series>_<piece>.json` | A | one DigiKey page per (series, piece): category 685, maker ST (497), In Stock, Active, no Marketplace |
| `census/list.sh`, `census/split.py`, `census/build.py` | A | the listing tools (plan-74-A's pattern) |
| `candidates.tsv` | A | every STM32 row of the listing: `part stock status series package supplier_package digikey_id datasheet_url` |
| `stock-check.tsv` | A | each row's own DigiKey `In Stock` line and status |
| `opd.tsv` | A | each row's STM32_open_pin_data match: `part refname xml package die` (or `none`) |
| `verdicts.tsv` | A | `add` (the 300) or `out` with a reason |
| `datasheets.tsv` | A | `datasheets/mcu/st/`: file, URL, bytes, SHA-256, the `add` parts it covers |
| `opd/` | A | the pinned copy of the XML files the `add` rows use, plus `LICENSE` (BSD-3-Clause) and `COMMIT` |
| `gen-stm32.py` | B | XML → `pinouts.json`, `devices.json`, `footprints-needed.txt` |
| `pinout-checks.tsv` | B | each generated pinout against its datasheet's pin table (text scan) |
| `facts.tsv` | B | clock, flash, RAM, VDD per device; source `xml` or `datasheet p.N` |
| `footprints-needed.txt` | B | one line per ST package code the `add` rows use, with the existing footprint it maps to or `new` |
| `footprints-C.json`, `footprints-D.json` | C, D | the new footprints, merged into `Parts/footprints.json` by `fp-merge.py` |
| `fp-pages.tsv` | C, D | footprint id → datasheet, page, figure of the recommended footprint |

## STM32_open_pin_data (measured at planning, 2026-11-25)

- Repository `STMicroelectronics/STM32_open_pin_data`, licence BSD-3-Clause (`LICENSE`:
  "Copyright (c) 2020, STMicroelectronics"). Last push 2026-11-16.
- `mcu/` holds 2,282 XML files, one per `RefName` (a die, a pin-count letter, a flash range and a
  package), e.g. `STM32F103C(8-B)Tx.xml`: `Family="STM32F1" Line="STM32F103" Package="LQFP48"`,
  `<Core>Arm Cortex-M3</Core>`, `<Flash>64</Flash><Flash>128</Flash>` (one per letter of `(8-B)`,
  checked on all 2,050 non-MPU files: 2,050 match, 0 do not), `<Ram>` (KB, one value or one per
  flash letter), `<Die>`, and `<Pin Name Position Type>`.
- Pin types over all files: I/O 177,233, Power 62,269, MonoIO 24,866, Reset 2,462, Boot 1,712,
  NC 1,201.
- 193 files put several names on one position (bonded pads), e.g. `STM32C011F(4-6)Px` pin 16:
  `PA9 [PA11]` and `PA11 [PA9]`; `STM32C031C(4-6)Tx` pin 29: `PA9` and `NC`.
- `<Frequency>` and `<Voltage>` are missing from 725 of the 2,050 MCU files (all of C0's 74, U3's
  213, U5's 107, H5's 115 …): those facts come from the datasheet.
- Ball rows used by BGA and WLCSP files: A–H, J–N, P, R, T, U, V, W, Y, AA–AG (no I, O, Q, S, X,
  Z: the grid style's letters). Only STM32MP1/MP2 (not MCUs) and STM32U3 WLCSP33/WLCSP42 use
  other positions (`1D8`, `19`): the U3 ones take `explicit` footprints with ball names.
- Matching DigiKey part numbers to RefNames: `(a-b-c)` → one of the letters, lowercase `x` → any
  one character. All 99 STM32 rows of the planning page match at least one RefName; 4 match two
  (the longest RefName wins).
