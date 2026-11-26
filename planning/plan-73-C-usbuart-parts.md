# plan-73-C: USB-UART bridges — the 16 parts, goldens, skipped parts, landing

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-73-A (the new model keys), plan-73-B (the datasheets and footprints)

C adds 10 `usbuart` devices with 16 parts to `schematical/schematical/Parts/serial.json`,
transcribed from the PDFs B placed:

- Silicon Labs: CP2102, CP2102N (two devices, by package), CP2102C, CP2104,
  CP2105, CP2108, and CP2110 (two devices, by package).
- WCH: CH340T.

It also does the following:

- regenerates the goldens;
- records the parts left out in `datasheets/skipped-parts.md`;
- updates the docs' device list;
- runs the landing tier once and archives plan-73.

Outcome of C:

- `listing_check` lists P0 + 16 parts. 16 more `|serial|` lines; 247 at planning
  (`grep -c '|serial|' backend/tests/golden/parts.txt` → 247).
- `trace_check` and `erc_pins_check` have 16 more lines each.
- `usbuart_test` runs the new behaviors on the shipped parts:
  - a CP2102N loopback;
  - CH340T TNOW and R232;
  - CP2105's two channels;
  - CP2110's RS-485 pin;
  - the CP2108's three supply domains in the voltage check.
- `skipped-parts.md` has a `## USB-UART bridges (plan-73)` section.

References: plan-73-A (§ Prerequisites, § 3, the model keys), plan-73-B (§ 3, the
footprint ids), `planning/plan-73-data/README.md` (the parts table, the stock
numbers, the datasheet pages), `docs/parts-format.md` § Pinouts and pins (line
150), § Parts (line 238), § `serial`, § `usbuart`; the existing bridges in
`serial.json` (device `CY7C65213`, `XR21B1411`: note, grades and `usbSupply` style).

## Prerequisites

plan-73-A § Prerequisites, all rows. C adds:

| Must be true | Command | Status |
|---|---|---|
| plan-73-A complete | `grep -c '"txEnable"\|"rxInvert"\|"configured"' backend/src/models/serial_usbuart.c` → ≥ 3 and `grep -c '"auxSupply"' backend/src/models/serial.c` → ≥ 1 | NOT MET at planning (A not built) |
| plan-73-B complete | `ls datasheets/controller-uart/{cp2102-9,cp2102n,cp2102c,cp2104,cp2105,cp2108,cp2110,ch340}.pdf \| wc -l` → 8 and `for id in QFN-20-3X3-SL QFN-24-4X4-SL-N QFN-24-4X4-SL QFN-28-5X5-SL QFN-28-5X5-SL-C QFN-64-9X9-SL; do grep -c "\"id\": *\"$id\"" schematical/schematical/Parts/footprints.json; done` → six 1s | NOT MET at planning (B not built) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The 10 devices of § 3 are in `serial.json` with the 16 parts of the data
  README's table, each `"manufacturer"`, `"datasheet"`, `"package": "QFN"` (CH340T:
  `"SSOP"`), `"pinout"`, `"footprint"` and `"grade": "I"`.
- Before adding, each part is in stock at DigiKey: In Stock above 0, looked up
  in Safari per AGENTS.md.
- Goldens regenerated: parts, traces and erc_pins each + 16 (+ the count of any
  Phase 1 drop subtracted), footprints unchanged.
- `usbuart_test` new cases pass on the shipped parts.
- `datasheets/skipped-parts.md` § USB-UART bridges (plan-73) lists the rows of the
  data README's "Rows with no stock" table.

### Non-goals

- No new model behavior. Anything the parts need that A's keys do not give is a
  Corrections entry and a fix in A's style, not new scope.
- GPIO pins are not mapped where their default function is a plain GPIO
  (CP2102N, CP2102C, CP2104, CP2105, CP2108, and CP2110's GPIO.0/6–9). This
  mirrors `XR21B1411` and `MCP2200` (their notes: alternates enabled by
  configuration "are not mapped").
- No FTDI part, and no CH340 other than CH340T (plan-73-A § Decisions).

## 2. Current State

- `serial.json` has 90 devices and 247 parts, 10 of them `usbuart` devices with
  25 parts (plan-73-A § 2, Measured populations).
- Every `usbuart` device uses drawer `["Interface", "Serial"]` and function
  `"Bridge UART"`. Their notes give the PDF, revision and the pages of the
  pinout, the defaults and the DC table.
- `CY7C65213` and `XR21B14xx` carry `usbSupply` for their USB-side regulator input.

## 3. The devices

Conventions for every device below:

- Pin names are the datasheet's. A combined pin is `A/B`, as `XR21B1411`'s
  `GPIO5/RTSB/RS485`. An overbarred name gets a `b` suffix, as in the CP2102N
  datasheet's own `SUSPENDb`/`RSTb`; WCH writes `#`.
- Roles: UART and modem inputs, VBUS, NOS#, XI and R232 are `input`. TXD, RTS,
  DTR, SUSPEND, SUSPENDb, ACT#, TNOW, XO and CKO are `output`. D+/D−, GPIO and
  RST are `inout` (the datasheets' "D I/O"). VDD, VIO, VIOHD, VREGIN/REGIN, V3
  and VPP are `power` (VPP: the ROM-programming capacitor pin, as `CY7C65213`'s
  `VCCD`). GND/VSS/VSSHD are `ground`, and NC is `nc`.
- Groups: `serial` for UART/modem pins, `usb` for D+/D−/VBUS, `crystal` for XI/XO.
- `activeLow`: RTS, CTS, DTR, DSR, DCD, RI, SUSPENDb, RST/RSTb/RESET, ACT#, NOS#,
  and every `#` name.
- Every QFN's centre GND is the footprint's unnumbered centre pad. No pinout row
  names it, as in `XR21B1411`'s note.
- Grade `"I"` (every datasheet: −40 to +85 °C). The grade `vcc` is the I/O supply.
  `usbSupply` holds the USB-side supply pins (plan-73-A § 3).

| Device | Pinout (pins) | Channels (`txd`, `rxd`, …) | Device keys | Grade `vcc` | `usbSupply` | Parts → footprint |
|---|---|---|---|---|---|---|
| `CP2102` | `qfn28` (`cp2102-9.pdf` p.11 Table 9): 1 DCD, 2 RI, 3 GND, 4 D+, 5 D−, 6 VDD, 7 REGIN, 8 VBUS, 9 RST, 10 NC, 11 SUSPENDb, 12 SUSPEND, 13–22 NC, 23 CTS, 24 RTS, 25 RXD, 26 TXD, 27 DSR, 28 DTR | one: TXD, RXD, rts RTS, cts CTS, dtr DTR, dsr DSR, dcd DCD, ri RI | reset RST, suspend SUSPEND, suspendLow SUSPENDb | [3.0, 3.6] (VDD, p.7; no VIO) | power [REGIN], pins [D+, D−, VBUS], vcc [4.0, 5.25] (p.9 regulator table, CP2102 row) | CP2102-GMR → `QFN-28-5X5-SL` |
| `CP2102N` | `qfn24` (`cp2102n.pdf` p.29–30 Table 5.2): 1 RI/CLK, 2 GND, 3 D+, 4 D−, 5 VIO, 6 VDD, 7 VREGIN, 8 VBUS, 9 RSTb, 10 NC, 11 GPIO.3/WAKEUP, 12 GPIO.2/RS485, 13 GPIO.1/RXT, 14 GPIO.0/TXT, 15 SUSPENDb, 16 NC, 17 SUSPEND, 18 CTS, 19 RTS, 20 RXD, 21 TXD, 22 DSR, 23 DTR, 24 DCD | one: TXD, RXD, RTS, CTS, DTR, DSR, DCD, ri RI/CLK | reset RSTb, suspend SUSPEND, suspendLow SUSPENDb | [1.71, 3.6] (VIO, p.10) | power [VDD, VREGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] (p.10) | CP2102N-A02-GQFN24, CP2102N-A02-GQFN24R → `QFN-24-4X4-SL-N` |
| `CP2102N-QFN20` | `qfn20` (p.31–32 Table 5.3): 1 GPIO.1/RS485, 2 GPIO.0/CLK, 3 GND, 4 D+, 5 D−, 6 VDD, 7 VREGIN, 8 VBUS, 9 RSTb, 10 NC, 11 SUSPENDb, 12 GND, 13 WAKEUP, 14 SUSPEND, 15 CTS, 16 RTS, 17 RXD, 18 TXD, 19 GPIO.3/RXT, 20 GPIO.2/TXT | one: TXD, RXD, RTS, CTS | reset RSTb, suspend SUSPEND, suspendLow SUSPENDb | [3.0, 3.6] (VDD; the QFN20 has no VIO pin) | power [VREGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] | CP2102N-A02-GQFN20, CP2102N-A02-GQFN20R → `QFN-20-3X3-SL` |
| `CP2102C` | `qfn28` (`cp2102c.pdf` p.18 Table 5.1): 1 DCD, 2 RI, 3 GND, 4 D+, 5 D−, 6 VDD, 7 VREGIN, 8 VBUS, 9 RSTb, 10 NC, 11 SUSPENDb, 12 SUSPEND, 13–22 NC, 23 CTS, 24 RTS, 25 RXD, 26 TXD, 27 DSR, 28 DTR | one: full modem set as `CP2102` | reset RSTb, suspend SUSPEND, suspendLow SUSPENDb | [3.0, 3.6] (VDD, p.8; the QFN28 has no VIO pin) | power [VREGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] (p.8) | CP2102C-A01-GQFN28R → `QFN-28-5X5-SL-C` |
| `CP2104` | `qfn24` (`cp2104.pdf` p.8 Table 7): 1 RI, 2 GND, 3 D+, 4 D−, 5 VIO, 6 VDD, 7 REGIN, 8 VBUS, 9 RST, 10 NC, 11 GPIO.3, 12 GPIO.2, 13 GPIO.1, 14 GPIO.0, 15 SUSPENDb, 16 VPP, 17 SUSPEND, 18 CTS, 19 RTS, 20 RXD, 21 TXD, 22 DSR, 23 DTR, 24 DCD | one: full modem set | reset RST, suspend SUSPEND, suspendLow SUSPENDb | [1.8, 3.6] (VIO, p.5) | power [VDD, REGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] (p.6 regulator table) | CP2104-F03-GM, CP2104-F03-GMR → `QFN-24-4X4-SL` |
| `CP2105` | `qfn24` (`cp2105.pdf` p.7–8 Table 7, p.9 Figure): 1 SUSPENDb/RI_SCI, 2 GND, 3 D+, 4 D−, 5 VIO, 6 VDD, 7 REGIN, 8 VBUS, 9 RST, 10 CTS_ECI, 11 RTS_ECI, 12 RXD_ECI, 13 TXD_ECI, 14 GPIO.1_ECI/DSR_ECI, 15 GPIO.0_ECI/DTR_ECI, 16 VPP/DCD_ECI, 17 SUSPENDb/RI_ECI, 18 CTS_SCI, 19 RTS_SCI, 20 RXD_SCI, 21 TXD_SCI, 22 GPIO.2_SCI/DSR_SCI, 23 GPIO.1_SCI/DTR_SCI, 24 GPIO.0_SCI/DCD_SCI | two (ECI first, the datasheet's interface order, p.18 Table 13): [TXD_ECI, RXD_ECI, rts RTS_ECI, cts CTS_ECI, suspendLow SUSPENDb/RI_ECI], [TXD_SCI, RXD_SCI, rts RTS_SCI, cts CTS_SCI, suspendLow SUSPENDb/RI_SCI] | reset RST | [1.8, 3.6] (VIO) | power [VDD, REGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] | CP2105-F01-GM, CP2105-F01-GMR → `QFN-24-4X4-SL` |
| `CP2108` | `qfn64` (`cp2108.pdf` p.12–14 Table 4.1): 1 RX3, 2 VSSHD, 3 VIOHD, 4 TX3, 5 CTS3, 6 RTS3, 7 RI3, 8 DCD3, 9 DSR3, 10 DTR3, 11 RI2, 12 DCD2, 13 CTS2, 14 RTS2, 15 RX2, 16 TX2, 17 DSR2, 18 DTR2, 19 SUSPENDb, 20 SUSPEND, 21 GPIO.15, 22 GPIO.14, 23 GPIO.13, 24 VIO, 25 VSS, 26 GPIO.12, 27 GPIO.11, 28 GPIO.10, 29 GPIO.9, 30 GPIO.8, 31 GPIO.7, 32 GPIO.6, 33 GPIO.5, 34 GPIO.4, 35 NC, 36 NC, 37 GPIO.3, 38 GPIO.2, 39 VIO, 40 GPIO.1, 41 GPIO.0, 42 RI1, 43 DCD1, 44 DSR1, 45 DTR1, 46 CTS1, 47 RTS1, 48 RX1, 49 TX1, 50 RI0, 51 DCD0, 52 DSR0, 53 DTR0, 54 CTS0, 55 RTS0, 56 RX0, 57 TX0, 58 VDD, 59 VSS, 60 VREGIN, 61 VBUS, 62 D+, 63 D−, 64 RESET | four, UART 0–3, each TXn, RXn, RTSn, CTSn, DTRn, DSRn, DCDn, RIn | reset RESET, suspend SUSPEND, suspendLow SUSPENDb, **auxSupply** power [VIOHD], pins [RX3, TX3, CTS3, RTS3], vcc [2.7, 6.0] (p.6; Table 4.1 "High Drive") | [3.0, 3.6] (VIO, p.6) | power [VDD, VREGIN], pins [D+, D−, VBUS], vcc [3.0, 5.5] (p.6: VDD 3.0–3.6, VREGIN 4.0–5.5) | CP2108-B03-GM, CP2108-B03-GMR → `QFN-64-9X9-SL` |
| `CP2110` | `qfn24` (`cp2110.pdf` p.8–9 Table 7, column –GM): 1 GPIO.0/CLK, 2 GND, 3 D+, 4 D−, 5 VIO, 6 VDD, 7 REGIN, 8 VBUS, 9 RST, 10 NC, 11 SUSPEND, 12 GPIO.9, 13 GPIO.8, 14 GPIO.7, 15 GPIO.6, 16 VPP, 17 SUSPENDb, 18 GPIO.5/RXT, 19 GPIO.4/TXT, 20 RX, 21 TX, 22 GPIO.3/RS485, 23 GPIO.2/CTS, 24 GPIO.1/RTS | one: TX, RX, rts GPIO.1/RTS, cts GPIO.2/CTS, txEnable GPIO.3/RS485, txLed GPIO.4/TXT, rxLed GPIO.5/RXT (the p.18 Table 14 defaults) | reset RST, suspend SUSPEND, suspendLow SUSPENDb | [1.8, 3.6] (VIO, p.5) | power [VDD, REGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] (p.7) | CP2110-F01-GM, CP2110-F01-GMR → `QFN-24-4X4-SL` |
| `CP2110-QFN28` | `qfn28` (same table, column –GM1): 1 GPIO.3/RS485, 2 GPIO.0/CLK, 3 GND, 4 D+, 5 D−, 6 VDD, 7 REGIN, 8 VBUS, 9 RST, 10 GPIO.9, 11 SUSPENDb, 12 SUSPEND, 13–15 NC, 16 GPIO.8, 17 GPIO.7, 18 VPP, 19 GPIO.6, 20–22 NC, 23 GPIO.2/CTS, 24 GPIO.1/RTS, 25 RX, 26 TX, 27 GPIO.5/RXT, 28 GPIO.4/TXT | as `CP2110` | as `CP2110` | [3.0, 3.6] (VDD; p.8: VIO "Internally connected to VDD on –GM1 packages") | power [REGIN], pins [D+, D−, VBUS], vcc [3.0, 5.25] | CP2110-F02-GM1 → `QFN-28-5X5-SL` |
| `CH340T` | `ssop20` (`ch340.pdf` p.3–4 § 4, SSOP20 column): 1 CKO, 2 ACT#, 3 TXD, 4 RXD, 5 V3, 6 UD+, 7 UD−, 8 GND, 9 XI, 10 XO, 11 CTS#, 12 DSR#, 13 RI#, 14 DCD#, 15 DTR#, 16 RTS#, 17 TNOW, 18 R232, 19 VCC, 20 NOS# | one: TXD, RXD, rts RTS#, cts CTS#, dtr DTR#, dsr DSR#, dcd DCD#, ri RI#, txEnable TNOW | configured ACT#, rxInvert R232 (no reset pin: power-on reset only, p.5) | [2.9, 5.3] (p.8–9: VCC 4.0–5.3 V at 5 V with V3 decoupled; 2.9–3.6 V at 3.3 V with V3 tied to VCC) | none (one supply) | CH340T (`"manufacturer": "WCH"`, `"package": "SSOP"`) → `SSOP-20` |

Facts for the notes (one note per device, in the existing notes' style):

- **CP2102N** (p.19): "By default, all of the GPIO pins are configured
  as a GPIO input", so TXT/RXT/RS485/CLK/WAKEUP are not mapped. CP2102C likewise.
- **CP2104** (p.16 Table 13): GPIO.0–3 default to GPIO input. **CP2105** (p.14):
  both interfaces default to GPIO Mode. Their modem pins are GPIO, and only
  TXD/RXD/RTS/CTS and the suspend pins are mapped. Its SUSPEND pins "default to
  active-low" (p.7), hence `suspendLow`.
- **CP2108**: GPIO.0–15 are GPIO by default, with alternates TX/RX toggle, RS-485
  and clock (p.14), not mapped. UART3's four high-drive pins run from VIOHD.
- **CP2110** (p.18 Table 14): GPIO.1 RTS, GPIO.2 CTS, GPIO.3 RS-485 (active-high,
  p.20), GPIO.4 TX Toggle and GPIO.5 RX Toggle by default. GPIO.8/9 default to
  push-pull outputs whose level the host sets, so they are not mapped (never
  driven). The TX/RX toggles are mapped to `txLed`/`rxLed`, as `CY7C65213`'s
  TXLED#.
- **CH340T**: DigiKey sells it under Olimex's number `SMD IC CH340T` (In Stock 4,
  2026-11-25). The part's name is WCH's orderable number `CH340T`.
  - p.5: "If the R232 pin is high … an inverter is automatically inserted inside
    the RXD pin"; p.6: R232 is "only checked once after power-on reset".
  - p.6: TNOW is high while the serial port is sending. XI/XO take a 12 MHz
    crystal, and the model runs on its own clock (`docs/parts-format.md`
    § usbuart). CKO and XO are never driven. NOS# (suspend disable) is read by
    nothing, because the model's link never suspends.

## Phases

> **NOTE — keep the checkboxes current as you go** (as in plan-73-A).

### Phase 1 — stock re-check

AGENTS.md: a part is added only while DigiKey shows it in stock, and stock is
looked up in Safari. The planning numbers are from 2026-11-25.

- [ ] Load the DigiKey Controllers page filtered "Bridge, USB to UART", In Stock +
  Active, through Safari:
  `LZ=/tmp/lz/node_modules planning/plan-69-data/census/facets.sh /tmp/p73-usbuart.json x/753 '{"110":[323050]}'`
  (`facets.sh` explains its setup). Every Silicon Labs row of the data README
  must still be listed with In Stock above 0.
- [ ] CP2102-GMR and CH340T (keyword probes, `planning/plan-69-data/census/kw.js`
  as in its README) → In Stock above 0.
- [ ] A row now at 0 is not added. Record it in Phase 4's `skipped-parts.md`
  section with the date, and subtract it from the counts in § 1, Phase 3 and
  Phase 5. Record the result here.

Acceptance: each of the 16 rows has a fresh In Stock number, or a recorded drop.
  Check: the two commands above, and the 16 numbers written into this phase (est. 5 min).
Commit: — (no files change unless a row drops; the numbers go in with Phase 2's commit)

### Phase 2 — the 10 devices and 16 parts

- [ ] `schematical/schematical/Parts/serial.json`: add the 10 devices of § 3 after
  the last `usbuart` device, in § 3's order.
  - Each has `"id"`, `"function": "Bridge UART"`, a `"description"` from the
    datasheet's title line, `"drawer": ["Interface", "Serial"]`, `"note"` (§ 3's
    facts plus the PDF revision and pages), `"pinouts"`, `"model": "usbuart"`,
    `"channels"` and the device keys.
  - `"grades": {"I": {"vcc": …, "note": …}}`, `"usbSupply"` (and `"auxSupply"`
    on CP2108), and `"parts"`.
- [ ] Each part: `"name"`, `"manufacturer"` (`"Silicon Labs"` / `"WCH"`),
  `"datasheet"` (the B id), `"package"`, `"pinout"`, `"footprint"` and
  `"grade": "I"`.

Acceptance: the library loads and lists exactly the new parts.
  Check: `cmake --build /tmp/logic-p73 && /tmp/logic-p73/listing_check --print | grep -c -E '^(CP210|CH340T)'` → 16 (est. 2 min); `/tmp/logic-p73/listing_check --print | grep '^CP2108-B03-GM|' | grep -c 'vccc=2700-6000'` → 1.
Commit: —

### Phase 3 — tests on the shipped parts and goldens

- [ ] `backend/tests/usbuart_test.c`:
  - `case_facts`: CP2108-B03-GMR has 4 channels and `domain_count == 3`; CP2105-F01-GM
    has 2 channels; CH340T has host `usb` and `domain_count == 0`.
  - New case `case_cp2102n`: on CP2102N-A02-GQFN24 with VIO and VDD on 3.3 V,
    VREGIN on 5 V and TXD looped to RXD, the terminal sends `"hi"` and the sent log
    reads `"hi"`, as `case_loopback` does for the existing parts.
  - New case `case_ch340t`: on 5 V, `send` one byte → TNOW (pin 17) 1 during the
    frame and 0 after it; ACT# (pin 2) 0. With R232 (pin 18) tied to VCC and TXD
    looped to RXD through an `SN74HC04N` gate (as plan-73-A's `case_rx_invert`),
    `"A"` comes back as `0x41`.
  - New case `case_cp2105_cp2110`: on CP2105-F01-GM, pins 1 and 17 read 1. On
    CP2110-F01-GM, GPIO.3/RS485 (pin 22) is 1 while a frame goes out.
  - `case_usb_supply` grows a CP2108 block: 3.3 V on VIO/VDD, 5 V on VREGIN and on
    VIOHD → starts. 5 V on VIO → refused with `voltage`.
- [ ] Regenerate `parts.txt`, `traces.txt` and `erc_pins.txt` (`docs/backend.md`
  line 904 and line 918 commands). `git diff --stat backend/tests/golden` → three
  files, +16 lines each, 0 removed; `footprints.txt` unchanged.
- [ ] Review the 16 `erc_pins.txt` lines against § 3's roles, using the table in
  `docs/backend.md` § ERC pin types (line 559) and the letters `i o b t p f u P O c e n`
  in `ErcPinType` order (`backend/include/erc.h:41`):
  - `input` → `i`; `output` → `o`, because a `usbuart` part has no 3-state pin
    (`docs/backend.md` line 605: only the `ns16550` TRI pin is);
  - `inout` → `b`; `power`/`ground` → `P`; `nc` → `n`;
  - `-` for nothing, since no pin is `absent`.

Acceptance: `usbuart_test` passes on the shipped parts, and the three goldens grow by exactly the new parts.
  Check: `/tmp/logic-p73/usbuart_test` → `all passed`; `ctest --test-dir /tmp/logic-p73 -R 'usbuart|listing|trace|erc_pins|footprint'` → 100% passed (est. 2 min).
Commit: —

### Phase 4 — skipped parts and docs

- [ ] `datasheets/skipped-parts.md`: a new section `## USB-UART bridges (plan-73)`
  with the table header of the file's other sections (`Part(s) | Device | Would
  have gone into | Why skipped`). It holds the rows of the data README's "Rows with
  no stock" table: FTDI ×18, CP2102N QFN28 ×2, CH340X, CH340G/C/N/K/E/B. Each row
  is `serial` / Interface → Serial, with its reason and date 2026-11-25.
- [ ] `docs/parts-format.md` § `usbuart` first sentence: the device list reads
  "(CP2102, CP2102N, CP2102C, CP2104, CP2105, CP2108, CP2110, CH340T,
  CY7C64225/65213, MCP2200, XR21B14xx, XR21V141x)".
- [ ] `docs/backend.md` § `usbuart_test` (line 1310): add the four shipped-part
  cases.

Acceptance: the docs and the skipped list name the new parts and the left-out ones.
  Check: `grep -c 'CH340T' docs/parts-format.md` → ≥ 1; `grep -c 'plan-73' datasheets/skipped-parts.md` → ≥ 1; `grep -c 'FT232RL' datasheets/skipped-parts.md` → ≥ 1 (est. 1 s).
Commit: —

### Phase 5 — landing tier and close

- [ ] Backend: `cmake -S backend -B /tmp/logic-p73 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p73 -j8 && ctest --test-dir /tmp/logic-p73 -j8` → `100% tests passed`, with as many tests as at the start (77 at planning) (est. 3 min; 22.7 s ctest at planning plus the build).
- [ ] Datasheet URLs: `node scripts/check-datasheets.js cp2102-9 cp2102n cp2102c cp2104 cp2105 cp2108 cp2110 ch340` → exit 0 (est. 1 min).
- [ ] App tests: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p73-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/SerialPortTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/DrawerCategoriesTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/BillOfMaterialsTests` → `** TEST SUCCEEDED **` (est. 12 min, the app build included). These are the classes that read every shipped part (round trip, drawer, BOM) or the serial keys (drift, Port menus), and no smaller run covers them.
- [ ] Size and coverage rules (plan-68; owner, 2026-11-25): `node scripts/source-size.js --check && node scripts/coverage.js --check` → exit 0 (est. 30 min: the app tier is measured only by plan-68's full Debug-suite run, and nothing smaller shows the new Swift lines run).
- [ ] Record each result next to its line.
- [ ] Fill the `Commit:` lines of A, B and C, and move `plan-73-A/B/C` to
  `planning/completed/`. `planning/plan-73-data` stays.

Acceptance: every gate line ticked with its result; A–C archived.
  Check: the lines above; `ls planning | grep -c '^plan-73-[A-C]'` → 0 and `ls planning/completed | grep -c '^plan-73-[A-C]'` → 3 (est. 50 min in all).
Commit: —

## Validation Plan

- Tests: `usbuart_test` (fixtures from A and shipped parts from C), `listing_check`,
  `trace_check`, `erc_pins_check` and `footprint_check` over the new lines, and the
  app classes of Phase 5.
- Coverage: the 16 parts are inside the goldens: Phase 3's diff is +16 in each of
  the three files. Every new model key is run on a shipped part (CH340T:
  `txEnable`, `configured`, `rxInvert`; CP2105: channel `suspendLow`; CP2108:
  `auxSupply`; CP2110: `txEnable`).
- Runtime proof: `case_cp2102n` and `case_ch340t` run real frames through shipped
  parts with the terminal API the app's terminal uses.
- Doc sync: `docs/parts-format.md` § `usbuart`, `docs/backend.md` § `usbuart_test`,
  `datasheets/skipped-parts.md`.
- Final gate: Phase 5, once.

## Decisions

See plan-73-A § Decisions. C's author calls, none user-visible except as listed:

- CP2102N and CP2110 are each two devices, split by package. A pinout without VIO
  needs a different `usbSupply` power list, and `usbSupply` names must exist in
  every pinout of a device (`bind_usb_supply` → `kind_pins` fails on a missing
  name).
- VDD goes in `usbSupply` where a separate VIO exists, so the I/O domain is powered
  only by VIO. This differs from `XR21B1422`, whose `usbSupply` lists only
  `VCC_REG`.
- The CH340T part is named `CH340T` (WCH's number), not Olimex's SKU.

## Corrections

## Summary

The work is transcription: 10 pinouts from seven Silicon Labs sheets and one WCH
sheet. The risk is a wrong pin number. The `pdftotext` footnote merge on the
CP2102/9 table is called out in the data README, and § 3 gives every pin, read
from the pages. The new behaviors ride on A's keys, which C's cases exercise on
real parts.
