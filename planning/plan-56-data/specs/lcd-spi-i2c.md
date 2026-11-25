# plan-56-E data: I2C / SPI LCD segment drivers (PCF8553, PCA8553, BU91795, BU9796A)

Sources (PDF pages, `datasheets/lcd-drivers/`): `pcf8553.pdf` (NXP rev. 5, 28 Sep 2021), `pca8553.pdf` (NXP rev. 4,
1 Oct 2021), `bu91795muf-m-e.pdf` (ROHM rev. 002, 29 Nov 2024), `bu9796axxx-e.pdf` (ROHM rev. 004, 18 Dec 2020).
Page citations are `<pdf id> p.N`, N the PDF page. Devices: `planning/plan-56-data/results/lcd-spi-i2c.json` (`lcddriver`
kind, display driver `lcd`), bench `lcd_spi_i2c_test.c`.

`BU97960MUV-E2` (named in the plan and the brief) is **not** in `bu9796axxx-e.pdf` (`grep 97960` finds nothing). It has
its own ROHM sheet, `bu97960muv-e.pdf` (34 pages, not filed in `datasheets/`; URL
`https://fscdn.rohm.com/en/products/databook/datasheet/ic/driver/lcd_segment/bu97960muv-e.pdf`, answers with a PDF):
1/8 or 1/4 duty, COM0-COM7 (COM4-COM7 share pins with SEG18-SEG15), 15 x 8 bit DDRAM, EVR contrast register, VQFN28
5 x 5 mm. Eight backplanes are beyond the `lcd` driver (static to 1:4), so it is skipped (see the result file).

## What is modeled, what is not

| Chip | Modeled | Not modeled |
|---|---|---|
| PCF8553, PCA8553 | I2C (4 strap addresses) and SPI (3-wire SDIO, IFS) ports, register map with read-back and auto-increment, Software_reset 2Ch and the RST pin, Display_ctrl_1 DE (display enable / power-down), the 20-byte display RAM and its COM x SEG layout, backplane and segment waveforms | the multiplex mode (MUX is stored; the pins always run a four-backplane frame, which shows a 1:3, 1:2 or static glass wired to COM0.. correctly because the RAM layout does not change with the mode), frame frequency FF (frame fixed at the 64 Hz reset value), BL blinking, INV, B bias, BOOST, OSC / COE and the CLK pin (an external clock is never required), PORE (power-on reset is always on), floating A0 / A1 straps (the sheet: internal pull-downs read 0; the engine has no tie for an open pin) |
| BU91795, BU9796A | I2C write port (7-bit address 0x3E), command byte (C bit) chain, ADSET with auto-increment every 4 bits, MODESET display on/off (BU9796A: bias bit stored), software reset (ICSET P1), all-pixel-off (APCTL P0), DDRAM and its COM x SEG layout, waveforms (fixed 1/4 duty, so no mode field is needed) | all-pixel-on (APCTL P1 is stored; every cell cannot be forced lit), blinking, DISCTL (frame frequency fixed at 80 Hz, inversion, power save: stored), oscillator mode (stored), the RAM address wrap 13h to 00h (the engine's pointer stays on the last register), RAM data after a non-ADSET command with C = 0 (dropped), I2C reads (the sheets define none) |

Not modeled anywhere: LCD bias voltages, contrast, VLCD, drive waveform shape (the engine is digital), brightness (none),
the IBIAS / VREF and TEST pins. The `lcd` driver releases (Z) every backplane and segment pin while the display is
disabled; the chips drive them to VSS (pcf8553 p.10, bu91795 p.13 / p.20). A glass reads both as unlit.

## Display RAM and the `lcd` driver

The driver drives backplane k in slot k of a frame (slot = frame / 4) and every segment pin with its RAM bit XOR the
frame level; a cell is lit when its segment pin and its backplane pin are both driven and differ.

* PCF8553 / PCA8553: 20 byte registers at 04h-17h, five per backplane (pcf8553 p.6 Table 4, p.11-12 Table 9, p.13 Figure 5):
  COM0 04h-08h, COM1 09h-0Dh, COM2 0Eh-12h, COM3 13h-17h; byte n of a row holds SEG8n (LSB) to SEG8n+7 (MSB). Driver keys:
  `ram {bytes 20, address 4}`, `ramOrder backplane`, `bitOrder lsb`. The layout is the same in every MUX mode (Table 9).
  Frame 64 Hz (Device_ctrl FF reset `01`, Table 5 p.7) = 15625 us.
* BU91795: DDRAM 12 x 4 bits, nibble addresses 04h-0Fh = SEG0-SEG11, 00h-03h and 10h-13h unused (bu91795 p.9). The first bit
  of a data byte is COM0 of its lower address: bits a, b, c, d of byte "a b c d e f g h" are COM0-COM3 of the address, e-h of
  the next (p.9 figure). Nibble registers `DD00`-`DD13` (4 bits), `ram` = DD04-DD0F, `bitOrder msb` (bit 3 = COM0), `ramOrder
  segment`. Frame 80 Hz (DISCTL P4P3 = 00, reset, p.12) = 12500 us.
* BU9796AFS: DDRAM 20 x 4 bits at 00h-13h = SEG0-SEG19 (bu9796axxx-e p.10); BU9796AMUV: 12 x 4, SEG0-SEG11 at 00h-0Bh, 0Ch-13h
  dummy (p.11). Same bit order. Frame 80 Hz (p.13) = 12500 us.

## PCF8553 and PCA8553 (NXP)

40 segment and 4 backplane outputs, I2C (400 kHz) or SPI (5 MHz), VDD and VLCD 1.8 V to 5.5 V (pcf8553 p.32 Table 17,
p.1). Orderable (Table 2, p.2, both sheets): `PCF8553DTT/AJ` (discontinuation notice 202107021DN, replaced by AY: obsolete),
`PCF8553DTT/AY`; `PCA8553DTT/AJ` (same notice, obsolete), `PCA8553DTT/AY`; all TSSOP56 (SOT364-1), reel of 2000.
PCA8553: AEC-Q100 grade 2 up to 105 C (pca8553 p.1); its only functional difference is Device_ctrl FF (three bits).
The product type numbers `PCF8553DTT/A`, `PCA8553DTT/A` (Table 1) are not orderable parts.

### Pins (TSSOP56, Figure 2 p.4, Table 3 p.4-5; PCA8553 identical)

| Pin | Name | Role | Pin | Name | Role |
|---|---|---|---|---|---|
| 1-6 | SEG34-SEG39 | output | 15 | RST | input (active LOW, 100 kOhm pull-up p.32) |
| 7-10 | COM0-COM3 | output | 16 | SDIO | inout (SPI data in/out, unused in I2C) |
| 11 | VLCD | power (analog) | 17 | A0 | input (I2C address bit, unused in SPI) |
| 12 | VDD | power | 18 | CLK | inout (internal oscillator out / external clock in) |
| 13 | IFS | input (VSS or open = I2C, VDD = SPI) | 19 | SCL | input |
| 14 | VSS | ground | 20 | SDA/CE | inout (I2C SDA, SPI CE active LOW) |
| 23-56 | SEG0-SEG33 | output | 21 | PORE | input (VDD = power-on reset enabled) |
| | | | 22 | A1 | input |

A0, A1, PORE and IFS have internal latching pull-downs (p.39, 15.3).

### Bus

* I2C (p.25-28): 7-bit address `0 1 1 1 0 A1 A0` (Table 12) = 0x38 + 2 A1 + A0, A0 / A1 tied to VSS or VDD; write: START,
  address+W, register pointer byte (00h-17h, other values ignored but acknowledged), data bytes with auto-increment, the
  pointer stops at 18h (p.27); read: a pointer write, then START, address+R, bytes from the pointer with auto-increment
  (the controller ACKs; the chip omits the NACK, p.26). The sheet's SDA is `SDA/CE`.
* SPI (p.28-30, Tables 14-15), mode 0 (input sampled on the rising SCL edge, output valid after the falling edge), CE =
  SDA/CE active LOW, data on the bidirectional SDIO: first byte `R/W 0 0 AP[4:0]` (R/W = 1 reads), then data bytes with
  auto-increment; AP above 17h ignored. The read data come out on SDIO right after the address byte (tt(SDI-SDO) = 0, p.38).
* Interface select: IFS (p.5); modeled as `i2c-spi` with `selectPin IFS`, `spiLevel 1`.

### Registers (Table 4 p.5-6; reset values p.10-11)

| Address | Register | Bits | Function | Reset |
|---|---|---|---|---|
| 00h | Software_reset | SR[7:0] | 0010 1100 (2Ch) resets every register, the RAM and the pointer (Table 8 p.11); anything else: no reset | 00h |
| 01h | Device_ctrl | PCF: 3:2 FF, 1 OSC, 0 COE; PCA: 4:2 FF, 1 OSC, 0 COE; other bits write 0 | FF frame frequency: PCF 00 32, 01 64, 10 96, 11 128 Hz; PCA 000-111 = 32, 64, 96, 128, 160, 192, 224, 256 Hz. OSC 0 internal oscillator on, 1 off (external clock). COE 1 clock on CLK (Table 5 p.7) | FF = 01 (64 Hz), OSC 0, COE 0: 04h |
| 02h | Display_ctrl_1 | 4 BOOST, 3:2 MUX, 1 B, 0 DE | MUX 00 1:4, 01 1:3, 10 1:2, 11 static; B 0 1/3, 1 1/2 bias; DE 1 display enabled, 0 power-down (Table 6 p.8) | 00h |
| 03h | Display_ctrl_2 | 2:1 BL, 0 INV | BL 00 off, 01 0.5 Hz, 10 1 Hz, 11 2 Hz blinking; INV 0 line, 1 frame inversion (Table 7 p.8-9) | 00h |
| 04h-17h | RAM | 8 bits each | display data, layout above | 00h |

Reset (p.10): power-on reset with PORE = VDD, the RST pin held LOW for at least 10 us, or Software_reset: every segment and
backplane output at VSS, 1:4 mux, 1/3 bias, blinking off, pointer 0, display and internal oscillator disabled (DE = 0), RAM 0.
After a reset the pointer must be set again (p.10). Power-down (DE = 0) keeps the registers and the bus works (p.10).
Clock: a clock must be supplied whenever the display is enabled (p.7 remark).

Model: the registers above (unused bits are `readOnly`, so they read 0), one phantom register `AP18` at 18h (read-only) so the
pointer sticks beyond 17h and the extra bytes are ignored, `triggers` `SWRST.SR=44` -> `reset`, `pins.reset RST` active 0,
`display.enable DISP1.DE=1`.

## BU91795MUF-M (ROHM, automotive)

48 segments (12 SEG x 4 COM), 1/4 duty, 1/3 bias, I2C write only, VDD 2.5 V to 6.0 V, VLCD 0 V to VDD - 2.4 V
(bu91795 p.1, p.4), -40 to +105 C, AEC-Q100 grade 2. Ordering (p.25): `BU91795MUF-ME2` (M: automotive, MUF: VQFN24FV4040,
E2: reel of 2500, drawing p.26). No obsolete note in the sheet.

### Pins (VQFN24FV4040, Figure 3 p.2 top view, Table 1 p.3)

1 OSCIN (input, VSS for the internal oscillator), 2 SCL (input), 3 SDA (inout, open drain), 4-15 SEG0-SEG11 (output), 16-19
COM0-COM3 (output), 20 VLCD (power, analog), 21 VDD (power), 22 VSS (ground), 23 TEST1 (input, VSS), 24 TEST2 (input, VSS =
power-on reset enabled, VDD = disabled: a software reset is then needed). EXT-PAD: VSS or open (footprint centre pad).
Pin 1 is on the bottom row's left in the top view; the pins run counter-clockwise.

### Bus and commands (p.8-13)

Slave address `0111 1100` write only (7-bit 0x3E). After START and the address every byte is a command byte `C D6..D0`
(p.9): C = 1: another command byte follows; C = 0: the command is the last, the following bytes are display data (two 4-bit
nibbles each, address auto-incremented every 4 bits), until START or STOP. Commands (p.11-13):

| Command | Byte (C = 1 shown) | Fields | Reset |
|---|---|---|---|
| ICSET | `1 1101 * P1 P0` | P1 = 1 software reset: display off, DDRAM address 00h, registers reset, DDRAM data kept (p.10); P0 oscillator 0 internal, 1 external clock (OSCIN) | P0 = 0 |
| DISCTL | `1 01 P4 P3 P2 P1 P0` | P4P3 frame FR 00 80, 01 71, 10 64, 11 53 Hz; P2 0 line / 1 frame inversion; P1P0 SR 00 power save 1, 01 power save 2, 10 normal, 11 high power | FR 00, P2 0, SR 10 |
| ADSET | `1 000 P3 P2 P1 P0` | DDRAM address 04h-0Fh; out of range (0-3) gives address 00h (p.12) | 00h |
| MODESET | `1 10 * P3 0 * *` | P3 1 display on, 0 off ("all outputs VSS after one frame") | off |
| BLKCTL | `1 1110 * P1 P0` | 00 off, 01 0.5 Hz, 10 1 Hz, 11 2 Hz | off |
| APCTL | `1 11111 P1 P0` | P1 all pixels on, P0 all pixels off (P0 wins), DDRAM unchanged, valid while the display is on | 0 0 |

Examples (p.15-17): start sequence `EAh` (ICSET reset), `F0h` (BLKCTL off), `A4h` (DISCTL 80 Hz, frame inversion, power save 1),
`E9h` (ICSET external clock), ADSET `04h` (C = 0) and six data bytes (addresses 04h-0Fh), STOP, then START, address, `C8h`
(MODESET display on). The DISPON sequence is `E8h` (ICSET internal), `BFh` (DISCTL), `F0h`, `FCh` (APCTL normal), `C8h`; the
DISPOFF sequence `E8h`, `C0h` (p.17). A software reset is required after power-up when TEST2 = VDD (p.3, p.19).

Model: nibble registers `DD00`-`DD13` (address = ADSET address), internal registers ICSET.OSC, DISCTL (FR, INV, SR), MODE.ON,
BLKCTL.B, APCTL (APON, APOFF), a `sink` space register `SINK` (target of RAM bytes after a non-ADSET C = 0 command), the stream
protocol with every command in a C = 1 and a C = 0 form (C = 1 ADSET uses the `read` action to set the pointer without a
payload, C = 0 ADSET and the ICSET reset use `pointer` with auto-increment), `display.enable [MODE.ON=1, APCTL.APOFF=0]`.

## BU9796AFS and BU9796AMUV (ROHM)

80-segment (20 x 4) and 48-segment (12 x 4) drivers, 1/4 duty, 1/2 or 1/3 bias, I2C write only, VDD 2.5 V to 5.5 V, VLCD 0 V
to VDD - 2.4 V, -40 to +85 C (bu9796axxx-e p.1, p.4). Ordering (Lineup p.24, p.29): `BU9796AFS-E2` (SSOP-A32, reel 2000),
`BU9796AMUV-E2` (VQFN024V4040, reel 2500), `BU9796AMUV-ZE2` (VQFN24V4040A, production site Z added, p.29). No obsolete note.
The two packages differ in segment count, so they are two devices with the same commands.

### Pins

* BU9796AFS, SSOP-A32 (Figure 3 p.2, Table 1 p.2): 1-18 SEG2-SEG19, 19-22 COM0-COM3, 23 VLCD, 24 VDD, 25 VSS, 26 TEST1, 27 TEST2,
  28 OSCIN, 29 SCL, 30 SDA, 31 SEG0, 32 SEG1.
* BU9796AMUV, VQFN24 (Figure 5 p.3 bottom view, Table 1 p.3): 1 OSCIN, 2 SCL, 3 SDA, 4-15 SEG0-SEG11, 16-19 COM0-COM3, 20 VLCD,
  21 VDD, 22 VSS, 23 TEST1, 24 TEST2; EXT-PAD VSS or open (identical to the BU91795 pinout).

Roles as for the BU91795. (Table 1 says TEST2 = L enables the power-on reset; the later text p.22 says TEST1 = L: an inconsistency
in the sheet, not modeled.)

### Commands (p.9-15)

Same bus, address 0x3E, C-bit chain and display-data rule as the BU91795, with these differences:

| Command | Byte (C = 1) | Difference |
|---|---|---|
| ADSET | `1 00 P4 P3 P2 P1 P0` | address 00000-10011 (00h-13h); out of range gives 00h (p.14); the RAM address wraps 13h to 00h (p.10) |
| MODESET | `1 10 * P3 P2 * *` | P3 display on; P2 bias 0 = 1/3, 1 = 1/2 (p.14); reset display off, 1/3 |
| DISCTL | as BU91795 | P4P3 = 11 is printed "50 Hz" in the table (p.13) and 53 Hz in the external-clock formula (p.14) |
| ICSET, BLKCTL, APCTL | as BU91795 | p.14-15 |

DDRAM: AFS 20 nibbles 00h-13h = SEG0-SEG19 (p.10); AMUV 12 nibbles 00h-0Bh = SEG0-SEG11, 0Ch-13h dummy (p.11), the registers
`DD0C`-`DD13` exist for the address sequence and show nothing.

## Footprints

| Package | Footprint | Source |
|---|---|---|
| TSSOP56 (SOT364-1, 6.1 x 14.0 mm, 0.5 mm) | existing `TSSOP-56` (same MO-153 body and pitch) | pcf8553 p.40 |
| VQFN24FV4040 (BU91795), VQFN024V4040 (BU9796AMUV-E2) | new `VQFN-24-4X4-ROHM`: R4, 4.0 mm body, pitch 0.5, terminal L 0.4, b 0.255 mid (0.21-0.30), pad 0.7 x 0.31, span 3.9, exposed pad 2.4; pin 1 bottom row left (counter-clockwise) | bu91795 p.26, bu9796axxx-e p.26 |
| VQFN24V4040A (BU9796AMUV-ZE2) | new `VQFN-24-4X4-ROHM-A`: as above, exposed pad 2.6 | bu9796axxx-e p.30 |
| SSOP-A32 (BU9796AFS) | new `SSOP-A32-ROHM`: R2, 5.4 x 13.6 mm body, pitch 0.8, lead b 0.36, tip to tip 7.8 (foot length not printed: taken as 1.2) | bu9796axxx-e p.25 |

## Datasheet URLs (each answers with a PDF)

`pcf8553` https://www.nxp.com/docs/en/data-sheet/PCF8553.pdf; `pca8553` https://www.nxp.com/docs/en/data-sheet/PCA8553.pdf;
`bu91795muf-m-e` https://fscdn.rohm.com/en/products/databook/datasheet/ic/driver/lcd_segment/bu91795muf-m-e.pdf;
`bu9796axxx-e` https://fscdn.rohm.com/en/products/databook/datasheet/ic/driver/lcd_segment/bu9796axxx-e.pdf.
(`curl` gets HTTP 404 from nxp.com; Node's `fetch`, as `scripts/check-datasheets.js` uses it, gets the PDF.)

## Engine needs

See `engineNeeds` in the result file: mode by register field, frame time by field, blinking, all-pixels-on, a disabled output
level, a `pointer` that keeps the current address, pointer wrap-around, open-pin strap ties.
