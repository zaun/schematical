# Skipped parts

Parts that were looked at for the catalog and left out for now, and why.
Unlike `obsolete-not-included.md`, these parts are still made; a later plan
may add them. Their PDFs are not in the tree.

## WDC (plan-14)

WDC chips are not sold by DigiKey, so plan-14 uses WDC's own list as the
stock source (https://wdc65xx.com/where-to-buy, WDC stock as of 01/01/2024;
read 2026-11-01).

| Part(s) | Device | Would have gone into | Why skipped |
|---|---|---|---|
| W65C134S8PLG-8, W65C134S8QG-8 | W65C134S microcontroller (65C02 core), `w65c134s.pdf` | `cpu` / Processor | Owner, 2026-11-01: skipped for now. Its on-chip timers, UART, serial interface bus, ports and tone generators have no MIT or zlib model to reuse and would all be written from scratch. WDC's internal ROM monitor (github.com/WesternDesignCenter/W65C134S-Internal-ROM-Monitor) is BSD-3, outside the MIT/zlib rule. |
| W65C265S8PLG-8, W65C265S8QG-8 | W65C265S microcontroller (65C816 core), `w65c265s.pdf` | `cpu` / Processor | Owner, 2026-11-01: skipped for now, for the W65C134S's reasons (its peripheral set is larger: four UARTs, eight timers, PWM). Its ROM monitor is BSD-3 and the source still carries a 1994–95 "Confidential and Proprietary" notice. W65C265S8QG-8 also shows 0 WDC stock. |
| W65C22N6TPLG-14 | W65C22N VIA, PLCC-44 | `pio` / Interface → VIA | 0 WDC stock (the stock rule). The W65C22N PDIP and the W65C22S parts are added. |
| W65C22S6TQG-14 | W65C22S VIA, QFP-44 | `pio` / Interface → VIA | WDC publishes no outline for its 44-pin QFP (`w65c22.pdf` gives the pinout only, p.28), so no footprint can be drawn; same rule as W65C02S6TQG-14 (plan-09-A § Decisions). |
| W65C816S6TQG-14 | W65C816S, QFP-44 | `cpu` / Processor | No WDC outline for the 44-pin QFP (`w65c816s.pdf` gives the pinout only, p.11); as above. |

## TI inverters (2026-11-07)

In-stock TI inverter parts (`node scripts/digikey.js --in-stock`, category Gates and Inverters,
Texas Instruments; 2026-11-07) left out when the inverter devices were added to `gate.json`.
TI inverting buffers with 3-state outputs ('240 types) and the SN74LVC1GX04 crystal-oscillator
driver were not taken up as inverters.

| Part(s) | Device | Would have gone into | Why skipped |
|---|---|---|---|
| SN74HC04ANS, SN74HC04ANSR, SN74HC04ANSRE4, SN74HC04APWR, SN74HC05ANSR, SN74HC05APW, SN74HC05APWR, SN74HC14ADBLE, SN74HC14ADBR, SN74HC14AN, SN74HC14ANSR, SN74HC14APW, SN74HCT04ANSR, SN74HCT04ANSRE4, SN74HCT04APW, SN74HCT04APWLE, SN74HCT04APWR, SN74HCU04AN, SN74HCU04ANSR, SN74HCU04APW | SN74HC04A, SN74HCT04A, SN74HCU04A, SN74HC05A, SN74HC14A (new "A" revisions) | `gate` / Logic → NOT | TI has product pages but no published datasheet (ti.com/lit symlink and gpn links return no PDF, 2026-11-07); the older SN74HC04/HCT04/HCU04/HC05/HC14 datasheets do not list these part numbers. |
| 74AC11014DW, 74AC11014DWR, SN74AHC14MDREP, SN74AHC2G04HDCTR, SN74AHC2G04HDCU3 | 74AC11014 hex Schmitt inverter; SN74AHC2G04 (H) dual inverter; SN74AHC14-EP | `gate` / Logic → NOT | No TI datasheet available (ti.com/lit returns no PDF; DigiKey links only a Rochester viewer page), so no pinout, timing or package drawing. |
| SN74LVC1G04YZVR, SN74LVC1G14YZVR, SN74LVC1GU04YZVR | 1G04 / 1G14 / 1GU04 in 4-ball DSBGA (YZV) | `gate` / Logic → NOT | No YZV0004 package drawing in any datasheet (sn74lvc1g04/1g14/1gu04 name the package but do not draw it), so no footprint. |
| CDC203DW, CDC203DWR | CDC203 3.3 V hex inverter / clock driver | `gate` / Logic → NOT | Clock-driver part with no logic family in the catalog list; CDC203DWR is also missing from the ordering table. |
| 74ACT11004DBR, 74ACT11004DWR, 74ACT11004PWR, CD4049UBDWR, CD4069UBPWG4, CD74HCT14PW, CD74HCU04MT, CD74HCU04QPWRQ1, JM38510/65701SCA, JM38510/65701SDA, SN7404NSR, SN7406DG4, SN7416NS, SN74AC04NS, SN74ACT04NS, SN74ACT14DBLE, SN74ACT14DRE4, SN74ACT14NS, SN74ACT14PWRQ1, SN74ACT16PWRQ1, SN74ACT16WBQARQ1, SN74AHC04QDR, SN74AHC04QPWRQ1, SN74AHC05NS, SN74AHC05NSR, SN74AHC05PWE4, SN74AHCT14NS, SN74AHCU04DGVR, SN74AHCU04NS, SN74AHCU04QPWRQ1, SN74AHCU04RGYR, SN74ALS04BDB, SN74ALS04BDE4, SN74ALS04BNS, SN74ALS05ADB, SN74ALS05ADBR, SN74ALS1004DR, SN74ALS1004NS, SN74ALS1005NS, SN74ALVC04DE4, SN74ALVC04NSR, SN74ALVC04PWE4, SN74AUC1G04YEAR, SN74AUC1G04YEPR, SN74AUC1G04YZAR, SN74AUC1G06YEAR, SN74AUC1G14YEPR, SN74AUC1GU04YEAR, SN74AUC1GU04YEPR, SN74AUC2G04YEPR, SN74AUC2G06YEPR, SN74AUC2GU04YZPR, SN74AUP1G04DCKTE4, SN74AUP2G06YFPR, SN74HC04IPWRG4Q1, SN74HC04IPWRQ1, SN74HC05DBR, SN74HC05NS, SN74HC05PW, SN74HC14DE4, SN74HC14N-NG, SN74HCT04DBR, SN74HCT04DE4, SN74HCU04DBR, SN74HCU7204PWR, SN74HCU7204PWT, SN74LS04DB, SN74LS04NS, SN74LS05NS, SN74LS06NS, SN74LS14NS, SN74LS19AD, SN74LS19ADB, SN74LS19ADBR, SN74LV04ANS, SN74LV04APW, SN74LV04DBR, SN74LV05ANS, SN74LV14APWT, SN74LVC04ANS, SN74LVC06ANS, SN74LVC14ANS, SN74LVC1G04MDBVREP, SN74LVC1G04QDBVRVS, SN74LVC1G04YEAR, SN74LVC1G04YEPR, SN74LVC1G04YZAR, SN74LVC1G04YZTR, SN74LVC1G06DBVTE4, SN74LVC1G06MDCKREP, SN74LVC1G06YEAR, SN74LVC1G06YZAR, SN74LVC1G14MDBVREP, SN74LVC1G14YEAR, SN74LVC1G14YEPR, SN74LVC1G14YZAR, SN74LVC1GU04DCKRQ1, SN74LVC1GU04YEAR, SN74LVC1GU04YZTR, SN74LVC2G04DCK3, SN74LVC2G04YEAR, SN74LVC2G04YEPR, SN74LVC2G04YZAR, SN74LVC2G06YEAR, SN74LVC2G06YEPR, SN74LVC2G14DCK3, SN74LVC2G14YEAR, SN74LVC2G14YEPR, SN74LVC2G14YZAR, SN74LVC2GU04YEAR, SN74LVC3G04DCT3, SN74LVC3G04YEAR, SN74LVC3G06YEAR, SN74LVC3G06YEPR, SN74LVC3G06YZAR, SN74LVC3G14YEAR, SN74LVC3G14YEPR, SN74LVC3G14YZAR, SN74LVC3GU04YEPR, SN74LVC3GU04YZPR, SN74LVCU04ANSR, SN74LVCU04DBLE, SN74LVCU04DR, SN74LVCU04PWLE, SN74LVU04ADGVR, SN74LVU04AQPWRQ1, SN74LVU04ARGYR, SN74S04DG4 | TI inverters (04, 05, 06, 14, U04, 1G/2G/3G and others) | `gate` / Logic → NOT | In stock at DigiKey but not in the current ordering table of their datasheet (older packing variants such as E4/G4/LE/NS without R, YEA/YEP/YZA/YZT NanoStar/NanoFree packages, DigiKey-only codes), so their package is not documented. |
