# Maxim (Analog Devices) SPI/I2C UARTs — MAX3100, MAX3140, MAX3107, MAX3108, MAX3109, MAX14830

Fact sheets for plan-12 (UART models). All PDFs are in `datasheets/controller-uart/`. In all six PDFs the PDF page number
equals the printed page number. Pins are named without overbar marks; active level is stated in the text.
Pin names with a slash (`CS/A0`, `DOUT/SDA`, `RTS/CLKOUT`) are one pin whose function is picked by a mode pin; the
repo name I suggest is given in the pin tables (`CS_A0` style is left to the planner).

## PDF → devices

| PDF | devices covered | census parts |
|---|---|---|
| max3100.pdf (24 p, Rev 2 1/09) | MAX3100 | MAX3100CPD+, MAX3100EPD+, MAX3100CEE+, MAX3100CEE+T, MAX3100EEE+, MAX3100EEE+T, MAX3100ETG+ |
| max3140.pdf (36 p, Rev 1 9/10) | MAX3140 (= MAX3100 UART core + RS-485/422 transceiver) | MAX3140CEI+, MAX3140EEI+ |
| max3107.pdf (52 p, Rev 7 5/15) | MAX3107 | MAX3107EAG+, MAX3107EAG+T, MAX3107ETG+, MAX3107ETG+T |
| max3108.pdf (56 p, Rev 2 1/13) | MAX3108 | MAX3108EWA+T |
| max3109.pdf (66 p, Rev 6 8/20) | MAX3109 (dual) | MAX3109ETJ+, MAX3109ETJ+T |
| max14830.pdf (68 p, Rev 7 2/16) | MAX14830 (quad) | MAX14830ETM+, MAX14830ETM+T |

Two families:
- **MAX3100 / MAX3140**: tiny 16-bit-word SPI/MICROWIRE UART, 8-word RX FIFO, no TX FIFO, 4-bit baud divisor
  table, no parity generator (9th bit is user data). MAX3140 is the same UART (stated "hardware- and
  software-compatible with the MAX3100", p1) on one die with a MAX3089-like RS-485 transceiver; the two halves
  share only supply/ground (p1).
- **MAX3107 / MAX3108 / MAX3109 / MAX14830**: "advanced UART" with 128-word TX and RX FIFOs, flat 8-bit register
  file (0x00–0x1F, plus 0x20–0x25 on 3109/14830), SPI (mode 0, 26 MHz) or I2C target, PLL + predivider +
  16.4 fractional baud generator, auto RTS/CTS, XON/XOFF, 9-bit multidrop, IrDA SIR/MIR, echo suppression, RS-485
  direction control, GPIOs. MAX3107 = reference; MAX3108 = MAX3107 in WLP with small register changes; MAX3109 = two
  MAX3108-like UARTs + global/sync/timer registers; MAX14830 = four UARTs (MAX3107-like register details, no sleep).
  Register-by-register deltas are listed in each section.

### Census parts that could not be decoded
None. Notes:
- Ordering tables list only some suffixes; the census adds the tape-and-reel (`+T`) or bulk (`+`) twin of a listed
  part (MAX3100CEE+T/EEE+T, MAX3107EAG+/ETG+ (PDF lists only `+T` and the dropped automotive ETG/V+T),
  MAX3109ETJ+T, MAX14830ETM+T). `+` = lead-free, `T` = tape and reel only; same die/package/grade.
- MAX14830: census VCC column says 1.71–3.6 V, but the datasheet's analog supply VA is **2.35–3.6 V** (VL, VEXT are
  1.71–3.6 V) (p8). Use 2.35–3.6 V for VA.
- MAX3107: census 2.35–3.6 V matches VA; VL/VEXT 1.71–3.6 V.

### Obsolete census rows
None (all rows are Active).

### Package/pinout names used
| name | parts | drawing (Maxim outline / land pattern) |
|---|---|---|
| dip14 | MAX3100CPD+, EPD+ | 14 PDIP, code P14-3, outline 21-0043 (max3100 p23) |
| qsop16 | MAX3100CEE+(T), EEE+(T) | 16 QSOP, code E16-1, outline 21-0055 (max3100 p23) |
| tqfn24-4x4 (MAX3100 pinout) | MAX3100ETG+ | 24 TQFN-EP 4x4, code T2444-4, outline 21-0139 (max3100 p23) |
| qsop28 | MAX3140CEI+, EEI+ | 28 QSOP, code E28M+1, outline 21-0055, land 90-0173 (max3140 p35) |
| ssop24 | MAX3107EAG+(T) | 24 SSOP 5.3 mm, code A24+1, outline 21-0056, land 90-0110 (max3107 p51) |
| tqfn24-3.5 (MAX3107 pinout) | MAX3107ETG+(T) | 24 TQFN-EP 3.5x3.5, code T243A3+1, outline 21-0188, land 90-0122 (max3107 p51) |
| wlp25 | MAX3108EWA+T | 25-bump WLP 2.1x2.1 mm, 0.4 mm pitch (p1), code W252B2+1, outline 21-0180, land: "Refer to Application Note 1891" (max3108 p55) |
| tqfn32 | MAX3109ETJ+(T) | 32 TQFN-EP 5x5, code T3255+4, outline 21-0140, land 90-0012 (max3109 p7) |
| tqfn48 | MAX14830ETM+(T) | 48 TQFN-EP 7x7, code T4877+3, outline 21-0144, land 90-0129 (max14830 p67) |

**None of the six PDFs contains a package outline or land-pattern drawing** — only the code/outline numbers above
("go to www.maximintegrated.com/packages"). Dimensions I can give from the PDFs: WLP 2.1x2.1 mm, 0.4 mm pitch
(max3108 p1); TQFN body sizes (in the names); SSOP "0.209 in / 5.30 mm" body width and QSOP "0.154 in" (census
case column). Pitches implied by pin count/body (not printed): QSOP 0.635 mm, SSOP-24 (5.3 mm) 0.65 mm, PDIP 2.54 mm
/ 7.62 mm rows, TQFN-32 5x5 and TQFN-48 7x7 0.5 mm. Exposed-pad sizes, lead lengths and the 24-TQFN pitches are
**not** in the PDFs — the footprint agent needs Maxim outlines 21-0043/0055/0056/0139/0140/0144/0180/0188.

---

## MAX3100

- Title: "MAX3100 SPI/MICROWIRE-Compatible UART in QSOP-16" (max3100.pdf p1). 24 pages.
- Function: UART with SPI/MICROWIRE target port, crystal oscillator, 4-bit baud divider, 8-word receive FIFO,
  IrDA SIR timing mode, 9-bit address-interrupt, software/hardware shutdown with receive-activity interrupt.

### Parts
| census part | package | pins | pinout | grade | supply |
|---|---|---|---|---|---|
| MAX3100CPD+ | 14 Plastic DIP (P14-3) | 14 | dip14 | 0 to +70 °C | 2.7–5.5 V |
| MAX3100EPD+ | 14 Plastic DIP | 14 | dip14 | −40 to +85 °C | 2.7–5.5 V |
| MAX3100CEE+, CEE+T | 16 QSOP (E16-1) | 16 | qsop16 | 0 to +70 °C | 2.7–5.5 V |
| MAX3100EEE+, EEE+T | 16 QSOP | 16 | qsop16 | −40 to +85 °C | 2.7–5.5 V |
| MAX3100ETG+ | 24 TQFN-EP 4x4 (T2444-4) | 24 + EP | tqfn24-4x4 | −40 to +85 °C | 2.7–5.5 V |
Decode (p1, p2): C = 0..70 °C, E = −40..85 °C; PD = 14 PDIP, EE = 16 QSOP, TG = 24 TQFN. One speed grade.

### Pinouts (p1 drawing, p5 table)
| signal | role | dip14 | qsop16 | tqfn24 |
|---|---|---|---|---|
| DIN | input (Schmitt) | 1 | 1 | 23 |
| DOUT | output, 3-state (Z when CS high) | 2 | 2 | 2 |
| SCLK | input (Schmitt) | 3 | 3 | 3 |
| CS | input, active low (Schmitt) | 4 | 4 | 4 |
| IRQ | output, open drain, active low | 5 | 6 | 5 |
| SHDN | input, active low (hardware shutdown) | 6 | 7 | 8 |
| GND | ground | 7 | 8 | 9 |
| X2 | output (crystal) | 8 | 9 | 10 |
| X1 | input (crystal / ext clock) | 9 | 10 | 11 |
| CTS | input, active low ("general-purpose") | 10 | 11 | 15 |
| RTS | output, active low ("general-purpose") | 11 | 13 | 16 |
| RX | input | 12 | 14 | 17 |
| TX | output | 13 | 15 | 21 |
| VCC | power | 14 | 16 | 22 |
| N.C. | nc | — | 5, 12 | 1, 6, 7, 12, 13, 14, 18, 19, 20, 24 |
| EP | TQFN exposed pad: "Connect EP to ground or leave unconnected" (p5); p1 drawing says connect to ground | | | EP |

### Host interface (SPI/MICROWIRE target; p3, p7–8, p18)
- 16-bit full-duplex word per CS-low frame, MSB (bit 15) first on both DIN and DOUT. DIN latched on SCLK **rising**
  edge; DOUT changes on SCLK **falling** edge (p7). Bit 15 (R) appears on DOUT after CS falls (tDV); MAX3140 p16
  states it outright: "The first bit (bit 15) of DOUT transitions on CS's falling edge, and bits 14–0 transition on
  SCLK's falling edge". SPI mode 0 (CPOL=0, CPHA=0) only (max3140 p15 Fig. 11; the other three modes are marked
  "not compatible").
- DIN bits 15,14 select the operation (p8 Tables 1–4, p18 Table 9):

| D15 D14 | operation | DIN bits 13..0 | DOUT bits 15..0 |
|---|---|---|---|
| 1 1 | Write Config | FEN SHDNi TM RM PM RAM IR ST PE L B3 B2 B1 B0 | R T 0×14 |
| 0 1 | Read Config | 0×13, TEST (bit 0) | R T FEN SHDNo TM RM PM RAM IR ST PE L B3 B2 B1 B0 |
| 1 0 | Write Data | 0 0 0 TE RTS Pt D7t..D0t | R T 0 0 0 RA/FE CTS Pr D7r..D0r |
| 0 0 | Read Data | all 0 | R T 0 0 0 RA/FE CTS Pr D7r..D0r |
- "Most operations, such as the clearing of internal registers, are executed only on CS's rising edge" (p7). Only
  16-bit words are expected; if CS goes high before the 16th bit "the sequence is aborted (i.e., data does not get
  written to individual registers)". Every CS falling edge starts a new 16-bit stream (p7).
- Write Data: when TE = 0, RTS, Pt and D7t–D0t are loaded on CS rising edge (data goes to the TX buffer); when TE = 1
  "only the RTS pin will be updated on CS's rising edge" (p10). Write Data also returns (and pops) receive data —
  the DOUT word is the same as Read Data (p8, p18; MAX3140 p21: "the DIN and DOUT WRITE DATA words are used
  simultaneously").
- Read Data "clears the R bit and interrupt IRQ" (p8).
- Write Config: clears the receive FIFO and the R, T, RA/FE, D0r–D7r, D0t–D7t, Pr, Pt registers; RTS and CTS
  unchanged. New config valid on CS rising edge if T = 1 and transmission is over, otherwise applied when the
  transmission finishes. FEN, SHDNi, IR, ST, PE, L, B3–B0 take effect after the current transmission; the mask bits
  TM, RM, PM, RAM take effect "immediately after the 16th clock's rising edge at SCLK" (p8). (The text says the
  write clears T; T is "transmit buffer empty" and shutdown "sets the T bit high" — treat T as 1 after a config
  write with no data pending.)
- Read Config returns the last configuration written. TEST (DIN bit 0 of Read Config) = 1 enters test mode: while
  CS = 0, RTS outputs the 16x baud clock, and "TX and RX are in digital loopback" (p8). (MAX3140 p21 instead says
  "The TX pin is low as long as CS remains low while in test mode" — conflict; see Notes.)
- DOUT is 3-state whenever CS is high; IRQ, TX, RTS are always active (p5).

### Registers / bits (Table 5 p9–10, Table 6 p11, Table 11 p19). POR states in brackets.
| bit | dir | meaning |
|---|---|---|
| B3–B0 [0000] | W/R | baud divisor select (Table 7) |
| FEN [0] | W/R | 0 = receive FIFO enabled, 1 = FIFO disabled (single receive register) |
| SHDNi [0] | W | 1 = enter software shutdown (after CS high; oscillator stops when transmitter idle) |
| SHDNo [0] | R | 1 = UART is in shutdown; with software shutdown not set until the current byte is sent (T = 1); set immediately by the SHDN pin |
| TM, RM, PM, RAM [0] | W/R | interrupt masks for T, R, Pr, RA/FE (1 = enabled) |
| IR [0] | W/R | 1 = IrDA timing mode |
| ST [0] | W/R | 0 = 1 stop bit, 1 = 2 stop bits transmitted; receiver needs only one |
| PE [0] | W/R | 1 = append Pt on transmit and expect a 9th bit on receive (no parity is calculated) |
| L [0] | W/R | 0 = 8 data bits, 1 = 7 data bits (D7t ignored, D7r reads 0) |
| TE [0] | W | 1 = update only RTS, do not transmit |
| RTS [0] | W | 1 drives RTS pin low; 0 drives it high (POR → pin high) |
| Pt [X] | W | transmitted 9th/parity bit when PE = 1, ignored when PE = 0 |
| D7t–D0t [X] | W | transmit data |
| R [0] | R | 1 = data available in receive register/FIFO |
| T [1] | R | 1 = transmit buffer empty (can accept a word) |
| RA/FE [0] | R | shutdown: RA, set by any RX transition since entering shutdown, cleared on exiting shutdown. Normal: FE, set if a 0 is received where the first stop bit is expected; cleared by the next properly framed character "independent of the FIFO being enabled"; not stored in the FIFO; not cleared by Read Data; cleared by Write Config. After FE the receiver resyncs to look for a start bit (p10, p11) |
| CTS | R | 1 = CTS pin low (bit is the inverse of the pin; p9) |
| Pr [X] | R | 9th bit of the oldest received word (0 when PE = 0); stored in the FIFO with the data |
| D7r–D0r [0] | R | oldest received data; all 0 when empty |

### Behavior
- Baud generator (p12 Table 7): baud = fOSC / (16 × BRD). The baud clock is 16× the bit rate (p5).

| B3..B0 | BRD | 1.8432 MHz | 3.6864 MHz | | B3..B0 | BRD | 1.8432 MHz | 3.6864 MHz |
|---|---|---|---|---|---|---|---|---|
| 0000 (POR) | 1 | 115.2k | 230.4k | | 1000 | 3 | 38.4k | 76.8k |
| 0001 | 2 | 57.6k | 115.2k | | 1001 | 6 | 19.2k | 38.4k |
| 0010 | 4 | 28.8k | 57.6k | | 1010 | 12 | 9600 | 19.2k |
| 0011 | 8 | 14.4k | 28.8k | | 1011 | 24 | 4800 | 9600 |
| 0100 | 16 | 7200 | 14.4k | | 1100 | 48 | 2400 | 4800 |
| 0101 | 32 | 3600 | 7200 | | 1101 | 96 | 1200 | 2400 |
| 0110 | 64 | 1800 | 3600 | | 1110 | 192 | 600 | 1200 |
| 0111 | 128 | 900 | 1800 | | 1111 | 384 | 300 | 600 |
- Clock: crystal 1.8432 or 3.6864 MHz between X1/X2, or a 45–55 % duty square wave on X1 with X2 open (p7, p15).
  The supply-current plot goes to 5 MHz external clock (p4); no max fOSC is printed. Oscillator start-up: "about
  25ms" (p12) / "typically within 50ms" (p10).
- Frames (p10 Fig. 5): start, D0..D6 (L=1) or D0..D7 (L=0), Pt if PE=1, 1 or 2 stop. LSB first.
- Receiver (p7, Fig. 3): start detected on RX high→low; start state = majority of the 7th, 8th, 9th 16x samples; data
  bits also majority-sampled. FIFO 8 words; "The FIFO is cleared if it overflows" (p7). (Table 11 p19 also calls
  RA/FE "Data overrun or framing error" — not stated elsewhere.)
- Transmitter: one TX buffer register + TX shift register (p6 Fig. 2). T = 1 when the buffer can take a word.
- Interrupts (p11 Table 6, Fig. 6): IRQ (active low, open drain) = (R·RM) + (T·TM) + (Pr·PM) + (RA/FE·RAM).
  R: set when data available, asserted "as long as R = 1 and RM = 1"; cleared when all data read. T source is an SR
  latch: set when the transmit buffer becomes empty, "cleared on CS's rising edge during a Read Data operation"
  (MAX3140 p23: on the rising edge of SCLK's 16th pulse during Read Data **or Write Data**). Pr: follows the oldest
  word's 9th bit (with FIFO enabled the interrupt is based on the oldest Pr; p15 says the parity interrupt "is
  controlled only by the data in the receive register, and is not affected by data in the FIFO"). RA/FE: level.
- 9-bit networks (p15): PE=1, L=0, PM=1 → IRQ on received 9th bit = 1.
- IrDA (p15, Fig. 9): TX bit period shortened to 3/16 of a bit; a data 0 is a light pulse = **TX low**; TX idles high.
  Receive: IrDA RX idles low, a data 0 is a **high** pulse on RX; sampled once, halfway into the high level; pulses
  shorter than ~1/16 bit ignored. Use 8N1 for IrDA compatibility.
- Shutdown (p12): software (SHDNi=1: entered after both TX buffer and TX shift register are sent; oscillator stops)
  or hardware (SHDN pin low: immediate, aborts transmission). Shutdown clears RX FIFO, R, RA/FE, D0r–D7r, Pr, Pt and
  sets T = 1. Config bits and RTS can still be written and CTS read while SHDNo = 1. RA goes high on any RX
  transition during shutdown (wake-up interrupt). Leaving: Write Config with SHDNi = 0 restarts the oscillator on CS
  rising edge if SHDN pin is high; first characters may be garbled / FE.
- Reset: power-on reset only (no reset pin). POR values as in the table (T=1, RTS pin high, FIFO empty).

### Timing (p3, VCC 2.7–5.5 V, CL 100 pF; all min unless "max")
tDV CS low→DOUT valid 100 max; tTR CS high→DOUT Z 100 max; tCSS 100; tCSH 0; tDO SCLK fall→DOUT valid 100 max;
tDS 100; tDH 0; tCP SCLK period 238 (≈4.2 MHz); tCH 100; tCL 100; tCS0 SCLK rise→CS fall 100; tCS1 CS rise→SCLK
rise 200; tCSW CS high 200; tr/tf outputs 10 ns max (ns throughout).

### VCC / input levels (p2)
2.7–5.5 V, all grades. Inputs (CS, SHDN, X1, CTS, RX, DIN, SCLK) abs max VCC+0.3 V → **not 5 V-tolerant above VCC**.
IRQ open drain may be pulled to 6 V. VIH 0.7 VCC, VIL 0.3 VCC, Schmitt hysteresis 0.05 VCC.

### Notes
- Test-mode TX behavior conflicts between max3100 p8 (digital loopback) and max3140 p21 (TX held low while CS low).
- R-bit clearing: MAX3140 p17 adds "R bit will clear on the falling edge of SCLK's 16th pulse if no new data is
  available" — the MAX3100 text says only "Reading data clears the R bit". Same die family; use the MAX3140 detail.

---

## MAX3140

- Title: "SPI/MICROWIRE-Compatible UART with Integrated True Fail-Safe RS-485/RS-422 Transceivers" (max3140.pdf p1).
- Nearest sibling: MAX3100. **UART identical** (same 4 16-bit operations, bits, baud table, FIFO, IrDA, 9-bit,
  shutdown; p15–26). Difference: 5 V only (4.75–5.25 V), 28-pin QSOP, plus an independent RS-485/422 transceiver
  whose logic side is unconnected to the UART internally (connect TX→DI, RX←RO, RTS→DE/RE externally; p1, p10 Fig.).

### Parts
| census part | package | pins | pinout | grade | supply |
|---|---|---|---|---|---|
| MAX3140CEI+ | 28 QSOP (E28M+1) | 28 | qsop28 | 0 to +70 °C | 4.75–5.25 V |
| MAX3140EEI+ | 28 QSOP | 28 | qsop28 | −40 to +85 °C | 4.75–5.25 V |

### Pinout qsop28 (p11–12, p35)
1 X2 output · 2 X1 input · 3 CTS input (active low) · 4 RTS output (active low) · 5 RX input · 6 TX output ·
7 H/F input (internal pull-down) · 8 GND ground · 9 RO output (3-state) · 10 RE input (active low) · 11 DE input ·
12 DI input · 13 SRL input (3-level, internally biased to VCC/2 by 100k/100k) · 14 N.C. · 15 TXP input (pull-down) ·
16 Y inout (driver non-inverting out; in half duplex also receiver + input) · 17 N.C. · 18 Z inout (driver inverting
out; half duplex also receiver − input) · 19 B input (receiver − input, full duplex) · 20 A input (receiver + input,
full duplex) · 21 RXP input (pull-down) · 22 VCC power · 23 DIN input · 24 DOUT output (3-state) · 25 SCLK input ·
26 CS input (active low) · 27 IRQ output (open drain, active low) · 28 SHDN input (active low).
Suggested repo names: `HF` for H/F. Y, Z, A, B are differential analog bus pins.

**What the transceiver adds:** RO, RE, DE, DI, H/F, SRL, TXP, RXP (digital/control) and Y, Z, A, B (bus).

### Transceiver digital model (p11–12 tables, p24–25)
- Driver: DE = 1 → Y = DI XOR TXP, Z = NOT Y (TXP = 1 inverts phase). DE = 0 → Y, Z high-Z.
- Receiver: RE = 0 → RO = ((A−B) ≥ −50 mV) XOR RXP in full duplex (H/F = 0), using (Y−Z) instead of (A−B) in half
  duplex (H/F = 1; "the receiver inputs are switched to the driver outputs, connecting outputs Y and Z to inputs A
  and B"). Threshold region −200…−50 mV is undefined (hysteresis 25 mV). Open/shorted/idle-terminated inputs →
  RO = 1 XOR RXP ("true fail-safe"). RE = 1 → RO high-Z.
- Shutdown: RE = 1 and DE = 0 held ≥ 600 ns → low-power shutdown (guaranteed not to enter if < 50 ns; tSHDN 50/200/600
  ns) (p7 Note 5). Enable from shutdown is slower (below).
- SRL: GND → 10 Mbps (no slew limit), VCC → 500 kbps, open → 115 kbps; may change during operation (p24). For a
  4-valued sim: 0 → fast, 1 → 500k, Z → 115k.
- Note on the digital model: in half duplex with DE = 1 the receiver sees its own driver (echo), as in hardware.

### Transceiver timing (p5–7, VCC 5 V ±5 %; typ / max ns)
| | SRL open (115k) | SRL = VCC (500k) | SRL = GND (10M) |
|---|---|---|---|
| tDPLH/tDPHL DI→Y/Z | 2030 / 2600 (min 500) | 720 / 1000 (min 250) | 34 / 60 |
| tDR/tDF | 1320 / 2500 | 530 / 950 | 14 / 25 |
| tDZH, tDZL enable | 3500 max | 2500 max | 150 max |
| tDLZ, tDHZ disable | 100 max | 100 max | 100 max |
| tRPLH/tRPHL A,B→RO | 127 / 200 | 127 / 200 | 106 / 150 |
| tRZL/tRZH/tRLZ/tRHZ | 20 / 50 | 20 / 50 | 20 / 50 |
| tDZH(SHDN), tDZL(SHDN) | 6000 max | 4500 max | 250 max |
| tRZH(SHDN), tRZL(SHDN) | 3500 max | 3500 max | 3500 max |
Receiver filtering in 115k/500k modes adds 20 % to receiver delay (p25).

### UART part
As MAX3100 (all sections above), with these printed additions: UART AC timing identical numbers (p4); DOUT bit 15
on CS fall (p16); R clears on falling edge of SCLK 16th pulse during Read/Write Data if no new data (p17, p21); T
interrupt cleared on the rising edge of SCLK's 16th pulse during Read Data or Write Data (p23); "Warning! … stable
crystal oscillator operation before configuration (typically ~25ms after power-up). At power-up, compare the WRITE
CONFIGURATION bits with the READ CONFIGURATION bits in a software loop until both match" (p20). Shutdown supply
lists UART-only ICC 0.47 mA and both-shut-down 20 µA (p2).

### VCC / inputs
4.75–5.25 V (p2, p11). All inputs abs max VCC + 0.3 V (p2) → not over-VCC tolerant. RS-485 logic inputs DE, DI, RE:
VIH 2.0 V; H/F, TXP, RXP: VIH 2.4 V; VIL 0.8 V (TTL levels, p3). UART inputs 0.7/0.3 VCC.

---

## MAX3107 (reference for the 128-word family)

- Title: "MAX3107 SPI/I2C UART with 128-Word FIFOs" (max3107.pdf p1).
- Function: one UART, 128-word TX and RX FIFOs, SPI (≤26 MHz) or I2C (≤400 kHz) target selected by pin, crystal
  oscillator / external clock, predivider + PLL, fractional baud generator, 2x/4x rate modes (≤24 Mbps), auto
  RTS/CTS, XON/XOFF, special characters, 9-bit multidrop with address filtering, RS-485 direction control with
  setup/hold, echo suppression, IrDA SIR/MIR, 4 GPIOs, sleep/autosleep/shutdown, separate VL (host) and VEXT (line)
  I/O supplies, internal 1.8 V LDO.

### Parts
| census part | package | pins | pinout | grade | supply |
|---|---|---|---|---|---|
| MAX3107EAG+, EAG+T | 24 SSOP (A24+1) | 24 | ssop24 | −40 to +85 °C | VA 2.35–3.6 V; VL, VEXT 1.71–3.6 V |
| MAX3107ETG+, ETG+T | 24 TQFN-EP 3.5x3.5 (T243A3+1) | 24 + EP | tqfn24-3.5 | −40 to +85 °C | same |
(p49 ordering; only one temperature/speed grade.)

### Pinouts (p12–13)
| signal | role | tqfn24 | ssop24 |
|---|---|---|---|
| V18 | power (1.8 V LDO out / core supply in) | 1 | 4 |
| I2C/SPI | input: high = SPI, low = I2C | 2 | 5 |
| LDOEN | input: high = internal LDO on | 3 | 6 |
| DOUT/SDA | SPI: DOUT output; I2C: SDA inout (open drain) | 4 | 7 |
| SCLK/SCL | input | 5 | 8 |
| CS/A0 | SPI: CS input active low; I2C: A0 address input (tie DGND or VL) | 6 | 9 |
| DIN/A1 | SPI: DIN input; I2C: A1 address input | 7 | 10 |
| IRQ | output, open drain, active low | 8 | 11 |
| RST | input, active low (hardware reset / shutdown) | 9 | 12 |
| VL | power (host I/O) | 10 | 13 |
| DGND | ground | 11 | 14 |
| GPIO0..GPIO3 | inout (weak pull-down) | 12..15 | 15..18 |
| CTS | input, active low | 16 | 19 |
| RTS/CLKOUT | output (3-stateable); RTS or baud clock | 17 | 20 |
| RX | input (weak pull-up to VEXT) | 18 | 21 |
| TX | output (3-stateable) | 19 | 22 |
| VEXT | power (line I/O) | 20 | 23 |
| XOUT | output (crystal) | 21 | 24 |
| XIN | input (crystal / clock) | 22 | 1 |
| AGND | ground | 23 | 2 |
| VA | power (analog/PLL/LDO) | 24 | 3 |
| EP | TQFN exposed pad → AGND ("not intended as an electrical connection point") | EP | — |

### Host interface (p44–47)
- Mode pin I2C/SPI: high = SPI, low = I2C.
- **SPI**: mode 0 only (CPOL = 0, CPHA = 0), MSB first. Frame = command byte `[W/R, A6..A0]` then data byte(s)
  (Fig. 15/16): read = R + 7-bit address, then the device shifts D7..D0 on SDO during the second byte; write = W +
  address + D7..D0 on SDI. Registers occupy 0x00–0x1F. The polarity of the W/R bit is **not printed** in any of the
  four datasheets (figures say only "W"/"R"); the Linux max310x driver uses bit 7 = 1 for write, 0 for read.
  Burst: keep CS low and keep clocking; "If the SPI burst address is different to 0x00, the MAX3107 automatically
  increments the register address after each SPI data byte"; at 0x00 it stays on THR/RHR (FIFO block transfer).
  Burst ends when CS goes high. 128-byte FIFO fill = (1 + 128) × 8 clocks. SDO state outside the data byte is not
  described.
- **I2C**: standard (100 kHz) and fast (400 kHz) (p8). 7-bit address 0b01011 A1 A0 (p45, Table 5): write/read bytes
  A1A0 = 00 → 0x58/0x59, 01 → 0x5A/0x5B, 10 → 0x5C/0x5D, 11 → 0x5E/0x5F (A1 = DIN/A1, A0 = CS/A0 tied to DGND or VL).
  Write: S, addr+W, ACK, register address, ACK ("only if the address is valid (NACK if not)"), data, ACK, … P.
  Read: S, addr+W, reg, Sr, addr+R, data…, master NACK on last, P. Burst write/read supported (auto-increment as
  in SPI; the "stays at 0x00" rule is printed only in the MAX3108/3109 I2C text, apply it here too).

### Register map (p14; reset 0x00 unless noted; full bit text p23–43)
| addr | name | mode | reset | bits 7..0 |
|---|---|---|---|---|
| 0x00 | RHR | R | X | RData7..0 (read pops RX FIFO) |
| 0x00 | THR | W | — | TData7..0 (write pushes TX FIFO) |
| 0x01 | IRQEn | R/W | 00 | CTSIEn RxEmtyIEn TxEmtyIEn TxTrgIEn RxTrgIEn STSIEn SpclChrIEn LSRErrIEn |
| 0x02 | ISR | COR | **60** | CTSInt RxEmptyInt TxEmptyInt TFifoTrigInt RFifoTrigInt STSInt SpCharInt LSRErrInt |
| 0x03 | LSRIntEn | R/W | 00 | — — NoiseIntEn RBreakIEn FrameErrIEn ParityIEn ROverrIEn RTimoutIEn |
| 0x04 | LSR | R | X0 | CTSbit — RxNoise RxBreak FrameErr RxParityErr RxOverrun RTimeout |
| 0x05 | SpclChrIntEn | R/W | 00 | — — MltDrpIntEn BREAKIntEn XOFF2IntEn XOFF1IntEn XON2IntEn XON1IntEn |
| 0x06 | SpclCharInt | COR | 00 | — — MultiDropInt BREAKInt XOFF2Int XOFF1Int XON2Int XON1Int |
| 0x07 | STSIntEn | R/W | 00 | — SleepIntEn ClkRdyIntEn — GPI3IntEn GPI2IntEn GPI1IntEn GPI0IntEn |
| 0x08 | STSInt | R/COR | 00 | — SleepInt ClockReady — GPI3Int GPI2Int GPI1Int GPI0Int |
| 0x09 | MODE1 | R/W | 00 | IRQSel AutoSleep ForcedSleep TrnscvCtrl RTSHiZ TxHiZ TxDisabl RxDisabl |
| 0x0A | MODE2 | R/W | 00 | EchoSuprs MultiDrop Loopback SpecialChr RxEmtyInv RxTrigInv FIFORst RST |
| 0x0B | LCR | R/W | **05** | RTS TxBreak ForceParity EvenParity ParityEn StopBits Length1 Length0 |
| 0x0C | RxTimeOut | R/W | 00 | TimOut7..0 |
| 0x0D | HDplxDelay | R/W | 00 | Setup3..0 Hold3..0 |
| 0x0E | IrDA | R/W | 00 | — — TxInv RxInv MIR — SIR IrDAEn (bit 2 "must be kept logic 0") |
| 0x0F | FlowLvl | R/W | 00 | Resume3..0 Halt3..0 |
| 0x10 | FIFOTrgLvl | R/W | **FF** | RxTrig3..0 TxTrig3..0 |
| 0x11 | TxFIFOLvl | R | 00 | TxFL7..0 |
| 0x12 | RxFIFOLvl | R | 00 | RxFL7..0 |
| 0x13 | FlowCtrl | R/W | 00 | SwFlow3..0 SwFlowEn GPIAddr AutoCTS AutoRTS |
| 0x14–0x17 | XON1, XON2, XOFF1, XOFF2 | R/W | 00 | 8-bit characters |
| 0x18 | GPIOConfg | R/W | 00 | GP3OD..GP0OD GP3Out..GP0Out |
| 0x19 | GPIOData | R/W | 00 | GPI3Dat..GPI0Dat (R) GPO3Dat..GPO0Dat |
| 0x1A | PLLConfig | R/W | **01** | PLLFactor1 PLLFactor0 PreDiv5..0 |
| 0x1B | BRGConfig | R/W | 00 | — — 4xMode 2xMode FRACT3..0 |
| 0x1C | DIVLSB | R/W | **01** | Div7..0 |
| 0x1D | DIVMSB | R/W | 00 | Div15..8 |
| 0x1E | CLKSource | R/W | **08** | CLKtoRTS — — ClockEn PLLBypass PLLEn CrystalEn — (bit 0 keep 0) |
| 0x1F | RevID | R | **A1** | 1010 0001 (check only upper nibble Ah, p47) |

Bit meanings (p23–43), shared by the whole family unless a later section says otherwise:
- **IRQEn**: each bit enables IRQ from the same-position ISR bit; does not affect ISR contents (p24).
- **ISR** (clear on read; reading ISR clears the bits and IRQ, p23): [7] CTSInt — any CTS transition. [6] RxEmptyInt —
  RX FIFO empty (inverted by MODE2[3]: then set "when data is put into the empty receive FIFO"). [5] TxEmptyInt —
  TX FIFO empty; it "turns high when the transmitter starts transmitting the last word", the line is idle one full
  character later (p15). [4] TFifoTrigInt — TX level ≥ trigger (FIFOTrgLvl[3:0]×8); cleared when level falls below
  trigger or on read. [3] RFifoTrigInt — RX level reaches trigger (FIFOTrgLvl[7:4]×8); with MODE2[2] set, generated
  when the RX FIFO is emptied to the trigger level instead. [2] STSInt — any enabled STSInt bit. [1] SpCharInt — any
  enabled SpclCharInt bit (special char, break, multidrop address). [0] LSRErrInt — any enabled LSR bit.
  Reset value 0x60 (RX and TX FIFOs empty).
- **LSRIntEn**: bits 5..0 enable LSR[5..0] into ISR[0].
- **LSR** — status of the word *most recently read from RHR*; not cleared by reading LSR; "stay set until the
  character with errors is read out of the RHR" (p27): [7] CTSbit = current CTS pin level (0 when CTS low).
  [5] RxNoise — the 3 samples of some bit disagreed (standard mode only). [4] RxBreak — break (RX low longer than a
  character) puts an all-zero character in the FIFO with this flag. [3] FrameErr — stop-bit error. [2] RxParityErr —
  parity mismatch; in multidrop mode = the received 9th bit. [1] RxOverrun — FIFO full and another word arrived (new
  data discarded, FIFO kept); cleared after LSR is read or the FIFO level falls below maximum. [0] RTimeout — the
  youngest character has sat in the FIFO longer than RxTimeOut character times; timer restarts on any RHR read or new
  character; cleared by reading a word or receiving one; disabled when RxTimeOut = 0.
- **SpclChrIntEn / SpclCharInt** (COR): [5] MultiDropInt — address (9th bit = 1) received in multidrop mode.
  [4] BREAKInt — break detected. [3..0] XOFF2/XOFF1/XON2/XON1 received (only when MODE2[4] SpecialChr = 1).
- **STSIntEn / STSInt** (R/COR): [6] SleepInt — set while in sleep, cleared on exit (not by read). [5] ClockReady —
  crystal + divider + PLL settled; only works with the crystal oscillator, not with an external clock; cleared when
  the clock is disabled, not by read. [3..0] GPIxInt — logic change on GPIO input x; cleared on read.
- **MODE1**: [7] IRQSel — 0 after any reset: IRQ pin low during power-up/reset, high when ready; set 1 for normal
  interrupt operation (p22, p31). [6] AutoSleep. [5] ForcedSleep (reads back current sleep state even while
  asleep). [4] TrnscvCtrl — auto direction control on RTS/CLKOUT. [3] RTSHiZ — 3-state RTS/CLKOUT. [2] TxHiZ —
  3-state TX. [1] TxDisabl — finish current character then stop; FIFO kept; TX high. [0] RxDisabl — receiver off
  after the current word; FIFO kept.
- **MODE2**: [7] EchoSuprs — receiver discards data while the transmitter is busy (extended by Hold). [6] MultiDrop
  — 9-bit mode: no parity generation/check, 9th bit replaces parity (LSR[2] = received 9th bit). [5] Loopback —
  internal TX→RX and RTS→CTS; TX pin 3-state; RTS pin still driven from LCR[7]; CTS pin disconnected. [4] SpecialChr
  — special-character detection (characters go into the FIFO and raise ISR[1]). [3] RxEmtyInv. [2] RxTrigInv.
  [1] FIFORst — clears both FIFOs; must be written back to 0. [0] RST — software reset of all registers and FIFOs,
  bus stays active; must be written back to 0.
- **LCR** (reset 0x05: Length = 01 → **6 bits**, StopBits = 1 → 2 stop, no parity; see Notes): [7] RTS — direct
  RTS/CLKOUT level (1 → pin high), only when CLKtoRTS = 0. [6] TxBreak — TX held low until cleared. [5] ForceParity
  (with ParityEn): parity bit forced **high if EvenParity = 0, low if EvenParity = 1**. [4] EvenParity (0 = odd).
  [3] ParityEn. [2] StopBits: 0 → 1 stop; 1 → 1.5 stop for 5-bit words, 2 stop for 6/7/8 (receiver accepts > 1 bit).
  [1:0] Length: 00 = 5, 01 = 6, 10 = 7, 11 = 8 bits (p33 Tables 1–2).
- **RxTimeOut**: timeout in character intervals (depends on length, parity, stop bits, baud); 0 = off (p34).
- **HDplxDelay**: Setup[7:4] — RTS/CLKOUT high this many bit times before the first start bit (auto direction
  control); Hold[3:0] — RTS/CLKOUT stays high this many bit times after the last stop bit, and the echo-suppression
  blanking extension. 0–15 bit intervals; also works in 2x/4x modes (p34).
- **IrDA**: [5] TxInv — invert TX (independent of IrDA). [4] RxInv — invert RX input. [3] MIR (with IrDAEn) — 1/4-bit
  pulses. [1] SIR (with IrDAEn) — 3/16-bit pulses. [0] IrDAEn. IrDA does not work in 2x/4x modes (p18).
- **FlowLvl**: Halt[3:0]×8 (0–120) = RX level where XOFF is sent / RTS deasserted (high); Resume[7:4]×8 = level where
  XON is sent / RTS asserted (low). (p35 text says "transmit FIFO threshold" for Resume — datasheet error; it is the
  receive FIFO, as MAX3108 p39 says.)
- **FIFOTrgLvl**: RxTrig[7:4]×8, TxTrig[3:0]×8 (reset 0xFF → 120/120).
- **TxFIFOLvl / RxFIFOLvl**: word counts (0–128); may be wrong if read while the UART updates them (p36).
- **FlowCtrl** (p37–38): [7:4] SwFlow — Table 3: SwFlow[3:2] (receive flow control = what the transmitter sends):
  00 none, 10 XON1/XOFF1, 01 XON2/XOFF2, 11 XON1+XON2 / XOFF1+XOFF2 (two-byte, 1 then 2). SwFlow[1:0] (transmit flow
  control / special-char detection = what the receiver compares): 00 none, 10 XON1/XOFF1, 01 XON2/XOFF2, 11 all
  four (two-byte: XON1 then XON2 must arrive in order for flow control). [3] SwFlowEn — enable auto XON/XOFF;
  if also SpecialChr, XON1/XOFF1 are flow characters and XON2/XOFF2 special characters. [2] GPIAddr — special
  character / multidrop address = XOFF2[7:4] : GPIO3..0 inputs (XOFF2[3:0] ignored). [1] AutoCTS. [0] AutoRTS.
  Received XON/XOFF used for flow control are not put in the FIFO and raise no interrupt (p22).
- **GPIOConfg**: [7:4] GPxOD 1 = open drain; [3:0] GPxOut 1 = output. Inputs are high-Z with weak pull-down.
  "The reference clock has to be active for the GPIOs to work" (p40).
- **GPIOData**: [7:4] input levels, [3:0] output levels.
- **PLLConfig**: PreDiv[5:0] 1..63; PLLFactor[7:6] — table below.
- **BRGConfig**: [5] 4xMode, [4] 2xMode (set only one), [3:0] FRACT.
- **DIVLSB/DIVMSB**: 16-bit integer divisor DIV, minimum 1.
- **CLKSource**: [7] CLKtoRTS — baud-generator clock (16x/8x/4x baud) on RTS/CLKOUT. [4] ClockEn — 1 enables
  clocking ("crystal or clock generator at XIN"), 0 disables; **reset 0** (MAX3107 only). [3] PLLBypass (reset 1) —
  bypass predivider and PLL. [2] PLLEn. [1] CrystalEn — 1 = crystal oscillator, 0 = external clock on XIN.
- **RevID** 0xA1.

### Clocking — exact (p16–18, p41–43, AC table p8)
```
XIN ──[CrystalEn: crystal osc (XIN/XOUT, 20 pF internal load caps each) | external clock]── fCLK
fCLK ── PLLBypass=1 ─────────────────────────────────────────────┐
fCLK ── ÷PreDiv (1..63) ── fPLLIN ── PLL ×{6,48,96,144} (PLLEn) ──┴── fREF (≤ 96 MHz) ── fractional BRG
```
- Crystal 1–4 MHz; external clock 0.5–35 MHz, 45–55 % duty; fREF ≤ 96 MHz (p8).
- Select crystal: ClockEn = 1, CrystalEn = 1. External clock: ClockEn = 1, CrystalEn = 0, XOUT open (p17).
- PLL ranges (Table 4, p41):

| PLLFactor[1:0] | × | fPLLIN min–max | fREF min–max |
|---|---|---|---|
| 00 | 6 | 500–800 kHz | 3–4.8 MHz |
| 01 | 48 | 850 kHz–1.2 MHz | 40.8–56 MHz |
| 10 | 96 | 425 kHz–1 MHz | 40.8–96 MHz |
| 11 | 144 | 390–667 kHz | 56–96 MHz |
- Fractional baud generator: D = fREF / (16 × baud) [use 8 or 4 instead of 16 in 2x / 4x mode];
  DIV = trunc(D) → DIVMSB:DIVLSB; FRACT = round(16 × (D − DIV)) → BRGConfig[3:0];
  actual baud = fREF / (16 × (DIV + FRACT/16)) (×2 or ×4 in 2x/4x). Example p18: 28.23 MHz, 190 kbaud → DIV 9,
  FRACT 5, 189 463 baud. With FRACT ≠ 0 the baud clock "is not regular and exhibits jitter" (p16).
- CLKOUT = 16× baud (1x), 8× (2x), 4× (4x) when CLKtoRTS = 1.
- 2x/4x modes: bits sampled once at mid-bit (no noise detect); IrDA unavailable (p18). (MAX3108/3109/14830 say 2x
  still takes three samples and only 4x samples once.)

### Behavior (p15–23)
- Receiver: start = RX high→low, mid-bit sampling with 3-sample majority (16x; Fig. 5, same as MAX3100: 7th/8th/9th
  sample); data LSB first; each FIFO entry stores the word + its error flags (parity, frame, noise, break).
- Transmitter: THR pushes TX FIFO; start/parity/stop added. Errata (p24): a THR write coinciding with the transmitter
  taking a word can lose that character; an RHR read coinciding with a received word can return a character twice
  (p23). (Real-chip bugs; a model may ignore them.)
- Auto direction control (MODE1[4]): RTS/CLKOUT = low (receive) when idle; goes high Setup bit-times before TX
  starts as soon as the TX FIFO has data; returns low Hold bit-times after the last stop bit; TxEmpty ISR[5] (p19,
  p31).
- Echo suppression (MODE2[7]): receiver gates data while transmitting and for Hold bit-times after (p20).
- AutoRTS (FlowCtrl[0]): RTS deasserted (high) when RX level reaches HALT, reasserted (low) when it falls to RESUME;
  no interrupts (p21). AutoCTS (FlowCtrl[1]): transmit only while CTS low; if CTS goes high mid-character the
  character completes then TX halts; CTSInt still works. Disable TX (MODE1[1]) before enabling AutoCTS/sw flow.
- XON/XOFF transmit control: receiver compares words with XON/XOFF; XOFF halts TX after the current word, XON
  restarts; matched characters filtered out. Receive control: XOFF sent when RX level reaches HALT, XON when the host
  reads it down to RESUME; for two-byte mode XON1/XOFF1 then XON2/XOFF2 (p22).
- Multidrop (p19): MODE2[6]; 8 data bits + 9th bit; MultiDropInt on address. Auto filtering (MODE2[6] + MODE2[4]):
  station address = XOFF2 (or XOFF2[7:4] + GPIO3..0 with GPIAddr; updated on GPIO changes); matching address → following
  data stored; non-matching → discarded; address words never stored. (MAX3108+ add: XOFF2Int also raised on own
  address.) Transmit of the 9th bit uses ForceParity (LCR[5]+LCR[3], EvenParity selects 0/1).
- Sleep (p22): ForcedSleep stops crystal, PLL, predivider, receiver, transmitter; SPI/I2C and registers stay alive.
  AutoSleep enters sleep when both FIFOs empty, no pending interrupts, and no input activity for 65 536 character
  times; exits on activity on any GPIO, RX or CTS; SleepInt on entry (and exit, p31). After wake, ClockReady
  signals clock stable (crystal only).
- Shutdown: RST pin low → everything off (interface, registers, FIFOs, clocks) except LDO; on RST high a hardware
  reset runs; IRQ goes high when initialization is done (with IRQSel = 0); must reprogram (p22).
- Power-up and IRQ (p22): with IRQSel = 0 IRQ is held low until the device is ready, then high; host then sets
  IRQSel = 1 for normal interrupts. Polling RevID is the alternative.
- Interrupt structure (p23): IRQ = IRQSel ? OR(ISR[i] & IRQEn[i]) : not-ready. Second level: LSR (masked by LSRIntEn
  → ISR[0]), SpclCharInt (→ ISR[1]), STSInt (→ ISR[2]). Reading ISR clears ISR and IRQ; SpclCharInt and STSInt clear
  on their own read; LSR bits clear only when their cause goes away.
- GPIO inputs generate GPIxInt on any change (enabled by STSIntEn[3:0] for routing).
- Outputs during reset: not tabulated for MAX3107 (IRQ low during reset/power-up; see MAX3109 for TX/RTS high).

### Timing (p8–9; VA 2.35–3.6, VL 1.71–3.6)
SPI: tCH+CL 38.4 ns min (26 MHz); tCH 16, tCL 16; tCSS (CS fall→SCLK rise) 0; tDS 5; tDH 3; tDO 20 max; tFT 10 max;
tCSH 32 ns. I2C: 100 / 400 kHz; tBUF 4.7 / 1.3 µs; tHD:STA 4.0 / 0.6; tLOW 4.7 / 1.3; tHIGH 4.0 / 0.6; tHD:DAT 0–0.9;
tSU:DAT 250 / 100 ns; tSU:STA 4.7 / 0.6; tSU:STO 4.7 / 0.6; spike filter 50 ns.

### VCC / inputs (p6–8)
VA 2.35–3.6 V, VL 1.71–3.6 V, VEXT 1.71–3.6 V, V18 1.65–1.95 V. Host pins abs max VL + 0.3 V; line pins (TX, RX,
RTS, CTS, GPIO) VEXT + 0.3 V; XIN 4.0 V. Not tolerant above their I/O supply. XIN VIH 1.2 V, VIL 0.3 V.

### Notes
- LCR reset 0x05 decodes (Table 2) to **6 data bits, 2 stop bits, no parity** — not 8N1. Program LCR (e.g. 0x03 = 8N1).
- Halt/Resume comparison wording varies ("reaches" vs "higher than" / "falls to" vs "below"); MAX3108 p22: RTS
  deasserted when filled "to a level higher than the HALT level", reasserted when it "falls to below the RESUME level".

---

## MAX3108

- Title: "MAX3108 SPI/I2C UART with 128-Word FIFOs in WLP" (max3108.pdf p1). "Register Compatible with the MAX3107".
- Parts: **MAX3108EWA+T** — 25-bump WLP 2.1x2.1 mm, 0.4 mm pitch (W252B2+1), −40 to +85 °C, VCC 1.71–3.6 V
  (2.35–3.6 V when the PLL is used), VL/VEXT 1.71–3.6 V (p1, p6). Pinout name `wlp25`.

### Ball map (p13–14, top view; 5x5, A1 at "+")
| | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|
| A | VCC power | MISO/SDA out/inout | CS/A0 in | IRQ out (OD) | VL power |
| B | V18 power | RST in | SCLK/SCL in | MOSI/A1 in | DGND ground |
| C | AGND ground | LDOEN in | N.C. | GPIO0 inout | GPIO1 inout |
| D | XIN in | XOUT out | SPI/I2C in | GPIO3 inout | CTS in |
| E | VEXT power | GPIO2 inout | RTS out | TX out | RX in (pull-up) |
Renames vs MAX3107: DOUT/SDA → MISO/SDA, DIN/A1 → MOSI/A1, VA → VCC, RTS/CLKOUT → RTS, mode pin printed SPI/I2C
(high = SPI). IRQ "asserted … during initial power-up" (p13).

### Differences from MAX3107 (everything else identical; register map p25)
- Register map: 0x00–0x1E only — **no RevID** (0x1F unused; init flow polls DIVLSB, p52).
- IRQEn/ISR names: ISR[6] RFifoEmptyInt, [5] TFifoEmptyInt ("transmit FIFO is empty and the transmitter is
  transmitting the last character"), [4] TxTrgInt, [3] RxTrigInt (same functions).
- **STSIntEn[7] TxEmptyIntEn / STSInt[7] TxEmptyInt** (new): set when TX FIFO empty **and** last character fully sent;
  cleared on STSInt read (p34).
- STSIntEn[3:0] GPIxIntEn *gate generation* of GPIxInt (not only routing) (p33).
- MODE1[7]: register map shows "—" but the bit description (p35) defines IRQSel exactly as MAX3107 ("global enable
  for all interrupts"); treat as IRQSel. MODE1[1] note: in auto direction control TxDisabl reads high when the
  transmitter is completely empty.
- LSR (p30): error bits clear "until the next character is read out of RHR"; **RxOverrun (LSR[1]) is cleared by
  reading RHR or LSR**. RTimeout set when *all* characters have been in the FIFO ≥ RxTimeOut.
- LCR[7] named RTSbit: RTS pin = RTSbit.
- CLKSource: **no ClockEn** (bit 4 "No Function" but resets to 1): CLKSource reset **0x18** (p47). Crystal selected by
  CrystalEn alone. Crystal internal load caps 16 pF (p18).
- Noise detection works in 1x **and 2x**; only 4x samples once (p17, p19).
- I2C: standard/fast/**fast-mode-plus 1 MHz** (p9); 16 addresses — each of MOSI/A1 and CS/A0 tied to DGND, VL, SCL or
  SDA (Table 5 p49, 8-bit write/read):

| A1 \ A0 | DGND | VL | SCL | SDA |
|---|---|---|---|---|
| DGND | D8/D9 | C2/C3 | C4/C5 | C6/C7 |
| VL | C8/C9 | CA/CB | CC/CD | CE/CF |
| SCL | D0/D1 | D2/D3 | D4/D5 | D6/D7 |
| SDA | C0/C1 | DA/DB | DC/DD | DE/DF |
  (verified on the page image). I2C burst auto-increments "unless the register being accessed is 0x00" (p50–51).
- SPI identical to MAX3107 (R/W + A6..A0, mode 0, burst rules) (p48). Timing p10: same except tCSH 30 ns.
- Supply: VCC 1.71–3.6 V with PLL disabled/bypassed, 2.35–3.6 V with PLL (p6). Power supplies may come up in any
  order (p54).
- Init: "wait for the IRQ output to deassert" or poll a known register (p52).

---

## MAX3109

- Title: "MAX3109 Dual Serial UART with 128-Word FIFOs" (max3109.pdf p1). Two UARTs "completely independent except
  for the input clock" (p16); register set per UART = MAX3108 set plus global/extended registers.
- Parts: **MAX3109ETJ+, MAX3109ETJ+T** — 32 TQFN-EP 5x5 (T3255+4, outline 21-0140, land 90-0012, p7), −40 to +85 °C,
  VCC 1.71–3.6 V (2.35 V min with PLL), VL/VEXT 1.71–3.6 V (p7, p65). Pinout `tqfn32`.

### Pinout tqfn32 (p14–16)
1 RST in (active low) · 2 MISO/SDA out/inout (Z when RST low) · 3 SCLK/SCL in · 4 GPIO7 inout · 5 CS/A0 in ·
6 MOSI/A1 in · 7 IRQ out (open drain; Z when RST low) · 8 VL power · 9 SPI/I2C in · 10 DGND ground · 11 GPIO0 inout
(UART0 clock out with TxSynch[7]) · 12 GPIO4 inout (UART1 clock out) · 13 GPIO1 inout (UART0 timer out) · 14 GPIO5
inout (UART1 timer out) · 15 CTS0 in · 16 CTS1 in · 17 TX1 out · 18 TX0 out · 19 RX0 in · 20 RX1 in · 21 RTS0 out ·
22 RTS1 out · 23 GPIO2 inout · 24 GPIO3 inout · 25 VEXT power · 26 XIN in · 27 XOUT out · 28 GPIO6 inout · 29 AGND
ground · 30 LDOEN in · 31 V18 power · 32 VCC power · EP → AGND ("Do not use EP as the main AGND connection").
GPIO ownership: UART0 GPIO0–3, UART1 GPIO4–7 (its GPIOConfg bit "GP0" = GPIO4, etc.; p28, p47).
Reset pin states (p14–15): TX_ and RTS_ logic high while RST low (or external V18 off); MISO and IRQ high-Z; RST low
also switches the V18 LDO off.

### SPI (p57–58)
- Mode 0. Command byte (Table 7): `bit7 W/R, bit6 0, bit5 U, bit4..0 A4..A0`. U = 0 → UART0, 1 → UART1.
- Write cycle (Fig. 18): MISO high-Z for the whole write. Read cycle (Fig. 19): MISO high-Z for the first 4 clocks
  of the command byte, then drives `0, 0, IRQ1, IRQ0` (GlobalIRQ, active low) on clocks 5–8, then D7..D0.
  **Fast read** (Fig. 20): an 8-clock read frame returns only the 2 IRQ bits; U ignored.
- Burst: as MAX3107 (auto-increment unless start address 0x00).
- Extended registers 0x20–0x25 via GloblComnd 0xCE (enable) / 0xCD (disable): while enabled the internal address is
  `0010 A3A2A1A0` (A4 ignored, U still selects the UART); disabled (default) `000A4 A3A2A1A0` (p53). Mapping (Table
  6): TxSynch SPI 0x00/I2C 0x20, SynchDelay1 0x01/0x21, SynchDelay2 0x02/0x22, TIMER1 0x03/0x23, TIMER2 0x04/0x24,
  RevID 0x05/0x25.
### I2C (p59–62)
Up to 1 MHz. Per-UART 7-bit addresses (Table 8, write/read): UART0 = the MAX3108 table above; UART1 = UART0 address
− 0x20 (e.g. A1A0 = DGND/DGND: UART0 D8/D9, UART1 B8/B9; DGND/VL: C2/C3 and A2/A3; SDA/DGND: C0/C1 and A0/A1).
Registers 0x20–0x25 directly addressable in I2C. Burst: auto-increment unless at 0x00.

### Register differences vs MAX3108 (map p28)
- Same 0x00–0x1E per UART (incl. STSInt[7] TxEmptyInt, SleepInt, CLKSource reset 0x18 with no ClockEn).
  **MODE1[7] is "—" (no IRQSel)**: "IRQ operates as a hardware active-low interrupt output" from power-up (p27).
- PLLConfig and CLKSource "can only be programmed by accessing UART0" (shared clock) (p28 note 4).
- 0x1F read **GlobalIRQ** (R, reset 0x03): [1] IRQ1, [0] IRQ0 — **0 = that UART has an interrupt pending**; set back
  to 1 when the UART's ISR is read (p52). Same value from either UART.
- 0x1F write **GloblComnd** (W, global — goes to both UARTs regardless of U): 0xE0–0xEF = Tx Command 0–15 (sync
  triggers), 0xCE = enable extended map, 0xCD = disable (Table 5 p53).
- 0x20 **TxSynch** (per UART): [7] CLKtoGPIO — UART reference clock (fractional-BRG input) out on GPIO0 (UART0) /
  GPIO4 (UART1). [6] TxAutoDis — TxDisabl set automatically when TX FIFO drains. [5] TrigDelay — delay start by
  SynchDelay. [4] SynchEn — transmitter waits for its trigger; forces MODE1[1] TxDisabl = 1. [3:0] TrigSel — which
  of the 16 Tx commands (0xE0+n) starts this UART (p54).
- 0x21/0x22 **SynchDelay1/2**: 16-bit delay in bit times (1/baud) from trigger to start bit (max 65 535) (p55).
- 0x23/0x24 **TIMER1/TIMER2**: Timer[14:0]; TIMER2[7] TmrToGPIO routes a 50 % clock to GPIO1 (UART0) / GPIO5
  (UART1): fTIMER = UARTClk / (1024 × Timer), UARTClk = 16 × baud (BRG output); both 0 → off (p56, p20).
- 0x25 **RevID**: bits printed 1100 0010 (0xC2) but the map note says reset 0xC1 and text "starting with 0xC0" (p28,
  p56). Conflict — see Notes.
- Trigger behavior (p21–24): transmission starts when the assigned command arrives, the transmitter is disabled and
  its FIFO has data. Command received at the 16th SCLK rising edge (SPI) / 8th SCL rising edge of the data byte
  (I2C). Intrinsic delay 5/UARTCLK ≤ tTRIG ≤ 6/UARTCLK (no fractional part; UARTCLK = BRG output). Interchip sync
  only in SPI (pull all CS low together). After auto-disable, re-enable by clearing TxAutoDis or toggling TxDisabl.
- Sleep: the two UARTs sleep separately; clocks fully off only when both sleep (p26). Shutdown < 1 µA (RST low);
  ready ≈ 200 µs after reset (p62); init by polling DIVLSB.
- Timing (p11): SPI as MAX3108 (tCSH 30 ns); I2C up to 1 MHz (fast-mode-plus: tLOW 0.5 µs, tHIGH 0.26 µs, tSU:DAT
  50 ns).

---

## MAX14830

- Title: "MAX14830 Quad Serial UART with 128-Word FIFOs" (max14830.pdf p1). Four UARTs, shared clock tree, one IRQ.
  Register set is MAX3107-style (IRQSel present) with **no sleep modes** and two extra bits; global/extended
  registers as MAX3109 (4 IRQ bits, 2 UART-select bits).
- Parts: **MAX14830ETM+, ETM+T** — 48 TQFN-EP 7x7 (T4877+3, outline 21-0144, land 90-0129, p67), −40 to +85 °C,
  **VA 2.35–3.6 V**, VL/VEXT 1.71–3.6 V (p8). Pinout `tqfn48`.

### Pinout tqfn48 (p15–17)
1 SPI/I2C in · 2 LDOEN in · 3 MISO/SDA · 4 SCLK/SCL in · 5 CS/A0 in · 6 MOSI/A1 in · 7 IRQ out (OD) · 8 RST in ·
9 VL power · 10 DGND ground · 11 GPIO0 (UART0 clock out) · 12 GPIO1 (UART0 timer) · 13 GPIO2 · 14 GPIO3 · 15 RTS0 out
· 16 CTS0 in · 17 RX0 in · 18 TX0 out · 19 GPIO4 (UART1 clock) · 20 GPIO5 (UART1 timer) · 21 GPIO6 · 22 GPIO7 ·
23 RTS1 · 24 CTS1 · 25 RX1 · 26 TX1 · 27 GPIO8 (UART2 clock) · 28 GPIO9 (UART2 timer) · 29 GPIO10 · 30 GPIO11 ·
31 RTS2 · 32 CTS2 · 33 RX2 · 34 TX2 · 35 GPIO12 (UART3 clock) · 36 GPIO13 (UART3 timer) · 37 GPIO14 · 38 GPIO15 ·
39 RTS3 · 40 CTS3 · 41 RX3 · 42 TX3 · 43 VEXT power · 44 XOUT out · 45 XIN in · 46 AGND ground · 47 VA power ·
48 V18 power · EP → AGND (not the main AGND connection). GPIOs are inout with weak pull-downs; RX_ weak pull-up.
GPIO ownership: UART n owns GPIO(4n)…GPIO(4n+3) (Tables 1, 5, 6 p38–50).

### SPI (p58–59)
Command byte (Table 10): `bit7 W/R, bit6 U1, bit5 U0, bit4..0 A4..A0`; U1U0 = 00/01/10/11 → UART0/1/2/3 (Table 11).
MISO high-Z during writes; during reads high-Z for the first 4 clocks, then IRQ3, IRQ2, IRQ1, IRQ0 (clocks 5–8,
GlobalIRQ, active low; driven on SCLK falling edges, verified on the p59 image), then D7..D0. Fast read: 8 clocks,
U bits ignored. Burst and extended addressing as MAX3109 (U1/U0 keep their meaning in extended mode, p54).
### I2C (p60, Table 12, write/read)
UART0 = MAX3108 table; UART1 = UART0 − 0x20; UART2 = UART0 − 0x80; UART3 = UART0 − 0xA0. Example A1A0 = DGND/DGND:
D8/D9, B8/B9, 58/59, 38/39; SDA/DGND: C0/C1, A0/A1, 40/41, 20/21. Up to 1 MHz.

### Register differences (map p28–29)
- IRQEn names: RFifoEmtyIEn, TFifoEmtyIEn, TFifoTrgIEn, RFifoTrgIEn (same functions). ISR reset 0x60, same bits.
- LSRIntEn[5] RxNoiseIntEn. LSR as MAX3107 ("stay set until the next character without errors is read", p34).
- **STSIntEn / STSInt: only [5] ClockReady and [3:0] GPIxInt** (no TxEmptyInt, no SleepInt) (p37–38).
- **MODE1: [7] IRQSel, [6:5] no function** (no AutoSleep/ForcedSleep), [4:0] as MAX3107 (p39). No sleep modes at all.
- MODE2[0] RST resets the selected UART only; "global registers are not reset" (p40).
- **IrDA[2] RTSInvert** — invert the RTS output (p43).
- **BRGConfig[6] CLKDisabl** — 1 stops that UART's internal clock (fast baud reprogramming: set, change divisor,
  clear; or power saving) (p52, p20).
- CLKSource reset **0x08**; bit 4 "can be programmed to logic 0 or logic 1" (no function); PLLConfig and CLKSource
  programmable only via UART0 (p29, p53). Crystal: CLKDisabl = 0 and CrystalEn = 1; 16 pF internal load caps (p20).
- 2x mode keeps 3-sample noise detection; 4x samples once (p20–21). Baud formula as MAX3107.
- 0x1F GlobalIRQ (reset 0x0F): IRQ3..IRQ0, 0 = pending; reading GlobalIRQ does not clear IRQ (p28, p53).
  0x1F GloblComnd, 0x20 TxSynch (CLKtoGPIO → GPIO0/4/8/12), 0x21–0x22 SynchDelay, 0x23–0x24 TIMER (TmrToGPIO →
  GPIO1/5/9/13; fTIMER = UARTClk/(1024 × Timer), UARTClk = 16/8/4 × baud in 1x/2x/4x) as MAX3109 (p54–57).
- 0x25 RevID = **0xB4** (p57; Rev 7 changed it from 0xB3).
- Power-up: IRQ low until ready (IRQSel = 0), or poll DIVLSB (p27). Shutdown: RST low, LDO stays on (p27).
- Trigger delay: 5×BR/16 ≤ tTRIG ≤ 6×BR/16, BR = fractional-divider output clock period (p24).
- Timing (p11–12): SPI tCH+CL 38.4 ns, tCH/tCL 16, tCSS 0, tDH 3, tDS 5, tDO 20 max, tFT 10, tCSH 30 ns; I2C 100 k /
  400 k / 1 MHz.

### Notes (MAX3109 / MAX14830)
- MAX3109 RevID: register bits read 0xC2, footnote says 0xC1, text "starting with 0xC0". Pick 0xC2 (the bit row)
  or flag; software is told to check only the upper nibble.
- MAX14830 auto transmitter disable text says "TxSynch[4]" (p24); the register (p54) puts TxAutoDis at bit 6 and
  SynchEn at bit 4. Use the register page.
- MAX14830 title page still claims "Integrated Internal Oscillator" but Rev 2 "Removed internal oscillator
  description throughout" (p67) — there is no internal oscillator; only crystal or XIN clock. Same for MAX3107 Rev 3.
- W/R bit polarity of the SPI command byte is not printed in any of the four datasheets (see MAX3107 section).
- Analog/drive data (GPIO 20 mA, VOH/VOL) omitted.
