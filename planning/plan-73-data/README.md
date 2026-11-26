# plan-73 data: USB-UART bridges

Written at planning, 2026-11-25. The census that set the scope is
`planning/plan-69-data/census/README.md` (rows for plan 73) and its raw pages here:

- `raw/usbuart.json`: DigiKey Controllers (753), Function "Bridge, USB to UART"
  (`110` = 323050), In Stock + Active: 38 parts; the maker facet.
- `raw/usbuart2.json`: the same page saved again with all 38 rows, each with its
  In Stock line (`firstPage`).
- `raw/ftdi.json`: FTDI (`-1` = 768) in 753, In Stock + Active: 17 parts, none a UART
  bridge (FIFO 5, I2C 3, FT1248 1, Controller 8).
- `raw/c753.json`: the whole Controllers category (Protocol and Function facets).

`fetch-datasheets.py` downloads the eight PDFs (plan-73-B Phase 1).

## The 38 in-stock rows against the library

`python3 -I` over `raw/usbuart2.json` and `schematical/schematical/Parts/serial.json`
(each row's part number looked up among every serial device's part names):

| Rows | Maker | In `serial.json` |
|---|---|---|
| 8 | Microchip (MCP2200: -I/SO, T-I/SO, -I/MQ, T-I/MQ, -I/SS, T-I/SS, -E/SS, T-E/SS) | all, device `MCP2200` |
| 7 | Infineon (CY7C65213 ×4, CY7C65213A ×2, CY7C64225 ×1) | all |
| 9 | MaxLinear (XR21B1411/1420/1422 ×1, XR21B1424 ×2, XR21V1410/1412 ×1, XR21V1414 ×2) | all |
| 14 | Silicon Labs | **none** |

The 14 Silicon Labs rows, with the In Stock quantity DigiKey showed (2026-11-25):

| Part | In Stock | Device (plan-73-C) | Pinout | Footprint (plan-73-B) |
|---|---|---|---|---|
| CP2102N-A02-GQFN20 | 6,619 | `CP2102N-QFN20` | `qfn20` | `QFN-20-3X3-SL` |
| CP2102N-A02-GQFN20R | 3,957 | `CP2102N-QFN20` | `qfn20` | `QFN-20-3X3-SL` |
| CP2102N-A02-GQFN24 | 1,302 | `CP2102N` | `qfn24` | `QFN-24-4X4-SL-N` |
| CP2102N-A02-GQFN24R | 1,877 | `CP2102N` | `qfn24` | `QFN-24-4X4-SL-N` |
| CP2102C-A01-GQFN28R | 1,450 | `CP2102C` | `qfn28` | `QFN-28-5X5-SL-C` |
| CP2104-F03-GM | 5,999 | `CP2104` | `qfn24` | `QFN-24-4X4-SL` |
| CP2104-F03-GMR | 1,533 | `CP2104` | `qfn24` | `QFN-24-4X4-SL` |
| CP2105-F01-GM | 2,148 | `CP2105` | `qfn24` | `QFN-24-4X4-SL` |
| CP2105-F01-GMR | 4,223 | `CP2105` | `qfn24` | `QFN-24-4X4-SL` |
| CP2108-B03-GM | 6 | `CP2108` | `qfn64` | `QFN-64-9X9-SL` |
| CP2108-B03-GMR | 1,938 | `CP2108` | `qfn64` | `QFN-64-9X9-SL` |
| CP2110-F01-GM | 4,163 | `CP2110` | `qfn24` | `QFN-24-4X4-SL` |
| CP2110-F01-GMR | 1,896 | `CP2110` | `qfn24` | `QFN-24-4X4-SL` |
| CP2110-F02-GM1 | 764 | `CP2110-QFN28` | `qfn28` | `QFN-28-5X5-SL` |

Two more rows from the keyword probes (`planning/plan-69-data/census/kw.js`, category 753):

| Part | In Stock | Device | Pinout | Footprint |
|---|---|---|---|---|
| CP2102-GMR | 8,705 | `CP2102` | `qfn28` | `QFN-28-5X5-SL` |
| CH340T (DigiKey sells it as Olimex `SMD IC CH340T`) | 4 | `CH340T` | `ssop20` | `SSOP-20` (exists) |

16 parts, 10 devices, 6 new footprints.

## Rows with no stock (left out, recorded in `datasheets/skipped-parts.md` by plan-73-C)

| Part(s) | Why |
|---|---|
| CP2102N-A02-GQFN28, CP2102N-A02-GQFN28R, CP2102-GM, CP2102C-A01-GQFN24, CP2102C-A01-GQFN24R, CP2102C-A01-GQFN28, CP2102N-A01-GQFN20, CP2102N-A01-GQFN20R, CP2102N-A01-GQFN24, CP2102N-A01-GQFN24R, CP2102N-A01-GQFN28, CP2102N-A01-GQFN28R | DigiKey In Stock 0 (keyword probe `CP2102`, all 18 rows listed, 2026-11-25: `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/x/753?keywords=CP2102" 12 "$(sed 's/slice(0,8)/slice(0,100)/' planning/plan-69-data/census/kw.js)"` → the 6 in-stock rows above and these 12 at 0) |
| FT232RL-TUBE, FT232RL-REEL, FT232RNL-TUBE, FT232RNL-REEL, FT232RQ-TRAY, FT232RQ-REEL, FT232RNQ-TRAY, FT232RNQ-REEL | In Stock 0 (probe `FT232R`, 8 rows) |
| FT231XS-R, FT231XS-U, FT231XQ-R, FT231XQ-T | In Stock 0 (probe `FT231X`) |
| FT230XS-R, FT230XS-U, FT230XQ-R, FT230XQ-T | In Stock 0 (probe `FT230X`) |
| FT234XD-R, FT234XD-T | In Stock 0 (probe `FT234X`) |
| CH340X (Olimex `SMD IC CH340X`) | In Stock 0 (probe `CH340`) |
| CH340G, CH340C, CH340N, CH340K, CH340E, CH340B | no DigiKey listing (probe `CH340`: 3 rows, CH340T, CH340X and a SparkFun board) |

## Datasheets (downloaded at planning 2026-11-25 with `fetch-datasheets.py`'s method)

| id (`datasheets/controller-uart/<id>.pdf`) | URL | Bytes | Pages | Pin table | Land pattern |
|---|---|---|---|---|---|
| `cp2102-9` | https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf | 2,172,800 | 26 | p.11 Table 9 | p.14 Table 11 (QFN-28) |
| `cp2102n` | https://www.silabs.com/documents/public/data-sheets/cp2102n-datasheet.pdf | 2,252,034 | 48 | p.29 Table 5.2 (QFN24), p.31 Table 5.3 (QFN20) | p.39 Table 7.2 (QFN24), p.43 Table 8.2 (QFN20) |
| `cp2102c` | https://www.silabs.com/documents/public/data-sheets/cp2102c-datasheet.pdf | 664,111 | 31 | p.18 Table 5.1 (QFN28) | p.24 Table 6.2 (QFN28) |
| `cp2104` | https://www.silabs.com/documents/public/data-sheets/CP2104.pdf | 2,067,999 | 22 | p.8 Table 7 | p.11 Table 9 (QFN-24) |
| `cp2105` | https://www.silabs.com/documents/public/data-sheets/cp2105.pdf | 1,601,911 | 24 | p.7–8 Table 7 | p.11 (QFN-24) |
| `cp2108` | https://www.silabs.com/documents/public/data-sheets/cp2108-datasheet.pdf | 813,001 | 35 | p.12–14 Table 4.1 | p.17 (QFN64) |
| `cp2110` | https://www.silabs.com/documents/public/data-sheets/CP2110.pdf | 1,185,044 | 26 | p.8–9 Table 7 (QFN24 and QFN28 columns) | p.13 (QFN24), p.15 (QFN28) |
| `ch340` | https://www.wch-ic.com/download/file?id=79 | 325,640 | 17 | p.3–4 § 4 | none: p.2 § 3 gives only body width 5.3 mm and pitch 0.65 mm for SSOP-20 |

`pdftotext -layout` merges the CP2102/9 table's footnote digits into the pin
numbers (it prints `CTS 233`, `DCD 13`, `SUSPEND 113`: pins 23, 1 and 11 with
note 3). The pin numbers in plan-73-C are read from the page, not the text dump.
