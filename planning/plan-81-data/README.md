# plan-81 data: OLED displays

Files of plan-81 (`planning/plan-81-*.md`). The census they start from is the shared planning census
of plans 76–82, `planning/plan-76-data/README.md` § Plan 81 (raw pages
`planning/plan-76-data/raw/o107.json` and `o99.json`, which keep every product row).

| File | Written by | Content |
|---|---|---|
| `census-rows.py` | planning (2026-11-25) | writes `census-rows.tsv` from the two raw pages |
| `census-rows.tsv` | planning (2026-11-25) | the 95 in-scope-maker rows of `o107` + `o99`, one per part: category, maker, part, DigiKey's own `In Stock` quantity, DigiKey's Controller Type, format, interface, display type, datasheet URL. Generator: `census-rows.py` (§ Regenerate) |
| `candidates.tsv` | A Phase 1 | `census-rows.tsv` plus `scope`, `reason` |
| `stock-check.tsv` | A Phase 1 | per `in` row: In Stock line, status, date |
| `datasheets.tsv` | A Phase 2 | per PDF: file, URL, bytes, SHA-256, how fetched |
| `verdicts.tsv` | A Phase 3 | per `in` row: `add` or `skip`, `target` (`oled-sim`, `oled-facts`, `charlcd`), controller, reason |
| `specs.tsv` | A Phase 4 | per (maker, datasheet, pinout): pins, bus, straps, glass, colour, supply, drawing page |
| `footprints-needed.txt` | A Phase 4 | footprint ids E draws, one per line |
| `gen-oled.py` | E Phase 2 | writes `Parts/oled.json`'s parts and the new `charlcd.json` devices from `specs.tsv` + `verdicts.tsv` |
| `fixtures/` | C, D | the command streams the model tests replay (also inline in `backend/tests/oled_test.c`) |

## Planning measurements (2026-11-25, at 686408f4)

Makers and counts (In Stock, Active, no Marketplace; `census-rows.tsv`):

| Maker | Graphic (107) | Character (99) | Total |
|---|---|---|---|
| Newhaven Display Intl | 28 | 19 | 47 |
| Display Visions | 17 | 7 | 24 |
| Orient Display | 13 | 0 | 13 |
| DLC Display Co.,Ltd | 11 | 0 | 11 |
| **all** | 69 | 26 | **95** |

`cut -f2 census-rows.tsv | sort | uniq -c` → 11 DLC, 24 Display Visions, 47 Newhaven, 13 Orient.

DigiKey's Controller Type of the 95 (`cut -f5 census-rows.tsv | sort | uniq -c | sort -rn`): `-` 35,
SSD1322 15, US2066 7, SSD1351 7, SSD1306 4, SSD1305 4, SSD1333 3, SSD1353 2, SSD1331 2, SSD1309 2,
"SPD0301, SSD1305, SSD1309" 2, SH1107 2, SSD1357Z 1, SSD1317 1, SSD1315 1, SSD1311 1, SSD1306BZ 1,
"SSD1306, SSD1315" 1, SH8601 1, SH1106G 1, "SH1106, SSD1306" 1, CH1116 1. A reads the 35 `-` rows'
controllers from their datasheets. Spot checks at planning (`pdftotext <pdf> - | grep -o -E
'SSD13[0-9]{2}[A-Z]*|SH110[0-9]|WS0010|US2066'`):

- Display Visions `oledl128-6e.pdf` (EA OLEDL128-6…): SSD1309, BS0/BS1 straps.
- Display Visions `oleds102-6e.pdf` (EA OLEDS102-6…): SSD1306B.
- Newhaven `NHD-2.23-12832UCB3.pdf`: SSD1305, BS1/BS2 straps.
- Newhaven `NHD-0420DZW-AY5.pdf` (character, 23 pages): controller not named ("Controller IC
  Upgraded", rev 15, 2021-02-25); interface "4/8-bit 6800/8080 Parallel, SPI", **6800 parallel the
  default** (jumpers J68 / J80, p. 4 table).

Datasheet hosts (`datasheet_url` column; probe with curl, Safari user agent, 2026-11-25):

| Host | Rows | Probe |
|---|---|---|
| `newhavendisplay.com` | 47 | curl 200, `%PDF-` (`NHD-1.91-176176B.pdf` 1,949,708 B) |
| `www.lcd-module.de` / `www.lcd-module.com` / `shop.lcd-module.de` | 17 / 6 / 1 | curl 200, `%PDF-` (`oledl128-6e.pdf` 1,064,388 B; `oleds102-6e.pdf` 1,343,763 B) |
| `www.orientdisplay.com` | 13 | curl 403; `scripts/safari-digikey-pdf.sh <url> out.pdf` → 477,365 B `%PDF-`, 18 pages, title `AOM12864A0-0.96WW-ANO` |
| `www.dlcdisplay.com` | 10 | the URL is a PDF.js viewer page; its file is `/nportal/fwebapi/cms/lowcode/file/getFileById?id=<id>` (read from the viewer's resource list in Safari). curl and Safari both get HTTP 430, a "Network Security Verification — drag slider to verify" page (17,334 B) |
| `mm.digikey.com` | 1 (DLC-D1.12ABW#1) | DigiKey copy: `scripts/safari-digikey-pdf.sh` |

Controller datasheets (curl, 2026-11-25, all `%PDF-`):

| Controller | URL | Bytes |
|---|---|---|
| SSD1306 (rev 1.1, 65 pages; PDF title "SSD1780") | `https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf` | 1,876,686 |
| SSD1305 | `https://cdn-shop.adafruit.com/datasheets/SSD1305.pdf` | 2,275,678 |
| SSD1309 | `https://www.lcd-module.de/fileadmin/eng/pdf/zubehoer/ssd1309.pdf` | 1,803,348 |
| SH1106 | `https://www.displayfuture.com/Display/datasheet/controller/SH1106.pdf` | 439,395 |
| SH1107 | `https://www.displayfuture.com/Display/datasheet/controller/SH1107.pdf` | 508,404 |
| US2066 | `https://newhavendisplay.com/content/app_notes/US2066.pdf` | 1,033,459 |

SSD1315 and CH1116 have no controller datasheet among these hosts (`displayfuture.com/…/SSD1315.pdf`
and `…/CH1116.pdf` return a 1,190-byte script page). plan-81 models them as the SSD1306 and SH1106
profiles (plan-81-C § 3.1), from the command tables their module datasheets print.

SSD1306 facts the model is written from (`pdftotext -layout SSD1306.pdf`): Table 7-1 (BS[2:0]: I2C
010, 6800 100, 8080 110, 4-wire SPI 000, 3-wire SPI 001); Table 8-1 (pin use per interface: SCLK =
D0, SDIN = D1, SDAOUT = D2, D/C# = SA0 in I2C); § 8.1.5 slave address `011110 SA0 R/W#`; "No data
read is provided in serial mode operation" (§ 8.7, text line 1838); Table 9-1 (the command table and
its RESET values); Table 10-3 (COM pins hardware configuration).

## Regenerate `census-rows.tsv`

```
python3 -I planning/plan-81-data/census-rows.py planning/plan-76-data/raw/o107.json planning/plan-76-data/raw/o99.json | diff - planning/plan-81-data/census-rows.tsv
```

→ no output (checked at planning, 2026-11-25).
