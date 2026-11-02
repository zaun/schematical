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
