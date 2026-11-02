# Bridges: I2C/SPI-to-UART bridges and the SPI-to-I2C bridge — datasheet fact sheet

Sources: all page numbers are PDF page numbers (= printed page numbers in these PDFs).
Repo paths: `datasheets/controller-uart/…`, `datasheets/controller-i2c/…`.

## PDF → devices

| PDF | Devices (sections below) |
|---|---|
| datasheets/controller-uart/sc16is750.pdf (NXP, Rev 7.1, 6 Feb 2025, 61 pp) | **SC16IS750** (also covers SC16IS760 = faster-SPI/fast-IrDA grade of the same design, and SC16IS740 = same die without GPIO/modem pins, TSSOP16 only) |
| datasheets/controller-uart/sc16is752.pdf (NXP, Rev 9.1, 5 Feb 2025, 56 pp) | **SC16IS752** (also covers SC16IS762 = faster-SPI/fast-IrDA grade) |
| datasheets/controller-uart/xr20m1170.pdf (MaxLinear) | **XR20M1170** |
| datasheets/controller-uart/xr20m1280.pdf (MaxLinear) | **XR20M1280** |
| datasheets/controller-uart/pi7c9x760.pdf (Diodes/Pericom) | **PI7C9X760** |
| datasheets/controller-uart/pi7c9x762.pdf (Diodes/Pericom) | **PI7C9X762** |
| datasheets/controller-i2c/sc18is604.pdf (NXP) | **SC18IS604** |

Census parts not fully decodable from the PDFs: **PI7C9X762Q2ZHEX** (pi7c9x762.pdf lists only PI7C9X762DZHEX; package ZH is decodable, "Q2" grade is not in the PDF) and **PI7C9X760CZDEX** (PDF lists only …DZDEX; C = earlier revision letter, same ZD package). Details in each section's Parts and the Summary at the end.

---

## SC16IS750

(also SC16IS740 and SC16IS760 — one register design; see "Variants")

- Title: "SC16IS740/750/760 — Single UART with I²C-bus/SPI interface, 64 bytes of transmit and receive FIFOs, IrDA SIR built-in support", Product data sheet Rev. 7.1, 6 Feb 2025. File: `datasheets/controller-uart/sc16is750.pdf`, pp. 1–51 (all technical content); pp. 52–61 soldering/legal.

### Variants (p. 1, p. 2, p. 21 note [9], p. 31 note [1], p. 45)
- SC16IS750: reference. SPI max 4 Mbit/s (p. 2), IrDA SIR up to 115.2 kbit/s only (EFCR[7] fast mode not available, p. 21 note [9]).
- SC16IS760: "functionally and electrically the same as the SC16IS750" except SPI clock up to 15 Mbit/s and IrDA SIR up to 1.152 Mbit/s (EFCR[7]=1 ¼-pulse mode) (p. 1). → **same device, speed grade**.
- SC16IS740: "functionally and electrically identical to the SC16IS750, with the exception of the programmable I/O pins which are only present on the SC16IS750" (p. 1). No GPIO0–7 / DSR/DTR/CD/RI pins; registers IODir, IOState, IOIntEna not supported; IOControl only has the software-reset bit (p. 15 notes [2],[3]); MSR[7:5], MSR[3:1] and MCR[0] (DTR) not available, IER[3] modem interrupt not available, IIR modem (0x00) and I/O-pin (0x30) sources not available (p. 16 note [1], p. 26–28 notes). SPI timing = SC16IS750 table (4 MHz) (p. 45 Table 39 "SC16IS740/750"). → same register design with features removed; model as SC16IS750 with GPIO/modem pins absent (a "no-GPIO" variant) or as a separate kind — planner's choice.

### Parts (ordering info Table 1, p. 3)
NXP orderable suffixes `,112` `,128` `,157` `,151` are NXP 12NC packing variants (tube / reel / tray); the PDF does not list them — package and grade are fixed by the base type number. All grades: VDD 2.5 V ± 0.2 V (−40…+85 °C) or 3.3 V ± 0.3 V (−40…+95 °C) (p. 40).

| Census part | Status | Base type | Package (NXP name / outline) | pins | pinout name | grade / supply |
|---|---|---|---|---|---|---|
| SC16IS740IPW,112 | Obsolete | SC16IS740IPW | TSSOP16, SOT403-1, body 4.4 mm | 16 | tssop16 | industrial; 2.3–2.7 V or 3.0–3.6 V; SPI 4 MHz |
| SC16IS740IPW,128 | Active | SC16IS740IPW | TSSOP16, SOT403-1 | 16 | tssop16 | same |
| SC16IS740IPW/Q900, | Active | SC16IS740IPW/Q900 | TSSOP16, SOT403-1 | 16 | tssop16 | AEC-Q100 compliant (p. 3 note [1]); same electricals |
| SC16IS750IBS,128 | Active | SC16IS750IBS | HVQFN24, SOT616-3, 4×4×0.85 mm, exposed pad | 24+EP | hvqfn24 | SPI 4 MHz, IrDA 115.2k |
| SC16IS750IBS,157 | Obsolete | SC16IS750IBS | HVQFN24, SOT616-3 | 24+EP | hvqfn24 | same |
| SC16IS760IBS,128 | Last Time Buy | SC16IS760IBS | HVQFN24, SOT616-3 | 24+EP | hvqfn24 | SPI 15 MHz (2.5 V: tCP ≥ 83 ns ≈ 12 MHz; 3.3 V: tCP ≥ 67 ns ≈ 15 MHz), IrDA up to 1.152 Mbit/s |

(TSSOP24 SOT355-1 parts SC16IS750IPW/760IPW exist in the PDF but are not in the census.)

### Pinouts (Figs. 5–7 pp. 6–8, Table 2 pp. 8–10)
Pin I2C/SPI selects the host interface; several pins change meaning (listed as `I2C-meaning / SPI-meaning`). Suggested pin names: `CS_A0`, `SI_A1`, `SO`, `SCL_SCLK`, `SDA`, `I2C_SPI`.

**tssop16 (SC16IS740IPW, SOT403-1)** (p. 6)
| pin | name | role | notes |
|---|---|---|---|
| 1 | VDD | power | |
| 2 | CS / A0 | input | SPI chip select (active LOW, Schmitt) / I2C address select A0 |
| 3 | SI / A1 | input | SPI data in / I2C address select A1 |
| 4 | SO | output (3-state) | SPI data out; in I2C mode "undefined, must be left n.c." (figure labels it n.c.) |
| 5 | SCL / SCLK | input | |
| 6 | SDA | inout (open-drain) | I2C data; in SPI mode "undefined pin, must be connected to VSS" (figure labels it VSS) |
| 7 | IRQ | output (open-drain, active LOW) | external pull-up required (1 kΩ @3.3 V, 1.5 kΩ @2.5 V) |
| 8 | I2C/SPI | input | HIGH = I2C, LOW = SPI |
| 9 | VSS | ground | |
| 10 | RTS | output | active LOW |
| 11 | CTS | input | active LOW |
| 12 | TX | output | |
| 13 | RX | input | |
| 14 | RESET | input | active LOW |
| 15 | XTAL1 | input | crystal / external clock in |
| 16 | XTAL2 | output | crystal out (leave open with external clock) |

**hvqfn24 (SC16IS750IBS / SC16IS760IBS, SOT616-3)** (p. 8, Table 2)
| pin | name | role |
|---|---|---|
| 1 | RESET | input (active LOW) |
| 2 | XTAL1 | input |
| 3 | XTAL2 | output |
| 4 | VDD | power |
| 5 | I2C/SPI | input |
| 6 | CS / A0 | input |
| 7 | SI / A1 | input |
| 8 | SO | output (3-state; n.c. in I2C mode) |
| 9 | SCL / SCLK | input |
| 10 | SDA | inout open-drain (tie to VSS in SPI mode) |
| 11 | IRQ | output open-drain active LOW |
| 12 | GPIO0 | inout |
| 13 | GPIO1 | inout |
| 14 | GPIO2 | inout |
| 15 | GPIO3 | inout |
| 16 | VSS | ground |
| 17 | GPIO4/DSR | inout (GPIO or DSR input, IOControl[1]) |
| 18 | GPIO5/DTR | inout (GPIO or DTR output) |
| 19 | GPIO6/CD | inout (GPIO or CD input) |
| 20 | GPIO7/RI | inout (GPIO or RI input) |
| 21 | RTS | output |
| 22 | CTS | input |
| 23 | TX | output |
| 24 | RX | input |
| EP | VSS | ground — exposed center pad, connect to ground (p. 10 note [4]) |

(TSSOP24 SOT355-1, not in census, p. 7: 1 CTS, 2 TX, 3 RX, 4 RESET, 5 XTAL1, 6 XTAL2, 7 VDD, 8 I2C/SPI, 9 CS/A0, 10 SI/A1, 11 SO(n.c.), 12 SCL/SCLK, 13 SDA(VSS), 14 IRQ, 15–18 GPIO0–3, 19 VSS, 20 GPIO4/DSR, 21 GPIO5/DTR, 22 GPIO6/CD, 23 GPIO7/RI, 24 RTS.)

GPIO0–GPIO7 inputs have an active pull-up (RPU 3.02–3.63 MΩ @3.3 V, 3.94–4.91 MΩ @2.5 V) (p. 10 note [2], p. 41). No other pin has a pull-up.

### Host interface
**Common**: one 8-bit "register address / subaddress" byte selects register A[3:0] (16 locations); bank selection by LCR/EFR/MCR state (see Registers). **No auto-increment**: in a multi-byte transfer every data byte goes to/comes from the *same* register ("the data bytes which follow will be written one after the other to the subaddress location", p. 37). This is how FIFO bursts work (address 0x00 = THR/RHR). Burst reads of IIR "should not be performed" (p. 22 note [5]).

**I2C target** (I2C/SPI pin HIGH) — Section 10 pp. 33–38
- Target only; Standard (100 kHz) and Fast mode (400 kHz); noise filter, tSP 50 ns (p. 2, p. 42).
- Address: 7-bit, set by A1, A0 each strapped to VDD, VSS, SCL or SDA (16 addresses). Table 32 (pp. 36–37), shown as 8-bit write address (bit0 = R/W, 0 = write):

| A1 | A0 | 8-bit addr | 7-bit |
|---|---|---|---|
| VDD | VDD | 0x90 | 0x48 |
| VDD | VSS | 0x92 | 0x49 |
| VDD | SCL | 0x94 | 0x4A |
| VDD | SDA | 0x96 | 0x4B |
| VSS | VDD | 0x98 | 0x4C |
| VSS | VSS | 0x9A | 0x4D |
| VSS | SCL | 0x9C | 0x4E |
| VSS | SDA | 0x9E | 0x4F |
| SCL | VDD | 0xA0 | 0x50 |
| SCL | VSS | 0xA2 | 0x51 |
| SCL | SCL | 0xA4 | 0x52 |
| SCL | SDA | 0xA6 | 0x53 |
| SDA | VDD | 0xA8 | 0x54 |
| SDA | VSS | 0xAA | 0x55 |
| SDA | SCL | 0xAC | 0x56 |
| SDA | SDA | 0xAE | 0x57 |

  (The datasheet does not describe *how* the chip detects SCL/SDA straps; a model can sample A0/A1 against SDA/SCL at each START.)
- Subaddress byte (Table 33, p. 38): bit7 not used; bits6:3 = A[3:0] register; bits2:1 = CH1,CH0 must be 00 (other values reserved); bit0 not used.
- Write (Fig. 23, p. 37): `S addr+W A subaddr A data A data A … P` — data bytes all go to the subaddress register.
- Read (Fig. 24, p. 38): `S addr+W A subaddr A Sr addr+R A data A … lastdata NA P` — bytes from the subaddress register, controller ACKs each, NACKs the last.
- Device returns NACK when the IOControl SRESET bit is written (p. 22 note [8]) — i.e. the data byte that resets the chip is not acknowledged.
- Bus time-out: "resets the serial bus interface if SDA is held LOW for a minimum of 25 ms" (p. 42 note [2]).
- MSB first; ACK on 9th clock. Clock stretching: Fig. 19 (generic I2C text) mentions "clock line held LOW while interrupt is serviced" but the datasheet gives no statement that this device stretches — treat as **no clock stretching** (not stated).

**SPI target** (I2C/SPI pin LOW) — Section 11 p. 39, timing pp. 45–48
- SPI mode 0 only (p. 2): SCLK idle LOW; SI sampled on SCLK rising edge; SO changes after SCLK falling edge (tDO "SCLK fall to SO valid"). MSB first. CS active LOW. SO is 3-stated when CS HIGH (tTR ≤ 100 ns).
- First byte (Table 34): bit7 = R/W (1 = read, 0 = write); bits6:3 = A[3:0]; bits2:1 = CH1,CH0 = 00; bit0 = don't care (X).
- Register write (Fig. 25a): command byte then data byte D7..D0 on SI.
- Register read (Fig. 25b): command byte on SI; data D7..D0 on SO during the following 8 clocks.
- FIFO write/read (Fig. 25c/d): with A[3:0]=0000, keep CS LOW and continue clocking: each further 8 clocks writes another byte to the TX FIFO / reads the next RX FIFO byte. Same rule for any register (repeat access to same address), but IIR burst read is prohibited (p. 22 note [5]).
- A transaction ends on CS HIGH; minimum CS HIGH tCSW 200 ns.
- Max SCLK: SC16IS740/750 4 MHz (tCP ≥ 250 ns); SC16IS760 15 MHz at 3.3 V (tCP 67 ns), 12 MHz at 2.5 V (tCP 83 ns).

### Registers (Tables 9–31, pp. 20–32)
Access conditions:
- **General set**: LCR[7] = 0.
- **Special set** (DLL @0x00, DLH @0x01): LCR[7] = 1 and LCR ≠ 0xBF. Other addresses while LCR[7]=1 (and ≠0xBF) are not stated by the PDF; the table only lists DLL/DLH in the special set (the general registers at 0x02–0x0F are presumably still reachable — not stated).
- **Enhanced set** (EFR @0x02, XON1 @0x04, XON2 @0x05, XOFF1 @0x06, XOFF2 @0x07): LCR = 0xBF.
- **TCR @0x06 / TLR @0x07** replace MSR/SPR when EFR[4] = 1 **and** MCR[2] = 1 (p. 21 note [7]).
- "EFR[4] write-enable": IER[7:4], FCR[5:4], MCR[7:5] and MCR[2] can only be modified when EFR[4] = 1 (pp. 21, 22, 26, 27).

| Addr A[3:0] | Name | R/W | Reset | Bits |
|---|---|---|---|---|
| 0x00 | RHR | R | — (pointer cleared) | received data (FIFO top) |
| 0x00 | THR | W | — | transmit data |
| 0x01 | IER | R/W | 0x00 | 7 CTS int en*; 6 RTS int en*; 5 Xoff int en*; 4 Sleep mode en*; 3 modem-status int en (750/760 only); 2 RX line status int en; 1 THR int en; 0 RHR int en (*EFR[4] needed to modify) |
| 0x02 | FCR | W | 0x00 | 7:6 RX trigger 00=8, 01=16, 10=56, 11=60 chars; 5:4 TX trigger (EFR[4]) 00=8, 01=16, 10=32, 11=56 spaces; 3 reserved; 2 TX FIFO reset (self-clearing); 1 RX FIFO reset (self-clearing); 0 FIFO enable |
| 0x02 | IIR | R | 0x01 | 7:6 = FCR[0] mirrored (both bits); 5:1 interrupt code; 0 = 0 interrupt pending / 1 none |
| 0x03 | LCR | R/W | 0x1D | 7 divisor latch enable; 6 set break; 5 set (forced) parity; 4 even parity; 3 parity enable; 2 stop bits; 1:0 word length |
| 0x04 | MCR | R/W | 0x00 | 7 clock divisor (0 = ÷1, 1 = ÷4) (EFR[4]); 6 IrDA mode enable (EFR[4]); 5 Xon Any (EFR[4]); 4 loopback; 3 reserved; 2 TCR/TLR enable (EFR[4]); 1 RTS (1 = RTS pin LOW/active); 0 DTR (1 = DTR LOW/active, only when GPIO5 is DTR) |
| 0x05 | LSR | R | 0x60 | 7 FIFO data error; 6 THR and TSR empty; 5 THR empty; 4 break interrupt; 3 framing error; 2 parity error; 1 overrun error; 0 data in receiver |
| 0x06 | MSR | R | bits3:0 = 0, 7:4 = inputs | 7 CD (=NOT pin); 6 RI; 5 DSR; 4 CTS; 3 ΔCD; 2 ΔRI (RI pin LOW→HIGH); 1 ΔDSR; 0 ΔCTS — Δ bits cleared on read |
| 0x07 | SPR | R/W | not reset | scratch |
| 0x06 | TCR | R/W | 0x00 | 7:4 RX FIFO level to resume; 3:0 RX FIFO level to halt; value×4 = characters (0–60) |
| 0x07 | TLR | R/W | 0x00 | 7:4 RX trigger (×4, 4–60 chars); 3:0 TX trigger (×4, 4–60 spaces); 0 = use FCR |
| 0x08 | TXLVL | R | 0x40 | 6:0 spaces available in TX FIFO (0–64); 7 = 0 |
| 0x09 | RXLVL | R | 0x00 | 6:0 characters in RX FIFO (0–64); 7 = 0 |
| 0x0A | IODir | R/W | 0x00 | bit n: GPIOn 0 = input, 1 = output (750/760 only) |
| 0x0B | IOState | R/W | (pins) | write: output latch for output pins; read: actual state of all 8 pins |
| 0x0C | IOIntEna | R/W | 0x00 | bit n: 1 = change on input GPIOn generates interrupt |
| 0x0D | reserved | — | — | all bits reserved, write 0 |
| 0x0E | IOControl | R/W | 0x00 | 7:4 reserved; 3 SRESET (write 1 → full reset, self-clears; I2C NACKs this write); 2 reserved; 1 GPIO[7:4] = modem pins (1 = RI, CD, DTR, DSR); 0 IOLATCH |
| 0x0F | EFCR | R/W | 0x00 | 7 IrDA mode 0 = 3/16 pulse (≤115.2k), 1 = 1/4 pulse (≤1.152M, 760 only); 6 reserved; 5 RTSINVER; 4 RTSCON (auto RS-485 direction); 3 reserved; 2 TXDISABLE; 1 RXDISABLE; 0 9-bit (multidrop) mode |
| 0x00 (LCR[7]=1, ≠0xBF) | DLL | R/W | not reset | divisor LSB |
| 0x01 (same) | DLH | R/W | not reset | divisor MSB |
| 0x02 (LCR=0xBF) | EFR | R/W | 0x00 | 7 auto CTS; 6 auto RTS; 5 special char detect; 4 enhanced functions enable; 3:0 software flow control (Table 3) |
| 0x04 (LCR=0xBF) | XON1 | R/W | not reset | |
| 0x05 (LCR=0xBF) | XON2 | R/W | not reset | |
| 0x06 (LCR=0xBF) | XOFF1 | R/W | not reset | |
| 0x07 (LCR=0xBF) | XOFF2 | R/W | not reset | also special character / RS-485 address |

Details:
- LCR (Tables 12–15 pp. 23–24): parity LCR[5:3]: xx0 none; 001 odd; 011 even; 101 forced 1; 111 forced 0. Stop: LCR[2]=0 → 1; =1 → 1.5 for 5-bit, 2 for 6/7/8-bit. Word length LCR[1:0]: 00=5, 01=6, 10=7, 11=8.
- LSR (Table 16 pp. 24–25): LSR[4:2] are the BI/FE/PE of the character at the top of the RX FIFO (next to be read); LSR[7] set if any error anywhere in RX FIFO, cleared when none remain. Break: "associated character is 0x00". LSR[1] overrun. Clear-on-read of LSR[1]/[4:2] is not stated (16C450-style: reading LSR clears OE; FE/PE/BI follow the FIFO top).
- MSR: CD/RI/DSR bits only when IOControl[1] = 1; with GPIOs as I/O, MSR[7:5]/[3:1] don't reflect the pins and no modem interrupt from them (p. 31). ΔRI = "RI input has changed state from LOW to HIGH" (p. 26) — i.e. trailing edge of RI (pin LOW→HIGH = MSR[6] 1→0). ΔCD/ΔDSR/ΔCTS any change.
- In loopback (MCR[4]): "MCR[1:0] signals are looped back into MSR[4:5]" (RTS→CTS, DTR→DSR) and TX looped to RX internally; TX output pin "disabled" (p. 8, 25). Loopback of OUT1/OUT2→RI/CD is not mentioned (no OUT1/OUT2 bits).
- IIR codes (Table 21 p. 28), priority 1 highest:

| IIR[5:0] | Pri | Source | Clear |
|---|---|---|---|
| 000001 | – | none | |
| 000110 | 1 | RX line status (OE, FE, PE, BI in RX FIFO) | read LSR (implied; not explicitly stated) |
| 001100 | 2 | RX time-out | read RHR (timer resets) |
| 000100 | 2 | RHR: data ready (FIFO off) or RX FIFO ≥ trigger | read RHR below trigger |
| 000010 | 3 | THR: TX empty (FIFO off) or TX FIFO spaces ≥ trigger ("passes above trigger level") | write THR (Fig. 33/38), or read IIR (not stated) |
| 000000 | 4 | modem status (750/760) | read MSR |
| 110000 | 5 | I/O pins change of state (750/760) | read IOState; writing IODir also clears (p. 30) |
| 010000 | 6 | Xoff received / special character | Xoff: cleared by Xon detection; special char: cleared by IIR read (p. 16) |
| 100000 | 7 | CTS or RTS pin changed active(LOW)→inactive(HIGH) | not stated (presumably IIR read) |

  So IIR reads e.g. 0xC1 with FIFOs on & no interrupt; time-out reads 0xCC (p. 51 of SC16IS752 errata uses "0xCC").
- Re-enabling IER[1] "will not cause a new interrupt if the THR is below the threshold" (p. 27 note [1]).
- Software flow-control EFR[3:0] (Table 3 pp. 12–13): EFR[3:2] TX: 00 none; 10 Xon1/Xoff1; 01 Xon2/Xoff2; 11 Xon1+Xon2 / Xoff1+Xoff2 (two-character sequences). EFR[1:0] RX: 00 none; 10 compare Xon1/Xoff1; 01 compare Xon2/Xoff2; 11 — combined with EFR[3:2]=10 or 01: "receiver compares Xon1 or Xon2, Xoff1 or Xoff2"; with EFR[3:2]=11 or 00: "receiver compares Xon1 and Xon2, Xoff1 and Xoff2" (sequential).

### Behavior
- **Clock/baud** (pp. 18–20, 44): XTAL1/XTAL2 crystal (max 24 MHz) or external clock on XTAL1 (max 48 MHz @2.5 V, 80 MHz @3.3 V; pulse widths ≥10 ns / ≥6 ns). Prescaler MCR[7]: ÷1 or ÷4. `divisor = (fXTAL1 / prescaler) / (baud × 16)`, divisor 1…65535 (DLH:DLL). Divisor 0 → no baud clock (UART disabled). Sampling 16× only. No BAUDOUT/RCLK pins. Baud ≤ 5 Mbit/s (p. 1). DLL/DLH must not be written during sleep.
- **Frames**: 5–8 data bits LSB first, start, optional parity (odd/even/forced), 1/1.5/2 stop. Break TX: LCR[6] forces TX LOW (p. 18 says "until LSR[6] is cleared by the software" — an evident typo for LCR[6]). Break RX: RX LOW longer than a full character incl. start/parity/stop → LSR[4], 0x00 character in FIFO.
- **TX/RX enable**: EFCR[2] TXDISABLE: TSR finishes current char, FIFO keeps accepting host data. EFCR[1] RXDISABLE: stops receiving immediately (p. 32).
- **FIFOs**: 64-byte TX, 64-byte RX with 3 error bits per char (p. 10). After reset FIFOs disabled ("trigger level is one character", p. 10). With FIFO disabled, TX still uses FIFO location (p. 22: "If the FIFO is disabled, the FIFO is still used to store the byte"; RX uses location zero). FIFO reset needs ≥2 XTAL1 clocks; wait 2 Tclk before accessing RHR/THR (p. 22 note [4], p. 23 note [1]). TX FIFO overflow loses characters.
  - Trigger levels: TLR nibble ≠ 0 overrides FCR (level = 4×nibble). With TLR RX in use, FCR[7:6] should be 00 (p. 29).
  - No DMA modes, no TXRDY/RXRDY pins.
- **RX time-out**: 4 character times after the last received character, when FIFO below trigger; counter reset at center of each received stop bit or each RX FIFO read (p. 18).
- **Interrupt pin IRQ**: open-drain, active LOW, asserted while any enabled interrupt pending (IIR[0]=0). Not gated by any OUT2 bit.
- **Auto RTS** (EFR[6]) (p. 11, 29): RTS active (LOW) while RX FIFO level < TCR[3:0]×4 halt level; de-asserted when halt level reached; re-asserted when level falls to TCR[7:4]×4 resume level. If TCR = 0 the FCR trigger levels are used. TCR[3:0] > TCR[7:4] must be programmed by software (no check).
- **Auto CTS** (EFR[7]) (p. 12): TX checks CTS before starting next character; CTS must go HIGH before middle of last stop bit to stop the following character. With auto CTS, CTS changes don't interrupt (except the IIR 0x20 CTS/RTS-inactive interrupt when IER[7]/IER[6] set).
- **Xon/Xoff** (pp. 12–13): RX: on matching Xoff(s), TX halts after current char; IIR[4] set if IER[5]; Xon(s) resume and clear IIR[4]. TX: sends Xoff when RX FIFO passes TCR halt (or FCR trigger), sends Xon when RX FIFO reaches TCR resume (or falls below FCR trigger). Xon/Xoff are sent with current word length (low bits). Xon Any (MCR[5]): any received char resumes. Special character (EFR[5]): RX data matched to XOFF2 → IIR[4] Xoff interrupt, character still stored in FIFO, TX not halted.
- **IrDA** (MCR[6]): SIR encoder/decoder; EFCR[7]=0 3/16-bit-time pulses, =1 1/4-bit-time pulses (760 only). Pulse polarity not printed in this PDF (standard SIR: TX idle LOW, HIGH pulse for a 0 bit; RX expects LOW pulses — **not stated in PDF**).
- **RS-485** (Section 9 pp. 32–33): EFCR[4] RTSCON: transmitter drives RTS: asserted (LOW) as soon as host writes data to TX FIFO, de-asserted (HIGH) after last bit transmitted; overrides MCR[1] and auto-RTS. EFCR[5] inverts that polarity. EFCR[0] 9-bit mode: parity bit is address flag; use LCR[5:3]=111 (forced parity 0) so address bytes (9th bit = 1) set parity error.
  - Normal multidrop (EFR[5]=0): receiver initially disabled ignores data bytes (9th bit 0) until an address byte arrives; address byte goes to RX FIFO with parity error + line-status interrupt; host enables/disables receiver (EFCR[1]).
  - Auto address detection (EFR[5]=1, XOFF2 = address): non-matching data/addresses discarded; matching address auto-enables receiver, is pushed into FIFO with the 9th bit in the parity-error bit, line-status interrupt; a later non-matching address auto-disables the receiver and is discarded.
- **Sleep** (p. 17–18): enabled when EFR[4]=1 and IER[4]=1; entered when RX idle, TX FIFO and TSR empty, no interrupt pending except THR, RX FIFO empty. UART clock stopped; wake on any RX change, modem input change, or write to TX FIFO. Not woken by GPIO change (GPIO reads/interrupts still work). Rev C/D: no RX wake in IrDA mode; Rev E: works.
- **GPIO** (pp. 30–31): IODir per bit; IOState write sets outputs, read returns pin states. Interrupt on change of inputs enabled by IOIntEna. IOLATCH=0: change → interrupt, reading IOState clears; if input returns to original state before read, interrupt clears. IOLATCH=1: input value latched into IOState at change; IOState read clears; returning doesn't clear. Writing IODir clears a pending GPIO interrupt. When IOControl[1]=1, GPIO7..4 = RI, CD, DTR, DSR and IODir/IOState/IOIntEna bits 7:4 have no effect on them.
- **Reset** (Table 4–5 pp. 14–15): RESET pin (active LOW, ≥3 µs), POR and IOControl[3] software reset are identical. Register reset values in table above; DLL, DLH, SPR, XON1/2, XOFF1/2 are **not** reset. Outputs after reset: TX HIGH, RTS HIGH, GPIOs inputs, IRQ HIGH (released). After POR wait ≥3 µs before communicating; td15 SCL delay after reset ≥ 3 µs (or 2 XTAL1 clocks, whichever less).
- Reserved/undefined: CH1:CH0 ≠ 00 reserved; register 0x0D reserved; FCR[3], MCR[3], IOControl[7:4,2], EFCR[6,3] reserved (write 0). SO pin undefined in I2C mode, SDA undefined in SPI mode.

### Timing (pp. 42–48)
- I2C (Table 37 p. 42): fSCL 0–400 kHz; tBUF 1.3 µs; tHD;STA 0.6 µs; tSU;STA 0.6 µs; tSU;STO 0.6 µs; tHD;DAT 0 ns; tSU;DAT 150 ns (Fast) / 250 ns (Std); tVD;DAT, tVD;ACK max 0.6 µs; tLOW 1.3 µs; tHIGH 0.6 µs; tSP 50 ns. td1 GPIO output valid after IOState write ACK 0.5 µs; td2/td3 modem interrupt valid/clear 0.2 µs; td4/td5 GPIO interrupt valid/clear 0.2 µs; td6/td7 RX interrupt valid (after stop bit) / clear 0.2 µs; td8 TX interrupt clear 0.5 µs (Fast) / 1.0 µs (Std). (Column labelled "Min" for these td values in the PDF; they behave as max delays.)
- SPI SC16IS740/750 (Table 39 p. 45): tCP ≥ 250 ns, tCH/tCL ≥ 100 ns, tCSS ≥ 100 ns, tCSH ≥ 20 ns, tDS ≥ 100 ns, tDH ≥ 20 ns, tDO ≤ 100 ns (100 pF), tTR ≤ 100 ns, tCSW ≥ 200 ns, td9–td14 200 ns (GPIO valid, DTR valid, TX/modem/GPIO/RX interrupt clear), tw(rst) ≥ 3 µs.
- SPI SC16IS760 (Table 40 p. 45–46): 2.5 V / 3.3 V: tCP 83/67 ns, tCH,tCL 30/25 ns, tCSS 100, tCSH 5, tDS 10, tDH 10, tDO 25/20 ns, tTR 100, tCSW 200 ns, td9–td14 200 ns.
- XTAL (Table 38 p. 44): see Clock.

### VCC
2.3–2.7 V (2.5 V grade, −40…+85 °C) or 3.0–3.6 V (3.3 V grade, −40…+95 °C) (p. 40). Inputs 5 V tolerant: VIH max 5.5 V on all inputs incl. XTAL1 (p. 40–41; "5.5 V … valid only when supply voltage is present") → inputMax 5.5 V.

### Package drawings
- TSSOP16 **SOT403-1** (p. 49): pitch e 0.65; body D 4.9–5.1 (5.0), E 4.3–4.5 (4.4); HE 6.2–6.6 (6.4); bp 0.19–0.30; Lp 0.50–0.75; L 1.0; A max 1.1. No land pattern printed.
- HVQFN24 **SOT616-3** (p. 50): D = E 3.9–4.1 (4.0); pitch e 0.5; b 0.18–0.30 (mid 0.24); L 0.3–0.5 (mid 0.4); exposed pad Dh = Eh 2.45–2.75 (mid 2.6); e1 = e2 2.5 (row span of terminal centres); A max 1.0, A1 0–0.05, c 0.2. Only min/max printed for b, L, Dh (no nominal). Land pattern not printed.
- (TSSOP24 SOT355-1 p. 51: D 7.7–7.9, same E/HE/pitch.)

### Obsolete
SC16IS740IPW,112; SC16IS750IBS,157. (SC16IS760IBS,128 = Last Time Buy.)

### Notes
- p. 18 break text "until LSR[6] is cleared" → should be LCR[6].
- MSR[6] RI note says "Reading IOState bit 6 does not reflect … RI" — should be bit 7 (p. 26).
- The IIR THR source text says "TX FIFO passes above trigger level" meaning spaces available ≥ TX trigger.
- XTAL table note [2] (p. 44) is empty in the PDF.
- How the ≥5 V-tolerant SDA/SCL straps are decoded and whether the chip stretches SCL: not stated.

---

## SC16IS752

(also SC16IS762 — same design, faster SPI / fast IrDA)

- Title: "SC16IS752; SC16IS762 — Dual UART with I²C-bus/SPI interface, 64 bytes of transmit and receive FIFOs, IrDA SIR built-in support", Rev 9.1, 5 Feb 2025. File `datasheets/controller-uart/sc16is752.pdf`, pp. 1–51.
- Difference from SC16IS750: **two UART channels (A, B)** selected by subaddress CH1:CH0 (00 = A, 01 = B); GPIO0–3 can become channel-B modem pins (IOControl[2]) and GPIO4–7 channel-A modem pins (IOControl[1]); IODir/IOState/IOIntEna/IOControl are **shared by both channels** (p. 20 note [7]). Everything else (registers, bits, reset values, FIFO, flow control, RS-485, IrDA, sleep, I2C address table, timing) is identical to SC16IS750 text (compare pp. 9–31 with SC16IS750 pp. 10–32). SC16IS762 vs 752: SPI up to 15 Mbit/s vs 4 Mbit/s and IrDA up to 1.152 Mbit/s (EFCR[7]) — "in all other aspects … functionally and electrically the same" (p. 1).

### Parts (Table 1 p. 3)
| Census part | Status | Package | pins | pinout | grade |
|---|---|---|---|---|---|
| SC16IS752IBS,128 | Active | HVQFN32 SOT617-1 5×5×0.85, EP | 32+EP | hvqfn32 | SPI 4 MHz |
| SC16IS752IBS,151 | Active | HVQFN32 SOT617-1 | 32+EP | hvqfn32 | SPI 4 MHz |
| SC16IS752IPW,112 | Obsolete | TSSOP28 SOT361-1, 4.4 mm | 28 | tssop28 | SPI 4 MHz |
| SC16IS752IPW,128 | Active | TSSOP28 SOT361-1 | 28 | tssop28 | SPI 4 MHz |
| SC16IS762IBS,128 | Active | HVQFN32 SOT617-1 | 32+EP | hvqfn32 | SPI 15 MHz, fast IrDA |
| SC16IS762IPW,112 (two census rows) | Obsolete | TSSOP28 SOT361-1 | 28 | tssop28 | SPI 15 MHz |
| SC16IS762IPW,128 | Active | TSSOP28 SOT361-1 | 28 | tssop28 | SPI 15 MHz |
Packing suffixes not in PDF (NXP 12NC packing variants). Supply/temperature same as SC16IS750.

### Pinouts (Figs. 2–3 pp. 5–6, Table 2 pp. 6–8)
**tssop28** (SOT361-1)
| pin | name | role |
|---|---|---|
| 1 | RTSA | output |
| 2 | CTSA | input |
| 3 | TXA | output |
| 4 | RXA | input |
| 5 | RESET | input (active LOW) |
| 6 | XTAL1 | input |
| 7 | XTAL2 | output |
| 8 | VDD | power |
| 9 | I2C/SPI | input (HIGH = I2C) |
| 10 | CS / A0 | input |
| 11 | SI / A1 | input |
| 12 | SO | output 3-state (n.c. in I2C mode) |
| 13 | SCL / SCLK | input |
| 14 | SDA | inout open-drain (VSS in SPI mode) |
| 15 | IRQ | output open-drain active LOW |
| 16 | CTSB | input |
| 17 | RTSB | output |
| 18 | GPIO0/DSRB | inout |
| 19 | GPIO1/DTRB | inout |
| 20 | GPIO2/CDB | inout |
| 21 | GPIO3/RIB | inout |
| 22 | VSS | ground |
| 23 | TXB | output |
| 24 | RXB | input |
| 25 | GPIO4/DSRA | inout |
| 26 | GPIO5/DTRA | inout |
| 27 | GPIO6/CDA | inout |
| 28 | GPIO7/RIA | inout |

**hvqfn32** (SOT617-1)
| pin | name | role |
|---|---|---|
| 1 | RXA | input |
| 2 | RESET | input |
| 3 | XTAL1 | input |
| 4 | XTAL2 | output |
| 5 | VDD | power |
| 6 | I2C/SPI | input |
| 7 | CS / A0 | input |
| 8 | SI / A1 | input |
| 9 | SO | output 3-state (n.c. in I2C mode) |
| 10 | SCL / SCLK | input |
| 11 | SDA | inout open-drain (VSS in SPI mode) |
| 12 | VSS | ground |
| 13 | VDD | power |
| 14 | IRQ | output open-drain |
| 15 | CTSB | input |
| 16 | RTSB | output |
| 17 | GPIO0/DSRB | inout |
| 18 | GPIO1/DTRB | inout |
| 19 | GPIO2/CDB | inout |
| 20 | GPIO3/RIB | inout |
| 21 | VSS | ground |
| 22 | TXB | output |
| 23 | RXB | input |
| 24 | GPIO4/DSRA | inout |
| 25 | GPIO5/DTRA | inout |
| 26 | GPIO6/CDA | inout |
| 27 | GPIO7/RIA | inout |
| 28 | VDD | power |
| 29 | VSS | ground |
| 30 | RTSA | output |
| 31 | CTSA | input |
| 32 | TXA | output |
| EP | VSS | exposed center pad = ground |

GPIO pins have active pull-ups (Table 36 p. 40).

### Host interface
Same as SC16IS750 (I2C address Table 32 pp. 35–36 — identical 16 addresses 0x90…0xAE; no auto-increment; FIFO bursts by repeated access; IIR burst read prohibited p. 20 note [5]), except subaddress / SPI command byte bits 2:1 = CH1,CH0: 00 = channel A, 01 = channel B, 10/11 reserved (Tables 33–34 pp. 37–38). Bit 7 = R/W in SPI (1 = read), unused in I2C; bit 0 unused. SPI mode 0, MSB first.

### Registers
Per-channel copy of the SC16IS750 register set (Table 10 pp. 19–20) for RHR/THR, IER, FCR/IIR, LCR, MCR, LSR, MSR, SPR, TCR, TLR, TXLVL, RXLVL, EFCR, DLL, DLH, EFR, XON1/2, XOFF1/2 — same bits and reset values. Shared (either CH value reaches the same register): IODir 0x0A, IOState 0x0B, IOIntEna 0x0C, IOControl 0x0E (note [7] "These registers apply to both channels").
- IOControl (Table 29 p. 29): 7:4 reserved; 3 SRESET (software reset of the device); 2 = 1 → GPIO[3:0] are RIB, CDB, DTRB, DSRB (GPIO3=RIB, GPIO2=CDB, GPIO1=DTRB, GPIO0=DSRB); 1 = 1 → GPIO[7:4] are RIA, CDA, DTRA, DSRA; 0 IOLATCH.
- MCR[0] DTR: drives DTRA (GPIO5) for channel A, DTRB (GPIO1) for channel B when selected as modem pins (p. 25 — text says "GPIO5 or GPIO1"; channel mapping by pin name).
- MSR[7:5]/[3:1] read CD/RI/DSR of the channel's modem pins when selected (p. 26).
- IIR modem (0x00) and I/O-pin (0x30) sources available on both channels (Table 6 p. 13). Which channel's IIR reports the shared GPIO interrupt is **not stated**.
- Reset values: Table 4 pp. 12–13 identical to SC16IS750.

### Behavior
Identical to SC16IS750 (pp. 9–32). Errata Appendix 18.1 (p. 51, Rev E fixes vs Rev D): Rev D can't wake from RX in IrDA sleep; Rev D RX FIFO overflow can't be cleared by FIFO reset while receiving; IIR value may be wrong if read near the X1 edge that generates the interrupt; IIR may read 0xCC (time-out) while LSR[0]=0 if host reads FIFO at the moment of time-out.

### Timing
I2C Table 37 p. 41: identical to SC16IS750. XTAL Table 38 p. 42: same (crystal ≤ 24 MHz; ext clock ≤ 48 MHz @2.5 V, 80 MHz @3.3 V; tw1/tw2 ≥ 10/6 ns). SPI Table 39 p. 44: **only one table** — the 15 MHz values (2.5 V/3.3 V: tCP 83/67, tCH/tCL 30/25, tCSS 100, tCSH 5, tDS 10, tDH 10, tDO ≤ 25/20, tTR ≤ 100, tCSW 200 ns; td9–td14 200 ns; tw(rst) 3 µs). The SC16IS752 is still limited to 4 Mbit/s SCLK by the feature list (p. 2); the PDF has no separate 4 MHz timing table for it (use SC16IS750 Table 39 values as the 752 grade).

### VCC
Same as SC16IS750: 2.3–2.7 V or 3.0–3.6 V; "5 V tolerant inputs" (p. 2); VI max 5.5 V.

### Package drawings
- TSSOP28 **SOT361-1** (p. 47): e 0.65; D 9.6–9.8; E 4.3–4.5; HE 6.2–6.6; bp 0.19–0.30; Lp 0.50–0.75; L 1.0; A max 1.1.
- HVQFN32 **SOT617-1** (p. 48): D = E 4.9–5.1 (5.0); pitch 0.5; b 0.18–0.30 (mid 0.24); L 0.3–0.5 (mid 0.4); Dh = Eh 2.95–3.25 (mid 3.1); e1 = e2 3.5; A max 1.0. Land pattern not printed.

### Obsolete
SC16IS752IPW,112; SC16IS762IPW,112 (both rows).

### Notes
- MSR text p. 26 ("Reading IOState bit 6 or bit 2 …") fine; SC16IS750's misprint fixed here.
- The SPI timing table for the 752 shows 15 MHz numbers — datasheet inconsistency with the 4 Mbit/s feature limit.

---

## PI7C9X760

- Title: "PI7C9X760 — I2C-Bus/SPI to UART Bridge Controller with 64 bytes of TX/RX FIFOs", Diodes/Pericom, Document DS40305 Rev 8-2, October 2024. File `datasheets/controller-uart/pi7c9x760.pdf`, pp. 1–47 (49 pp total).
- Relation: a functional clone of NXP SC16IS750 (block diagram labels even reuse NXP figure codes 002aab014/002aab396, p. 2; same pinout as SC16IS750IBS HVQFN24, same I2C address table, same register map 0x00–0x0F with the same bits and reset values). **Differences from SC16IS750** (so treat as a separate device / superset):
  1. Extra "special function" register bank (SFREN/SFR/ASR/CPR/RFD/RLS/TFD/TIDLE/TRCTL/ISCR) reached with LCR = 0xBF (pp. 34–36).
  2. Programmable sample rate and fractional-style prescaler: `Baud = XIN / (DL × 2^(M + 2·MCR[7] − 1) × (16 − SCR + N))` (p. 36), defaults give the SC16IS750 formula. "Baud rates up to 16 Mbit/s in 4X sampling" (p. 1).
  3. Faster interfaces: SPI up to 33 MHz @3.3 V (27 @2.5 V, 18 @1.8 V); external clock up to 64 MHz @3.3 V (p. 38–41). Supply 1.62–3.63 V.
  4. IrDA 1.1 (EFCR[7]) available (like SC16IS760).
  5. I2C: the device **NACKs a write to THR when the TX FIFO is full** (p. 19).
  6. MCR[3] = OP2 (loopback → CD) and MCR[2] doubles as OP1 (loopback → RI) (p. 29); SPR reset value 0xFF; IOState default 0xFF; DLL default 0x01 (p. 25–33).
  7. IER[7]/[6] described as "CTS/DSR" and "RTS/DTR" interrupt enables; SFR[1] auto DSR/DTR flow control.

### Parts (Ordering Information p. 47)
| Census part | Status | Decode | Package | pinout | grade |
|---|---|---|---|---|---|
| PI7C9X760CZDEX | Active | PI7C9X760 + revision letter **C** + package code **ZD** (24-contact TQFN 4×4) + **E** = Pb-free/Green + **X** = tape & reel (p. 47 notes 4–5) | ZD, 24-TQFN 4×4×0.85 (Diodes PD-2100), exposed pad | hvqfn24 (same pin map as SC16IS750IBS) | industrial −40…+85 °C; VDD 1.62–3.63 V |

The PDF's ordering table lists only **PI7C9X760DZDEX** (marking "760DZDE", p. 46). The census part has letter **C** instead of D. The PI7C9X762 revision history (pi7c9x762.pdf p. 51) shows Diodes uses this letter as the silicon/product revision ("Changed the revision to C", "Remove C from part numbers except ordering information"), so PI7C9X760CZDEX = the earlier revision-C ordering code of the same chip in the same ZD package. Decodable, but the PDF itself does not list the C code — flag.

### Pinout (p. 3 figure, pp. 4–5 table) — 24-TQFN ZD
Identical to SC16IS750 HVQFN24: 1 RESET (active LOW), 2 XTAL1, 3 XTAL2, 4 VDD, 5 I2C/SPI (HIGH = I2C), 6 CS/A0, 7 SI/A1, 8 SO (n.c. in I2C mode; figure labels it NC), 9 SCL/SCLK, 10 SDA (VSS in SPI mode), 11 IRQ (open-drain, active LOW), 12 GPIO0, 13 GPIO1, 14 GPIO2, 15 GPIO3, 16 VSS, 17 GPIO4/DSR, 18 GPIO5/DTR, 19 GPIO6/CD, 20 GPIO7/RI, 21 RTS, 22 CTS, 23 TX, 24 RX; center pad = VSS (p. 5). Roles as SC16IS750. (The pin table p. 5 misprints GPIO5/6/7 as "modem's DSR pin"; the pin names show DTR/CD/RI.) No GPIO pull-ups are mentioned.

### Host interface
- I2C target (pp. 19–21): Standard/Fast (≤400 kHz). Address table (Table 9 p. 21) = SC16IS750 Table 32 (A1/A0 ∈ VDD,VSS,SCL,SDA → 0x90…0xAE 8-bit, 0x48…0x57 7-bit). (Text says "eight possible slave addresses … two different ways to select each" — contradicts its own 16-entry table; table rules.) Sub-address (Table 10 p. 21): bit7 reserved, 6:3 A3:A0, 2:1 channel = 00 (others reserved), bit0 reserved. Write / read formats as SC16IS750 (Figs. 10–11 p. 19). ACK every byte; **NACK when the host writes THR while TX FIFO is full** (p. 19).
- SPI target (pp. 22–23): mode 0, MSB first, CS# active LOW. First byte (Table 11): bit7 R/W (1 = read), 6:3 A3:A0, 2:1 channel 00, bit0 reserved; then data. "The 64 byte TX FIFO can be loaded … or 64 byte RX FIFO … unloaded in one SPI write or read sequence" (p. 23) — continue clocking with CS low. Timing figures pp. 41–42 draw the command as `R/W A3 A2 A1 A0 0 CH X` (bit2 = 0, bit1 = CH).
- No auto-increment documented. SFR[4] "Registers burst R/W enable if set" (p. 35) — its behaviour (probably address auto-increment) is **not described**; flag.

### Registers (pp. 25–36)
Access rules exactly as SC16IS750 for the standard set (LCR[7]=0 general; LCR[7]=1 & LCR≠0xBF → DLL/DLH; LCR=0xBF → EFR/XON1/XON2/XOFF1/XOFF2; EFR[4]=1 & MCR[2]=1 → TCR/TLR at 0x06/0x07), plus SFR[2] gating: with LCR=0xBF and SFR[2]=1, addresses 0x02/0x04–0x09/0x0F show the special set instead of EFR/XON/XOFF; MSR/TCR/TXLVL/RXLVL/EFCR say "accessible when SFR[2]=0".

Standard set — same bits as SC16IS750 with these differences:
| Addr | Name | Default | Difference vs SC16IS750 |
|---|---|---|---|
| 0x00 | DLL | **0x01**, reset only at POR | "WO" in table (likely R/W) |
| 0x01 | IER | 0x00 | bit7 "CTS/DSR interrupt", bit6 "RTS/DTR interrupt"; bit4 sleep conditions: no interrupts pending, modem inputs not toggling, RX idle HIGH, TX/RX FIFO empty; exit on modem input toggle, RX going LOW, TX FIFO load; "Crystal is stopped" |
| 0x02 | FCR | 0x00 | FCR[0]=0: "Other FCR bits are not programmable, trigger level is one character" |
| 0x02 | EFR | 0x00 | bit5: if RX flow control compares Xoff2, Xoff2 is not stored and generates both Xoff and special-char interrupts; bit4 also enables IIR[5:4], TCR/TLR writes and sleep |
| 0x04 | MCR | 0x00 | bit3 **OP2** (loopback: → CD internally); bit2 **OP1/TCR-TLR enable** (loopback: → RI; else selects TCR/TLR); bit0 DTR loops to MSR[5]; MCR[7:5], MCR[3:2] need EFR[4] |
| 0x05 | LSR | 0x60 | same |
| 0x06 | MSR | 0x00 (table) | loopback: CD = MCR[3], RI = MCR[2], DSR = MCR[0], CTS = MCR[1] |
| 0x07 | SPR | **0xFF**, POR only | preserved in sleep |
| 0x0B | IOState | **0xFF** | |
| 0x0D | SFREN (LCR=0xBF) | 0x00 | write 0x5A to enable SFR access |
| 0x0E | IOControl | 0x00 | bit3 software reset "will reset all channels"; GPIO regs 0x0A–0x0E channel-independent |
| 0x0F | EFCR | 0x00 | bit7 IrDA 1.0 (3/16) / 1.1 (1/4, ≤1.152 Mbps) |
| XON1/XON2/XOFF1/XOFF2 | | 0x00, POR only | XOFF2 "accessible when LCR=0xBF and SFREN≠0x5A" (p. 32) |

Special set (LCR = 0xBF):
| Addr | Name | Access | Default | Bits |
|---|---|---|---|---|
| 0x0D | SFREN | LCR=0xBF | 0x00 | 7:0 = 0x5A enables SFR |
| 0x07 | SFR | LCR=0xBF and SFREN=0x5A | 0x00 | 7 crystal feedback resistor disable; 6 RFD/RLS select (0 RFD, 1 RLS); 5 reserved; 4 register burst R/W enable; 3 internal loopback RX→TX; 2 special register access enable (CPR, ISCR, TIDLE, TRCTL…); 1 auto DSR/DTR flow control enable; 0 force transmitter to always transmit |
| 0x02 | ASR | LCR=0xBF, SFR[2]=1 | 0x00 | RO. 7:6 reserved; 5:4 Xon/Xoff state 00 idle, 01 Xoff received, 10 TX off, 11 Xon received; 3:2 reserved; 1 remote TX disabled ("TX has sent XOFF or RTS message"); 0 transmitter terminated by remote |
| 0x04 | CPR | same | **0x10** | 7:4 CPRM (M, recommended 1 or 2); 3:0 CPRN (N) |
| 0x05 | RFD | same, SFR[6]=0 | 0x00 | RO count of data in RX FIFO |
| 0x05 | RLS | same, SFR[6]=1 | 0x00 | RO count of bytes with errors in RX FIFO |
| 0x06 | TFD | same | 0x00 | RO count of data in TX FIFO |
| 0x08 | TIDLE | same | 0x00 | transmit idle time control (units not given) |
| 0x09 | TRCTL (SCR) | same | **0x06** | 7:4 SCR sample-clock value; 3 transmit in-band Xon enable; 2 TX empty interrupt enable; 1 RX timeout enable; 0 TX idle insertion enable |
| 0x0F | ISCR | same | 0x00 | 7 CTS/RTS-change int active; 6 Xoff/special int active; 5 modem int active; 4 THR int active; 3 RX timeout int active; 2 RHR int active; 1 RX line error int active; 0 CLSTATUS: write 1 clears the interrupt status, self-clearing |
IrDA constraint: SCR = N or (16 − SCR + N) > 1 (p. 36).

### Behavior
As SC16IS750 (pp. 6–18: trigger levels, auto RTS/CTS, Xon/Xoff tables, reset Table 2, interrupts Table 4 with identical IIR codes, sleep, time-out 4 char times, break, RS-485 auto direction/inversion/multidrop/auto address) with:
- Baud: prescaler ÷1/÷4 (MCR[7]); divisor {DLH,DLL} 1…65535; sample rate = 16 − SCR + CPRN (Tables 5–8 pp. 15–17 use sample rates 16–26, e.g. 24 MHz, 1 Mbit/s → DL 1, sample 24). Full formula (p. 36): Baud = XIN / (DL × 2^(M + 2·MCR[7] − 1) × (16 − SCR + N)); with CPR default (M=1, N=0) and SCR=0 this equals XIN/(prescaler × DL × 16).
- Clock: crystal ≤ 24 MHz; external clock ≤ 64 MHz @3.3 V, 250 MHz?? @2.5 V (as printed — evidently a typo, likely 50), 24 MHz @1.8 V ±10 %, 32 MHz @1.8 V ±5 % (p. 38).
- IrDA (p. 24): MCR[6] enables; 3/16-bit HIGH pulse per "0" bit (IrDA 1.0, ≤115.2 kbps) or 1/4-bit (EFCR[7]=1, IrDA 1.1, ≤1.152 Mbps); TX idles LOW, RX idles LOW; each received light pulse → logic 0 data bit (text: "returns a logic 1"? — p. 24 says "Each time it senses a light pulse, it returns a logic 1 to the data bit stream" — contradicts encoding; flag). Mentions an "ENIR# pin" for IrDA at power-up — no such pin exists on this package (copy-paste from another Exar-style datasheet).
- RESET: active LOW, pulse > 40 ns (p. 4) (I2C timing table still lists TD15 SCL delay after reset 3 µs).
- Reset values: Table 2 p. 10 (same as SC16IS750); DLL/DLH/SPR/XON/XOFF reset only by POR.
- Outputs after reset: TX HIGH, RTS HIGH, I/Os inputs, IRQ released (Table 3 p. 10).

### Timing (pp. 38–42), TA −40…+85 °C
- I2C: as SC16IS750 Fast mode (fSCL ≤ 400 kHz, tBUF 1.3 µs, tHD;STA 0.6, tSU;STA 0.6, tSU;DAT 150 ns, tHD;DAT 0, tLOW 1.3, tHIGH 0.6, tVD 0.6 µs); tSP 100 ns; TD1–TD7 0.2 µs; TD8 0.5 µs (fast) / 1.0 µs (std); TD15 3 µs.
- SPI (p. 41): 1.8 / 2.5 / 3.3 V: fSCLK max 18 / 27 / 33 MHz; TCP 56/36/30 ns; TCH,TCL 28/18/15; TDS 6.0/5.0/4.0; TDH 0; TDO max 22/13/11 ns (70 pF); TCSS 100; TCSH 20; TTR 100; TCSW 200 ns; TD9–TD14 200 ns.

### VCC
1.62–3.63 V (1.8 V ±10 %, 2.5 V ±10 %, 3.3 V ±10 %), −40…+85 °C (p. 37). VIH max 5.5 V on inputs ("5.5 V steady voltage tolerance … only when supply present") → inputMax 5.5 V.

### Package drawing
24-TQFN **ZD (ZD24)**, Diodes PD-2100 rev C (p. 47, outline + recommended land pattern on same page): body D = E 3.90/4.00/4.10; pitch 0.50 BSC; b 0.18/0.25/0.30; L 0.35/0.40/0.45; exposed pad D2 = E2 2.15–2.75 (no nom; note 6 "major EDAP 2.25×2.25"); A 0.80/0.85/0.90; A1 0–0.05; A3 0.20 ref; pin-1 chamfer on pad C0.35×45°. Land pattern: pads 0.70 × 0.28 (24×), pitch 0.50, pad-row span 3.86 (outer-to-outer? printed "3.86" across), thermal land 2.30 × 2.30.

### Obsolete
None.

### Notes
- Ordering code letter C vs D (see Parts).
- IRQ pull-up text says "1 ohm / 1.5 ohm" (should be kΩ).
- Text "eight possible slave addresses" vs 16-row table.
- ENIR# pin reference and IrDA "returns a logic 1" text are inconsistent with the device.

---

## PI7C9X762

- Title: "PI7C9X762 — I2C-Bus/SPI to Dual UART Bridge Controller with 64 bytes of TX/RX FIFOs", Document DS40306 Rev 10-2, January 2026. File `datasheets/controller-uart/pi7c9x762.pdf`, pp. 1–49 (51 pp).
- Relation: the dual-channel version of PI7C9X760, a functional clone of SC16IS752. Register descriptions are word-for-word the PI7C9X760's (text diff), with channel select 00 = A, 01 = B (Tables 10–11, p. 21–22). GPIO registers 0x0A–0x0E are channel independent; software reset resets all channels (p. 33). Pinout identical to SC16IS752IBS HVQFN32. Same DC/AC tables as PI7C9X760 (SPI 18/27/33 MHz, I2C 400 kHz, crystal 24 MHz, ext clock 64 MHz @3.3 V). "Industrial temperature range −40 °C to 85 °C" (p. 1).

### Parts (Ordering Information p. 49)
| Census part | Status | Decode | Package | pinout | grade |
|---|---|---|---|---|---|
| PI7C9X762Q2ZHEX | Active | **Not covered by the PDF.** Ordering table lists only **PI7C9X762DZHEX** (ZH = 32-contact TQFN W-QFN5050-32, E = green, X = tape & reel). The census part keeps the **ZH** package code and E/X suffixes; "Q2" in place of the revision letter is not defined anywhere in pi7c9x762.pdf. p. 1 only says "For automotive applications requiring specific change control (AEC-Q100…, PPAP) … contact us". Most likely the automotive-qualified (AEC-Q100, probably grade 2 = −40…+105 °C — not stated) version of the same die in the same ZH package. | ZH, 32-TQFN 5×5 (PD-2070) — package decodable | hvqfn32 (same pin map as SC16IS752IBS) | supply 1.62–3.63 V per this PDF; automotive temperature grade unknown |

Census row description "SPI BRIDGE W-QFN5050-32" matches the ZH package name. **pi7c9x762.pdf does not cover PI7C9X762Q2ZHEX by part number**; it covers the function/pinout/package of the industrial PI7C9X762DZHEX.

### Pinout (p. 3) — 32-TQFN ZH
1 RXA, 2 RESET, 3 XTAL1, 4 XTAL2, 5 VDD, 6 I2C/SPI, 7 CS/A0, 8 SI/A1, 9 SO (NC in I2C), 10 SCL/SCLK, 11 SDA (VSS in SPI), 12 VSS, 13 VDD, 14 IRQ, 15 CTSB, 16 RTSB, 17 GPIO0/DSRB, 18 GPIO1/DTRB, 19 GPIO2/CDB, 20 GPIO3/RIB, 21 VSS, 22 TXB, 23 RXB, 24 GPIO4/DSRA, 25 GPIO5/DTRA, 26 GPIO6/CDA, 27 GPIO7/RIA, 28 VDD, 29 VSS, 30 RTSA, 31 CTSA, 32 TXA; exposed pad = VSS. Roles as SC16IS752.

### Host interface / Registers / Behavior / Timing / VCC
Same as PI7C9X760 above, per channel (CH bits 2:1 = 00 A, 01 B; 10/11 reserved). IOControl[2] → GPIO[3:0] = RIB, CDB, DTRB, DSRB; IOControl[1] → GPIO[7:4] = RIA, CDA, DTRA, DSRA. SPI pages 41–42 identical; I2C table p. 38 identical.

### Package drawing
32-TQFN **ZH (ZH32)**, W-QFN5050-32, Diodes PD-2070 rev E (p. 48; outline + recommended land pattern): D = E 4.90/5.00/5.10; pitch 0.50 BSC; b 0.18/0.25/0.30; L 0.35/0.40/0.45; D2 = E2 3.15/3.25/3.35; A 0.70/0.75/0.80; A1 0/0.02/0.05; A3 0.203 ref; pad chamfer C0.35×45°. Land pattern: 0.70 × 0.27 pads (32×), pitch 0.50, pad span 4.10, thermal land 3.25 × 3.25. MSL 1.

### Obsolete
None.

### Notes
- Census part number not in PDF (Q2 code) — see Parts.

---

## XR20M1170

- Title: "XR20M1170 — I2C/SPI UART with 64-Byte FIFO", MaxLinear (Exar), Rev 1.1.3, 2 Feb 2022, doc 233DSR00. Every page carries the watermark "The product(s) mentioned in this data sheet are Not Recommended for New Designs (NRND)". File `datasheets/controller-uart/xr20m1170.pdf`, pp. 1–60.
- Relation: SC16IS750-like register map (16 addresses A3:A0, same names TXLVL/RXLVL/IODir/IOState/IOIntEna/IOControl/EFCR, TCR/TLR) **plus** Exar features: fractional divisor DLD (1/16 steps) with 16X/8X/4X sampling, NACK on TX-FIFO-full write, different I2C address base (0x60), not 5 V tolerant, 1.62–3.63 V. Differs from SC16IS750 in reset values (LCR 0x1D same; TCR 0x0F; SPR 0xFF), IOControl[2] reserved (single channel), sleep stops the oscillator, interrupt clearing rules.

### Parts (Ordering Information p. 3, image-checked)
Ordering code: XR20M1170 + **I** (industrial −40…+85 °C) + package **L24** = 24-pin QFN, **L16** = 16-pin QFN, **G24** = 24-lead TSSOP, **G16** = 16-lead TSSOP + optional **TR** (tape & reel; otherwise tray for QFN / tube for TSSOP) + **-F** (lead-free).
| Census part | Status | Package | pins | pinout | grade |
|---|---|---|---|---|---|
| XR20M1170IG16-F | Not For New Designs | 16-lead TSSOP 4.4 mm (POD-00000117), tube | 16 | tssop16 | −40…+85 °C, VCC 1.62–3.63 V |
| XR20M1170IG24-F | Obsolete | 24-lead TSSOP 4.4 mm (POD-00000058), tube | 24 | tssop24 | same |
| XR20M1170IL24-F | Obsolete | 24-pin QFN 4×4×0.9 (POD-00000130), tray | 24+pad | qfn24 | same |
| XR20M1170IL24TR-F | Obsolete | same, tape & reel | 24+pad | qfn24 | same |
(The PDF also lists XR20M1170IG16TR-F, IL16-F, IL16TR-F — not in census.)

### Pinouts (Fig. 2 p. 2; pin table pp. 3–5)
Names: `A0_CS` (I2C address A0 / SPI CS# active LOW, Schmitt), `A1_SI`, `SO`, `SCL` (I2C clock, idles HIGH / SPI clock, idles LOW), `SDA`, `I2C_SPI` (HIGH = I2C, LOW = SPI; table types it I/O), `IRQ` (open-drain active LOW), `RESET` (active LOW, >40 ns), `RTS`, `CTS`, `DTR`, `DSR`, `CD`, `RI` (all active LOW).
| signal | role | qfn24 | tssop24 | tssop16 | (qfn16) |
|---|---|---|---|---|---|
| SO | output 3-state (I2C mode: leave unconnected) | 1 | 4 | 3 | 1 |
| GPIO0 | inout | 2 | 5 | – | – |
| GPIO1 | inout | 3 | 6 | – | – |
| I2C/SPI# | input | 4 | 7 | 4 | 2 |
| RX | input | 5 | 8 | 5 | 3 |
| TX | output | 6 | 9 | 6 | 4 |
| GPIO2 | inout | 7 | 10 | – | – |
| XTAL1 | input | 8 | 11 | 7 | 5 |
| XTAL2 | output | 9 | 12 | 8 | 6 |
| GPIO3 | inout | 10 | 13 | – | – |
| GND | ground | 11 | 14 | 9 | 7 |
| SDA | inout open-drain (SPI mode: **connect to VCC**) | 12 | 15 | 10 | 8 |
| SCL | input | 13 | 16 | 11 | 9 |
| IRQ# | output open-drain | 14 | 17 | 12 | 10 |
| RTS# | output | 15 | 18 | 13 | 11 |
| GPIO5/DTR# | inout | 16 | 19 | – | – |
| GPIO4/DSR# | inout | 17 | 20 | – | – |
| RESET# | input | 18 | 21 | 14 | 12 |
| CTS# | input | 19 | 22 | 15 | 13 |
| GPIO6/CD# | inout | 20 | 23 | – | – |
| GPIO7/RI# | inout | 21 | 24 | – | – |
| VCC | power | 22 | 1 | 16 | 14 |
| A0/CS# | input | 23 | 2 | 1 | 15 |
| A1/SI | input | 24 | 3 | 2 | 16 |
| center pad | — **not electrically connected inside**; solder, optionally to GND (p. 5) | PAD | – | – | PAD |
16-pin parts have no GPIO/DTR/DSR/CD/RI pins (registers still exist). RX: tie HIGH (100 kΩ) if unused.

### Host interface
- **I2C** (pp. 7–9): Standard/Fast, ≤400 kHz. Address Table 1 (p. 9) — only **8 distinct addresses**; A1 = SCL behaves like VCC and A1 = SDA like GND:

| A1 | A0 | 8-bit | 7-bit |
|---|---|---|---|
| VCC or SCL | VCC | 0x60 | 0x30 |
| VCC or SCL | GND | 0x62 | 0x31 |
| VCC or SCL | SCL | 0x64 | 0x32 |
| VCC or SCL | SDA | 0x66 | 0x33 |
| GND or SDA | VCC | 0x68 | 0x34 |
| GND or SDA | GND | 0x6A | 0x35 |
| GND or SDA | SCL | 0x6C | 0x36 |
| GND or SDA | SDA | 0x6E | 0x37 |

  Sub-address (Table 2): bit7 reserved; 6:3 A3:A0; 2:1 channel 00 (others reserved); bit0 reserved. Write `S addr W A subaddr A data… P`; read `S addr W A subaddr A Sr addr R A data A … last NA P` (Figs. 4–5). **Device ACKs every write byte but NACKs a byte written to THR when the TX FIFO is full** (p. 7). No auto-increment (FIFO burst = repeated access to same subaddress implied).
- **SPI** (pp. 10–11): SCL idles LOW; first byte bit7 R/W# (1 = read), 6:3 A3:A0, 2:1 = 00, bit0 X (Table 3, Figs. 7–8 show `R/W A3 A2 A1 A0 0 0 X`); then data byte(s) on SI (write) or SO (read). Whole 64-byte TX/RX FIFO can be written/read in one CS# low sequence (Figs. 9–10). MSB first; SO valid after SCL falling edge (TDO), SI sampled on rising (mode 0 implied by timing; mode number not stated). Max 18 MHz @3.3 V, 16 MHz @2.5 V, 8 MHz @1.8 V.

### Registers (Tables 7–8 pp. 24–26; descriptions pp. 26–41; reset Table 16 p. 41)
Access: general set LCR[7]=0; DLL 0x00 / DLM 0x01 when LCR[7]=1 and LCR≠0xBF; **DLD 0x02** when LCR[7]=1, LCR≠0xBF and EFR[4]=1; EFR 0x02, XON1 0x04, XON2 0x05, XOFF1 0x06, XOFF2 0x07 when LCR=0xBF. Offset 0x06 = TCR and 0x07 = TLR only when EFR[4]=1 and MCR[2]=1 (Tables 12–13 p. 33), else MSR/SPR. "Shaded" (EFR[4]-gated) bits: IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5] and MCR[2], TCR, TLR, DLD — with EFR[4]=0 they are latched (keep their value) (p. 40).

| Addr | Name | R/W | Reset | Bits |
|---|---|---|---|---|
| 0x00 | RHR / THR | R / W | xx | data |
| 0x01 | IER | R/W | 0x00 | 7 CTS# int en*; 6 RTS# int en*; 5 Xoff int en*; 4 sleep mode en*; 3 modem status int; 2 RX line status int; 1 TX empty (THR) int; 0 RX data int |
| 0x02 | ISR | R | 0x01 | 7:6 FIFOs enabled (both = FCR[0]); 5:0 interrupt code (Table 9) |
| 0x02 | FCR | W | 0x00 | 7:6 RX trigger 00=8, 01=16, 10=56, 11=60; 5:4 TX trigger* 00=8, 01=16, 10=32, 11=56 spaces; 3 reserved (legacy DMA, no function); 2 TX FIFO reset; 1 RX FIFO reset; 0 FIFO enable (must be 1 when writing other FCR bits; resets need ≥3 XTAL clocks and are only active with FCR[0]=1) |
| 0x03 | LCR | R/W | 0x1D | 7 divisor enable; 6 set break; 5 forced parity; 4 even parity; 3 parity enable; 2 stop bits; 1:0 word length (5,6,7,8). Reset 0x1D = 6 bits, 2 stop, even parity enabled (datasheet's own "default" markers p. 31) |
| 0x04 | MCR | R/W | 0x00 | 7 clock prescaler ÷4*; 6 IR mode enable*; 5 Xon-Any*; 4 internal loopback; 3 OP2# (internal only; →CD in loopback); 2 OP1# (→RI in loopback) / TCR-TLR enable*; 1 RTS# (1 = LOW); 0 DTR# (1 = LOW) |
| 0x05 | LSR | R | 0x60 | 7 RX FIFO global error; 6 THR & TSR empty; 5 THR empty; 4 break; 3 framing; 2 parity; 1 overrun; 0 data ready |
| 0x06 | MSR | R | 7:4 = inverted inputs | 7 CD; 6 RI; 5 DSR; 4 CTS; 3 ΔCD; 2 ΔRI (RI# LOW→HIGH); 1 ΔDSR; 0 ΔCTS |
| 0x07 | SPR | R/W | 0xFF (power-up only) | scratch, kept in sleep |
| 0x06 | TCR* | R/W | **0x0F** | 7:4 resume level ×4; 3:0 halt level ×4 (0–60) |
| 0x07 | TLR* | R/W | 0x00 | 7:4 RX trigger ×4 (0 → FCR); 3:0 TX trigger ×4 (0 → FCR) |
| 0x08 | TXLVL | R | 0x40 | 6:0 spaces (0–64), bit7 0 |
| 0x09 | RXLVL | R | 0x00 | 6:0 chars (0–64) |
| 0x0A | IODir | R/W | 0x00 | 1 = output |
| 0x0B | IOState | R/W | 0x00 | read pin states / write outputs |
| 0x0C | IOIntEna | R/W | 0x00 | per-input interrupt enable; bits 7:4 ignored when IOControl[1]=1 |
| 0x0D | reserved | – | 0x00 | |
| 0x0E | IOControl | R/W | 0x00 | 7:4 reserved; 3 UART software reset (self-clearing); 2 reserved; 1 GPIO[7:4] = modem RI#, CD#, DTR#, DSR#; 0 IOLatch |
| 0x0F | EFCR | R/W | 0x00 | 7 fast IR (IrDA 1.1, ¼ pulse); 6 reserved; 5 auto-RS485 RTS# invert; 4 auto RS-485 enable; 3 reserved; 2 TX disable; 1 RX disable; 0 9-bit (multidrop) mode |
| 0x00 (LCR[7]=1) | DLL | R/W | 0x01 (power-up only) | |
| 0x01 (LCR[7]=1) | DLM | R/W | 0x00 (power-up only) | |
| 0x02 (LCR[7]=1, EFR[4]=1) | DLD | R/W | 0x00 | 7:6 reserved; 5:4 sampling 00 = 16X, 01 = 8X, 1x = 4X; 3:0 fractional divisor (n/16) |
| 0x02 (LCR=0xBF) | EFR | R/W | 0x00 | 7 auto CTS; 6 auto RTS; 5 special char detect; 4 enhanced function enable; 3:0 software flow control (Table 15 = same encoding as SC16IS750) |
| 0x04–0x07 (LCR=0xBF) | XON1, XON2, XOFF1, XOFF2 | R/W | 0x00 (power-up only) | |
(* = requires EFR[4]=1 to modify.)

ISR codes (Table 9 p. 29), priority: 1 `000110` LSR; 2 `001100` RX time-out; 3 `000100` RX data ready; 4 `000010` TX ready; 5 `000000` MSR; 6 `110000` GPIO; 7 `010000` Xoff/special char; 8 `100000` CTS#/RTS# change (LOW→HIGH); `000001` none.
Generation/clear (pp. 28): LSR int from LSR bits 1,2,3,4,7 — cleared by reading all error characters out of FIFO; RXRDY by trigger — cleared when FIFO below trigger; time-out after "4 char + 12 bits" — cleared by reading RHR; TXRDY: TX FIFO empty (FIFO off) / spaces above trigger / transmitter empty in auto RS-485 — cleared by reading ISR or writing THR; MSR — read MSR; GPIO — read IOState; Xoff — cleared by Xon received; special char — read ISR; CTS#/RTS# — read MSR. IER[1]: "If the THR is empty when this bit is enabled, an interrupt will be generated" (p. 27) — unlike SC16IS750 note.

### Behavior
- **Clock/baud** (pp. 12–14): crystal 1.8432–24 MHz; external clock ≤64 MHz @3.3 V, 50 MHz @2.5 V, 24 MHz @1.8 V ±10 % (32 MHz @1.8 V ±5 %) (p. 43). Prescaler MCR[7] ÷1/÷4. Divisor = DLM:DLL + DLD[3:0]/16 (1 … 65535.9375). Data rate = fXTAL / prescaler / (divisor × sampling) where sampling = 16, 8, 4 (DLD[5:4]). Jitter: 8X with odd DLD → ±1/16 bit; 4X with DLD odd and not multiple of 4 → ±1/8 bit (exact fractional distribution not documented). Divisor default 1 at power-up.
- **Receiver** (p. 16): start bit detected on falling edge, validated at half bit (8/4/2 sampling clocks), data/stop sampled mid-bit. Time-out = 4 word lengths (LCR[1:0]) + 12 bit times (≈3.7–4.6 char times). Break: only one break character loaded into FIFO (p. 34). Overrun: new data overwrites the RSR, not put in FIFO.
- **Transmitter**: LSB first; LSR[5] set when FIFO empty; LSR[6] when FIFO+TSR empty.
- **IRQ# pin** (Tables 4–5 p. 11): LOW while any enabled interrupt pending; open drain. In auto RS-485, TX interrupt = transmitter (TSR) empty in non-FIFO mode.
- **Auto RTS** (p. 17): EFR[6]; must first assert RTS# (MCR[1]=1). RTS# HIGH when RX FIFO ≥ TCR[3:0]×4 (halt), LOW when ≤ TCR[7:4]×4 (resume). IER[6] interrupt on RTS# LOW→HIGH (ISR[5]).
- **Auto CTS** (p. 17–18): EFR[7]; TX stops after current character's stop bit when CTS# HIGH; IER[7] interrupt on CTS# LOW→HIGH.
- **Xon/Xoff** (p. 19): as SC16IS750; flow control characters are **not** placed in RX FIFO; Xoff sent at halt level, Xon at resume level. EFR[5] special char = XOFF2 compared using word length bits; if EFR[1:0]=01 (Xon2/Xoff2 compare) Xoff2 is not stored and gives both interrupts (p. 40). Xon-Any (MCR[5]).
- **Auto RS-485** (p. 19): EFCR[4]: RTS# LOW (transmit) when data loaded into THR, de-asserted after last stop bit; EFCR[5] inverts. Multidrop EFCR[0] with LCR[5:3]=111: normal mode (EFR[5]=0) and auto-address (EFR[5]=1, XOFF2 = address) exactly as SC16IS750 (pp. 19–20).
- **IrDA** (p. 21): MCR[6]; 3/16-bit HIGH pulse per 0 bit (≤115.2 kbps) or ¼-bit (EFCR[7]=1, ≤1.152 Mbps). TX and RX idle LOW in IR mode. (Text "returns a logic 1" on a light pulse, but Fig. 18 shows a pulse decoded as 0.)
- **Sleep** (p. 22): IER[4]=1 (EFR[4]); enters when no interrupt pending, modem inputs not toggling (MSR[3:0]=0), RX idle HIGH (FIFO-empty not listed). Stops the crystal oscillator (XTAL2 no clock). Wakes on RX start bit (HIGH→LOW), byte written to THR/FIFO, change on CTS#/DSR#/CD#/RI#; returns to sleep automatically after interrupts are serviced; modem wake needs MSR read. First characters after wake may be lost.
- **Loopback** (p. 23): TX→RX internal; TX, RTS#, DTR# pins **held** (TX HIGH); CTS#, DSR#, CD#, RI# inputs ignored; MSR[4]=MCR[1], MSR[5]=MCR[0], MSR[6]=MCR[2], MSR[7]=MCR[3]. Auto RTS/CTS not supported in loopback.
- **GPIO** (p. 37): IODir/IOState/IOIntEna/IOLatch as SC16IS750. IOIntEna[7:4] ignored when IOControl[1]=1. (Text says "If IOControl[0] = 1 (GPIO pins are selected as modem IOs)" — typo for IOControl[1].)
- **Reset** (Table 16 p. 41): RESET# >40 ns LOW resets registers and outputs; values in table above. DLL/DLM, SPR, XON/XOFF reset only at power-up (not by RESET# nor IOControl[3]). Outputs: TX HIGH, RTS# HIGH, DTR# HIGH, IRQ# HIGH (released). IOControl[3] text: "will reset the modem IO select register bits" (p. 37) — scope of software reset unclear.

### Timing (pp. 43–52)
- I2C (p. 44): fSCL ≤ 400 kHz; tBUF 1.3 µs; tHD;STA 0.6; tSU;STA 0.6; tHD;DAT 0; tSU;DAT 150 ns (Fast) / 250 ns (Std); tLOW 1.3; tHIGH 0.6; tVD;ACK/tVD;DAT ≤ 0.6 µs; **tSP 0.5 µs**(as printed, likely 50 ns); TD1–TD7 0.2 µs; TD8 0.5 µs (Fast) / 1.0 (Std); TD15 SCL delay after reset 3 µs.
- SPI (p. 48), 1.8 / 2.5 / 3.3 V: fSCL ≤ 8 / 16 / 18 MHz; TCP ≥ 125 / 63 / 55 ns; TCH,TCL ≥ 62 / 31 / 27; TDS ≥ 30 / 20 / 15; TDH ≥ 10; TDO ≤ 30 / 20 / 15 ns (70 pF); TCSS ≥ 100; TCSH ≥ 20; TTR ≤ 100; TCSW ≥ 200 ns; TD9–TD14 200 ns.

### VCC
1.62–3.63 V (1.8/2.5/3.3 V ±10 %), −40…+85 °C (p. 42). **Not 5 V tolerant**: abs max any pin GND−0.3 … 4 V; VIH max = VCC (p. 42); XTAL1 explicitly not 5 V tolerant (p. 12). inputMax = VCC (≤ 3.63 V).

### Package drawings
- 24-pin QFN 4×4×0.9, **POD-00000130 rev B** (outline p. 53, land pattern/stencil p. 54): D = E 4.00 BSC; e 0.50; b 0.18/0.25/0.30; L 0.35/0.40/0.45; D2 = E2 2.50/2.65/2.80; A 0.80/0.90/1.00; A1 0/0.02/0.05; A3 0.20 ref; K ≥ 0.20. Land pattern: pad 0.30 × 0.65 (approx., p. 54), pitch 0.50, thermal land 2.70 × 2.70; stencil 2.50 × 2.50.
- 24-lead TSSOP 4.4 mm, **POD-00000058 rev D** (outline p. 57, land pattern p. 58): D 7.70/7.80/7.90; E 6.40 BSC; E1 4.30/4.40/4.50; e 0.65; b 0.19–0.30; L 0.45/0.60/0.75; L1 1.0 ref; A ≤ 1.20; A1 0.05–0.15; A2 0.80/1.00/1.05; c 0.09–0.20.
- 16-lead TSSOP 4.4 mm, **POD-00000117 rev A** (p. 59, JEDEC MO-153 AB, no land pattern page): D 4.90/5.00/5.10; E 6.40 BSC; E1 4.30/4.40/4.50; e 0.65; b 0.19–0.30; L 0.45/0.60/0.75; L1 1.00 ref; A ≤ 1.20; A1 0.05–0.15; A2 0.80/1.00/1.05; c 0.09–0.20.

### Obsolete
XR20M1170IG24-F, XR20M1170IL24-F, XR20M1170IL24TR-F (Obsolete); XR20M1170IG16-F is "Not For New Designs".

### Notes
- I2C address table: only 8 unique addresses (SCL ≡ VCC, SDA ≡ GND for A1).
- tSP printed 0.5 µs (unit glitch). TVD;DAT unit printed "ns" with 0.6.
- LCR reset 0x1D yet bit descriptions call 6-bit/2-stop/even parity the "default" — consistent with 0x1D.

---

## XR20M1280

- Title: "XR20M1280 — I2C/SPI UART with 128-byte FIFO and Integrated Level Shifters", MaxLinear Rev 1.0.1, 5 Jul 2021, doc 214DSR01. File `datasheets/controller-uart/xr20m1280.pdf`, pp. 1–62.
- Relation / differences from XR20M1170: a different register architecture — the **XR16M1280-style 16550 map with only 8 addresses (A2:A0)** and many bank-switched enhanced registers (FCTR, EMSR, FC, TRG, SHR, SFR, GPIOLVL, GPIOINT, GPIO3T, GPIOINV, GPIOSEL, DREV) instead of the SC16IS-style TXLVL/RXLVL/IODir/… at 0x08–0x0F; 128-byte FIFOs; four selectable trigger tables A–D; independent TX/RX baud generators; RS-485 setup/turn-around delays; TX address-bit for 9-bit mode; SLEEP/PWRDN# pin; EN485# and ENIR# power-up straps; four supply rails (VCC_CORE, VCC_BUS, VCC_UART, VCC_GPIO); 5 V tolerant inputs (except XTAL1); SPI to 26 MHz. Same I2C address table (0x60–0x6E, 8 unique) and NACK-on-TX-full.

### Parts (Ordering p. 2)
| Census part | Status | Package | pins | pinout | GPIOs | grade |
|---|---|---|---|---|---|---|
| XR20M1280IL24TR-F | Active | QFN-24 4×4×0.9 (POD-00000130), tape & reel | 24+pad | qfn24 | 4 (GPIO0–3) | −40…+85 °C; each VCC 1.62–3.63 V |
| XR20M1280IL32-F | Active | QFN-32 5×5×0.9 (POD-00000037), tray | 32+pad | qfn32 | 8 | same |
(XR20M1280IL24-F tray also listed, not in census.)

### Pinouts (Fig. 2 p. 3; table pp. 4–5)
Mode-dependent pins as `I2C / SPI` meaning.
**qfn32**: 1 I2C/SPI# (in), 2 ENIR# (in, sampled at power-up), 3 NC / SO (out, 3-state), 4 SDA (inout OD) / tie LOW (GND) in SPI, 5 SCL (in), 6 IRQ# (OD out), 7 A1 / SI (in), 8 A0 / CS# (in), 9 RESET# (in), 10 SLEEP/PWRDN# (inout: SLEEP input after power-up, or PWRDN# output when FCTR[0]=1), 11 EN485# (in, sampled at power-up), 12 GND, 13 VCC_BUS (power), 14 NC, 15 NC, 16 VCC_GPIO (power, for GPIO[7:4]), 17 GPIO7, 18 GPIO6, 19 GPIO5, 20 GPIO4 (inout), 21 VCC_UART (power, UART pins + GPIO[3:0]), 22 RI#/GPIO3, 23 CD#/GPIO2, 24 DSR#/GPIO1, 25 DTR#/GPIO0 (inout), 26 CTS# (in), 27 RTS# (out), 28 TX (out), 29 RX (in), 30 VCC_CORE (power), 31 XTAL1 (in, not 5 V tolerant), 32 XTAL2 (out); center pad = GND.
**qfn24**: 1 NC / SO, 2 SDA / (GND), 3 SCL, 4 IRQ#, 5 A1 / SI, 6 A0 / CS#, 7 RESET#, 8 SLEEP/PWRDN#, 9 GND, 10 VCC_BUS, 11 EN485#, 12 CTS#, 13 TX, 14 RX, 15 RTS#, 16 VCC_UART, 17 RI#/GPIO3, 18 CD#/GPIO2, 19 DSR#/GPIO1, 20 DTR#/GPIO0, 21 VCC_CORE, 22 XTAL1, 23 XTAL2, 24 I2C/SPI#; center pad = GND. (No ENIR#, no GPIO4–7, no VCC_GPIO.)
Note the pin table says SO is pin 1 (QFN-24) / 3 (QFN-32) and in I2C mode "must be left unconnected" (figure labels NC); SDA in SPI mode "should be tied LOW".

### Host interface
- I2C (pp. 6–7): ≤400 kHz; address map identical to XR20M1170 (0x60…0x6E, 8 unique; Table 1). Sub-address (Table 2): bits 7:6 reserved; **5:3 = A2:A0**; 2:1 channel 00; bit0 reserved. NACK when writing THR with TX FIFO full. Formats as XR20M1170.
- SPI (pp. 8–9): first byte bit7 R/W# (1 = read); bit6 reserved (0); 5:3 A2:A0; 2:1 = 00; bit0 X. Max 26 MHz. Whole 128-byte FIFO in one CS# sequence. SCL idle LOW.

### Registers (Tables 9–10 pp. 26–28; descriptions pp. 29–47; reset Table 21 p. 48)
Bank selection (A2:A0):
- LCR[7]=0: 000 RHR(R)/THR(W); 001 IER; 010 ISR(R)/FCR(W) (with EFR[4]=0 also reachable when LCR≠0xBF); 011 LCR; 100 MCR; 101 LSR(R)/**SHR(W)**; 110 MSR(R)/**SFR(W, needs EFR[4])**; 111 SPR (FCTR[6]=0, SFR[0]=0) / **GPIOLVL** (FCTR[6]=0, SFR[0]=1) / **EMSR(W)** and **FC(R)** (FCTR[6]=1).
- LCR[7]=1 and LCR≠0xBF: 000 DLL (or **DREV** read-only device revision when DLL=DLM=0x00); 001 DLM; 010 DLD (EFR[4]=1). With DLD[6]=1, DLD[7] selects whether DLL/DLM/DLD[5:0] writes go to the TX (0) or RX (1) generator (Table 18).
- LCR = 0xBF: 000 FC(R)/TRG(W); 001 FCTR; 010 EFR; 100–111 XON1, XON2, XOFF1, XOFF2 (SFR[0]=0) or GPIOINT, GPIO3T, GPIOINV, GPIOSEL (SFR[0]=1). 011 = LCR.

| Reg | Reset | Bits |
|---|---|---|
| IER | 0x00 | 7 CTS# int*; 6 RTS# int*; 5 Xoff int*; 4 sleep*; 3 modem; 2 RX line status; 1 TX empty; 0 RX data |
| ISR | 0x01 | 7 FIFO enabled; 6 GPIO interrupt status (≠ ISR[7] when GPIO int pending; = ISR[7] otherwise, Table 12); 5:0 code |
| FCR (W) | 0x00 | 7:6 RX trigger select; 5:4 TX trigger select*; 3 wake-up interrupt enable*; 2 TX FIFO reset; 1 RX FIFO reset; 0 FIFO enable (must be 1 to write other bits) |
| LCR | **0x00** | as 16550 (reset 0x00 = 5 bits, 1 stop, no parity) |
| MCR | 0x00 | 7 prescaler ÷4*; 6 IrDA enable*; 5 Xon-Any*; 4 loopback; 3 OP2# (loopback→CD); **2 GPIO[3:0] select: 0 = modem RI#, CD#, DTR#, DSR# (default), 1 = GPIO** (also OP1#→RI in loopback); 1 RTS#; 0 DTR# |
| LSR | 0x60 | as 16550 (7 FIFO error, 6 TEMT, 5 THRE, 4 BI, 3 FE, 2 PE, 1 OE, 0 DR). Break indication remains until RX returns HIGH |
| SHR (W) | 0x00 | 7:4 RS-485 setup delay (0–15 bit times RTS# asserted before first byte); 3:0 RS-485 turn-around delay (0–15 bits after last byte) / auto-RTS hysteresis select with Table-D (Table 15: 0000 0, 0001 ±4, 0010 ±6, 0011 ±8, 0100 ±8, 0101 ±16, 0110 ±24, 0111 ±32, 1000 ±40, 1001 ±44, 1010 ±48, 1011 ±52, 1100 ±12, 1101 ±20, 1110 ±28, 1111 ±36 chars) |
| MSR | 7:4 = NOT inputs | as 16550 (ΔRI = RI# LOW→HIGH) |
| SFR (W, EFR[4]) | 0x00 | 7 TX address bit (9th bit = 1 for next THR byte, self-clearing; needs LCR[5:3]=111); 6 9-bit mode enable; 5 RX disable; 4 TX disable; 3 fast IR (IrDA 1.1); 2 GPIO interrupt enable; 1 reserved (0); 0 GPIO register access enable |
| SPR | 0xFF | scratch (kept in sleep; reset to 0xFF by reset and power cycle) |
| GPIOLVL | 0x00 | read: input pin states (inverted per GPIOINV) / output latch; write: output latch |
| EMSR (W, FCTR[6]=1) | 0x00 | 7 Xoff/special int mode (1 = cleared only by ISR read); 6 LSR int immediate (1) vs delayed (0); 5 reserved; 4 TX, RTS#, DTR# outputs 3-state; 3 invert RTS# in RS-485 (1 = HIGH during TX); 2 send TX immediately (next byte goes straight to TSR, self-clearing); 1:0 FIFO count select: x0 RX count, 01 TX count, 11 alternate RX/TX on successive reads |
| FC (R) | 0x00 | FIFO level count (RX if FCTR[7]=0, TX if 1 at LCR=0xBF; via SPR per EMSR[1:0] when FCTR[6]=1) |
| TRG (W) | **0x01** | programmable trigger level (Table-D); FCTR[7] = 0 RX, 1 TX |
| FCTR | 0x00 | 7 TRG/FC select RX(0)/TX(1); 6 scratchpad swap; 5:4 trigger table A/B/C/D; 3 auto RS-485 direction control; 2 IrDA RX inversion (1 = RX idles HIGH); 1 reserved; 0 SLEEP(0, input)/PWRDN#(1, output) |
| EFR | 0x00 | 7 auto CTS; 6 auto RTS; 5 special char; 4 enhanced enable (enables IER[7:4], ISR[5:4], FCR[5:3], MCR[7:5], DLD, SHR, SFR writes); 3:0 software flow control (same table) |
| DLL/DLM | 0x01/0x00 (power-up only), both TX & RX | |
| DLD | 0x00 | 7 BRG select (TX 0 / RX 1, with bit 6); 6 independent TX/RX BRG enable; 5:4 sampling 00 16X, 01 8X, 1x 4X; 3:0 fraction n/16 |
| DREV | — | device revision (value not printed) |
| XON1/2, XOFF1/2 | 0x00 | |
| GPIOINT | 0x00 | per-input interrupt enable |
| GPIO3T | 0x00 | per-output 3-state (1 = Hi-Z) |
| GPIOINV | 0x00 | 0: interrupt on LOW→HIGH, GPIOLVL reads true; 1: on HIGH→LOW, reads inverted |
| GPIOSEL | **0xFF** | 0 = output, 1 = input |
(* = needs EFR[4]=1.)

Trigger tables (Table 13 p. 34; FCTR[5:4] selects, last write applies to both TX and RX):
- A (00, 16C550): RX 1/4/8/14 (FCR[7:6]=00/01/10/11); TX 1 only.
- B (01, 16C650A): RX 8/16/24/28; TX (FCR[5:4]) 16/8/24/30.
- C (10, 16x654): RX 8/16/56/60; TX 8/16/32/56.
- D (11): programmable via TRG (1–128 implied) for RX and TX separately.

ISR codes (Table 11 p. 31): 1 `000110` LSR; 2 `001100` RX time-out; 3 `000100` RX ready; 4 `000010` TX ready; 5 `000000` MSR; 6 `010000` Xon/Xoff/special char; 7 `100000` CTS#/RTS# LOW→HIGH; `000001` none **or wake-up interrupt**. GPIO interrupt is flagged via ISR[6] (Table 12), not a code. Clearing (p. 31): LSR int by **reading LSR**; RXRDY by reading below trigger; time-out by reading RHR; TXRDY by reading ISR or writing THR; MSR by reading MSR; Xon/Xoff by reading ISR (EMSR[7] modifies); special char by ISR read or next char; RTS#/CTS# by reading MSR; wake-up by ISR read; GPIO by reading GPIOLVL.

### Behavior
- Baud (pp. 14–15): as XR20M1170 fractional formula (DLD[3:0]/16; 8X with odd DLD → ±1/16 jitter) but **independent TX/RX generators** (DLD[6:7]). Crystal ≤24 MHz; external clock ≤ 40 / 64 / 96 MHz at 1.8 / 2.5 / 3.3 V (p. 50). Max data rate 24 Mbps.
- FIFOs 128 bytes TX/RX (+3 error bits). Time-out = 4 char + 12 bit times.
- Auto RTS hysteresis (p. 19, Table 7 for Table-C): RTS# HIGH at one trigger level above selected (8→16, 16→56, 56→60, 60→60), LOW at one level below (8→0, 16→8, 56→16, 60→56); Table-D uses SHR hysteresis.
- Auto Xoff sent two character times after RX FIFO crosses trigger; Xon when FIFO < one level below (Table 8 p. 21). Flow chars not placed in FIFO.
- Auto RS-485 (FCTR[3]; EN485# LOW at power-up enables it): RTS# asserted when data loaded, de-asserted one bit time after last stop bit (+SHR delay); TX interrupt becomes TSR-empty; EMSR[3] inverts.
- 9-bit mode: SFR[6]; normal multidrop / auto address (EFR[5], XOFF2) as XR20M1170; TX address via SFR[7].
- IrDA: MCR[6] or ENIR# LOW at power-up (QFN-32 only); SFR[3] fast mode; FCTR[2] RX inversion.
- Sleep (p. 24): IER[4] — conditions: no interrupt, modem inputs quiet, RX idle (HIGH normal / LOW IR), divisor ≠ 0, TX and RX FIFOs empty; oscillator stopped; wake as XR20M1170. SLEEP pin (FCTR[0]=0): enters sleep after current TSR byte and RSR byte complete even with data in FIFOs; wakes only when SLEEP de-asserted (pin polarity: "asserted" — active level not stated; name SLEEP suggests HIGH = sleep). PWRDN# output (FCTR[0]=1) LOW while asleep. FCR[3] wake-up interrupt.
- Loopback (p. 25): TX held HIGH, RTS#/DTR# de-asserted; inputs ignored; MSR[7:4] ← MCR[3], MCR[2], MCR[0], MCR[1].
- EMSR[4] 3-states TX/RTS#/DTR#.
- Reset (Table 21 p. 48): see table; outputs TX HIGH, RTS# HIGH, DTR# HIGH (table cut at page end; IRQ presumably released).

### Timing
- I2C (p. 51): identical numbers to XR20M1170 (fSCL ≤400 kHz, tSP printed 0.5 µs, TD1–TD7 0.2 µs, TD8 0.5/1.0 µs, TD15 3 µs).
- SPI (p. 55, single column, 1.62–3.63 V): fSCL ≤ 26 MHz; TCP ≥ 40 ns; TCH,TCL ≥ 20; TCSS ≥ 25; TCSH ≥ 20; TDS ≥ 10; TDH ≥ 10; TDO ≤ 100 ns (15 pF) (as printed — larger than half a period; datasheet inconsistency); TTR ≤ 100; TCSW ≥ 25 ns; TD9–TD14 200 ns.

### VCC
VCC_CORE, VCC_BUS, VCC_UART, VCC_GPIO each 1.62–3.63 V (pp. 5, 49). Inputs 5 V tolerant (VIH max 5.5 V, SDA/SCL VSIH max 5.5 V) except XTAL1 (p. 12, 49). Abs max table still says "Voltage at any pin GND−0.3 to 4 V" (p. 49) — contradicts 5 V tolerance; flag. inputMax 5.5 V (non-XTAL inputs).

### Package drawings
- QFN-24 4×4×0.9 **POD-00000130 rev B** (pp. 59–60): same drawing as XR20M1170 qfn24 (D2/E2 2.50/2.65/2.80, b 0.18/0.25/0.30, L 0.35/0.40/0.45, e 0.50).
- QFN-32 5×5×0.9 **POD-00000037 rev B** (outline p. 61, land pattern/stencil p. 62; JEDEC MO-220 VHHD-4): D = E 5.00 BSC; e 0.50; b 0.18/0.25/0.30; L 0.35/0.40/0.45; D2 = E2 3.50/3.65/3.80; A 0.80/0.90/1.00; A1 0/0.02/0.05; A3 0.20 ref; K ≥ 0.20. Land pattern: pads 0.275 × 0.625 (32×), thermal land 3.80 × 3.80.

### Obsolete
None.

### Notes
- MCR[2] polarity: 0 = modem pins (default), opposite of SC16IS-style IOControl.
- Abs-max 4 V vs "5 V tolerant inputs" conflict; TDO 100 ns at 26 MHz conflict.
- DREV value not given.

---

## SC18IS604

- Title: "SC18IS604 — SPI to I²C-bus bridge", NXP Product data sheet Rev. 1.0, 22 Sep 2021. File `datasheets/controller-i2c/sc18is604.pdf`, pp. 1–19 (technical), 20–29 soldering/legal.
- Function: SPI target (slave, mode 3) that acts as a single I²C-bus **controller** (master-transmitter / master-receiver) with 255-byte TX and RX buffers, 5 GPIOs, internal RC oscillator, open-drain INT. Functional replacement for SC18IS600 except: new pinout, 255-byte buffers (vs 96), 5 GPIOs (no quasi-bidirectional mode), 1.71 V min supply, −40…+105 °C (p. 1).

### Parts (Tables 1–2 p. 2)
| Census part | Status | Decode | Package | pinout | grade |
|---|---|---|---|---|---|
| SC18IS604PWJ | Active | type SC18IS604PW (TSSOP16) + orderable suffix **J** = 13" reel Q1, 2500 pcs, standard mark | TSSOP16 **SOT403-1**, 4.4 mm body | tssop16 | VDD 1.71–3.6 V, Tamb −40…+105 °C |

### Pinout (Fig. 2 / Table 3 p. 3) — tssop16
| pin | name | role | notes |
|---|---|---|---|
| 1 | SDA | inout (open-drain) | I²C data |
| 2 | SCL | output (open-drain I²C clock; table says "O") | the device must sense SCL for clock stretching/arbitration (implied, p. 8) — model as open-drain inout |
| 3 | INT | output, open-drain, active LOW | must be pulled HIGH; must not be held LOW at power-on/reset |
| 4 | RESET | input, active LOW | glitch filter: rejected <20 ns, accepted ≥125 ns |
| 5 | GPIO4 | input | input only |
| 6 | GPIO3 | inout | |
| 7 | GPIO2 | inout | |
| 8 | CS | input, active LOW | also wakes from Deep Power-down |
| 9 | GPIO0 | inout | |
| 10 | MOSI | input | |
| 11 | SCLK | input | |
| 12 | VDD | power | |
| 13 | VSS | ground | |
| 14 | MISO | output | |
| 15 | VREFP | power/input | "connect to VDD" |
| 16 | GPIO1 | inout | |

### Host (SPI target) interface
- **SPI mode 3** (CPOL = 1, CPHA = 1) (pp. 1, 4, Fig. 17 p. 16): SCLK idles HIGH; MOSI sampled on the rising (trailing) edge, MISO changes on the falling (leading) edge (tv(Q) ≤ 28 ns @3.0–3.6 V, ≤ 32 ns @1.71–3.0 V); data setup tDS ≥ 10 ns, hold tDH ≥ 7 ns (Table 13 p. 15). CS active LOW frames each message.
- Bit order MSB first by default; command 0x18 can switch to LSB first (below).
- Max SCLK 1.2 MHz ("tested to 1.2 MHz, guaranteed down to 0 Hz", Table 13 note [1]).
- **Inter-byte gap**: tSPICLKW ≥ 8 µs SCLK HIGH between two SPI bytes; tCSW ≥ 0.4 µs CS HIGH between two SPI transactions; tSPILAG1 ≥ 0.4 µs (enable lag in SPI→I²C transaction); td ≥ 35 µs from last SCLK pulse to SDA LOW (START) in a SPI→I²C transaction (Table 14 p. 16). (tSPILEAD appears in Fig. 18 without a value.)
- MISO state when CS HIGH: not stated (not described as 3-state). MISO is a "3 V tolerant" pin (not 5 V tolerant) (p. 14 note [3]).
- One SPI message = one CS-low frame; commands (Section 6.5 pp. 9–13):

| Cmd | Bytes after command (MOSI) | Action |
|---|---|---|
| 0x00 | N (1–255, count of data bytes excluding address), target addr (A[7:1] + bit0 ignored, forced 0), data1 … dataN | **Write N bytes**: I²C START, addr+W, N data bytes, STOP (implied). On success INT asserted, I2CStat = 0xF0. |
| 0x01 | N (1–255), target addr (bit0 ignored, forced 1) | **Read N bytes**: START, addr+R, read N bytes into receive buffer, STOP. INT asserted when done; data fetched later with 0x06. |
| 0x02 | Nw, Nr, target addr+W, write data 0 … Nw−1 (figure labels "DATA WRITE BYTE 0 … N"), target addr+R | **Read after write**: write Nw bytes to target then read Nr bytes from target into receive buffer; INT when done. Whether a repeated START or STOP+START separates the phases is **not stated** (combined-format Sr is the usual SC18IS60x behaviour). Note that the host sends the target address twice (+W and +R). |
| 0x03 | N1, N2, target1 addr+W, N1 data bytes, target2 addr+W, N2 data bytes | **Write after write**: write N1 bytes to target 1, then N2 bytes to target 2. Separation (Sr vs P/S) not stated. |
| 0x06 | (host clocks dummy bytes) | **Read buffer**: device returns receive-buffer bytes on MISO, data1 … dataN. Buffer is overwritten by a later 0x01/0x02 before it is read. Bytes beyond those received (or the version string terminator) return stale buffer contents (p. 13). |
| 0x18 | config byte | **SPI configuration**: 0x81 = LSB first, 0x42 = MSB first (default) (Table 10 p. 11). Other values: not defined. |
| 0x20 | register addr, data | **Write internal register** (one register per transaction, no auto-increment). Processed immediately. |
| 0x21 | register addr, dummy | **Read internal register**: register value returned on MISO during the dummy byte. One register, no auto-increment. |
| 0x30 | 0x5A, 0xA5 | **Deep Power-down**: oscillator stops; device ignores SPI. Only the exact 3-byte sequence enters power-down. Exit: drive CS LOW (after first raising CS HIGH after the sequence). |
| 0xFE | — | **Read version**: places the 16-byte string "SC18IS604 1.0.0\0" (53 43 31 38 49 53 36 30 34 20 31 2E 30 2E 30 00) into the read buffer and asserts INT; read it with 0x06 (p. 13). |
Other command codes: not defined. Buffers: TX 255 bytes, RX 255 bytes (p. 1); N max 255. Status 0xF9 "invalid data count" for a bad byte count (text truncated; presumably N = 0 or N exceeding the bytes actually sent).

### Registers (Table 4 p. 4; descriptions pp. 4–8)
| Addr | Name | R/W | Reset | Bits |
|---|---|---|---|---|
| 0x00 | IOConfig | R/W | 0x00 | [7:6] IO3.1:IO3.0, [5:4] IO2, [3:2] IO1, [1:0] IO0 — per GPIO0–3: 00 or 01 = input-only, 10 = push-pull output, 11 = open-drain output (Table 5 p. 4) |
| 0x01 | IOState | R/W | 0xXX | [7:5] 0/reserved; [4] GPIO4 (input only, writes ignored); [3:0] GPIO3–0. Write sets output latches; read returns actual pin states |
| 0x02 | I2CClock (I2CClk) | R/W | **0x19** (25 → 75 kHz) | CR[7:0]: f_SCL = 15 000 000 / (8 × I2CClk) Hz (Eq. 1 p. 6); min 5 (375 kHz), e.g. 7 → 268 kHz, 9 → 208 kHz, 19 → 99 kHz, 255 → 7.4 kHz (Table 7). Values < 5: not allowed (behaviour not stated) |
| 0x03 | I2CTO | R/W | 0x00 | [7:1] TO; [0] TE (1 = timeout enabled). timeout = ((I2CClock + 1) / 15 000 000) × ((1 + TO[7:1]) × 512) seconds, approximate (p. 7) |
| 0x04 | I2CStat | R | 0x00 in Table 4 / **0xF0** per Table 9 text ("default status after reset") | [7:4] = 1111, [3:0] status code (below). Reading I2CStat de-asserts INT |
| 0x05 | I2CAdr | R/W | 0x00 | [7:1] device's own I²C address (not needed for operation; avoid conflicts); [0] unused, write 0 |

I2CStat codes (Table 9 pp. 7–8):
| Code | Meaning | INT? |
|---|---|---|
| 0xF0 | transaction completed successfully (also reset default) | yes on completion; not after reset |
| 0xF1 | target address not acknowledged (address NACK) | yes |
| 0xF2 | data byte not acknowledged by target (table title says "device address not acknowledged", text says "has not acknowledged the byte that has just been transmitted") | yes |
| 0xF3 | I²C busy — a transaction is in progress; host must not start another | no |
| 0xF8 | I²C timeout — transaction exceeded I2CTO (e.g. lost arbitration repeatedly or SCL held LOW by a target) | yes |
| 0xF9 | invalid data count (byte count in a read/write command invalid) | yes |

### I²C controller behaviour (as documented)
- Generates START, address+R/W, data, ACK/NACK and STOP; handles "protocol, arbitration and timing" (p. 1). Single controller but multi-controller-aware: **arbitration loss** → waits for bus free and re-sends START, repeating until the timeout (if enabled) expires → status 0xF8 + INT (p. 7).
- **Clock stretching**: supported implicitly (clock synchronization; timeout covers "a target … continuously pulling SCL LOW", p. 8).
- **Address NACK** → 0xF1, transaction aborted (STOP presumed), INT. **Data NACK during write** → 0xF2, INT (abort presumed). During reads the controller ACKs each byte and NACKs the last (standard; not explicitly stated).
- Bus speed set by I2CClock; Fast-mode up to 375 kHz achievable (datasheet title says 400 kHz-capable interface; Table 15 limits are the generic I²C spec values).
- Timeout only if I2CTO.TE = 1.
- No glitch suppression on SCL/SDA (Table 13 note [2]).
- Busy indication: I2CStat = 0xF3 while running.

### INT pin
Open-drain active LOW. Asserted at the end of every I²C transaction (codes 0xF0, F1, F2, F8, F9) and after command 0xFE. De-asserted when I2CStat is read (register read 0x21 0x04) or by RESET (p. 3). Not asserted after reset.

### GPIO
GPIO0–3 configurable (IOConfig): input-only, push-pull, or "pseudo open-drain" (cannot be pulled above VDD — not 5 V tolerant in that mode? text: "The pin cannot be pulled up above VDD. The pins are not 5 V tolerant when VDD is grounded", p. 5). GPIO4 input only. All inputs Schmitt-triggered with glitch suppression (signal acceptance ≥ 50 ns). Weak pull-up current Ipu 10–90 µA at VI = 0 (Table 12 p. 15) — implies inputs have weak pull-ups (which pins not stated). IOState reset 0xXX (pins as inputs).

### Reset / power
- RESET LOW: registers to defaults (Table 4), I²C and SPI hardware reset; INT released. Defaults: IOConfig 0x00 (all GPIO inputs), I2CClock 0x19, I2CTO 0x00 (timeout disabled), I2CAdr 0x00, I2CStat 0xF0; SPI bit order MSB first (default). Buffer contents after reset: not stated.
- Deep Power-down via 0x30 0x5A 0xA5; wake on CS LOW. IDD(tpd) typ 6 µA.
- Clock: internal RC oscillator only (I²C timing formulas use 15 MHz).

### Timing (pp. 15–17)
- SPI: see above (tDS 10, tDH 7, tv(Q) ≤ 28/32 ns; tSPICLKW ≥ 8 µs between bytes; tCSW ≥ 0.4 µs; td ≥ 35 µs SCLK→SDA START; SCLK ≤ 1.2 MHz).
- I²C (Table 15 p. 17, spec limits): fSCL 0–400 kHz; tBUF ≥ 1.3 µs; tHD;STA ≥ 0.6; tSU;STA ≥ 0.6; tSU;STO ≥ 0.6 (Std: 4.0); tHD;DAT ≥ 0; tSU;DAT ≥ 100 ns (Fast) / 250 (Std); tLOW ≥ 1.3; tHIGH ≥ 0.6; tVD ≤ 0.6 µs; tSP 50 ns. Actual SCL generated from I2CClock: period = 8 × I2CClk / 15 MHz (duty not stated).
- RESET: tgr 20 ns max (rejected), tsa ≥ 125 ns (accepted).

### VCC
1.71–3.6 V, −40…+105 °C (Table 12 p. 14). 5 V tolerant I/O: VI max 5.4 V with VDD ≥ 1.71 V for all pins except MISO (3.6 V max) (Table 11 p. 14). inputMax 5.4 V (MISO is an output; open-drain GPIO pins limited to VDD).

### Package drawing
TSSOP16 **SOT403-1** (outline p. 18): e 0.65; D 4.9–5.1; E 4.3–4.5; HE 6.2–6.6; bp 0.19–0.30; Lp 0.50–0.75; L 1.0; A max 1.1; A1 0.05–0.15; A2 0.80–0.95; c 0.1–0.2. Reflow footprint (p. 19, Fig. 21): P1 0.650 pitch, P2 0.750, Ay 7.200 (pad row outer span), By 4.500 (inner span), C 1.350 (pad length), D1 0.400 (pad width), D2 0.600, Gx 5.600, Gy 5.300, Hx 5.800, Hy 7.450. Same outline as SC16IS740IPW's SOT403-1.

### Obsolete
None.

### Notes
- I2CTO table header says "address 0x04" (p. 6) but register map gives 0x03 (p. 4); I2CStat is 0x04.
- I2CStat default 0x00 (Table 4) vs 0xF0 (Table 9 text).
- 0xF2 description title duplicated from 0xF1; 0xF9 sentence truncated in the PDF.
- Whether read-after-write / write-after-write use repeated START is not stated.
- SCL typed "O" though stretching/arbitration need it sensed.

---

## Summary

- Devices: SC16IS750 (+740/760 variants), SC16IS752 (+762), PI7C9X760, PI7C9X762, XR20M1170, XR20M1280, SC18IS604.
- All 25 census rows decoded except the caveats: **PI7C9X762Q2ZHEX** — package ZH (32-TQFN 5×5) decodable, but the "Q2" code is not in pi7c9x762.pdf (only PI7C9X762DZHEX); likely the automotive-grade variant of the same die. **PI7C9X760CZDEX** — PDF lists only PI7C9X760DZDEX; "C" is the earlier product-revision letter, same ZD package. NXP ",112/,128/,151/,157" packing suffixes are not in the PDFs (package fixed by base type).
- Not decodable from the PDFs: SC18IS604 START-vs-repeated-START in combined commands; PI7C9X76x SFR[4] "burst R/W" and TIDLE semantics; XR20M1280 DREV value and SLEEP pin active level; NXP I²C clock-stretching by the UART bridges (not stated).
