# plan-71 data — CAN, RS-485, RS-232 and LIN line transceivers

Working files of plan-71 (planning/plan-71-A … J). Written at planning time
(2026-11-25, at 63e2f7d2) unless a letter says otherwise.

## raw/ (planning census, 2026-11-25)

Each file is one DigiKey category page through Safari, filtered In Stock +
Active, saved by `planning/plan-69-data/census/facets.sh` (the shared tools,
read-only here; method and owner decisions in
`planning/plan-69-data/census/README.md`). Fields: `productCount`,
`filters` (every facet with its options' `productCount`), `firstPage`
(the first 100 products: part number, maker, In Stock lines).

| File | Category (id) | Extra filter | Total |
|---|---|---|---|
| `c710.json` | Drivers, Receivers, Transceivers (710) | — | 5,874 |
| `can.json` | 710 | Protocol `578` = 325925 (CANbus) | 574 |
| `rs485.json` | 710 | Protocol 399047 (RS422, RS485), 399058 (RS485) | 1,777 |
| `rs232.json` | 710 | Protocol 398950 (RS232) | 1,433 |
| `lin.json` | 710 | Protocol 365041 (LIN), 365059 (LINbus) | 130 |

Show one: `python3 -I planning/plan-69-data/census/show.py planning/plan-71-data/raw/can.json Manufacturer 'Package / Case'`.

## The pick at planning time

Top three makers per protocol (ADI = "Analog Devices Inc." 505 + "Analog
Devices Inc./Maxim Integrated" 175; legacy Rochester labels are not makers),
each capped at its 200 most-stocked parts:

| Protocol | Maker (facet count) | Picked |
|---|---|---|
| CAN | Texas Instruments (204) | 200 |
| CAN | NXP USA Inc. (108) | 108 |
| CAN | Microchip Technology (95) | 95 |
| RS-485 | ADI (717 + 415 = 1,132) | 200 |
| RS-485 | Texas Instruments (334) | 200 |
| RS-485 | MaxLinear, Inc. (111) | 111 |
| RS-232 | ADI (505 + 229 = 734) | 200 |
| RS-232 | Texas Instruments (251) | 200 |
| RS-232 | Renesas Electronics Corporation (101) | 101 |
| LIN | Texas Instruments (39) | 39 |
| LIN | NXP USA Inc. (34) | 34 |
| LIN | onsemi (18) | 18 |
| | **total** | **1,506** |

Package facets across the four protocols (all makers, summed facet counts:
`python3` over the four raw files' filter `16`): 114 distinct Package / Case
values, 70 leaded (3,430 part listings) and 44 leadless (DFN, QFN, SON, CSP,
BGA: 484 part listings).

## Datasheet hosts (probed 2026-11-25)

`python3 -I <scratch>/probe.py` (urllib, a Safari User-Agent plus `Accept`
and `Accept-Language`) saved a `%PDF-` file from each:

- TI `https://www.ti.com/lit/ds/symlink/tcan1042h.pdf` (1,356,223 bytes)
- ADI `https://www.analog.com/media/en/technical-documentation/data-sheets/max3483-max3491.pdf` (3,334,161)
- MaxLinear `https://www.maxlinear.com/ds/sp3485.pdf` (936,182)
- NXP `https://www.nxp.com/docs/en/data-sheet/TJA1051.pdf` (209,490)
- Microchip `https://ww1.microchip.com/downloads/en/DeviceDoc/20005167C.pdf` (800,643)
- onsemi `https://www.onsemi.com/pdf/datasheet/ncv7329-d.pdf` (507,933)
- Renesas, DigiKey's own link `https://www.renesas.com/en/document/dst/hin202e-hin206e-hin207e-hin208e-hin211e-hin213e-hin232e-datasheet` (1,400,008).
  Guessed Renesas URLs (`…/document/dst/isl3232e-datasheet`) return 404, so
  A uses DigiKey's link for every Renesas row.
- DigiKey links MaxLinear rows to its own copies (`mm.digikey.com/…`); A
  rewrites them to `maxlinear.com/ds/<part>.pdf` (the memory note on
  download blocks), and falls back to `scripts/safari-digikey-pdf.sh`.

## Files the letters add

- `census/` (A): the listing scripts, adapted from `planning/plan-58-data/census/`
  and `planning/plan-48-data/census/`.
- `listing/` (A): one saved page per (maker, protocol, split).
- `candidates.tsv`, `stock-check.tsv`, `verdicts.tsv`, `specs.tsv` (A, B–D).
- `footprints-needed.txt`, `footprints-E.json`, `footprints-F.json`,
  `footprint-map.tsv` (B–F).
