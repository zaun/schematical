# plan-76 data (sensors)

The planning census of plans 76–82 is `README.md` in this folder (§ Plan 76). This file lists
plan-76's own files. Every file here is written by the letter named in its row.

| Path | Written by | What |
|---|---|---|
| `census/list76.sh` | A | the listing: one Safari page per (family, category, maker, piece) |
| `census/split76.py` | A | splits a (family, maker) page over 100 rows by a facet (§ A 3.1) |
| `census/check76.py` | A | a saved page is non-empty and every row is of the requested maker |
| `census/build76.py` | A | `candidates.tsv`, `stock-check.tsv`; `--verify` |
| `raw-A/<family>_<cat>_<maker>_<piece>.json` | A | the listing pages, product rows kept |
| `candidates.tsv` | A | one row per (family, maker, part); scope and verdict columns |
| `stock-check.tsv` | A | DigiKey's own `In Stock` line per `in` row |
| `census/fetch76.py` | B | downloads (`--probe`: the planning probe URLs below) |
| `census/textscan76.py` | B | verdicts from the PDFs' text; `--verify` |
| `verdicts.tsv` | B | `add` / `skip` per `in` row |
| `specs-<family>.tsv` | B (skeleton), C–K (filled) | one row per (maker, datasheet, package) |
| `pinouts/<datasheet>-<package>.json` | C–K | one part-file pinout per row |
| `census/specs-check76.py` | C | checks every filled specs row (§ C 3.4) |
| `footprints-needed.txt` | C–K | new footprint ids with their drawing reference |
| `footprints-<family>.json` | D, E, G, H, J, K | new footprints, merged by `census/fp-merge76.py` |
| `census/gen-sensor.py` | M | writes `Parts/sensor.json` from the specs files |

Planning download probe (2026-11-25, Python `urllib`, Safari user agent plus `Accept` and
`Accept-Language`; `200` = the body starts `%PDF-`). The URLs are the census pages' own
`datasheetUrl` values (the first row of each maker in the scratchpad copies of `b5xx.json` that
kept their product rows):

| Maker (family) | Host | Result |
|---|---|---|
| Allegro (current, magnetic) | `www.allegromicro.com` (`ACS711-Datasheet.ashx`) | 200, 1,257,307 B |
| LEM (current) | `www.lem.com` | 200, 1,965,469 B |
| Melexis (current) | `www.melexis.com` | 200, 1,368,861 B |
| TI (all) | `www.ti.com/lit/ds/symlink/tmcs1108.pdf` (rewritten from DigiKey's `suppproductinfo.tsp?…gotoUrl=…/lit/gpn/tmcs1108`) | 200, 2,212,276 B |
| Amphenol Telaire (humidity), NovaSensor (pressure) | `www.amphenol-sensors.com/hubfs/Documents/…` | 200, 472,058 B; 200, 1,098,253 B |
| Sensirion (humidity) | `sensirion.com/media/documents/…` | 200, 1,049,911 B |
| TE (humidity, motion) | `www.te.com/commerce/DocumentDelivery/DDEController?…` | 200, 1,973,501 B |
| IST (humidity) | `www.ist-ag.com/sites/default/files/downloads/…` | 200, 625,448 B |
| Broadcom (light) | `docs.broadcom.com/docs/AV02-1169EN` → an HTML page (16,588 B); **`docs.broadcom.com/doc/AV02-1169EN`** → 200, 338,742 B | rewrite `/docs/` → `/doc/` |
| Vishay (light) | `www.vishay.com/docs/…` | 200, 295,562 B |
| ams OSRAM (light) | `look.ams-osram.com/m/…/original/SFH-5711.pdf` | 200, 1,561,157 B |
| Diodes (magnetic) | `www.diodes.com/assets/Datasheets/…` | 200, 577,809 B |
| Infineon (magnetic) | `www.infineon.com/assets/row/public/documents/…` | 200, 2,284,179 B |
| ADI (temperature, motion) | `www.analog.com/media/en/technical-documentation/data-sheets/…` | 200, 1,275,312 B |
| Memsic (motion) | `www.memsic.com/Public/Uploads/uploadfile/files/…` | 200, 1,525,518 B |
| ST (motion, pressure) | `www.st.com/content/ccc/resource/…` | 200, 1,205,748 B |
| ST's former NXP pressure sensors | `www.nxp.com/docs/en/data-sheet/…` (36 of ST's first 42 pressure rows link nxp.com) | 200 (`PCT2075.pdf`, 511,371 B) |
| Honeywell (pressure) | `prod-edam.honeywell.com/content/dam/…` | 200, 3,747,087 B |
| Amphenol All Sensors (pressure) | `428059.fs1.hubspotusercontent-na1.net/hubfs/428059/Product Data Sheets/DS-0023-Rev-A.pdf` → 200, 286,218 B; `www.allsensors.com/datasheets/DS-0024_Rev_C3.pdf` → **404** | a 404 row tries DigiKey's copy, else `skip` (`no PDF`) |
| NXP (temperature) | `www.nxp.com/docs/en/data-sheet/PCT2075.pdf` | 200, 511,371 B |
| Microchip (temperature) | `ww1.microchip.com/downloads/aemDocuments/…` | 200, 599,186 B |
| DigiKey copies (`mm.digikey.com`, several makers) | `scripts/safari-digikey-pdf.sh` | the AGENTS.md route |

Reference-board parts in scope (`planning/plan-43-data/candidates.tsv` family `sensor`, rows
210–219): `TCN75AVOA713` (Microchip, temperature, 518) and `VEML7700-TT` (Vishay, light, 536).
The other eight (ADE7953, MAX31855, AS5600, AR1021, EMC2101, SCD41, TS4231, VBPW34FASR) are in
categories outside plan-76's seven families.
