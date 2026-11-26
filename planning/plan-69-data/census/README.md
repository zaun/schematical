# Planning census for plans 69–74 (2026-11-25)

All counts: DigiKey through Safari (`scripts/safari-digikey.sh`, AGENTS.md), each
category page filtered **In Stock** (`-2` = 5) and **Active** (`1989` = 0). They are
facet counts (`productCount` of each facet option). DigiKey's In Stock facet also
counts Marketplace and Factory-only stock (see the memory note in
`planning/completed/plan-58-A-slide-switch-data.md` and
`planning/plan-58-data/census/`), so the A letter of each plan checks stock part by part.

Tools (this folder): `url.js` (the `?s=` lz-string filter URL), `facets.sh` (one page,
saved), `show.py` (prints a facet, sorted by count), `kw.js` (keyword probe: count plus
the first 8 rows with their In Stock lines). Raw pages saved without product rows beyond
page one: `planning/plan-NN-data/raw/*.json`.

Commands: `census/facets.sh <out> x/<cat> '<extra>'`, then
`python3 -I census/show.py <out> Manufacturer`.

DigiKey's legacy labels for Rochester stock (Harris Corporation, National Semiconductor,
Intersil, Sipex, Fairchild, Elantec, SMSC, Rochester Electronics) are not makers
(plan-48 § 2). "Analog Devices Inc." (505) and "Analog Devices Inc./Maxim Integrated"
(175) count as one maker, ADI (plan-48).

| Plan | Family | Category (id) | Extra filter | Total | Top makers (in-stock Active) | raw file |
|---|---|---|---|---|---|---|
| 69 | TVS diodes (all) | TVS Diodes (144) | — | 24,278 | Littelfuse 3,287; Vishay GS 2,515; YAGEO 1,376 | plan-69-data/raw/c144.json |
| 69 | TVS, Type Zener | 144 | `183`:[423536] | 22,406 | Littelfuse 3,252; Vishay GS 2,508; YAGEO 1,365; MDE 1,222; Bourns 1,193 | tvsz.json |
| 69 | TVS, Type Steering (rail to rail): ESD arrays | 144 | `183`:[408274] | 426 | Semtech 88; Littelfuse 34; Diodes Inc. 33; onsemi 26; TI 24 | tvss.json |
| 70 | Optoisolators, transistor / photovoltaic output | 903 | — | 3,695 | Isocom 930; Vishay 671; onsemi 505; Broadcom 361; Toshiba 305 | plan-70-data/raw/c903.json |
| 70 | Optoisolators, logic output | 902 | — | 924 | Broadcom 376; Toshiba 159; Renesas 118; onsemi 88 | c902.json |
| 70 | Optoisolators, triac / SCR output | 904 | — | 731 | onsemi 197; Vishay 127; Würth 102; Isocom 91 | c904.json |
| 70 | Digital isolators | 901 | — | 1,895 | ADI 944 (774 + 170); TI 330; Skyworks 321; NVE 81 | c901.json |
| 71 | Drivers, receivers, transceivers (all) | 710 | — | 5,874 | — | plan-71-data/raw/c710.json |
| 71 | CAN | 710 | `578`:[325925] CANbus | 574 | TI 204; NXP 108; Microchip 95; ADI 80 (54 + 26) | can.json |
| 71 | RS-485 | 710 | `578`:[399047 RS422/RS485, 399058 RS485] | 1,777 | ADI 1,132 (717 + 415); TI 334; MaxLinear 111; Renesas 81 | rs485.json |
| 71 | RS-232 | 710 | `578`:[398950] | 1,433 | ADI 734 (505 + 229); TI 251; Renesas 101; MaxLinear 75 | rs232.json |
| 71 | LIN | 710 | `578`:[365041 LIN, 365059 LINbus] | 130 | TI 39; NXP 34; onsemi 18; Infineon 14 | lin.json |
| 72 | I/O expanders | 749 | — | 453 | TI 101; ADI 78 (75 + 3); NXP 65; Microchip 62 | plan-72-data/raw/c749.json |
| 72 | I2C buffers / repeaters / accelerators | 756 | `405`:[802080 I2C-Hotswap, 356918 I2C, 797936, 752420, 802081] | 148 | ADI 92 (91 + 1); NXP 23; TI 13; Diodes 10 | i2cbuf.json |
| 72 | I2C muxes / switches | Signal Switches, Multiplexers, Decoders (743) | keyword probes (`kw.js`, In Stock + Active) | — | `TCA954` 11 (TI); `PI4MSD` 10 (Diodes); `PCA984` 4 (NXP); `PCA9548`, `PCA9544`, `PCA9543`, `PCA9540`, `PCA9541` 0 each; `LTC4306` 0; `MAX735` 0 | c743.json (category facets) |
| 73 | USB bridges, Function "Bridge, USB to UART" | Controllers (753) | `110`:[323050] | 38 | Silicon Labs 14; MaxLinear 9; Microchip 8; Infineon 7 | plan-73-data/raw/usbuart.json, usbuart2.json (all 38 rows) |
| 73 | FTDI, all in-stock Active in 753 | 753 | `-1`:[768] | 17 | none is a UART bridge: FIFO 5, I2C 3, FT1248 1, Controller 8 | ftdi.json |
| 74 | Ethernet transceivers / PHYs | 710 | `578`: Ethernet, IEEE 802.3, PHY, MII, RMII, Gigabit (13 ids, in the raw file's request) | 267 | Microchip 151; TI 57; ADI 25 (24 + 1); Infineon 15 | plan-74-data/raw/eth710.json |
| 74 | Ethernet ICs in Controllers | 753 | `578`:[345107] | 314 | Function: Switch 156, Controller 109, USB to Ethernet 24, MAC 18, PHY 7 | eth753.json |
| 74 | Single-port Ethernet controllers with a host bus | 753 | Ethernet + Function Controller/MAC/PHY + Interface Parallel, SPI, SPI+Parallel, I2C+SPI, Bus | 74 | Microchip 64; WIZnet 4; onsemi 2; Renesas 2; Cirrus 1; NetBurner 1 | ethc2.json |
| 74 | RJ45 jacks with magnetics | 365 | — | 1,112 | Pulse 222; Bel Fuse 185; TE 119; Abracon 113 | c365.json |

Keyword probes for named parts (`kw.js`; category 753, no stock filter, so every row
shows its own In Stock line):

- `FT232R` 8 rows, every one `In Stock: 0` (FT232RL-TUBE/-REEL, FT232RNL, FT232RQ, FT232RNQ).
- `FT231X` 4 rows, `FT230X` 4 rows, `FT234X` 2 rows: every one `In Stock: 0`.
- `CH340` 3 rows: `SMD IC CH340T` (Olimex LTD) **In Stock: 4**; `SMD IC CH340X` (Olimex) 0;
  SparkFun 16278 (a board) 0. DigiKey lists no WCH maker entry.
- `CH9102` 0 rows.
- `CP2102` 18 rows: CP2102-GMR 8,705; CP2102N-A02-GQFN20 6,619; -GQFN20R 3,957;
  -GQFN24 1,302; -GQFN24R 1,877; CP2102C-A01-GQFN28R 1,450; the other 12 rows (CP2102N-A02-GQFN28/-R, CP2102-GM,
  CP2102C-A01-GQFN24/-R/-GQFN28, every CP2102N-A01-*) In Stock 0 (all 18 rows listed with `slice(0,100)`).
- `TCA9548A` (743, no filter) 5 rows: TCA9548APWR 27,262; TCA9548ARGER 36,264;
  TCA9548ARGERQ1 32,039; TCA9548AMRGER 0; TCA9548ADGSR 0.

Datasheet download probes:

- silabs.com blocks curl (memory: datasheet-download-blocks), but
  `scripts/safari-digikey-pdf.sh https://www.silabs.com/documents/public/data-sheets/cp2102n-datasheet.pdf out.pdf`
  saved a 2,252,034-byte, 10-page PDF titled "CP2102N Data Sheet" (2026-11-25).
- WCH: `curl -sL -A "Mozilla/5.0" "https://www.wch-ic.com/download/file?id=79"` gives the
  4-page "CH340 Datasheet, Version 3D" (2026-11-25).

Owner decisions taken at planning (2026-11-25):

1. Pick rule: the top 3 makers per family by DigiKey in-stock count, Active (and Last
   Time Buy) only. A family with more than 200 parts at one maker keeps that maker's
   200 most-stocked. Named parts (CH340, FT232, CP2102) are always in scope.
2. Families besides those asked for: RS-232, digital isolators, LIN, I2C buffers and muxes.
3. Simulation: logic where it is cheap. IO expanders run on the existing I2C/SPI bus
   engines, USB-UART bridges on the existing `usbuart` model, optocouplers and digital
   isolators as logic, and CAN/RS-485/RS-232/LIN transceivers as logic. TVS/ESD parts and
   Ethernet ICs are facts only.
4. Ethernet: PHYs, single-port controllers with an SPI or parallel host bus plus WIZnet's
   parts, and RJ45 jacks with magnetics. Switch ICs and USB-to-Ethernet bridges are out.
5. IO expanders: Microchip is added as a 4th maker.
6. A named part with no in-stock DigiKey listing is left out and recorded in
   `datasheets/skipped-parts.md`. So FT232R/FT231X/FT230X are out (In Stock 0), and
   CH340T is in (Olimex listing, In Stock 4).
