# Datasheets not included

Datasheet PDFs the owner chose to leave out of the catalog (not because they
are obsolete; see `obsolete-not-included.md` for those). The PDF is deleted
and listed here instead.

| PDF | Family | Category / group | Notes |
|-----|--------|------------------|-------|
| `counters/cd4060b.pdf` (CD4060B) | counter | Logic → Counter | 14-stage ripple counter with on-chip RC/crystal oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/CD4060B |
| `counters/cd74hc4060.pdf` (CD74HC4060, CD74HCT4060) | counter | Logic → Counter | 14-stage ripple counter with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/CD74HC4060 |
| `counters/sn74hc4060.pdf` (SN74HC4060) | counter | Logic → Counter | 14-stage ripple counter with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/SN74HC4060 |
| `counters/cd4045b.pdf` (CD4045B) | counter | Logic → Counter | 21-stage counter with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/CD4045B |
| `divider/cd4521b.pdf` (CD4521B) | counter | Logic → Divider | 24-stage frequency divider with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/CD4521B |
| `divider/cd4536b.pdf` (CD4536B) | counter | Logic → Divider | Programmable timer with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/CD4536B |
| `divider/cd4541b.pdf` (CD4541B) | counter | Logic → Divider | Programmable timer with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/CD4541B |
| `divider/sn74ls292.pdf` (SN74LS292, SN74LS294) | counter | Logic → Divider | Programmable frequency dividers with on-chip oscillator; left out (owner, 2026-11-01); https://www.ti.com/product/SN74LS292 |
| `shifters/sn74abt8996.pdf` (SN74ABT8996) | shift | Logic → Shift register | IEEE 1149.1 addressable scan port; no use in the simulator (owner, 2026-11-01); https://www.ti.com/product/SN74ABT8996 |
| `latch/sn74lvc1g373b-q1.pdf` (SN74LVC1G373B-Q1) | latch | Logic → Latch | No orderable part numbers yet (product preview, SLVSN16, Aug 2026); https://www.ti.com/product/SN74LVC1G373B-Q1 |
| `osc/ECOC-2522.pdf` (ECOC-2522) | oscillator | Oscillator | OCXO with a voltage-control pin (pin 1 VC, pin 2 V Ref), so in effect a VCXO; VCXOs left out (owner, 2026-11-01); https://ecsxtal.com/store/pdf/ECOC-2522.pdf |
| `module-disp-char/LCM-X01602DXX-D.pdf` (Lumex LCM-S01602DSR/D) | charlcd | Displays → Character | 1998 drawing sheet: no controller, font, AC timing or execution times; left out (owner, 2026-11-01) |
| `module-disp-char/LCM-x01602Dxx+A-page1[1].pdf` (Lumex LCM-S01602DTR/A) | charlcd | Displays → Character | Page 1 of 2 only (no pinout or electrical data); "14-Ø1.00" callout contradicts its 16-hole drawing; left out (owner, 2026-11-01) |
| `controller-uart/ch432.pdf` (CH432) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CH432` finds none); plan-12-A §4.1; https://www.wch-ic.com/products/CH432.html |
| `controller-uart/ch438.pdf` (CH438) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CH438` finds none); plan-12-A §4.1; https://www.wch-ic.com/products/CH438.html |
| `controller-uart/ch9432.pdf` (CH9432) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CH943` finds none); plan-12-A §4.1; https://www.wch-ic.com/products/CH9432.html |
| `controller-uart/ch9434.pdf` (CH9434) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CH943` finds none); plan-12-A §4.1; https://www.wch-ic.com/products/CH9434.html |
| `controller-uart/ch9437.pdf` (CH9437) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CH943` finds none); plan-12-A §4.1; https://www.wch-ic.com/products/CH9437.html |
| `controller-uart/ch9438.pdf` (CH9438) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CH943` finds none); plan-12-A §4.1; https://www.wch-ic.com/products/CH9438.html |
| `controller-uart/wk2124.pdf` (WK2124) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `WK2` finds none); plan-12-A §4.1; http://www.wkmic.com/ |
| `controller-uart/wk2132.pdf` (WK2132) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `WK2` finds none); plan-12-A §4.1; http://www.wkmic.com/ |
| `controller-uart/wk2168.pdf` (WK2168) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `WK2` finds none); plan-12-A §4.1; http://www.wkmic.com/ |
| `controller-uart/wk2204.pdf` (WK2204) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `WK2` finds none); plan-12-A §4.1; http://www.wkmic.com/ |
| `controller-uart/wk2212.pdf` (WK2212) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `WK2` finds none); plan-12-A §4.1; http://www.wkmic.com/ |
| `controller-uart/pi7c9x1170.pdf` (PI7C9X1170) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `PI7C9X11` finds only PI7C9X112/113, other chips); plan-12-A §4.1; https://www.diodes.com/part/view/PI7C9X1170 |
| `controller-uart/pi7c9x1172.pdf` (PI7C9X1172) | serial | Interface → Bridge UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `PI7C9X11` finds only PI7C9X112/113, other chips); plan-12-A §4.1; https://www.diodes.com/part/view/PI7C9X1172 |
| `controller-uart/sc16c550b.pdf` (SC16C550B) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `SC16C550` finds none); plan-12-A §4.1; https://www.nxp.com/part/SC16C550B |
| `controller-uart/sc28l202.pdf` (SC28L202) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `SC28L202` finds none); plan-12-A §4.1; https://www.nxp.com/part/SC28L202 |
| `controller-uart/xr16l2550.pdf` (XR16L2550) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: absent from the in-stock UART category list); plan-12-A §4.1; https://www.maxlinear.com/ |
| `controller-uart/xr16m2750.pdf` (XR16M2750) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: absent from the in-stock UART category list); plan-12-A §4.1; https://www.maxlinear.com/ |
| `controller-uart/xr16v598.pdf` (XR16V598) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: absent from the in-stock UART category list); plan-12-A §4.1; https://www.maxlinear.com/ |
| `controller-uart/xr16v698.pdf` (XR16V698) | serial | Interface → UART | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: absent from the in-stock UART category list); plan-12-A §4.1; https://www.maxlinear.com/ |
| `controller-i2c/cp2120.pdf` (CP2120) | serial | Interface → I2C controller | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `CP2120` finds only MCP2120); plan-12-A §4.1; https://www.silabs.com/ |
| `controller-spi/sc18is606.pdf` (SC18IS606) | serial | Interface → I2C controller | No part in stock at DigiKey (2026-11-02, `node scripts/digikey.js --in-stock`: search `SC18IS` finds only SC18IS604); plan-12-A §4.1; https://www.nxp.com/part/SC18IS606 |
| `controller-uart/com8017.pdf` (COM8017, COM8502) | serial | Interface → UART | No in-stock part has a known package with a pinout in the PDF (`planning/plan-12-data/parts.tsv` reasons); plan-12-A §4.1; https://www.microchip.com/ |
| `controller-uart/m66230.pdf` (M66230) | serial | Interface → UART | No package drawing for Renesas 24P2W-A can be found (`planning/plan-12-data/footprints.md` § SOP-24-24P2W); plan-12-A §4.1; https://www.renesas.com/ |
| `controller-uart/tl16c2752.pdf` (TL16C2752) | serial | Interface → UART | The datasheet names the extended registers but does not describe their bits, so it cannot be modeled fully (`planning/plan-12-data/specs/ti-c.md` § TL16C2752); plan-12-A §4.1; https://www.ti.com/product/TL16C2752 |
